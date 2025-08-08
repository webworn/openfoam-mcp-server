"""
Shock and Detonation Toolbox Demo Program

Calculate the FROZEN post shock state based on the initial gas 
state and the shock speed.  Evaluates the shock jump conditions using a fixed
composition using SDToolbox function PostShock_fr.

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
from sdtoolbox.postshock import PostShock_fr

# Initial state specification:
# P1 = Initial Pressure  
# T1 = Initial Temperature 
# U = Shock Speed 
# q = Initial Composition 
# mech = Cantera mechanism File name

P1 = 100000; P1atm = P1/ct.one_atm
T1 = 300
U = 2000
q = 'H2:2 O2:1 N2:3.76'
mech = 'Mevel2017.cti'
 
gas = PostShock_fr(U, P1, T1, q, mech)
Ps = gas.P/ct.one_atm

print(' ')
print('Initial state: ' + q + ', P1 = %.2f atm,  T1 = %.2f K' % (P1atm,T1) )
print('Mechanism: ' + mech)
print('Frozen postshock state: Ps = %.2f atm, Ts = %.2f K' % (Ps,gas.T))
print(' ')
