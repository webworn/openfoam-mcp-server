% Shock and Detonation Toolbox Demo Program
% 
% Computation of reactant and product (HP, CJ) enthalpies, fit to linear functions
% Compute .cti file coefficients, adjust values to compensate for average
% molar mass, and matching HP temperature and CJ speed.  Test one-step and
% two-gamma .cti file models
%
% Requires creating appropriate .cti files and setting case switch
% 'cti_file' to treat case of interest, editing species list to suit. 
% This version is set to use stoichiometric pentane-air as the reactants
% and example .cti files are given on the SDT website
%  
% ################################################################################
% Theory, numerical methods and applications are described in the following report:
% 
% SDToolbox Numerical Tools for Shock and Detonation Wave Modeling, Explosion 
% Dynamics Laboratory, Contributors: S. Browne, J. Ziegler, N. Bitter, B. Schmidt, 
% J. Lawson and J. E. Shepherd, GALCIT Technical Report FM2018.001 Revised January 2021.
% 
% Please cite this report and the website if you use these routines. 
% 
% Please refer to LICENCE.txt or the above report for copyright and disclaimers.
% 
% http://shepherd.caltech.edu/EDL/PublicResources/sdt/
% 
% ################################################################################ 
% Updated Feb 2021
%%
clear; clc; close all;
disp('Properties and Enthalpy fits for reduced thermodynamic models ');
%% initial gas state
P1 = 1.0*oneatm; T1 = 298.15;
%% size of arrays
k_max  = 27;  % temperature fits
j_max = 30;   % isentrope fit
miny = -5;   % enthalpy plots 
maxy = 5;    % enthalpy plots
%% cases
cti_file = 'full';
switch cti_file
    case 'one_step'
        % one step with realistc but limited thermodynamics
        mech = 'pentane_one_step.cti';
        q = 'NC5H12:1.  O2:8  N2:30.08';    %pentane-air
        plot_species={'H2O','CO2', 'NC5H12'  'O2'  'N2'};
        curve_fit = false;
    case 'full'
        %pentane full chemistry
        mech = ['pentane_thermo.cti'];
        gas  = Solution(mech);
        q = 'NC5H12:1.  O2:8  N2:30.08';    %pentane-air
        plot_species = {'H2O','CO2','CO','H2','H','O','OH'};
        %q = 'C3H8:1. O2:5 N2:5';    %pentane-air
        curve_fit = true;
    case 'two-gamma_lo'
        %low temperature fit
        mech = 'pentane_two_gamma.cti';
        gas  = Solution(mech);  % constant pressure thermo
        q = 'A:1.0 B:0.0';
        plot_species={'A','B'}; mech = 'pentane_two_gamma.cti';
        curve_fit = false;
    case 'two-gamma_hi'
        % high temperature fit
        mech = 'pentane_two_gamma_CJ.cti';
        gas  = Solution(mech); %  For CJ thermo
        q = 'A:1.0 B:0.0';
        plot_species={'A','B'};
        curve_fit = false;
end
%%
nsp = nSpecies(gas);
set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
a1_fr =soundspeed_fr(gas);
gas = equilibrate(gas, 'HP'); 
T_HP = temperature(gas);
disp(['constant pressure temperature (K) ', num2str(T_HP)]);
set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
equilibrate(gas, 'UV');
disp(['constant volume temperature (K) ', num2str(temperature(gas))]); 
disp(['constant volume pressure (MPa) ', num2str(pressure(gas)/1E6)]); 
[cj_speed] = CJspeed(P1, T1, q, mech);
disp(['CJ speed (m/s) ', num2str(cj_speed)]);
MCJ = cj_speed/a1_fr;
disp(['CJ Mach number ', num2str(MCJ)]);
[gas] = PostShock_eq(cj_speed, P1, T1, q, mech);
TCJ = temperature(gas);
h_CJ = enthalpy_mass(gas);
PCJ = pressure(gas);
SCJ = entropy_mass(gas);
VCJ = 1./density(gas);
w_CJ = meanMolecularWeight(gas);
disp(['CJ temperature (K) ', num2str(TCJ)]);
disp(['CJ pressure (MPa) ', num2str(PCJ/1E6)]); 
[gas] = PostShock_fr(cj_speed, P1, T1, q, mech);
disp(['vN temperature (K) ', num2str(temperature(gas))]);
disp(['vN pressure (MPa) ', num2str(pressure(gas)/1E6)]); 

