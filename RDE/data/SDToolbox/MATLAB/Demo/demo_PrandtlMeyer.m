% Shock and Detonation Toolbox Demo Program
% 
% Calculates Prandtl-Meyer function and polar. For consistency, if an equilibrium 
% expansion is used, the equilibrium sound speed sound be used in computing the
% Mach number. Likewise, if the expansion is frozen (fixed composition), use the 
% frozen sound speed in computing Mach number.
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
disp('demo_PrandtlMeyer') 
 %% 
 %  SET TYPE OF EXPANSION COMPUTATION 
 % Frozen:  EQ = false   Equilibrium EQ =  true
 EQ = false;
%%
% set the initial state and compute properties
P1 = 100000.; T1 = 300.; 
q = 'O2:0.2095 N2:0.7808 CO2:0.0004 Ar:0.0093';    
mech = 'airNASA9ions.cti';   
gas = Solution(mech);
set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
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
U = a1*(1.0001);
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
     x = moleFractions(gas);   % update mole fractions based on last gas state
     set(gas,'Density',rho(n),'Entropy',s1,'MoleFractions',x);
     if EQ
         % required  for equilibrium expansion
         equilibrate(gas,'SV');   
         a(n) = soundspeed_eq(gas);  
     else
         % use this for frozen expansion
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
% plot pressure-deflection polar
	figure(1); clf;
	plot(180*omega(:)/pi,P(:)/P1,'LineWidth',2);  
%	axis([0 maxtheta 0 maxP]);
	title(['PM polar, free-stream speed ',num2str(U,5),' m/s'],'FontSize', 12);
	xlabel('flow deflection (deg)','FontSize', 12);
	ylabel('pressure P/P1','FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);
% plot PM function-Mach number
	figure(2); clf;
	plot(M(:),180*omega(:)/pi,'LineWidth',2);  
	title(['PM Function, free-stream speed ',num2str(U,5),' m/s'],'FontSize', 12);
	ylabel('PM function (deg)','FontSize', 12);
	xlabel('Mach number','FontSize', 12);
	set(gca,'FontSize',12,'LineWidth',2);
