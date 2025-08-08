% Shock and Detonation Toolbox Demo Program
% 
% Generates plots and output files for a constant volume
% explosion simulation of the reaction following 
% instantaneous isentropic compression from a specified initial condition.
% The time dependence of species, pressure, and temperature 
% are computed using the user supplied reaction mechanism file.
% Effective one-step parameters are computed with numerical perturbations
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
% Updated July 2019
% Tested with: 
%     MATLAB 2017b and 2018a, Cantera 2.3 and 2.4
% Under these operating systems:
%     Windows 8.1, Windows 10, Linux (Debian 9)
%%
clear;clc;close all;
disp('demo_cv')
%%
%Define initial state  and reaction mechanism
P0 = 3.11E6; T0 = 1525;     %vN state for stoichiometric methane-air detonation
q = 'CH4:1 O2:2 N2:7.52';
mechanism = 'gri30_highT';
mech = [mechanism '.cti'];
plots = true;
%%
% SET UP GAS OBJECT
gas = Solution(mech);
set(gas, 'T', T0, 'P', P0, 'X', q);
% Use the following code with specified compression ratio for arbitary 
% compression ratios.
% rho_0 = density(gas);
% s_0 = entropy_mass(gas);
% CR = 30; %volume compression ratio
% set(gas,'Density',CR*rho_0,'Entropy',s_0,'MoleFractions',q);
rho1 = density(gas);
P1 = pressure(gas);
T1 = temperature(gas);
disp(['CV explosion computation']);
disp(['Mechanism: ',mech]);
disp(['Initial State:']);
disp(['   Composition: ',q]);
disp(['   Pressure ',num2str(P1),' (Pa)']);
disp(['   Temperature ',num2str(T1),' (K)']);
disp(['   Density ',num2str(rho1),' (kg/m3)']);
cv = cv_mass(gas);
w = meanMolecularWeight(gas);
gammaminusone = gasconstant/w/cv;
%%
% Compute evoluation of state
tfinal = 1E-4;    % need to set final time sufficiently long for low temperature cases
[CVout] = cvsolve(gas,'t_end',tfinal,'abs_tol',1E-12,'rel_tol',1E-12);
t_ind = CVout.ind_time;   % induction time
t_pulse = CVout.exo_time; % pulse width
disp(['   Induction time = ',num2str(t_ind),' (s)']);
disp(['   Pulse time  = ',num2str(t_pulse),' (s)']);
%%
% Plotting
if (plots)
yes = true(1);
cv_plot(CVout,'maxt',tfinal,'P_scale',1E6,'P_unit','P (MPa)','t_scale',1E-3,'t_unit','t (ms)','major_species',{'CH4','CO2', 'CO','H2', 'O2', 'H2O'},...
    'minor_species',{'H', 'O', 'OH', 'CH2','CH3','H2O', 'H2O2', 'HO2'},'species_plt','mole','prnfig',yes);
end
%%
% Effective one-step parameters. 
% See Development of One-Step Chemistry Models for Flame and Ignition
% Simulation S.P.M. Bane, J.L. Ziegler, and J.E. Shepherd GALCIT Report
% GALCITFM:2010.002 March 30, 2010

% Compute perturbed reaction time to get effective activation energy
DeltaT = .001*T1;
set(gas,'Density', rho1,'Temperature',T1+DeltaT,'MoleFractions',q);
[out] = cvsolve(gas,'t_end',tfinal,'abs_tol',1E-12,'rel_tol',1E-12);
t_ind_prime = out.ind_time;
theta = 2. - T1*(t_ind_prime - t_ind)/t_ind/DeltaT;
E_a = theta*T1*gasconstant;
% Compute perturbed reaction time to get effective density exponent
Deltarho = .01*rho1;
set(gas,'Density', rho1+Deltarho,'Temperature',T1,'MoleFractions',q);
[out] = cvsolve(gas,'t_end',tfinal,'abs_tol',1E-12,'rel_tol',1E-12);
t_ind_prime = out.ind_time;
reac_order = -1*rho1*(t_ind_prime - t_ind)/t_ind/Deltarho;
disp(['   E_a  = ',num2str(E_a/4.184E6),' kcal/mol']);
disp(['   Theta  = ',num2str(theta)]);
disp(['   g-1  = ',num2str(gammaminusone)]);
disp(['   n_rho  = ',num2str(reac_order)]);
disp(['   W  = ',num2str(w),' (kg/kmol)']);
disp(['   C_v  = ',num2str(cv),' (J/kg K)']);
