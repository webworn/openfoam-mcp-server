% Shock and Detonation Toolboox
% http://www.galcit.caltech.edu/EDL/public/cantera/html/SD_Toolbox/
% GasPropD.m
% Thermodynamic and transport properties of gases at fixed pressure as a 
% function of temperature. Edit to choose either frozen or equilibrium 
% composition state.  The mechanism file must contain transport 
% parameters for each species and specify the transport model 'Mix'.   
% This version writes a single data point to the display
% JES 25 May 2013
%% Input data
clear; clc;
display('GasPropD')
Ru = gasconstant;
T0 = 298.15; 
P0 = 1.0*oneatm;   % specify pressure in multiples of an atmosphere
%q = 'O2:0.21  N2:0.79';   % "combustion" air 
%q = 'O2:0.2095  N2:0.7809 AR:0.0093 CO2:0.00039';    % "real" air
q = 'CO2:1';
mech = 'airNASA9.cti';
transport  =  'Mix';       % simple mixture average transport model 
gas = importPhase(mech,'airNASA9_mix');
%% Compute properties at T P and q
T = 3000
P = P0
% shot 2767 
%T = 1505.
%P = 12700.
%q = 'O2:0.1690  N2:0.733 NO:0.073 O:0.025';    % "tunnel" air at BL edge
display(['Computing  properties for ',q,' using ',mech]);
set(gas, 'T', T, 'P', P, 'X', q);
%equilibrate(gas,'TP');   %comment out to get frozen properties
rho = density(gas);  
Wgas = meanMolarMass(gas); 
Rgas = Ru/meanMolarMass(gas);
h = enthalpy_mass(gas); 
u = intEnergy_mass(gas);
cp = cp_mass(gas);
cv = cp_mass(gas)-Rgas;
gamma_f = cp/cv;
a_e = soundspeed_eq(gas);
a_f = soundspeed_fr(gas);
mu = viscosity(gas);
nu = mu/rho;
kcond = thermalConductivity(gas);
kdiff = kcond/(rho*cp_mass(gas));
Pr = mu*cp_mass(gas)/kcond;
display(['   Pressure ',num2str(P/1E3),' (kPa)']);
display(['   Temperature ',num2str(T),' (K)']);
display(['   Density ',num2str(rho),' (kg/m3)']);
display(['   Mean molar mass ',num2str(Wgas),' (kg/kmol)']);
display(['   Gas constant ',num2str(Rgas),' (kg/kmol)']);
display(['   Enthalpy ',num2str(h/1E6),' (MJ/kg)']);
display(['   Energy ',num2str(u/1E6),' (MJ/kg)']);
display(['   Cp ',num2str(cp),' (J/kg K)']);
display(['   Cv ',num2str(cv),' (J/kg K)']);
display(['   Equilibrium Sound speed ',num2str(a_e),' (m/s)']);
display(['   Frozen Sound speed ',num2str(a_f),' (m/s)']);
display(['   Frozen Cp/Cv ',num2str(gamma_f),' (m/s)']);
display(['   viscosity ',num2str(mu),' (kg/m s)']);
display(['   viscosity (kinematic)',num2str(nu),' (m2/s)']);
display(['   thermal conductivity ',num2str(kcond),' (W/m K)']);
display(['   thermal diffusivity ',num2str(kdiff),' (m2/s)']);
display(['   Prandtl number ',num2str(Pr)]);
display(['   ']);

