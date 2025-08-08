function [FH,FP] = FHFP(w1,gas2,gas1)
% Uses the momentum and energy conservation equations to calculate
% error in pressure and enthalpy given shock speed, upstream (gas1)
% and downstream states (gas2).  States are not modified by these routines.
% 
% FUNCTION SYNTAX:
%     [FH,FP] = FHFP(w1,gas2,gas1)
% 
% INPUT:
%     w1 = shock speed (m/s)
%     gas2 = gas object at working/downstream state
%     gas1 = gas object at initial/upstream state
% 
% OUTPUT:
%     FH,FP = error in enthalpy and pressure

    P1 = pressure(gas1);
    H1 = enthalpy_mass(gas1);
    r1 = density(gas1);
    
    P2 = pressure(gas2);
    H2 = enthalpy_mass(gas2);
    r2 = density(gas2);
    
    w1s = w1^2;
    w2s = w1s*(r1/r2)^2;

    FH = H2 + 0.5*w2s - (H1 + 0.5*w1s);
    FP = P2 + r2*w2s - (P1 + r1*w1s);
