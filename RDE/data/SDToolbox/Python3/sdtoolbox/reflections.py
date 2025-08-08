""""
Shock and Detonation Toolbox
"reflections" module

Calculates post-reflected-shock states for frozen and equilibrium cases.
 
This module defines the following functions:

    reflected_fr
    reflected_eq
    PostReflectedShock_fr
    PostReflectedShock_eq
    FHFP_reflected_fr

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
from sdtoolbox.thermo import eq_state,state

def reflected_fr(gas1,gas2,gas3,UI):
    """
    Calculates frozen post-reflected-shock state assumming u1 = 0.

    FUNCTION SYNTAX:
        [p3,UR,gas3] = reflected_fr(gas1,gas2,gas3,UI)

    INPUT:
        gas1 = gas object at initial state
        gas2 = gas object at post-incident-shock state (already computed)
        gas3 = working gas object
        UI = incident shock speed (m/s)

    OUTPUT:
        p3 = post-reflected-shock pressure (Pa)
        UR = reflected shock speed (m/s)
        gas3 = gas object at frozen post-reflected-shock state

    """
    # Lower bound on volume/density ratio (globally defined)
    from sdtoolbox.config import volumeBoundRatio
        
    p2 = gas2.P
    p1 = gas1.P
    rho2 = gas2.density
    v2=1/rho2
    rho1 = gas1.density
    v1=1/rho1
    T2 = gas2.T

    u2 = np.sqrt((p2-p1)*(v1-v2)) # particle velocity

    # BASIC PRELIMINARY GUESS
    v3 = v2/volumeBoundRatio
    p3 = p2 + rho2*(UI**2)*(1-v3/v2)
    T3 = T2*p3*v3/(p2*v2)

    gas3.TPX = T3, p3, gas2.X
    gas3 = PostReflectedShock_fr(u2,gas2,gas3)
    p3 = gas3.P
    UR = (p3-p2)/u2/rho2-u2
    
    return [p3,UR,gas3]


def reflected_eq(gas1,gas2,gas3,UI):
    """
    Calculates equilibrium post-reflected-shock state assumming u1 = 0.

    FUNCTION SYNTAX:
        [p3,UR,gas3] = reflected_eq(gas1,gas2,gas3,UI)

    INPUT:
        gas1 = gas object at initial state
        gas2 = gas object at post-incident-shock state (already computed)
        gas3 = working gas object
        UI = incident shock speed (m/s)

    OUTPUT:
        p3 = post-reflected-shock pressure (Pa)
        UR = reflected shock speed (m/s)
        gas3 = gas object at equilibrium post-reflected-shock state

    """
    # Lower bound on volume/density ratio (globally defined)
    from sdtoolbox.config import volumeBoundRatio
    
    p2 = gas2.P
    p1 = gas1.P
    rho2 = gas2.density
    v2=1/rho2
    rho1 = gas1.density
    v1=1/rho1
    T2 = gas2.T


    u2 = np.sqrt((p2-p1)*(v1-v2)) # particle velocity

    # BASIC PRELIMINARY GUESS
    v3 = v2/volumeBoundRatio
    p3 = p2 + rho2*(UI**2)*(1-v3/v2)
    T3 = T2*p3*v3/(p2*v2)

    gas3.TPX = T3, p3, gas2.X
    gas3 = PostReflectedShock_eq(u2, gas2,gas3);
    p3 = gas3.P
    UR = (p3-p2)/u2/rho2-u2;    
    
    return [p3,UR,gas3]


def PostReflectedShock_fr(u2,gas2,gas3):
    """
    Calculates frozen post-reflected-shock state for a specified shock velocity.

    FUNCTION SYNTAX:
        gas3 = PostReflectedShock_fr(u2,gas2,gas3)

    INPUT:
        u2 = current post-incident-shock lab frame particle speed
        gas2 = gas object at post-incident-shock state (already computed)
        gas3 = working gas object

    OUTPUT:
        gas3 = gas object at frozen post-reflected-shock state

    """
    # INITIALIZE ERROR VALUES (globally defined)
    from sdtoolbox.config import ERRFT,ERRFV

    # CALCULATE POST-REFLECTED SHOCK STATE
    r2 = gas2.density
    V2 = 1/r2
    
    j = 0
    deltaT = 1000
    deltaV = 1000

    ##################################################################################################
    # PRELIMINARY GUESS
    P = gas3.P
    H = gas3.enthalpy_mass
    T = gas3.T
    r = gas3.density
    V = 1/r
    ##################################################################################################
    
    # START LOOP
    while ((abs(deltaT) > ERRFT*T) or (abs(deltaV) > ERRFV*V)):
        j = j + 1
        
        if j == 500:
            print ('Calculation did not converge for U = %.2f' % (u2))
            return
                
        
        ##############################################################################################
        # CALCULATE FH & FP FOR GUESS 1
        [FH,FP] = FHFP_reflected_fr(u2,gas3,gas2)
        ##############################################################################################
        # TEMPERATURE PERTURBATION
        DT = T*0.02
        Tper = T + DT
        Vper = V
        Rper = 1/Vper
        [Pper, Hper] = state(gas3,Rper,Tper)
        
        # CALCULATE FHX & FPX FOR "IO" STATE
        [FHX,FPX] = FHFP_reflected_fr(u2,gas3,gas2)

        # ELEMENTS OF JACOBIAN
        DFHDT = (FHX-FH)/DT
        DFPDT = (FPX-FP)/DT
        ##############################################################################################
        # VOLUME PERTURBATION
        DV = 0.02*V
        Vper = V + DV
        Tper = T
        Rper = 1/Vper
        
        [Pper, Hper] = state(gas3,Rper,Tper)
        
        # CALCULATE FHX & FPX FOR "IO" STATE
        [FHX,FPX] = FHFP_reflected_fr(u2,gas3,gas2)
       
        # ELEMENTS OF JACOBIAN
        DFHDV = (FHX-FH)/DV
        DFPDV = (FPX-FP)/DV
        ##############################################################################################
        #INVERT MATRIX
        J = DFHDT*DFPDV - DFPDT*DFHDV
        b = [DFPDV, -DFHDV, -DFPDT, DFHDT]
        a = [-FH, -FP]
        deltaT = (b[0]*a[0]+b[1]*a[1])/J
        deltaV = (b[2]*a[0]+b[3]*a[1])/J

        
        ############################
        # CHECK & LIMIT CHANGE VALUES
        ############################
        # TEMPERATURE
        # VOLUME
        DTM = 0.2*T
        if (abs(deltaT) > DTM):
            deltaT = DTM*deltaT/abs(deltaT)
         
        ############################
        # VOLUME
        V3X = V + deltaV;
        if V3X > V2:
            DVM = 0.5*(V2 - V)
        else:
            DVM = 0.2*V
         
        if abs(deltaV) > DVM:
            deltaV = DVM*deltaV/abs(deltaV)
         
        ##############################################################################################
        # MAKE THE CHANGES
        T = T + deltaT
        V = V + deltaV
        r = 1/V
        [P, H] = state(gas3,r,T)
    
    return gas3


def PostReflectedShock_eq(u2,gas2,gas3):
    """
    Calculates equilibrium post-reflected-shock state for a specified shock velocity.

    FUNCTION SYNTAX:
        gas3 = PostReflectedShock_fr(u2,gas2,gas3)

    INPUT:
        u2 = current post-incident-shock lab frame particle speed
        gas2 = gas object at post-incident-shock state (already computed)
        gas3 = working gas object

    OUTPUT:
        gas3 = gas object at equilibrium post-reflected-shock state

    """
    # INITIALIZE ERROR VALUES (globally defined)
    from sdtoolbox.config import ERRFT,ERRFV

    # CALCULATE POST-REFLECTED SHOCK STATE
    r2 = gas2.density
    V2 = 1/r2

    j = 0
    deltaT = 1000
    deltaV = 1000

    ##################################################################################################
    # PRELIMINARY GUESS
    P = gas3.P
    H = gas3.enthalpy_mass
    T = gas3.T
    r = gas3.density
    V = 1/r
    [P, H] = eq_state(gas3,r,T)
    ##################################################################################################
    # START LOOP

    while (abs(deltaT) > ERRFT*T or abs(deltaV) > ERRFV*V):
        j = j + 1
        if j == 500:
            print ('Calculation did not converge for U = %.2f' % (u2))
            return gas3
                
        ##############################################################################################
        # CALCULATE FH & FP FOR GUESS 1
        [FH,FP] = FHFP_reflected_fr(u2,gas3,gas2);
        ##############################################################################################
        # TEMPERATURE PERTURBATION
        DT = T*0.02
        Tper = T + DT
        Vper = V
        Rper = 1/Vper
        [Pper, Hper] = eq_state(gas3,Rper,Tper)
        # CALCULATE FHX & FPX FOR "IO" STATE
        [FHX,FPX] = FHFP_reflected_fr(u2,gas3,gas2)

        # ELEMENTS OF JACOBIAN
        DFHDT = (FHX-FH)/DT
        DFPDT = (FPX-FP)/DT
        ##############################################################################################

        # VOLUME PERTURBATION
        DV = 0.02*V
        Vper = V + DV
        Tper = T
        Rper = 1/Vper
        [Pper, Hper] = eq_state(gas3,Rper,Tper)
        # CALCULATE FHX & FPX FOR "IO" STATE
        [FHX,FPX] = FHFP_reflected_fr(u2,gas3,gas2)
        # ELEMENTS OF JACOBIAN
        DFHDV = (FHX-FH)/DV
        DFPDV = (FPX-FP)/DV
        ##############################################################################################

        # INVERT MATRIX
        J = DFHDT*DFPDV - DFPDT*DFHDV
        b = [DFPDV, -DFHDV, -DFPDT, DFHDT]
        a = [-FH, -FP]
        deltaT = (b[0]*a[0]+b[1]*a[1])/J; deltaV = (b[2]*a[0]+b[3]*a[1])/J

        
        ############################
        # CHECK & LIMIT CHANGE VALUES
        ############################
        # TEMPERATURE
        # VOLUME
        DTM = 0.2*T
        if (abs(deltaT) > DTM):
            deltaT = DTM*deltaT/abs(deltaT)
         
        ############################
        # VOLUME
        V3X = V + deltaV;
        if V3X > V2:
            DVM = 0.5*(V2 - V)
        else:
            DVM = 0.2*V
         
        if abs(deltaV) > DVM:
            deltaV = DVM*deltaV/abs(deltaV)
         
        ##############################################################################################
        # MAKE THE CHANGES
        T = T + deltaT
        V = V + deltaV
        r = 1/V
        [P, H] = eq_state(gas3,r,T)

    return gas3


def FHFP_reflected_fr(u2,gas3,gas2):
    """
    Uses the momentum and energy conservation equations to calculate error in 
    current pressure and enthalpy guesses. In this case, state 3 is frozen.

    FUNCTION SYNTAX:
        [FH,FP] = FHFP_reflected_fr(u2,gas3,gas2)

    INPUT:
        u2 = current post-incident-shock lab frame particle speed
        gas3 = working gas object
        gas2 = gas object at post-incident-shock state (already computed)

    OUTPUT:
        FH,FP = error in enthalpy and pressure

    """
    P2 = gas2.P
    H2 = gas2.enthalpy_mass
    r2 = gas2.density
    
    P3 = gas3.P
    H3 = gas3.enthalpy_mass
    r3 = gas3.density
    
    FH = H3 - H2 - 0.5*(u2**2)*((r3/r2)+1)/(r3/r2-1)
    FP = P3 - P2 - r3*(u2**2)/(r3/r2-1)

    return [FH,FP]

