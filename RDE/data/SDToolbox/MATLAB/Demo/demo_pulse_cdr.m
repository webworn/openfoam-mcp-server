%% demo_pulse_cdr.m
%
% Computes adiabatic compression and explosion with a specified volume as a
% function of time.  Set up for Gaussian pulse model with two parameters
%   CR - compression ratio (Vmax/Vmin)
%   tau - pulse width parameter in Gaussian function. 
%
% Requires:
%   f_volume.m defines normalized volume and derivative as a function of time
%   cvsys_v.m to carry out the integration of energy and species equations
%
% Integrates the equations twice, once for nonreactive, once for reactive case. 
% Computes a single case with specified parameters.  Optionally plots
% output for thermodynamic quantities and species.
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
plot_cv = true;
plot_species = true;
%% parameters for f_volume function
CR = 30;
V_min = 1./CR;   % ratio of minimum to maximum volume (1/compression ratio)    
tau = 1.07E-3 ;  %  half-width parameter tau_p (s) for volume function
t_min = 3*tau;   % time (s) of location of minimum volume
t_end = 6*tau;
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
P1 = oneatm; T1 = 298.15;
gas = Solution(mech);
set(gas, 'T', T1, 'P', P1, 'X', q);
rho_0 = density(gas);
Y_0 = massFractions(gas);
%% integration parameters
tel = [0 t_end];
rel_tol = 1e-12;
abs_tol = 1e-12;
%% reactive integration
reac = true;
options = odeset('RelTol',rel_tol,'AbsTol',abs_tol,'Stats','off');
y0 = [temperature(gas)
      Y_0];
mw = molecularWeights(gas);
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
while (temp_diff(k_s) < .01 && k_s < np-1)
    k_s = k_s + 1;
end
if (k_s == np-1 )
    display(['No explosion detected in ',num2str(t_end), ' s']);
    expl_time = 0.0;
    expl_temp = 0.0;
else
expl_time = time_r(k_s);
expl_temp = temp_r(k_s);
end
temp_rise_max = max(temp_r-temp_nr);
if (temp_rise_max < 500)
        display(['Mild or no reaction, max temperature rise ',num2str(temp_rise_max),' K']);
end
vol = zeros(1,np);
rho_r = zeros(1,np);
p_r = zeros(1, np);
p_nr = zeros(1,np);
for k=1:np
    [V, dVdt] = f_volume(time_r(k),tau,V_min,t_min);
    vol(k) = V; dvoldt(k) = dVdt;
    rho_r(k) = rho_0/V;
    set(gas, 'T', temp_r(k), 'Rho', rho_r(k), 'Y', y_r(1:end,k));
    p_r(k) = pressure(gas);
    set(gas, 'T', temp_nr(k), 'Rho', rho_r(k), 'Y', Y_0);
    p_nr(k) = pressure(gas);
end
expl_pres = p_r(k_s);
p_max = max(p_r);
T_max = max(temp_r);
vmin = min(vol);
% fit spline to reactive temperature profile
[pp, p] = csaps(time_r,temp_r);
% evaluate derivative
fprime = fnder(pp,1);
D = fnval(fprime,time_r);
D_exp = max(D);
k_exp = find((D == D_exp),1,'first');
time_exp = time_r(k_exp);
display(['Pulse minimum time (s) ',num2str(t_min)]);
display(['Pulse half width (s) ',num2str(tau)]);
display(['compression ratio ',num2str(1/vmin)]);
display(['Ignition time (s) ',num2str(expl_time)]);
display(['Ignition temperature (K) ',num2str(expl_temp)]);
display(['Ignition pressure (MPa) ',num2str(expl_pres/1E6)]);
display(['Explosion time (s) ',num2str(time_exp)]);
display(['Maximum temperature (Ka) ',num2str(T_max)]);
display(['Maximum pressure (MPa) ',num2str(p_max/1E6)]);
%% plotting
xmult = 1E3;
xunit = 'ms';
    font ='TimesRoman';
    fontsize = 12;
    fontsmall = 8;
