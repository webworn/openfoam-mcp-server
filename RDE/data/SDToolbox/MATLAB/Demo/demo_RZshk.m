% Shock and Detonation Toolbox Demo Program
% 
% Generate plots and output files for a reaction zone behind a shock front traveling at speed U.
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
clear;clc;close all;
disp('demo_RZshk')
graphing = true;

P1 = 13.33; T1 = 300; U1 = 4000; 
q = 'CO2:0.96 N2:0.04';
mech = 'airNASA9noions.cti';

gas1 = Solution(mech);
set(gas1, 'T', T1, 'P', P1, 'X', q);
nsp = nSpecies(gas1);

% FIND POST SHOCK STATE FOR GIVEN SPEED
[gas] = PostShock_fr(U1, P1, T1, q, mech);

% SOLVE REACTION ZONE STRUCTURE ODES
[out] = zndsolve(gas,gas1,U1,'t_end',0.5,'advanced_output',false,...
                    'rel_tol',1e-12,'abs_tol',1e-12);
                
if graphing
    maxx = max(out.distance);
    figs = znd_plot(out,'maxx',maxx,...
                    'major_species','All',...
                    'xscale','log','show',false);
    figure(figs(1))
    print('-depsc2','-r600','shk-T.eps')
    figure(figs(2))
    print('-depsc2','-r600','shk-P.eps')
    figure(figs(3))
    print('-depsc2','-r600','shk-M.eps')
    figure(figs(4))
    print('-depsc2','-r600','shk-therm.eps')
    figure(figs(5))
    xlim([1e-5,maxx]);
    ylim([1e-10,1]);
    print('-depsc2','-r600','shk-Y.eps') 
end

% save output file for later comparison
rzn = out;
save 'rzn.mat' rzn
