"""
Shock and Detonation Toolbox Demo Program

Generate plots and output files for a ZND detonation with the shock front traveling at speed U.

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
from sdtoolbox.postshock import PostShock_fr
from sdtoolbox.znd import zndsolve
from sdtoolbox.utilities import znd_plot, znd_fileout
import cantera as ct

P1 = 100000 
T1 = 300
U1 = 3000
q = 'H2:2 O2:1 N2:3.76'
mech = 'Mevel2017.cti'
file_name = 'h2air'

# Set up gas object
gas1 = ct.Solution(mech)
gas1.TPX = T1,P1,q

# Find post shock state for given speed
gas = PostShock_fr(U1, P1, T1, q, mech)

# Solve ZND ODEs, make ZND plots
znd_out = zndsolve(gas,gas1,U1,advanced_output=True)
znd_plot(znd_out)
znd_fileout(file_name,znd_out)

print('Reaction zone pulse width (exothermic length) = %.4g m' % znd_out['exo_len_ZND'])
print('Reaction zone induction length = %.4g m' % znd_out['ind_len_ZND'])
print('Reaction zone pulse time (exothermic time) = %.4g s' % znd_out['exo_time_ZND'])
print('Reaction zone induction time = %.4g s' % znd_out['ind_time_ZND'])
