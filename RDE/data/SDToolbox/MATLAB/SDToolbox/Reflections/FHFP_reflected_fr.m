function  [FH,FP] = FHFP_reflected_fr(u2,gas3,gas2)
% Uses the momentum and energy conservation equations to calculate error in 
% current pressure and enthalpy guesses. In this case, state 3 is frozen.
% 
% FUNCTION SYNTAX:
%     [FH,FP] = FHFP_reflected_fr(u2,gas3,gas2)
% 
% INPUT:
%     u2 = current post-incident-shock lab frame particle speed
%     gas3 = working gas object
%     gas2 = gas object at post-incident-shock state (already computed)
% 
% OUTPUT:
%     FH,FP = error in enthalpy and pressure
        
P2 = pressure(gas2);
H2 = enthalpy_mass(gas2);
r2 = density(gas2);

P3 = pressure(gas3);
H3 = enthalpy_mass(gas3);
r3 = density(gas3);

FH = H3 -H2- 0.5*u2^2* (r3/r2+1)/(r3/r2-1);
FP = P3 - P2 - r3*u2^2/(r3/r2-1);
