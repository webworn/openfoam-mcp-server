"""
Shock and Detonation Toolbox Demo Program

Calculates shock polar using FROZEN post-shock state based the initial gas properties 
and the shock speed. Plots shock polar using three different sets of coordinates.
Saves archive of numpy arrays for beta, theta, and M_2.

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
    Windows 8.1, Windows 10, Linux (Debian 9)s
"""
import cantera as ct
from sdtoolbox.thermo import soundspeed_fr
from sdtoolbox.postshock import PostShock_fr #, PostShock_eq
import numpy as np
from sys import exit
from numpy import interp # directly takes a query point
from scipy.interpolate import pchip # creates a PCHIP interpolation function that can then be given a query point
from scipy.optimize import fminbound
import matplotlib.pyplot as plt

# Suppress the 'divide by zero' warning encountered for the last data point in the arctan calls
# (where U = w) so that we can keep the code as similar as possible to the MATLAB equivalent
# Numpy actually handles this case and returns the correct value for the limit arctan(inf)
import warnings
warnings.filterwarnings("ignore", category=RuntimeWarning)

##
# set the initial state and compute properties
P1 = 100000; T1 = 300 
q = 'H2:2 O2:1 N2:3.76'   
mech = 'Mevel2017.cti'
gas1 = ct.Solution(mech)
gas1.TPX = T1,P1,q
rho1 = gas1.density
a1 = soundspeed_fr(gas1)

##
# Set freestream velocity and find limiting speeds
U = 1500.
beta_min = np.arcsin(a1/U)  # Mach angle
wmin = a1 + 1.  # start just above sound speed
# check to see if shock speed is above sound speed
if (U < a1):
    exit('Shock speed below sound speed. Exiting.')


# set maximum normal speed
wmax = U

# initialize variables for plotting
rho2 = []; w1 = []; w2 = []; u2 = []; v = []; a2 = []; P2 = []; beta = []; theta = [];

##
# compute shock jump conditions over range from minimum to maximum normal
# speeds.  Adjust the increment to get a smooth output curve.
wstep = 5
nsteps = int((wmax-wmin)/wstep)
for w in np.linspace(wmin,wmax,num=nsteps):
    gas = PostShock_fr(w, P1, T1, q, mech)
    # gas = PostShock_eq(w, P1, T1, q, mech) #will not converge at low temperature.
    rho2.append(gas.density)
    a2.append(soundspeed_fr(gas))
    ratio = rho1/rho2[-1]
    w1.append(w)
    w2.append(w*ratio)
    P2.append(gas.P)
    beta.append(np.arcsin(w/U))
    v.append(U*np.cos(beta[-1]))
    theta.append(beta[-1] - np.arctan(w2[-1]/np.sqrt(U**2-w**2)))
    u2.append(np.sqrt(w2[-1]**2 + v[-1]**2))

## pick up normal shock as last point
w = U
gas = PostShock_fr(w, P1, T1, q, mech)
rho2[-1] = gas.density
a2[-1] = soundspeed_fr(gas)
ratio = rho1/rho2[-1]
w1[-1] = w
w2[-1] = w*ratio
P2[-1] = gas.P
beta[-1] = np.arcsin(w/U)
v[-1] = U*np.cos(beta[-1])
theta[-1] = beta[-1] - np.arctan(w2[-1]/np.sqrt(U**2-w**2))
u2[-1] = np.sqrt(w2[-1]**2 + v[-1]**2)

## post process output 
# compute Mach number 
u2 = np.asarray(u2); a2 = np.asarray(a2); P2 = np.asarray(P2)
beta = np.asarray(beta); theta = np.asarray(theta)
M2 = u2/a2

np.savez('BTM',beta=beta,theta=theta,M2=M2)

# find sonic point
# Note: numpy.interp takes a query point directly, like MATLAB's interp1. 
# However, it requires the x-coordinates of the data points to be monotonically increasing.
# Make sure x-data gets sorted to be monotonically increasing before attempting any interpolation
inds = M2.argsort()
M2 = M2[inds]; beta = beta[inds]; theta = theta[inds];
beta_star = interp(1.0,M2,beta)

inds = beta.argsort()
M2 = M2[inds]; beta = beta[inds]; theta = theta[inds];
theta_star = interp(beta_star,beta,theta)

# find maximum deflection estimate
theta_max_guess = max(theta)
inds = theta.argsort()
M2 = M2[inds]; beta = beta[inds]; theta = theta[inds];
beta_max_guess = interp(theta_max_guess,theta,beta)

inds = beta.argsort()
M2 = M2[inds]; beta = beta[inds]; theta = theta[inds];
fun = pchip(beta,theta)
beta_max = fminbound(lambda x: -fun(x), .9*beta_max_guess, 1.1*beta_max_guess)
theta_max = fun(beta_max)

## compute velocity components downstream in lab frame
u2x = u2*np.cos(theta)
u2y = u2*np.sin(theta)

## create plots

# plot pressure-deflection polar
plt.figure(1)
plt.plot(180*theta/np.pi,P2/1e6,'k:',linewidth=2)
plt.title('Shock Polar Air, free-stream speed '+str(round(U,5))+' m/s',fontsize=12)
plt.xlabel('deflection angle (deg)',fontsize= 12)
plt.ylabel('pressure (MPa)',fontsize= 12)
plt.tick_params(labelsize=12)
plt.xlim(xmin=0)
plt.ylim(ymin=0)


# plot pressure-deflection polar
plt.figure(2)
plt.plot(180*theta/np.pi,180*beta/np.pi,'k:',linewidth=2)  
plt.title('Shock Polar Air, free-stream speed '+str(round(U,5))+' m/s',fontsize=12)
plt.xlabel('deflection angle (deg)',fontsize=12)
plt.ylabel('wave angle (deg)',fontsize=12)
plt.tick_params(labelsize=12)
plt.xlim(xmin=0)
plt.ylim(0,90)

# plot velocity polar
plt.figure(3)
plt.plot(u2x,u2y,'k:',linewidth=2)  
plt.title('Shock Polar Air, free-stream speed '+str(round(U,5))+' m/s',fontsize=12)
plt.xlabel('u2x (m/s)',fontsize=12)
plt.ylabel('u2y (m/s)',fontsize=12)
plt.tick_params(labelsize=12)
plt.ylim(ymin=0)

plt.show()
