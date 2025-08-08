"""
Shock and Detonation Toolbox Demo Program

Generates plots and output files for a constant volume
explosion simulation where the initial conditions are shocked reactants
using shock speed given by CJ detonation simulation. The time dependence
of species, pressure, and temperature are computed using the user
supplied reaction mechanism file.

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
from sdtoolbox.postshock import CJspeed, PostShock_fr
from sdtoolbox.cv import cvsolve
from sdtoolbox.utilities import CJspeed_plot, cv_plot
import cantera as ct
import datetime

P1 = 100000 
T1 = 300
q = 'H2:2 O2:1 N2:3.76'
mech = 'Mevel2017.cti'
file_name = 'h2air'

# Find CJ speed and related data, make CJ diagnostic plots
cj_speed,R2,plot_data = CJspeed(P1,T1,q,mech,fullOutput=True)
CJspeed_plot(plot_data,cj_speed)

# Set up gas object
gas1 = ct.Solution(mech)
gas1.TPX = T1,P1,q

# Find post shock state for given speed
gas = PostShock_fr(cj_speed, P1, T1, q, mech)

# Solve constant volume explosion ODEs, make cv plots
CVout = cvsolve(gas,t_end=1e-6,max_step=1e-8)
cv_plot(CVout)

print('Reaction zone pulse time (exothermic time) = '+str(CVout['exo_time']))
print('Reaction zone induction time = '+str(CVout['ind_time']))


############################################################
# CREATE OUTPUT TEXT FILE
############################################################

fid = open(file_name+'_'+str(cj_speed)+'.txt','w')
d = datetime.date.today().strftime("%B %d, %Y")

P = P1/ct.one_atm

fid.write('# CV: EXPLOSION STRUCTURE CALCULATION\n')
fid.write('# CALCULATION RUN ON %s\n\n' % d)

fid.write('# INITIAL CONDITIONS\n')
fid.write('# TEMPERATURE (K) %4.1f\n' % T1)
fid.write('# PRESSURE (ATM) %2.1f\n' % P)
fid.write('# DENSITY (KG/M^3) %1.4e\n' % gas1.density)
fid.write('# SPECIES MOLE FRACTIONS: '+q+'\n')

fid.write('# SHOCK SPEED (M/S) %5.2f\n\n' % cj_speed)

fid.write('# Induction Times\n')
fid.write('# Time to Peak DTDt =   %1.4e\n' % CVout['ind_time'])
fid.write('# Time to 0.1 Peak DTDt =   %1.4e\n' % CVout['ind_time_10'])
fid.write('# Time to 0.9 Peak DTDt =   %1.4e\n\n' % CVout['ind_time_90'])

fid.write('# REACTION ZONE STRUCTURE\n\n')

fid.write('# THE OUTPUT DATA COLUMNS ARE:\n')
fid.write('Variables = "Time(s)", "Temperature(K)", "Pressure(atm)"\n')


for val in zip(CVout['time'], CVout['T'], CVout['P']):
    fid.write('%1.4e \t %5.1f \t %3.2f\n' % val)

fid.close()

