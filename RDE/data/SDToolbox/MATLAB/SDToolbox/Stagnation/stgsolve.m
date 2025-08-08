function [output] = stgsolve(gas,gas1,U1,Delta,varargin)
% Shock and Detonation Toolbox
% http://shepherd.caltech.edu/EDL/public/sdt/SD_Toolbox/
%
% stgsolve - Reaction zone structure computation for blunt body flow using
% Hornungs approximate of linear gradient in rho u 
%
% FUNCTION
% SYNTAX
% [output] = stgsolve(gas,gas1,U1,Delta)
%
% INPUT
% gas = Cantera gas object - postshock state
% gas1 = Cantera gas object - initial state
% U1 = Shock Velocity
% Delta = shock standoff distance
%
% OPTIONAL INPUT (positional argument):
% t_end = end time for integration, in sec. If not included
%         as an input, set to 10^-3 by default.
%
% OUTPUT
% Structure
%   output.time = time array
%   output.distance = distance array
%
%   output.T = temperature array
%   output.P = pressure array
%   output.rho = density array
%   output.U = velocity array
%   output.thermicity = thermicity array
%
%   output.M = Mach number array
%   output.af = frozen sound speed array
%   output.g = gamma (cp/cv) array
%   output.wt = mean molecular weight array
%   output.sonic = sonic parameter (c^2-U^2) array
%
% SUBFUNCTION CALLS
% odeset.m, cputime.m, ode15s.m, size.m, max.m, abs.m, transpose.m, line.m,
% gca.m, axes.m, set.m, get.m
% Cantera Functions: density.m, pressure.m, nSpecies.m, soundspeed.m, set.m, meanMolecularWeights.m,
%    gasconstant.m, oneatm.m, netProdRates.m, enthalpies_RT.m, cp_mass.m,
%    cv_mass.m


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Define the initial density, pressure, number of species and species mass fraction information
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
r1 = density(gas1);
r = density(gas);
U = U1*r1/r;
PSC = pressure(gas);
nsp = nSpecies(gas);
y0 = [1
      r
      U
      0
      massFractions(gas)];
  
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Parse optional name-value pair arguments
default_t_end = 1e-3;
default_rel_tol = 1e-5;
default_abs_tol = 1e-8;

p = inputParser;
addRequired(p,'gas');
addRequired(p,'gas1');
addRequired(p,'U1');
addRequired(p,'Delta');
addParameter(p,'t_end',default_t_end,@isnumeric);
addParameter(p,'abs_tol',default_abs_tol,@isnumeric);
addParameter(p,'rel_tol',default_rel_tol,@isnumeric);
parse(p,gas,gas1,U1,Delta,varargin{:});
t_end = p.Results.t_end;
abs_tol = p.Results.abs_tol;
rel_tol = p.Results.rel_tol;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

tel = [0 t_end];
options = odeset('RelTol',rel_tol,'AbsTol',abs_tol,'Stats','off');
warning off MATLAB:ode15s:IntegrationTolNotMet;

% Call the integrator to march in time - the equation set is defined in the
% routine stgsys.m
out = ode15s(@stgsys,tel,y0,options,gas,U1,r1,PSC,Delta);

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
output.Delta = Delta;

temp_grad = zeros(b,1);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Extract TIME, TEMPERATURE, PRESSURE, and DENSITY arrays from integrator output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
for i = 1:1:b
    output.time(i) = out.x(i);
    % compute species
    for m=1:nsp
        y(m,1) = abs(out.y(m+4,i));
    end
    % set gas in order to compute mean molar mass (MW)
    rho = out.y(2,i);
    set(gas, 'Rho', rho, 'Y', transpose(y));
    wt = meanMolecularWeight(gas);
    % compute gas temperature from P, rho and MW
    T = (out.y(1,i)*PSC/rho)*(wt/gasconstant);
    % Now set gas state (without iteration) using P, T and Y
    set(gas, 'T', T, 'P', out.y(1,i)*PSC, 'Y', transpose(y));
    % set other variables in output structure
    output.P(i) = out.y(1,i)*PSC;
    output.rho(i) = rho;
    output.U(i) = out.y(3,i);
    output.distance(i) = out.y(4,i);
    output.T(i) = T;
    for k =1:nsp
        output.species(i,k) = y(k,1);
    end

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Extract WEIGHT, GAMMA, SOUND SPEED, VELOCITY, MACH NUMBER, c^2-U^2,
    % THERMICITY, and TEMPERATURE GRADIENT 
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %Vectors
    wdot = netProdRates(gas);
    hs = enthalpies_RT(gas)*gasconstant*T;
    mw = molecularWeights(gas);

    %Scalars
    cp = cp_mass(gas);
    cv = cv_mass(gas);
    g = cp/cv;
    af = soundspeed_fr(gas);     % frozen soundspeed(gas);
    M = output.U(i)/af;          % Mach Number in shock-fixed frame
    eta = 1 - M^2;               % Sonic Parameter
    sonic = af^2*eta;
    walpha = U1*r1/Delta/rho;% area change function

    sum = 0;
    for n = 1:1:nsp
        dykdt = mw(n)*wdot(n)/rho; %Net production rate of species z (kg/sec)
        drdy = -wt/mw(n); % mean molecular weight / molecular weight of species z
        a = hs(n)/(cp*T*mw(n));  % factor that accounts for chemical energy change
        sum = sum - (drdy + a)*dykdt;
    end
    sigmadot = sum;  %thermicity
                           
    Pdot = -rho*U^2*(sigmadot-walpha)/eta/PSC;   %Pressure Derivative (Normalized by Post-Shock pressure to reduce size)
    rdot = -rho*(sigmadot-walpha*M^2)/eta;       %Density Derivative
    Udot = U*(sigmadot-walpha)/eta;                  %Velocity Derivative

    % FIND TEMPERATURE GRADIENT
    temp_grad(i) = T*(Pdot/out.y(1,i) - rdot/r);
 
    % Assign output structure
    output.thermicity(i) = sigmadot;
    output.M(i) = M;
    output.af(i) = af;
    output.g(i) = g;
    output.wt(i) = wt;
    output.sonic(i) = sonic;    
end

output.gas1 = gas1;
output.U1 = U1;
