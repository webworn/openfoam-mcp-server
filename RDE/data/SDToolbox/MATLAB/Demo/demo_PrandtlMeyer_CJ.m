% Shock and Detonation Toolbox Demo Program
% 
% Calculates Prandtl-Meyer function and polar expanded from CJ state.  
% For consistency, if an equilibrium expansion is used, the equilibrium sound speed sound 
% should be used  in computing the Mach number. Likewise, if the expansion is frozen 
% (fixed composition), use the frozen sound speed in computing Mach number.
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
disp('demo_PrandtlMeyer_CJ') 
 %% 
 %  SET TYPE OF EXPANSION COMPUTATION 
 % Frozen:  EQ = false   Equilibrium: EQ =  true
 EQ = true;
%%
% set the initial state and compute properties
P1 = 100000.; T1 = 300.; 
q = 'O2:1 N2:3.76 H2:2';    
mech = 'Mevel2017.cti'; 
gas = Solution(mech);
%% Find CJ speed
[cj_speed] = CJspeed(P1, T1, q, mech);

%% Evaluate gas state
[gas] = PostShock_eq(cj_speed,P1, T1, q, mech);

if EQ
    % use this for equilibrium expansion
    equilibrate(gas,'TP');   
    a1 = soundspeed_eq(gas);   
else
    % use this for frozen expansion
    a1 = soundspeed_fr(gas);  
end
x1 = moleFractions(gas);
rho1 = density(gas);
v1 = 1/rho1;
s1 = entropy_mass(gas);
h1 = enthalpy_mass(gas);
%%
% Set freestream velocity .01% above sound speed
U1 = a1*(1.0001);
mu_min = asin(a1/U1);  %Mach angle
% check to see if flow speed is supersonic
if (U1 < a1)
     exit;
end
% stagnation enthalpy
h0 = h1 + U1^2/2;
% estimate maximum  speed
wmax = sqrt(2*h0);
% initialize variables for computing PM properties and plotting
rho = 0.;
w = 0.;
a = 0.;
P = 0.;
T = 0.;
h = 0.;
mu = 0.;
M = 0.;
omega = 0.;
v2 = 200*v1;
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
     r(n) = density(gas);
     streamwidth(n) = rho1*U1/u(n)/r(n);
     n = n + 1;
 end
    nmax = n-1;
 % define integrand for PM function and integrate with trapezoidal rule
 % compute angle alpha of straight characteristics - measure clockwise
 % from horizontal direction.   
      int = -1*sqrt(M.^2-1)./M.^2./rho;
      omega(1) = 0.;
      alpha(1) = -1.*asin(1/M(1));
      time(1) = 0.;
      X(1) = 0.;
      Y(1) = -1.E-2;
      R(1) = sqrt(X(1)^2+Y(1)^2);
 for n=2:nmax
     omega(n) = omega(n-1) +  (int(n-1) + int(n))*(rho(n)-rho(n-1))/2.;
     alpha(n) = (omega(n)-mu(n)); 
     U = 0.5*(u(n) + u(n-1));
     deltaalpha = abs(alpha(n) - alpha(n-1));
     theta = 0.5*(omega(n) + omega(n-1));
     Alpha = 0.5*(alpha(n) + alpha(n-1));
     deltat = R(n-1)*deltaalpha/(U*sin(theta-Alpha));
     Ux = U*cos(theta);
     Uy = U*sin(theta);
     time(n) = time(n-1) + deltat;
     X(n) = X(n-1) + Ux*deltat;
     Y(n) = Y(n-1) + Uy*deltat;
     R(n) = sqrt(X(n)^2 + Y(n)^2);
 end
% plot pressure-deflection polar
	figure(1); clf;
	plot(180*omega(:)/pi,P(:)/P1,'LineWidth',2);  
%	axis([0 maxtheta 0 maxP]);
	title(['PM polar, post- CJ state, wave frame'],'FontSize', 12);
	xlabel('flow deflection (deg)','FontSize', 12);
	ylabel('pressure P/P1','FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);
% plot PM function-Mach number
	figure(2); clf;
	plot(M(:),180*omega(:)/pi,'LineWidth',2);  
	title(['PM Function, post- CJ state, wave frame'],'FontSize', 12);
	ylabel('PM function (deg)','FontSize', 12);
	xlabel('Mach number','FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);
% plot alpha-Mach number
	figure(3); clf;
	plot(M(:),180*alpha(:)/pi,'LineWidth',2);  
	title(['alpha, post- CJ state, wave frame'],'FontSize', 12);
	ylabel('alpha (deg)','FontSize', 12);
	xlabel('Mach number','FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);
 % plot normalized stream width vs Mach number
	figure(4); clf;
	plot(M(:),streamwidth(:),'LineWidth',2);  
	title(['stream tube width post- CJ state, wave frame'],'FontSize', 12);
	ylabel('width','FontSize', 12);
	xlabel('Mach number','FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);
 % plot particle path
	figure(5); clf;
	plot(X(:),Y(:),'LineWidth',2);  
	title(['X-Y in wave frame'],'FontSize', 12);
	ylabel('Y','FontSize', 12);
	xlabel('X','FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);
% plot particle path time
	figure(6); clf;
	plot(M(:),time(:),'LineWidth',2);  
	title(['time vs M in wave frame'],'FontSize', 12);
	ylabel('time','FontSize', 12);
	xlabel('M','FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);
% plot particle path in alpha-radius coordinates
	figure(7); clf;
	polar(alpha(:),R(:));  
	title(['radius vs alpha in wave frame'],'FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);
