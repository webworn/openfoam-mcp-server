"""
Shock and Detonation Toolbox Demo Program

Creates arrays for frozen Hugoniot curve for shock wave in air, Rayleigh Line 
with specified shock speed, and four representative isentropes.  
Options to create plot and output file.

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
from sdtoolbox.thermo import soundspeed_fr
import numpy as np
import matplotlib.pyplot as plt

# Initial conditions
# P1 = Initial Pressure
# T1 = Initial Temperature
# q = Initial Composition MUST use capital letters - STRING
# mech = Mechanism File name in CTI format - STRING
# fname =  output file name (use zero '0' for no files)
# U1 = shock speed
# plt_num = 0 no plot,  =1 plot
P1 = 100000; T1 = 300;
q = 'O2:0.2095 N2:0.7808 CO2:0.0004 Ar:0.0093'  
mech = 'airNASA9ions.cti'
fname = 'AirFr'
U1 = 1000.
plt_num = 1
# EDIT VALUES ABOVE THIS LINE
##############################
# set initial state
gas1 = ct.Solution(mech);
gas1.TPX = T1, P1, q
h1 = gas1.enthalpy_mass
r1 = gas1.density
v1 = 1.0/gas1.density
s1 = gas1.s

#Get postshock state
gas = PostShock_fr(U1,P1,T1,q,mech)
v_ps = 1.0/gas.density
P_ps = gas.P/ct.one_atm

# RAYLEIGH LINE, 
minv = 0.9*v_ps; maxv = 1.00*v1; stepv = 0.01*v1;
n = np.int((maxv-minv)/stepv);
vR = np.zeros(n,float);
PR = np.zeros(n,float); 

i = 0
v2  = maxv
while(i < n):
    vR[i] = v2;
    PRpa = (P1 - r1**2*U1**2*(v2-v1))
    PR[i] = PRpa/ct.one_atm
    i = i + 1; v2 = v2 - stepv
print ('Rayleigh Line Array Created')

# Frozen (reactant) HUGONIOT
n = 50
PH1 = np.zeros(n+1,float);
vH1 = np.zeros(n+1,float);

PH1[0] = P1/ct.one_atm; vH1[0] = v1

Umin = 1.1*soundspeed_fr(gas1)

stepU = (1.1*U1-Umin)/float(n)
i = 0
while(i < n):
    U = Umin + stepU*float(i)
    gas = PostShock_fr(U, P1, T1, q, mech)
    PH1[i+1] = gas.P/ct.one_atm; vH1[i+1] = 1/gas.density
    i = i + 1

print ('Reactant Hugoniot Array Created')

# Compute four frozen isentropes
# one passing through initial state
# one passing through postshock state
# two in between
gas = PostShock_fr(U1,P1,T1,q,mech)
Ph = gas.P
Sh = gas.s
Th = gas.T
qh = gas.X
mult = 0.95
imax = int(np.log(P1/Ph)/np.log(mult))
deltas = (Sh - s1)/3.
s = s1
Ps = np.zeros((imax,4),float)
vs = np.zeros((imax,4),float)
for j in range(0,4):
    pp = Ph*1.15
    for i in range(0,imax):
        gas.SP = s, pp
        Ps[i,j] = gas.P/ct.one_atm
        vs[i,j] = 1/gas.density
        pp = pp*mult;
    s = s + deltas;
    print('Isentrope Array Created, s = %.2f' %(s))

if(fname == 0):
    print ('No output files created')
else:
    #Create Output Data File for Tecplot
    import datetime
    a = np.size(PR)-1; k = 0;
    d = datetime.date.today(); P = P1/ct.one_atm

    fn = fname + '_%d_RH_air_s_R.txt' % U1
    outfile = open(fn, 'w')
    outfile.write('# RAYLEIGH LINE FOR GIVEN SHOCK SPEED\n')
    outfile.write('# CALCULATION RUN ON %s\n\n' % d)
    outfile.write('# INITIAL CONDITIONS\n')
    outfile.write('# TEMPERATURE (K) %.1f\n' % T1)
    outfile.write('# PRESSURE (ATM) %.1f\n' % P)
    outfile.write('# DENSITY (KG/M^3) %.4f\n' % r1)
    outfile.write('# SPECIES MOLE FRACTIONS: ' + q + '\n')
    outfile.write('# MECHANISM: ' + mech + '\n')
    outfile.write('# SHOCK SPEED (M/S) %.2f\n\n' % U1)
    outfile.write('# THE OUTPUT DATA COLUMNS ARE:\n')
    outfile.write('Variables = "Specific Volume", "Pressure"\n')
    while k <= a:
        outfile.write('%.4E \t %.4E \n' % (vR[k], PR[k]))
        k = k + 1
    outfile.close()
   
    a = np.size(PH1)-1; k = 0;
    fn = fname + '_RH_air_s_H_fr.txt' 
    outfile = open(fn, 'w')
    outfile.write('# Frozen HUGONIOT FOR GIVEN MIXTURE\n')
    outfile.write('# CALCULATION RUN ON %s\n\n' % d)
    outfile.write('# INITIAL CONDITIONS\n')
    outfile.write('# TEMPERATURE (K) %.1f\n' % T1)
    outfile.write('# PRESSURE (ATM) %.1f\n' % P)
    outfile.write('# DENSITY (KG/M^3) %.4f\n' % r1)
    outfile.write('# SPECIES MOLE FRACTIONS: ' + q + '\n')
    outfile.write('# MECHANISM: ' + mech + '\n\n')
    outfile.write('# THE OUTPUT DATA COLUMNS ARE:\n')
    outfile.write('Variables = "Specific Volume", "Pressure"\n')
    while k <= a:
        outfile.write('%.4E \t %.4E\n' % (vH1[k], PH1[k]))
        k = k + 1
    outfile.close()
    
if (plt_num == 0):
    print ('No plot created')
else:
    vv = np.zeros(imax,float)
    Pps = np.zeros(imax,float)
    xmin = 0
    xmax = max(np.amax(vR),np.amax(vH1))
    ymin = 0
    ymax = max(np.amax(PR),np.amax(PH1))
    fig, ax = plt.subplots()
    line1, = ax.plot(vR,PR,label="Rayleigh")
    line2, = ax.plot(vH1,PH1,'k',label="Fr Hugoniot")
    s = s1
    for j in range(0,4):
        for i in range(0, imax):
            vv[i] = vs[i,j]
            Pps[i] = Ps[i,j]
        entropy = 's = ' + '%d J/kg K'% s
        line3, = ax.plot(vv,Pps,label=entropy)
        s = s + deltas;
    ax.set_title('Shock in air, shock speed = '+str(U1)+' m/s')
    ax.set_xlabel(r'volume (m$^3$/kg)')
    ax.set_ylabel('pressure (atm)')
    ax.set_xlim([xmin,xmax])
    ax.set_ylim([ymin,ymax])
    ax.legend()
    plt.show()
   
