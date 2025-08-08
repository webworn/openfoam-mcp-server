% Shock and Detonation Toolbox Demo Program
% 
% This program computes the ratio of specific heats and logarithmic
% isentrope slope using several approaches and compares the results
% graphically. See Appendix G of the report (below).
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
clear;clc;
disp('demo_g.m Ratio of Specific Heat Evaluation')
% set composition 
%mech = 'Hong2011.cti';
%mech = 'gri30_highT.cti';
mech = 'Mevel2017.cti';
gas = Solution(mech);
q = 'O2:1 H2:2.002'; 
T1 = 300;
P1 = oneatm;
% find species index for plotting
iH = speciesIndex(gas,'H');
iO = speciesIndex(gas,'O');
iOH = speciesIndex(gas,'OH');
iH2O = speciesIndex(gas,'H2O');
iH2 = speciesIndex(gas,'H2');
iO2 = speciesIndex(gas,'O2');
% Use constant volume explosion as starting point for isentropic expansion
set(gas, 'T', T1, 'P', P1, 'X', q);
equilibrate(gas,'UV');
T2 = temperature(gas);
S2 = entropy_mass(gas);
V2 = 1/density(gas);
V  = V2;
X2 = moleFractions(gas);
for i = 1:20
    % equilibrium state computations
    set(gas,'V',V,'S',S2);
    equilibrate(gas,'SV');
    P(i) = pressure(gas);
    R(i) = density(gas);
    T(i) = temperature(gas);
    %species
    X = moleFractions(gas);
    xH(i) = X(iH);
    xO(i) = X(iO);
    xOH(i) = X(iOH);
    xH2(i) = X(iH2);
    xO2(i) = X(iO2);
    xH2O(i) = X(iH2O);
    % gamma computed from Gruneisen coefficient
    grun_eq(i) = gruneisen_eq(gas);
    grun_fr(i) = gruneisen_fr(gas);
    a_eq(i) = soundspeed_eq(gas);
    a_fr(i) = soundspeed_fr(gas); 
    % logarithmic slope of isentropes in P-V coordinates
    kappa_fr(i) =  a_fr(i)*a_fr(i)*R(i)/P(i);
    kappa_eq(i) =  a_eq(i)*a_eq(i)*R(i)/P(i);
    % logarithmic slope of isentropes in T-V coordinates
    % equilibrium
    set(gas,'S',S2,'V',1.01*V)
    equilibrate(gas,'SV');
    T_2 = temperature(gas);
    set(gas,'S',S2,'V',0.99*V)
    equilibrate(gas,'SV');
    T_1 = temperature(gas);
    tvdvdt_eq(i) = -V*(T_2-T_1)/(0.02*V)/T(i);
    % frozen
    set(gas,'S',S2,'V',V*1.01,'X',X2)
    T_2 = temperature(gas);
    set(gas,'S',S2,'V',0.99*V,'X',X2)
    T_1 = temperature(gas);
    tvdvdt_fr(i) = -1.*V*(T_2-T_1)/(0.02*V)/T(i);
     % compute equilibrium values of isothermal dP/drho
    set(gas,'T',T(i),'V',1.01*V)
    equilibrate(gas,'TV');
    P2 = pressure(gas);
    set(gas,'T',T(i),'V',0.99*V)
    equilibrate(gas,'TV');
    P1 = pressure(gas);
    dpdr(i) = -V*V*(P2-P1)/(0.02*V);
    % compute ratio of specific heats from thermodynamic identities
    gamma_eq(i) = a_eq(i)*a_eq(i)/dpdr(i);
    gamma_fr(i) = a_fr(i)*a_fr(i)*R(i)/P(i);
    % compute the difference between equilbrium and frozen dpdrho_T
    diff(i) = dpdr(i)*R(i)/P(i);
    % compute equilibrium values of dvdt_P
    set(gas,'T',T(i)*1.01,'P',P(i));
    equilibrate(gas, 'TP');
    V_2 = 1./density(gas);
    set(gas,'T',T(i)*0.99,'P',P(i));
    equilibrate(gas, 'TP');
    V_11 = 1./density(gas);
    dvdt = (V_2-V_11)/(0.02*T(i));
    G_mult(i) = dvdt*T(i)*R(i);
    % compute frozen specific heats from thermodynamic properties
    set(gas,'T',T(i),'V',V);
    equilibrate(gas, 'TV');
    gamma_fr_thermo(i) = cp_mass(gas)/cv_mass(gas);
    % compute equilibrium specific heats from properties and definition
    set(gas,'T',T(i)*1.01,'V',V);
    equilibrate(gas, 'TV');
    U2 = intEnergy_mass(gas);
    set(gas,'T',T(i)*0.99,'V',V);
    equilibrate(gas, 'TV');
    U1 = intEnergy_mass(gas);
    CVEQ = (U2-U1)/(.02*T(i));
    set(gas,'T',T(i)*1.01,'P',P(i));
    equilibrate(gas, 'TP');
    H2 = enthalpy_mass(gas);
    set(gas,'T',T(i)*0.99,'P',P(i));
    equilibrate(gas, 'TP');
    H1 = enthalpy_mass(gas);
    CPEQ = (H2-H1)/(.02*T(i));
    gamma_eq_thermo(i) = CPEQ/CVEQ;
    ratio(i) = gamma_eq(i)/gamma_eq_thermo(i);
    % frozen pressure and temperature 
    set(gas,'S',S2,'V',V,'X',X2);
    P_f(i) = pressure(gas);
    T_f(i) = temperature(gas);
    % next state
    V = 2.*V;
