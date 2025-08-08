function [gas] = shk_eq_calc(U1, gas, gas1, ERRFT, ERRFV)
% Calculates equilibrium post-shock state using Reynolds' iterative method.
% 
%     FUNCTION SYNTAX:
%         [gas] = shk_calc(U1,gas,gas1,ERRFT,ERRFV)
%     
%     INPUT:
%         U1 = shock speed (m/s)
%         gas = working gas object
%         gas1 = gas object at initial state
%         ERRFT,ERRFV = error tolerances for iteration
% 
%     OUTPUT:
%         gas = gas object at equilibrium post-shock state

% Lower bound on volume/density ratio (globally defined)
run('SDTconfig.m'); % loads volumeBoundRatio from global configuration file

r1 = density(gas1);
V1 = 1/r1;
P1 = pressure(gas1);
T1 = temperature(gas1);

j = 0;
deltaT = 1000; deltaV = 1000;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%PRELIMINARY GUESS
V = V1/volumeBoundRatio;
r = 1/V;
P = P1 + r1*(U1^2)*(1-V/V1);
T = T1*P*V/(P1*V1);
[P, H] = eq_state(gas,r,T);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%START LOOP
while((abs(deltaT) > ERRFT*T) | (abs(deltaV) > ERRFV*V))
    
    j = j + 1;
    if(j == 500)
        disp(['shk_eq_calc did not converge for U = ',num2str(U1)])
        return
    end       
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %CALCULATE FH & FP FOR GUESS 1
    [FH,FP] = FHFP(U1,gas,gas1);
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %TEMPERATURE PERTURBATION
    DT = T*0.02; 
    Tper = T + DT;
    Vper = V;
    Rper = 1/Vper;
    [Pper, Hper] = eq_state(gas,Rper,Tper);
    
    %CALCULATE FHX & FPX FOR 'IO' STATE
    [FHX,FPX] = FHFP(U1,gas,gas1);
    %ELEMENTS OF JACOBIAN
    DFHDT = (FHX-FH)/DT;
    DFPDT = (FPX-FP)/DT;
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %VOLUME PERTURBATION
    DV = 0.02*V;
    Vper = V + DV;
    Tper = T;
    Rper = 1/Vper;
    [Pper, Hper] = eq_state(gas,Rper,Tper);
    
    %CALCULATE FHX & FPX FOR 'IO' STATE
    [FHX,FPX] = FHFP(U1,gas,gas1);
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
    V2X = V + deltaV;
    if (V2X > V1)
        DVM = 0.5*(V1 - V);
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
    [P, H] = eq_state(gas,r,T);

end

T2 = T;
V2 = V;
P2 = P;
H2 = H;
r2 = r;
