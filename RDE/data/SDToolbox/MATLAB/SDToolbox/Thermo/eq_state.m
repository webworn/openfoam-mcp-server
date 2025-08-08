function [P, H] = eq_state(gas,r1,T1)
% Calculates equilibrium state given T & rho.
% Used in PostShock module.
% 
% FUNCTION SYNTAX:
%     [P,H] = eq_state(gas,r1,T1)
% 
% INPUT:
%     gas = working gas object (gas object is changed and corresponds to new equilibrium state)
%     r1,T1 = desired density and temperature
% 
% OUTPUT
%     P,H = equilibrium pressure and enthalpy at given temperature and density

    set(gas, 'Density', r1, 'T', T1);
    
    equilibrate(gas, 'TV');
    P = pressure(gas);
    H = enthalpy_mass(gas);
