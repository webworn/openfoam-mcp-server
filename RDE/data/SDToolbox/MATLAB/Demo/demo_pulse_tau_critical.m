%% demo_pulse_tau_critical.m
% Shock and Detonation Toolbox Demo Program
%
% Computes adiabatic compression and explosion with a specified volume as a
% fuction of time.  Set up for Gaussian pulse model with two parameters
%   CR - compression ratio (Vmax/Vmin)
%   tau - pulse width paramter in Gaussian function. 
%
%
% Requires:
%   f_volume.m defines normalized volume and derivative as a function of time
%   cvsys_v.m to carry out the integration of energy and species equations
%
% Integrates the equations twice, once for nonreactive, once for reactive case.  Finds the
% critical pulse width for a set of  compression ratios as described in:
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
close all; clc; clear;
plot_cv = true;  % plot temperature and pressure
plot_species = false;  % plot major and minor species
file_out = true;  % output summary to file
%% chemical composition and mechanism  
q = 'H2:2 O2:1 N2:1';
%mechanism = 'Mevel2015';
mechanism = 'Hong2011';
%mechanism='Keromnes2013';
%mechanism='Burke2012';
%mechanism='sandiego20161214_H2only';
%mechanism='GRI30';
mech = [mechanism '.cti'];
%% initial conditions
P1 = oneatm; T1 = 298.15;
gas = Solution(mech);
set(gas, 'T', T1, 'P', P1, 'X', q);
rho_0 = density(gas);
Y_0 = massFractions(gas);
if (file_out)
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % CREATE OUTPUT TEXT FILE
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    file_name = ['V_ignition_' mechanism];
    fn = [file_name, '.txt'];
    d = date;
    fid = fopen(fn, 'w');
    fprintf(fid, '# V(t): EXPLOSION STRUCTURE CALCULATION\n');
    fprintf(fid, '# Date: %s\n', d);
    fprintf(fid, ['# Mechanism: ' mech '\n']);
    fprintf(fid, ['# Species mole fractions: ' q '\n']);
    fprintf(fid, ['# Initial temperature (K): ',num2str(T1), '\n']);
    fprintf(fid, ['# Initial pressure (Pa): ',num2str(P1), '\n']);
    fprintf(fid, 'tau_p(s),CR,T_ign(K),P_ign(Pa), t_c (s), t_ign(s), t_ind(s)\n');
