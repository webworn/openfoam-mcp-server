function dydt = cvsys_v(t,y,gas,mw,rho_0,reac,tau,V_min,t_min)
% Evaluates the system of ordinary differential equations for an adiabatic, 
%     constant-volume, zero-dimensional reactor. 
%     It assumes that the 'gas' object represents a reacting ideal gas mixture.
% 
%     INPUT:
%         t = time
%         y = solution array [temperature, species mass 1, 2, ...]
%         gas = working gas object
%     
%     OUTPUT:
%         An array containing time derivatives of:
%             temperature and species mass fractions, 
%         formatted in a way that the integrator in cvsolve can recognize.
%
% normalized volume and volume derivative supplied by f_volume function
[V, dVdt] = f_volume(t,tau,V_min,t_min);
rho = rho_0/V;
% Set the state of the gas, based on the current solution vector and volume
set(gas, 'T', y(1), 'Rho', rho, 'Y', y(2:end));
nsp = nSpecies(gas);
cv = cv_mass(gas);
w = meanMolecularWeight(gas);
work =  -gasconstant*y(1)*dVdt/V/w/cv;
% energy equation
if (reac)
    wdot = netProdRates(gas);
    Tdot = - y(1) * gasconstant * (enthalpies_RT(gas) - ones(nsp,1))' ...
        * wdot / rho /cv;
    Tdot = Tdot + work;
else
    Tdot = work;
end
% set up column vector for dydt
dydt = [ Tdot
    zeros(nsp,1) ];
% species equations
if (reac)
    for i = 1:nsp
        dydt(i+1,1) = mw(i)*wdot(i)/rho;
    end
end
end
    % end
