"""
Shock and Detonation Toolbox Demo Program

Creates arrays for Rayleigh Line with specified shock speed, Reactant, and Product 
Hugoniot Curves for  H2-air mixture.  Options to create output file and plots.

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
from sdtoolbox.postshock import PostShock_fr, PostShock_eq, CJspeed, hug_eq
from sdtoolbox.thermo import soundspeed_fr
from scipy.optimize import fsolve
import numpy as np
import matplotlib.pyplot as plt

# Initial conditions
# P1 = Initial Pressure
# T1 = Initial Temperature
# q = Initial Composition MUST use capital letters - STRING
# mech = Mechanism File name in CTI format - STRING
# fname = Mixture name used for output file names (use zero '0' if
#         no output file desired
# od = Overdrive for Rayleigh Line -- multiplied by CJ speed to find the
#      slope of the Rayleigh Line
# gbound = Overdrive factor to compute bound on the maximum gamma
P1 = 100000; T1 = 300;
q = 'H2:2 O2:1 N2:3.76'
mech  = 'Mevel2017.cti'
fname = 'h2air'
od = 1;
gbound = 1.4
makePlot = True
# EDIT VALUES ABOVE THIS LINE
##############################
gas1 = ct.Solution(mech);
gas1.TPX = T1, P1, q


h1 = gas1.enthalpy_mass
r1 = gas1.density
v1 = 1.0/gas1.density

#Get CJ Point
cj_speed = CJspeed(P1,T1,q,mech)
gas = PostShock_eq(cj_speed,P1,T1,q,mech)
vcj = 1.0/gas.density
Pcj = gas.P/ct.one_atm

print ('CJ Point Found')

U1 = od*cj_speed

#Find Postshock specific volume for U1
gas = PostShock_fr(U1, P1, T1, q, mech)
vsj = 1.0/gas.density
Psj = gas.P/ct.one_atm

#Find Gamma 
gas = PostShock_fr(gbound*cj_speed, P1, T1, q, mech)
g = gas.cp_mass/gas.cv_mass

# RAYLEIGH LINE, SJUMP MIN & MAX, GAMMA CONSTRAINT
minp = 0.1*vcj; maxp = 2.0*vcj; step = 0.01*vcj;
i = 0; v2 = minp; 
n = np.int((maxp-minp)/step);
vR = np.zeros(n,float);
PR = np.zeros(n,float); 

min_line = np.zeros(n,float);
max_line = np.zeros(n,float);
gamma = np.zeros(n,float); 

while(i < n):
    vR[i] = v2;
    PRpa = (P1 - r1**2*U1**2*(v2-v1))
    PR[i] = PRpa/ct.one_atm

    min_line[i] = 1.0/40.0; max_line[i] = 1.0/1.005;
    gamma[i] = (g-1.0)*r1/(g+1.0)
    
    i = i + 1; v2 = v2 + step
print ('Rayleigh Line Array Created')

# Frozen (reactant) HUGONIOT
# The method used for finding the Hugoniot for the reactants is
# very robust.
n = 50
PH1 = np.zeros(n+1,float);
vH1 = np.zeros(n+1,float);

PH1[0] = P1/ct.one_atm; vH1[0] = v1

Umin = 1.1*soundspeed_fr(gas1)

stepU = (1.4*U1-Umin)/float(n)
i = 0
while(i < n):
    U = Umin + stepU*float(i)
    gas = PostShock_fr(U, P1, T1, q, mech)
    PH1[i+1] = gas.P/ct.one_atm; vH1[i+1] = 1/gas.density
    i = i + 1

print ('Reactant Hugoniot Array Created')

gas = PostShock_eq(U1, P1, T1, q, mech)
veq = 1.0/gas.density
Peq = gas.P/ct.one_atm

# PRODUCT HUGONIOT
# The method used for finding the product hugoniot is not
# quite as robust as for reactants so some care must be taken
# at large overdrives.
# Get the first point on the product Hugoniot - CV combustion
gas.TPX = T1, P1, q
gas.equilibrate('UV')
Ta = gas.T
Pcv = gas.P/ct.one_atm
va = 1.0/gas.density

n = np.int((va-0.4*vcj)/0.01);
PH2 = np.zeros(n+1,float);
vH2 = np.zeros(n+1,float);

PH2[0] = gas.P/ct.one_atm; vH2[0] = va;

i = 0; vb = va;
while(i < n):
    vb = va - (i+1)*0.01
    #Always starts at new volume and previous temperature to find next state
    fval = fsolve(hug_eq,Ta,args=(vb,h1,P1,v1,gas))
    gas.TD = fval, 1.0/vb
    PH2[i+1] = gas.P/ct.one_atm; vH2[i+1] = vb
    i = i + 1

print ('Product Hugoniot Array Created')

if(fname == 0):
    print ('No output files created')
else:
    #Create Output Data File for Tecplot
    import datetime
    a = np.size(PR)-1; k = 0;
    d = datetime.date.today(); P = P1/ct.one_atm

    fn = fname + '_%d_R.txt' % U1
    outfile = open(fn, 'w')
    outfile.write('# RAYLEIGH LINE, SJUMP MAX & MIN, AND GAMMA CONSTRAINT FOR GIVEN SHOCK SPEED\n')
    outfile.write('# CALCULATION RUN ON %s\n\n' % d)
    outfile.write('# INITIAL CONDITIONS\n')
    outfile.write('# TEMPERATURE (K) %.1f\n' % T1)
    outfile.write('# PRESSURE (ATM) %.1f\n' % P)
    outfile.write('# DENSITY (KG/M^3) %.4f\n' % r1)
    outfile.write('# SPECIES MOLE FRACTIONS: ' + q + '\n')
    outfile.write('# MECHANISM: ' + mech + '\n')
    outfile.write('# SHOCK SPEED (M/S) %.2f\n\n' % U1)
    outfile.write('# THE OUTPUT DATA COLUMNS ARE:\n')
    outfile.write('Variables = "Specific Volume", "Pressure", "Max Line", "Min Line", "Gamma Line"\n')
    while k <= a:
        outfile.write('%.4E \t %.1f \t %.4f \t %.4f \t %.4f\n' % (vR[k], PR[k], min_line[k], max_line[k], gamma[k]))
        k = k + 1
    outfile.close()
   
    a = np.size(PH1)-1; k = 0;
    fn = fname + '_%d_RH_H_fr.txt' % U1
    outfile = open(fn, 'w')
    outfile.write('# REACTANT HUGONIOT FOR GIVEN MIXTURE\n')
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
        outfile.write('%.4E \t %.1f\n' % (vH1[k], PH1[k]))
        k = k + 1
    outfile.close()
    
    a = np.size(PH2)-1; k = 0;
    fn = fname + '_%d_RH_H_eq.txt' % U1
    outfile = open(fn, 'w')
    outfile.write('# PRODUCT HUGONIOT FOR GIVEN MIXTURE\n')
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
        outfile.write('%.4E \t %.1f\n' % (vH2[k], PH2[k]))
        k = k + 1
    outfile.close()
    
    fn = fname + '_%d_RH_points.txt' % U1
    outfile = open(fn, 'w')
    outfile.write('# SPECIFIC POINTS FOR GIVEN SHOCK SPEED\n')
    outfile.write('# CALCULATION RUN ON %s\n\n' % d)
    outfile.write('# INITIAL CONDITIONS\n')
    outfile.write('# TEMPERATURE (K) %.1f\n' % T1)
    outfile.write('# PRESSURE (ATM) %.1f\n' % P)
    outfile.write('# DENSITY (KG/M^3) %.4f\n' % r1)
    outfile.write('# SPECIES MOLE FRACTIONS: ' + q + '\n')
    outfile.write('# MECHANISM: ' + mech + '\n')
    outfile.write('# SHOCK SPEED (M/S) %.2f\n\n' % U1)
    outfile.write('# THE OUTPUT DATA COLUMNS ARE:\n')
    outfile.write('# ORDER = Initial, PostShock, CJ, CV\n')
    outfile.write('Variables = "Specific Volume", "Pressure"\n')
    outfile.write('%.4E \t %.1f\n' % (v1, P))
    outfile.write('%.4E \t %.1f\n' % (vsj, Psj))
    outfile.write('%.4E \t %.1f\n' % (vcj, Pcj))
    outfile.write('%.4E \t %.1f\n' % (va, Pcv))
    outfile.close()

if makePlot:
    xmin = 0
    xmax = max(np.amax(vR),np.amax(vH1),np.amax(vH2))
    ymin = 0
    ymax = max(np.amax(PR),np.amax(PH1),np.amax(PH2))
    fig, ax = plt.subplots()
    line1, = ax.plot(vR,PR,label="Rayleigh")
    line2, = ax.plot(vH1,PH1,label="Reac Hugoniot")
    line3, = ax.plot(vH2,PH2,label="Prod Hugoniot")
    line4, = ax.plot(vcj,Pcj,marker='s',linestyle='none',label="CJ")
    ax.set_title('Detonation, CJ speed ='+str(cj_speed))
    ax.set_xlabel('volume')
    ax.set_ylabel('pressure')
    ax.set_xlim([xmin,xmax])
    ax.set_ylim([ymin,ymax])
    ax.legend()
    plt.show()  
