% Shock and Detonation Toolbox Demo Program
% 
% Generates plots and output files for a constant volume
% explosion simulation where the initial conditions are adiabaically
% compressed reactants. The time dependence of species, pressure, and
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
% Updated July 2019
% Tested with: 
%     MATLAB 2017b and 2018a, Cantera 2.3 and 2.4
% Under these operating systems:
%     Windows 8.1, Windows 10, Linux (Debian 9)
%%
clear;clc;close all;
disp('demo_cv_comp')
%%
%Define initial state (uncompressed)
P1 = oneatm; T1 = 298.15;  
q = 'H2:2 O2:1';
%mechanism = 'Mevel2015';
mechanism = 'Hong2011';
%mechanism='Keromnes2013';
%mechanism='Burke2012';
%mechanism='sandiego20161214_H2only';
%mechanism='GRI30';
mech = [mechanism '.cti'];
plot_ind = true;
file = true;  % set true or false as needed
file_name = ['compression_ignition_' mechanism];
%%
% SET UP GAS OBJECT
gas = Solution(mech);
set(gas, 'T', T1, 'P', P1, 'X', q);
rho1 = density(gas);
s1 = entropy_mass(gas);
disp(['Adiabatic compression CV explosion computation for ',mech,' with composition ',q])
disp('Initial (uncompressed) State');
disp(['   Pressure ',num2str(P1),' (Pa)']);
disp(['   Temperature ',num2str(T1),' (K)']);
disp(['   Density ',num2str(rho1),' (kg/m3)']);
if (file)
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % CREATE OUTPUT TEXT FILE
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    fn = [file_name, '.txt'];
    d = date;

    fid = fopen(fn, 'w');
    fprintf(fid, '# CV: EXPLOSION STRUCTURE CALCULATION - CR version\n');
    fprintf(fid, '# CALCULATION RUN ON %s\n\n', d);

    fprintf(fid, '# Uncompressed state\n');
    fprintf(fid, '# TEMPERATURE (K) %4.1f\n', T1);
    fprintf(fid, '# PRESSURE (ATM) %2.1f\n', P1/oneatm);
    fprintf(fid, '# DENSITY (KG/M^3) %1.4e\n', rho1);
    fprintf(fid, ['# SPECIES MOLE FRACTIONS: ' q '\n']);
    fprintf(fid, '# Compressed state: \n');
    fprintf(fid, 'CR,T(K),P(MPa),t_ind(s),t_pulse(s),Theta,g-1,n_rho\n');
end
%%
i = 0;
% Define a series of post-compression states by isentropic compression of
% initial state
for compression_ratio = 15:60
    i = i + 1;
    set(gas,'Density',compression_ratio*rho1,'Entropy',s1,'MoleFractions',q);
% % Comment out above and use following instructions for given T, P =
% % constant states
%     i = i + 1;
%     for T = 900.0:10.0:1020.0
    T2(i) = temperature(gas);
    P2(i) = pressure(gas); 
    rho2(i) = density(gas);
    cv = cv_mass(gas);
    w = meanMolecularWeight(gas);
    gammaminusone = gasconstant/w/cv;
    %%
    disp('Compressed State');
    disp(['   Compression ratio ',num2str(compression_ratio)]);
    disp(['   Pressure ',num2str(P2(i)),' (Pa)']);
    disp(['   Temperature ',num2str(T2(i)),' (K)']);
    disp(['   Density ',num2str(rho2(i)),' (kg/m3)']);
    %%
    % Compute constant-volume explosion
    tfinal = 10000.;    % need to set final time sufficiently long for low temperature cases
    [CVout] = cvsolve(gas,'t_end',tfinal,'abs_tol',1E-12,'rel_tol',1E-12);
    t_ind(i) = CVout.ind_time;
    t_pulse(i) = CVout.exo_time;
    disp(['   Induction time = ',num2str(t_ind(i)),' (s)']);
    disp(['   Pulse time  = ',num2str(t_pulse(i)),' (s)']);
   % Compute perturbed reaction time to get effective activation energy
    DeltaT = .001*T2(i);
    set(gas,'Density', rho2(i),'Temperature',T2(i)+DeltaT,'MoleFractions',q);
    [out] = cvsolve(gas,'t_end',tfinal,'abs_tol',1E-12,'rel_tol',1E-12);
    t_ind_prime = out.ind_time;
    theta(i) = 2. - T2(i)*(t_ind_prime - t_ind(i))/t_ind(i)/DeltaT;
    E_a = theta(i)*T2(i)*gasconstant;
    % Compute perturbed reaction time to get effective density exponent
    Deltarho = .01*rho2(i);
    set(gas,'Density', rho2(i)+Deltarho,'Temperature',T2(i),'MoleFractions',q);
    [out] = cvsolve(gas,'t_end',tfinal,'abs_tol',1E-12,'rel_tol',1E-12);
    t_ind_prime = out.ind_time;
    reac_order(i) = -1*rho2(i)*(t_ind_prime - t_ind(i))/t_ind(i)/Deltarho;
    disp(['   E_a  = ',num2str(E_a/4.184E6),' kcal/mol']);
    disp(['   Theta  = ',num2str(theta(i))]);
    disp(['   g-1  = ',num2str(gammaminusone)]);
    disp(['   n_rho  = ',num2str(reac_order(i))]);
    if (file)
        y = [compression_ratio;T2(i);P2(i)/1E6;t_ind(i);t_pulse(i);theta(i);gammaminusone;reac_order(i)];
        fprintf(fid, '%1.4g,%1.4g,%6.2f,%3.2e,%3.2e,%3.2e,%3.2e,%3.2e\n', y);
    end
end
if (file) 
    fclose(fid);
end
if (plot_ind)
    %
    % plot induction time vs reciprocal temperature
    figure('Name', 'Adiabatic Compression Ignition')
    Tbar = 1E3./T2;
    maxy = max(t_ind(:));
    miny = min(t_ind(:));
    maxx = max(Tbar(:));
    minx = min(Tbar(:));
    fontsize=12;
    set(gca,'FontSize',fontsize,'LineWidth',2);
    semilogy(Tbar(:),t_ind(:));
    axis([minx maxx miny maxy]);
    xlabel('1000/T','FontSize',fontsize);
    ylabel('induction time (s)','FontSize',fontsize);
    title('Adiabatic compression CV ignition','FontSize',fontsize);
    
    % plot pressure vs temperature
    figure('Name', 'Adiabatic Compression Ignition')
    P = P2/1E6';
    maxy = max(P(:));
    miny = min(P(:));
    maxx = max(T2(:));
    minx = min(T2(:));
    fontsize=12;
    set(gca,'FontSize',fontsize,'LineWidth',2);
    plot(T2(:),P(:));
    axis([minx maxx miny maxy]);
    xlabel('temperature (K)','FontSize',fontsize);
    ylabel('pressure(MPa)','FontSize',fontsize);
    title('Adiabatic compression CV ignition','FontSize',fontsize);
end
