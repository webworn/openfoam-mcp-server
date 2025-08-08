"""
Shock and Detonation Toolbox Demo Program

Calculates Prandtl-Meyer function and polar originating from CJ state. 
Calculates oblique shock wave moving into expanded detonation products or a 
specified bounding atmosphere.
Evaluates properties along expansion isentrope and computed performance
of rotating detonation engine based on ideal axial flow model.  Creates
plots of shock-expansion polar interaction and properties on the
expansion isentrope.

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
from sdtoolbox.thermo import soundspeed_fr, soundspeed_eq
from sdtoolbox.postshock import CJspeed, PostShock_eq, PostShock_fr
from scipy.interpolate import pchip # creates a PCHIP interpolation function that can then be given a query point
from scipy.optimize import fminbound
from sys import exit
import matplotlib.pyplot as plt
import numpy as np

# Suppress the 'divide by zero' warning encountered for some data points in the trig calls
# so that we can keep the code as similar as possible to the MATLAB equivalent
import warnings
warnings.filterwarnings("ignore", category=RuntimeWarning)

print('demo_PrandtlMeyerDetn')

# SET TYPE OF SHOCK COMPUTATION 
#EQ = True # Equilibrium
EQ = False # Frozen

##
# set the initial state and compute properties
P1 = 100000.; T1 = 300.;  #nominal initial state
#P1 = 150000.; T1 = 255.;  #Schwer and Kailasanath initial state
#P1 = 240000.; T1 = 300.;  #kasahara initial state
#P1 = 200000.; T1 = 300.;  #Fujiwara initial state
#q = 'C2H4:1.0, O2:3';     #ethylene oxygen, Kasahara
#q = 'H2:2.0, O2:1.0, AR:7.0'; #hydrogen oxygen argon
#q = 'H2:2.0, O2:1.0';  #hydrogen oxygen
#q = 'H2:2.0, O2:1.0, N2:3.76';  #hydrogen air
#q = 'C2H4:1.0, O2:3, N2:11.28';     #ethylene air
q  = 'CH4:1.0, O2:2.0, N2:7.52';  #methane-air
#q = 'C3H8:1.0, O2:5';  #propane oxygen
mech = 'gri30_highT.cti'
gas = ct.Solution(mech)
gas.TPX = T1,P1,q

h1 = gas.enthalpy_mass
s1 = gas.entropy_mass
rho1 = gas.density
a1_fr = soundspeed_fr(gas)
w1 = gas.mean_molecular_weight
R1 = ct.gas_constant/w1
gamma1_fr =  a1_fr**2*rho1/P1

print('Layer detonation computation for '+mech+' with composition '+q)
print('State 1 - Initial state of reacting layer')
print('   Pressure '+str(P1)+' (Pa)')
print('   Temperature '+str(T1)+' (K)')
print('   Density '+str(rho1)+' (kg/m3)')
print('   Sound speed (frozen) '+str(a1_fr)+' (m/s)')
print('   Enthalpy '+str(h1)+' (J/kg)')
print('   Entropy '+str(s1)+' (J/kg K)')
print('   gamma (frozen) '+str(gamma1_fr)+' ')

##
# Find CJ speed
U_CJ = CJspeed(P1, T1, q, mech)
# Evaluate CJ gas state
gas = PostShock_eq(U_CJ,P1, T1, q, mech)
x2 = gas.X
P2 = gas.P
T2 = gas.T
rho2 = gas.density
a2_eq = soundspeed_eq(gas)  
s2 = gas.entropy_mass
h2 = gas.enthalpy_mass
w2 = rho1*U_CJ/rho2
u2 = U_CJ-w2
gamma2_eq =  a2_eq**2*rho2/P2

print('State 2 - CJ ')
print('   CJ speed '+str(U_CJ)+' (m/s)')
print('   Pressure '+str(P2)+' (Pa)')
print('   Temperature '+str(T2)+' (K)')
print('   Density '+str(rho2)+' (kg/m3)')
print('   Enthalpy '+str(h2)+' (J/kg)')
print('   Entropy '+str(s2)+' (J/kg K)')
print('   w2 (wave frame) '+str(w2)+' (m/s)')
print('   u2 (lab frame) '+str(u2)+' (m/s)')
print('   a2 (equilibrium) '+str(a2_eq)+' (m/s)')
print('   gamma2 (equilibrium) '+str(gamma2_eq)+' ')

##
# Set triple point speed and downstream conditions acounting for the specified wave angle.   Need to
# be careful about the geometrical considerations and branches of the trigonometric functions. 
# This computation is only reliable for detonations that have ang = 90 and no upstream layer speed.  


ang = 90  # wave angle in degrees measured relative to freestream direction in counterclockwise (positive) direction
angle_det = np.deg2rad(ang)  # detonation wave angle (0< angle_det < Pi), angles greater than pi/2 deflect flow downward
# compute triple-point (upstream) speed, this assumes that speed of
# upstream flow is zero in lab frame
u_tp = U_CJ/np.sin(angle_det)
if (u_tp > U_CJ*(1.001)):
    # compute tangential (to detonation wave) velocity
    vt_det = np.sqrt(u_tp**2 - U_CJ**2)
    if (angle_det < np.pi/2):
         theta_det = angle_det - np.arctan(w2/vt_det)
    else:
        theta_det = -1*(angle_det - np.arctan(w2/vt_det))
    # compute flow deflection downstream of shock
    # downstream speed
    U2 = w2/np.cos(angle_det-theta_det);
else:
    vt_det = 0
    U2 = w2*(1.0001)  # start just above M=1 to avoid singularities
    theta_det = 0

##
print('Detonation Wave');
print('   Wave angle '+str(np.rad2deg(angle_det))+' (deg)')
print('   Downstream flow angle '+str(np.rad2deg(theta_det))+' (deg)')
print('   Triple point speed '+str(u_tp)+' (m/s)')
print('   Downsteam speed in wave frame (x) '+str(U2)+' (m/s)')
print('   Speed tangential to detonation (z) '+str(vt_det)+' (m/s)')

# stagnation enthalpy downstream of the detonation
h0 = h2 + U2**2/2
# estimate maximum  speed
wmax = np.sqrt(2*h0)
# initialize variables for computing PM properties and plotting
rho = []; a = []; P = []; T = []; h = []; mu = []; M = []; Me = []; u = []; ue = [];
v1 = 1/rho2  # lower limit to specific volume in PM fan
v2 = 50*v1   # upper limit to specific volume in PM fan
vstep = 1000

##
# Compute expansion conditions over range from minimum to maximum specific volume.  
# User can adjust the increment and endpoint to get a smooth output curve and reliable integral.
# Could also try a logarithmic range for some cases. 
for v in np.linspace(v1,v2,num=vstep):
    rho.append(1/v)
    x = gas.X   # update mole fractions based on last gas state
    gas.SVX = s2,v,x
    gas.equilibrate('SV') # required  for equilibrium expansion
    h.append(gas.enthalpy_mass)
    u.append(np.sqrt(2*(h0-h[-1]))) # flow speed within expansion
    # Need to explicitly upcast to complex, otherwise get NaN:
    ue.append(np.real(np.sqrt(2*(np.asarray(h1-h[-1],dtype=complex))))) # ideal axial velocity for RDE model
    a.append(soundspeed_eq(gas))  # use this for equilibrium expansion, almost always the case for detonation products
    #a.append(soundspeed_fr(gas))  # use this for frozen expansion   
    Me.append(ue[-1]/a[-1])
    M.append(u[-1]/a[-1])
    if M[-1] < 1:
        # Sometimes the first M value has been observed to be ~0.999 which leads to errors
        # in the arcsin and sqrt functions that follow
        M[-1] = 1.00001
    mu.append(np.arcsin(1/M[-1]))
    T.append(gas.T)
    P.append(gas.P)

# define integrand for PM function and integrate with trapezoidal rule
M = np.asarray(M); rho = np.asarray(rho)
integrand = -1*np.sqrt(M**2-1)/M**2/rho

omega = [theta_det]

for n in range(1,vstep):
     omega.append(omega[n-1] +(integrand[n-1] + integrand[n])*(rho[n]-rho[n-1])/2)

##
# compute shock polar
# First compute upstream state for RDE model, expand detonation products to ambient pressure
# Set freesteam velocity to expansion speed.  Accounts for shear across upstream contact 

P = np.asarray(P)
if EQ:
    gas.SPX = s2,P1,x
    gas.equilibrate('SP')
    a4 = soundspeed_eq(gas)
    U4fun = pchip(P/P1,u)
    U4 = U4fun(1)
else:
    # alternative state upstream: reactants  (need to use frozen shock calculation with this)
    #qs = 'C2H4:1.00 O2:3';     %ethylene oxygen
    qs = 'O2:1.00,N2:3.76'     # combustion air
    Ts  = 300.
    gas.TPX = Ts,P1,qs
    a4 = soundspeed_fr(gas)
    U4 = u_tp # no shear between bounding gas and reacting layer

##
# states upstream of shock
rho4  = gas.density
P4 = gas.P
T4 = gas.T
s4 = gas.entropy_mass
x4 = gas.X
h4 = gas.enthalpy_mass
gamma4 = a4**2*rho4/P4
beta_min = np.arcsin(a4/U4) # Mach angle

print('State 4 - upstream of oblique shock')
print('   Flow speed (wave frame) '+str(U4)+' (m/s)')
print('   Flow speed (lab frame) '+str(U4-u_tp)+' (m/s)')
print('   Pressure '+str(P4)+' (Pa)')
print('   Temperature '+str(T4)+' (K)')
print('   Density '+str(rho4)+' (kg/m3)')
print('   Sound speed (frozen) '+str(a4)+' (m/s)')
print('   Enthalpy '+str(h4)+' (J/kg)')
print('   Entropy '+str(s4)+' (J/kg K)')
print('   gamma  '+str(gamma4)+' ')
print('   Mach angle  '+str(np.rad2deg(beta_min))+' (deg)')

##
# oblique shock computation, state 4 is upstream and state 5 downstream
wmin = a4 + 1. #start just above sound speed
# check to see if shock speed is above sound speed
if (U4 < a4):
    exit('Shock speed below sound speed. Exiting.')

# set maximum normal speed
wmax = U4
# initialize variables for plotting
rho5 = []; w4 = []; w5 = []; u5 = []; vt4 = []; a5 = []; 
P5 = []; T5 = []; h5 = []; s5 = []; beta = []; theta = [];

##
# compute shock jump conditions over range from minimum to maximum normal
# speeds.  Adjust the increment to get a smooth output curve.
wstep = 5
nsteps = int((wmax-wmin)/wstep) 
for w in np.linspace(wmin,wmax,num=nsteps):
    if EQ:
        # equilibrium state
        gas = PostShock_eq(w, P4, T4, x4, mech)
        a5.append(soundspeed_eq(gas))
    else:
        #  for non-reactive or cold upstream state, use frozen shock calculation
        gas = PostShock_fr(w, P4, T4, qs, mech)
        a5.append(soundspeed_fr(gas))
    
    rho5.append(gas.density)
    ratio = rho4/rho5[-1]
    w4.append(w)
    w5.append(w*ratio)
    P5.append(gas.P)
    T5.append(gas.T)
    h5.append(gas.enthalpy_mass)
    s5.append(gas.entropy_mass)
    beta.append(np.arcsin(w/U4))
    vt4.append(U4*np.cos(beta[-1]))
    theta.append(beta[-1] - np.arctan(w5[-1]/np.sqrt(U4**2-w**2)))
    u5.append(np.sqrt(w5[-1]**2 + vt4[-1]**2))

##
omega = np.asarray(omega); P5 = np.asarray(P5)
# plot pressure-deflection polar
plt.figure(1)
plt.plot(np.rad2deg(omega),P/P1,linewidth=2)
plt.plot(np.rad2deg(theta),P5/P1,linewidth=2) 
plt.title('PM polar and shock polar for detonation layer',fontsize=12)
plt.xlabel('flow deflection (deg)',fontsize=12)
plt.ylabel('pressure P/P1',fontsize=12)
plt.tick_params(labelsize=12)
plt.xlim(xmin=0)
plt.legend(['PM polar','Shock polar'],loc='upper right')

# plot PM function-Mach number
plt.figure(2)
plt.plot(M,np.rad2deg(omega),linewidth=2)
plt.title('PM Function expanding from CJ state',fontsize=12)
plt.ylabel('PM function (deg)',fontsize=12)
plt.xlabel('Mach number',fontsize=12)
plt.tick_params(labelsize=12)
plt.xlim(xmin=1)
plt.ylim(ymin=0)

# plot PM function velocity-pressure
plt.figure(3)
plt.plot(P/P1,u,linewidth=2)
plt.plot(P/P1,ue,linewidth=2)
plt.title('PM Function vs Axial expansion from CJ state',fontsize=12)
plt.ylabel('speed (m/s)',fontsize=12)
plt.xlabel('Pressure',fontsize=12)
plt.tick_params(labelsize=12)  
plt.xlim(xmin=0)
plt.ylim(ymin=0)
plt.legend(['PM polar','Axial expansion'],loc='upper right')
    
##
#  Input limits for finding intersection of polars - use plots to obtain
#  guesses
pmin = 3
pmax = 8
# define intersection function
P5_theta_fun = pchip(P5/P1,theta)
iP = P.argsort()
Porig = P; P = P[iP]; omegasort = omega[iP];
P_omega_fun = pchip(P/P1,omegasort)
fun = lambda x: (P5_theta_fun(x)-P_omega_fun(x))**2
# solve for intersection 
Pshock = fminbound(fun, pmin, pmax)

# compute postshock conditions - state 5
thetashock = np.rad2deg(P5_theta_fun(Pshock))

P5_beta_fun = pchip(P5/P1,beta);
betashock =  np.rad2deg(P5_beta_fun(Pshock))

P5_u_fun = pchip(P5/P1,u5)
ushock =  P5_u_fun(Pshock)

P5_rho_fun = pchip(P5/P1,rho5)
rhoshock =  P5_rho_fun(Pshock)

P5_h_fun = pchip(P5/P1,h5)
hshock =  P5_h_fun(Pshock)

P5_s_fun = pchip(P5/P1,s5)
sshock =  P5_s_fun(Pshock)

P5_a_fun = pchip(P5/P1,a5)
ashock =  P5_a_fun(Pshock)
M5 = ushock/ashock

P5_T_fun = pchip(P5/P1,T5)
Tshock =  P5_T_fun(Pshock)
gamma5 = ashock**2*rhoshock/(Pshock*P1)

# find shock normal speed and Mach number
P5_w_fun = pchip(P5/P1,w4)
wshock =  P5_w_fun(Pshock)
Mshock = wshock/a4

print('State 5 - downstream of oblique shock')
print('   Flow speed (wave frame) '+str(ushock)+' (m/s)')
print('   Pressure '+str(Pshock*P1)+' (Pa)')
print('   Temperature '+str(Tshock)+' (K)')
print('   Density '+str(rhoshock)+' (kg/m3)')
print('   Enthalpy '+str(hshock)+' (J/kg)')
print('   Entropy '+str(sshock)+' (J/kg K)')
print('   Sound speed '+str(ashock)+' (m/s)')
print('   Gamma '+str(gamma5)+' ')
print('   Mach number '+str(M5)+' ')
print('   Wave angle  '+str(betashock)+' (deg)')
print('   Flow angle  '+str(thetashock)+' (deg)')
print('   Upstream of wave - equivalent normal shock speed '+str(wshock)+' (m/s)')
print('   Upstream of wave - equivalent normal shock Mach number '+str(Mshock)+' ')

##
# compute postexpansion conditions - state 3
u = np.asarray(u); a = np.asarray(a); h = np.asarray(h); mu = np.asarray(mu);  T = np.asarray(T);
uorig = u; u = u[iP]
P_u_fun = pchip(P/P1,u)
u3 = P_u_fun(Pshock)

aorig = a; a = a[iP]
P_a_fun = pchip(P/P1,a)
a3 = P_a_fun(Pshock)

horig = h; h = h[iP]
P_h_fun = pchip(P/P1,h)
h3 = P_h_fun(Pshock)

rhoorig = rho; rho = rho[iP]
P_rho_fun = pchip(P/P1,rho)
rho3 = P_rho_fun(Pshock)

omega3 = np.rad2deg(P_omega_fun(Pshock))

M3 = u3/a3

muorig = mu; mu = mu[iP]
P_mu_fun = pchip(P/P1,mu)
mu3 = np.rad2deg(P_mu_fun(Pshock))

Torig = T; T = T[iP]
P_T_fun = pchip(P/P1,T)
T3 = P_T_fun(Pshock)

gamma3 = a3**2*rho3/(P1*Pshock)

print('State 3 - downstream of expansion');
print('   Flow speed (wave frame) '+str(u3)+' (m/s)')
print('   Pressure '+str(Pshock*P1)+' (Pa)')
print('   Temperature '+str(T3)+' (K)')
print('   Density '+str(rho3)+' (kg/m3)')
print('   Enthalpy '+str(h3)+' (J/kg)')
print('   Sound speed '+str(a3)+' (m/s)')
print('   Gamma '+str(gamma3)+' ')
print('   Mach number '+str(M3)+' ')
print('   Characteristic angle  '+str(mu3)+' (deg)')
print('   PM deflection angle  '+str(omega3)+' (deg)')

##
# ideal perfomance velocity considering pure axial flow and isentropic expansion to ambient pressure
ue = np.asarray(ue);
ueorig = ue; ue = ue[iP]
P_ue_fun = pchip(P/P1,ue)
ueideal = P_ue_fun(1)

rhoideal = P_rho_fun(1)
aideal = P_a_fun(1)
hideal = P_h_fun(1)

print('Axial Model- Expansion to P1');
print('   Axial speed '+str(ueideal)+' (m/s)')
print('   Density '+str(rhoideal)+' (kg/m3)')
print('   Enthalpy '+str(hideal)+' (J/kg)')
print('   Sound speed '+str(aideal)+' (m/s)')
print('   Mach number '+str(ueideal/aideal)+' ')

# Restore all array orders to original before sorting by a different x-coordinate
P = Porig; u = uorig; a = aorig; h = horig; rho = rhoorig; mu = muorig; T = Torig; ue = ueorig

## expansion to Mach 1, remove nonzero elements of Me array

# Sort everything by the x-coordinates (i.e., Me)
Me = np.asarray(Me)
iMe = Me.argsort(); Meorig = Me; Me = Me[iMe]
first = np.argwhere(Me)[0][0]
ue = ue[iMe]; h = h[iMe]; P = P[iMe]; T = T[iMe]; rho = rho[iMe]; a = a[iMe];

uefun = pchip(Me[first:],ue[first:])
hfun = pchip(Me[first:],h[first:])
Pfun = pchip(Me[first:],P[first:])
Tfun = pchip(Me[first:],T[first:])
rhofun = pchip(Me[first:],rho[first:])
afun = pchip(Me[first:],a[first:])

uesonic = uefun(1)
hsonic = hfun(1)
Psonic = Pfun(1)
Tsonic = Tfun(1)
rhosonic = rhofun(1)
asonic = afun(1)

ToverMdot = (Psonic-ct.one_atm)/(rhosonic*uesonic) + uesonic

# Restore all array orders to original before sorting by a different x-coordinate
P = Porig; a = aorig; h = horig; rho = rhoorig; T = Torig; ue = ueorig

ih = h.argsort(); hsort = h[ih]
Psort = P[ih]
hPfun = pchip(hsort,Psort)
Pm = hPfun(h1)

P = Porig;  h = horig;

print('Axial Model- Expansion to Mach 1')
print('   Limiting pressure '+str(Pm)+' (Pa)')
print('   Axial speed '+str(uesonic)+' (m/s)')
print('   Pressure '+str(Psonic)+' (Pa)')
print('   Temperature '+str(Tsonic)+' (K)')
print('   Density '+str(rhosonic)+' (kg/m3)')
print('   Enthalpy '+str(hsonic)+' (J/kg)')
print('   Sound speed '+str(asonic)+' (m/s)')
print('   Specific thrust '+str(ToverMdot)+' (m/s)')

# compute specific thrust for all exit pressures and one atm ambient pressure
ToM = ue[first:] + (P[first:]-ct.one_atm)/(rho[first:]*ue[first:])

# plot specific thrust vs pressure
plt.figure(5)
plt.ylim((0,3000))
plt.plot(P[first:]/P1,ToM,'k',linewidth=2)
plt.plot(Psonic/P1,ToverMdot,marker='*',markerfacecolor='k',markeredgecolor='k',markersize=10)
plt.text(Psonic/P1,0.85*ToverMdot,'Sonic Axial Flow State',horizontalalignment='center')
plt.ylabel('specific thrust (m/s)',fontsize=12)
plt.xlabel('Pressure (P/P1)',fontsize=12)
plt.tick_params(labelsize=12)

# expansion to a vacuum - using last point in expansion computation
uevac = ue[-1]
Pvac = P[-1]
Tvac = T[-1]
rhovac = rho[-1]
hvac = h[-1]
avac = a[-1]

print('Axial Model- Expansion to Vacuum')
print('   Axial speed '+str(uevac)+' (m/s)')
print('   Pressure '+str(Pvac)+' (Pa)')
print('   Temperature '+str(Tvac)+' (K)')
print('   Density '+str(rhovac)+' (kg/m3)')
print('   Enthalpy '+str(hvac)+' (J/kg)')
print('   Sound speed '+str(avac)+' (m/s)')
print('   Mach number '+str(uevac/avac)+' ')

# plot PM function enthalpy-pressure
plt.figure(4)
plt.plot(P/P1,h/1e6,linewidth=2,color='k');
plt.plot(1,h1/1E6,marker='s',markerfacecolor='k',markeredgecolor='none',markersize=10)
plt.text(2.1,h1/1E6+.1E0,'Initial State (1)',horizontalalignment='center');
plt.plot(P2/P1,h2/1E6,marker='d',markerfacecolor='k',markeredgecolor='none',markersize=10)
plt.text(P2/P1,h2/1E6+.1E0,'CJ State (2)',horizontalalignment='center');
plt.plot(Pshock,h3/1E6,marker='o',markerfacecolor='k',markeredgecolor='none',markersize=10)
plt.text(Pshock+.5,h3/1E6,'Expansion downstream State (3)');
plt.plot(P4/P1,h4/1E6,marker='^',markerfacecolor='k',markeredgecolor='none',markersize=10)
plt.text(P4/P1+0.5,h4/1E6,'Shock upstream State (4)')
plt.plot(Pshock,hshock/1E6,marker='^',markerfacecolor='none',markeredgecolor='k',markersize=10)
plt.text(Pshock,hshock/1E6+0.1,'Shock downstream State (5)',horizontalalignment='center')
plt.plot(Psonic/P1,hsonic/1E6,marker='*',markerfacecolor='k',markeredgecolor='k',markersize=10)
plt.text(Psonic/P1+0.5,hsonic/1E6,'Sonic Axial Flow State')
plt.title('PM Function expanding from CJ state',fontsize=12)
plt.ylabel('enthalpy (MJ/kg)',fontsize=12)
plt.xlabel('Pressure (P/P1)',fontsize=12)
plt.tick_params(labelsize=12)
plt.xlim(xmin=0)

plt.show()
