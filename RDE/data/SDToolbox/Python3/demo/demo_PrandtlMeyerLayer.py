"""
Shock and Detonation Toolbox Demo Program

Calculates Prandtl-Meyer function and polar originating from lower layer postshock state. 
Calculates oblique shock wave moving into expanded detonation products or a 
specified bounding atmosphere.  
Two-layer version with arbitrary flow in lower layer (1), oblique wave in upper layer (2).  
Upper and lower layers can have various compositions as set by user.

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
from sdtoolbox.postshock import PostShock_eq, PostShock_fr
import cantera as ct
import numpy as np
import matplotlib.pyplot as plt
from sys import exit

# Suppress the 'divide by zero' warning encountered for the last data point in the arctan calls
# (where U = w) so that we can keep the code as similar as possible to the MATLAB equivalent
# Numpy actually handles this case and returns the correct value for the limit arctan(inf)
import warnings
warnings.filterwarnings("ignore", category=RuntimeWarning)

##
#  SET TYPE OF SHOCK COMPUTATION 
# Frozen:  EQ = False   Equilibrium EQ =  True
EQ = False
##
# set the initial state and compute properties
PA = 100000.; TA = 1000.  # layer state
q = 'PG:1'
mech = 'PG14.cti'

gas = ct.Solution(mech)
gas.TPX = TA,PA,q

x1 = gas.X
P1 = gas.P
T1 = gas.T
rho1 = gas.density
v1 = 1/rho1
a1 = soundspeed_fr(gas) 
s1 = gas.entropy_mass
h1 = gas.enthalpy_mass
RU = ct.gas_constant
W = gas.mean_molecular_weight
R = RU/W
##
# Set lower layer velocity using the Mach number
M2 = 1.05
layer_speed = a1*M2
U = layer_speed
mu_min = np.arcsin(a1/U) # Mach angle
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
for v in np.linspace(v1,v2,num=1000):
    rho.append(1/v)
    x = gas.X   # update mole fractions based on last gas state
    
    # workaround to avoid unsized object error when only one species in a .cti file
    # (flagged to be fixed in future Cantera version)
    if gas.n_species > 1:
        gas.SVX = s1,1/rho[-1],x
    else:
        gas.SV = s1,1/rho[-1]
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
    mu.append(np.arcsin(1/M[-1]))
    T.append(gas.T)
    P.append(gas.P)


# convert lists to Numpy arrays to allow vectorized operations
M = np.asarray(M); rho = np.asarray(rho); P = np.asarray(P) 

# define integrand for PM function and integrate with trapezoidal rule

integrand = -1*np.sqrt(M**2-1)/M**2/rho

omega = [0.]   
for n in range(1,len(rho)):
    omega.append(omega[n-1] +(integrand[n-1] + integrand[n])*(rho[n]-rho[n-1])/2.)

 
##
# compute shock polar
# First compute upstream state
# for RDE model, expand detonation products to ambient pressure
if EQ:
   gas.SPX = s1,PA,x
   gas.equilibrate('SP')
   cs = soundspeed_eq(gas); # use 'cs' instead of 'as' since latter is reserved keyword
else:
    #alternative state upstream: reactants  (need to use frozen shock calculation with this)
    #qs = 'C2H4:1.00 O2:3';     %ethylene oxygen
    #qs = 'O2:1.00 N2:3.76';     %combustion air
    qs = ' PG:1';
    # Hans case 1
    Pb = PA/25        # lower than PA for solutions to exist 
    rhob = rho1/2
    Tb = Pb/R/rhob
    # Alternative is to set Tb directly
    gas.TPX = Tb,Pb,qs
    cs = soundspeed_fr(gas)

##
# states upstream of shock
rhos  = gas.density
Ps = gas.P
Ts = gas.T
xs = gas.X
cs = soundspeed_fr(gas)

## 
# oblique shock computation
# Set freestream velocity using Mach number and find limiting speeds
M2 = 2.4
US = M2*cs
beta_min = np.arcsin(cs/US) # Mach angle
wmin = cs + 1. # start just above sound speed
# check to see if shock speed is above sound speed
if US < cs:
    exit('Shock speed below sound speed. Exiting.')

# set maximum normal speed
wmax = US
# initialize variables for plotting
rho2 = []; w1 = []; w2 = []; u2 = []; vt = []; a2 = []; P2 = []; beta = []; theta = []; 

##
# compute shock jump conditions over range from minimum to maximum normal
# speeds.  Adjust the increment to get a smooth output curve.

step = 5 # approximate desired step size (will not be exactly followed)
npoints = int((wmax-wmin)/step)

for w in np.linspace(wmin,wmax,num=npoints): 
    if EQ:
        # equilibrium state
        gas = PostShock_eq(w, Ps, Ts, xs, mech)
        a2.append(soundspeed_eq(gas))
    else:
        # for non-reactive or cold upstream state, use frozen shock calculation
        gas = PostShock_fr(w, Ps, Ts, qs, mech)
        a2.append(soundspeed_fr(gas))

    rho2.append(gas.density)
    ratio = rhos/rho2[-1]
    w1.append(w)
    w2.append(w*ratio)
    P2.append(gas.P)
    beta.append(np.arcsin(w/US))
    vt.append(US*np.cos(beta[-1]))
    theta.append(beta[-1] - np.arctan(w2[-1]/np.sqrt(US**2-w**2)))
    u2.append(np.sqrt(w2[-1]**2 + vt[-1]**2))


# pick up normal shock as last point
w = US
if EQ:
    # equilibrium state
    gas = PostShock_eq(w, Ps, Ts, xs, mech)
    a2[-1] = soundspeed_eq(gas)
else:
# for non-reactive or cold upstream state, use frozen shock calculation
    gas = PostShock_fr(w, Ps, Ts, qs, mech)  
    a2[-1] = soundspeed_fr(gas)

rho2[-1] = gas.density
ratio = rhos/rho2[-1];
w1[-1] = w
w2[-1] = w*ratio
P2[-1] = gas.P
beta[-1] = np.arcsin(w/US)
vt[-1] = US*np.cos(beta[-1])
theta[-1] = beta[-1] - np.arctan(w2[-1]/np.sqrt(US**2-w**2));
u2[-1] = np.sqrt(w2[-1]**2 + vt[-1]**2);


omega = np.asarray(omega); theta = np.asarray(theta); 
P = np.asarray(P); P2 = np.asarray(P2);
##
# plot pressure-deflection polar
plt.figure(1)
plt.plot(180*omega/np.pi,P/PA,linewidth=2) 
plt.plot(180*theta/np.pi,P2/PA,linewidth=2)
plt.title('PM polar and shock polar for detonation layer',fontsize=12)
plt.xlabel('flow deflection (deg)',fontsize=12)
plt.ylabel('pressure P/PA',fontsize=12)
plt.tick_params(labelsize=12)
plt.xlim(xmin=0)
plt.ylim(ymin=0)

# plot PM function-Mach number
plt.figure(2)
plt.plot(M,180*omega/np.pi,linewidth=2);  
plt.title('PM Function, free-stream speed '+str(round(U,5))+' m/s',fontsize=12);
plt.ylabel('PM function (deg)',fontsize=12);
plt.xlabel('Mach number',fontsize=12);
plt.tick_params(labelsize=12)
plt.xlim(xmin=1)
plt.ylim(ymin=0)

plt.show()
