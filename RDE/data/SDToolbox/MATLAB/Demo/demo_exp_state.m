% Shock and Detonation Toolbox Demo Program
% 
% Calculates mixture properties for explosion states (UV,HP, TP).
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
disp('demo_exp_state')
%% set initial state, composition, and gas object
trans = true;  % not all mechanisms have trans data defined
 CASE = 'HP';   %constant pressure adiabatic explosion
% CASE = 'UV';   %constant volume adiabatic explosion
% CASE = 'TP';   %specified temperature and pressure explosion
P1 = oneatm; T1 = 298.15; 
q = 'H2:6 O2:1  N2:3.76';    
if (trans)
    mech = 'gri30_highT.cti';
%     transport  =  'Mix';       % simple mixture average transport model 
%     gas = Solution(mech,'gri30_mix');
    transport = 'Multi';      % multicomponent transport model
    gas = Solution(mech,'gri30_multi');
    set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
    rho1 = density(gas);
    mu1 = viscosity(gas);
    nu1 = mu1/rho1;
    kcond1 = thermalConductivity(gas);
    kdiff1 = kcond1/(rho1*cp_mass(gas));
else
    gas = Solution(mech);
    set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
    rho1 = density(gas);
end
%% Find Explosion state
equilibrate(gas,CASE);  
%% Evaluate properties of gas object 
T2 = temperature(gas);
P2 = pressure(gas);
rho2 = density(gas);
V2 = 1/rho2;
S2 = entropy_mass(gas);
x2 = moleFractions(gas);
c2_eq = soundspeed_eq(gas);
c2_fr = soundspeed_fr(gas);
gamma2_fr =  c2_fr*c2_fr*rho2/P2;
gamma2_eq =  c2_eq*c2_eq*rho2/P2;
if (trans)
    mu = viscosity(gas);
    nu = mu/rho2;
    kcond = thermalConductivity(gas);
    kdiff = kcond/(rho2*cp_mass(gas));
    Pr = mu*cp_mass(gas)/kcond;
end
%% Print out
disp([CASE,' computation for ',mech,' with composition ',q])
disp('Initial State');
disp(['   Pressure ',num2str(P1),' (Pa)']);
disp(['   Temperature ',num2str(T1),' (K)']);
disp(['   Density ',num2str(rho1),' (kg/m3)']);
disp(['   Specific heat at constant pressure ',num2str(cp_mass(gas)),' (J/kg K)']);
if (trans)
    disp(['   Transport model ',transport]);
    disp(['   viscosity ',num2str(mu1),' (kg/m s)']);
    disp(['   viscosity (kinematic) ',num2str(nu1),' (m2/s)']);
    disp(['   thermal conductivity ',num2str(kcond1),' (W/m K)']);
    disp(['   thermal diffusivity ',num2str(kdiff1),' (m2/s)']);
end
disp([CASE,' State']);
disp(['   Pressure ',num2str(P2),' (Pa)']);
disp(['   Temperature ',num2str(T2),' (K)']);
disp(['   Density ',num2str(rho2),' (kg/m3)']);
disp(['   Specific heat at constant pressure ',num2str(cp_mass(gas)),' (J/kg K)']);
disp(['   Expansion ratio ',num2str(rho1/rho2),' (kg/m3)']);
disp(['   Entropy ',num2str(S2),' (J/kg-K)']);
disp(['   Sound speed (frozen) ',num2str(c2_fr),' (m/s)']);
disp(['   Sound speed (equilibrium) ',num2str(c2_eq),' (m/s)']);
disp(['   gamma2 (based on frozen sound speed) ',num2str(gamma2_fr),' (m/s)']);
disp(['   gamma2 (based on equilibrium sound speed) ',num2str(gamma2_eq),' (m/s)']);
if (trans)
    disp(['   viscosity ',num2str(mu),' (kg/m s)']);
    disp(['   viscosity (kinematic) ',num2str(nu),' (m2/s)']);
    disp(['   thermal conductivity ',num2str(kcond),' (W/m K)']);
    disp(['   thermal diffusivity ',num2str(kdiff),' (m2/s)']);
    disp(['   specific heat Cp ',num2str(cp_mass(gas)),' (J/kg K)']);
    disp(['   Prandtl number ',num2str(Pr)]);
end
disp(['Reactants at final temperature']);
set(gas,'Temperature',T2,'Pressure',P1,'MoleFractions',q);
rho1 = density(gas);
disp(['   Temperature ',num2str(T2),' (K)']);
disp(['   Density ',num2str(rho1),' (kg/m3)']);
disp(['   Specific heat at constant pressure ',num2str(cp_mass(gas)),' (J/kg K)']);
if (trans)
    mu1 = viscosity(gas);
    nu1 = mu1/rho1;
    kcond1 = thermalConductivity(gas);
    kdiff1 = kcond1/(rho1*cp_mass(gas));
    disp(['   viscosity ',num2str(mu1),' (kg/m s)']);
    disp(['   viscosity (kinematic) ',num2str(nu1),' (m2/s)']);
    disp(['   thermal conductivity ',num2str(kcond1),' (W/m K)']);
    disp(['   thermal diffusivity ',num2str(kdiff1),' (m2/s)']);
end
