"""
Shock and Detonation Toolbox
"postshock" module

Calculates CJ detonation speed and post-shock states for frozen and equilibrium cases.
 
This module defines the following functions:

    LSQ_CJspeed
    hug_fr
    hug_eq
    FHFP
    CJ_calc
    CJspeed
    PostShock_fr
    PostShock_eq
    shock_calc
    shk_eq_calc

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

import cantera as ct
import numpy as np
from sdtoolbox.thermo import eq_state,state

def LSQ_CJspeed(x,y):
    """
    Determines least squares fit of parabola to input data

    FUNCTION SYNTAX:
    [a,b,c,R2,SSE,SST] = LSQ_CJspeed(x,y)

    INPUT:
        x = independent data points
        y = dependent data points

    OUTPUT:
        a,b,c = coefficients of quadratic function (ax^2 + bx + c = 0)
        R2 = R-squared value
        SSE = sum of squares due to error
        SST = total sum of squares

    """
    # Calculate Sums
    k = 0
    X = 0.0; X2 = 0.0; X3 = 0.0; X4 = 0.0;
    Y = 0.0; Y1 = 0.0; Y2 = 0.0;
    a = 0.0; b = 0.0; c = 0.0; R2 = 0.0
    n = len(x)

    while k < n:
        X = X + x[k]
        X2 = X2 + x[k]**2
        X3 = X3 + x[k]**3
        X4 = X4 + x[k]**4
        Y = Y + y[k]
        Y1 = Y1 + y[k]*x[k]
        Y2 = Y2 + y[k]*x[k]**2
        k= k + 1
    m = float(Y)/float(n)

    den = (X3*float(n) - X2*X)
    temp = (den*(X*X2-X3*float(n))+X2*X2*(X*X-float(n)*X2)-X4*float(n)*(X*X-X2*float(n)))
    temp2 = (den*(Y*X2-Y2*float(n)) + (Y1*float(n)-Y*X)*(X4*float(n)-X2*X2))

    b = temp2/temp
    a = 1.0/den*(float(n)*Y1 - Y*X - b*(X2*float(n)-X*X))
    c = 1/float(n)*(Y - a*X2 - b*X)

    k= 0; SSE = 0.0; SST = 0.0;

    f = np.zeros(len(x),float)
    
    while k < len(x):
        f[k] = a*x[k]**2 + b*x[k] + c
        SSE = SSE + (y[k] - f[k])**2
        SST = SST + (y[k] - m)**2
        k = k + 1
    R2 = 1 - SSE/SST

    return [a,b,c,R2,SSE,SST]


def hug_fr(x,vb,h1,P1,v1,gas):
    """
    Computes difference in enthalpy computed from assumed (T, V)
    state and fixed composition (frozen) hugoniot evaluation.  Used
    with root solver such as 'fsolve' to compute frozen hugoniot as a function
    of volume.  Input gas object is modified to correspond to input (T, V).

    FUNCTION SYNTAX:
        diff = hug_fr(x,vb,h1,P1,v1,gas)
    
    USAGE:
        fval = fsolve(hug_fr,Ta,args=(vb,h1,P1,v1,gas))
            = frozen Hugoniot temperature (K) corresponding to vb 

    INPUT:
        Ta = initial guess for frozen Hugoniot temperature (K)
        vb = desired frozen Hugoniot specific volume (m^3/kg)
        h1 = enthalpy at state 1 (J/kg)
        P1 = pressure at state 1 (Pa)
        v1 = specific volume at state 1 (m^3/kg)
        gas = working gas object

    OUTPUT:
        enthalpy difference

    """
    gas.TD = x, 1.0/vb
    hb1 = gas.enthalpy_mass
    Pb = ct.gas_constant*x/(gas.mean_molecular_weight*vb)
    
    hb2 = h1 + 0.5*(Pb-P1)*(vb+v1)
    return hb2-hb1
  
    
def hug_eq(x,vb,h1,P1,v1,gas):
    """ 
    Computes difference in enthalpy computed from assumed (T, V)
    state and equilibrium composition state hugoniot evaluation.  Used
    with root solver such as 'fsolve' to compute equilibrium hugoniot as a function
    of volume.  Input gas object is modified to correspond to input (T, V) and
    an equilibrium composition.

    FUNCTION SYNTAX:
        diff = hug_eq(x,vb,h1,P1,v1,gas)
    
    USAGE:
        fval = fsolve(hug_eq,Ta,args=(vb,h1,P1,v1,gas))
            = equilibrium Hugoniot temperature (K) corresponding to vb 

    INPUT:
        Ta = initial guess for equilibrium Hugoniot temperature (K)
        vb = desired equilibrium Hugoniot specific volume (m^3/kg)
        h1 = enthalpy at state 1 (J/kg)
        P1 = pressure at state 1 (Pa)
        v1 = specific volume at state 1 (m^3/kg)
        gas = working gas object

    OUTPUT:
        enthalpy difference
        
    """
    gas.TD = x, 1.0/vb
    gas.equilibrate('TV')
    hb1 = gas.enthalpy_mass
    Pb = ct.gas_constant*x/(gas.mean_molecular_weight*vb)
    hb2 = h1 + 0.5*(Pb-P1)*(vb+v1)
    return hb2-hb1


def FHFP(w1,gas2,gas1):
    """
    Uses the momentum and energy conservation equations to calculate
    error in pressure and enthalpy given shock speed, upstream (gas1)
    and downstream states (gas2).  States are not modified by these routines.

    FUNCTION SYNTAX:
        [FH,FP] = FHFP(w1,gas2,gas1)
    
    INPUT:
        w1 = shock speed (m/s)
        gas2 = gas object at working/downstream state
        gas1 = gas object at initial/upstream state

    OUTPUT:
        FH,FP = error in enthalpy and pressure

    """
    P1 = gas1.P
    H1 = gas1.enthalpy_mass
    r1 = gas1.density
    P2 = gas2.P
    H2 = gas2.enthalpy_mass
    r2 = gas2.density
    w1s = w1**2
    w2s = w1s*(r1/r2)**2
    FH = H2 + 0.5*w2s - (H1 + 0.5*w1s)
    FP = P2 + r2*w2s - (P1 + r1*w1s)
    return [FH, FP]


def CJ_calc(gas, gas1, ERRFT, ERRFV, x):
    """
    Calculates the Chapman-Jouguet wave speed using Reynolds' iterative method.

    FUNCTION SYNTAX:
        [gas,w1] = CJ_calc(gas,gas1,ERRFT,ERRFV,x)

    INPUT:
        gas = working gas object
        gas1 = gas object at initial state
        ERRFT,ERRFV = error tolerances for iteration
        x = density ratio

    OUTPUT:
        gas = gas object at equilibrium state
        w1 = initial velocity to yield prescribed density ratio

    """
    T = 2000; r1 = gas1.density
    V1 = 1/r1
    i = 0; DT = 1000; DW = 1000;
    #PRELIMINARY GUESS
    V = V1/x; r = 1/V; w1 = 2000
    [P, H] = eq_state(gas,r,T)
    # START LOOP
    while (abs(DT) > ERRFT*T or abs(DW) > ERRFV*w1):
        i = i + 1
        if i == 500:
            'i = 500'
            return;
        # CALCULATE FH & FP FOR GUESS 1
        [FH,FP] = FHFP(w1,gas,gas1)
        # TEMPERATURE PERTURBATION
        DT = T*0.02; Tper = T + DT
        Vper = V; Rper = 1/Vper
        Wper = w1
        [Pper, Hper] = eq_state(gas,Rper,Tper)
        # CALCULATE FHX & FPX FOR "IO" STATE
        [FHX,FPX] = FHFP(Wper,gas,gas1)
        #ELEMENTS OF JACOBIAN
        DFHDT = (FHX-FH)/DT; DFPDT = (FPX-FP)/DT
        # VELOCITY PERTURBATION
        DW = 0.02*w1; Wper = w1 + DW
        Tper = T; Rper = 1/V
        [Pper, Hper] = eq_state(gas,Rper,Tper)
        # CALCULATE FHX & FPX FOR "IO" STATE
        [FHX,FPX] = FHFP(Wper,gas,gas1)
        #ELEMENTS OF JACOBIAN
        DFHDW = (FHX-FH)/DW; DFPDW = (FPX-FP)/DW
        # INVERT MATRIX
        J = DFHDT*DFPDW - DFPDT*DFHDW
        b = [DFPDW, -DFHDW, -DFPDT, DFHDT]
        a = [-FH, -FP]
        DT = (b[0]*a[0]+b[1]*a[1])/J; DW = (b[2]*a[0]+b[3]*a[1])/J
        # CHECK & LIMIT CHANGE VALUES
        # VOLUME
        DTM = 0.2*T
        if abs(DT) > DTM:
            DT = DTM*DT/abs(DT);
        # MAKE THE CHANGES
        T = T + DT; w1 = w1 + DW
        [P, H] = eq_state(gas,r,T);
    return [gas, w1]


def CJspeed(P1, T1, q, mech, fullOutput=False):
    """
    Calculates CJ detonation velocity for a given pressure, temperature, and
    composition.

    FUNCTION SYNTAX:
        If only CJ speed required:
        cj_speed = CJspeed(P1,T1,q,mech)
        If full output required:
        [cj_speed,R2,plot_data] = CJspeed(P1,T1,q,mech,fullOutput=True)

    INPUT:
        P1 = initial pressure (Pa)
        T1 = initial temperature (K)
        q = reactant species mole fractions in one of Cantera's recognized formats
        mech = cti file containing mechanism data (e.g. 'gri30.cti')

    OPTIONAL INPUT:
        fullOutput = set True for R-squared value and pre-formatted plot data
                    (the latter for use with sdtoolbox.utilities.CJspeed_plot)

    OUTPUT
        cj_speed = CJ detonation speed (m/s)
        R2 = R-squared value of LSQ curve fit (optional)
        plot_data = tuple (rr,w1,dnew,a,b,c)
                    rr = density ratio
                    w1 = speed
                    dnew = minimum density
                    a,b,c = quadratic fit coefficients

    """
    #DECLARATIONS
    numsteps = 20; maxv = 2.0; minv = 1.5
    w1 = np.zeros(numsteps+1,float)
    rr = np.zeros(numsteps+1,float)
    gas1 = ct.Solution(mech)
    gas  = ct.Solution(mech)
    #INTIAL CONDITIONS
    gas.TPX  = T1, P1, q
    gas1.TPX = T1, P1, q
    #INITIALIZE ERROR VALUES & CHANGE VALUES
    ERRFT = 1.0*10**-4;  ERRFV = 1.0*10**-4
    i = 1
    T1 = gas1.T; P1 = gas1.P
    counter = 1; R2 = 0.0; cj_speed = 0.0
    a = 0.0; b = 0.0; c = 0.0; dnew = 0.0
    while (counter <= 4) or (R2 < 0.99999):
        step = (maxv-minv)/float(numsteps); i = 0; x = minv
        while x <= maxv:
            gas.TPX = T1, P1, q
            [gas, temp] = CJ_calc(gas, gas1, ERRFT, ERRFV, x)
            w1[i] = temp
            rr[i] = gas.density/gas1.density
            i = i + 1; x = x + step
        [a,b,c,R2,SSE,SST] = LSQ_CJspeed(rr,w1)
        dnew = -b/(2.0*a)
        minv = dnew - dnew*0.001
        maxv = dnew + dnew*0.001
        counter = counter + 1
        cj_speed = a*dnew**2 + b*dnew + c
    
    if fullOutput:
        # Optional output data for plotting (with sdtoolbox.utilities.CJspeed_plot)
        plot_data = (rr,w1,dnew,a,b,c)        
        return [cj_speed,R2,plot_data]
    else:
        return cj_speed


def PostShock_fr(U1, P1, T1, q, mech):
    """
    Calculates frozen post-shock state for a specified shock velocity and pre-shock state.

    FUNCTION SYNTAX:
        gas = PostShock_fr(U1,P1,T1,q,mech)
    
    INPUT:
        U1 = shock speed (m/s)
        P1 = initial pressure (Pa)
        T1 = initial temperature (K)
        q = reactant species mole fractions in one of Cantera's recognized formats
        mech = cti file containing mechanism data (e.g. 'gri30.cti')
    
    OUTPUT:
        gas = gas object at frozen post-shock state
    
    """
    # INITIALIZE ERROR VALUES
    from sdtoolbox.config import ERRFT,ERRFV
    
    gas1 = ct.Solution(mech)
    gas  = ct.Solution(mech)
    # INTIAL CONDITIONS
    gas.TPX = T1, P1, q
    gas1.TPX= T1, P1, q
    # CALCULATES POST-SHOCK STATE
    gas = shk_calc(U1, gas, gas1, ERRFT, ERRFV)
    return gas


def PostShock_eq(U1, P1, T1, q, mech):
    """
    Calculates equilibrium post-shock state for a specified shock velocity and pre-shock state.

    FUNCTION SYNTAX:
        gas = PostShock_eq(U1,P1,T1,q,mech)

    INPUT:
        U1 = shock speed (m/s)
        P1 = initial pressure (Pa)
        T1 = initial temperature (K)
        q = reactant species mole fractions in one of Cantera's recognized formats
        mech = cti file containing mechanism data (e.g. 'gri30.cti')

    OUTPUT:
        gas = gas object at equilibrium post-shock state

    """
    # INITIALIZE ERROR VALUES
    from sdtoolbox.config import ERRFT,ERRFV
    
    gas1 = ct.Solution(mech);
    gas  = ct.Solution(mech);
    # INTIAL CONDITIONS    
    # workaround to avoid unsized object error when only one species in a .cti file
    # (flagged to be fixed in future Cantera version)
    if len(q)>1:
        gas.TPX = T1, P1, q
        gas1.TPX= T1, P1, q
    else:
        gas.TP = T1, P1
        gas1.TP= T1, P1
    # CALCULATES POST-SHOCK STATE
    gas = shk_eq_calc(U1, gas, gas1, ERRFT, ERRFV)
    return gas


def shk_calc(U1, gas, gas1, ERRFT, ERRFV):
    """
    Calculates frozen post-shock state using Reynolds' iterative method.

    FUNCTION SYNTAX:
        gas = shk_calc(U1,gas,gas1,ERRFT,ERRFV)
    
    INPUT:
        U1 = shock speed (m/s)
        gas = working gas object
        gas1 = gas object at initial state
        ERRFT,ERRFV = error tolerances for iteration

    OUTPUT:
        gas = gas object at frozen post-shock state

    """
    # Lower bound on volume/density ratio (globally defined)
    from sdtoolbox.config import volumeBoundRatio
    
    r1 = gas1.density; V1 = 1/r1
    P1 = gas1.P; T1 = gas1.T
    i = 0
    deltaT = 1000; deltaV = 1000
    # PRELIMINARY GUESS
    Vg = V1/volumeBoundRatio; rg = 1/Vg
    Pg = P1 + r1*(U1**2)*(1-Vg/V1); Tg = T1*Pg*Vg/(P1*V1)
    [Pg, Hg] = state(gas,rg,Tg)
    # SAVE STATE
    V = Vg; r = rg; P = Pg
    T = Tg; H = Hg
    # START LOOP
    while(abs(deltaT) > ERRFT*T or abs(deltaV) > ERRFV*V):
        i = i + 1
        if i == 500:
            print('shk_calc did not converge for U = ', U1)
            return gas
        # CALCULATE FH & FP FOR GUESS 1
        [FH,FP] = FHFP(U1,gas,gas1)

        # TEMPERATURE PERTURBATION
        DT = T*0.02; Tper = T + DT;
        Vper = V; Rper = 1/Vper;
        [Pper, Hper] = state(gas,Rper,Tper)
        # CALCULATE FHX & FPX FOR "IO" STATE
        [FHX,FPX] = FHFP(U1,gas,gas1)
        # ELEMENTS OF JACOBIAN
        DFHDT = (FHX-FH)/DT; DFPDT = (FPX-FP)/DT

        # VOLUME PERTURBATION
        DV = 0.02*V; Vper = V + DV
        Tper = T; Rper = 1/Vper
        [Pper, Hper] = state(gas,Rper,Tper)
        # CALCULATE FHX & FPX FOR "IO" STATE
        [FHX,FPX] = FHFP(U1,gas,gas1)
        # ELEMENTS OF JACOBIAN
        DFHDV = (FHX-FH)/DV; DFPDV = (FPX-FP)/DV

        # INVERT MATRIX
        J = DFHDT*DFPDV - DFPDT*DFHDV
        b = [DFPDV, -DFHDV, -DFPDT, DFHDT]
        a = [-FH, -FP]
        deltaT = (b[0]*a[0]+b[1]*a[1])/J; deltaV = (b[2]*a[0]+b[3]*a[1])/J
    
        # CHECK & LIMIT CHANGE VALUES
        # TEMPERATURE
        DTM = 0.2*T
        if abs(deltaT) > DTM:
            deltaT = DTM*deltaT/abs(deltaT)
        # VOLUME
        V2X = V + deltaV
        if V2X > V1:
            DVM = 0.5*(V1 - V)
        else:
            DVM = 0.2*V
        if abs(deltaV) > DVM:
            deltaV = DVM*deltaV/abs(deltaV)
        # MAKE THE CHANGES
        T = T + deltaT; V = V + deltaV; r = 1/V
        [P, H] = state(gas,r,T)

    return gas


def shk_eq_calc(U1, gas, gas1, ERRFT, ERRFV):
    """
    Calculates equilibrium post-shock state using Reynolds' iterative method.

    FUNCTION SYNTAX:
        gas = shk_calc(U1,gas,gas1,ERRFT,ERRFV)
    
    INPUT:
        U1 = shock speed (m/s)
        gas = working gas object
        gas1 = gas object at initial state
        ERRFT,ERRFV = error tolerances for iteration

    OUTPUT:
        gas = gas object at equilibrium post-shock state

    """    
    # Lower bound on volume/density ratio (globally defined)
    from sdtoolbox.config import volumeBoundRatio
    
    r1 = gas1.density; V1 = 1/r1
    P1 = gas1.P; T1 = gas1.T
    i = 0
    deltaT = 1000; deltaV = 1000
    # PRELIMINARY GUESS
    V = V1/volumeBoundRatio; r = 1/V
    P = P1 + r1*(U1**2)*(1-V/V1); T = T1*P*V/(P1*V1)
    [P, H] = eq_state(gas,r,T)
    # START LOOP
    while(abs(deltaT) > ERRFT*T or abs(deltaV) > ERRFV*V):
        i = i + 1
        if i == 500:
            print('shk_calc did not converge for U = ', U1)
            return gas
        # CALCULATE FH & FP FOR GUESS 1
        [FH,FP] = FHFP(U1,gas,gas1)

        # TEMPERATURE PERTURBATION
        DT = T*0.02; Tper = T + DT
        Vper = V; Rper = 1/Vper
        [Pper, Hper] = eq_state(gas,Rper,Tper)
        # CALCULATE FHX & FPX FOR "IO" STATE
        [FHX,FPX] = FHFP(U1,gas,gas1)
        # ELEMENTS OF JACOBIAN
        DFHDT = (FHX-FH)/DT; DFPDT = (FPX-FP)/DT

        # VOLUME PERTURBATION
        DV = 0.02*V; Vper = V + DV
        Tper = T; Rper = 1/Vper
        [Pper, Hper] = eq_state(gas,Rper,Tper)
        # CALCULATE FHX & FPX FOR "IO" STATE
        [FHX,FPX] = FHFP(U1,gas,gas1)
        # ELEMENTS OF JACOBIAN
        DFHDV = (FHX-FH)/DV; DFPDV = (FPX-FP)/DV

        # INVERT MATRIX
        J = DFHDT*DFPDV - DFPDT*DFHDV
        b = [DFPDV, -DFHDV, -DFPDT, DFHDT]
        a = [-FH, -FP]
        deltaT = (b[0]*a[0]+b[1]*a[1])/J; deltaV = (b[2]*a[0]+b[3]*a[1])/J
    
        # CHECK & LIMIT CHANGE VALUES
        # TEMPERATURE
        DTM = 0.2*T
        if abs(deltaT) > DTM:
            deltaT = DTM*deltaT/abs(deltaT)
        # VOLUME
        V2X = V + deltaV
        if V2X > V1:
            DVM = 0.5*(V1 - V)
        else:
            DVM = 0.2*V
        if abs(deltaV) > DVM:
            deltaV = DVM*deltaV/abs(deltaV)
        # MAKE THE CHANGES
        T = T + deltaT; V = V + deltaV; r = 1/V
        [P, H] = eq_state(gas,r,T)

    return gas
