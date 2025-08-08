% Shock and Detonation Toolbox Demo Program
% 
% Explosion computation simulating adiabatic compression ignition with
% control volume approach and effective piston used for compression.
% Requires adiasys.m  function for ODE solver.
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
clear; clc; close all;
disp('Transient Compression Ignition')
%%
% compression apparatus dimensions
X0 = 0.377;    %height of gas volume (m)
R0 = 0.032/2;  %diameter of gas volume (m)
U0 = -5.8;      %initial speed of compression piston (m/s)
A  = pi*R0*R0; %cross section area of compression volume (m2)
L = 0.40 ;     %estimated length of piston
Mp = A*L*8000.; %estimated mass of piston
V = X0*A;      % volume of gas  (m3)
M  = 7.995;       %effective mass of piston  (kg)
m = M/A;       % specific mass of effective piston
% initial gas state
P1 = oneatm; T1 = 300;
mech = 'Mevel2017.cti';
gas  = Solution(mech);
q = 'H2:10 O2:1';    
set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
mw = molecularWeights(gas);
rho0 = density(gas);
%%
nsp = nSpecies(gas);
y0 = [X0
      U0
      temperature(gas)
      massFractions(gas)];
tel = [0 0.1];
options = odeset('RelTol',1.e-5,'AbsTol',1.e-12,'Stats','off');
% t0 = cputime;
% out.x = time, out.y = temperature and species profiles
out = ode15s(@adiasys,tel,y0,options,gas,mw,rho0,X0,m);
% compute pressure
nout = length(out.y(1,:));
for n = 1:nout
    rho = X0*rho0/(out.y(1,n)+.001);
    T = out.y(3,n);
    for k =1:nsp
        Y(k) = out.y(k+3,n);
    end
    set(gas, 'T', T, 'Rho', rho, 'Y', Y);
    P(n) = pressure(gas);
end
%%
figure('Name', 'Compression Ignition')
maxx = max(out.x);
minx = min(out.x);
fontsize=12;
set(gca,'FontSize',fontsize,'LineWidth',2);
plot(out.x(:),out.y(3,:));
xlabel('Time (s)','FontSize',fontsize);
ylabel('Temperature (K)','FontSize',fontsize);
title('Gas Temperature','FontSize',fontsize);
xlim([minx maxx])

figure('Name', 'Compression Ignition');
set(gca,'FontSize',fontsize,'LineWidth',2);
plot(out.x(:),P(:));
xlabel('Time (s)','FontSize',fontsize);
ylabel('Pressure (Pa)','FontSize',fontsize);
title('Gas Pressure','FontSize',fontsize);
xlim([minx maxx])
    
 figure('Name', 'Compression Ignition') 
set(gca,'FontSize',fontsize,'LineWidth',2);
plot(out.x(:),out.y(2,:));
xlabel('Time (s)','FontSize',fontsize);
ylabel('Piston speed (m/s)','FontSize',fontsize);
title('Piston Speed','FontSize',fontsize);
xlim([minx maxx])



