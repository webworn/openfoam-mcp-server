% Shock and Detonation Toolbox Demo Program
% 
% Calculates 2 reflection conditions
%   1) equilibrium post-initial-shock state behind a shock traveling at CJ speed (CJ state) 
%      followed by equilibrium post-reflected-shock state
%   2) frozen post-initial-shock state behind a CJ wave 
%      followed by frozen post-reflected-shock state
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
disp('demo_CJ_and_shock_state')

% set initial state, composition, and gas object
P1 = 100000; T1 = 300; 
q ='CH4:0.5 O2:1 N2:3.76';
mech = 'gri30_highT.cti';
gas1 = Solution(mech);
set(gas1,'Temperature',T1,'Pressure',P1,'MoleFractions',q);

% Find CJ speed
[cj_speed] = CJspeed(P1, T1, q, mech);

% Evaluate gas state
[gas] = PostShock_eq(cj_speed,P1, T1, q, mech);

% Evaluate properties of gas object 
T2 = temperature(gas);
P2 = pressure(gas);
R2 = density(gas);
V2 = 1/R2;
S2 = entropy_mass(gas);
w2 = density(gas1)*cj_speed/R2;
u2 = cj_speed - w2;
x2 = moleFractions(gas);
a2_eq = soundspeed_eq(gas);
a2_fr = soundspeed_fr(gas);
gamma2_fr =  a2_fr*a2_fr*R2/P2;
gamma2_eq =  a2_eq*a2_eq*R2/P2;

% Print out
disp(['CJ computation for ',mech,' with composition ',q])
disp('CJ Parameters')
disp(['   UCJ ',num2str(cj_speed),' (m/s)']);
disp(['   Pressure ',num2str(P2),' (Pa)']);
disp(['   Temperature ',num2str(T2),' (K)']);
disp(['   Density ',num2str(R2),' (kg/m3)']);
disp(['   Entropy ',num2str(S2),' (J/kg-K)']);
disp(['   w2 (wave frame) ',num2str(w2),' (m/s)']);
disp(['   u2 (lab frame) ',num2str(u2),' (m/s)']);
disp(['   a2 (frozen) ',num2str(a2_fr),' (m/s)']);
disp(['   a2 (equilibrium) ',num2str(a2_eq),' (m/s)']);
disp(['   gamma2 (frozen) ',num2str(gamma2_fr),' (m/s)']);
disp(['   gamma2 (equilibrium) ',num2str(gamma2_eq),' (m/s)']);
disp('--------------------------------------');
%
gas3 = Solution(mech);
[p3,UR,gas3] = reflected_eq(gas1,gas,gas3,cj_speed);
disp(['Reflected CJ shock (equilibrium) computation for ',mech,' with composition ',q])
disp(['   CJ speed ',num2str(cj_speed),' (m/s)']);
disp(['   Reflected wave speed ',num2str(UR),' (m/s)']);
disp('Post-Reflected Shock Parameters')
disp(['   Pressure ',num2str(pressure(gas3)),' (Pa)']);
disp(['   Temperature ',num2str(temperature(gas3)),' (K)']);
disp(['   Density ',num2str(density(gas3)),' (kg/m3)']);
disp('--------------------------------------');
%
disp(['Shock computation for ',mech,' with composition ',q]);
[gas] = PostShock_fr(cj_speed,P1, T1, q, mech);
disp(['    shock speed ',num2str(cj_speed),' (m/s)']);
disp('Postshock State');
disp(['    Pressure ',num2str(pressure(gas)),' (Pa)']);
disp(['    Temperature ',num2str(temperature(gas)),' (K)']);
disp(['    Density ',num2str(density(gas)),' (kg/m3)']);
disp(['    Entropy ',num2str(entropy_mass(gas)),' (J/kg-K)']);
w2 = cj_speed*density(gas1)/density(gas);
u2 = cj_speed-w2;
disp(['    w2 (wave frame) ',num2str(w2),' (m/s)']);
disp(['    u2 (lab frame) ',num2str(u2),' (m/s)']);
disp(['    a2 (frozen) ',num2str(soundspeed_fr(gas)),' (m/s)']);
gamma2_fr =  soundspeed_fr(gas)^2*density(gas)/pressure(gas);
disp(['    gamma2 (frozen) ',num2str(gamma2_fr),' (m/s)']);
disp('--------------------------------------');
%
[p3,UR,gas3] = reflected_fr(gas1,gas,gas3,cj_speed);
disp(['Reflected CJ shock (frozen) computation for ',mech,' with composition ',q])
disp(['   Shock speed ',num2str(cj_speed),' (m/s)']);
disp(['   Reflected wave speed ',num2str(UR),' (m/s)']);
disp('Post-Reflected Shock Parameters')
disp(['   Pressure ',num2str(pressure(gas3)),' (Pa)']);
disp(['   Temperature ',num2str(temperature(gas3)),' (K)']);
disp(['   Density ',num2str(density(gas3)),' (kg/m3)']);

