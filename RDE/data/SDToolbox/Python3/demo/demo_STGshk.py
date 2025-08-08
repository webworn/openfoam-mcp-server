"""
Shock and Detonation Toolbox Demo Program

Generate plots and output files for a steady reaction zone between a shock and a blunt body using
Hornung model of linear profile of rho u on stagnation streamline.
 
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
from sdtoolbox.postshock import PostShock_fr
from sdtoolbox.stagnation import stgsolve
from sdtoolbox.utilities import znd_plot
import matplotlib.pyplot as plt
from numpy import logspace,log10
import pickle

graphing = True

P1 = 13.33; T1 = 300; U1 = 4000; Delta = 5
q = 'CO2:0.96 N2:0.04'
mech = 'airNASA9noions.cti'

gas1 = ct.Solution(mech)
gas1.TPX = T1,P1,q
nsp = gas1.n_species

# FIND POST SHOCK STATE FOR GIVEN SPEED
gas = PostShock_fr(U1, P1, T1, q, mech)

t_end = 0.1

# MATLAB's ode solvers automatically interpolate additional values beyond what is
# needed to meet tolerances. Use the optional t_eval keyword from the integrator
# to do something similar (otherwise plotted solution very sparse at short times)
t_dense = logspace(-10,log10(t_end),num=500)

# SOLVE REACTION ZONE STRUCTURE ODES
out = stgsolve(gas,gas1,U1,Delta,t_end=t_end,max_step=1e-4,t_eval=t_dense)

if graphing:
    # Demonstrate using the figure outputs from znd_plot to make further formatting
    # adjustments, then saving
    # Note that znd_plot still works with stgsolve since the output structures
    # contain many of the same fields
    
    maxx = max(out['distance'])
    figT,figP,figM,figTherm,figSpec = znd_plot(out,maxx=maxx,major_species='All',
                                               xscale='log',show=False)
    
    figSpec.axes[0].set_xlim((1e-5,None))
    figSpec.axes[0].set_ylim((1e-10,1))
    
    plt.show()
    
    figT.savefig('stg-T.eps',bbox_inches='tight')
    figP.savefig('stg-P.eps',bbox_inches='tight')
    figM.savefig('stg-M.eps',bbox_inches='tight')
    figTherm.savefig('stg-therm.eps',bbox_inches='tight')
    figSpec.savefig('stg-Y.eps',bbox_inches='tight')

# Can't pickle Cantera Solution objects due to underlying C++ structure, 
# so remove it from dictionary first
out.pop('gas1') 
pickle.dump(out,open('stg.p','wb'))



#% save output file for later comparison
#stg = out;
#save 'stg.mat' stg ;

#if graphing:    
#    masterFontSize = 12
#    defaultColors = ['#1f77b4',
#                     '#ff7f0e',
#                     '#2ca02c',
#                     '#d62728',
#                     '#9467bd',
#                     '#8c564b',
#                     '#e377c2',
#                     '#7f7f7f',
#                     '#bcbd22',
#                     '#17becf']
#    plt.rc('font',size=masterFontSize)
#    # Change linestyle once all colors cycled through
#    plt.rc('axes', prop_cycle=(cycler('linestyle', ['-', '--', '-.',':'])*
#                               cycler('color',defaultColors[:4])))
#
#
#    figTitle = 'Stagnation Zone Structure U = '+str(U1)+' m/s'+' Delta = '+str(Delta)+' m'
#    plt.figure()
#    plt.title(figTitle,y=1.08)
#    maxy = max(out['P'])
#    miny = min(out['P'])
#    minx = 1e-6
#    maxx = max(out['distance'])
#    plt.semilogx(out['distance'],out['P'])
#    plt.axis([minx,maxx,miny,maxy])
#    plt.xlabel('distance (m)')
#    plt.ylabel('pressure (Pa)')
#    plt.savefig('stg-P.eps',bbox_inches='tight')
#   
#    plt.figure()
#    plt.title(figTitle,y=1.08)
#    maxy = max(out['T'])
#    miny = min(out['T'])
#    plt.semilogx(out['distance'],out['T'])
#    plt.axis([minx,maxx,miny,maxy])
#    plt.xlabel('distance (m)')
#    plt.ylabel('temperature (K)')
#    plt.savefig('stg-T.eps',bbox_inches='tight')
#    
#    plt.figure()
#    plt.title(figTitle,y=1.08)
#    maxy = max(out['rho'])
#    miny = min(out['rho'])
#    plt.semilogx(out['distance'],out['rho'])
#    plt.axis([minx,maxx,miny,maxy])
#    plt.xlabel('distance (m)')
#    plt.ylabel('density (kg/m$^3$)')
#    plt.ticklabel_format(style='sci',axis='y',scilimits=(0,0))
#    plt.savefig('stg-rho.eps',bbox_inches='tight')
#
#    plt.figure()
#    plt.title(figTitle,y=1.08)
#    plt.loglog(out['distance'],out['species'].T)
#    plt.axis([minx,maxx,1e-5,1])
#    plt.xlabel('distance (m)')
#    plt.ylabel('species mass fraction')
#    plt.legend(gas.species_names,loc='center left',bbox_to_anchor=(1, 0.6),ncol=1)
#    plt.subplots_adjust(right=0.6)
#    plt.savefig('stg-Y.eps',bbox_inches='tight')
#    
#    plt.show()
#
#
