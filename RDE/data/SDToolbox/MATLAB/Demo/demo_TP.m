% Shock and Detonation Toolbox Demo Program
% 
% Explosion computation simulating constant temperature and pressure reaction.
% Reguires function tpsys.m for ode solver
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
disp('Constant Temperature and Pressure Reactor')
%% initial gas state
P1 = oneatm; T1 = 3000;
mech = 'Mevel2015.cti';
gas  = Solution(mech);
q = 'H2:10 O2:1';    
set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
%% set up for integration routine
mw = molecularWeights(gas);
nsp = nSpecies(gas);
y0 = [massFractions(gas)];
t_end = 1e-5;
tel = [0 t_end];
options = odeset('RelTol',1.e-5,'AbsTol',1.e-12,'Stats','off');
% out.x = time, out.y = species profiles
out = ode15s(@tpsys,tel,y0,options,gas,mw,T1,P1);
%% Plotting species
figure('Name', 'TP Reactor')
maxx = max(out.x);
minx = min(out.x);
fontsize=12;
set(gca,'FontSize',fontsize,'LineWidth',2,...
    'linestyleorder',{'-','-.','--',':'},'nextplot','add');
plot(out.x(:),out.y(:,:));
xlabel('Time (s)','FontSize',fontsize);
ylabel('species mass fraction','FontSize',fontsize);
title('TP Reactor','FontSize',fontsize);
xlim([minx maxx]);
legend(speciesName(gas,1:nsp),'Location','eastoutside');

