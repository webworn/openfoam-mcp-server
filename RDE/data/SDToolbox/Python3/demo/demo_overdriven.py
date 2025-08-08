"""
Shock and Detonation Toolbox Demo Program

Computes detonation and reflected shock wave pressure for overdriven
waves. Both the post-initial-shock and the post-reflected-shock states
are equilibrium states.  Creates output file.

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
from sdtoolbox.postshock import CJspeed,PostShock_eq
from sdtoolbox.reflections import reflected_eq
import numpy as np
import datetime

# set initial state, composition, and gas object
P1 = 100000 
T1 = 300 
q = 'H2:0.31 N2O:0.69'    
mech = 'Mevel2015.cti'
gas1 = ct.Solution(mech)
gas1.TPX = T1,P1,q

# Find CJ speed
cj_speed = CJspeed(P1, T1, q, mech)
gas = PostShock_eq(cj_speed,P1, T1, q, mech)
print('CJ computation for '+mech+' with composition '+q)
P2 = gas.P
print('   CJ speed '+str(cj_speed)+' (m/s)')
print('   CJ pressure '+str(P2)+' (Pa)')

# Evaluate gas state

start = 1; stop = 1.5; step = 0.05 # approximate desired step size (will not be exactly followed)
npoints = int((stop-start)/step)
speed = []; vs = []; ps = []; pr = []; vr = [];
for f in np.linspace(start,stop,num=npoints):
    u_shock = f*cj_speed
    speed.append(u_shock)
    gas = PostShock_eq(u_shock,P1, T1, q, mech)
    # Evaluate properties of gas object 
    vs.append(1./gas.density)
    ps.append(gas.P)
    print('   U/U_cj '+str(f))
    # Print out
    gas3 = ct.Solution(mech)
    [p3,UR,gas3] = reflected_eq(gas1,gas,gas3,u_shock)
    pr.append(p3)
    vr.append(1./gas3.density)
    

# print output file
fn = 'overdriven_reflection.txt'
d = datetime.date.today()
fid = open(fn, 'w')
fid.write('# Reflection of Overdriven Detonation\n')
fid.write('# Calculation run on %s\n' % d )
fid.write('# Initial conditions\n')
fid.write('# Temperature (K) %4.1f\n' % T1)
fid.write('# Pressure (Pa) %2.1f\n' % P1)
fid.write('# Density (kg/m^3) %1.4e\n' % gas1.density)
fid.write('# Initial species mole fractions: '+q+'\n')
fid.write('# Reaction mechanism: '+mech+'\n\n')
fid.write('Variables = "detonation speed (m/s)", "detonation pressure (Pa)", "detonation volume (m3/kg)", "reflected shock pressure (Pa)", "reflected shock volume (m3/kg)"\n')

for i in range(npoints):
     fid.write('%14.5e 	 %14.5e 	 %14.5e 	 %14.5e 	 %14.5e \n' % (speed[i], ps[i], vs[i], pr[i], vr[i]))
fid.close()
