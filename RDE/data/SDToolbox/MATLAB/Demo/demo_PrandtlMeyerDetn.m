% Shock and Detonation Toolbox Demo Program
% 
% Calculates Prandtl-Meyer function and polar originating from CJ state. 
% Calculates oblique shock wave moving into expanded detonation products or a 
% specified bounding atmosphere.
% Evaluates properties along expansion isentrope and computed performance
% of rotating detonation engine based on ideal axial flow model.  Creates
% plots of shock-expansion polar interaction and properties on the
% expansion isentrope.
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
clear;
clc;
close all;
disp('demo_PrandtlMeyerDetn')
 %% 
 %  SET TYPE OF SHOCK COMPUTATION 
 % Frozen:  EQ = false   Equilibrium EQ =  true
 %EQ = true;
 EQ = false;
%%
% set the initial state and compute properties
P1 = 100000.; T1 = 300.;  %nominal initial state
%P1 = 150000.; T1 = 255.;  %Schwer and Kailasanath initial state
%P1 = 240000.; T1 = 300.;  %kasahara initial state
%P1 = 200000.; T1 = 300.;  %Fujiwara initial state
%q = 'C2H4:1.0 O2:3';     %ethylene oxygen, Kasahara
%q = 'H2:2.0 O2:1.0 AR:7.0'; %hydrogen oxygen argon
%q = 'H2:2.0 O2:1.0';  %hydrogen oxygen
%q = 'H2:2.0 O2:1.0 N2:3.76';  %hydrogen air
%q = 'C2H4:1.0 O2:3 N2:11.28';     %ethylene air
q  = 'CH4:1.0 O2:2.0 N2:7.52';  %methane-air
%q = 'C3H8:1.0 O2:5';  %propane oxygen
mech = 'gri30_highT.cti';
gas = Solution(mech);
set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
h1 = enthalpy_mass(gas);
s1 = entropy_mass(gas);
rho1 = density(gas);
a1_fr = soundspeed_fr(gas);
w1 = meanMolecularWeight(gas);
R1 = gasconstant/w1;
gamma1_fr =  a1_fr*a1_fr*rho1/P1;
disp(['Layer detonation computation for ',mech,' with composition ',q])
disp(['State 1 - Initial state of reacting layer'])
disp(['   Pressure ',num2str(P1),' (Pa)']);
disp(['   Temperature ',num2str(T1),' (K)']);
disp(['   Density ',num2str(rho1),' (kg/m3)']);
disp(['   Sound speed (frozen) ',num2str(a1_fr),' (m/s)']);
disp(['   Enthalpy ',num2str(h1),' (J/kg)']);
disp(['   Entropy ',num2str(s1),' (J/kg K)']);
disp(['   gamma (frozen) ',num2str(gamma1_fr),' ']);
%%
% Find CJ speed
[U_CJ] = CJspeed(P1, T1, q, mech);
% Evaluate CJ gas state
[gas] = PostShock_eq(U_CJ,P1, T1, q, mech);
x2 = moleFractions(gas);
P2 = pressure(gas);
T2 = temperature(gas);
rho2 = density(gas);
a2_eq = soundspeed_eq(gas);   
s2 = entropy_mass(gas);
h2 = enthalpy_mass(gas);
w2 = rho1*U_CJ/rho2;
u2 = U_CJ-w2;
gamma2_eq =  a2_eq*a2_eq*rho2/P2;
disp('State 2 - CJ ');
disp(['   CJ speed ',num2str(U_CJ),' (m/s)']);
disp(['   Pressure ',num2str(P2),' (Pa)']);
disp(['   Temperature ',num2str(T2),' (K)']);
disp(['   Density ',num2str(rho2),' (kg/m3)']);
disp(['   Enthalpy ',num2str(h2),' (J/kg)']);
disp(['   Entropy ',num2str(s2),' (J/kg K)']);
disp(['   w2 (wave frame) ',num2str(w2),' (m/s)']);
disp(['   u2 (lab frame) ',num2str(u2),' (m/s)']);
disp(['   a2 (equilibrium) ',num2str(a2_eq),' (m/s)']);
disp(['   gamma2 (equilibrium) ',num2str(gamma2_eq),' ']);
%%
% Set triple point speed and downstream conditions acounting for the specified wave angle.   Need to
% be careful about the geometrical considerations and branches of the
% trigonometric functions. 
% This computation is only reliable for detonations that have ang = 90 and
% no upstream layer speed.  
%  
ang = 90.;  % wave angle in degrees measured relative to freestream direction in counterclockwise (positive) direction
angle_det = pi*ang/180.;  %detonation wave angle (0< angle_det < Pi), angles greater than pi/2 deflect flow downward
% compute triple-point (upstream) speed, this assumes that speed of
% upstream flow is zero in lab frame
u_tp = U_CJ/sin(angle_det);
if (u_tp > U_CJ*(1.001)) 
    %compute tangential (to detonation wave) velocity
    vt_det = sqrt(u_tp^2 - U_CJ^2);
    if (angle_det < pi/2)
         theta_det = angle_det - atan(w2/vt_det);
    else
        theta_det = -1.*(angle_det - atan(w2/vt_det));
    end
    % compute flow deflection downstream of shock
    % downstream speed
    U2 = w2/cos(angle_det-theta_det);
