function dydt = adiasys(t,y,gas,mw,rho0,X0,m)
% Shock and Detonation Toolbox
% http://shepherd.caltech.edu/EDL/public/cantera/html/SD_Toolbox/
%
% adiasys.m
% Evaluates the system of ordinary differential equations for an adiabatic, 
% rapid compression,zero-dimensional reactor. 
% It assumes that the 'gas' object represents a reacting ideal gas mixture. 
%
% FUNCTION
% SYNTAX
% out = ode15s(@adiasys,tel,y0,options,gas,mw);
%
% INPUT
% tel = integration time interval (s)
% y0 = array of initial guesses
%   gas temperature
%   gas mass fractions
% options = options string for fsolve
% gas = working gas object
% mw = array of molecular weights
%
% OUTPUT
% out = matrix of output
%   out.x = time
%   out.y = X, U, temperature and species profiles

% Set the state of the gas, based on the current solution vector.
rho = X0*rho0/(y(1)+.001);
set(gas, 'T', y(3), 'Rho', rho, 'Y', y(4:end));
nsp = nSpecies(gas);
P = pressure(gas);
% energy equation
wdot = netProdRates(gas);
Tdot = -1*(y(3) * gasconstant * (enthalpies_RT(gas) - ones(nsp,1))' ...
      * wdot / rho + P*y(2)/(rho*y(1)))/cv_mass(gas);
% Tdot = -1*(y(3) * gasconstant * (enthalpies_RT(gas) - ones(nsp,1))' ...
%        * wdot / rho)/cv_mass(gas);
Udot =  (P-oneatm)/m;
% set up column vector for dydt
dydt = [ y(2)
         Udot
         Tdot
	     zeros(nsp,1) ];

% species equations
for i = 1:nsp
  dydt(i+3) = mw(i)*wdot(i)/rho;
end
