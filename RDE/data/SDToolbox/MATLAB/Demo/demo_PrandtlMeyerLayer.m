% Shock and Detonation Toolbox Demo Program
% 
% Calculates Prandtl-Meyer function and polar originating from lower layer postshock state. 
% Calculates oblique shock wave moving into expanded detonation products or a 
% specified bounding atmosphere.  
% Two-layer version with arbitrary flow in lower layer (1), oblique wave in upper layer (2).  
% Upper and lower layers can have various compositions as set by user.
%  
% ################################################################################
% Theory, numerical methods and applications are described in the following report:
% 
%     Numerical Solution Methods for Shock and Detonation Jump Conditions, S.
%     Browne, J. Ziegler, and J. E. Shepherd, GALCIT Report FM2006.006 - R3,
%     California Institute of Technology Revised September, 2018.
% 
% Please cite this report and the website if you use these routines. 
% 
% Please refer to LICENCE.txt or the above report for copyright and disclaimers.
% 
% http://shepherd.caltech.edu/EDL/PublicResources/sdt/
% 
% ################################################################################ 
% Updated September 2018
% Tested with: 
%     MATLAB 2017b and 2018a, Cantera 2.3 and 2.4
% Under these operating systems:
%     Windows 8.1, Windows 10, Linux (Debian 9)
%%
clear;clc;
disp('demo_PrandtlMeyerLayer') 
 %% 
 %  SET TYPE OF SHOCK COMPUTATION 
 % Frozen:  EQ = false   Equilibrium EQ =  true
 EQ = false;
%%
% set the initial state and compute properties
PA = 100000.; TA = 1000.;  %layer state
q = 'PG:1';
mech = 'PG14.cti';
gas = Solution(mech);
% q = 'O2:1. N2:3.76';  %air
% mech = 'gri30_highT.cti';
% gas = Solution(mech);
set(gas,'Temperature',TA,'Pressure',PA,'MoleFractions',q);
x1 = moleFractions(gas);
P1 = pressure(gas);
T1 = temperature(gas);
rho1 = density(gas);
v1 = 1/rho1;
a1 = soundspeed_fr(gas);  
s1 = entropy_mass(gas);
h1 = enthalpy_mass(gas);
RU = gasconstant;
W = meanMolecularWeight(gas);
R = RU/W;
%%
% Set lower layer velocity using the Mach number
M2 = 1.05;
layer_speed = a1*M2;
U = layer_speed;
mu_min = asin(a1/U);  %Mach angle
% check to see if flow speed is supersonic
if (U < a1)
     exit;
end
% stagnation enthalpy
h0 = h1 + U^2/2;
% estimate maximum  speed
wmax = sqrt(2*h0);
% initialize variables for computing PM properties and plotting
v2 = 50*v1;
vstep = 0.001*(v2-v1);
n = 1;
%%
% Compute expansion conditions over range from minimum to maximum specific volume.  
% User can adjust the increment and endpoint to get a smooth output curve and reliable integral.
% Could also try a logarithmic range for some cases.
 for v = v1:vstep:v2
     rho(n) = 1/v;
     x = moleFractions(gas);   %update mole fractions based on last gas state
     set(gas,'Density',rho(n),'Entropy',s1,'MoleFractions',x);  
     if EQ
         %required  for equilibrium expansion
         equilibrate(gas,'SV');   
         a(n) = soundspeed_eq(gas);  
     else
         %use this for frozen expansion
         a(n) = soundspeed_fr(gas);
     end
     h(n) = enthalpy_mass(gas);
     u(n) = sqrt(2*(h0-h(n)));
     M(n) = u(n)/a(n);
     mu(n) = asin(1/M(n));
     T(n) = temperature(gas);
     P(n) = pressure(gas);
     n = n + 1;
 end
    nmax = n-1;
 % define integrand for PM function and integrate with trapezoidal rule
      int = -1*sqrt(M.^2-1)./M.^2./rho;
      omega(1) = 0.;
 for n=2:nmax
     omega(n) = omega(n-1) +  (int(n-1) + int(n))*(rho(n)-rho(n-1))/2.;
 end
 %%
 % compute shock polar
 % First compute upstream state
 % for RDE model, expand detonation products to ambient pressure
 if EQ
   set(gas,'Entropy',s1,'Pressure',PA,'MoleFractions',x);
   equilibrate(gas,'SP');
   as = soundspeed_eq(gas);
 else
 %%
 % alternative state upstream: reactants  (need to use frozen shock
 % calculation with this)
