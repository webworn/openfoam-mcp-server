function [output] = cpsolve(gas,varargin)
% Solves the constant-volume, adiabatic explosion model equations.
% Use the ODE system defined in cvsys, taking the gas object input as the
% initial state.
% 
% FUNCTION SYNTAX:
%     output = cvsolve(gas,varargin)
% 
% INPUT:
%     gas = working gas object
% 
% OPTIONAL INPUT (name-value pairs):
%     t_end = end time for integration, in sec. If not included
%             as an input, set to 10^-3 by default.%
%      rel_tol = relative tolerance
%      abs_tol = absolute tolerance
% 
% OUTPUT:
%     output = a structure containing the following results:
%          time = time array
%          T = temperature profile array
%          P = pressure profile array
%          speciesY = species mass fraction array
%          speciesX = species mole fraction array
%          
%          gas = working gas object
%          
%          exo_time = pulse width (in secs) of temperature gradient (using 1/2 max)
%          ind_time = time to maximum temperature gradient
%          ind_time_10 = time to 10% of maximum temperature gradient
%          ind_time_90 = time to 90% of maximum temperature gradient
%
% Last edit:  Feb 4, 2021
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%RUN CONSTANT VOLUME EXPLOSION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
mw = molecularWeights(gas);
P0 = pressure(gas);
nsp = nSpecies(gas);

y_mass = massFractions(gas);
y0 = [temperature(gas)
      y_mass];

  
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Parse optional name-value pair arguments
default_t_end = 1e-3;
default_rel_tol = 1e-5;
default_abs_tol = 1e-12;

p = inputParser;
addRequired(p,'gas');
addParameter(p,'t_end',default_t_end,@isnumeric);
addParameter(p,'abs_tol',default_abs_tol,@isnumeric);
addParameter(p,'rel_tol',default_rel_tol,@isnumeric);
parse(p,gas,varargin{:});
t_end = p.Results.t_end;
abs_tol = p.Results.abs_tol;
rel_tol = p.Results.rel_tol;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

tel = [0 t_end];
options = odeset('RelTol',rel_tol,'AbsTol',abs_tol,'Stats','off');
% t0 = cputime;

% out.x = time, out.y = temperature and species profiles
out = ode15s(@cpsys,tel,y0,options,gas,mw,P0);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%EXTRACT OUTPUT INFORMATION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
[~,b] = size(out.y);

%Initalize output matrices
output.ind_time = 0;
output.ind_time_90 = 0;
output.ind_time_10 = 0;
output.exo_time = 0;
output.time = zeros(1,b);
output.P = zeros(1,b);
output.T = zeros(1,b);
output.speciesY = zeros(b,nsp);
output.speciesX = zeros(b,nsp);

temp_grad = zeros(1,b);

for i = 1:b
    T = out.y(1,i);
    Y = out.y(2:end,i);
    set(gas, 'T', T, 'P', P0, 'Y', Y');
    P = pressure(gas);
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % FIND TEMPERATURE GRADIENT                                                 % 
    % Conservation of Energy @ p = constant => h0 = h(T)                             % 
    %  dhdt = 0 =>  cp*dTdt + sum(hi(T)*dyidt) = 0; dyidt = wdoti*wti/rho %
    % dTdt = -sum(hi(T)*wdoti*wti)/(cp*rho)                                     % 
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    cp = cp_mass(gas);
    wdot = netProdRates(gas);
    hs = enthalpies_RT(gas);
    R = gasconstant;
    wt = meanMolecularWeight(gas);
    rho = density(gas);
    sum = 0;
    for z = 1:1:nsp
        w = mw(z);
        h = R*T*hs(z)/w;
        wd = wdot(z);
        sum = sum + h*wd*w;
    end
   temp_grad(i) = -sum/(rho*cp);
    
    output.time(i) = out.x(1,i);
    output.T(i) = T;
    output.P(i) = P;
    output.rho(i) = rho;
    output.speciesY(i,:) = Y;
    output.speciesX(i,:) = moleFractions(gas);
    output.gas = gas;
end

%%%%%%% TESTING %%%%%%%
% figure()
% plot(output.time,output.T)
% output.temp_grad = temp_grad;
%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%FIND INDUCTION TIME - MAXIMUM TEMPERATURE GRADIENT
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
[~,n] = max(temp_grad);

if (n == b)
    disp('Error: Maximum temperature gradient occurs at the end of the reaction zone')
    disp('       Your final integration length may be too short,')
    disp('       your mixture may be too rich/lean, or something else may be wrong')
    disp(' ')
    output.ind_time = output.time(b); 
    output.ind_time_10 = output.time(b); 
    output.ind_time_90 = output.time(b); 
    output.exo_time = 0; 
    display(['Induction Time: ' num2str(output.ind_time)])
    display(['Exothermic Pulse Time: ' num2str(output.exo_time)])
    return
elseif (n == 1)
    disp('Error: Maximum temperature gradient occurs at the beginning of the reaction zone')
    disp('       Your final integration length may be too short,')
    disp('       your mixture may be too rich/lean, or something else may be wrong')
    disp(' ')
    output.ind_time = output.time(1); 
    output.ind_time_10 = output.time(1); 
    output.ind_time_90 = output.time(1); 
    output.exo_time = 0; 
    display(['Induction Time: ' num2str(output.ind_time)])
    display(['Exothermic Pulse Time: ' num2str(output.exo_time)])
    return
else
    output.ind_time = output.time(n);
    k = 1;
    MAX10 = 0.1*max(temp_grad);
    d = temp_grad(1);
    while(d < MAX10)
        k = k + 1;
        d = temp_grad(k);
    end
    output.ind_time_10 = output.time(k);
    k = 1;
    MAX90 = 0.9*max(temp_grad);
    d = temp_grad(1);
    while(d < MAX90)
        k = k + 1;
        d = temp_grad(k);
    end
    output.ind_time_90 = output.time(1,k);

    % find exothermic time
    tsteps = size(output.T,2);
    max_temp_grad = max(temp_grad ); % max temperature
    half_T_flag1 = 0;
    half_T_flag2 = 0;
    for j = 1:tsteps-1  % Go into a loop to find two times when Temperature is half its maximum
        if half_T_flag1 ==0
            if temp_grad(j) > 0.5* max_temp_grad
                half_T_flag1 = 1;
                tstep1 = j;
            end
        elseif half_T_flag2 ==0
            if temp_grad(j) < 0.5* max_temp_grad
                half_T_flag2 = 1;
                tstep2 = j;
            else
                tstep2 = 0;
            end
        end
    end
end

%Exothermic time for CV explosion
if(tstep2 == 0)
    disp('Error: No pulse in the temperature gradient')
    disp('       Your final integration length may be too short,')
    disp('       your mixture may be too rich/lean, or something else may be wrong') 
    output.exo_time = 0; 
else
    output.exo_time = output.time(tstep2) - output.time(tstep1); 
end
