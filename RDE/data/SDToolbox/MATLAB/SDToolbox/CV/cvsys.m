function dydt = cvsys(t,y,gas,mw,R0)
% Evaluates the system of ordinary differential equations for an adiabatic, 
%     constant-volume, zero-dimensional reactor. 
%     It assumes that the 'gas' object represents a reacting ideal gas mixture.
% 
%     INPUT:
%         t = time  (not used in this routine but required by ode solver
%         y = solution array [temperature, species mass fraction 1, 2, ...]
%         gas = working gas object
%         mw = array of molar masses for species in gas object
%         R0 = mass density (fixed)
%     
%     OUTPUT:
%         An array containing time derivatives of:
%         temperature and species mass fractions, 
%         formatted in a way that the integrator in cvsolve can recognize.
%
%  Edit:  4 Feb 2021
% Set the state of the gas, based on the current solution vector.
set(gas, 'T', y(1), 'Rho', R0, 'Y', y(2:end));
nsp = nSpecies(gas);

% energy equation
wdot = netProdRates(gas);
Tdot = - temperature(gas) * gasconstant * (enthalpies_RT(gas) - ones(nsp,1))' ...
       * wdot / (R0*cv_mass(gas));

% set up column vector for dydt
dydt = [ Tdot
	 zeros(nsp,1) ];

% species equations
for i = 1:nsp
  dydt(i+1) = mw(i)*wdot(i)/R0;
end
