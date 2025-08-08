"""
Shock and Detonation Toolbox Demo Program

Computes ZND and CV models of detonation with the shock front
traveling at the CJ speed.  Evaluates various measures of the reaction
zone thickness and exothermic pulse width, effective activation energy
and Ng stability parameter. 
 
################################################################################
Theory, numerical methods and applications are described in the following report:

SDToolbox - Numerical Tools for Shock and Detonation Wave Modeling,
Explosion Dynamics Laboratory, Contributors: S. Browne, J. Ziegler,
N. Bitter, B. Schmidt, J. Lawson and J. E. Shepherd, GALCIT
Technical Report FM2018.001 Revised January 2021.

Please cite this report and the website if you use these routines. 

Please refer to LICENCE.txt or the above report for copyright and disclaimers.

http://shepherd.caltech.edu/EDL/public/sdt/SD_Toolbox/

################################################################################ 
Updated August 2018 - Revised 1/19/2019 to correct Ng Chi parameter definition
Tested with: 
    Python 3.5 and 3.6, Cantera 2.3 and 2.4
Under these operating systems:
    Windows 8.1, Windows 10, Linux (Debian 9)
"""
from sdtoolbox.postshock import CJspeed, PostShock_fr, PostShock_eq
from sdtoolbox.znd import zndsolve
from sdtoolbox.cv import cvsolve
from sdtoolbox.utilities import CJspeed_plot, znd_plot
import cantera as ct
import numpy as np

def gavrikov(delta,theta,Tvn,T0):
    """
    Correlation function for detonation cell width 
    proposed by Gavrikov et al COMBUSTION AND FLAME 120:19�33 (2000)
    based on using a reaction zone length based on time to 50limiting
    reactant consumption in constant volume explosion approximation using vn
    postshock velocity to convert time to distance.   Tested against a range
    of fuel-oxidizer diluent mixtures

    INPUT:
        delta = reaction zone length based on time to 50consumption of limiting
        reactant from CV computation and delta = time * w_VN
        theta = Ea/RT_VN,  effective reduced activation energy based on CV
        computation
        Tvn = von Neumann (postshock temperature behind CJ shock wave)
        T0 = initial temperature
    """
    # Constants
    a = -0.007843787493
    b = 0.1777662961
    c = 0.02371845901
    d = 1.477047968
    e = 0.1545112957
    f = 0.01547021569
    g = -1.446582357
    h = 8.730494354
    i = 4.599907939
    j = 7.443410379
    k = 0.4058325462
    m = 1.453392165
    # Define nondimensional parameters
    X = theta
    Y = Tvn/T0
    z = Y*(a*Y-b) + X*(c*X-d + (e-f*Y)*Y) + g*np.log(Y) + h*np.log(X) + Y*(i/X - k*Y/X**m) - j
    lam = delta*np.power(10,z)
    return lam

def ng(delta,chi):
    """
    Correlation function for detonation cell size from
    Ng, Hoi Dick, Yiguang Ju, and John H. S. Lee. 2007. Assessment of
    Detonation Hazards in High-Pressure Hydrogen Storage from Chemical
    Sensitivity Analysis. INTERNATIONAL JOURNAL OF HYDROGEN ENERGY 32 (1):
    93-99.
    Tested only against low pressure H2-air data.
    
    INPUT:
        delta = reaction zone length based on peak thermicity in ZND simulation
        chi = theta*Delta_i/Delta_r where 
              theta = reduced effective activation energy from CV computation
              Delta_i = distance to peak thermicity from ZND computation
              Delta_r = w_vN/sigmadot_max from ZND computation
    
    See Ng et al.  Combustion Theory and Modeling 2005 for a discussion of
    the chi parameter. 
    """ 
    # Constants
    A0 = 30.465860763763;
    a1 = 89.55438805808153;
    a2 = -130.792822369483;
    a3 = 42.02450507117405;
    b1 = -0.02929128383850;
    b2 = 1.0263250730647101E-5;
    b3 = -1.031921244571857E-9;
    lam = delta*(A0 + ((a3/chi + a2)/chi + a1)/chi + ((b3*chi + b2)*chi + b1)*chi);
    return lam

P1 = 100000; T1 = 300
q = 'H2:2 O2:1 N2:3.76'
mech = 'Mevel2017.cti'
fname = 'h2air'

# Find CJ speed and related data, make CJ diagnostic plots
cj_speed,R2,plot_data = CJspeed(P1,T1,q,mech,fullOutput=True)
CJspeed_plot(plot_data,cj_speed)

# Set up gas object
gas1 = ct.Solution(mech)
gas1.TPX = T1,P1,q

# Find equilibrium post shock state for given speed
gas = PostShock_eq(cj_speed, P1, T1, q, mech)
u_cj = cj_speed*gas1.density/gas.density

