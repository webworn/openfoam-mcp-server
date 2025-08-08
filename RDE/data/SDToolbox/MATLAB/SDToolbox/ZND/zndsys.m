function dydt = zndsys(t,y,gas,U1,r1,PSC)
%% zndsys
% Shock and Detonation Toolbox
% http://shepherd.caltech.edu/EDL/PublicResources/sdt
% ZND Model Detonation Structure Computation module
% Revised 1/21/2021
%
% FUNCTION  SYNTAX:
%
% dydt = zndsys(t,y,gas,U1,r1,PSC)
% ALWAYS called by an ODE solver, in this case: out = ode15s(@zndsys,tel,y0,options,gas,U1,r1,PSC)
%       tel = time span
%       y0 = initial conditions array
%       options = specified in znd.m
%       gas = Cantera gas object
%       U1 = Shock Velocity
%       r1 = initial density
%       PSC = Post-shock pressure
%
% INPUT
% t = time
% y = solution array [pressure, density, position, species mass 1, 2, ..]
% gas = Cantera gas object
% U1 = Shock Velocity
% r1 = initial density
% PSC = Post-shock pressure
%
% OUTPUT
% dydt = Array of ODEs to be solved by ode15s  
% dydt = [dpressuredt, ddensitydt, dpostiondt,dmass1dt, dmass2dt, ...]
%
% SUBFUNCTION CALLS
% Cantera Functions: set.m, meanMolecularWeight.m, gasconstant.m,
%       density.m, nSpecies.m, netProdRates.m, enthalpies_RT.m,
%       molecularWeights.m, cp_mass.m, soundspeed.m, temperature.m 
%% compute current gas state from current solution
rho = y(2);
set(gas, 'P', y(1)*PSC, 'Rho', rho, 'Y', y(4:end));
wt = meanMolecularWeight(gas);
T = temperature(gas);
nsp = nSpecies(gas);

%Vectors
wdot = netProdRates(gas);
hs = enthalpies_RT(gas)*gasconstant*T;
mw = molecularWeights(gas);

%Scalars
cp = cp_mass(gas);
c = soundspeed(gas);

U = U1*r1/rho;
M = U/c;                       %Mach Number
eta = 1 - M^2;                 %Sonic Parameter
 
%% Loop through all of the species,calculate thermicity for this time step
sum = 0;
for i = 1:nsp
    dykdt(i) = mw(i)*wdot(i)/rho; %Net production rate of species z (kg/sec)
    drdy = -wt/mw(i); % mean molecular weight / molecular weight of species z
    a = hs(i)/(cp*T*mw(i));  % factor that accounts for chemical energy change
    sum = sum - (drdy + a)*dykdt(i);
end
sigmadot = sum;                     %Thermicity

Pdot = -rho*U^2*sigmadot/eta/PSC;   %Pressure Derivative (Normalized by Post-Shock pressure to reduce size)
rdot = -rho*sigmadot/eta;           %Density Derivative
%% column vector is used to return solution
dydt = [ Pdot
         rdot
         U
         zeros(nsp,1) ];

% species equations 
for i = 1:nsp
  dydt(i+3) = mw(i)*wdot(i)/rho; % mass production rates
end