else
    vt_det = 0.;
    U2 = w2*(1.0001);  %start just above M=1 to avoid singularities
    theta_det = 0.;
end
%%
disp('Detonation Wave');
disp(['   Wave angle ',num2str(180.*angle_det/pi),' (deg)']);
disp(['   Downstream flow angle ',num2str(180.*theta_det/pi),' (deg)']);
disp(['   Triple point speed ',num2str(u_tp),' (m/s)']);
disp(['   Downsteam speed in wave frame (x) ',num2str(U2),' (m/s)']);
disp(['   Speed tangential to detonation (z) ',num2str(vt_det),' (m/s)']);
% stagnation enthalpy downstream of the detonation
h0 = h2 + U2^2/2;
% estimate maximum  speed
wmax = sqrt(2*h0);
% initialize variables for computing PM properties and plotting
rho = 0.;
a = 0.;
P = 0.;
T = 0.;
h = 0.;
mu = 0.;
M = 0.;
Me = 0.;
omega = 0.;
v1 = 1/rho2;  %lower limit to specific volume in PM fan
v2 = 50*v1;   %upper limit to specific volume in PM fan
vstep = 0.001*(v2-v1);
n = 1;
%%
% Compute expansion conditions over range from minimum to maximum specific volume.  
% User can adjust the increment and endpoint to get a smooth output curve and reliable integral.
% Could also try a logarithmic range for some cases.
 for v = v1:vstep:v2
     rho(n) = 1/v;
     x = moleFractions(gas);   %update mole fractions based on last gas state
     set(gas,'Density',rho(n),'Entropy',s2,'MoleFractions',x);  
     equilibrate(gas,'SV');   %required  for equilibrium expansion
     h(n) = enthalpy_mass(gas);
     u(n) = sqrt(2*(h0-h(n)));  %flow speed within expansion
     ue(n) = real(sqrt(2*(h1-h(n)))); %ideal axial velocity for RDE model
     a(n) = soundspeed_eq(gas);  %use this for equilibrium expansion, almost always the case for detonation products
%     a(n) = soundspeed_fr(gas);  %use this for frozen expansion   
     Me(n) = ue(n)/a(n);
     M(n) = u(n)/a(n);
     mu(n) = asin(1/M(n));
     T(n) = temperature(gas);
     P(n) = pressure(gas);
     n = n + 1;
 end
    nmax = n-1;
 % define integrand for PM function and integrate with trapezoidal rule
      int = -1*sqrt(M.^2-1)./M.^2./rho;
      omega(1) = theta_det;
 for n=2:nmax
     omega(n) = omega(n-1) +  (int(n-1) + int(n))*(rho(n)-rho(n-1))/2.;
 end
 %%
 % compute shock polar
 % First compute upstream state
 % for RDE model, expand detonation products to ambient pressure
