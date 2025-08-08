% Shock and Detonation Toolbox Demo Program
% 
% Generate plots and output files for a ZND detonation with the shock front
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
% Updated September 2018
% Tested with: 
%     MATLAB 2017b and 2018a, Cantera 2.3 and 2.4
% Under these operating systems:
%     Windows 8.1, Windows 10, Linux (Debian 9)
%%
clear;clc;
disp('demo_ZNDCJ')

P1 = 100000; T1 = 300; 
q = 'H2:2 O2:1 N2:3.76';
mech = 'Mevel2017.cti'; 

fname = 'h2air';

[cj_speed, curve, ~, dnew, plot_data] = CJspeed(P1, T1, q, mech);
CJspeed_plot(2,plot_data,curve,dnew)

gas1 = Solution(mech);
set(gas1, 'T', T1, 'P', P1, 'X', q);

% FIND POST SHOCK STATE FOR GIVEN SPEED
[gas] = PostShock_fr(cj_speed, P1, T1, q, mech);

% SOLVE ZND DETONATION ODES
[out] = zndsolve(gas,gas1,cj_speed,'advanced_output',true);
znd_plot(out);
znd_fileout(fname,out)

disp(['Reaction zone pulse width (exothermic length) = ',num2str(out.exo_len_ZND),' m']);
disp(['Reaction zone induction length = ',num2str(out.ind_len_ZND),' m']);
disp(['Reaction zone pulse time (exothermic time) = ',num2str(out.exo_time_ZND),' s']);
disp(['Reaction zone induction time = ',num2str(out.ind_time_ZND),' s']);
