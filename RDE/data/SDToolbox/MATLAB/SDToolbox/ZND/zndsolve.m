function [output] = zndsolve(gas,gas1,U1,varargin)
%% zndsolve
% Shock and Detonation Toolbox
% http://shepherd.caltech.edu/EDL/PublicResources/sdt
% ZND Model Detonation Structure Computation module
% Revised 1/21/2021
%
% FUNCTION SYNTAX:
% [output] = zndsolve(gas,gas1,U1)
%
% INPUT:
% gas = Cantera gas object - postshock state
% gas1 = Cantera gas object - initial state
% U1 = Shock Velocity
%
% OPTIONAL INPUT (name-value pairs):
%      t_end = end time for integration, in sec
%      rel_tol = relative tolerance
%      abs_tol = absolute tolerance
%      advanced_output = calculates optional extra parameters such as induction lengths
%      max_step = maximum step size (in time) that solver is allowed to
%      take
%
% OUTPUT:
%      output = a dictionary containing the following results:
%          time = time array
%          distance = distance array
%             
%          T = temperature array
%          P = pressure array
%          rho = density array
%          U = velocity array
%          thermicity = thermicity array
%          species = species mass fraction array
%             
%          M = Mach number array
%          af = frozen sound speed array
%          g = gamma (cp/cv) array
%          wt = mean molecular weight array
%          sonic = sonic parameter (c^2-U^2) array
%             
%          tfinal = final target integration time
%          xfinal = final distance reached
%            
%          gas1 = a copy of the input initial state
%          U1 = shock velocity
%            
%          and, if advanced_output=True:
%          ind_time_ZND = time to maximum thermicity gradient
%          ind_len_ZND = distance to maximum thermicity gradient
%          exo_time_ZND = pulse width (in secs) of thermicity  (using 1/2 max)
%          ind_time_ZND = pulse width (in meters) of thermicity (using 1/2 max)
%          max_thermicity_ZND = maximum value of thermicity
%   
% SUBFUNCTION CALLS
% Cantera Functions: set.m, meanMolecularWeight.m, gasconstant.m,
%       density.m, nSpecies.m, netProdRates.m, enthalpies_RT.m,
%       molecularWeights.m, cp_mass.m, soundspeed.m, temperature.m 
%
% SDToolbox model from ZND folder
%       zndsys.m  - defines odes for integration 
%
%%
%Define the initial density, pressure, number of species and species mass fraction information
r1 = density(gas1);
PSC = pressure(gas);
nsp = nSpecies(gas);
y0 = [1
      density(gas)
      0
      massFractions(gas)];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Parse optional name-value pair arguments
default_t_end = 1e-3;
default_advanced_output = false;
default_rel_tol = 1e-5;
default_abs_tol = 1e-8;
default_max_step = 1E-5;

p = inputParser;
addRequired(p,'gas');
addRequired(p,'gas1');
addRequired(p,'U1');
addParameter(p,'t_end',default_t_end,@isnumeric);
addParameter(p,'advanced_output',default_advanced_output,@islogical);
addParameter(p,'abs_tol',default_abs_tol,@isnumeric);
addParameter(p,'rel_tol',default_rel_tol,@isnumeric);
addParameter(p,'max_step',default_max_step,@isnumeric);
parse(p,gas,gas1,U1,varargin{:});
t_end = p.Results.t_end;
advanced_output = p.Results.advanced_output;
abs_tol = p.Results.abs_tol;
rel_tol = p.Results.rel_tol;
max_step = p.Results.max_step;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

tel = [0 t_end];
options = odeset('RelTol',rel_tol,'AbsTol',abs_tol,'MaxStep',max_step,'Stats','off');
warning off MATLAB:ode15s:IntegrationTolNotMet;

% Call the integrator to march in time - the equation set is defined in the
% routine ZNDReactor.m
out = ode15s(@zndsys,tel,y0,options,gas,U1,r1,PSC);

output = out;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Extract information from the large output array 'out' and split up into
%individual  data arrays for each physical quantity of interest
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Find the output array dimensions
[a,b] = size(out.y); % a = 3 + species; b = number of iterations

%Initialize species profile for current iteration with zeros 
y = zeros(nsp,1); 
%Initalize output matrices

output.time = zeros(b,1);
output.tfinal = t_end;
output.distance = zeros(b,1);
output.P = zeros(b,1);
output.T = zeros(b,1);
output.U = zeros(b,1);
output.rho = zeros(b,1);
output.thermicity = zeros(b,1);
output.M = zeros(b,1);
output.af = zeros(b,1);
output.g = zeros(b,1);
output.wt = zeros(b,1);
output.sonic = zeros(b,1);
output.species = zeros(b,nsp);
if advanced_output
    output.ind_len_ZND = 0;
    output.ind_time_ZND = 0;
    output.exo_len_ZND = 0;
    output.exo_time_ZND = 0;
    output.max_thermicity_ZND = 0;
end

