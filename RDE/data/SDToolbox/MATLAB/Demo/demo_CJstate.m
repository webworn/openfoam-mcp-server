% Shock and Detonation Toolbox Demo Program
% 
% Calculates the CJ speed using the Minimum Wave Speed Method and 
% then finds the equilibrium state of the gas behind a shock wave 
% traveling at the CJ speed.
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
% Updated July 2018
% Tested with: 
%     MATLAB R2018a, Cantera 2.3 and 2.4
% Under these operating systems:
%     Windows 10, Linux (Debian 9)
%%
clear;clc;
disp('demo_CJstate')
%% set initial state, composition, and gas object
P1 = 100000; T1 = 295; 
q = 'H2:2 O2:1 N2:3.76';    
mech = 'Mevel2017.cti';
gas1 = Solution(mech);
set(gas1,'Temperature',T1,'Pressure',P1,'MoleFractions',q);

%% Find CJ speed
[cj_speed] = CJspeed(P1, T1, q, mech);

%% Evaluate gas state
[gas] = PostShock_eq(cj_speed,P1, T1, q, mech);

%% Evaluate properties of gas object 
T2 = temperature(gas);
P2 = pressure(gas);
R2 = density(gas);
V2 = 1/R2;
S2 = entropy_mass(gas);
w2 = density(gas1)*cj_speed/R2;
u2 = cj_speed - w2;
x2 = moleFractions(gas);
c2_eq = soundspeed_eq(gas);
c2_fr = soundspeed_fr(gas);
gamma2_fr =  c2_fr*c2_fr*R2/P2;
gamma2_eq =  c2_eq*c2_eq*R2/P2;

%% Print out
disp(['CJ computation for ',mech,' with composition ',q])
disp(['Initial state P1 = ',num2str(P1),' Pa & T1 = ',num2str(T1),' K']);
disp(['CJ speed ',num2str(cj_speed),' (m/s)']);
disp('CJ State');
disp(['   Pressure ',num2str(P2),' (Pa)']);
disp(['   Temperature ',num2str(T2),' (K)']);
disp(['   Density ',num2str(R2),' (kg/m3)']);
disp(['   Entropy ',num2str(S2),' (J/kg-K)']);
disp(['   w2 (wave frame) ',num2str(w2),' (m/s)']);
disp(['   u2 (lab frame) ',num2str(u2),' (m/s)']);
disp(['   c2 (frozen) ',num2str(c2_fr),' (m/s)']);
disp(['   c2 (equilibrium) ',num2str(c2_eq),' (m/s)']);
disp(['   gamma2 (frozen) ',num2str(gamma2_fr)]);
disp(['   gamma2 (equilibrium) ',num2str(gamma2_eq)]);
