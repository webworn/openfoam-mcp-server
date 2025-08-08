function [gas3] = PostReflectedShock_eq(u2, gas2,gas3)
% Calculates equilibrium post-reflected-shock state for a specified shock velocity.
% 
% FUNCTION SYNTAX:
%     [gas3] = PostReflectedShock_fr(u2,gas2,gas3)
% 
% INPUT:
%     u2 = current post-incident-shock lab frame particle speed
%     gas2 = gas object at post-incident-shock state (already computed)
%     gas3 = working gas object
% 
% OUTPUT:
%     gas3 = gas object at equilibrium post-reflected-shock state

format long;

%INITIALIZE ERROR VALUES
run('SDTconfig.m'); % loads ERRFT and EERFV from global configuration file

%CALCULATE POST-REFLECTED SHOCK STATE
P2 = pressure(gas2);
H2 = enthalpy_mass(gas2);
T2 = temperature(gas2);
r2 = density(gas2);
V2 = 1/r2;

j = 0;
deltaT = 1000;
deltaV = 1000;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%PRELIMINARY GUESS
P = pressure(gas3);
H = enthalpy_mass(gas3);
T = temperature(gas3);
r = density(gas3);
V = 1/r;
[P, H] = eq_state(gas3,r,T);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%START LOOP

flag = 0;

while((abs(deltaT) > ERRFT*T) | (abs(deltaV) > ERRFV*V))
    
    j = j + 1;
    
    if(j == 500)
        disp(['shk_calc did not converge for U = ',num2str(U2)])
        flag = 1;
        return
    end       
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %CALCULATE FH & FP FOR GUESS 1
    [FH,FP] = FHFP_reflected_fr(u2,gas3,gas2);
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %TEMPERATURE PERTURBATION
    DT = T*0.02;
    Tper = T + DT;
    Vper = V;
    Rper = 1/Vper;
    
    [Pper, Hper] = eq_state(gas3,Rper,Tper);
    
    %CALCULATE FHX & FPX FOR 'IO' STATE
    [FHX,FPX] = FHFP_reflected_fr(u2,gas3,gas2);

    %ELEMENTS OF JACOBIAN
    DFHDT = (FHX-FH)/DT;
    DFPDT = (FPX-FP)/DT;
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %VOLUME PERTURBATION
    DV = 0.02*V;
    Vper = V + DV;
    Tper = T;
    Rper = 1/Vper;
    
    [Pper, Hper] = eq_state(gas3,Rper,Tper);
    
    %CALCULATE FHX & FPX FOR 'IO' STATE
    [FHX,FPX] = FHFP_reflected_fr(u2,gas3,gas2);
   
    %ELEMENTS OF JACOBIAN
    DFHDV = (FHX-FH)/DV;
    DFPDV = (FPX-FP)/DV;
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %USE MATLAB MATRIX INVERTER
    J = [DFHDT DFHDV; DFPDT DFPDV];
    a = [-FH; -FP];
    b = J\a;
    deltaT = b(1);
    deltaV = b(2);
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %CHECK & LIMIT CHANGE VALUES
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %TEMPERATURE
    %VOLUME
    DTM = 0.2*T;
    if (abs(deltaT) > DTM)
        deltaT = DTM*deltaT/abs(deltaT);
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %VOLUME
    V3X = V + deltaV;
    if (V3X > V2)
        DVM = 0.5*(V2 - V);
    else
        DVM = 0.2*V;
    end
    if (abs(deltaV) > DVM)
        deltaV = DVM*deltaV/abs(deltaV);
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %MAKE THE CHANGES
    T = T + deltaT;
    V = V + deltaV;
    r = 1/V;
    [P, H] = eq_state(gas3,r,T);

end

T3 = T;
V3 = V;
P3 = P;
H3 = H;
r3 = r;
