"""
Shock and Detonation Toolbox Demo Program

Calculates the solution to ideal shock tube problem. Three cases possible: 
    conventional nonreactive driver (gas), 
    constant volume combustion driver (uv),
    CJ detonation (initiate at diaphragm) driver (cj)

Creates P-u relationship for shock wave and expansion wave, plots results
and interpolates to find shock speed and states 2 and 3 given states 1
and 4.  Creates tabulation of relevant states for solution.

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
from sdtoolbox.thermo import soundspeed_fr,soundspeed_eq
from sdtoolbox.postshock import PostShock_fr,PostShock_eq,CJspeed
import numpy as np
from scipy.interpolate import pchip # creates a PCHIP interpolation function that can then be given a query point
from scipy.optimize import fminbound
import matplotlib.pyplot as plt

## Select case for the driver operations
CASE_DRIVER = 'gas'  # nonreactive gas driver
#CASE_DRIVER = 'uv'   # constant pressure gas driver
#CASE_DRIVER = 'cj'   # detonation driver (initiation at diaphragm)

## Select case for the driven gas operations (use equilibrium only for very strong shocks)
CASE_DRIVEN = 'frozen' # non reactive driven gas
#CASE_DRIVEN = 'equilibrium' #reactive driven gas
print('demo_ShockTube, driver case: '+CASE_DRIVER+' driven case: '+CASE_DRIVEN)

## set initial state, composition, and gas object for driven section
P1 = 1200.; T1 = 300. 
q1 = 'N2:1.0 O2:3.76';   
driven_mech = 'airNASA9noions.cti'
driven_gas = ct.Solution(driven_mech)
driven_gas.TPX = T1,P1,q1

## Evaluate initial state 
rho1 = driven_gas.density
a1 = soundspeed_fr(driven_gas)

## Evaluate post-shock state (frozen) for a range of shock speeds 
print('Generating points on shock P-u curve')
Ustart = a1*1.01; Ustop = 8*a1; Ustep = 25
nsteps = int((Ustop-Ustart)/Ustep)

a2 = []; P2 = []; T2 = []; rho2 = []; u2 = []; Us = []

for U in np.linspace(Ustart,Ustop,num=nsteps):
    if CASE_DRIVEN=='frozen':
        shocked_gas = PostShock_fr(U, P1, T1, q1, driven_mech)
    elif CASE_DRIVEN=='equilibrium':
        shocked_gas = PostShock_eq(U, P1, T1, q1, driven_mech)
    a2.append(soundspeed_fr(shocked_gas))
    P2.append(shocked_gas.P)
    T2.append(shocked_gas.T)
    rho2.append(shocked_gas.density)
    w2 = rho1*U/shocked_gas.density
    u2.append(U - w2)
    Us.append(U)

if CASE_DRIVER=='gas':
    ## Set initial state for driver section - pressurized gas and no reaction
    # for nonreacting driver, use frozen expansion
    EQ_EXP = False
    P_driver = 3e6; T_driver = 300.;
    q4 = 'He:1.0'   
    driver_mech = 'Mevel2017.cti'
    driver_gas = ct.Solution(driver_mech)
    driver_gas.TPX = T_driver,P_driver,q4
    P4 = driver_gas.P
    T4 = driver_gas.T
    a3 = [soundspeed_fr(driver_gas)]
    u3 = [0]
    
elif CASE_DRIVER=='uv':
    ## Set initial state for driven section - combustion driver shock tube 
    # using constant volume state approximation  for reacting driver, use equilibrium expansion
    EQ_EXP = True
    P_driver_fill = P1; T_driver_fill = T1;
    q4 = 'C2H2:1.0 O2:2.5' 
    driver_mech = 'gri30_highT.cti'
    driver_gas = ct.Solution(driver_mech)
    driver_gas.TPX = T_driver_fill,P_driver_fill,q4
    driver_gas.equilibrate('UV')
    P4 = driver_gas.P
    T4 = driver_gas.T
    a3 = [soundspeed_eq(driver_gas)]
    u3 = [0]
    
elif CASE_DRIVER=='cj':
    ## Detonation driver with CJ wave moving away from diaphragm
    EQ_EXP = True
    P_driver_fill = P1; T_driver_fill = T1;
    q4 = 'C2H2:1.0 O2:2.5'
    driver_mech = 'gri30_highT.cti'
    driver_gas = ct.Solution(driver_mech)
    driver_gas.TPX = T_driver_fill,P_driver_fill,q4
    rho4 = driver_gas.density
    cj_speed = CJspeed(P_driver_fill, T_driver_fill, q4, driver_mech)
    driver_gas = PostShock_eq(cj_speed,P_driver_fill, T_driver_fill, q4, driver_mech)
    P4 = driver_gas.P
    T4 = driver_gas.T
    a3 = [soundspeed_eq(driver_gas)]
    w3 = cj_speed*rho4/driver_gas.density
    u3 = [w3-cj_speed]

## Evaluate initial state for expansion computation
rho3 = [driver_gas.density]
P3 = [driver_gas.P]
T3 = [driver_gas.T]
S4 = driver_gas.entropy_mass

## compute unsteady expansion (frozen)
print('Generating points on isentrope P-u curve')

vv = 1/rho3[0]
while u3[-1] < u2[-1]:
    vv = vv*1.01
    driver_gas.SVX = S4,vv,driver_gas.X
    
    if EQ_EXP:
        # required for equilibrium expansion
        driver_gas.equilibrate('SV')
        a3.append(soundspeed_eq(driver_gas))  
    else:
        # use this for frozen expansion
        a3.append(soundspeed_fr(driver_gas))

    P3.append(driver_gas.P)
    T3.append(driver_gas.T)
    rho3.append(driver_gas.density)
    u3.append(u3[-1] - 0.5*(P3[-1]-P3[-2])*(1/(rho3[-1]*a3[-1]) + 1./(rho3[-2]*a3[-2])))

## Input limits for finding intersection of polars
# Convert all lists to numpy arrays
P2 = np.asarray(P2); u2 = np.asarray(u2); P3 = np.asarray(P3); u3 = np.asarray(u3);
T2 = np.asarray(T2); a2 = np.asarray(a2); rho2 = np.asarray(rho2);
T3 = np.asarray(T3); a3 = np.asarray(a3); rho3 = np.asarray(rho3);
Us = np.asarray(Us)

# Sort all State 2 arrays by P2; State 3 arrays by P3.
iP2 = P2.argsort()
iP3 = P3.argsort()

P2_u2_fun = pchip(P2[iP2],u2[iP2])
u_max = P2_u2_fun(P4)

pmin = 1.01
pmax = max(P2)/P1

# define intersection function
P2P1_u2_fun = pchip(P2[iP2]/P1,u2[iP2])
P3P1_u3_fun = pchip(P3[iP3]/P1,u3[iP3])
fun = lambda x: (P2P1_u2_fun(x)-P3P1_u3_fun(x))**2

# solve for intersection of P-u curves to find pressure at contact surface
P_contact = fminbound(fun, pmin, pmax)

# find shock speed
P2P1_Us_fun = pchip(P2[iP2]/P1,Us[iP2])
U_shock = P2P1_Us_fun(P_contact)
M_shock = U_shock/a1

# find velocity and states on driven side of contact surface
u2_contact = P2P1_u2_fun(P_contact)
P2P1_T2_fun = pchip(P2[iP2]/P1,T2[iP2])
T2_contact = P2P1_T2_fun(P_contact)
P2P1_a2_fun = pchip(P2[iP2]/P1,a2[iP2])
a2_contact = P2P1_a2_fun(P_contact)
P2P1_rho2_fun = pchip(P2[iP2]/P1,rho2[iP2])
rho2_contact = P2P1_rho2_fun(P_contact)
M2 = u2_contact/a2_contact

# find velocity and states on driver side of contact surface
u3_contact = P3P1_u3_fun(P_contact)
P3P1_T3_fun = pchip(P3[iP3]/P1,T3[iP3])
T3_contact = P3P1_T3_fun(P_contact)
P3P1_a3_fun = pchip(P3[iP3]/P1,a3[iP3])
a3_contact = P3P1_a3_fun(P_contact)
P3P1_rho3_fun = pchip(P3[iP3]/P1,rho3[iP3])
rho3_contact = P3P1_rho3_fun(P_contact)
M3 = u3_contact/a3_contact

##
# Display results on screen
print('Driven State (1)')
print('   Fill gas '+q1)
print('   Pressure '+str(P1/1e3)+' (kPa)')
print('   Temperature '+str(T1)+' (K)')
print('   Sound speed '+str(a1)+' (m/s)')
print('   Density '+str(rho1)+' (kg/m3)')

if CASE_DRIVER=='cj':
    print('Driver State (CJ)')
    print('   Fill gas '+q4)
    print('   Fill Pressure '+str(P_driver_fill/1E3)+' (kPa)')
    print('   Fill Temperature '+str(T_driver_fill)+' (K)')
    print('   CJ speed '+str(cj_speed)+' (m/s)')
elif CASE_DRIVER=='uv':
    print('Driver State (UV)')
    print('   Fill gas '+q4)
    print('   Fill Pressure '+str(P_driver_fill/1E3)+' (kPa)')
    print('   Fill Temperature '+str(T_driver_fill)+' (K)')
elif CASE_DRIVER=='gas':
    print('Driver State (4)')
    print('   Fill gas '+q4)

print('   Pressure '+str(P4/1e3)+' (kPa)')
print('   Temperature '+str(T4)+' (K)')
print('   Sound speed '+str(a3[0])+' (m/s)')
print('   Density '+str(rho3[0])+' (kg/m3)')
print('Solution matching P-u for states 2 & 3')
print('Shock speed '+str(U_shock)+' (m/s)')
print('Shock Mach number '+str(M_shock)+' ')
print('Shock Pressure ratio '+str(P_contact)+' ')
print('Postshock state (2) in driven section')
print('   Pressure '+str(P1*P_contact/1e3)+' (kPa)')
print('   Velocity '+str(u2_contact)+' (m/s)')
print('   Temperature '+str(T2_contact)+' (K)')
print('   Sound speed '+str(a2_contact)+' (m/s)')
print('   Density '+str(rho2_contact)+' (kg/m3)')
print('   Mach number '+str(M2)+' ')
print('Expansion state (3) in driver section')
print('   Pressure '+str(P1*P_contact/1e3)+' (kPa)')
print('   Velocity '+str(u3_contact)+' (m/s)')
print('   Temperature '+str(T3_contact)+' (K)')
print('   Sound speed '+str(a3_contact)+' (m/s)')
print('   Density '+str(rho3_contact)+' (kg/m3)')
print('   Mach number '+str(M3)+' ')

Z = rho2_contact*a2_contact/(rho3_contact*a3_contact)

print('Impedance ratio at contact surface (rho a)_2/(rho a)_3 '+str(Z))

##
# plot pressure-velocity diagram
plt.figure(1)

plt.plot(u2,P2/P1,linewidth=2) 
plt.plot(u3,P3/P1,linewidth=2)
plt.title('Shocktube P-u relations',fontsize=12)
plt.xlabel('Velocity (m/s)',fontsize=12)
plt.ylabel('Pressure P/P1',fontsize=12)
plt.axis([u3[0], max(u2), 0, P4/P1])
plt.legend(['Driven shock','Driver expansion'],loc='upper center')
plt.show()
