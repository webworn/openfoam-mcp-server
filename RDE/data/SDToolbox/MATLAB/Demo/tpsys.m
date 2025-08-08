function dydt = tpsys(t,y,gas,mw,T,P)
% Evaluates the system of ordinary differential equations for an adiabatic, 
%     constant-temperature and pressure, zero-dimensional reactor. 
%     It assumes that the 'gas' object represents a reacting ideal gas mixture.
% 
%     INPUT:
%         t   = time
%         y   = solution array [species mass fraction 1, 2, ...]
%         gas = working gas object
%         mw  = array of species molar masses
%         T   = temperature
%         P   = pressure
%     
%     OUTPUT:
%         An array containing time derivatives of:
%              species mass fractions, 
%         formatted for the calling integrator, usually ode15s.
%

% Set the state of the gas, based on the current solution vector.
set(gas, 'T', T, 'P', P, 'Y', y(1:end));
nsp = nSpecies(gas);
% species molar production rates
wdot = netProdRates(gas);
% set up column vector for dydt
dydt = [ zeros(nsp,1) ];
% species time evolution equations
rrho = 1.0/density(gas);
for i = 1:nsp
  dydt(i) = rrho*mw(i)*wdot(i);
end