%%
T0 = 300.;
for k=1:1:k_max
    T(k) = T0;
    set(gas,'Temperature',T0,'Pressure',P1,'MoleFractions',q);
    h_1(k) = enthalpy_mass(gas);
    cp_1(k) = cp_mass(gas);
    gas = equilibrate(gas, 'TP'); 
    h_2(k) = enthalpy_mass(gas);
    cp_2(k) = cp_mass(gas);
    Y(k,:) = massFractions(gas);
    T0 = T0 + 100.;
end
%% 
fontsize = 12;
font ='TimesRoman';
set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
maxx = max(T);
minx = min(T);
fontsize=12;
plot_k = []; plot_labels = {};
species = speciesNames(gas);
for i = 1:length(plot_species)
    if any(ismember(species,plot_species{i}))
        plot_k(end+1) = speciesIndex(gas,plot_species(i));
        plot_labels{end+1} = plot_species{i};
    end
end
%%
figure('Name','Equilibrium Composition')
set(gca,'FontSize',fontsize,'LineWidth',1,...
    'linestyleorder',{'-','-.','--',':'},...
    'nextplot','add');
set(gca, 'Yscale', 'log');
set(gca, 'Xscale', 'linear');
axis([minx maxx 1.0E-7 1]);
plot(T(:),Y(:,plot_k));
xlabel('Temperature (K)','FontSize',fontsize);
ylabel('species mass fraction','FontSize',fontsize);
legend(plot_labels,'Location','west','Fontsize',8);
%%
P1 = 10*oneatm;
T0 = 300.;
for k=1:1:k_max
    T(k) = T0;
    set(gas,'Temperature',T0,'Pressure',P1,'MoleFractions',q);
    h_1(k) = enthalpy_mass(gas);
    gas = equilibrate(gas, 'TP'); 
    h_3(k) = enthalpy_mass(gas);
    Y(k,:) = massFractions(gas);
    T0 = T0 + 100.;
end
%
P1 = 100*oneatm;
T0 = 300.;
for k=1:1:k_max
    T(k) = T0;
    set(gas,'Temperature',T0,'Pressure',P1,'MoleFractions',q);
    h_1(k) = enthalpy_mass(gas);
    gas = equilibrate(gas, 'TP'); 
    h_4(k) = enthalpy_mass(gas);
    Y(k,:) = massFractions(gas);
    T0 = T0 + 100.;
end
%%
figure('Name', 'Enthalpy')
set(gca,'FontSize',fontsize,'LineWidth',1,...
    'linestyleorder',{'-','-.','--',':'},...
    'nextplot','add');
set(gca, 'Yscale', 'linear');
set(gca, 'Xscale', 'linear');
axis([minx maxx miny maxy]);
plot(T(:),h_1(:)/1E6);
hold on
plot(T,h_2/1E6)
plot(T,h_3/1E6)
plot(T,h_4/1E6)
xlabel('Temperature (K)','FontSize',fontsize);
ylabel('Enthalpy (MJ/kg)','FontSize',fontsize);
legend({'Reactants','Products (1 atm)','Products (10 atm)','Products (100 atm)'},'Location','southeast','Fontsize',8);
hold off
%%
figure('Name', 'Specific heat Cp')
set(gca,'FontSize',fontsize,'LineWidth',1,...
    'linestyleorder',{'-','-.','--',':'},...
    'nextplot','add');
