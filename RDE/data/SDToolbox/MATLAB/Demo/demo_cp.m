% demo_cp
% Shock and Detonation Toolbox Demo Program
%
% Generates plots and output files for a adiabatic 
% explosion simulation with constant pressure.
% The time dependence of species, pressure, and
% temperature are computed using the user supplied reaction mechanism file.
%
% ################################################################################
% Theory, numerical methods and applications are described in the following report:
%
% IGNITION MODELING AND THE  CRITICAL DECAY RATE CONCEPT
%
% J. E. SHEPHERD
% GRADUATE AEROSPACE LABORATORIES
% CALIFORNIA INSTITUTE OF TECHNOLOGY
% PASADENA, CA 91125
% GALCIT REPORT EDL2019.002  July 2019
%
% Please cite this report and the website if you use these routines.
%
% Please refer to LICENCE.txt or the above report for copyright and disclaimers.
%
% http://shepherd.caltech.edu/EDL/PublicResources/sdt/
%
% ################################################################################
% Updated December 2019
% Tested with:
%     MATLAB 2017b and 2018a, Cantera 2.3 and 2.4
% Under these operating systems:
%     Windows 8.1, Windows 10, Linux (Debian 9)
%%
clear;clc;close all;
plots = true;
disp('demo_cp')
q = 'CH4:1 O2:2 N2:7.52';
mechanism = 'gri30_highT';

mech = [mechanism '.cti'];
P0 = 3.11E6; T0 = 1525;
gas = Solution(mech);
set(gas, 'T', T0, 'P', P0, 'X', q);
rho1 = density(gas);
P1 = pressure(gas);
T1 = temperature(gas);
disp(['CP explosion computation']);
disp(['Mechanism: ',mech]);
disp(['Initial State:']);
disp(['   Composition: ',q]);
disp(['   Pressure ',num2str(P1),' (Pa)']);
disp(['   Temperature ',num2str(T1),' (K)']);
disp(['   Density ',num2str(rho1),' (kg/m3)']);
tfinal = 1E-4;    % need to set final time sufficiently long for low temperature cases
[CPout] = cpsolve(gas,'t_end',tfinal,'abs_tol',1E-12,'rel_tol',1E-12);
t_ind = CPout.ind_time;   % induction time
t_pulse = CPout.exo_time; % pulse width
disp(['   Induction time = ',num2str(t_ind),' (s)']);
disp(['   Pulse time  = ',num2str(t_pulse),' (s)']);
%%
% Plotting
if (plots)
yes = true(1);
cp_plot(CPout,'maxt',tfinal,'P_scale',0,'P_unit','P (MPa)','t_scale',1E-3,'t_unit','t (ms)','major_species',{'CH4','CO2', 'CO','H2', 'O2', 'H2O'},...
    'minor_species',{'H', 'O', 'OH', 'CH2','CH3','H2O', 'H2O2', 'HO2'},'species_plt','mole','prnfig',yes);
end



