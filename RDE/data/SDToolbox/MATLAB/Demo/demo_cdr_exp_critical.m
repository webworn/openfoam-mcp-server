% demo_cdr_exp_critical
% Shock and Detonation Toolbox Demo Program
%
% Generates plots and output files for a constant volume
% explosion simulation with critical decay rate model. Computes critical
% decay rate by sucessive approximation. 
%
% The initial conditions are adiabaically
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
disp('demo_cdr_exp_critical')
%%
close all; clc; clear;
plot_cv = false;  % plot temperature and pressure
plot_species = false;  % plot major and minor species
title_on = false;  % print titles above plots
file_out = true;  % output summary to file
%% chemical composition and mechanism
q = 'H2:2 O2:1';
%mechanism = 'Mevel2015';
mechanism = 'Hong2011';
%mechanism='Keromnes2013';
%mechanism='Burke2012';
%mechanism='sandiego20161214_H2only';
%mechanism='GRI30';
mech = [mechanism '.cti'];
%% initial conditions
P1 = 1.01325E5; T1 = 298.15;
gas = Solution(mech);
set(gas, 'T', T1, 'P', P1, 'X', q);
rho_0 = density(gas);
Y_0 = massFractions(gas);
s_0 = entropy_mass(gas);
if (file_out)
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % CREATE OUTPUT TEXT FILE
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    file_name = ['cdr_exp_critical_' mechanism];
    fn = [file_name, '.txt'];
    d = date;
    fid = fopen(fn, 'w');
    fprintf(fid, '# CRITICAL DECAY RATE CALCULATION FOR CV EXPLOSION\n');
    fprintf(fid, '# Date: %s\n', d);
    fprintf(fid, ['# Mechanism: ' mech '\n']);
    fprintf(fid, ['# Species mole fractions: ' q '\n']);
    fprintf(fid, ['# Initial temperature (K): ',num2str(T1), '\n']);
    fprintf(fid, ['# Initial pressure (MPa): ',num2str(P1/1E6), '\n']);
    fprintf(fid, 'tau(s),CR,T_ign(K),P_ign(MPa),t_ign(s),P_max(MPa),t_max(s)\n');
