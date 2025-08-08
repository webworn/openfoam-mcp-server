% demo_cdr_exp
% Shock and Detonation Toolbox Demo Program
%
% Generates plots and output files for a constant volume
% explosion simulation with critical decay rate model using exponential
% increase in volume following instanteous isentropic compression. 
% 
% The time dependence of species, pressure, and
% temperature are computed using the user supplied reaction mechanism file.
%
% ################################################################################
% Theory, numerical methods and applications are described in the following report:
%
% IGNITION MODELING
% AND THE
% CRITICAL DECAY RATE CONCEPT
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
disp('demo_cdr_exp');
clear;clc;close all;
plot_t = true;
disp('demo_cdr_exp')
q = 'H2:2 O2:1';
mechanism = 'Hong2011';
mech = [mechanism '.cti'];
P0 = 1.01325E5; T0 = 298.15;
gas = Solution(mech);
set(gas, 'T', T0, 'P', P0, 'X', q);
mw = molecularWeights(gas);
rho0 = density(gas);
s0 = entropy_mass(gas);
nsp = nSpecies(gas);
y_mass = massFractions(gas);
tau = 0.003;
for compression_ratio  = [30]
    set(gas,'Density',compression_ratio*rho0,'Entropy',s0,'MoleFractions',q);
    T1 = temperature(gas);
    P1 = pressure(gas);
    rho1 = density(gas);
    t_end = 0.0015;
    tel = [0 t_end];
    rel_tol = 1E-12;
    abs_tol = 1E-12;
    if (plot_t)
        t_unit = 'Time (ms)';
        t_mult = 1000.;
        font ='TimesRoman';
        fontsize = 12;
        set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
        figure('Name','temperature' );
        xlabel(t_unit,'FontSize',fontsize);
        ylabel('Temperature (K)','FontSize',fontsize);
        maxx = t_end*t_mult;
        minx = 0;
        maxy = 4500;
        miny = 500;
        fontsize=12;
        axis([minx maxx miny maxy]);
        box on
        hold on
    end
    taui = [10000.,0.00112820,0.00101420,0.00099528,0.00099262,0.00099200,0.0009915];
    imax = length(taui);
    for i=1:imax
        tau = taui(i);
        reac  = true;
        %for tau  = 10000.   % for checking induction time
        %while (reac)
        %        tau = tau*0.99;
        set(gas, 'T', T1, 'P', P1, 'X', q);
        y0 = [T1
            1
            y_mass];
        options = odeset('RelTol',rel_tol,'AbsTol',abs_tol,'Stats','off');
        out = ode15s(@f,tel,y0,options,gas,mw,rho1,tau);
        steps = length(out.y);
        yd = zeros(nsp+2,steps);
        for k=1:steps
            yd(:,k)  = f(1,out.y(:,k),gas,mw,rho1,tau);
        end
        if (max(yd(1,1:steps)) > 0)
            reac = true;
            [ydmax kdmax] = max(yd(1,1:steps));
            time_max = out.x(kdmax);
            display(['tau ',num2str(tau),' induction time ',num2str(time_max)]);
            tau_last = tau;
        else
            reac = false;
            display(['tau ',num2str(tau),' no reaction']);
        end
        if (plot_t)
            plot(out.x*t_mult,out.y(1,:))
        end
    end
    display([' CR ',num2str(compression_ratio),' tau* ',num2str(0.5*(tau+tau_last))]);
    if (plot_t)
        hold off
    end
end
plot_species(gas,out,t_end);

function dydt = f(t,y,gas,mw,rho_0,tau)
rho = rho_0/y(2);
set(gas, 'T', y(1), 'Rho', rho, 'Y', y(3:end));
nsp = nSpecies(gas);
wdot = netProdRates(gas);
h_RT = enthalpies_RT(gas);
cv = cv_mass(gas);
w = meanMolecularWeight(gas);
Tdot  = 0.;
for i = 1:nsp
    Tdot = - y(1)*gasconstant*(h_RT(i)-1.)*wdot(i)/(rho*cv) + Tdot;
end
Tdot = Tdot - gasconstant*y(1)/(w*tau*cv);
for i = 1:nsp
    ydt(i) = mw(i)*wdot(i)/rho;
end
Vdot = y(2)/tau;
dydt = [ Tdot
    Vdot
    ydt' ];
end

function plot_species(gas,out,t_end)
t_mult = 1000.;
% font ='TimesRoman';
% fontsize = 12;
% set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
maxx = t_end*t_mult;
minx = 0;
fontsize=12;
xunit ='ms';
major_species = {'H2', 'O2', 'H2O'};
minor_species = {'H', 'O', 'OH', 'H2O2', 'HO2'};
visible = 'On';
xscale = 'linear';
% create mole fractions
nsp = nSpecies(gas);
len = length(out.x);
mw = molecularWeights(gas);
speciesX = zeros(nsp,len);
a = zeros(len);
for k=1:len
    a(k) = 0.;
    for i=1:nsp
        speciesX(i,k) = out.y(i+2,k)/mw(i);
        a(k) = a(k) + speciesX(i,k);
    end
    for i=1:nsp
        speciesX(i,k) = speciesX(i,k)/a(k);
    end
end
% Major species mole fractions as a function of position
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
        %       plot(t_mult*out.x(:),out.y(major_k+2,:));
        plot(t_mult*out.x(:),speciesX(major_k,:));
        axis([minx maxx 1.0E-10 1]);
        box on
        xlabel(['t (',xunit,')'],'FontSize',fontsize);
        ylabel('Species mole fraction','FontSize',fontsize);
        %       ylabel('Species mass fraction','FontSize',fontsize);
        %       title(['Cantera  ', mechanism, ' species ', q, ' tau ',num2str(tau)],'FontSize',fontsize);
        legend(major_labels,'Location','southeast');
    end
end

% Minor species mole fractions as a function of position
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
        box on
        %       plot(t_mult*out.x(:),out.y(minor_k+2,:));
        plot(t_mult*out.x(:),speciesX(minor_k,:));
        axis([minx maxx 1.0E-10 1]);
        xlabel(['t (',xunit,')'],'FontSize',fontsize);
        %        ylabel('Species mass fraction','FontSize',fontsize);
        ylabel('Species mole fraction','FontSize',fontsize);
        %        title(['Cantera  ', mechanism, ' species ', q, ' tau ',num2str(tau)],'FontSize',fontsize);
        legend(minor_labels,'Location','southeast');
    end
end
end

