"""
Shock and Detonation Toolbox Demo Program

Calculate detonation Chapman-Jouguet (CJ) speed based on the
initial gas state.  Uses the minimum wave speed method and the SDToolbox
function 'CJspeed' from the 'postshock' module.
 
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
from sdtoolbox.postshock import CJspeed
from sdtoolbox.utilities import CJspeed_plot

# Initial state specification:
# P1 = Initial Pressure  
# T1 = Initial Temperature 
# U = Shock Speed 
# q = Initial Composition 
# mech = Cantera mechanism File name

P1 = 100000 
P1atm = P1/ct.one_atm
T1 = 300
U = 2000
q = 'H2:2 O2:1 N2:3.76'
mech = 'Mevel2017.cti'
gas = ct.Solution(mech)

# Outputs:
# cj_speed - detonation speed 
# R2 - R-squared value of wave speed - density fit
# plot_data - further values relating to the fit procedure
[cj_speed,R2,plot_data] = CJspeed(P1, T1, q, mech, fullOutput=True)
dratio_cj = plot_data[2]

#print ('Initial state: ' + q + ', P1 = %.2f atm,  T1 = %.2f K' % (P1atm,T1) )
#print ('Mechanism: ' + mech)
#print ('CJ Speed is %.2f m/s' % cj_speed)

print('CJ computation for '+mech+' with composition '+q)
print('CJ speed '+str(round(cj_speed,4))+' (m/s)'+' Density ratio '+str(round(dratio_cj,4)))
print('Use demo_CJstate to find thermodynamic properties at CJ point')

# Get diagnostic plots for minimum wave speed calculation
CJspeed_plot(plot_data,cj_speed)