end
% plotting
	figure(1); clf;
    hold on
	plot(T(:),grun_fr(:),':','LineWidth',1);  
  	plot(T(:),kappa_fr(:)-1,'-.','LineWidth',1);  
    plot(T(:),gamma_fr(:)-1,'--','LineWidth',1); 
    plot(T(:),gamma_fr_thermo(:)-1,'-','LineWidth',1); 
	title(['Frozen \gamma comparisons'],'FontSize', 12,'FontName','Times New Roman');
	xlabel('temperature (K)','FontSize', 12,'FontName','Times New Roman');
	ylabel('\gamma - 1','FontSize', 12,'FontName','Times New Roman');
    legend('G_{fr}','\kappa_{fr}-1','\gamma_{fr}-1','(c_p/c_v)_{fr} - 1');
	set(gca,'FontSize',10,'LineWidth',1);
    hold off
%
	figure(2); clf;
    hold on
	plot(T(:),grun_eq(:),':','LineWidth',1);  
  	plot(T(:),kappa_eq(:)-1,'-.','LineWidth',1);  
    plot(T(:),gamma_eq(:)-1,'--','LineWidth',1); 
    plot(T(:),gamma_eq_thermo(:)-1,'-','LineWidth',1); 
	title(['Equilibrium \gamma comparisons'],'FontSize', 12,'FontName','Times New Roman');
	xlabel('temperature (K)','FontSize', 12,'FontName','Times New Roman');
	ylabel('\gamma - 1','FontSize', 12,'FontName','Times New Roman');
    legend('G_{eq}','\kappa_{eq}-1','\gamma_{eq}-1','(c_p/c_v)_{eq} - 1');
	set(gca,'FontSize',10,'LineWidth',1);
    hold off
    %
	figure(3); clf;
    hold on
    plot(T(:),gamma_fr(:)-1,':','LineWidth',1); 
    plot(T(:),gamma_eq_thermo(:)-1,'-.','LineWidth',1); 
  	plot(T(:),kappa_eq(:)-1,'--','LineWidth',1);  
	title(['Frozen vs Equilibirum \gamma comparisons'],'FontSize', 12,'FontName','Times New Roman');
	xlabel('temperature (K)','FontSize', 12,'FontName','Times New Roman');
	ylabel('\gamma - 1','FontSize', 12,'FontName','Times New Roman');
    legend('\gamma_{fr}-1','\gamma_{eq}-1','\kappa_{eq}-1');
	set(gca,'FontSize',10,'LineWidth',1);
    hold off
    %
	figure(4); clf;
    plot(T,diff,'k-','LineWidth',1); 
	title(['Equilibrium for (\rho/P)(dP/d\rho)_T'],'FontSize', 12,'FontName','Times New Roman');
	xlabel('temperature (K)','FontSize', 12,'FontName','Times New Roman');
	ylabel('(\rho/P)(dP/d\rho)_T','FontSize', 12,'FontName','Times New Roman');
	set(gca,'FontSize',10,'LineWidth',1);