set(gca, 'Yscale', 'linear');
set(gca, 'Xscale', 'linear');
axis([minx maxx 0 3000.0;]);
plot(T(:),cp_1(:));
hold on
plot(T,cp_2)
%plot(T,h_3/1E6)
%plot(T,h_4/1E6)
xlabel('Temperature (K)','FontSize',fontsize);
ylabel('Specific heat C_p (J/kg/K)','FontSize',fontsize);
legend({'Reactants','Products (1 atm)'},'Location','southeast','Fontsize',8);
hold off
if (curve_fit)
    %% generate and fit enthalpy to linear relationship to find .cti coefficients
    P1 = 1*oneatm;
    T0 = 300.;
    set(gas,'Temperature',T0,'Pressure',P1,'MoleFractions',q);
    w_r = meanMolecularWeight(gas);
    gas = equilibrate(gas, 'TP');
    w_p = meanMolecularWeight(gas);
    w_bar = (w_r + w_p)/2;
    disp(['average molar mass ',num2str(w_bar)]);
    
    T0 = 300.;
    for k=1:1:k_max
        x(k) = T0;
        set(gas,'Temperature',T0,'Pressure',P1,'MoleFractions',q);
        y_r(k) = enthalpy_mass(gas);
        gas = equilibrate(gas, 'TP');
        y_p(k) = enthalpy_mass(gas);
        T0 = T0 + 100.;
    end
    %% fit reactant enthalpy
    myFit = LinearModel.fit(x,y_r);
    figure('Name','Reactant enthalpy')
    plot(myFit);
    xlabel('Temperature (K)','FontSize',fontsize);
    ylabel('Enthalpy (J/kg)','FontSize',fontsize);
    title('');
    intercept = myFit.Coefficients{1,1};
    slope = myFit.Coefficients{2,1};
    fprintf('Reactant slope and intercept %11.5e %11.5e\n',slope,intercept);
    R_r = gasconstant/w_r;
    gamma_r = slope/(slope - R_r);
    fprintf('Reactant molar mass, gas constant, effective gamma %11.5e %11.5e %11.5e\n',w_r,R_r,gamma_r);
    fprintf('Reactant cti coefficients a0, a5 %16.12e %16.12e\n',slope/R_r,intercept/R_r);
    fprintf('values based on average molar mass\n');
    % adjust values of constan to use average molar mass
    R_bar = gasconstant/w_bar;
    gamma_r = slope/(slope - R_bar);
    a5_r = intercept/R_bar;
    fprintf('Reactant average molar mass, gas constant, effective gamma %11.5e %11.5e %11.5e\n',w_bar,R_bar,gamma_r);
    fprintf('Reactant average molar mass cti coefficients a0, a5 %16.12e %16.12e\n\n',slope/R_bar,intercept/R_bar);
    %% fit HP product enthalpy vs temperature
    myFit = LinearModel.fit(x,y_p);
    figure('Name','HP Product enthalpy')
    plot(myFit);
    xlabel('Temperature (K)','FontSize',fontsize);
    ylabel('Enthalpy (J/kg)','FontSize',fontsize);
    title('');
    intercept = myFit.Coefficients{1,1};
    slope = myFit.Coefficients{2,1};
    h_hpfit = slope*T+intercept;
    fprintf('HP Product slope and intercept %11.5e %11.5e\n',slope,intercept);
    R_p = gasconstant/w_p;
    gamma_p = slope/(slope - R_p);
    fprintf('values based on actual molar mass\n');
    fprintf('HP Product molar mass, gas constant, effective gamma %11.5e %11.5e %11.5e\n',w_p,R_p,gamma_p);
    fprintf('HP Product cti coefficients a0, a5 %16.12e %16.12e\n',slope/R_p,intercept/R_p);
    fprintf('values based on average molar mass\n');
    % adjust values to use average molar mass
    R_bar = gasconstant/w_bar;
    gamma_p = slope/(slope - R_bar);
    CppRbar = slope/R_bar;
    fprintf('HP Product average molar mass, gas constant, effective gamma %11.5e %11.5e %11.5e\n',w_bar,R_bar,gamma_p);
    fprintf('HP Product average molar mass cti coefficients a0, a5 %16.12e %16.12e\n',slope/R_bar,intercept/R_bar);
    % adjust value of product a5 to  match HP temperature
    a5_p = intercept/R_bar + slope*T1/R_bar - CppRbar*T_HP;
    fprintf('Adjusted (match HP state) average molar mass product cti coefficient a5  %16.12e\n\n',a5_p);
    %
    %%
    % fit product isentrope from CJ point
    V0 = VCJ;
    [gas] = PostShock_eq(cj_speed,P1, T1, q, mech);
    for j=1:1:j_max
        sv = [SCJ, V0];
        setState_SV(gas,sv);
        equilibrate(gas,'SV');
        T_CJ(j) = temperature(gas);
        h_cj(j) = enthalpy_mass(gas);
        V0 = V0*1.1;
    end
    % fit enthalpy vs temperature along CJ isentrope
    myFit = LinearModel.fit(T_CJ,h_cj);
    figure('Name','CJ Product enthalpy')
    plot(myFit);
    xlabel('Temperature (K)','FontSize',fontsize);
    ylabel('Enthalpy (J/kg)','FontSize',fontsize);
    title('');
    intercept = myFit.Coefficients{1,1};
    slope = myFit.Coefficients{2,1};
    fprintf('CJ Product slope and intercept %11.5e %11.5e\n',slope,intercept);
    R_CJ = gasconstant/w_CJ;
    gamma_CJ = slope/(slope - R_CJ);
    fprintf('values based on actual CJ molar mass\n');
    fprintf('Product CJ molar mass, gas constant, effective gamma %11.5e %11.5e %11.5e\n',w_CJ,R_CJ,gamma_CJ);
    fprintf('Product CJ cti coefficients a0, a5 %16.12e %16.12e\n',slope/R_CJ,intercept/R_CJ);
    fprintf('values based on average molar mass\n');
    gamma_pCJ = slope/(slope - R_bar);
    fprintf('CJ isentrope (average molar mass) effective gamma %11.5e\n', gamma_pCJ);
    fprintf('CJ isentrope (average molar mass) cti coefficients a0, a5 %16.12e %16.12e\n\n',slope/R_bar,intercept/R_bar);
    %h_cjfit = slope*T+intercept;
    %%
    % match computed CJ Mach number
    gamma2_eq = gamma_pCJ;
    gamma1_fr = gamma_r;
    M1 =  MCJ;  % CJ Mach number
    e = M1^(-2)*(gamma2_eq/gamma1_fr)^2*(1+gamma1_fr*M1*M1)^2/(2*(gamma2_eq*gamma2_eq-1))-1/(gamma1_fr-1)-M1*M1/2;
    disp(['Nondimensional 2-gamma energy release q/a1^2 ',num2str(e)])
    a0_pCJ = gamma2_eq/(gamma2_eq-1);
    a5_pCJ = a5_r - gamma1_fr*T1*e;
    h_cjfit = R_bar*(a0_pCJ*T+a5_pCJ);
    fprintf('Adjusted (match CJ,average molar mass) product cti coefficients a0, a5 %16.12e %16.12e\n',a0_pCJ,a5_pCJ);
    %
    %% comparison plot
    figure('Name', 'Enthalpy, Detonation Products, Shocked Reactants')
    set(gca,'FontSize',fontsize,'LineWidth',1,...
        'linestyleorder',{'-','-.','--',':'},...
        'nextplot','add');
    set(gca, 'Yscale', 'linear');
    set(gca, 'Xscale', 'linear');
    axis([minx maxx miny maxy]);
    plot(T,h_1/1E6);
    hold on
    plot(T_CJ,h_cj/1E6);
    plot(T,h_cjfit/1E6,'-.k');
    plot(T,h_hpfit/1E6,'--g');
    plot(TCJ,h_CJ/1E6,'+k');
    xlabel('Temperature (K)','FontSize',fontsize);
    ylabel('Enthalpy (MJ/kg)','FontSize',fontsize);
    legend({'Reactants','Detonation products','Fit to detonation products','Fit to HP products','CJ state' },'Location','southeast','Fontsize',8);
    hold off
end

