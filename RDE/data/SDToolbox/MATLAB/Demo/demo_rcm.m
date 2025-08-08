%% demo_rcm.m
% Shock and Detonation Toolbox Demo Program
%
% computes adiabatic compression and explosion with rcm volume-time file
% input.  Requires defining a function rcm_volume(t) and uses the function
% rcmsys.m to carry out the integration.  Integrates the equations twice,
% once for nonreactive, once for reactive case.  
%
% Requires:
%   rcm_volume.m defines normalized volume and derivative as a function of time
%   rcmsys.m to carry out the integration of energy and species equations
%   An asci file with  time, V, DVDT  
%
% This program uses an example set downloaded from the SUNG lab website at
% University of Connecticut.  https://combdiaglab.engr.uconn.edu/database/rcm-database/
% The background of the modeling and results of processing example data are
% provided in the report
%
% IGNITION MODELING AND THE  CRITICAL DECAY RATE CONCEPT
%
% J. E. SHEPHERD
% GRADUATE AEROSPACE LABORATORIES
% CALIFORNIA INSTITUTE OF TECHNOLOGY
% PASADENA, CA 91125
% GALCIT REPORT EDL2019.002  February 2020
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
plot_data = true;
plot_species = true;
% set initial conditions
q = 'H2:12.5 O2:6.25 N2:81.25';
P1 =2.3039*oneatm; T1 = 400.95;
% read in data files  - these example files were downloaded from 
% https://combdiaglab.engr.uconn.edu/database/rcm-database/moist-h2/
%
% The details of the RCM experiments, the presentation and interpretation
% of the data are given in:
%
% Apurba  K.  Das,  Chih-Jen  Sung,  Yu  Zhang,  and  Gaurav  Mittal.   
% Ignition  delay  study  of  moist  hydrogen/oxidizermixtures using a rapid compression machine. 
% International Journal of Hydrogen Energy, 37(8):6901–6911, April2012. 
% The data need to be downloaded from the UConn website maintaining the directory
% organization from that site.    Rsults of validation of various reaction mechanisms
% using these data sets is given in:
% 
filename = '0_H2O/70bar/400K/IG_1.33/vtim';
fileID = fopen(filename,'r');
% The following format is for a text file with three columns t V(t) and
% dVdt separated by spaces. 
FormatString = ['%f' '%f' '%f'];   
A = textscan(fileID,FormatString);
Z = cell2mat(A);
% report on file contents
npoints = length(Z);
k_min = find((Z(:,2) == min(Z(2:end,2))),1);
t_min = Z(k_min,1);
t_max = max(Z(:,1));
v_min = Z(k_min,2);
v_max = max(Z(:,2));
CR_orig = v_max/v_min;
v_orig = Z(:,2);   % save for later comparison
dvdt_orig = Z(:,3);
display(['file   ', filename]);
display(['length ',num2str(npoints)]);
display(['duration (s) ',num2str(t_max)]);
display(['volume minimum  time (s) ',num2str(t_min)]);
display(['maximum volume  ',num2str(v_max)]);
display(['minimum volume  ',num2str(v_min)]);
display(['compression ratio (raw) ',num2str(CR_orig)]);
% smooth volume data, smoothing parameters need to be chosen carefully and
% the initial volume adjusted to maintain the experimental compression
% ratio and reasonable fidelity to the original data. 
[B,window] = smoothdata(Z(:,2),'gaussian',100);
Z(:,2) = B;
Z(1,2) = CR_orig*min(Z(:,2));
CR_smooth = Z(1,2)/min(Z(2:end,2));
% fit spline
[pp, p] = csaps(Z(:,1),B);
% evaluate derivative
fprime = fnder(pp,1);
C = fnval(fprime,Z(:,1));
%smooth derivative
Z(:,3) = smoothdata(C,'gaussian',300);
%% chemical  mechanism  
%mechanism = 'Mevel2015';
mechanism = 'Hong2011';
%mechanism='Keromnes2013';
%mechanism='Burke2012';
%mechanism='sandiego20161214_H2only';
%mechanism='GRI30';
mech = [mechanism '.cti'];
%% set initial state
gas = Solution(mech);
set(gas, 'T', T1, 'P', P1, 'X', q);
rho_0 = density(gas);
s_0 = entropy_mass(gas);
Y_0 = massFractions(gas);
% compute ideal compressed state
set(gas,'Density',CR_orig*rho_0,'Entropy',s_0,'MoleFractions',q);
Pc_ideal = pressure(gas); Tc_ideal = temperature(gas);
set(gas, 'T', T1, 'P', P1, 'X', q);
%% integration parameters  - be careful with setting rtol too low if 
% volume and derivative data are too noisy.
t_end = 5*t_min;    
tel = [0 t_end];
rel_tol = 1e-5;
abs_tol = 1e-8;
%% reactive integration
reac = true;
options = odeset('RelTol',rel_tol,'AbsTol',abs_tol,'Stats','off');
y0 = [temperature(gas)
      Y_0];
