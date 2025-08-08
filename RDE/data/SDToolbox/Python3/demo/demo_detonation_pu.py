"""
Shock and Detonation Toolbox Demo Program

Calculate the hugoniot and p-u relationship for shock waves centered on
the CJ state. Generates an output file.

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

from sdtoolbox.postshock import CJspeed, PostShock_eq
from sdtoolbox.thermo import soundspeed_eq
from sdtoolbox.reflections import reflected_eq
import cantera as ct
import numpy as np
import datetime

# set initial state, composition, and gas object
P1 = 100000
T1 = 300
q = 'H2:0.31 N2O:0.69'    
mech = 'Mevel2015.cti'
gas1 = ct.Solution(mech)

gas1.TPX = T1,P1,q
rho1 = gas1.density

# Find CJ speed
cj_speed = CJspeed(P1, T1, q, mech)

# CJ state
gas = PostShock_eq(cj_speed,P1, T1, q, mech)
print('CJ computation for '+mech+' with composition ',q)
P2 = gas.P
T2 = gas.T
q2 = gas.X
rho2 = gas.density
w2 = rho1/rho2*cj_speed
u2= cj_speed-w2
Umin = soundspeed_eq(gas)
print('CJ speed '+str(cj_speed)+' (m/s)');
print('CJ State');
print('   Pressure '+str(P2)+' (Pa)');
print('   Particle velocity '+str(u2)+' (m/s)');

# reflected shock from CJ detonation
gas3 = ct.Solution(mech);
[p3,Ur,gas3] = reflected_eq(gas1,gas,gas3,cj_speed);
v3 = 1./gas.density
print('Reflected shock (equilibrium) computation for ',mech,' with composition ',q)
print('   Reflected wave speed '+str(Ur)+' (m/s)')
print('   Reflected shock pressure '+str(gas3.P)+' (Pa)')
# Bounds for reflected shock speed
Umax = Ur+u2
print('   Maximum reflected shock speed '+str(Umax)+' (m/s)')
print('   Minimum reflected shock speed '+str(Umin)+' (m/s)')
print('Compute shock adiabat and wave curve starting at the CJ state')



step = 10 # approximate desired step size (will not be exactly followed)
npoints = int((Umax-Umin)/step)
p = []; v = []; u3 = [];
for ushock in np.linspace(Umin,Umax,num=npoints):
    gas = PostShock_eq(ushock, P2, T2, q2, mech);
    p.append(gas.P)
    v.append(1/gas.density)
    u3.append(u2-(ushock - ushock*rho2/gas.density))
    print('   shock velocity '+str(ushock)+' (m/s)')



print('Final Point (should be rigid wall reflection point)')
p[-1] = p3
v[-1] = v3
u3[-1] = 0.
print('   shock velocity '+str(ushock)+' (m/s)')
print('   post shock pressure '+str(p[-1])+' (Pa)')
print('   post shock volume '+str(v[-1])+' (m3/kg)')
print('   post shock velocity '+str(u3[-1])+' (m/s)')

fn = 'cj-pu.txt';
d = datetime.date.today()
fid = open(fn, 'w');
fid.write('# Shock wave centered on CJ Detonation State\n')
fid.write('# Calculation run on %s\n' % d )
fid.write('# Initial conditions\n')
fid.write('# Temperature (K) %4.1f\n' % T1)
fid.write('# Pressure (Pa) %2.1f\n'% P1)
fid.write('# Density (kg/m^3) %1.4e\n'% gas1.density)
fid.write('# Initial species mole fractions: '+q+'\n')
fid.write('# Reaction mechanism: '+mech+'\n\n')
fid.write('Variables = "particle velocity (m/s)", "pressure (Pa)", "volume (m3/kg)"\n')
          
for i in range(npoints):
     fid.write('%14.5E \t %14.5E \t %14.5E \n' % (u3[i], p[i], v[i]))
fid.close()

