"""
Shock and Detonation Toolbox Demo Program

Generate plots and output files for a reaction zone behind a shock front traveling at speed U.
 
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
from sdtoolbox.znd import zndsolve
from sdtoolbox.utilities import znd_plot
import matplotlib.pyplot as plt
import pickle

graphing = True

P1 = 13.33; T1 = 300; U1 = 4000
q = 'CO2:0.96 N2:0.04'
mech = 'airNASA9noions.cti'

gas1 = ct.Solution(mech)
gas1.TPX = T1,P1,q
nsp = gas1.n_species

# FIND POST SHOCK STATE FOR GIVEN SPEED
gas = PostShock_fr(U1, P1, T1, q, mech)

# SOLVE REACTION ZONE STRUCTURE ODES
out = zndsolve(gas,gas1,U1,t_end=0.2,max_step=0.01,relTol=1e-12,absTol=1e-12)

if graphing:
    # Demonstrate using the figure outputs from znd_plot to make further formatting
    # adjustments, then saving
    maxx = max(out['distance'])
    figT,figP,figM,figTherm,figSpec = znd_plot(out,maxx=maxx,major_species='All',
                                               xscale='log',show=False)
    
    figSpec.axes[0].set_xlim((1e-5,None))
    figSpec.axes[0].set_ylim((1e-10,1))
    
    plt.show()
    
    figP.savefig('shk-P.eps',bbox_inches='tight')
    figT.savefig('shk-T.eps',bbox_inches='tight')
    figM.savefig('shk-M.eps',bbox_inches='tight')
    figTherm.savefig('shk-therm.eps',bbox_inches='tight')
    figSpec.savefig('shk-Y.eps',bbox_inches='tight')

# Can't pickle Cantera Solution objects due to underlying C++ structure, 
# so remove it from dictionary first
out.pop('gas1') 
pickle.dump(out,open('rzn.p','wb'))
