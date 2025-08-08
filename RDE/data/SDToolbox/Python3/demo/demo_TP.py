"""
Shock and Detonation Toolbox Demo Program

Explosion computation simulating constant temperature and pressure reaction.

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
from cycler import cycler

def tpsys(t,y,gas,mw,T,P):
    """
     Evaluates the system of ordinary differential equations for an adiabatic, 
         constant-temperature and pressure, zero-dimensional reactor. 
         It assumes that the 'gas' object represents a reacting ideal gas mixture.
     
         INPUT:
             t   = time
             y   = solution array [species mass fraction 1, 2, ...]
             gas = working gas object
             mw  = array of species molar masses
             T   = temperature
             P   = pressure
         
         OUTPUT:
             An array containing time derivatives of:
                  species mass fractions, 
            formatted in a way that the integrator in this demo can recognize.
    """

    # Set the state of the gas, based on the current solution vector.
    gas.TPY = T,P,y
    nsp = gas.n_species
    # species molar production rates
    wdot = gas.net_production_rates
    # set up column vector for dydt
    dYdt = []
    # species time evolution equations
    rrho = 1.0/gas.density
    for i in range(nsp):
      dYdt.append(rrho*mw[i]*wdot[i])
    
    return dYdt

## initial gas state
P1 = ct.one_atm; T1 = 3000
mech = 'Mevel2015.cti'
gas = ct.Solution(mech)
q = 'H2:10 O2:1'   
gas.TPX = T1,P1,q
## set up for integration routine
mw = gas.molecular_weights
nsp = gas.n_species
y0 = gas.Y
t_end = 1e-5;
tel = [0, t_end]

time = [tel[0]]
species = [gas.Y]

# Set up ODE system for integration
extraParams = (gas,mw,T1,P1)
r = ode(tpsys)
dt = 1e-8
r.set_integrator('lsoda', method='bdf', rtol=1e-5, atol=1e-12)
r.set_initial_value(y0,tel[0]).set_f_params(*extraParams)

# Call the integrator to march in time - the equation set is defined in tpsys() 
while r.successful() and r.t < tel[1]:
    r.integrate(r.t + dt)    
    #############################################################################################
    # Extract TIME, DISTANCE, VELOCITY, TEMPERAURE and SPECIES arrays from integrator output
    #############################################################################################        
    time.append(r.t)
    species.append(r.y)
del r

species = np.array(species)

## Plotting species
plt.figure(num='TP Reactor')
masterFontSize = 12
defaultColors = ['#1f77b4',
                 '#ff7f0e',
                 '#2ca02c',
                 '#d62728',
                 '#9467bd',
                 '#8c564b',
                 '#e377c2',
                 '#7f7f7f',
                 '#bcbd22',
                 '#17becf']
plt.rc('font',size=masterFontSize)
# Change linestyle once all colors cycled through
plt.rc('axes', prop_cycle=(cycler('marker',['None','o','v','^'])*
                           cycler('linestyle', ['-', '--', ':', '-.'])*
                           cycler('color',defaultColors)))

plt.plot(time,species,linewidth=2,markevery=50);
plt.ticklabel_format(style='sci',axis='x',scilimits=(0,0))
plt.xlabel('Time (s)')
plt.ylabel('species mass fraction')
plt.title('TP Reactor')
plt.xlim([min(time),max(time)])
plt.ylim((0,None))
#plt.tight_layout()
plt.legend(gas.species_names,loc='center left', bbox_to_anchor=(1, 0.5),ncol=4)
#plt.tight_layout()
plt.subplots_adjust(right=0.5)
plt.show()