# Find frozen post shock state for given speed
gas = PostShock_fr(cj_speed, P1, T1, q, mech)

# Solve ZND ODEs, make ZND plots
out = zndsolve(gas,gas1,cj_speed,t_end=1e-3,advanced_output=True)

# Find CV parameters including effective activation energy
gas.TPX = T1,P1,q
gas = PostShock_fr(cj_speed, P1, T1, q, mech)
Ts = gas.T; Ps = gas.P
Ta = Ts*1.02
gas.TPX = Ta,Ps,q
CVout1 = cvsolve(gas)
Tb = Ts*0.98
gas.TPX = Tb,Ps,q
CVout2 = cvsolve(gas)
# Approximate effective activation energy for CV explosion
taua = CVout1['ind_time']
taub = CVout2['ind_time']
if taua==0 and taub==0:
    theta_effective_CV = 0
else:
    theta_effective_CV = 1/Ts*((np.log(taua)-np.log(taub))/((1/Ta)-(1/Tb)))

#  Find Gavrikov induction length based on 50% limiting species consumption,
#  fuel for lean mixtures, oxygen for rich mixtures
#  Westbrook time based on 50% temperature rise
limit_species = 'H2'
i_limit = gas.species_index(limit_species)
gas.TPX = Ts,Ps,q
X_initial = gas.X[i_limit]
gas.equilibrate('UV')
X_final = gas.X[i_limit]
T_final = gas.T
X_gav = 0.5*(X_initial - X_final) + X_final
T_west = 0.5*(T_final - Ts) + Ts


for i,X in enumerate(CVout1['speciesX'][i_limit,:]):
    if X > X_gav:
        t_gav = CVout1['time'][i]
        
x_gav = t_gav*out['U'][0]        
for i,T in enumerate(CVout1['T']):
    if T < T_west:
        t_west = CVout1['time'][i]
x_west = t_west*out['U'][0]

max_thermicity_width_ZND = u_cj/out['max_thermicity_ZND']
chi_ng = theta_effective_CV*out['ind_len_ZND']/max_thermicity_width_ZND
cell_gav = gavrikov(x_gav,theta_effective_CV, Ts, T1)
cell_ng = ng(out['ind_len_ZND'], chi_ng)

print('ZND computation results; ')
print('Reaction zone computation end time = {:8.3e} s'.format(out['tfinal']))
print('Reaction zone computation end distance = {:8.3e} m'.format(out['xfinal']))
print(' ')
print('T (K), initial = {:1.5g}, final = {:1.5g}, max = {:1.5g} K'.format(out['T'][0],out['T'][-1],max(out['T'])))
print('P (Pa), initial = {:1.5g}, final = {:1.5g}, max = {:1.5g} K'.format(out['P'][0],out['P'][-1],max(out['P'])))
print('M, initial = {:1.5g}, final = {:1.5g}, max = {:1.5g}'.format(out['M'][0],out['M'][-1],max(out['M'])))
print('u (m/s), initial = {:1.5g}, final = {:1.5g}, u_cj = {:1.5g}'.format(out['U'][0],out['U'][-1],u_cj))
print(' ')
print('Reaction zone thermicity half-width = {:8.3e} m'.format(out['exo_len_ZND']))
print('Reaction zone maximum thermicity distance = {:8.3e} m'.format(out['ind_len_ZND']))
print('Reaction zone thermicity half-time = {:8.3e} s'.format(out['exo_time_ZND']))
print('Reaction zone maximum thermicity time = {:8.3e} s'.format(out['ind_time_ZND']))
print('Reaction zone width (u_cj/sigmadot_max) = {:8.3e} m'.format(max_thermicity_width_ZND))
print(' ')
print('CV computation results; ')
print('Time to dT/dt_max = {:8.3e} s'.format(CVout1['ind_time']))
print('Distance to dT/dt_max = {:8.3e} m'.format(CVout1['ind_time']*out['U'][0]))
print('Reduced activation energy) = {:8.3e}'.format(theta_effective_CV))
print('Time to 50% consumption = {:8.3e} s'.format(t_gav))
print('Distance to 50% consumption = {:8.3e} m'.format(x_gav))
print('Time to 50% temperature rise = {:8.3e} s'.format(t_west))
print('Distance to 50% temperature = {:8.3e} m'.format(x_west))
print(' ')
print('Cell size predictions ')
print('Gavrikov correlation = {:8.3e} m'.format(cell_gav))
print('Ng et al Chi Parameter = {:8.3e} m'.format(chi_ng))
print('Ng et al correlation = {:8.3e} m'.format(cell_ng))
print('Westbrook correlation = {:8.3e} m'.format(29*x_west))


znd_plot(out,maxx=0.001,
         major_species=['H2', 'O2', 'H2O'],
         minor_species=['H', 'O', 'OH', 'H2O2', 'HO2'])


