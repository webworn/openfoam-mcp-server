"""
Shock and Detonation Toolbox Demo Program

Explosion computation simulating adiabatic compression ignition with
control volume approach and effective piston used for compression.

This demo still uses an old, slow method of solving ODE systems in Python, with 
a constant time-step. It is sufficiently fast for the purposes of the demo, but
for development of new ODE systems, please refer to the method used in, e.g.
zndsolve, cvsolve, stgsolve, where scipy.integrate.solve_ivp is called instead,
and the ODE system is defined as a callable class method rather than a function.

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
import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import ode

def adiasys(t,y,gas,mw,rho0,X0,m):
    """
    Evaluates the system of ordinary differential equations for an adiabatic, 
    rapid compression, zero-dimensional reactor. Used by the ODE solver in the
    main body of the demo program.
    It assumes that the 'gas' object represents a reacting ideal gas mixture. 
    
    INPUT:
        t = time
        y = solution array [position, velocity, temperature, species mass 1, 2, ...]
        gas = working gas object
        mw = array of molecular weights of each species in gas object
        rho0 = initial density
        X0 = initial position/height
        m = specific mass of effective piston
    
    OUTPUT:
        An array containing time derivatives of:
            position, velocity, temperature and species mass fractions, 
        formatted in a way that the integrator in this demo can recognize.
    """
    # Set the state of the gas, based on the current solution vector.
    rho = X0*rho0/(y[0]+.001)
    gas.TDY = y[2],rho,y[3:]
    nsp = gas.n_species
    P = gas.P
    # energy equation
    wdot = gas.net_production_rates
    Tdot = -1*(y[2] * ct.gas_constant * np.dot(gas.standard_enthalpies_RT - np.ones(nsp),wdot) 
            / rho + P*y[1]/(rho*y[0]))/gas.cv_mass
    Udot =  (P-ct.one_atm)/m
    # set up column vector for dydt
    dYdt = []    
    # species equations
    for i in range(nsp):
        dYdt.append(mw[i]*wdot[i]/rho)
    
    return np.hstack((y[1], Udot, Tdot, dYdt))



###################################################
print('Transient Compression Ignition')

# compression apparatus dimensions
X0 = 0.377          # height of gas volume (m)
R0 = 0.032/2        # diameter of gas volume (m)
U0 = -5.8           # initial speed of compression piston (m/s)
A  = np.pi*R0**2    # cross section area of compression volume (m2)
L = 0.40            # estimated length of piston
Mp = A*L*8000       # estimated mass of piston
V = X0*A            # volume of gas  (m3)
M  = 7.995          # effective mass of piston  (kg)
m = M/A             # specific mass of effective piston

# initial gas state
P1 = ct.one_atm; T1 = 300.
mech = 'Mevel2017.cti'
gas  = ct.Solution(mech)
q = 'H2:10 O2:1'    
gas.TPX = T1,P1,q
mw = gas.molecular_weights
rho0 = gas.density

##
nsp = gas.n_species

y0 = np.hstack((X0,U0,gas.T,gas.Y))

# Set the integration parameters for the ODE solver
tel = [0.,0.1] # Timespan

# Initialize output matrices.
time = [tel[0]]
distance = [X0]
velocity = [U0]
temperature = [gas.T]
species = [gas.Y]

# Set up ODE system for integration
extraParams = (gas,mw,rho0,X0,m)
r = ode(adiasys)
dt = 1e-5
r.set_integrator('lsoda', method='bdf', rtol=1e-5, atol=1e-8)
r.set_initial_value(y0,tel[0]).set_f_params(*extraParams)

# Call the integrator to march in time - the equation set is defined in adiasys() 
while r.successful() and r.t < tel[1]:
    r.integrate(r.t + dt)    
    #############################################################################################
    # Extract TIME, DISTANCE, VELOCITY, TEMPERAURE and SPECIES arrays from integrator output
    #############################################################################################        
    time.append(r.t)
    distance.append(r.y[0])
    velocity.append(r.y[1])
    temperature.append(r.y[2])
    species.append(r.y[3:])
del r

# compute pressure
P = []
for n in range(len(time)):
    rho = X0*rho0/(distance[n]+.001)
    gas.TDY = temperature[n],rho,species[n]
    P.append(gas.P)

plt.figure(1)
fontsize=12
plt.plot(time,temperature)
plt.xlabel('Time (s)',fontsize=fontsize)
plt.ylabel('Temperature (K)',fontsize=fontsize)
plt.title('Gas Temperature',fontsize=fontsize)
plt.xlim((min(time),max(time)))

plt.figure(2)
plt.plot(time,P)
plt.xlabel('Time (s)',fontsize=fontsize)
plt.ylabel('Pressure (Pa)',fontsize=fontsize)
plt.title('Gas Pressure',fontsize=fontsize)
plt.xlim((min(time),max(time)))

plt.figure(3)
plt.plot(time,velocity)
plt.xlabel('Time (s)',fontsize=fontsize)
plt.ylabel('Piston speed (m)',fontsize=fontsize)
plt.title('Piston Speed',fontsize=fontsize)
plt.xlim((min(time),max(time)))

plt.show()



