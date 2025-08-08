"""
Shock and Detonation Toolbox Demo Program

Computes detonation and reflected shock wave pressure for overdriven waves.   
Varies density of initial state and detonation wave speed. Creates an output file.

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
from sdtoolbox.reflections import reflected_eq
import cantera as ct
import numpy as np
import datetime

# set initial state, composition, and gas object
P0 = 100000; T0 = 300
q = 'H2:0.31 N2O:0.69'   
mech = 'Mevel2015.cti'
gas0 = ct.Solution(mech)
gas1 = ct.Solution(mech)
gas3 = ct.Solution(mech)
gas0.TPX = T0,P0,q

# Find compressed state (isentropic)
rho0 = gas0.density
s0 = gas0.entropy_mass
x0 = gas0.X

# open output file
# print output file
fn = 'precompressed_detonation_reflection.txt'
d = datetime.date.today()
fid = open(fn, 'w')
fid.write('# Reflection of Overdriven Detonation\n')
fid.write('# Calculation run on %s\n' % d )
fid.write('# Initial species mole fractions: '+q+'\n')
fid.write('# Reaction mechanism: '+mech+'\n\n')
rho0 = 2*rho0

# loop through initial state with isentropic compression
start = rho0; stop = 4*rho0; step = 0.5*rho0
nsteps = int((stop-start)/step)

for rho1 in np.linspace(start,stop,num=nsteps):
    gas1.SVX = s0,1/rho1,x0
    P1 = gas1.P
    T1 = gas1.T

    print('Density '+str(rho1)+' (kg/m^3)')
    fid.write('# Initial conditions\n')
    fid.write('# Temperature (K) %4.1f\n' % T1)
    fid.write('# Pressure (Pa) %2.1f\n' % P1)
    fid.write('# Density (kg/m^3) %1.4e\n' % rho1)

    # Find CJ speed
    cj_speed = CJspeed(P1, T1, q, mech)
    print('CJspeed '+str(cj_speed)+' (m/s)');
    gas = PostShock_eq(cj_speed,P1, T1, q, mech)
    P2 = gas.P
 
    # Evaluate overdriven detonations and reflected shocks
    fstart = 1.; fstop = 1.5; fstep = 0.05
    fnsteps = int((fstop-fstart)/fstep)
    speed = []; vs = []; ps = []; pr = []; vr = []
    for f in np.linspace(fstart,fstop,num=fnsteps):
        u_shock = f*cj_speed
        speed.append(u_shock)
        print('   Detonation Speed '+str(speed[-1])+' (m/s)')
        gas = PostShock_eq(u_shock,P1, T1, q, mech)
        # Evaluate properties of gas object 
        vs.append(1./gas.density)
        ps.append(gas.P)
        [p3,UR,gas3] = reflected_eq(gas1,gas,gas3,u_shock)
        pr.append(p3)
        vr.append(1./gas3.density)


    print('-------------------------------------------');        
    fid.write('Zone T = rho'+str(fnsteps+1)+'\n')
    fid.write('Variables = "detonation speed (m/s)", "detonation pressure (Pa)", "detonation volume (m3/kg)", "reflected shock pressure (Pa)", "reflected shock volume (m3/kg)"\n')
    for i in range(fnsteps):
        fid.write(' %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \n' % (speed[i], ps[i], vs[i], pr[i], vr[i]))

fid.close()