end
%%   Compress reactants isentropically
tau = 0.0014;
for compression_ratio = [30 31 32]
    set(gas,'Density',compression_ratio*rho_0,'Entropy',s_0,'MoleFractions',q);
    T2 = temperature(gas);
    P2 = pressure(gas);
    rho2 = density(gas);
    cv = cv_mass(gas);
    w = meanMolecularWeight(gas);
    gammaminusone = gasconstant/w/cv;
    %
    disp('Compressed State');
    disp(['   Compression ratio ',num2str(compression_ratio)]);
    disp(['   Pressure ',num2str(P2/1E6),' (MPa)']);
    disp(['   Temperature ',num2str(T2),' (K)']);
    disp(['   Density ',num2str(rho2),' (kg/m3)']);
    %% integration parameters
    t_end = 1.5E-3;    % need to set final time sufficiently long for low temperature cases
    tel = [0 t_end];
    rel_tol = 1e-12;
    abs_tol = 1e-12;
    options = odeset('RelTol',rel_tol,'AbsTol',abs_tol,'Stats','off');
    expl = true;
    %% debug
    figure('Name','temperature' );
    fontsize=12;
    axis([0 t_end 500 4500]);
    hold on
        while(expl  && tau > 1E-8)    % Don't let the loop run idefinitely
          tau = tau*0.98;
          i = i + 1;
        % reactive integration
        set(gas,'Temperature',T2,'Pressure',P2,'MoleFractions',q);
        reac = true;
        y0 = [temperature(gas)
            1
            Y_0];
        mw = molecularWeights(gas);
        out = ode15s(@cvsys_cdr,tel,y0,options,gas,mw,rho2,reac,tau);
        time_r = out.x;
        temp_r = out.y(1,:);
        v_r = out.y(2,:);
        y_r = out.y(3:end,:);
        %% nonreactive integration
        reac = false;
        set(gas,'Temperature',T2,'Pressure',P2,'MoleFractions',q);
        out = ode15s(@cvsys_cdr,tel,y0,options,gas,mw,rho2,reac,tau);
        time_n = out.x;
        temp_n = out.y(1,:);
        v_n = out.y(2,:);
        y_n = out.y(3:end,:);
        %% interpolate nonreactive case to reactive output times
        % find explosion time, temperature and pressure
        temp_nr = interp1(time_n,temp_n,time_r);
        v_nr = interp1(time_n,v_n,time_r);
        temp_diff = (temp_r-temp_nr)./temp_nr;
        np = length(time_r);
        k_s = 1;  % skip first k_s elements that are less than .005
        while (temp_diff(k_s) < .001 && k_s < np-1)
            k_s = k_s + 1;
        end
        if (k_s == np-1 )
            display(['tau = ', num2str(tau),' No explosion detected in ',num2str(t_end), ' s']);
            expl_time = 0.0;
            expl_temp = 0.0;
            expl = false;
        else
            expl_time = interp1(temp_diff(k_s:end),time_r(k_s:end),0.01);
            expl_temp = interp1(time_r(k_s:end),temp_r(k_s:end),expl_time);
        end
        temp_rise_max = max(temp_r-temp_nr);
        if (temp_rise_max < 500)
            display(['tau = ', num2str(tau),' Mild or no reaction, max temperature rise ',num2str(temp_rise_max),' K']);
            expl = false;
        else
            expl = true;
        end
        % end
        if (expl)
            display(['Volume decay time (s) ',num2str(tau)]);
            display(['Ignition temperature (K) ',num2str(expl_temp)]);
            display(['Ignition time (s)',num2str(expl_time)]);
            %% compute pressure
            np = length(time_r);
            p_r = zeros(1,np);
            Tdot_r = zeros(1,np);
            work = zeros(1,np);
            nsp = nSpecies(gas);
            x_r=zeros(nsp,np);
            for k=1:np
                rho_r(k) = rho2/v_r(k);
                set(gas, 'T', temp_r(k), 'Rho', rho_r(k), 'Y', y_r(1:end,k));
                p_r(k) = pressure(gas);
                x_r(:,k) = moleFractions(gas);
                % reconstruct energy terms
                cv = cv_mass(gas);
                w = meanMolecularWeight(gas);
                work(k) =  gasconstant*temp_r(k)/w/cv/tau;
                wdot = netProdRates(gas);
                Tdot_r(k) = - temp_r(k) * gasconstant * (enthalpies_RT(gas) - ones(nsp,1))' ...
                    * wdot / rho_r(k) /cv;
                Tdot_r(k) = Tdot_r(k) + work(k);
            end
            npn = length(time_n);
            p_n = zeros(1,npn);
            for k = 1:npn
                rho_n(k) = rho2/v_n(k);
                set(gas, 'T', temp_n(k), 'Rho', rho_n(k), 'Y', y_n(1:end,k));
                p_n(k) = pressure(gas);
            end
            expl_pres = interp1(time_r(k_s:end),p_r(k_s:end),expl_time);
            [p_max, k_max] = max(p_r);
            p_max_time = time_r(k_max);
            [Tdot_max,tk_max] = max(Tdot_r);
            display(['Ignition pressure (MPa) ',num2str(expl_pres/1E6)]);
            display(['Maximum pressure (MPa) ',num2str(p_max/1E6)]);
            display(['Maximum pressure time (s)',num2str(p_max_time)]);
        else
            display(['NO explosion at volume decay time of ',num2str(tau)])
        end
        %% debug plotting
        set(gca,'FontSize',fontsize,'LineWidth',2);
        plot(time_r,temp_r);
        xlabel('t (s)','FontSize',fontsize);
        ylabel('T (K)','FontSize',fontsize);
        %% plotting
        xunit = 'ms';
        xmult = 1E3;
        maxx = max(xmult*time_r);
        minx = min(xmult*time_r);
        
        if (plot_cv)
            font ='TimesRoman';
            fontsize = 12;
            set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
            % plot temperature
            figure('Name','temperature' );
            maxy = max(temp_r);
            miny = 0;
            fontsize=12;
            axis([minx maxx miny maxy]);
            set(gca,'FontSize',fontsize,'LineWidth',2);
            plot(xmult*time_r,temp_r);
            hold on
            plot(xmult*time_n,temp_n);
            xlabel(['t (',xunit,')'],'FontSize',fontsize);
            ylabel('T (K)','FontSize',fontsize);
            title(['Cantera  ', mechanism, ' species ', q, ' tau ',num2str(tau)],'FontSize',fontsize);
            legend( 'reactive','nonreactive','Location','northeast')
            hold off
            %            plot pressure
            figure('Name','pressure' );
            maxy = max(p_r/1E6);
            miny = 0;
            fontsize=12;
            axis([minx maxx miny maxy]);
            set(gca,'FontSize',fontsize,'LineWidth',2);
            plot(xmult*time_r,p_r/1E6);
            hold on
            plot(xmult*time_n,p_n/1E6);
            xlabel(['t (',xunit,')'],'FontSize',fontsize);
            ylabel('P (MPa)','FontSize',fontsize);
            title(['Cantera  ', mechanism, ' species ', q, ' tau ',num2str(tau)],'FontSize',fontsize);
            legend( 'reactive','nonreactive','Location','northeast')
            hold off
            % plot relative volume
            figure('Name','relative volume' );
            maxy = max(v_r);
            miny = min(min(min(v_n),min(v_nr)));
            fontsize=12;
            axis([minx maxx miny maxy]);
            set(gca,'FontSize',fontsize,'LineWidth',2);
            plot(xmult*time_r,v_r);
            hold on
            %    plot(time_r,v_nr);   %reactive and nonreactive volumes should be the
            %    same!
            xlabel(['t (',xunit,')'],'FontSize',fontsize);
            str_1 = ['$$v/v_0$$'];
            y_lab = ylabel(str_1,'FontSize',fontsize);
            set(y_lab,'Interpreter','latex')
            if (title_on)
                title(['Cantera  ', mechanism, ' species ', q, ' tau ',num2str(tau)],'FontSize',fontsize);
            end
            hold off
            % plot source terms
            figure('Name','source terms' );
            maxy = max(max(work),max(Tdot_r));
            miny = min(min(min(Tdot_r),min(work)));
            fontsize=12;
            axis([minx maxx miny maxy]);
            set(gca,'FontSize',fontsize,'LineWidth',2);
            semilogy(xmult*time_r,work);
            hold on
            semilogy(xmult*time_r,Tdot_r);
            xlabel(['t (',xunit,')'],'FontSize',fontsize);
            str_2 = ['source (s$^-1$)'];
            y_lab = ylabel(str_2,'FontSize',fontsize);
            set(y_lab,'Interpreter','latex')
            title(['Cantera  ', mechanism, ' species ', q, ' tau ',num2str(tau)],'FontSize',fontsize);
            str_3 = ['$dT/dt$'];
            leg = legend( 'work',str_3,'Location','northeast');
            set(leg,'Interpreter','latex');
            hold off
        end
        
        if (plot_species)
            %         if (expl_time > 0.)
            %             minx =expl_time;
            %             maxx = 1.01*p_max_time;
            %         end
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
                    figMaj = figure('Visible',visible,'Name','Major species');
                    set(gca,'linestyleorder',{'-','-.','--',':'},'nextplot','add','XScale',xscale,'yscale','log');
                    plot(xmult*time_r(:),x_r(major_k,:));
                    axis([minx maxx 1.0E-10 1]);
                    xlabel(['t (',xunit,')'],'FontSize',fontsize);
                    ylabel('Species mole fraction','FontSize',fontsize);
                    title(['Cantera  ', mechanism, ' species ', q, ' tau ',num2str(tau)],'FontSize',fontsize);
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
                    figMin = figure('Visible',visible,'Name','Minor species');
                    set(gca,'linestyleorder',{'-','-.','--',':'},'nextplot','add','XScale',xscale,'yscale','log');
                    plot(xmult*time_r(:),x_r(minor_k,:));
                    axis([minx maxx 1.0E-10 1]);
                    xlabel(['t (',xunit,')'],'FontSize',fontsize);
                    ylabel('Species mole fraction','FontSize',fontsize);
                    title(['Cantera  ', mechanism, ' species ', q, ' tau ',num2str(tau)],'FontSize',fontsize);
                    legend(minor_labels,'Location','southeast');
                end
            end
        end
        % if (i == imax)
        %     expl = false;
        % end
    end
    if (file_out)
        y = [tau; compression_ratio; expl_temp; expl_pres/1E6; expl_time; p_max/1E6; p_max_time];
        fprintf(fid, '%6.3e,%6.2f,%6.3e,%6.3e,%6.2e,%6.3e,%6.3e\n', y);
    end
