"""
Shock and Detonation Toolbox
"thermo" module

Calculates various thermodynamic quantities, including sound speed and the 
Gruneisen coefficient for frozen and equilibrium cases.
 
This module defines the following functions:

    soundspeed_eq
    soundspeed_fr
    gruneisen_eq
    gruneisen_fr
    eq_state
    state
    
################################################################################
Theory, numerical methods and applications are described in the following report:

    SDToolbox Numerical Tools for Shock and Detonation Wave Modeling, 
    Explosion Dynamics Laboratory, Contributors: 
    S. Browne, J. Ziegler, N. Bitter, B. Schmidt, J. Lawson and J. E. Shepherd, 
    GALCIT Technical Report FM2018.001 Revised January 2021.
    California Institute of Technology, Pasadena, CA USA

Please cite this report and the website if you use these routines. 

Please refer to LICENCE.txt or the above report for copyright and disclaimers.

http://shepherd.caltech.edu/EDL/PublicResources/sdt/


################################################################################ 
Updated January 2021
Tested with: 
    Python 3.79 and Cantera 2.4
Under these operating systems:
    Windows 10, Linux (Ubuntu)
"""
import numpy as np

def soundspeed_eq(gas):
    """
    Computes the equilibrium sound speed by using a finite
    difference approximation. Directly evaluating pressure at two density/specific
    volume states along an isentrope requires use of equilibrate('SV'). However,
    this may not always converge at high pressure. Instead, a more robust method
    using equilibrate('TP') is used that employs thermodynamic identities detailed
    further in Appendix G2 of the report.
    
    The old version of this code that used 'SV' instead of 'TP' is commented out
    below for reference.
    
    FUNCTION SYNTAX:
        ae =  soundspeed_eq(gas)
    
    INPUT:
        gas = working gas object (restored to original state at end of function)
    
    OUTPUT:
        ae = equilibrium sound speed = sqrt({d P/d rho)_s, eq) (m/s)
    
    """
#    Old method which is not reliable, 'SV' will not converge at high pressure
#    rho0 = gas.density
#    p0 = gas.P
#    s0 = gas.entropy_mass
#    rho1 = 1.01*rho0
#    x0 = gas.X;
#    gas.SVX =  s0, 1./rho1,x0
#    gas.equilibrate('SV')
#    p1 = gas.P
#    dpdrho_s = (p1 - p0)/(rho1 - rho0)
#    aequil = math.sqrt(dpdrho_s)
#    gas.SVX =  s0, 1./rho0, x0
    
    # New method based on Taylor series expansion, centered differences using thermodynamic identities
    T0 = gas.T
    P0 = gas.P
    x0 = gas.X
    
    T2 = 1.01*T0
    T1 = 0.99*T0    
    gas.TP =  T1, P0
    gas.equilibrate('TP')
    s1 = gas.entropy_mass
    
    gas.TP =  T2, P0
    gas.equilibrate('TP')
    s2 = gas.entropy_mass
    
    DSDT = (s2 - s1)/(T2 - T1)
    
    P1 = 0.99*P0
    P2 = 1.01*P0
    gas.TP =  T0, P1
    gas.equilibrate('TP')
    s1 = gas.entropy_mass
    
    gas.TP =  T0, P2
    gas.equilibrate('TP')
    s2 = gas.entropy_mass
    
    DSDP = (s2 - s1)/(P2 - P1)
    
    DTDP = -DSDP/DSDT
    
    TA = T0 + DTDP*(P1-P0)
    gas.TP = TA, P1
    gas.equilibrate('TP')
    rhoA = gas.density
    
    TB = T0 + DTDP*(P2-P0)
    gas.TP = TB, P2
    gas.equilibrate('TP')
    rhoB = gas.density
    
    DRHODP = (P2-P1)/(rhoB-rhoA)
    
    ae = np.sqrt(DRHODP)
    
    # Restore gas object to original state
    gas.TPX =  T0, P0, x0
    return ae

