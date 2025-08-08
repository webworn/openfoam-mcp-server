"""
Shock and Detonation Toolbox Demo Program

Calculates post-relected-shock state for a specified shock speed and a specified 
initial mixture.  In this demo, both shocks are reactive, i.e. the computed states 
behind both the incident and reflected shocks are EQUILIBRIUM states.  
The reflected state is computed by the SDToolbox function reflected_eq.

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
from sdtoolbox.postshock import CJspeed, PostShock_eq
from sdtoolbox.reflections import reflected_eq

# Initial state specification:
# P1 = Initial Pressure  
# T1 = Initial Temperature 
# U = Shock Speed 
# q = Initial Composition 
# mech = Cantera mechanism File name
P1 = 100000
T1 = 300
q = 'H2:2 O2:1 N2:3.76'    
mech = 'Mevel2017.cti'         
gas1 = ct.Solution(mech)
gas1.TPX = T1, P1, q

print ('Initial state: ' + q + ', P1 = %.2f atm,  T1 = %.2f K' % (P1/ct.one_atm,T1) )
print ('Mechanism: ' + mech)

# create gas objects for other states
gas2 = ct.Solution(mech)
gas3 = ct.Solution(mech)

# compute minimum incident wave speed
fig_num = 0;
cj_speed = CJspeed(P1, T1, q, mech)  
# incident wave must be greater than or equal to cj_speed for
# equilibrium computations
UI = 1.2*cj_speed

print('Incident shock speed UI = %.2f m/s' % (UI))

# compute postshock gas state object gas2
gas2 = PostShock_eq(UI, P1, T1, q, mech);
P2 = gas2.P/ct.one_atm;

print ('Equilibrium Post-Incident-Shock State')
print ('T2 = %.2f K, P2 = %.2f atm' % (gas2.T,P2))

# compute reflected shock post-shock state gas3
[p3,UR,gas3]= reflected_eq(gas1,gas2,gas3,UI);
# Outputs:
# p3 - pressure behind reflected wave
# UR = Reflected shock speed relative to reflecting surface
# gas3 = gas object with properties of postshock state

P3 = gas3.P/ct.one_atm
print ('Equilibrium Post-Reflected-Shock State')
print ('T3 = %.2f K,  P3 = %.2f atm' % (gas3.T,P3))
print ("Reflected Wave Speed = %.2f m/s" % (UR))

# gas states
print('Incident gas state')
gas1()
print('Post-incident-shock gas state')
gas2()
print('Post-reflected-schock gas state')
gas3()