%    qs = 'C2H4:1.00 O2:3';     %ethylene oxygen
%    qs = 'O2:1.00 N2:3.76';     %combustion air
     qs = ' PG:1';
     % Hans case 1
    Pb = PA/25;        % lower than PA for solutions to exist 
    rhob = rho1/2;
    Tb = Pb/R/rhob;
 % Alternative is to set Tb directly   
    set(gas,'Temperature',Tb,'Pressure',Pb,'MoleFractions',qs);
    as = soundspeed_fr(gas);
 end
 %%
 % states upstream of shock
 rhos  = density(gas);
 Ps = pressure(gas);
 Ts = temperature(gas);
 xs = moleFractions(gas);
 as = soundspeed_fr(gas);
 %% 
 % oblique shock computation
 % Set freestream velocity using Mach number and find limiting speeds
 M2 = 2.4;
US = M2*as;
beta_min = asin(as/US);  %Mach angle
wmin = as + 1.;  %start just above sound speed
% check to see if shock speed is above sound speed
if (US < as)
     exit;
end
% set maximum normal speed
wmax = US;
% initialize variables for plotting
rho2 = 0.;
w1 = 0.;
w2 = 0.;
u2 = 0.;
vt = 0.;
a2 = 0.;
P2 = 0.;
beta = 0.;
theta = 0.;
n = 1;
%%
% compute shock jump conditions over range from minimum to maximum normal
% speeds.  Adjust the increment to get a smooth output curve. 
for w = wmin:5:wmax
if EQ
    % equilibrium state
    [gas] = PostShock_eq(w, Ps, Ts, xs, mech);
    a2(n) = soundspeed_eq(gas);
else
    %%
    %  for non-reactive or cold upstream state, use frozen shock calculation
    [gas] = PostShock_fr(w, Ps, Ts, qs, mech);
    a2(n) = soundspeed_fr(gas);
end
    rho2(n) = density(gas);
    ratio = rhos/rho2(n);
    w1(n) = w;
    w2(n) = w*ratio;
    P2(n) = pressure(gas);
    beta(n) = asin(w/US);
    vt(n) = US*cos(beta(n));
    theta(n) = beta(n) - atan(w2(n)/sqrt(US^2-w^2));
    u2(n) = sqrt(w2(n)^2 + vt(n)^2);
    n = n + 1;
end
% pick up normal shock as last point
    w = US;
if EQ
    % equilibrium state
     [gas] = PostShock_eq(w, Ps, Ts, xs, mech);
     a2(n) = soundspeed_eq(gas);
else
%%
%  for non-reactive or cold upstream state, use frozen shock calculation
    [gas] = PostShock_fr(w, Ps, Ts, qs, mech);  
    a2(n) = soundspeed_fr(gas);
end
    rho2(n) = density(gas);
    ratio = rhos/rho2(n);
    w1(n) = w;
    w2(n) = w*ratio;
    P2(n) = pressure(gas);
    beta(n) = asin(w/US);
    vt(n) = US*cos(beta(n));
    theta(n) = beta(n) - atan(w2(n)/sqrt(US^2-w^2));
    u2(n) = sqrt(w2(n)^2 + vt(n)^2);

%%
% plot pressure-deflection polar
	figure(1); clf;
    hold on
	plot(180*omega(:)/pi,P(:)/PA,'LineWidth',2);  
  	plot(180*theta(:)/pi,P2(:)/PA,'LineWidth',2);  
%	axis([0 maxtheta 0 maxP]);
	title(['PM polar and shock polar for detonation layer'],'FontSize', 12);
	xlabel('flow deflection (deg)','FontSize', 12);
	ylabel('pressure P/PA','FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);
    hold off
% plot PM function-Mach number
	figure(2); clf;
	plot(M(:),180*omega(:)/pi,'LineWidth',2);  
	title(['PM Function, free-stream speed ',num2str(U,5),' m/s'],'FontSize', 12);
	ylabel('PM function (deg)','FontSize', 12);
	xlabel('Mach number','FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);
