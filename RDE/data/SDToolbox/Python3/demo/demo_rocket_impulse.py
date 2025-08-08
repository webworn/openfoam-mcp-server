"""
Shock and Detonation Toolbox Demo Program

Computes rocket performance using quasi-one dimensional isentropic flow using 
both frozen and equilibrium properties for a range of helium dilutions in a 
hydrogen-oxygen mixture.  Plots impulse as a function of dilution.

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
import cantera # don't rename as ct, conflicts with a variable name in this script
from sdtoolbox.thermo import soundspeed_eq
import numpy as np
import matplotlib.pyplot as plt

# set composition and stagnation state
mech = 'Mevel2017.cti'
gas = cantera.Solution(mech)
gas1 = cantera.Solution(mech)

he = []; I_fr = []; I_eq = []
for helium in range(40):
    he.append((0.1*helium)/(0.1*helium+1.5))
    q = 'O2:0.5 H2:1.001 He:'+str(0.1*helium)
    if helium == 0:
        q = 'O2:0.5 H2:1.001'

    Tc = 300
    Pc = 1.e7
    gas.TPX = Tc,Pc,q
    print('Computing chamber conditions and isentropic quasi 1-d flow for '+q+' using '+mech)
    # compute equilibrium properties for constant pressure burn
    gas.equilibrate('HP')
    qc = gas.X
    # Isentropic expansion from stagnation state to low pressure
    Tt = gas.T
    St = gas.entropy_mass
    Ht = gas.enthalpy_mass
    Pt = gas.P
    Rt = gas.density
    ct = soundspeed_eq(gas)
    gammat_eq = ct**2*Rt/Pt
    
    print('Total pressure '+str(Pt)+' (Pa)')
    print('Total temperature '+str(Tt)+' (K)')
    print('Total density '+str(Rt)+' (kg/m3)')
    print('Total entropy '+str(St)+' (J/kg-K)')
    print('Total sound speed (equilibrium) '+str(ct)+' (m/s)')
    print('gamma2 (equilibrium) '+str(gammat_eq)+' (m/s)')
    
    # ambient pressure for impulse computation
    Pa = 0.0
    print('Computing isentropic expansion')
    pp = Pt
    
    P = []; R = []; T = [];
    h_eq = []; u_eq = []; Isp_eq = [];
    h_fr = []; R_fr = []; T_fr = []; u_fr = []; Isp_fr = [];
    
    imax = 90
    for i in range(imax):
        pp = pp*0.95
        # compute equilibrium isentrope
        gas.SPX = St,pp,gas.X        
        gas.equilibrate('SP')
        
        P.append(gas.P)
        R.append(gas.density)
        T.append(gas.T)
        h_eq.append(gas.enthalpy_mass)
        u_eq.append(np.sqrt(2*(Ht-h_eq[i])))
        Isp_eq.append((u_eq[i]+ (P[i] - Pa)/(u_eq[i]*R[i]))/9.81)
        # compute frozen impulse  (first call is so we get a good guess for entropy)
        gas1.TPX = T[i],pp,qc
        gas1.SPX = St,pp,qc
        h_fr.append(gas1.enthalpy_mass)
        R_fr.append(gas1.density)
        T_fr.append(gas1.T)
        u_fr.append(np.sqrt(2*(Ht-h_fr[i])))
        Isp_fr.append((u_fr[i]+ (P[i] - Pa)/(u_fr[i]*R_fr[i]))/9.81)

    I_fr.append(Isp_fr[-1])
    I_eq.append(Isp_eq[-1])
    print('final pressure '+str(P[-1])+' (Pa)')
    print('final specific impulse (eq) '+str(Isp_eq[-1])+' (s)')
    print('final velocity (eq) '+str(u_eq[-1])+' (m/s)')
    print('final specific impulse (fr) '+str(Isp_fr[-1])+' (s)')
    print('final velocity (fr) '+str(u_fr[-1])+' (m/s)')
    print('-------------------------------------------\n\n')


#
# Plot the impulses
#
plt.figure
fontsize = 12
plt.plot(he, I_fr, 'r')
plt.plot(he, I_eq, 'b')
plt.xlabel('helium mole fraction ',fontsize=fontsize);
plt.ylabel('Isp (s)',fontsize=fontsize);
plt.title('Warm gas thruster performance',fontsize=fontsize);
plt.legend(['Frozen','Equilibrium'])
plt.tick_params(labelsize=12)
plt.xlim(xmin=0)

plt.show()