def soundspeed_fr(gas):
    """
    Computes the frozen sound speed by using a finite
    difference approximation and evaluating frozen composition states on the
    isentrope passing through the reference (S, V) state supplied by the gas
    object passed to the function. 
    
    FUNCTION SYNTAX:
        afrz =  soundspeed_fr(gas)
    
    INPUT:
        gas = working gas object (restored to original state at end of function)
    
    OUTPUT:
        afrz = frozen sound speed = sqrt({d P/d rho)_{s,x0}) 

    """
    rho0 = gas.density
    p0 = gas.P
    s0 = gas.entropy_mass
    rho1 = 1.001*rho0
    x0 = gas.X
    # workaround to avoid unsized object error when only one species in a .cti file
    # (flagged to be fixed in future Cantera version)
    if gas.n_species > 1:
        gas.SVX =  s0, 1./rho1, x0
    else:
        gas.SV = s0, 1./rho1
    p1 = gas.P
    dpdrho_s = (p1 - p0)/(rho1 - rho0)
    afrz = np.sqrt(dpdrho_s)
    
    # Restore gas object to original state
    if gas.n_species > 1:
        gas.SVX =  s0, 1./rho0, x0
    else:
        gas.SV = s0, 1./rho0
        
    return afrz

def gruneisen_eq(gas):
    """
    Computes the equilibrium Gruneisen coefficient by using a centered finite
    difference approximation and evaluating equilibrium states on the
    isentrope passing through the reference (S, V) state supplied by the gas
    object passed to the function. If the reference state is not in
    equilibrium, the result of the call is still valid but requires care in
    interpretation.
    
    Note: Because this function uses equilibrate('SV'), it could be susceptible to the same
    problems at high pressure as soundspeed_eq. If such issues are encountered,
    this function could be altered in the same was as soundspeed_eq was.
    For details on how this could be implemented, refer to Appendix G3 of the report.
    
    FUNCTION SYNTAX:
        G_eq =  gruneisen_eq(gas)
    
    INPUT:
        gas = working gas object (restored to original state at end of function)
    
    OUTPUT:
        G_eq = equilibrium Gruneisen coefficient [-de/dp)_{v,eq} =
                -(v/T)dT/dv)_{s,eq} = + (rho/T)(dT/d rho)_{s,eq}]
    
    """
    rho0 = gas.density
    T0 = gas.T
    s0 = gas.entropy_mass
    rho1 = 0.99*rho0
    x0 = gas.X;
    gas.SVX =  s0, 1./rho1, x0
    gas.equilibrate('SV')
    T1 = gas.T
    dtdrho = (T1 - T0)/(rho1 - rho0)
    rho =  (rho1+rho0)/2
    T = (T1+T0)/2
    G_eq = dtdrho*rho/T
    # Restore to original state
    gas.SVX =  s0, 1./rho0, x0
    return G_eq

def gruneisen_fr(gas):
    """
    Computes the frosen Gruneisen coefficient by using a centered finite
    difference approximation and evaluating frozen states on the
    isentrope passing through the reference (S, V) state supplied by the gas
    object passed to the function. 
    
    FUNCTION SYNTAX:
        G_fr =  gruneisen_fr(gas)
    
    INPUT:
        gas = working gas object (not modified in function)
    
    OUTPUT:
        G_eq = frozen Gruneisen coefficient [-de/dp)_{v,x0} =
                -(v/T)dT/dv)_{s,x0} = + (rho/T)(dT/d rho)_{s,x0}]
    
    """
    # Frozen Gruneisen coefficient
    rho0 = gas.density
    T0 = gas.T
    s0 = gas.entropy_mass
    rho1 = 0.99*rho0
    x0 = gas.X;
    gas.SVX =  s0, 1./rho1, x0
    T1 = gas.T
    dtdrho = (T1 - T0)/(rho1 - rho0)
    rho =  (rho1+rho0)/2
    T = (T1+T0)/2
    G_fr = dtdrho*rho/T
    # Restore to original state
    gas.SVX =  s0, 1./rho0, x0
    return G_fr

def eq_state(gas,r1,T1):
    """
    Calculates equilibrium state given T & rho.
    Used in postshock module.
    
    FUNCTION SYNTAX:
        [P,H] = eq_state(gas,r1,T1)
    
    INPUT:
        gas = working gas object (gas object is changed and corresponds to new equilibrium state)
        r1,T1 = desired density and temperature
    
    OUTPUT
        P,H = equilibrium pressure and enthalpy at given temperature and density

    """
    gas.TD = T1,r1
    gas.equilibrate('TV')
    P = gas.P
    H = gas.enthalpy_mass
    return [P, H]

def state(gas,r1,T1):
    """
    Calculates frozen state given T & rho.
    Used in postshock module.
    
    FUNCTION SYNTAX:
        [P,H] = state(gas,r1,T1)
    
    INPUT:
        gas = working gas object (gas object is changed and corresponds to new frozen state)
        r1,T1 = desired density and temperature
    
    OUTPUT:
        P,H = frozen pressure and enthalpy at given temperature and density

    """
    gas.TD = T1,r1
    P = gas.P
    H = gas.enthalpy_mass
    return [P, H]

