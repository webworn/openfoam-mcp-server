"""
Shock and Detonation Toolbox Demo Program

Creates plot for equilibrium product Hugoniot curve near CJ point, 
shows Rayleigh Line with slope Ucj and four isentropes bracketing CJ point. 
Creates plot showing Gruneisen coefficient, denominator in Jouguet's rule, isentrope slope.

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
from sdtoolbox.postshock import CJspeed, PostShock_eq, hug_eq
from sdtoolbox.thermo import gruneisen_eq, soundspeed_eq
from scipy.optimize import fsolve
import numpy as np
import matplotlib.pyplot as plt

print('demo_RH_CJ_isentropes')
## Input data
P1 = 100000; T1 = 300;
q = 'H2:2 O2:1 N2:3.76'   
mech = 'Mevel2017.cti'
  
## compute initial state
gas1 = ct.Solution(mech)
gas1.TPX = T1,P1,q
h1 = gas1.enthalpy_mass
r1 = gas1.density
v1 = 1/r1

## compute CJ speed
cj_speed = CJspeed(P1, T1, q, mech);
print('CJ speed '+str(cj_speed))

## compute CJ state
gas = PostShock_eq(cj_speed,P1, T1, q, mech)
vcj = 1/gas.density
pcj = gas.P
Pcj = pcj/ct.one_atm
tcj = gas.T
scj = gas.entropy_mass
qcj = gas.X
U1 = cj_speed

## Compute RAYLEIGH Line
vR = []; PR = [];

vmin = 0.3*vcj
vmax = 1.7*vcj
vinc = .01*vcj
vsteps = int((vmax-vmin)/vinc)

for v2 in np.linspace(vmin,vmax,num=vsteps):
    vR.append(v2) 
    PRpa = (P1 - r1**2*U1**2*(v2 - v1))
    PR.append(PRpa/ct.one_atm) # Pressure in atmospheres  

print('Rayleigh Line Array Created')

## Compute product Hugoniot 
# Use CV combust as initial state 
gas.TPX = T1,P1,q
gas.equilibrate('UV')
Ta = gas.T
va = 1/gas.density

PH2 = [gas.P/ct.one_atm]
vH2 = [va]
Grun = [gruneisen_eq(gas)]
gamma = [(soundspeed_eq(gas))**2/(vH2[0]*gas.P)]
denom = [1 + Grun[0]*(vH2[0]-v1)/(2*vH2[0])]

i = 0
vb = va
while vb > vmin:
    i = i + 1
    vb = va - i*.01
    # use new volume and previous temperature to find next state
    fval = fsolve(hug_eq,Ta,args=(vb,h1,P1,v1,gas))
    gas.TD = fval,1/vb
    
    PH2.append(gas.P/ct.one_atm)
    vH2.append(vb)
    Grun.append(gruneisen_eq(gas))
    gamma.append((soundspeed_eq(gas))**2/(vH2[-1]*gas.P))
    denom.append(1 + Grun[-1]*(vH2[-1]-v1)/(2*vH2[-1]))


print('Product Hugoniot Array Created')

## isentropes near CJ point
gas.TPX = tcj,pcj,qcj
s = scj*.98
deltas = 0.01*scj

jmax = 4
PR = np.asarray(PR)

# initialize arrays
Ps_f = np.zeros((len(PR),jmax))
Ps_e = np.copy(Ps_f)
vs_f = np.copy(Ps_f)
vs_e = np.copy(Ps_f)

for j in range(jmax):
    for i,pp in enumerate(PR*ct.one_atm):
        # frozen (CJ composition)
        gas.SPX = s,pp,qcj
        Ps_f[i,j] = gas.P/ct.one_atm
        vs_f[i,j] = 1/gas.density
        # equilibrium 
        gas.equilibrate('SP')
        Ps_e[i,j] =gas.P/ct.one_atm;
        vs_e[i,j] = 1/gas.density        
    s = s + deltas

print('Isentropes created')

## plot creation
maxP = max(PR)
minP = min(PR)
maxV = max(vR)
minV = min(vR)

plt.figure(1)
plt.plot(vR,PR,'k:',linewidth=2);  
plt.plot(vcj,Pcj,'ko',linewidth=2)
plt.plot(vH2,PH2,'r',linewidth=2)

for j in range(jmax):
    plt.plot(vs_e[:,j],Ps_e[:,j],'b',linewidth=2)


plt.axis([minV,maxV,minP,maxP])
plt.xlabel(r'specific volume (m$^3$/kg)',fontsize=14);
plt.ylabel('pressure (atm)',fontsize=14);
plt.legend(['Rayleigh','CJ','Hugoniot','Isentropes'])
plt.tick_params(labelsize=12)


plt.figure(2)
plt.plot(vH2,Grun,'r',linewidth=2)
plt.plot(vH2,denom,'b',linewidth=2)
plt.plot(vH2,gamma,'g',linewidth=2)
plt.axis([minV,maxV,0,1.5])
plt.xlabel(r'specific volume (m$^3$/kg)',fontsize=14)
plt.ylabel('Coefficent',fontsize=14)
plt.legend(['Gruneisen', 'denominator', 'gamma'])
plt.tick_params(labelsize=12)

plt.show()
