"""
Shock and Detonation Toolbox Demo Program

Calculates Prandtl-Meyer function and polar. For consistency, if an equilibrium 
expansion is used, the equilibrium sound speed sound be used in computing the
Mach number. Likewise, if the expansion is frozen (fixed composition), use the 
frozen sound speed in computing Mach number.

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

from sdtoolbox.thermo import soundspeed_eq,soundspeed_fr
import cantera as ct
import numpy as np
import matplotlib.pyplot as plt
from sys import exit

##
#  SET TYPE OF EXPANSION COMPUTATION 
# Frozen:  EQ = false   Equilibrium EQ =  true
EQ = False
##
# set the initial state and compute properties
P1 = 100000.; T1 = 300. 
q = 'O2:0.2095 N2:0.7808 CO2:0.0004 Ar:0.0093';    
mech = 'airNASA9ions.cti'
gas = ct.Solution(mech)
gas.TPX = T1,P1,q

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
U = a1*1.0001
mu_min = np.arcsin(a1/U)  # Mach angle
# check to see if flow speed is supersonic
if U < a1:
    exit('Flow subsonic. Exiting.')

# stagnation enthalpy
h0 = h1 + U**2/2
# estimate maximum  speed
wmax = np.sqrt(2*h0)
# initialize variables for computing PM properties and plotting
v2 = 50*v1
rho = []; a = []; h = []; u = []; M = []; mu = []; T = []; P = [];
##
# Compute expansion conditions over range from minimum to maximum specific volume.  
# User can adjust the increment and endpoint to get a smooth output curve and reliable integral.
# Could also try a logarithmic range for some cases. 
for n,v in enumerate(np.linspace(v1,v2,num=1000)):
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


# convert lists to Numpy arrays to allow vectorized operations
M = np.asarray(M); rho = np.asarray(rho); P = np.asarray(P)

# define integrand for PM function and integrate with trapezoidal rule
integrand = -1*np.sqrt(M**2-1)/M**2/rho

omega = [0.]
for n in range(1,len(rho)):
    omega.append(omega[n-1] +(integrand[n-1] + integrand[n])*(rho[n]-rho[n-1])/2)

omega = np.asarray(omega)                 
# plot pressure-deflection polar
plt.figure(1)
plt.plot(180*omega/np.pi,P/P1,linewidth=2) 
plt.title('PM polar, free-stream speed '+str(round(U,5))+' m/s',fontsize=12)
plt.xlabel('flow deflection (deg)',fontsize=12)
plt.ylabel('pressure P/P1',fontsize=12)
plt.xlim(xmin=0)
plt.ylim(0,1)
plt.tick_params(labelsize=12)

# plot PM function-Mach number
plt.figure(2)
plt.plot(M,180*omega/np.pi,linewidth=2);
plt.title('PM Function, free-stream speed '+str(round(U,5))+' m/s',fontsize=12)
plt.xlabel('Mach number',fontsize=12)
plt.ylabel('PM function (deg)',fontsize=12)
plt.xlim(xmin=1)
plt.ylim(ymin=0)
plt.tick_params(labelsize=12)

plt.show()
