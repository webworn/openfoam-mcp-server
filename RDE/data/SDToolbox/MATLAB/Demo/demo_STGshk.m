% Shock and Detonation Toolbox Demo Program
% 
% Generate plots and output files for a steady reaction zone between a shock and a blunt body using
% Hornung model of linear profile of rho u on stagnation streamline.
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
disp('demo_STGshk')
graphing = true;

P1 = 13.33; T1 = 300; U1 = 4000; Delta = 5;
q = 'CO2:0.96 N2:0.04';
mech = 'airNASA9noions.cti';

gas1 = Solution(mech);
set(gas1, 'T', T1, 'P', P1, 'X', q);
nsp = nSpecies(gas1);

% FIND POST SHOCK STATE FOR GIVEN SPEED
[gas] = PostShock_fr(U1, P1, T1, q, mech);

% SOLVE REACTION ZONE STRUCTURE ODES
[out] = stgsolve(gas,gas1,U1,Delta,'t_end',0.5,...
                    'rel_tol',1e-12,'abs_tol',1e-12);

if graphing
    maxx = max(out.distance);
    figs = znd_plot(out,'maxx',maxx,...
                    'major_species','All',...
                    'xscale','log','show',false);
    figure(figs(1))
    print('-depsc2','-r600','stg-T.eps')
    figure(figs(2))
    print('-depsc2','-r600','stg-P.eps')
    figure(figs(3))
    print('-depsc2','-r600','stg-M.eps')
    figure(figs(4))
    print('-depsc2','-r600','stg-therm.eps')
    figure(figs(5))
    xlim([1e-5,maxx]);
    ylim([1e-10,1]);
    print('-depsc2','-r600','stg-Y.eps') 
end
    
% save output file for later comparison
stg = out;
save 'stg.mat' stg ;
