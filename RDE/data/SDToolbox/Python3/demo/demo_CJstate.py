"""
Shock and Detonation Toolbox Demo Program

Calculates the CJ speed using the Minimum Wave Speed Method and 
then finds the equilibrium state of the gas behind a shock wave 
traveling at the CJ speed.
 
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
from sdtoolbox.postshock import PostShock_eq
from sdtoolbox.thermo import soundspeed_eq, soundspeed_fr
# Initial state specification:
# P1 = Initial Pressure  
# T1 = Initial Temperature 
# U = Shock Speed 
# q = Initial Composition 
# mech = Cantera mechanism File name

# stoichiometric H2-air detonation at nominal atmospheric conditions
P1 = 100000.
T1 = 295
q ='H2:2 O2:1 N2:3.76'
mech = 'Mevel2017.cti'
gas_initial = ct.Solution(mech)
gas_initial.TPX = T1, P1, q
rho_1 = gas_initial.density

# compute CJ speed
[cj_speed,R2,plot_data] = CJspeed(P1, T1, q, mech, fullOutput=True)  

# compute equilibrium CJ state parameters
gas = PostShock_eq(cj_speed, P1, T1, q, mech)
ae = soundspeed_eq(gas)
af = soundspeed_fr(gas)
rho_2 = gas.density
gammae = ae**2*rho_2/gas.P
gammaf = af**2*rho_2/gas.P
w2 = cj_speed*rho_1/rho_2
u2 = cj_speed-w2
print ('CJ computation for ' + mech + ' with composition ' + q )
print ('Initial conditions: P1 = %.3e Pa & T1 = %.2f K'  % (P1,T1)  )
print ('CJ Speed   %.1f m/s' % cj_speed)
print ('CJ State')
print ('   Pressure   %.3e Pa' % gas.P)
print ('   Temperature  %.1f K' % gas.T)
print ('   Density  %.3f kg/m3' % gas.density)
print ('   Entropy  %.3e J/K' % gas.entropy_mass)
print ('   w2 (wave frame) %.1f m/s' % w2)
print ('   u2 (lab frame) %.1f m/s' % u2)
print ('   c2 frozen %.1f m/s' % af)
print ('   c2 equilbrium %.1f m/s' % ae)
print ('   gamma2 frozen %.3f ' % gammaf)
print ('   gamma2 equilbrium %.3f ' % gammae)
