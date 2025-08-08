% Shock and Detonation Toolbox Demo Program
% 
% Calculates shock polar using FROZEN post-shock state based the initial gas properties 
% and the shock speed. Plots shock polar using three different sets of coordinates.
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
disp('demo_oblique') 
%%
% set the initial state and compute properties
P1 = 100000; T1 = 300; 
q = 'H2:2 O2:1 N2:3.76';    
mech = 'Mevel2017.cti'; 
gas1 = Solution(mech);
set(gas1,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
rho1 = density(gas1);
a1 = soundspeed_fr(gas1);
%%
% Set freestream velocity and find limiting speeds
U = 1500.;
beta_min = asin(a1/U);  %Mach angle
wmin = a1 + 1.;  %start just above sound speed
% check to see if shock speed is above sound speed
if (U < a1)
     exit;
end
% set maximum normal speed
wmax = U;
% initialize variables for plotting
rho2 = 0.;
w1 = 0.;
w2 = 0.;
u2 = 0.;
v = 0.;
a2 = 0.;
P2 = 0.;
beta = 0.;
theta = 0.;
n = 1;
%%
% compute shock jump conditions over range from minimum to maximum normal
% speeds.  Adjust the increment to get a smooth output curve. 
for w = wmin:5:wmax
    [gas] = PostShock_fr(w, P1, T1, q, mech);
%    [gas] = PostShock_eq(w, P1, T1, q, mech);  %will not converge at low
%    temperature.
    rho2(n) = density(gas);
    a2(n) = soundspeed_fr(gas);
    ratio = rho1/rho2(n);
    w1(n) = w;
    w2(n) = w*ratio;
    P2(n) = pressure(gas);
    beta(n) = asin(w/U);
    v(n) = U*cos(beta(n));
    theta(n) = beta(n) - atan(w2(n)/sqrt(U^2-w^2));
    u2(n) = sqrt(w2(n)^2 + v(n)^2);
    n = n + 1;
end
% pick up normal shock as last point
    w = U;
    [gas] = PostShock_fr(w, P1, T1, q, mech);
    rho2(n) = density(gas);
    a2(n) = soundspeed_fr(gas);
    ratio = rho1/rho2(n);
    w1(n) = w;
    w2(n) = w*ratio;
    P2(n) = pressure(gas);
    beta(n) = asin(w/U);
    v(n) = U*cos(beta(n));
    theta(n) = beta(n) - atan(w2(n)/sqrt(U^2-w^2));
    u2(n) = sqrt(w2(n)^2 + v(n)^2);
    %%  post process output 
    % compute Mach number 
    M2 = u2./a2;
    % find sonic point
    beta_star = interp1(M2,beta,1.0);
    theta_star = interp1(beta,theta,beta_star);
    % find maximum deflection estimate
    theta_max_guess = max(theta);
    beta_max_guess = interp1(theta,beta,theta_max_guess);
    fun = @(x)interp1(beta,theta,x,'pchip');
    beta_max = fminbnd(@(x)-fun(x), .9*beta_max_guess, 1.1*beta_max_guess);
    theta_max = interp1(beta, theta, beta_max, 'pchip');
    % compute velocity components downstream in lab frame
    u2x = u2.*cos(theta);
    u2y = u2.*sin(theta);
    %% create plots
% plot pressure-deflection polar
	figure(1); clf;
	plot(180*theta(:)/pi,P2(:)/1e6,'k:','LineWidth',2);  
%	axis([0 maxtheta 0 maxP]);
	title(['Shock Polar Air, free-stream speed ',num2str(U,5),' m/s'],'FontSize', 12);
	xlabel('deflection angle (deg)','FontSize', 12);
	ylabel('pressure (MPa)','FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);
% plot pressure-deflection polar
	figure(2); clf;
	plot(180*theta(:)/pi,180*beta(:)/pi,'k:','LineWidth',2);  
	title(['Shock Polar Air, free-stream speed ',num2str(U,5),' m/s'],'FontSize', 12);
	xlabel('deflection angle (deg)','FontSize', 12);
	ylabel('wave angle (deg)','FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);
% plot velocity polar
	figure(3); clf;
	plot(u2x(:),u2y(:),'k:','LineWidth',2);  
	title(['Shock Polar Air, free-stream speed ',num2str(U,5),' m/s'],'FontSize', 12);
	xlabel('u2x (m/s)','FontSize', 12);
	ylabel('u2y (m/s)','FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);


