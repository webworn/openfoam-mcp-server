% Shock and Detonation Toolbox Demo Program
% 
% Computes ZND and CV models of detonation with the shock front
% traveling at the CJ speed.  Evaluates various measures of the reaction
% zone thickness and exothermic pulse width, effective activation energy
% and Ng stability parameter. 
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
% Updated January 2019
% Tested with: 
%     MATLAB R2017b, Cantera 2.3 and 2.4
% Under these operating systems:
%     Windows 10, Linux (Debian 9)
%%
clear;clc; close all;
disp('demo_ZND')

P1 = 100000; T1 = 300; 
%q = 'H2:.075 CO:.075 O2:0.1785  N2:0.6715';
%q = 'H2:0.15 O2:0.1785  N2:0.6715';
%q = 'CO:1 H2:1 O2:1 N2:3.76';
q = 'CH4:1 O2:2 N2:7.52';

%mech = 'Keromnes2013.cti'; 
mech = 'gri30_highT.cti';

fname = 'H2-CO-air';

[cj_speed, curve, ~, dnew, plot_data] = CJspeed(P1, T1, q, mech);
%CJspeed_plot(2,plot_data,curve,dnew)

gas1 = Solution(mech);
set(gas1, 'T', T1, 'P', P1, 'X', q);

% FIND EQUILIBRIUM POST SHOCK STATE FOR GIVEN SPEED
[gas] = PostShock_eq(cj_speed, P1, T1, q, mech);
u_cj = cj_speed*density(gas1)/density(gas);

% FIND FROZEN POST SHOCK STATE FOR GIVEN SPEED
[gas] = PostShock_fr(cj_speed, P1, T1, q, mech);

% SOLVE ZND DETONATION ODES
[out] = zndsolve(gas,gas1,cj_speed,'advanced_output',true,'t_end',.005);

b = length(out.T);
disp(['ZND computation results; ']);
disp(['Mixture ', q]);
disp(['Mechanism ', mech]);
disp(['Initial temperature ',num2str(T1,'%8.3e'),' K']);
disp(['Initial pressure ',num2str(P1,'%8.3e'),' Pa']);
disp(['CJ speed ',num2str(cj_speed,'%8.3e'),' m/s']);
disp([' ']);
disp(['Reaction zone computation end time = ',num2str(out.tfinal,'%8.3e'),' s']);
disp(['Reaction zone computation end distance = ',num2str(out.xfinal,'%8.3e'),' m']);
disp([' ']);
disp(['T (K), initial = ' num2str(out.T(1),5) ', final ' num2str(out.T(b),5) ', max ' num2str(max(out.T(:)),5)]);
disp(['P (Pa), initial = ' num2str(out.P(1),3) ', final ' num2str(out.P(b),3) ', max ' num2str(max(out.P(:)),3)]);
disp(['M, initial = ' num2str(out.M(1),3) ', final ' num2str(out.M(b),3) ', max ' num2str(max(out.M(:)),3)]);
disp(['u (m/s), initial = ' num2str(out.U(1),5) ', final ' num2str(out.U(b),5) ', cj ' num2str(u_cj,5)]);
disp([' ']);
disp(['Reaction zone thermicity half-width = ',num2str(out.exo_len_ZND,'%8.3e'),' m']);
disp(['Reaction zone maximum thermicity distance = ',num2str(out.ind_len_ZND,'%8.3e'),' m']);
disp(['Reaction zone thermicity half-time = ',num2str(out.exo_time_ZND, '%8.3e'),' s']);
disp(['Reaction zone maximum thermicity time = ',num2str(out.ind_time_ZND,'%8.3e'),' s']);
disp(['Cell width estimate (A=29) ',num2str(29*out.ind_len_ZND, '%8.3e'),' m']);
yes = true(1);
znd_plot(out,'maxx',25,'major_species',{'CH4', 'O2', 'CO', 'N2', 'CO2', 'H2O'},...
    'minor_species',{'H', 'O', 'OH', 'H2O2', 'HO2','CH2','CH3','H2'},'species_plt','mole','prnfig','yes');