mw = molecularWeights(gas);
out = ode15s(@rcmsys,tel,y0,options,gas,mw,rho_0,reac,Z);
time_r = out.x;
temp_r = out.y(1,:);
y_r = out.y(2:end,:);
%% nonreactive integration
reac = false;
set(gas, 'T', T1, 'P', P1, 'X', q);
out = ode15s(@rcmsys,tel,y0,options,gas,mw,rho_0,reac,Z);
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
%expl_time = interp1(temp_diff(k_s:end),time_r(k_s:end),0.01);
expl_time = time_r(k_s);
%expl_temp = interp1(time_r(k_s:end),temp_r(k_s:end),expl_time); 
expl_temp = temp_r(k_s);
end
temp_rise_max = max(temp_r-temp_nr);
if (temp_rise_max < 500)
        display(['Mild or no reaction, max temperature rise ',num2str(temp_rise_max),' K']);
end
for k=1:np
    [V, dVdt] = rcm_volume(time_r(k),Z);
    rho_r(k) = rho_0/V;
    set(gas, 'T', temp_r(k), 'Rho', rho_r(k), 'Y', y_r(1:end,k));
    p_r(k) = pressure(gas);
    set(gas, 'T', temp_nr(k), 'Rho', rho_r(k), 'Y', Y_0);
    p_nr(k) = pressure(gas);
end
% expl_pres = interp1(time_r(k_s:end),p_r(k_s:end),expl_time);
expl_pres = p_r(k_s);
p_max = max(p_r);
%p_max_time = interp1(p_r(k_s:end),time_r(k_s:end),p_max);
P_c = max(p_nr);
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
t_exp = time_r(k_exp);
display(['Initial pressure (MPa) ',num2str(P1/1E6)]);
display(['Initial temperature (K) ',num2str(T1)]);
display(['Peak compression ratio (smooth) ',num2str(CR_smooth)]);
display(['Ideal peak pressure (MPa) ',num2str(Pc_ideal/1E6)]);
display(['Ideal peak temperature (K) ',num2str(Tc_ideal)]);
display(['Compression peak time (s) ',num2str(time_c)]);
display(['Compression pressure P_c (MPa) ',num2str(P_c/1E6)]);
display(['Compression temperature T_c (K) ',num2str(T_c)]);
display(['Explosion time (s) ',num2str(t_exp)]);
display(['Induction time (s) ',num2str(t_exp-time_c)]);
display(['Ignition time (s) ',num2str(expl_time)]);
display(['Ignition temperature (K) ',num2str(expl_temp)]);
display(['Ignition pressure (MPa) ',num2str(expl_pres/1E6)]);
display(['Maximum pressure (MPa) ',num2str(p_max/1E6)]);
%% plotting
font ='TimesRoman';
fontsize = 12;
fontsmall = 8;
if (plot_cv)    
	set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
    figure('Name','temperature' );
    maxx = 0.050;
