% Shock and Detonation Toolbox Demo Program
% 
% Calculates the frozen shock (vN = von Neumann) state of the gas behind 
% the leading shock wave in a CJ detonation.
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
disp('demo_vN_state')
%% set initial state, composition, and gas object
P1 = 100000.; T1 = 300.; 
%q = 'H2:2.00 O2:1.0 N2:3.76';
%q = 'C2H4:1.00 O2:3 N2:11.28'; 
q = 'C2H2:1 O2:2.5 AR:3';
%q = 'C2H4:1. O2:3';    
%q = 'O2:1. N2:3.76';
mech = 'gri30_highT.cti';
gas1 = Solution(mech);
set(gas1,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
%% Evaluate initial state
R1 = density(gas1);
c1_fr=soundspeed_fr(gas1);
cp1 = cp_mass(gas1);
w1 =meanMolecularWeight(gas1);
gamma1_fr =  c1_fr*c1_fr*R1/P1;
%% Set shock speed
[cj_speed] = CJspeed(P1, T1, q, mech);
Us = cj_speed;
%% Evaluate gas state
%q = 'O2:1. N2:3.76';
[gas] = PostShock_eq(Us,P1, T1, q, mech);

% Evaluate properties of gas object 
T2 = temperature(gas);
P2 = pressure(gas);
R2 = density(gas);
V2 = 1/R2;
S2 = entropy_mass(gas);
w2 = density(gas1)*Us/R2;
u2 = Us - w2;
x2 = moleFractions(gas);
c2_eq = soundspeed_eq(gas);
c2_fr = soundspeed_fr(gas);
gamma2_fr =  c2_fr*c2_fr*R2/P2;
gamma2_eq =  c2_eq*c2_eq*R2/P2;

%% Print out
disp(['CJ computation for ',mech,' with composition ',q])
disp(['Initial state'])
disp(['   Pressure ',num2str(P1),' (Pa)']);
disp(['   Temperature ',num2str(T1),' (K)']);
disp(['   Density ',num2str(R1),' (kg/m3)']);
disp(['   c1 (frozen) ',num2str(c1_fr),' (m/s)']);
disp(['   gamma1 (frozen) ',num2str(gamma1_fr),' ']);
disp(['   Cp1 (frozen) ',num2str(cp1),' ']);
disp(['   W1  ',num2str(w1),' ']);
disp('vN State');
disp(['Shock speed ',num2str(Us),' (m/s)']);
disp(['   Pressure ',num2str(P2),' (Pa)']);
disp(['   Temperature ',num2str(T2),' (K)']);
disp(['   Density ',num2str(R2),' (kg/m3)']);
disp(['   Entropy ',num2str(S2),' (J/kg-K)']);
disp(['   w2 (wave frame) ',num2str(w2),' (m/s)']);
disp(['   u2 (lab frame) ',num2str(u2),' (m/s)']);
disp(['   c2 (frozen) ',num2str(c2_fr),' (m/s)']);
disp(['   c2 (equilibrium) ',num2str(c2_eq),' (m/s)']);
disp(['   gamma2 (frozen) ',num2str(gamma2_fr),' ']);
disp(['   gamma2 (equilibrium) ',num2str(gamma2_eq),' ']);
