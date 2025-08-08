function [gas] = PostShock_eq(U1, P1, T1, q, mech)
% Calculates equilibrium post-shock state for a specified shock velocity and pre-shock state.
% 
% FUNCTION SYNTAX:
%     [gas] = PostShock_eq(U1,P1,T1,q,mech)
% 
% INPUT:
%     U1 = shock speed (m/s)
%     P1 = initial pressure (Pa)
%     T1 = initial temperature (K)
%     q = reactant species mole fractions in one of Cantera's recognized formats
%     mech = cti file containing mechanism data (e.g. 'gri30.cti')
% 
% OUTPUT:
%     gas = gas object at equilibrium post-shock state

%INITIALIZE ERROR VALUES
run('SDTconfig.m'); % loads ERRFT and EERFV from global configuration file

gas1 = Solution(mech);
gas = Solution(mech); 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%INTIAL CONDITIONS
set(gas, 'T', T1, 'P', P1, 'X', q);    
set(gas1, 'T', T1, 'P', P1, 'X', q); 

format long;

%CALCULATES POST-SHOCK STATE
[gas] = shk_eq_calc(U1, gas, gas1, ERRFT, ERRFV);
