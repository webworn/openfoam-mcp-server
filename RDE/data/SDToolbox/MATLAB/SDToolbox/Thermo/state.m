function [P, H] = state(gas,r1,T1)
% Calculates frozen state given T & rho.
% Used in PostShock module.
% 
% FUNCTION SYNTAX:
%     [P,H] = state(gas,r1,T1)
% 
% INPUT:
%     gas = working gas object (gas object is changed and corresponds to new frozen state)
%     r1,T1 = desired density and temperature
% 
% OUTPUT:
%     P,H = frozen pressure and enthalpy at given temperature and density
    
    setDensity(gas, r1);
    set(gas, 'T', T1);
    P = pressure(gas);
    H = enthalpy_mass(gas);