% Set freesteam velocity to expansion speed.  Accounts
% for shear across upstream contact surface 
if EQ
   set(gas,'Entropy',s2,'Pressure',P1,'MoleFractions',x);
   equilibrate(gas,'SP');
   a4 = soundspeed_eq(gas);
   U4 = interp1(P/P1,u,1,'pchip');  % interpolate PM expansion results to ambient pressure
 else
%%
% alternative state upstream: reactants  (need to use frozen shock
% calculation with this)
%    qs = 'C2H4:1.00 O2:3';     %ethylene oxygen
    qs = 'O2:1.00 N2:3.76';     %combustion air
    Ts  = 300.;
    set(gas,'Temperature',Ts,'Pressure',P1,'MoleFractions',qs);
    a4 = soundspeed_fr(gas);
    U4 = u_tp;  %no shear between bounding gas and reacting layer
 end
 %%
 % states upstream of shock
 rho4  = density(gas);
 P4 = pressure(gas);
 T4 = temperature(gas);
 s4 = entropy_mass(gas);
 x4 = moleFractions(gas);
 h4 = enthalpy_mass(gas);
 gamma4 = a4*a4*rho4/P4;
 beta_min = asin(a4/U4);  %Mach angle
disp('State 4 - upstream of oblique shock');
disp(['   Flow speed (wave frame) ',num2str(U4),' (m/s)']);
disp(['   Flow speed (lab frame) ',num2str(U4-u_tp),' (m/s)']);
disp(['   Pressure ',num2str(P4),' (Pa)']);
disp(['   Temperature ',num2str(T4),' (K)']);
disp(['   Density ',num2str(rho4),' (kg/m3)']);
disp(['   Sound speed (frozen) ',num2str(a4),' (m/s)']);
disp(['   Enthalpy ',num2str(h4),' (J/kg)']);
disp(['   Entropy ',num2str(s4),' (J/kg K)']);
disp(['   gamma  ',num2str(gamma4),' ']);
disp(['   Mach angle  ',num2str(beta_min*180./pi),' (deg)']);
%% 
% oblique shock computation, state 4 is upstream and state 5 downstream
% Choices 
wmin = a4 + 1.;  %start just above sound speed
% check to see if shock speed is above sound speed
if (U4 < a4)
     exit;
end
% set maximum normal speed
wmax = U4;
% initialize variables for plotting
rho5 = 0.;
w4 = 0.;
w5 = 0.;
u5 = 0.;
vt4 = 0.;
a5 = 0.;
P5 = 0.;
h5 = 0.;
s5 = 0.;
beta = 0.;
theta = 0.;
n = 1;
%%
% compute shock jump conditions over range from minimum to maximum normal
% speeds.  Adjust the increment to get a smooth output curve. 
for w = wmin:5:wmax
if EQ
    % equilibrium state
    [gas] = PostShock_eq(w, P4, T4, x4, mech);
    a5(n) = soundspeed_eq(gas);
else
    %%
    %  for non-reactive or cold upstream state, use frozen shock calculation
    [gas] = PostShock_fr(w, P4, T4, qs, mech);
    a5(n) = soundspeed_fr(gas);
end
    rho5(n) = density(gas);
    ratio = rho4/rho5(n);
    w4(n) = w;
    w5(n) = w*ratio;
    P5(n) = pressure(gas);
    T5(n) = temperature(gas);
    h5(n) = enthalpy_mass(gas);
    s5(n) = entropy_mass(gas);
    beta(n) = asin(w/U4);
    vt4(n) = U4*cos(beta(n));
    theta(n) = beta(n) - atan(w5(n)/sqrt(U4^2-w^2));
    u5(n) = sqrt(w5(n)^2 + vt4(n)^2);
    n = n + 1;
end

% pick up normal shock as last point
    w = U4;