end
%%   Loop through compression ratio  
% set parameters for integration
rel_tol = 1e-8;
abs_tol = 1e-8;
tau = 5E-05 ;  % need to start with a value that is larger (but not too large) than  for first CR value
factor = 0.99;  % factor to reduce tau while seeking the critical condition
for CR = 51:60 % 
    display(['CR = ', num2str(CR)]);
    V_min = 1./CR;
    expl = true;
    % loop through values of tau to find critical value tau that causes
    % explosion to fail
    while(expl  && tau > 1E-6)    % Don't let the loop run idefinitely
        tau = tau*factor;
        % reactive integration
        reac = true;
        t_min = 3*tau;   % time (s) of location of minimum volume
        t_end = 6*tau;   % need to set final time sufficiently long for low temperature cases
        % integration parameters
        tel = [0 t_end];
        options = odeset('RelTol',rel_tol,'AbsTol',abs_tol,'Stats','off');
        y0 = [temperature(gas)
            Y_0];
        mw = molecularWeights(gas);
        set(gas, 'T', T1, 'P', P1, 'X', q);
        out = ode15s(@cvsys_v,tel,y0,options,gas,mw,rho_0,reac,tau,V_min,t_min);
        time_r = out.x;
        temp_r = out.y(1,:);
        y_r = out.y(2:end,:);
        %% nonreactive integration
        reac = false;
        set(gas, 'T', T1, 'P', P1, 'X', q);
        out = ode15s(@cvsys_v,tel,y0,options,gas,mw,rho_0,reac,tau,V_min,t_min);
        time_n = out.x;
        temp_n = out.y(1,:);
        y_n = out.y(2:end,:);
        %% interpolate nonreactive case to reactive output times
        % find explosion time, temperature and pressure
        temp_nr = interp1(time_n,temp_n,time_r);
        temp_diff = (temp_r-temp_nr)./temp_nr;
        np = length(time_r);
        k_s = 1;  % skip first k_s elements that are less than .005
        while (temp_diff(k_s) < .005 && k_s < np-1)
            k_s = k_s + 1;
        end
        if (k_s == np-1 )
            display(['tau = ', num2str(tau),' No explosion detected in ',num2str(t_end), ' s']);
            time_ign = 0.0;
            expl_temp = 0.0;
            expl = false;
        else
            time_ign = time_r(k_s);
            expl_temp = temp_r(k_s);
            display(['tau = ', num2str(tau),' Explosion detected at ',num2str(time_ign), ' s T = ',num2str(expl_temp), ' K']);
        end
        temp_rise_max = max(temp_r-temp_nr);
        if (temp_rise_max < 500)
            display(['tau = ', num2str(tau),' Mild or no reaction, max temperature rise ',num2str(temp_rise_max),' K']);
            expl = false;
        else
            expl = true;
        end
    end
    if (expl)
        display(['Pulse half width (s) ',num2str(tau)]);
        display(['Compression ratio ',num2str(CR)]);
    else
        display(['NO explosion at tau ',num2str(tau)])
    end
    %% back up and recompute the last case that actually exploded
    tau = tau/factor;
    reac = true;
    t_min = 3*tau;   % time (s) of location of minimum volume
    t_end = 6*tau;   % need to set final time sufficiently long for low temperature cases
    % integration parameters
    tel = [0 t_end];
    options = odeset('RelTol',rel_tol,'AbsTol',abs_tol,'Stats','off');
    y0 = [temperature(gas)
        Y_0];
    mw = molecularWeights(gas);
    set(gas, 'T', T1, 'P', P1, 'X', q);
    out = ode15s(@cvsys_v,tel,y0,options,gas,mw,rho_0,reac,tau,V_min,t_min);
    time_r = out.x;
    temp_r = out.y(1,:);
    y_r = out.y(2:end,:);
    k_s = 1;  % skip first k_s elements that are less than .005
    while (temp_diff(k_s) < .005 && k_s < np-1)
        k_s = k_s + 1;
    end
    time_ign = time_r(k_s);
    expl_temp = temp_r(k_s);
    %% nonreactive integration
    reac = false;
    set(gas, 'T', T1, 'P', P1, 'X', q);
    out = ode15s(@cvsys_v,tel,y0,options,gas,mw,rho_0,reac,tau,V_min,t_min);
    time_n = out.x;
    temp_n = out.y(1,:);
    y_n = out.y(2:end,:);
    % interpolate nonreactive case to reactive output times
    temp_nr = interp1(time_n,temp_n,time_r);
    %% compute pressure
    np = length(time_r);
    p_r = zeros(1,np);
    rho_r = zeros(1,np);
    tau_r = zeros(1,np);
    for k=1:np
        [V, dVdt] = f_volume(time_r(k),tau,V_min,t_min);
        tau_r(k) = V/dVdt;
        rho_r(k) = rho_0/V;
        set(gas, 'T', temp_r(k), 'Rho', rho_r(k), 'Y', y_r(1:end,k));
        p_r(k) = pressure(gas);
    end
    npn = length(time_n);
    p_n = zeros(1,npn);
    rho_n = zeros(1,npn);
    for k = 1:npn
        [V, dVdt] = f_volume(time_n(k),tau,V_min,t_min);
        rho_n(k) = rho_0/V;
        set(gas, 'T', temp_n(k), 'Rho', rho_n(k), 'Y', y_r(1:end,k));
        p_n(k) = pressure(gas);
    end
    tau_r_expl = tau_r(k_s);
    [p_max, k_max] = max(p_r);
    T_max = max(temp_r);
    expl_pres = p_r(k_s);
    P_c = max(p_n);
    T_c = max(temp_nr);
    k_c = find((temp_nr == T_c),1,'first');
    time_c = time_r(k_c);
    % fit spline to reactive temperature profile
    [pp, p] = csaps(time_r,temp_r);
    % evaluate derivative
    fprime = fnder(pp,1);
    D = fnval(fprime,time_r);
    D_exp = max(D);
    k_exp = find((D == D_exp),1,'first');
    time_exp = time_r(k_exp);
    p_max_time = time_r(k_max);
    % display results
    display(['Compression ratio ',num2str(CR)]);
    display(['Critical pulse width ',num2str(tau)]);
    display(['Compression peak time (s) ',num2str(time_c)]);
    display(['Compression temperature (K) ',num2str(T_c)]);
    display(['Compression pressure (MPa) ',num2str(P_c/1E6)]);
    display(['Ignition time (s) ',num2str(time_ign)]);
    display(['Ignition temperature (K) ',num2str(expl_temp)]);
    display(['Ignition pressure (MPa) ',num2str(expl_pres/1E6)]);
    display(['Decay rate at ignition time (s) ',num2str(tau_r_expl)]);
    display(['Explosion time (s) ',num2str(time_exp)]);
    display(['Induction time (s) ',num2str(time_exp - time_c)]);
    display(['Maximum pressure (MPa) ',num2str(p_max/1E6)]);
    display(['Maximum temperature (K)',num2str(T_max)]);
    %% plotting
    font ='TimesRoman';
    fontsize = 12;
    fontsmall = 8;
    if (plot_cv)
        set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
        % plot temperature
        figure('Name','temperature' );
        maxx = max(time_r);
        minx = min(time_r);
        maxy = max(temp_r);
        miny = min(temp_r);
        fontsize=12;
        axis([minx maxx miny maxy]);
        hold on
        set(gca,'FontSize',fontsize,'LineWidth',1);
        plot(time_r,temp_r);
        plot(time_n,temp_n);
        xlabel('t (s)','FontSize',fontsize);
        ylabel('T (K)','FontSize',fontsize);
        title(['Cantera  ', mechanism, ' species ', q, ' CR ',num2str(CR),' tau ',num2str(tau)],'FontSize',fontsmall,'Interpreter','none');
        legend('reactive','nonreactive','Location','southeast');
        hold off
        % plot pressure
        figure('Name','pressure' );
        maxx = max(time_r);
        minx = min(time_r);
        maxy = max(p_r/1E6);
        miny = min(p_n/1E6);
        fontsize=12;
        axis([minx maxx miny maxy]);
        hold on
        set(gca,'FontSize',fontsize,'LineWidth',1);
        plot(time_r,p_r/1E6);
        plot(time_n,p_n/1E6);
        xlabel('t (s)','FontSize',fontsize);
        ylabel('P (MPa)','FontSize',fontsize);
        title(['Cantera  ', mechanism, ' species ', q, ' CR ',num2str(CR),' tau ',num2str(tau)],'FontSize',fontsmall,'Interpreter', 'none');
        legend('reactive','nonreactive','Location','northeast');
        hold off
    end
    
    if (plot_species)
        %     if (time_ign > 0.)
        %         minx =time_ign;
        %         maxx = 1.01*p_max_time;
        %     end
        major_species = {'H2', 'O2', 'H2O'};
        minor_species = {'H', 'O', 'OH', 'H2O2', 'HO2'};
        visible = 'On';
        xscale = 'linear';
        % Major species mass fractions as a function of position
        if ~isempty(major_species)
            if strcmp(string(major_species),'All')
                major_species = speciesNames(gas);
            end
            major_k = []; major_labels = {};
            species = speciesNames(gas);
            for i = 1:length(major_species)
                if any(ismember(species,major_species{i}))
                    major_k(end+1) = speciesIndex(gas,major_species(i));
                    major_labels{end+1} = major_species{i};
                end
            end
            if ~isempty(major_k)
                figMaj = figure('Visible',visible);
                set(gca,'linestyleorder',{'-','-.','--',':'},'nextplot','add','XScale',xscale,'yscale','log');
                set(gca,'FontSize',fontsize,'LineWidth',1);
                plot(time_r(:),y_r(major_k,:));
                axis([minx maxx 1.0E-10 1]);
                xlabel('time (s)','FontSize',fontsize);
                ylabel('Species mass fraction','FontSize',fontsize);
                title(['Cantera  ', mechanism, ' species ', q, ' CR ',num2str(CR),' tau ',num2str(tau)],'FontSize',fontsmall,'Interpreter','none');
                legend(major_labels,'Location','southeast');
            end
        end
        
        % Minor species mass fractions as a function of position
        if ~isempty(minor_species)
            minor_k = []; minor_labels = {};
            species = speciesNames(gas);
            for i = 1:length(minor_species)
                if any(ismember(species,minor_species{i}))
                    minor_k(end+1) = speciesIndex(gas,minor_species(i));
                    minor_labels{end+1} = minor_species{i};
                end
            end
            if ~isempty(minor_k)
                figMin = figure('Visible',visible);
                set(gca,'linestyleorder',{'-','-.','--',':'},'nextplot','add','XScale',xscale,'yscale','log');
                set(gca,'FontSize',fontsize,'LineWidth',1);
                plot(time_r(:),y_r(minor_k,:));
                axis([minx maxx 1.0E-10 1]);
                xlabel('time (s)','FontSize',fontsize);
                ylabel('Species mass fraction','FontSize',fontsize);
                title(['Cantera  ', mechanism, ' species ', q, ' CR ',num2str(CR),' tau ',num2str(tau)],'FontSize',fontsmall,'Interpreter','none');
                legend(minor_labels,'Location','southeast');
            end
        end
    end
    if (file_out)
        y = [tau; CR; expl_temp; expl_pres; time_c; time_ign; time_exp-time_c];
        fprintf(fid, '%1.4g,%6.2f,%6.3e,%6.3e,%6.2e,%6.3e,%6.3e\n', y);
    end
end
if (file_out)
    fclose(fid);
end
