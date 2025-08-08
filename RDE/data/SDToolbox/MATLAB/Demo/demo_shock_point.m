% Shock and Detonation Toolbox Demo Program
% 
% This is a demostration of how to compute frozen and equilibrium postshock
% conditions for a single shock Mach number
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
close all;clear;clc;
disp('shock_point')
transport = true;
mech = 'airNASA9noions.cti';
gas1 = Solution(mech);
gas2 = Solution(mech);
gas3 = Solution(mech);
%% set initial state of gas.
P1 = 770.;
T1 = 728;
q = 'O2:1 N2:3.76';
set(gas1,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
rho1 = density(gas1);
af1 = soundspeed_fr(gas1);
M1 = 7.16;   % shock Mach number
speed = M1*af1;
gamma1_fr = af1^2*rho1/P1;
if (transport)
    mu1 = viscosity(gas1);
    nu1 = mu1/rho1;
    kcond1 = thermalConductivity(gas1);
    kdiff1 = kcond1/(rho1*cp_mass(gas1));
    Pr = mu1*cp_mass(gas1)/kcond1;
end

%%  Print out initial state
disp(['Shock computation for ',mech,' with composition ',q]);
disp(['Initial State']);
disp(['   Shock speed ',num2str(speed),' (m/s)']);
disp(['   Frozen sound Speed ',num2str(af1),' (m/s)']);
disp(['   Shock Mach number ',num2str(M1)]);
disp(['   Pressure ',num2str(P1),' (Pa)']);
disp(['   Temperature ',num2str(T1),' (K)']);
disp(['   Density ',num2str(rho1),' (kg/m3)']);
disp(['   gamma2 (based on frozen sound speed) ',num2str(gamma1_fr),' (m/s)']);
disp(['   Specific heat at constant pressure ',num2str(cp_mass(gas1)),' (J/kg K)']);
if (transport)
    disp(['   viscosity ',num2str(mu1),' (kg/m s)']);
    disp(['   viscosity (kinematic)',num2str(nu1),' (m2/s)']);
    disp(['   thermal conductivity ',num2str(kcond1),' (W/m K)']);
    disp(['   thermal diffusivity ',num2str(kdiff1),' (m2/s)']);
    disp(['   Prandtl number ',num2str(Pr)]);
end

%% Postshock (Frozen)
[gas2] = PostShock_fr(speed,P1, T1, q, mech);
af2 = soundspeed_fr(gas2);
ae2 = soundspeed_eq(gas2);
P2 = pressure(gas2);
T2 = temperature(gas2);
S2 = entropy_mass(gas2);
rho2 = density(gas2);
gamma2_fr = af2*af2*rho2/P2;
gamma2_eq = ae2*ae2*rho2/P2;
w2 = rho1*speed/rho2;
u2 = speed - w2;
if (transport)
    mu = viscosity(gas2);
    nu = mu/rho2;
    kcond = thermalConductivity(gas2);
    kdiff = kcond/(rho2*cp_mass(gas2));
    Pr = mu*cp_mass(gas2)/kcond;
end

%%  Print frozen shock state
disp(['Frozen shock state'])
disp(['   Pressure ',num2str(P2),' (Pa)']);
disp(['   Temperature ',num2str(T2),' (K)']);
disp(['   Density ',num2str(rho2),' (kg/m3)']);
disp(['   Specific heat at constant pressure ',num2str(cp_mass(gas2)),' (J/kg K)']);
disp(['   Expansion  ratio ',num2str(rho1/rho2),' (kg/m3)']);
disp(['   Entropy ',num2str(S2),' (J/kg-K)']);
disp(['   Sound speed (frozen) ',num2str(af2),' (m/s)']);
disp(['   Sound speed (equilibrium) ',num2str(ae2),' (m/s)']);
disp(['   gamma2 (based on frozen sound speed) ',num2str(gamma2_fr),' (m/s)']);
disp(['   gamma2 (based on equilibrium sound speed) ',num2str(gamma2_eq),' (m/s)']);
if (transport)
    disp(['   viscosity ',num2str(mu),' (kg/m s)']);
    disp(['   viscosity (kinematic)',num2str(nu),' (m2/s)']);
    disp(['   thermal conductivity ',num2str(kcond),' (W/m K)']);
    disp(['   thermal diffusivity ',num2str(kdiff),' (m2/s)']);
    disp(['   Prandtl number ',num2str(Pr)]);
end

%%  Postshock (Equilibrium)
[gas3] = PostShock_eq(speed,P1, T1, q, mech);
af3 = soundspeed_fr(gas3);
ae3 = soundspeed_eq(gas3);
P3 = pressure(gas3);
T3 = temperature(gas3);
rho3 = density(gas3);
S3 = entropy_mass(gas3);
gamma3_fr = af3*af3*rho3/P3;
gamma3_eq =  ae3*ae3*rho3/P3;
w3 = rho1*speed/rho3;
u3 = speed - w3;
if (transport)
    mu = viscosity(gas3);
    nu = mu/rho3;
    kcond = thermalConductivity(gas3);
    kdiff = kcond/(rho3*cp_mass(gas3));
    Pr = mu*cp_mass(gas3)/kcond;
end

%%  Print equilibrium shock state
disp(['Equilibrium shock state'])
disp(['   Pressure ',num2str(P3),' (Pa)']);
disp(['   Temperature ',num2str(T3),' (K)']);
disp(['   Density ',num2str(rho3),' (kg/m3)']);
disp(['   Specific heat at constant pressure ',num2str(cp_mass(gas3)),' (J/kg K)']);
disp(['   Expansion  ratio ',num2str(rho1/rho3),' (kg/m3)']);
disp(['   Entropy ',num2str(S3),' (J/kg-K)']);
disp(['   Sound speed (frozen) ',num2str(af3),' (m/s)']);
disp(['   Sound speed (equilibrium) ',num2str(ae3),' (m/s)']);
disp(['   gamma2 (based on frozen sound speed) ',num2str(gamma3_fr),' (m/s)']);
disp(['   gamma2 (based on equilibrium sound speed) ',num2str(gamma3_eq),' (m/s)']);
if (transport)
    disp(['   viscosity ',num2str(mu),' (kg/m s)']);
    disp(['   viscosity (kinematic)',num2str(nu),' (m2/s)']);
    disp(['   thermal conductivity ',num2str(kcond),' (W/m K)']);
    disp(['   thermal diffusivity ',num2str(kdiff),' (m2/s)']);
    disp(['   Prandtl number ',num2str(Pr)]);
end