if EQ
    % equilibrium state
     [gas] = PostShock_eq(w, P4, T4, x4, mech);
     a5(n) = soundspeed_eq(gas);
else
%%
%  for non-reactive or cold upstream state, use frozen shock calculation
    [gas] = PostShock_fr(w, P4, T4, qs, mech);  
    a5(n) = soundspeed_fr(gas);
end
    rho5(n) = density(gas);
    ratio = rho4/rho5(n);
    w4(n) = w;
    w5(n) = w*ratio;
    P5(n) = pressure(gas);
    T5(n) = temperature(gas);
    h5(n) = enthalpy_mass(gas);
    s5(n) = entropy_mass(gas);
    beta(n) = asin(w/U4);
    vt4(n) = U4*cos(beta(n));
    theta(n) = beta(n) - atan(w5(n)/sqrt(U4^2-w^2));
    u5(n) = sqrt(w5(n)^2 + vt4(n)^2);

%%
% plot pressure-deflection polar
	figure(1); clf;
    hold on
	plot(180*omega(:)/pi,P(:)/P1,'LineWidth',2);  
  	plot(180*theta(:)/pi,P5(:)/P1,'LineWidth',2);  
%	axis([0 maxtheta 0 maxP]);
	title(['PM polar and shock polar for detonation layer'],'FontSize', 12);
	xlabel('flow deflection (deg)','FontSize', 12);
	ylabel('pressure P/P1','FontSize', 12);
    legend('PM polar','Shock polar','Location','NorthEast')
	set(gca,'FontSize',12,'LineWidth',2);
    hold off
