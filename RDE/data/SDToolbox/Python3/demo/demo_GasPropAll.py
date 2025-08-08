"""
Shock and Detonation Toolbox Demo Program

Mixture thermodynamic and transport properties of gases at fixed pressure
as a function of temperature. Edit to choose either frozen or equilibrium
composition state.  The mechanism file must contain transport parameters
for each species and specify the transport model 'Mix'. This version
writes a single (T,P) case to the display.
  
################################################################################
Theory, numerical methods and applications are described in the following report:

SDToolbox - Numerical Tools for Shock and Detonation Wave Modeling,
Explosion Dynamics Laboratory, Contributors: S. Browne, J. Ziegler,
N. Bitter, B. Schmidt, J. Lawson and J. E. Shepherd, GALCIT
Technical Report FM2018.001 Revised January 2021.

Please cite this report and the website if you use these routines. 

Please refer to LICENSE.txt or the above report for copyright and disclaimers.

http://shepherd.caltech.edu/EDL/PublicResources/sdt/

################################################################################ 
Updated September 2018
Tested with: 
    Python 3.5 and 3.6, Cantera 2.3 and 2.4
Under these operating systems:
    Windows 8.1, Windows 10, Linux (Debian 9)
"""
import cantera as ct
from sdtoolbox.thermo import soundspeed_eq,soundspeed_fr

## Input data
Ru = ct.gas_constant
T0 = 273.15
P0 = 1.0*ct.one_atm # specify pressure in multiples of an atmosphere
q = 'O2:0.2095 N2:0.7809 AR:0.0093 CO2:0.00039 nC10H22:0.00613 '# "real" air + decane
mech = 'JetSurf2.cti'
transport = 'Mix' # simple mixture average transport model 
gas = ct.Solution(mech,'gas');
## Compute properties at T P and q
T = 2000
P = P0
print('Computing  properties for '+q+' using '+mech)
gas.TPX = T,P,q
gas.equilibrate('TP') # comment out to get frozen properties
rho = gas.density
Wgas = gas.mean_molecular_weight
Rgas = Ru/Wgas
h = gas.enthalpy_mass
u = gas.int_energy_mass
cp = gas.cp_mass
cv = gas.cv_mass
gamma_f = cp/cv
a_e = soundspeed_eq(gas)
a_f = soundspeed_fr(gas)
mu = gas.viscosity
nu = mu/rho
kcond = gas.thermal_conductivity
kdiff = kcond/(rho*cp)
Pr = mu*cp/kcond;
#
species = 'nC10H22'
i_species = gas.species_index(species)
#DIJ = gas.binary_diff_coeffs # Binary diffusion coefficients
DI = gas.mix_diff_coeffs      # Mixture diffusion Coefficients
#
print('   Pressure '+str(P/1e3)+' (kPa)')
print('   Temperature '+str(T)+' (K)')
print('   Density '+str(rho)+' (kg/m3)')
print('   Mean molar mass '+str(Wgas)+' (kg/kmol)')
print('   Gas constant '+str(Rgas)+' (kg/kmol)')
print('   Enthalpy '+str(h/1e6)+' (MJ/kg)')
print('   Energy '+str(u/1e6)+' (MJ/kg)')
print('   Cp '+str(cp)+' (J/kg K)')
print('   Cv '+str(cv)+' (J/kg K)')
print('   Equilibrium Sound speed '+str(a_e)+' (m/s)')
print('   Frozen Sound speed '+str(a_f)+' (m/s)')
print('   Frozen Cp/Cv '+str(gamma_f)+' (m/s)')
print('   viscosity '+str(mu)+' (kg/m s)')
print('   viscosity (kinematic) '+str(nu)+' (m2/s)')
print('   thermal conductivity '+str(kcond)+' (W/m K)')
print('   thermal diffusivity '+str(kdiff)+' (m2/s)')
print('   Prandtl number '+str(Pr))
print('   Mixture mass diffusivity of '+species+' '+str(DI[i_species])+' (m2/s)')
print('   Lewis number of '+species+' '+str(kdiff/DI[i_species]))
print('   Schmidt number of '+species+' '+str(nu/DI[i_species]))
i_N2 = gas.species_index('N2')
print('   Mixture mass diffusivity of '+'N2'+' '+str(DI[i_N2])+' (m2/s)')
print('   Lewis number of '+'N2'+' '+str(kdiff/DI[i_N2]))
print('   Schmidt number of '+'N2'+' '+str(nu/DI[i_N2]))
print('   ')

