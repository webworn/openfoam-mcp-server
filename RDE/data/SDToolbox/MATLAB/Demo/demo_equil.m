% Shock and Detonation Toolbox Demo Program
% 
% Equilibrium computation at over a range of temperatures and pressures
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
disp('Constant Temperature and Pressure Equilibrium')
%% initial gas state
P1 = oneatm; T1 = 300;
mech = 'airNASA9ions.cti';
gas  = Solution(mech);
q = 'N2:0.7809 O2:0.2095  Ar:0.0093  CO2:0.0004';    %earths atmosphere
nsp = nSpecies(gas);
T1 = 500.;
for k=1:1:75
    T(k) = T1;
    set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
    gas = equilibrate(gas, 'TP');   
    X(k,:) = moleFractions(gas);
    T1 = T1 + 250.;
end
%%
figure('Name', 'Equilibrium Composition')
maxx = max(T);
minx = min(T);
fontsize=12;
set(gca,'FontSize',fontsize,'LineWidth',2,...
    'linestyleorder',{'-','-.','--',':'},...
    'nextplot','add');
set(gca, 'Yscale', 'log');
set(gca, 'Xscale', 'linear');
axis([minx maxx 1.0E-5 1]);
plot(T(:),X(:,:));
xlabel('Temperature (K)','FontSize',fontsize);
ylabel('species mole fraction','FontSize',fontsize);
legend(speciesName(gas,1:nsp),'Location','eastoutside','Fontsize',8);

