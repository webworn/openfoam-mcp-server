function [gas, w1] = CJ_calc(gas, gas1, ERRFT, ERRFV, x)
% Calculates the Chapman-Jouguet wave speed using Reynolds' iterative method.
% 
% FUNCTION SYNTAX:
%     [gas,w1] = CJ_calc(gas,gas1,ERRFT,ERRFV,x)
% 
% INPUT:
%     gas = working gas object
%     gas1 = gas object at initial state
%     ERRFT,ERRFV = error tolerances for iteration
%     x = density ratio
% 
% OUTPUT:
%     gas = gas object at equilibrium state
%     w1 = initial velocity to yield prescribed density ratio
%

T = 2000;
r1 = density(gas1);
V1 = 1/r1;
P1 = pressure(gas1);

j = 0;
deltaT = 1000;
deltaW = 1000;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%PRELIMINARY GUESS
V = V1/x;
r = 1/V;
w1 = 2000;
[P, H] = eq_state(gas,r,T);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%START LOOP
while((abs(deltaT) > ERRFT*T) | (abs(deltaW) > ERRFV*w1))

    j = j + 1;
	if(j == 500)
        'j = 500'
        return
	end       
	
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	%CALCULATE FH & FP FOR GUESS 1
	[FH,FP] = FHFP(w1,gas,gas1);
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	%TEMPERATURE PERTURBATION
	DT = T*0.02;
	Tper = T + DT;
	Vper = V;
	Rper = 1/Vper;
	Wper = w1;
	[Pper, Hper] = eq_state(gas,Rper,Tper);
	
	%CALCULATE FHX & FPX FOR 'IO' STATE
	[FHX,FPX] = FHFP(Wper,gas,gas1);
	%ELEMENTS OF JACOBIAN
	DFHDT = (FHX-FH)/DT;
	DFPDT = (FPX-FP)/DT;
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	%VELOCITY PERTURBATION
	DW = 0.02*w1;
	Wper = w1 + DW;
	Tper = T;
	Rper = 1/V;
	[Pper, Hper] = eq_state(gas,Rper,Tper);
	
	%CALCULATE FHX & FPX FOR 'IO' STATE
    [FHX,FPX] = FHFP(Wper,gas,gas1);
	%ELEMENTS OF JACOBIAN
	DFHDW = (FHX-FH)/DW;
	DFPDW = (FPX-FP)/DW;
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	%USE MATLAB MATRIX INVERTER
	J = [DFHDT DFHDW; DFPDT DFPDW];
	a = [-FH; -FP];
	b = J\a;
	deltaT = b(1);
	deltaW = b(2);
	
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	%CHECK & LIMIT CHANGE VALUES
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	%VOLUME
	DTM = 0.2*T;
	if (abs(deltaT) > DTM)
        deltaT = DTM*deltaT/abs(deltaT);
	end
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	%MAKE THE CHANGES
	T = T + deltaT;
	w1 = w1 + deltaW;
	r;
	[P, H] = eq_state(gas,r,T);
end