temp_grad = zeros(b,1);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Extract TIME, TEMPERATURE, PRESSURE, and DENSITY arrays from integrator output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
for i = 1:1:b
    output.time(i) = out.x(i);
    m = 4;
    while(m <= a)
        y(m-3,1) = abs(out.y(m,i));
        m = m + 1;
    end

    set(gas, 'Rho', out.y(2,i), 'Y', transpose(y));
    wt = meanMolecularWeight(gas);
    T = (out.y(1,i)*PSC/out.y(2,i))*(wt/gasconstant);
    set(gas, 'T', T, 'P', out.y(1,i)*PSC, 'Y', transpose(y));
    
    %output.P(i) = out.y(1,i)*PSC/oneatm;
    output.P(i) = out.y(1,i)*PSC;
    output.rho(i) = out.y(2,i);
    output.distance(i) = out.y(3,i);
    output.T(i) = T;
    x = moleFractions(gas);
    for k =1:nsp
        output.species(i,k) = y(k,1);
        output.speciesX(i,k) = x(k,1);     
    end

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Extract WEIGHT, GAMMA, SOUND SPEED, VELOCITY, MACH NUMBER, c^2-U^2,
    % THERMICITY, and TEMPERATURE GRADIENT
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    den = output.rho(i);
    nsp = nSpecies(gas);

    %Vectors
    wdot = netProdRates(gas);
    hs = enthalpies_RT(gas)*gasconstant*T;
    mw = molecularWeights(gas);

    %Scalars
    cp = cp_mass(gas);
    cv = cv_mass(gas);
    g = cp/cv;
    af = sqrt(g*gasconstant/wt*T);% soundspeed(gas);
    r = density(gas);
    
    U = U1*r1/r;
    M = U/af;                               %Mach Number
    eta = 1 - M^2;                          %Sonic Parameter
    sonic = eta*af^2;
 
    sum = 0;
    for n = 1:1:nsp
        h = hs(n)/mw(n);
        wd = wdot(n);
        w = mw(n);
        dykdt = w*wd/den;
        drdy = -den*wt/w;
        term = den*h/(cp*T);
        sum = sum - (drdy + term)*dykdt;
    end
    sigma = sum/den;                        %Thermicity
    Pdot = -U^2*sum/eta/PSC;                %Pressure Derivative
    rdot = -sum/eta;                        %Density Derivative

    % FIND TEMPERATURE GRADIENT
    temp_grad(i) = T*(Pdot/out.y(1,i) - rdot/r);
 
    % Assign ouptput structure
    output.U(i) = U;
    output.thermicity(i) = sigma;
    output.M(i) = M;
    output.af(i) = af;
    output.g(i) = g;
    output.wt(i) = wt;
    output.sonic(i) = sonic;    
end


if advanced_output
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %Find INDUCTION TIME and LENGTH based on MAXIMUM THERMICITY
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    [~,n] = max(output.thermicity);

    output.ind_time_ZND = output.time(n);
    output.ind_len_ZND = output.distance(n);
    output.xfinal = output.distance(b);
    output.max_thermicity_ZND = max(output.thermicity);  

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %Check for Eigenvalue Detonation
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    if (n == b)
        disp('Error: Maximum thermicity occurs at the end of the reaction zone')
        disp('       You may have an eigenvalue detonation, your final integration length may be too short,')
        disp('       your mixture may be too rich/lean, or something else may be wrong')
        disp(' ')
        disp(['Mach Number (end of reaction): ' num2str(output.M(b)) ' - if close to 1, check for eigenvalue detonation'])
        output.ind_time_ZND = output.time(b); 
        output.ind_len_ZND = output.distance(b); 
        output.exo_time_ZND = 0; 
        output.exo_len_ZND = 0;  
        disp(['Induction Time: ' num2str(output.ind_time_ZND)])
        disp(['Exothermic Pulse Time: ' num2str(output.exo_time_ZND)])
        return
    elseif (n == 1)
        disp('Error: Maximum thermicity occurs at the beginning of the reaction zone')
        disp('       You may have an eigenvalue detonation, your final integration length may be too short,')
        disp('       your mixture may be too rich/lean, or something else may be wrong')
        disp(' ')
        disp(['Mach Number (end of reaction): ' num2str(output.M(b)) ' - if close to 1, check for eigenvalue detonation'])
        output.ind_time_ZND = output.time(1); 
        output.ind_len_ZND = output.distance(1); 
        output.exo_time_ZND = 0; 
        output.exo_len_ZND = 0;  
        disp(['Induction Time: ' num2str(output.ind_time_ZND)])
        disp(['Exothermic Pulse Time: ' num2str(output.exo_time_ZND)])
        return
    else
        max_sigmadot = max(output.thermicity); % max thermicity
        half_sigmadot_flag1 = 0;
        half_sigmadot_flag2 = 0;
        % Go into a loop to find two times when sigma_dot is half its maximum
        for j = 1:b
            if half_sigmadot_flag1 ==0
                if output.thermicity(j) > 0.5* max_sigmadot
                    half_sigmadot_flag1 = 1;
                    tstep1 = j;
                end
            elseif half_sigmadot_flag2 ==0
                if output.thermicity(j) < 0.5* max_sigmadot
                    half_sigmadot_flag2 = 1;
                    tstep2 = j;
                else
                    tstep2 = 0;
                end
            end
        end
    end

    %Exothermic time for ZND detonation
    if(tstep2 == 0)
        disp('Error: No pulse in the thermicity')
        disp('       You may have an eigenvalue detonation, your final integration length may be too short,')
        disp('       your mixture may be too rich/lean, or something else may be wrong') 
        output.exo_time_ZND = 0; 
        output.exo_len_ZND = 0;  
    else
        output.exo_time_ZND = output.time(tstep2) - output.time(tstep1); 
        output.exo_len_ZND = output.distance(tstep2) - output.distance(tstep1);  
    end
end

% Append extra data used to make output file (via znd_fileout)
output.gas1 = gas1;
output.U1 = U1;