%    maxx = max(time_r);
    minx = min(time_r);
    maxy = 1.1*max(temp_r);
    %miny = min(temp_r);
    miny = 0.;
    axis([minx maxx miny maxy]);
    box on
    hold on
    set(gca,'FontSize',fontsize,'LineWidth',1);
    plot(time_r,temp_r,'k');
    plot(time_n,temp_n,'--k');
    xlabel('t (s)','FontSize',fontsize);
    ylabel('T (K)','FontSize',fontsize);
%    title(['Cantera  ', mechanism, ' species ', q,' P1 ',num2str(P1/1E6),' T1 ',num2str(T1)],'FontSize',fontsmall,'Interpreter', 'none');
    legend('reactive','nonreactive','Location','southeast');
    hold off
    figure('Name','pressure' );
%    maxx = max(time_r);
    minx = min(time_r);
    maxy = 1.1*max(p_r/1E6);
    %miny = min(p_nr/1E6);
    miny = 0.;
    axis([minx maxx miny maxy]);
    box on
    hold on
    set(gca,'FontSize',fontsize,'LineWidth',1);
    plot(time_r,p_r/1E6,'k');
    plot(time_r,p_nr/1E6,'--k');
    xlabel('t (s)','FontSize',fontsize);
    ylabel('P (MPa)','FontSize',fontsize);
%    title(['Cantera  ', mechanism, ' species ', q,' P1 ',num2str(P1/1E6),' T1 ',num2str(T1)],'FontSize',fontsmall,'Interpreter', 'none');
    legend('reactive','nonreactive','Location','southeast');
    hold off
end
if (plot_data)
    %
    figure('Name','volume' );
%    maxx = 5*t_min;
    minx = 0.0;
    maxy = max(v_orig);
    miny = 0.0;
    fontsize=12;
    box on
    hold on
    axis([minx maxx miny maxy]);
    set(gca,'FontSize',fontsize,'LineWidth',1);
    plot(Z(1:50:end,1),v_orig(1:50:end),'.k');
    plot(Z(:,1),Z(:,2),'k');
    xlabel('t (s)','FontSize',fontsize);
    ylabel('V ','FontSize',fontsize);
 %   title([' Volume data file  ', filename],'FontSize',fontsmall,'Interpreter', 'none');
    legend('original','smoothed','Location','northeast');
    hold off
%
    figure('Name','volume derivative' );
%    maxx = 5*t_min;
    minx = 0.0;
    maxy = max(dvdt_orig);
    miny = min(dvdt_orig);
    fontsize=12;
    box on
    hold on
    axis([minx maxx miny maxy]);
    set(gca,'FontSize',fontsize,'LineWidth',1);
    plot(Z(:,1),dvdt_orig,'color',[.6 .6 .6]);
    plot(Z(:,1),Z(:,3),'k');
    xlabel('t (s)','FontSize',fontsize);
    ylabel('dVdt ','FontSize',fontsize);
%    title([' Volume data file  ', filename],'FontSize',fontsmall,'Interpreter', 'none');
    legend('original','smoothed','Location','southeast');
    hold off
end

if (plot_species)
    if (t_exp > 0.)
    minx =0;
    maxx = 1.05*t_exp;
    end
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
            box on
            plot(time_r(:),y_r(major_k,:));
            axis([minx maxx 1.0E-10 1]);
            xlabel('t (s)','FontSize',fontsize);
            ylabel('Species mass fraction','FontSize',fontsize);
%            title(['Cantera  ', mechanism, ' species ', q,' P1 ',num2str(P1/1E6),' T1 ',num2str(T1)],'FontSize',fontsmall,'Interpreter', 'none');
            legend(major_labels,'Location','southwest');
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
            box on
            plot(time_r(:),y_r(minor_k,:));
            axis([minx maxx 1.0E-10 1]);
            xlabel('t (s)','FontSize',fontsize);
            ylabel('Species mass fraction','FontSize',fontsize);
 %           title(['Cantera  ', mechanism, ' species ', q,' P1 ',num2str(P1/1E6),' T1 ',num2str(T1)],'FontSize',fontsmall,'Interpreter', 'none');
            legend(minor_labels,'Location','northwest');
        end
    end

end