%
	figure(5); clf;
    plot(T,G_mult,'k-','LineWidth',1); 
	title(['Equilibrium value of (T/v)dv/dT_p'],'FontSize', 12,'FontName','Times New Roman');
	xlabel('temperature (K)','FontSize', 12,'FontName','Times New Roman');
	ylabel('(T/v)(dv/dT)_P','FontSize', 12,'FontName','Times New Roman');
	set(gca,'FontSize',10,'LineWidth',1);
%
    figure(6); clf;
    hold on
    plot(T,xH,'LineWidth',1); 
    plot(T,xO,'LineWidth',1); 
    plot(T,xOH,'LineWidth',1'); 
    plot(T,xH2,'LineWidth',1); 
    plot(T,xO2,'LineWidth',1); 
    plot(T,xH2O,'LineWidth',1); 
 	title(['Species distribution'],'FontSize', 12,'FontName','Times New Roman');
	xlabel('temperature (K)','FontSize', 12,'FontName','Times New Roman');
	ylabel('mole fraction','FontSize', 12,'FontName','Times New Roman');
    legend('H','O','OH','H2','O2','H2O','Location','northwest');
	set(gca,'FontSize',10,'LineWidth',1,'Yscale','log','Ylim',[1E-4 1],'YMinorTick','on');
    hold off
    %
    V_p = 1./R;
	figure(7); clf;
    hold on
    plot(V_p,T_f,'k--','LineWidth',1); 
    plot(V_p,T,'k-','LineWidth',1); 
  	title(['Frozen vs Equilibirum isentrope'],'FontSize', 12,'FontName','Times New Roman');
	xlabel('volume (m^3/kg)','FontSize', 12,'FontName','Times New Roman');
	ylabel('temperature (K)','FontSize', 12,'FontName','Times New Roman');
    legend('Frozen','Equilibrium','Location','northeast');
	set(gca,'FontSize',10,'LineWidth',1,'Yscale','log','Ylim',[100 4000],'YMinorTick','on','Xscale','log','Xlim',[1 1E5],'XMinorTick','on');
    hold off
    %
    figure(8); clf;
    hold on
    plot(V_p,P_f/oneatm,'k--','LineWidth',1); 
    plot(V_p,P/oneatm,'k-','LineWidth',1); 
  	title(['Frozen vs Equilibirum isentrope'],'FontSize', 12,'FontName','Times New Roman');
	xlabel('volume (m^3/kg)','FontSize', 12,'FontName','Times New Roman');
	ylabel('pressure (atm)','FontSize', 12,'FontName','Times New Roman');
    legend('Frozen','Equilibrium','Location','northeast');
	set(gca,'FontSize',10,'LineWidth',1,'Yscale','log','Ylim',[1E-5 20],'YMinorTick','on','Xscale','log','Xlim',[1 1E5],'XMinorTick','on');
    hold off
    %
    %% fitting isentropes
    % restrict fit to volumes less than 1000 times initial value
    valid = find(V_p < 1000.*V2);
    last = length(valid);
    % first find log values
    vlog = log(V_p(1:last));
    telog = log(T(1:last));
    tflog = log(T_f(1:last));
    pelog = log((P(1:last)/oneatm));
    pflog = log((P_f(1:last)/oneatm));
    % fit log with linear fit to get slopes.  For higher accuracy use a 2nd
    % order or higher polynomial as there is enough departure from
    % linearity 
    f = fit(vlog',telog','poly1');
    kfit_te = f.p1;
    figure(9); clf
    plot(f,vlog,telog,'o');
    title(['Equilibrium T-V fit, slope = ', num2str(kfit_te)]);
    xlabel('log specific volume');
    ylabel('log specific temperature');
    %
    f = fit(vlog',tflog','poly1');
    kfit_tf = f.p1;
    figure(10); clf
    plot(f,vlog,tflog,'o')
    title(['Frozen T-V fit, slope = ', num2str(kfit_tf)]);
    xlabel('log specific volume');
    ylabel('log specific temperature');
    %
    f = fit(vlog',pelog','poly1');
    kfit_pe = f.p1;
    figure(11); clf
    plot(f,vlog,pelog,'o')
     title(['Equilibrium P-V fit, slope = ', num2str(kfit_pe)]);
    xlabel('log specific volume');
    ylabel('log specific temperature');
   %
    f = fit(vlog',pflog','poly1');
    kfit_pf = f.p1;
    figure(12); clf
    plot(f,vlog,pflog,'o')
     title(['Frozen P-V fit, slope = ', num2str(kfit_pf)]);
    xlabel('log specific volume');
    ylabel('log specific temperature');

