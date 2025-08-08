function [U_cj, curve, goodness, dnew, plot_data] = CJspeed(P1, T1, q, mech)
% Calculates CJ detonation velocity for a given pressure, temperature, and
% composition.
%
% FUNCTION SYNTAX:
%     If only CJ speed required:
%     U_cj = CJspeed(P1,T1,q,mech)
%     If full output required:
%     [U_cj, curve, goodness, dnew, plot_data] = CJspeed(P1,T1,q,mech)
%
% INPUT:
%     P1 = Initial Pressure (Pa)
%     T1 = Initial Temperature (K)
%     q = string of reactant species mole fractions
%     mech = cti file containing mechanism data (i.e. 'gri30.cti')
%
% OUTPUT:
%     cj_speed = CJ detonation speed (m/s)
%     curve = cfit object of LSQ fit
%     goodness = goodness of fit statistics for curve
%     dnew = CJ density ratio
%     plot_data = structure containing additional parameters to use 
%                 for external plotting by Utilities/CJspeed_plot

gas1 = Solution(mech);
gas = Solution(mech); 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%INTIAL CONDITIONS
set(gas, 'T', T1, 'P', P1, 'X', q);    
set(gas1, 'T', T1, 'P', P1, 'X', q); 

format long;

%CHANGE ERROR VALUES IF NECESSARY, DEFAULTS ARE THE SAME AS IN SDTconfig files.
ERRFT = 1.0*10^-4;
ERRFV = 1.0*10^-4;

% i = 1;
T1 = temperature(gas1);
P1 = pressure(gas1);

counter = 1;
numsteps = 20;
max = 1.5;
min = 2.0;
g = 0;

while ((counter <= 4) || (g < 0.99999))
	step = (max-min)/numsteps;
	i = 1;
	for x = min:step:max
        set(gas, 'T', T1, 'P', P1, 'X', q);    
        [gas, w1(i)] = CJ_calc(gas, gas1, ERRFT, ERRFV, x);
        rr(i) = density(gas)/density(gas1);
        i = i + 1;
	end   

	F = fitoptions('method','NonlinearLeastSquares','StartPoint',[1,1,1]);
	ftpe = fittype('a*x^2+b*x+c','coeff',{'a','b','c'});
	[curve,goodness,~]=fit(transpose(rr),transpose(w1),ftpe,F);
    
    a = curve.a;
    b = curve.b;
    
    g = goodness.rsquare;
	dnew = -b/(2*a);
	min = dnew - dnew*0.001;
	max = dnew + dnew*0.001;

    counter = counter + 1;
end

[h,cj_speed] = predint(curve,dnew,0.95,'obs','on');
upbound = h(2)-cj_speed;

U_cj = cj_speed + .001;  %Make sure Hugoniot and Rayleigh intersect

% Collect extra outputs into a structure that can be used to make plots 
% (via the function 'util_CJspeed_plot', included alongside the demo scripts)

plot_data = struct;
plot_data.rr = rr;
plot_data.w1 = w1;
plot_data.cj_speed = cj_speed;
plot_data.upbound = upbound;

