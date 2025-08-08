"""
Shock and Detonation Toolbox Demo Program

Generates plots and output files for a constant volume
explosion simulation where the initial conditions are adiabaically
compressed reactants. The time dependence of species, pressure, and
temperature are computed using the user supplied reaction mechanism file.

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
from sdtoolbox.cv import cvsolve
import matplotlib.pyplot as plt
import numpy as np
import datetime

# Define initial state (uncompressed)
P1 = ct.one_atm; T1 = 300  
q = 'H2:2 O2:1 N2:3.76'
#mechanism = 'Mevel2015';
#mechanism = 'Hong2011';
#mechanism='Keromnes2013';
#mechanism='Burke2012';
mechanism='sandiego20161214_H2only';

mech = mechanism+'.cti'
file = True  # set true or false as needed
file_name = 'compression_ignition_'+mechanism

##
# SET UP GAS OBJECT
gas = ct.Solution(mech)
gas.TPX = T1,P1,q
rho1 = gas.density
s1 = gas.entropy_mass

print('Adiabatic compression CV explosion computation for '+mech+' with composition '+q)
print('Initial (uncompressed) State')
print('   Pressure '+str(P1)+' (Pa)')
print('   Temperature '+str(T1)+' (K)')
print('   Density '+str(rho1)+' (kg/m3)')
if file:
    ##################################################################################################
    # CREATE OUTPUT TEXT FILE
    ##################################################################################################
    fn = file_name+'.txt'
    d = datetime.date.today()

    fid = open(fn, 'w')
    fid.write('# CV: EXPLOSION STRUCTURE CALCULATION\n')
    fid.write('# CALCULATION RUN ON %s \n\n' % d)
    fid.write('# Uncompressed state \n')
    fid.write('# TEMPERATURE (K) %4.1f \n' % T1)
    fid.write('# PRESSURE (ATM) %2.1f \n' % (P1/ct.one_atm))
    fid.write('# DENSITY (KG/M^3) %1.4e \n' % rho1)
    fid.write('# SPECIES MOLE FRACTIONS: '+q+'\n')
    fid.write('# Compressed state:  T(K), P(MPa), t_ind(s), t_pulse(s)\n')

##
# Define a series of post-compression states by isentropic compression of initial state
T2 = []; P2 = []; rho2 = []; t_ind = []; t_pulse = []
step = 5; start = 10; stop = 60
nsteps = int((stop-start)/step)
for compression_ratio in np.linspace(start,stop,num=nsteps):
    gas.SVX = s1,1/(compression_ratio*rho1),q
    T2.append(gas.T)
    P2.append(gas.P)
    rho2.append(gas.density)
    
    print('Compressed State')
    print('   Pressure '+str(P2[-1])+' (Pa)')
    print('   Temperature '+str(T2[-1])+' (K)')
    print('   Density '+str(rho2[-1])+' (kg/m3)')

    # Compute constant-volume explosion
    # need to set final time sufficiently long for low temperature cases
    CVout = cvsolve(gas,t_end=50.,max_step=1)
    t_ind.append(CVout['ind_time'])
    t_pulse.append(CVout['exo_time'])
    print('   Induction time = '+str(t_ind[-1])+' (s)')
    print('   Pulse time  = '+str(t_pulse[-1])+' (s)')
    
    if file:
        y = (T2[-1], P2[-1]/1e6, t_ind[-1], t_pulse[-1])
        fid.write('%1.4g \t %6.2f \t %3.2e \t %3.2e\n' % y)


if file:
    fid.close()

P2 = np.asarray(P2); T2 = np.asarray(T2)
# plot induction time vs reciprocal temperature
plt.figure(1)
Tbar = 1e3/T2
maxy = max(t_ind)
miny = min(t_ind)
maxx = max(Tbar)
minx = min(Tbar)
fontsize=12
plt.axis([minx,maxx,miny,maxy])
plt.semilogy(Tbar,t_ind)
plt.xlabel('1000/T',fontsize=fontsize)
plt.ylabel('induction time (s)',fontsize=fontsize)
plt.title('Adiabatic compression CV ignition',fontsize=fontsize)

# plot induction time vs reciprocal temperature
plt.figure(2)
P = P2/1e6
maxy = max(P)
miny = min(P)
maxx = max(T2)
minx = min(T2)
plt.axis([minx,maxx,miny,maxy])
plt.plot(T2,P)
plt.xlabel('temperature (K)',fontsize=fontsize)
plt.ylabel('pressure(MPa)',fontsize=fontsize)
plt.title('Adiabatic compression CV ignition',fontsize=fontsize)
