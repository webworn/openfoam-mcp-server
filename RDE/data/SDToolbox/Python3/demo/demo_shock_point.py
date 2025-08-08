"""
Shock and Detonation Toolbox Demo Program

This is a demostration of how to compute frozen and equilibrium postshock
conditions for a single shock Mach number

################################################################################
Theory, numerical methods and applications are described in the following report:

SDToolbox - Numerical Tools for Shock and Detonation Wave Modeling,
Explosion Dynamics Laboratory, Contributors: S. Browne, J. Ziegler,
N. Bitter, B. Schmidt, J. Lawson and J. E. Shepherd, GALCIT
Technical Report FM2018.001 Revised January 2021.

Please cite this report and the website if you use these routines. 

Please refer to LICENCE.txt or the above report for copyright and disclaimers.

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
from sdtoolbox.postshock import PostShock_eq,PostShock_fr

transport = True
mech = 'airNASA9noions.cti'
gas1 = ct.Solution(mech)
gas2 = ct.Solution(mech)
gas3 = ct.Solution(mech)

## set initial state of gas.
P1 = 770
T1 = 728
q = 'O2:1 N2:3.76'
gas1.TPX = T1,P1,q
rho1 = gas1.density
af1 = soundspeed_fr(gas1)
M1 = 7.16 # shock Mach number
speed = M1*af1
gamma1_fr = af1**2*rho1/P1
if transport:
    mu1 = gas1.viscosity
    nu1 = mu1/rho1
    kcond1 = gas1.thermal_conductivity
    kdiff1 = kcond1/(rho1*gas1.cp_mass)
    Pr = mu1*gas1.cp_mass/kcond1

## Print out initial state
print('Shock computation for '+mech+' with composition '+q)
print('Initial State')
print('   Shock speed '+str(speed)+' (m/s)')
print('   Frozen sound Speed '+str(af1)+' (m/s)')
print('   Shock Mach number '+str(M1))
print('   Pressure '+str(P1)+' (Pa)')
print('   Temperature '+str(T1)+' (K)')
print('   Density '+str(rho1)+' (kg/m3)')
print('   gamma2 (based on frozen sound speed) '+str(gamma1_fr)+' (m/s)')
print('   Specific heat at constant pressure '+str(gas1.cp_mass)+' (J/kg K)')
if transport:
    print('   viscosity '+str(mu1)+' (kg/m s)')
    print('   viscosity (kinematic)'+str(nu1)+' (m2/s)')
    print('   thermal conductivity '+str(kcond1)+' (W/m K)')
    print('   thermal diffusivity '+str(kdiff1)+' (m2/s)')
    print('   Prandtl number '+str(Pr))

## Postshock (Frozen)
gas2 = PostShock_fr(speed,P1,T1,q,mech)
af2 = soundspeed_fr(gas2)
ae2 = soundspeed_eq(gas2)
P2 = gas2.P
T2 = gas2.T
S2 = gas2.entropy_mass
rho2 = gas2.density
gamma2_fr = af2**2*rho2/P2
gamma2_eq = ae2**2*rho2/P2
w2 = rho1*speed/rho2
u2 = speed - w2
if transport:
    mu = gas2.viscosity
    nu = mu/rho2
    kcond = gas2.thermal_conductivity
    kdiff = kcond/(rho2*gas2.cp_mass)
    Pr = mu*gas2.cp_mass/kcond

## Print frozen shock state
print('Frozen shock state')
print('   Pressure '+str(P2)+' (Pa)')
print('   Temperature '+str(T2)+' (K)')
print('   Density '+str(rho2)+' (kg/m3)')
print('   Specific heat at constant pressure '+str(gas2.cp_mass)+' (J/kg K)')
print('   Expansion  ratio '+str(rho1/rho2)+' (kg/m3)')
print('   Entropy '+str(S2)+' (J/kg-K)')
print('   Sound speed (frozen) '+str(af2)+' (m/s)')
print('   Sound speed (equilibrium) '+str(ae2)+' (m/s)')
print('   gamma2 (based on frozen sound speed) '+str(gamma2_fr)+' (m/s)')
print('   gamma2 (based on equilibrium sound speed) '+str(gamma2_eq)+' (m/s)')
if transport:
    print('   viscosity '+str(mu)+' (kg/m s)')
    print('   viscosity (kinematic)'+str(nu)+' (m2/s)')
    print('   thermal conductivity '+str(kcond)+' (W/m K)')
    print('   thermal diffusivity '+str(kdiff)+' (m2/s)')
    print('   Prandtl number '+str(Pr))

##  Postshock (Equilibrium)
gas3 = PostShock_eq(speed,P1,T1,q,mech)
af3 = soundspeed_fr(gas3)
ae3 = soundspeed_eq(gas3)
P3 = gas3.P
T3 = gas3.T
rho3 = gas3.density
S3 = gas3.entropy_mass
gamma3_fr = af3**2*rho3/P3
gamma3_eq = ae3**2*rho3/P3
w3 = rho1*speed/rho3
u3 = speed - w3
if transport:
    mu = gas3.viscosity
    nu = mu/rho3
    kcond = gas3.thermal_conductivity
    kdiff = kcond/(rho3*gas3.cp_mass)
    Pr = mu*gas3.cp_mass/kcond

##  Print equilibrium shock state
print('Equilibrium shock state')
print('   Pressure '+str(P3)+' (Pa)')
print('   Temperature '+str(T3)+' (K)')
print('   Density '+str(rho3)+' (kg/m3)')
print('   Specific heat at constant pressure '+str(gas3.cp_mass)+' (J/kg K)')
print('   Expansion  ratio '+str(rho1/rho3)+' (kg/m3)')
print('   Entropy '+str(S3)+' (J/kg-K)')
print('   Sound speed (frozen) '+str(af3)+' (m/s)')
print('   Sound speed (equilibrium) '+str(ae3)+' (m/s)')
print('   gamma2 (based on frozen sound speed) '+str(gamma3_fr)+' (m/s)')
print('   gamma2 (based on equilibrium sound speed) '+str(gamma3_eq)+' (m/s)')
if transport:
    print('   viscosity '+str(mu)+' (kg/m s)')
    print('   viscosity (kinematic)'+str(nu)+' (m2/s)')
    print('   thermal conductivity '+str(kcond)+' (W/m K)')
    print('   thermal diffusivity '+str(kdiff)+' (m2/s)')
    print('   Prandtl number '+str(Pr))
