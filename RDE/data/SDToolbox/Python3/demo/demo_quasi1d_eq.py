"""
Shock and Detonation Toolbox Demo Program

Computes ideal quasi-one dimensional flow using equilibrium properties to 
determine exit conditions for expansion to a specified pressure.  
Carries out computation for a range of helium dilutions.

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
import cantera
from sdtoolbox.thermo import soundspeed_eq
import numpy as np

# set composition and stagnation state
mech = 'Mevel2017.cti'
gas = cantera.Solution(mech)
gas1 = cantera.Solution(mech)

Tc = 300;
Pc = 1e6;
for helium in range(1,21):
    q = 'O2:1 H2:2.001 He:'+str(helium)
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
    
    P = []; R = []; T = []; c = [];  M = []; 
    h_eq = []; u_eq = []; Isp_eq = [];
    h_fr = []; u_fr = []; Isp_fr = []; 
    
    imax = 200
    for i in range(imax):
        if i < 10:
            pp = pp*0.99
        else:
            pp = pp*0.95

        # compute equilibrium isentrope
        gas.SPX = St,pp,gas.X
        gas.equilibrate('SP')
        
        P.append(gas.P)
        R.append(gas.density)
        T.append(gas.T)
        c.append(soundspeed_eq(gas))
        h_eq.append(gas.enthalpy_mass)
        u_eq.append(np.sqrt(2*(Ht-h_eq[i])))
        Isp_eq.append((u_eq[i]+ (P[i] - Pa)/(u_eq[i]*R[i]))/9.81)
        M.append(u_eq[i]/c[i])
        
        # compute frozen impulse
        gas1.SPX = St,pp,qc
        h_fr.append(gas1.enthalpy_mass)
        u_fr.append(np.sqrt(2*(Ht-h_fr[i])))
        Isp_fr.append((u_fr[i]+ (P[i] - Pa)/(u_fr[i]*R[i]))/9.81)
    
    ##
    print('final pressure '+str(P[-1])+' (Pa)')
    print('final specific impulse (eq) '+str(Isp_eq[-1])+' (s)')
    print('final velocity (eq) '+str(u_eq[-1])+' (m/s)')
    print('final specific impulse (fr) '+str(Isp_fr[-1])+' (s)')
    print('final velocity (fr) '+str(u_fr[-1])+' (m/s)')

    # bracket sonic state
    M = np.asarray(M)
    istar = np.argmax(M>1)
    
    # interpolate to find throat conditions
    Pstar = P[istar-1] + (P[istar] - P[istar-1])*(1.0 - M[istar-1])/(M[istar]-M[istar-1])
    Tstar = T[istar-1] + (T[istar] - T[istar-1])*(1.0 - M[istar-1])/(M[istar]-M[istar-1])
    Rstar = R[istar-1] + (R[istar] - R[istar-1])*(1.0 - M[istar-1])/(M[istar]-M[istar-1])
    cstar = c[istar-1] + (c[istar] - c[istar-1])*(1.0 - M[istar-1])/(M[istar]-M[istar-1])
    rhocstar = cstar*Rstar
    
    print('Sonic state (eq) pressure '+str(Pstar)+' (Pa)')
    print('Sonic state (eq) temperature '+str(Tstar)+' (s)')
    print('Sonic state (eq) velocity '+str(cstar)+' (m/s)')
    print('Sonic state (eq) mass flux '+str(rhocstar)+' (kg/m^2s)')
    
    print('-------------------------------------------\n\n')
    
    # compute the area ratio A/A* based on equilibrium flow properties
    u_eq = np.asarray(u_eq); R = np.asarray(R)
    area = rhocstar/(u_eq*R)