end
if (file_out)
    fclose(fid);
end

function dydt = cvsys_cdr(t,y,gas,mw,rho_0,reac,tau)
% Evaluates the system of ordinary differential equations for an adiabatic,
%     constant-volume, zero-dimensional reactor.
%     It assumes that the 'gas' object represents a reacting ideal gas mixture.
%
%     INPUT:
%         t = time
%         y = solution array [temperature, species mass 1, 2, ...]
%         gas = working gas object
%
%     OUTPUT:
%         An array containing time derivatives of:
%             temperature, volume and species mass fractions,
%         formatted in a way that the integrator in cvsolve can recognize.
%
% normalized volume and volume derivative
Vdot = y(2)/tau;
rho = rho_0/y(2);
% Set the state of the gas, based on the current solution vector and volume
set(gas, 'T', y(1), 'Rho', rho, 'Y', y(3:end));
nsp = nSpecies(gas);
cv = cv_mass(gas);
w = meanMolecularWeight(gas);
work =  -gasconstant*y(1)/(tau*w*cv);
% energy equation
Tdot = 0.;
if (reac)
    wdot = netProdRates(gas);
    h_RT = enthalpies_RT(gas);
    for i = 1:nsp
        Tdot = - y(1)*gasconstant*(h_RT(i)-1.)*wdot(i)/(rho*cv) + Tdot;
    end
end
Tdot = Tdot + work;
% set up column vector for dydt
dydt = [ Tdot
    Vdot
    zeros(nsp,1) ];
% species equations
if (reac)
    for i = 1:nsp
        dydt(i+2,1) = mw(i)*wdot(i)/rho;
    end
end
end