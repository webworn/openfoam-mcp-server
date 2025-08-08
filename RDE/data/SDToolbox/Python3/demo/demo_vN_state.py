"""
Shock and Detonation Toolbox Demo Program

Calculates the frozen shock (vN = von Neumann) state of the gas behind 
the leading shock wave in a CJ detonation.

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
from sdtoolbox.thermo import soundspeed_fr, soundspeed_eq
from sdtoolbox.postshock import CJspeed,  PostShock_eq

## set initial state, composition, and gas object
P1 = 100000.; T1 = 300.
#q = 'H2:2.00 O2:1.0 N2:3.76'
#q = 'C2H4:1.00 O2:3 N2:11.28' 
q = 'C2H2:1 O2:2.5 AR:3'
#q = 'C2H4:1. O2:3'   
#q = 'O2:1. N2:3.76'
mech = 'gri30_highT.cti'
gas1 = ct.Solution(mech)
gas1.TPX = T1,P1,q

## Evaluate initial state
R1 = gas1.density
c1_fr = soundspeed_fr(gas1)
cp1 = gas1.cp_mass
w1 = gas1.mean_molecular_weight
gamma1_fr = c1_fr**2*R1/P1

## Set shock speed
cj_speed = CJspeed(P1, T1, q, mech)
Us = cj_speed

## Evaluate gas state
# q = 'O2:1. N2:3.76'
gas = PostShock_eq(Us,P1, T1, q, mech)

## Evaluate properties of gas object 
T2 = gas.T
P2 = gas.P
R2 = gas.density
V2 = 1/R2
S2 = gas.entropy_mass
w2 = gas1.density*Us/R2
u2 = Us - w2
x2 = gas.X
c2_eq = soundspeed_eq(gas)
c2_fr = soundspeed_fr(gas)
gamma2_fr = c2_fr**2*R2/P2
gamma2_eq = c2_eq**2*R2/P2

## Print out
print('CJ computation for '+mech+' with composition '+q)
print('Initial state')
print('   Pressure '+str(P1)+' (Pa)')
print('   Temperature '+str(T1)+' (K)')
print('   Density '+str(R1)+' (kg/m3)')
print('   c1 (frozen) '+str(c1_fr)+' (m/s)')
print('   gamma1 (frozen) '+str(gamma1_fr)+' ')
print('   Cp1 (frozen) '+str(cp1)+' ')
print('   W1  '+str(w1)+' ')
print('vN State')
print('Shock speed '+str(Us)+' (m/s)')
print('   Pressure '+str(P2)+' (Pa)')
print('   Temperature '+str(T2)+' (K)')
print('   Density '+str(R2)+' (kg/m3)')
print('   Entropy '+str(S2)+' (J/kg-K)')
print('   w2 (wave frame) '+str(w2)+' (m/s)')
print('   u2 (lab frame) '+str(u2)+' (m/s)')
print('   c2 (frozen) '+str(c2_fr)+' (m/s)')
print('   c2 (equilibrium) '+str(c2_eq)+' (m/s)')
print('   gamma2 (frozen) '+str(gamma2_fr)+' ')
print('   gamma2 (equilibrium) '+str(gamma2_eq)+' ')