% plot PM function-Mach number
	figure(2); clf;
	plot(M(:),180*omega(:)/pi,'LineWidth',2);  
	title(['PM Function expanding from CJ state '],'FontSize', 12);
	ylabel('PM function (deg)','FontSize', 12);
	xlabel('Mach number','FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);
% plot PM function velocity-pressure
	figure(3); clf;
    hold on
	plot(P(:)/P1,u(:),'LineWidth',2);  
    plot(P(:)/P1,ue(:),'LineWidth',2); 
    hold off
	title(['PM Function vs Axial expansion from CJ state'],'FontSize', 12);
	ylabel('speed (m/s)','FontSize', 12);
	xlabel('Pressure','FontSize', 12);
    legend('PM polar','Axial expansion','Location','NorthEast')
	set(gca,'FontSize',12,'LineWidth',2);
    %%
%  Input limits for finding intersection of polars - use plots to obtain
%  guesses
pmin = 3.;
pmax = 8.;
% define intersection function
fun = @(x) (interp1(P5/P1,theta,x,'pchip')-interp1(P/P1,omega,x,'pchip'))^2;  
% solve for intersection 
Pshock = fminbnd(@(x) fun(x), pmin, pmax);
%%
% compute postshock conditions - state 5
thetashock = 180*interp1(P5/P1,theta,Pshock,'pchip')/pi;
betashock =  180*interp1(P5/P1,beta,Pshock,'pchip')/pi; 
ushock = interp1(P5/P1,u5,Pshock,'pchip');
rhoshock = interp1(P5/P1,rho5,Pshock,'pchip');
hshock = interp1(P5/P1,h5,Pshock,'pchip');
sshock = interp1(P5/P1,s5,Pshock,'pchip');
ashock = interp1(P5/P1,a5,Pshock,'pchip');
M5 = ushock/ashock;
Tshock = interp1(P5/P1,T5,Pshock,'pchip');
gamma5 = ashock*ashock*rhoshock/(Pshock*P1);
% find shock normal speed and Mach number
wshock = interp1(P5/P1,w4,Pshock,'pchip');
Mshock = wshock/a4;
disp('State 5 - downstream of oblique shock');
disp(['   Flow speed (wave frame) ',num2str(ushock),' (m/s)']);
disp(['   Pressure ',num2str(Pshock*P1),' (Pa)']);
disp(['   Temperature ',num2str(Tshock),' (K)']);
disp(['   Density ',num2str(rhoshock),' (kg/m3)']);
disp(['   Enthalpy ',num2str(hshock),' (J/kg)']);
disp(['   Entropy ',num2str(sshock),' (J/kg K)']);
disp(['   Sound speed ',num2str(ashock),' (m/s)']);
disp(['   Gamma ',num2str(gamma5),' ']);
disp(['   Mach number ',num2str(M5),' ']);
disp(['   Wave angle  ',num2str(betashock),' (deg)']);
disp(['   Flow angle  ',num2str(thetashock),' (deg)']);
disp(['   Upstream of wave - equivalent normal shock speed ',num2str(wshock),' (m/s)']);
disp(['   Upstream of wave - equivalent normal shock Mach number ',num2str(Mshock),' ']);
%%
% compute postexpansion conditions - state 3
u3 = interp1(P/P1,u,Pshock,'pchip');
a3 = interp1(P/P1,a,Pshock,'pchip');
h3 = interp1(P/P1,h,Pshock,'pchip');
rho3 = interp1(P/P1,rho,Pshock,'pchip');
omega3 = 180.*interp1(P/P1,omega,Pshock,'pchip')/pi;
M3 = u3/a3;
mu3 = 180*interp1(P/P1,mu,Pshock,'pchip')/pi;
T3 = interp1(P/P1,T,Pshock,'pchip');
gamma3 = a3*a3*rho3/(P1*Pshock);
disp('State 3 - downstream of expansion');
disp(['   Flow speed (wave frame) ',num2str(u3),' (m/s)']);
disp(['   Pressure ',num2str(Pshock*P1),' (Pa)']);
disp(['   Temperature ',num2str(T3),' (K)']);
disp(['   Density ',num2str(rho3),' (kg/m3)']);
disp(['   Enthalpy ',num2str(h3),' (J/kg)']);
disp(['   Sound speed ',num2str(a3),' (m/s)']);
disp(['   Gamma ',num2str(gamma3),' ']);
disp(['   Mach number ',num2str(M3),' ']);
disp(['   Characteristic angle  ',num2str(mu3),' (deg)']);
disp(['   PM deflection angle  ',num2str(omega3),' (deg)']);
%%
% ideal perfomance velocity considering pure axial flow and isentropic 
% expansion to ambient pressure
ueideal = interp1(P/P1,ue,1,'pchip');
rhoideal = interp1(P/P1,rho,1,'pchip');
aideal = interp1(P/P1,a,1,'pchip');
hideal = interp1(P/P1,h,1,'pchip');
disp('Axial Model- Expansion to P1');
disp(['   Axial speed ',num2str(ueideal),' (m/s)']);
disp(['   Density ',num2str(rhoideal),' (kg/m3)']);
disp(['   Enthalpy ',num2str(hideal),' (J/kg)']);
disp(['   Sound speed ',num2str(aideal),' (m/s)']);
disp(['   Mach number ',num2str(ueideal/aideal),' ']);
% expansion to Mach 1, remove nonzero elements of Me array
first = find(Me,1);
last = length(Me);
uesonic = interp1(Me(first:last),ue(first:last),1,'pchip');
hsonic = interp1(Me(first:last),h(first:last),1,'pchip');
Psonic = interp1(Me(first:last),P(first:last),1,'pchip');
Tsonic = interp1(Me(first:last),T(first:last),1,'pchip');
rhosonic  = interp1(Me(first:last),rho(first:last),1,'pchip');
asonic  = interp1(Me(first:last),a(first:last),1,'pchip');
ToverMdot = (Psonic-oneatm)/(rhosonic*uesonic) + uesonic ;
Pm = interp1(h,P,h1,'pchip');
disp('Axial Model- Expansion to Mach 1');
disp(['   Limiting pressure ',num2str(Pm),' (Pa)']);
disp(['   Axial speed ',num2str(uesonic),' (m/s)']);
disp(['   Pressure ',num2str(Psonic),' (Pa)']);
disp(['   Temperature ',num2str(Tsonic),' (K)']);
disp(['   Density ',num2str(rhosonic),' (kg/m3)']);
disp(['   Enthalpy ',num2str(hsonic),' (J/kg)']);
disp(['   Sound speed ',num2str(asonic),' (m/s)']);
disp(['   Specific thrust ',num2str(ToverMdot),' (m/s)']);
% compute specific thrust for all exit pressures and one atm ambient pressure
for k = first:last
ToM(k) = ue(k) + (P(k)-oneatm)/(rho(k)*ue(k));
end
    % plot specific thrust vs pressure
	figure(5); clf;
    ylim([0 3000]);
    hold on
    plot(P(first:last)/P1,ToM(first:last),'k','LineWidth',2); 
    plot(Psonic/P1,ToverMdot,'Marker','*','MarkerFaceColor','k','MarkerEdgeColor','k','MarkerSize',10);
    text(Psonic/P1,0.9*ToverMdot,'Sonic Axial Flow State','horizontalAlignment','center','FontName','TimesNewRoman'); 
    hold off
	ylabel('specific thrust (m/s)','FontSize', 12,'FontName','TimesNewRoman');
	xlabel('Pressure (P/P1)','FontSize', 12,'FontName','TimesNewRoman');
	set(gca,'FontSize',12,'LineWidth',2,'FontName','TimesNewRoman');
% expansion to a vacuum - using last point in expansion computation
uevac = ue(last:last);
Pvac = P(last:last);
Tvac = T(last:last);
rhovac = rho(last:last);
hvac = h(last:last);
avac = a(last:last);
disp('Axial Model- Expansion to Vacuum');
disp(['   Axial speed ',num2str(uevac),' (m/s)']);
disp(['   Pressure ',num2str(Pvac),' (Pa)']);
disp(['   Temperature ',num2str(Tvac),' (K)']);
disp(['   Density ',num2str(rhovac),' (kg/m3)']);
disp(['   Enthalpy ',num2str(hvac),' (J/kg)']);
disp(['   Sound speed ',num2str(avac),' (m/s)']);
disp(['   Mach number ',num2str(uevac/avac),' ']);
% plot PM function enthalpy-pressure
figure(4); clf;
hold on
plot(P(:)/P1,h(:)/1E6,'LineWidth',2,'Color','k');
plot(1,h1/1E6,'Marker','s','MarkerFaceColor','k','MarkerEdgeColor','none','MarkerSize',10);
text(2.1,h1/1E6+.1E0,'Initial State (1)','horizontalAlignment','center');
plot(P2/P1,h2/1E6,'Marker','d','MarkerFaceColor','k','MarkerEdgeColor','none','MarkerSize',10);
text(P2/P1,h2/1E6+.1E0,'CJ State (2)','horizontalAlignment','center');
plot(Pshock,h3/1E6,'Marker','o','MarkerFaceColor','k','MarkerEdgeColor','none','MarkerSize',10);
text(Pshock+.5,h3/1E6,'Expansion downstream State (3)');
plot(P4/P1,h4/1E6,'Marker','^','MarkerFaceColor','k','MarkerEdgeColor','none','MarkerSize',10);
text(P4/P1+0.5,h4/1E6,'Shock upstream State (4)');
plot(Pshock,hshock/1E6,'Marker','^','MarkerFaceColor','none','MarkerEdgeColor','k','MarkerSize',10);
text(Pshock,hshock/1E6+0.2,'Shock downstream State (5)','horizontalAlignment','center');
plot(Psonic/P1,hsonic/1E6,'Marker','*','MarkerFaceColor','k','MarkerEdgeColor','k','MarkerSize',10);
text(Psonic/P1+0.5,hsonic/1E6,'Sonic Axial Flow State');   
hold off
title(['PM Function expanding from CJ state'],'FontSize', 12);
ylabel('enthalpy (MJ/kg)','FontSize', 12);
xlabel('Pressure (P/P1)','FontSize', 12);
set(gca,'FontSize',12,'LineWidth',2);
