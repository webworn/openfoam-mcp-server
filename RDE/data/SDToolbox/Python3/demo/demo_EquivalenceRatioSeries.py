"""
Shock and Detonation Toolbox Demo Program

This is a demostration of how to compute basic explosion parameters (UV,
HP and ZND) as a function of equivalence ratio for a specified fuel,
oxider and diluent. Creates a set of plots and an output file
'ExplosionParametersPhi.txt'.

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
from sdtoolbox.thermo import soundspeed_fr,soundspeed_eq
from sdtoolbox.postshock import PostShock_fr,PostShock_eq,CJspeed
from sdtoolbox.reflections import reflected_eq
from sdtoolbox.znd import zndsolve
from sdtoolbox.cv import cvsolve
import numpy as np
import matplotlib.pyplot as plt
import datetime

P1 = ct.one_atm; T1 = 300
mech = 'Mevel2017.cti'
gas  = ct.Solution(mech)
gas1 = ct.Solution(mech)
gas2 = ct.Solution(mech)
nsp  = gas.n_species

# plots?
plots = True
# output file?
output = False #True

# set initial species to be varied
fuel = 'H2'
oxidizer = 'O2'
diluent = 'N2'

# find species indices
ifuel = gas.species_index(fuel)
ioxidizer = gas.species_index(oxidizer)
idiluent = gas.species_index(diluent)

print('Initial Conditions')
print('   Pressure = '+str(P1)+' (Pa) Temperature = '+str(T1)+' (K)')
print('   Mechanism: '+mech)
print('Initial Species in Mixture ')
print('   fuel '+fuel)
print('   oxidizer '+oxidizer)
print('   diluent '+diluent)
print('--------------------------------------')

## define number of points, range of equivalence ratio, stoichiometric
# fuel-oxidizer molar ratio, diluent-oxidizer ratio
npoints = 20
phimin = 0.5
phimax = 3
deltaphi = (phimax - phimin)/(npoints-1)
stoich = 2.0   # ratio of fuel to oxidizer for stoichiometric mixture
beta = 3.76  # ratio of diluent to oxidizer 

## loop through cases

phi = []; xfuel = []; xoxidizer = []; xdiluent = []
hp_rho = []; hp_exp = []; hp_T = []; hp_ae = []
uv_P = []; uv_T = []; uv_ae = []
Ucj = []
vn_T = []; vn_P = []; vn_rho = []; vn_af = []
ind_len_ZND = []; exo_len_ZND = []
cj_T = []; cj_P = []; cj_rho = []; cj_af = []
ref_P = np.zeros(npoints); Uref = np.zeros(npoints)
P3 = []; theta_effective_CV = []

print('Computing parameters for  '+str(npoints)+' cases')
print('--------------------------------------')

for i in range(npoints):
    # define composition, this part must be modified to suit each composition
    phi.append(phimin + i*deltaphi)
    x = np.zeros(nsp)
    xfuel.append(stoich*phi[-1]/(1 + beta +stoich*phi[-1]))
    xoxidizer.append(1.0/(1 + beta + stoich*phi[-1]))
    x[ifuel] = xfuel[-1]
    x[ioxidizer] = xoxidizer[-1]
    x[idiluent] = beta/(1.0 + beta + stoich*phi[-1])
    print('Case '+str(i+1)+': phi = '+str(phi[-1]))
    print('Molefractions')
    print('   fuel ('+fuel+'): '+str(x[ifuel]))
    print('   oxidizer ('+oxidizer+'): '+ str(x[ioxidizer]))
    print('   diluent ('+diluent+'): '+ str(x[idiluent]))
    gas.TPX = T1,P1,x
    init_rho = gas.density
    init_af = soundspeed_fr(gas)
    
    # Constant Pressure Explosion State
    gas.equilibrate('HP')
    hp_rho.append(gas.density)
    hp_exp.append(init_rho/hp_rho[-1])
    hp_T.append(gas.T)
    hp_ae.append(soundspeed_eq(gas))
    
    # Constant Volume Explosion State
    gas.TPX = T1,P1,x    
    gas.equilibrate('UV')
    uv_P.append(gas.P)
    uv_T.append(gas.T)
    uv_ae.append(soundspeed_eq(gas))
    
    # CJ speed
    gas.TPX = T1,P1,x    
    Ucj.append(CJspeed(P1, T1, x, mech))

    # vN state
    gas1 = PostShock_fr(Ucj[-1], P1, T1, x, mech)
    vn_T.append(gas1.T)
    vn_P.append(gas1.P)
    vn_rho.append(gas1.density)
    vn_af.append(soundspeed_fr(gas1))

    # ZND Structure
    ZNDout = zndsolve(gas1,gas,Ucj[-1],t_end=1e-4,advanced_output=True)
    ind_len_ZND.append(ZNDout['ind_len_ZND'])
    exo_len_ZND.append(ZNDout['exo_len_ZND'])

    # CJ state
    gas1 = PostShock_eq(Ucj[-1],P1, T1,x,mech)
    cj_T.append(gas1.T)
    cj_P.append(gas1.P)
    cj_rho.append(gas1.density)
    cj_af.append(soundspeed_fr(gas1))
    # Reflected CJ state
    [ref_P[i],Uref[i],gas2] = reflected_eq(gas,gas1,gas2,Ucj[-1])

    # State 3 - Plateau at end of Taylor wave
    # print('Generating points on isentrope and computing Taylor wave velocity')
    w2 = gas.density*Ucj[-1]/cj_rho[-1]
    S2 = gas1.entropy_mass
    u2 = Ucj[-1] - w2
    u = [u2]
    P = [gas1.P]
    R = [gas1.density]
    V = [1/R[0]]
    #T = [gas1.T]
    a = [soundspeed_eq(gas1)]
    vv = 1/cj_rho[-1]
    
    while u[-1] > 0:
        vv = vv*1.01
        gas1.SVX = S2,vv,gas1.X
        gas1.equilibrate('SV')
        P.append(gas1.P)
        R.append(gas1.density)
        V.append(1/R[-1])
        a.append(soundspeed_eq(gas1))
        u.append(u[-1] + 0.5*(P[-1]-P[-2])*(1/(R[-1]*a[-1]) + 1/(R[-2]*a[-2])))

    # estimate plateau conditions (state 3) by interpolating to find the u = 0 state.
    P3.append(P[-1] + u[-1]*(P[-2]-P[-1])/(u[-2]-u[-1]))

    # Approximate the effective activation energy using finite differences
    gas = PostShock_fr(Ucj[-1], P1, T1, x, mech)
    Ts = gas.T; Ps = gas.P
    Ta = Ts*(1.02)
    gas.TPX = Ta,Ps,x
    CVout1 = cvsolve(gas,t_end=5e-5)
    Tb = Ts*(0.98)
    gas.TPX = Tb,Ps,x
    CVout2 = cvsolve(gas,t_end=5e-5)   
    # Approximate effective activation energy for CV explosion
    taua = CVout1['ind_time']
    taub = CVout2['ind_time']
    if taua==0 and taub==0:
        theta_effective_CV.append(0)
    else:
        theta_effective_CV.append(1/Ts*((np.log(taua)-np.log(taub))/((1/Ta)-(1/Tb)))) 

    print('--------------------------------------');


###############################
# CREATE OUTPUT TEXT FILE
###############################
if output:
    fn = 'ExplosionParametersPhi.txt'
    d = datetime.date.today()
    fid = open(fn, 'w')
    fid.write('# Explosion Parameters\n')
    fid.write('# Computation done on %s\n' % d)
    fid.write('# Mechanism %s\n' % mech)
    fid.write('# Initial Species in Mixture \n')
    fid.write('# fuel %s\n' % fuel)
    fid.write('# oxidizer %s\n' % oxidizer)
    fid.write('# diluent %s\n' % diluent)
    fid.write('# Initial conditions\n')
    fid.write('# Temperature (K) %4.1f\n' % T1)
    fid.write('# Pressure (Pa) %2.1f\n\n' % P1)
    fid.write('Variables = "Equivalence Ratio","X fuel","X oxidizer","CV Pressure","CV Temperature","HP expansion","HP soundspeed","CJ Speed","CJ Pressure ","CJ sound speed","vN Pressure ","Reflected Pressure","Reflected Speed","Plateau Pressure","Ind Length","Energy Length","Theta Effective"\n')
    for i in range(npoints): 
        fid.write('%14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e  \t %14.5e  \t %14.5e  \t %14.5e  \t %14.5e  \t %14.5e \t %14.5e \t %14.5e \t %14.5e\n' %
                  (phi[i],xfuel[i],xoxidizer[i],uv_P[i],uv_T[i],hp_exp[i],hp_ae[i],Ucj[i],cj_P[i],cj_af[i],vn_P[i],ref_P[i],Uref[i],P3[i],ind_len_ZND[i],exo_len_ZND[i],theta_effective_CV[i]))
    fid.close()


if plots:
    plt.figure(1)
    maxy = max([max(cj_T),max(hp_T),max(uv_T)])
    miny = min([min(cj_T),min(hp_T),min(uv_T)])
    maxx = max(phi)
    minx = min(phi)
    fontsize=12
    plt.xlabel('Equivalence Ratio',fontsize=fontsize)
    plt.ylabel('Temperature (K)',fontsize=fontsize)
    plt.title('Explosion Temperature',fontsize=fontsize)
    plt.axis([minx,maxx,miny,maxy])
    plt.plot(phi,cj_T,'r')
    plt.plot(phi,hp_T,'g')
    plt.plot(phi,uv_T,'b')
    plt.legend(['CJ', 'HP', 'UV'])
    plt.tight_layout()


    ref_P = np.asarray(ref_P); cj_P = np.asarray(cj_P); uv_P = np.asarray(uv_P); P3 = np.asarray(P3)
    plt.figure(2)
    maxy = max([max(cj_P),max(uv_P),max(ref_P),max(P3)])
    miny = min([min(cj_P),min(uv_P),min(ref_P),min(P3)])
    plt.xlabel('Equivalence Ratio',fontsize=fontsize)
    plt.ylabel('Pressure (MPa)',fontsize=fontsize)
    plt.title('Explosion Pressure',fontsize=fontsize)
    plt.axis([minx,maxx,miny/1e6,maxy/1e6])
    plt.plot(phi,ref_P/1e6,'r')
    plt.plot(phi,cj_P/1e6,'b')
    plt.plot(phi,uv_P/1e6,'g')
    plt.plot(phi,P3/1e6,'c')
    plt.legend(['CJ ref','CJ', 'UV','P3'])
    plt.tight_layout()

    plt.figure(3)
    maxy = max([max(Ucj),max(Uref),max(hp_ae)])
    miny = min([min(Ucj),min(Uref),min(hp_ae)])
    plt.xlabel('Equivalence Ratio',fontsize=fontsize)
    plt.ylabel('speed (m/s)',fontsize=fontsize)
    plt.title('Explosion Speed',fontsize=fontsize)
    plt.axis([minx,maxx,miny,maxy])
    plt.plot(phi,Uref,'r')
    plt.plot(phi,Ucj,'b')
    plt.plot(phi,hp_ae,'g')
    plt.legend(['CJ ref','CJ', r'$a_e$ (HP)'])
    plt.tight_layout()

    plt.figure(4)
    maxy = max([max(ind_len_ZND),max(exo_len_ZND)])
    miny = min([min(ind_len_ZND),min(exo_len_ZND)])
    plt.xlabel('Equivalence Ratio',fontsize=fontsize)
    plt.ylabel('length (m)',fontsize=fontsize)
    plt.title('Explosion Lengths',fontsize=fontsize)
    plt.axis([minx,maxx,miny,maxy])
    plt.semilogy(phi,ind_len_ZND,'r')
    plt.semilogy(phi,exo_len_ZND,'b')
    plt.legend([r'$\Delta_{\sigma max}$',r'$\Delta_{\sigma}$'])
    plt.tight_layout()
    
    plt.show()

