"""
Shock and Detonation Toolbox Demo Program

Calculates Prandtl-Meyer function and polar expanded from CJ state.  
For consistency, if an equilibrium expansion is used, the equilibrium sound speed sound 
should be used  in computing the Mach number. Likewise, if the expansion is frozen 
(fixed composition), use the frozen sound speed in computing Mach number.

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
from sdtoolbox.thermo import soundspeed_eq,soundspeed_fr
import cantera as ct
import numpy as np
import matplotlib.pyplot as plt
from sys import exit

##
#  SET TYPE OF EXPANSION COMPUTATION 
# Frozen:  EQ = False   Equilibrium: EQ =  True
EQ = True
##
# set the initial state and compute properties
P1 = 100000.; T1 = 300. 
q = 'O2:1 N2:3.76 H2:2'   
mech = 'Mevel2017.cti'
gas = ct.Solution(mech)

# Find CJ speed
cj_speed = CJspeed(P1, T1, q, mech)
# Evaluate gas state
gas = PostShock_eq(cj_speed,P1, T1, q, mech)

if EQ:
    # use this for equilibrium expansion
    gas.equilibrate('TP') 
    a1 = soundspeed_eq(gas)
else:
    # use this for frozen expansion
    a1 = soundspeed_fr(gas)  

x1 = gas.X
rho1 = gas.density
v1 = 1/rho1
s1 = gas.entropy_mass
h1 = gas.enthalpy_mass
##
# Set freestream velocity .01% above sound speed
U1 = a1*1.0001
mu_min = np.arcsin(a1/U1)  # Mach angle
# check to see if flow speed is supersonic
if U1 < a1:
    exit('Flow subsonic. Exiting.')

# stagnation enthalpy
h0 = h1 + U1**2/2
# estimate maximum  speed
wmax = np.sqrt(2*h0)
# initialize variables for computing PM properties and plotting
v2 = 200*v1
rho = []; a = []; h = []; u = []; M = []; mu = []; T = []; P = []; streamwidth = [];
##
# Compute expansion conditions over range from minimum to maximum specific volume.  
# User can adjust the increment and endpoint to get a smooth output curve and reliable integral.
# Could also try a logarithmic range for some cases. 
for v in np.linspace(v1,v2,num=1000):
    rho.append(1/v)
    x = gas.X   # update mole fractions based on last gas state
    gas.SVX = s1,1/rho[-1],x
    if EQ:
        # required  for equilibrium expansion
        gas.equilibrate('SV')   
        a.append(soundspeed_eq(gas))
    else:
        # use this for frozen expansion
        a.append(soundspeed_fr(gas))

    h.append(gas.enthalpy_mass)
    u.append(np.sqrt(2*(h0-h[-1])))
    M.append(u[-1]/a[-1])
    if M[-1] < 1:
        # Sometimes the first M value has been observed to be ~0.999 which leads to errors
        # in the arcsin and sqrt functions that follow
        M[-1] = 1.00001
    mu.append(np.arcsin(1/M[-1]))
    T.append(gas.T)
    P.append(gas.P)
    streamwidth.append(rho1*U1/u[-1]/rho[-1])


# convert lists to Numpy arrays to allow vectorized operations
M = np.asarray(M); rho = np.asarray(rho); P = np.asarray(P)

# define integrand for PM function and integrate with trapezoidal rule
# compute angle alpha of straight characteristics - measure clockwise from horizontal direction.
integrand = -1*np.sqrt(M**2-1)/M**2/rho

omega = [0.]
alpha = [-1.*np.arcsin(1/M[0])]
time = [0.]
X = [0.]
Y = [-1e-2]
R = [np.sqrt(X[0]**2+Y[0]**2)]



for n in range(1,len(rho)):
    omega.append(omega[n-1] +(integrand[n-1] + integrand[n])*(rho[n]-rho[n-1])/2.)
    alpha.append(omega[n]-mu[n])
    U = 0.5*(u[n] + u[n-1])
    deltaalpha = abs(alpha[n] - alpha[n-1])
    theta = 0.5*(omega[n] + omega[n-1])
    Alpha = 0.5*(alpha[n] + alpha[n-1])
    deltat = R[n-1]*deltaalpha/(U*np.sin(theta-Alpha))
    Ux = U*np.cos(theta)
    Uy = U*np.sin(theta)
    time.append(time[n-1] + deltat)
    X.append(X[n-1] + Ux*deltat)
    Y.append(Y[n-1] + Uy*deltat)
    R.append(np.sqrt(X[n]**2 + Y[n]**2))

omega = np.asarray(omega); alpha = np.asarray(alpha);                 
# plot pressure-deflection polar
plt.figure(1)
plt.plot(180*omega/np.pi,P/P1,linewidth=2) 
plt.title('PM polar, post- CJ state, wave frame',fontsize=12)
plt.xlabel('flow deflection (deg)',fontsize=12)
plt.ylabel('pressure P/P1',fontsize=12)
plt.tick_params(labelsize=12)
plt.xlim(0,90)
plt.ylim(ymin=0)

# plot PM function-Mach number
plt.figure(2)
plt.plot(M,180*omega/np.pi,linewidth=2)
plt.title('PM Function, post- CJ state, wave frame',fontsize=12)
plt.xlabel('Mach number',fontsize=12)
plt.ylabel('PM function (deg)',fontsize=12)
plt.tick_params(labelsize=12)
plt.xlim(xmin=1)
plt.ylim(ymin=0)

# plot alpha-Mach number
plt.figure(3)
plt.plot(M,180*alpha/np.pi,linewidth=2)
plt.title('alpha, post- CJ state, wave frame',fontsize=12)
plt.xlabel('Mach number',fontsize=12)
plt.ylabel('alpha (deg)',fontsize=12)
plt.tick_params(labelsize=12)
plt.xlim(xmin=1)

# plot normalized stream width vs Mach number
plt.figure(4)
plt.plot(M,streamwidth,linewidth=2)
plt.title('stream tube width post- CJ state, wave frame',fontsize=12)
plt.xlabel('Mach number',fontsize=12)
plt.ylabel('width',fontsize=12)
plt.tick_params(labelsize=12)
plt.xlim(xmin=1)
plt.ylim(ymin=0)

# plot particle path
plt.figure(5)
plt.plot(X,Y,linewidth=2)
plt.title('X-Y in wave frame',fontsize=12)
plt.xlabel('X',fontsize=12)
plt.ylabel('Y',fontsize=12)
plt.tick_params(labelsize=12)
plt.xlim(xmin=0)
plt.ylim(ymin=0)


# plot particle path time
plt.figure(6)
plt.plot(M,time,linewidth=2)  
plt.title('time vs M in wave frame',fontsize=12)
plt.xlabel('M',fontsize=12)
plt.ylabel('time',fontsize=12)
plt.tick_params(labelsize=12)
plt.xlim(xmin=1)
plt.ylim(ymin=0)
plt.ticklabel_format(style='sci',axis='y',scilimits=(0,0))

# plot particle path in alpha-radius coordinates
plt.figure(7)
plt.polar(alpha,R)  
plt.title('radius vs alpha in wave frame',fontsize=12)
plt.tick_params(labelsize=12)
plt.ylim(ymin=0)
plt.tight_layout()

plt.show()
