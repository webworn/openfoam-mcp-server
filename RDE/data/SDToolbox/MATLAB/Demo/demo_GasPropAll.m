% Shock and Detonation Toolbox Demo Program
%
% Mixture thermodynamic and transport properties of gases at fixed pressure
% as a function of temperature. Edit to choose either frozen or equilibrium
% composition state.  The mechanism file must contain transport parameters
% for each species and specify the transport model 'Mix'. This version
% writes a single (T,P) case to the disp.
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
% Please refer to LICENSE.txt or the above report for copyright and disclaimers.
% 
% http://shepherd.caltech.edu/EDL/PublicResources/sdt/
% 
% ################################################################################ 
% Updated September 2018
% Tested with: 
%     MATLAB 2017b and 2018a, Cantera 2.3 and 2.4
% Under these operating systems:
%     Windows 8.1, Windows 10, Linux (Debian 9)
%% Input data
clear; clc;close all;
disp('GasPropAll')
Ru = gasconstant;
T0 = 273.15; 
P0 = 1.0*oneatm;   % specify pressure in multiples of an atmosphere
q = 'O2:0.2095 N2:0.7809 AR:0.0093 CO2:0.00039 nC10H22:0.00613';    % "real" air + decane
mech = 'JetSurf2.cti';
transport  =  'Mix';       % simple mixture average transport model 
gas = Solution(mech,'gas');
%% Compute properties at T P and q
T = 2000.;
P = P0;
disp(['Computing  properties for ',q,' using ',mech]);
set(gas, 'T', T, 'P', P, 'X', q);
equilibrate(gas,'TP');   %comment out to get frozen properties
rho = density(gas);  
Wgas = meanMolecularWeight(gas); 
Rgas = Ru/meanMolecularWeight(gas);
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
%
species = 'nC10H22';
i_species = speciesIndex(gas,species);
%DIJ = binDiffCoeffs(gas);   %Binary diffusion coefficients
DI = mixDiffCoeffs(gas);     %Mixture diffusion Coefficients
%
disp(['   Pressure ',num2str(P/1E3),' (kPa)']);
disp(['   Temperature ',num2str(T),' (K)']);
disp(['   Density ',num2str(rho),' (kg/m3)']);
disp(['   Mean molar mass ',num2str(Wgas),' (kg/kmol)']);
disp(['   Gas constant ',num2str(Rgas),' (kg/kmol)']);
disp(['   Enthalpy ',num2str(h/1E6),' (MJ/kg)']);
disp(['   Energy ',num2str(u/1E6),' (MJ/kg)']);
disp(['   Cp ',num2str(cp),' (J/kg K)']);
disp(['   Cv ',num2str(cv),' (J/kg K)']);
disp(['   Equilibrium Sound speed ',num2str(a_e),' (m/s)']);
disp(['   Frozen Sound speed ',num2str(a_f),' (m/s)']);
disp(['   Frozen Cp/Cv ',num2str(gamma_f),' (m/s)']);
disp(['   viscosity ',num2str(mu),' (kg/m s)']);
disp(['   viscosity (kinematic) ',num2str(nu),' (m2/s)']);
disp(['   thermal conductivity ',num2str(kcond),' (W/m K)']);
disp(['   thermal diffusivity ',num2str(kdiff),' (m2/s)']);
disp(['   Prandtl number ',num2str(Pr)]);
disp(['   Mixture mass diffusivity of ',species,' ',num2str(DI(i_species)),' (m2/s)']);
disp(['   Lewis number of ',species,' ',num2str(kdiff/DI(i_species))]);
disp(['   Schmidt number of ',species,' ',num2str(nu/DI(i_species))]);
i_N2 = speciesIndex(gas,'N2');
disp(['   Mixture mass diffusivity of ','N2',' ',num2str(DI(i_N2)),' (m2/s)']);
disp(['   Lewis number of ','N2',' ',num2str(kdiff/DI(i_N2))]);
disp(['   Schmidt number of ','N2',' ',num2str(nu/DI(i_N2))]);
disp(['   ']);

