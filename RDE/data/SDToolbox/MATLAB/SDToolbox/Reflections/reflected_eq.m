function [p3,UR,gas3] = reflected_eq(gas1,gas2,gas3,UI)
% Calculates equilibrium post-reflected-shock state assumming u1 = 0.
% 
% FUNCTION SYNTAX:
%     [p3,UR,gas3] = reflected_eq(gas1,gas2,gas3,UI)
% 
% INPUT:
%     gas1 = gas object at initial state
%     gas2 = gas object at post-incident-shock state (already computed)
%     gas3 = working gas object
%     UI = incident shock speed (m/s)
% 
% OUTPUT:
%     p3 = post-reflected-shock pressure (Pa)
%     UR = reflected shock speed (m/s)
%     gas3 = gas object at equilibrium post-reflected-shock state

% Lower bound on volume/density ratio (globally defined)
run('SDTconfig.m'); % loads volumeBoundRatio from global configuration file

p2 = pressure(gas2);
p1 = pressure(gas1);
rho2 = density(gas2);
v2=1/rho2;
rho1 = density(gas1);
v1=1/rho1;
gamma2 = cp_mole(gas2)/cv_mole(gas2);
T1 = temperature(gas1);
T2 = temperature(gas2);

u2 = sqrt((p2-p1)*(v1-v2)); %particle velocity
w2 = UI - u2; % velocity in shock fixed frame

% % Approximate constant gamma guesses
% p3 = p2* (5*gamma2+1+sqrt(17*gamma2^2 + 3*gamma2 + 1)  )/4/gamma2;
% rho3 = (p3-p2)/((p3-p2)/rho2-u2^2);
% v3=1/rho3;
% T3 = T1*p3*(1/rho3)/(p1*(1/rho1));

%BASIC PRELIMINARY GUESS
v3 = v2/volumeBoundRatio;
p3 = p2 + rho2*(UI^2)*(1-v3/v2);
T3 = T2*p3*v3/(p2*v2);

set(gas3, 'T', T3, 'P', p3, 'X', moleFractions(gas2)); 
[gas3] = PostReflectedShock_eq(u2, gas2,gas3);
p3 = pressure(gas3);
UR = (p3-p2)/u2/rho2-u2;
