"""
Shock and Detonation Toolbox Demo Program

Equilibrium computation at over a range of temperatures and pressures
 
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
import matplotlib.pyplot as plt
import numpy as np
from cycler import cycler

## initial gas state
P1 = ct.one_atm; T1 = 300
mech = 'airNASA9ions.cti'
gas  = ct.Solution(mech)
q = 'N2:0.7809, O2:0.2095, Ar:0.0093, CO2:0.0004' # Earth's atmosphere
nsp = gas.n_species

X = []
T = np.arange(500,stop=20000,step=100)
for T1 in T:
    gas.TPX = T1,P1,q
    gas.equilibrate('TP')
    X.append(gas.X)

X = np.array(X)
##
plt.figure(num='Equilibrium Composition')
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
plt.rc('axes', prop_cycle=(cycler('linestyle', ['-', '--', ':', '-.'])*
                           cycler('color',defaultColors)))

plt.semilogy(T,X,linewidth=2)
plt.axis([min(T), max(T), 1.0e-5, 1])
plt.xlabel('Temperature (K)')
plt.ylabel('species mole fraction')
plt.legend(gas.species_names,loc='center left', bbox_to_anchor=(1, 0.5),ncol=2)
plt.subplots_adjust(right=0.6)
plt.show()
