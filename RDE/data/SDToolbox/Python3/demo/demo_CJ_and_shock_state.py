"""
Shock and Detonation Toolbox Demo Program

Calculates 2 reflection conditions
  1) equilibrium post-initial-shock state behind a shock traveling at CJ speed (CJ state) 
     followed by equilibrium post-reflected-shock state
  2) frozen post-initial-shock state behind a CJ wave 
     followed by frozen post-reflected-shock state
 
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
from sdtoolbox.postshock import CJspeed, PostShock_eq, PostShock_fr
from sdtoolbox.reflections import reflected_eq, reflected_fr
from sdtoolbox.thermo import soundspeed_eq, soundspeed_fr
from numpy import set_printoptions

# set initial state, composition, and gas object
P1 = 100000; T1 = 300
q = 'CH4:0.5 O2:1 N2:3.76'
mech = 'gri30_highT.cti'
gas1 = ct.Solution(mech);
gas1.TPX = T1,P1,q

# Find CJ speed
cj_speed = CJspeed(P1, T1, q, mech)

# Evaluate gas state
gas = PostShock_eq(cj_speed,P1, T1, q, mech)

# Evaluate properties of gas object 
T2 = gas.T
P2 = gas.P
R2 = gas.density
V2 = 1/R2
S2 = gas.entropy_mass
w2 = gas1.density*cj_speed/R2
u2 = cj_speed - w2
x2 = gas.X
a2_eq = soundspeed_eq(gas)
a2_fr = soundspeed_fr(gas)
gamma2_fr =  a2_fr**2*R2/P2
gamma2_eq =  a2_eq**2*R2/P2

# Print out
set_printoptions(precision=4)
print('CJ computation for '+mech+' with composition '+q)
print('CJ Parameters')
print('   UCJ '+str(cj_speed)+' (m/s)')
print('   Pressure '+str(P2)+' (Pa)')
print('   Temperature '+str(T2)+' (K)')
print('   Density '+str(R2)+' (kg/m3)')
print('   Entropy '+str(S2)+' (J/kg-K)')
print('   w2 (wave frame) '+str(w2)+' (m/s)')
print('   u2 (lab frame) '+str(u2)+' (m/s)')
print('   a2 (frozen) '+str(a2_fr)+' (m/s)')
print('   a2 (equilibrium) '+str(a2_eq)+' (m/s)')
print('   gamma2 (frozen) '+str(gamma2_fr)+' (m/s)')
print('   gamma2 (equilibrium) '+str(gamma2_eq)+' (m/s)')
print('--------------------------------------')
#
gas3 = ct.Solution(mech)
p3,UR,gas3 = reflected_eq(gas1,gas,gas3,cj_speed)
print('Reflected CJ shock (equilibrium) computation for '+mech+' with composition '+q)
print('   CJ speed '+str(cj_speed)+' (m/s)')
print('   Reflected wave speed '+str(UR)+' (m/s)')
print('Post-Reflected Shock Parameters')
print('   Pressure '+str(gas3.P)+' (Pa)')
print('   Temperature '+str(gas3.T)+' (K)')
print('   Density '+str(gas3.density)+' (kg/m3)')
print('--------------------------------------');
#
print('Shock computation for '+mech+' with composition '+q)
gas = PostShock_fr(cj_speed,P1,T1,q,mech)
print('    shock speed '+str(cj_speed)+' (m/s)')
print('Postshock State')
print('    Pressure '+str(gas.P)+' (Pa)')
print('    Temperature '+str(gas.T)+' (K)')
print('    Density '+str(gas.density)+' (kg/m3)')
print('    Entropy '+str(gas.entropy_mass)+' (J/kg-K)')
w2 = cj_speed*gas1.density/gas.density
u2 = cj_speed-w2
print('    w2 (wave frame) '+str(w2)+' (m/s)')
print('    u2 (lab frame) '+str(u2)+' (m/s)')
print('    a2 (frozen) '+str(soundspeed_fr(gas))+' (m/s)')
gamma2_fr =  soundspeed_fr(gas)**2*gas.density/gas.P
print('    gamma2 (frozen) '+str(gamma2_fr)+' (m/s)')
print('--------------------------------------')
#
[p3,UR,gas3] = reflected_fr(gas1,gas,gas3,cj_speed);
print('Reflected CJ shock (frozen) computation for '+mech+' with composition '+q)
print('   Shock speed '+str(cj_speed)+' (m/s)')
print('   Reflected wave speed '+str(UR)+' (m/s)')
print('Post-Reflected Shock Parameters')
print('   Pressure '+str(gas3.P)+' (Pa)')
print('   Temperature '+str(gas3.T)+' (K)')
print('   Density '+str(gas3.density)+' (kg/m3)')