if (plot_cv)    
	set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
    figure('Name','temperature' );
    maxx = max(xmult*time_r);
    minx = min(xmult*time_r);
    maxy = max(temp_r);
    miny = 0;
    axis([minx maxx miny maxy]);
    hold on
    box on
    set(gca,'FontSize',fontsize,'LineWidth',1);
    plot(xmult*time_r,temp_r);
    plot(xmult*time_n,temp_n);
    xlabel(['t (' xunit ')'],'FontSize',fontsize);
    ylabel('T (K)','FontSize',fontsize);
    %title(['Cantera  ', mechanism, ' species ', q,' CR ',num2str(CR),' tau ',num2str(tau)],'FontSize',fontsmall,'Interpreter', 'none');
    hold off
     figure('Name','pressure' );
    maxy = max(p_r/1E6);
    maxy = 14;
    miny = 0;
%    maxx = 10.;
    axis([minx maxx miny maxy]);
    hold on
    box on
    set(gca,'FontSize',fontsize,'LineWidth',1);
    plot(xmult*time_r,p_r/1E6);
    plot(xmult*time_r,p_nr/1E6);
    xlabel(['t (' xunit ')'],'FontSize',fontsize);
    ylabel('P (MPa)','FontSize',fontsize);
%    title(['Cantera  ', mechanism, ' species ', q,' CR ',num2str(CR),' tau ',num2str(tau)],'FontSize',fontsmall,'Interpreter', 'none');
    legend('reactive','nonreactive')
    hold off
    %
    figure('Name','volume' );
    maxy = max(vol);
    miny = 0;
    axis([minx maxx miny maxy]);
    hold on
    box on
    set(gca,'FontSize',fontsize,'LineWidth',1);
    plot(xmult*time_r,vol);
     xlabel(['t (' xunit ')'],'FontSize',fontsize);
    ylabel('V/Vo','FontSize',fontsize);
 %   title(['Cantera  ', mechanism, ' species ', q,' CR ',num2str(CR),' tau ',num2str(tau)],'FontSize',fontsmall,'Interpreter', 'none');
    hold off
        %
    figure('Name','volume time derivative' );
    maxy = max(dvoldt);
    miny = min(dvoldt);
    axis([minx maxx miny maxy]);
    hold on
     box on
   set(gca,'FontSize',fontsize,'LineWidth',1);
    plot(xmult*time_r,dvoldt);
     xlabel(['t (' xunit ')'],'FontSize',fontsize);
   ylabel('d(V/Vo)/dt (s)','FontSize',fontsize);
%    title(['Cantera  ', mechanism, ' species ', q,' CR ',num2str(CR),' tau ',num2str(tau)],'FontSize',fontsmall,'Interpreter', 'none');
    hold off
    
    figure('Name','time scale v/dvdt' );
    tau_v = vol./dvoldt;
    %maxy = max(tau_v);
    %miny = min(tau_v);
    maxy = 2*tau/sqrt(CR);
    miny = -2*tau/sqrt(CR);
    axis([minx maxx miny maxy]);
    hold on
     box on
   set(gca,'FontSize',fontsize,'LineWidth',1);
    plot(xmult*time_r,tau_v);
     xlabel(['t (' xunit ')'],'FontSize',fontsize);
   ylabel('V/(dV/dt)','FontSize',fontsize);
%    title(['Cantera  ', mechanism, ' species ', q,' CR ',num2str(CR),' tau ',num2str(tau)],'FontSize',fontsmall,'Interpreter', 'none');
    hold off
end

if (plot_species)
    if (expl_time > 0.)
    %minx =expl_time;
    %maxx = 1.01*p_max_time;
    end
     major_species = {'H2', 'O2', 'N2', 'H2O'};
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
            plot(xmult*time_r(:),y_r(major_k,:));
            axis([minx maxx 1.0E-10 1]);
             xlabel(['t (' xunit ')'],'FontSize',fontsize);
           ylabel('Species mass fraction','FontSize',fontsize);
  %          title(['Cantera  ', mechanism, ' species ', q,' CR ',num2str(CR),' tau ',num2str(tau)],'FontSize',fontsmall,'Interpreter', 'none');
            legend(major_labels,'Location','southeast');
                  box on
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
            plot(xmult*time_r(:),y_r(minor_k,:));
            axis([minx maxx 1.0E-10 1]);
            xlabel(['t (' xunit ')'],'FontSize',fontsize);
           ylabel('Species mass fraction','FontSize',fontsize);
 %           title(['Cantera  ', mechanism, ' species ', q,' CR ',num2str(CR),' tau ',num2str(tau)],'FontSize',fontsmall,'Interpreter', 'none');
            legend(minor_labels,'Location','southeast');
                  box on
        end
    end

end

