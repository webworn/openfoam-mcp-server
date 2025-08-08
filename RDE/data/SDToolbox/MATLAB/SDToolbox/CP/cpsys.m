function dydt = cpsys(t,y,gas,mw,P0)
% Evaluates the system of ordinary differential equations for an adiabatic, 
%     constant-pressure, zero-dimensional reactor. 
%     It assumes that the 'gas' object represents a reacting ideal gas mixture.
% 
%     INPUT:
%         t = time
%         y = solution array [temperature, species mass 1, 2, ...]
%         gas = working gas object
%         mw = array of molar masses of species
%         P0 = gas pressure (constant)
% 
%     
%     OUTPUT:
%         An array containing time derivatives of:
%             temperature and species mass fractions, 
%         formatted in a way that the integrator in cvsolve can recognize.
%
% Last edit: Feb 4, 2021
% Set the state of the gas, based on the current solution vector.
set(gas, 'T', y(1), 'P', P0, 'Y', y(2:end));
nsp = nSpecies(gas);
% energy equation
wdot = netProdRates(gas);
rho = density(gas);
Tdot = - temperature(gas) * gasconstant * enthalpies_RT(gas)' ...
       * wdot / (rho*cp_mass(gas));

% set up column vector for dydt
dydt = [ Tdot
	 zeros(nsp,1) ];

% species equations
for i = 1:nsp
  dydt(i+1) = mw(i)*wdot(i)/rho;
end
