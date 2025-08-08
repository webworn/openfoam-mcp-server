"""
Shock and Detonation Toolbox
"stagnation" module

Shock wave reaction zone structure for stagnation point flow

This module defines the following functions:

    stgsolve
    
and the following classes:
    
    StgSys
    
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
from sdtoolbox.thermo import soundspeed_fr
from sdtoolbox.znd import getThermicity
import numpy as np
from scipy.integrate import solve_ivp

class StgSys(object):
    def __init__ (self,gas,U1,r1,Delta):
        self.gas = gas
        self.U1 = U1
        self.r1 = r1
        self.Delta = Delta

    def __call__(self,t,y):
        self.gas.DPY = y[1],y[0],y[4:]
        rho = y[1]
        wdot = self.gas.net_production_rates
        mw = self.gas.molecular_weights

        c = soundspeed_fr(self.gas)
        
        U = y[2]                                    # velocity has to be updated
        M = U/c                                     # Mach Number
        eta = 1-M**2                                # Sonic Parameter
        walpha = self.U1*self.r1/self.Delta/rho     # area change function
         
        sigmadot = getThermicity(self.gas)
        
        Pdot = -rho*U**2*(sigmadot-walpha)/eta      # Pressure Derivative
        rdot = -rho*(sigmadot-walpha*M**2)/eta      # Density Derivative
        Udot = U*(sigmadot-walpha)/eta              # Velocity Derivative
        
        dYdt = mw*wdot/rho # mass production rates
        
        return np.hstack((Pdot,rdot,Udot,U,dYdt))


def stgsolve(gas,gas1,U1,Delta,
             t_end=1e-3,max_step=1e-4,t_eval=None,
             relTol=1e-5,absTol=1e-8):
    """
    Reaction zone structure computation for blunt body flow using
    Hornung's approximation of linear gradient in rho u 
    
    FUNCTION SYNTAX:
    output = stgsolve(gas,gas1,U1,Delta,**kwargs)
    
    INPUT
        gas = Cantera gas object - postshock state
        gas1 = Cantera gas object - initial state
        U1 = shock velocity (m/s)
        Delta = shock standoff distance (m)
        
    OPTIONAL INPUT:
        t_end = end time for integration, in sec
        max_step = maximum time step for integration, in sec
        t_eval = array of time values to evaluate the solution at.
                    If left as 'None', solver will select values.
                    Sometimes these may be too sparse for good-looking plots.
        relTol = relative tolerance
        absTol = absolute tolerance
    
    
    OUTPUT:
        output = a dictionary containing the following results:
            time = time array
            distance = distance array
            
            T = temperature array
            P = pressure array
            rho = density array
            U = velocity array
            thermicity = thermicity array
            distance = distance array
            species = species mass fraction array
            
            M = Mach number array
            af = frozen sound speed array
            g = gamma (cp/cv) array
            wt = mean molecular weight array
            sonic = sonic parameter (c^2-U^2) array
                        
            gas1 = a copy of the input initial state
            U1 = shock velocity
            Delta = shock standoff distance
    """

    r1 = gas1.density
    r = gas.density
    U = U1*r1/r

    x_start = 0
    y0 = np.hstack((gas.P,r,U,x_start,gas.Y)) # scaled pressure starts at 1, i.e. PSC/PSC

    tel = [0,t_end] # Timespan

    output = {}
    
    out = solve_ivp(StgSys(gas,U1,r1,Delta),tel,y0,method='Radau',
                    atol=absTol,rtol=relTol,max_step=max_step,t_eval=t_eval)

    output['time'] = out.t
    output['P'] = out.y[0,:]
    output['rho'] = out.y[1,:]
    output['U'] = out.y[2,:]
    output['distance'] = out.y[3,:]
    output['species'] = out.y[4:,:]
    
    # Initialize additional output matrices where needed
    b = len(output['time'])    
    output['T'] = np.zeros(b)
    output['thermicity'] = np.zeros(b)
    output['M'] = np.zeros(b)
    output['af'] = np.zeros(b)
    output['g'] = np.zeros(b)
    output['wt'] = np.zeros(b)
    output['sonic'] = np.zeros(b)
    output['Delta'] = Delta

    # Have to loop for operations involving the working gas object
    for i,P in enumerate(output['P']):
        gas.DPY = output['rho'][i],P,output['species'][:,i]
        output['T'][i] = gas.T
    
        #################################################################################################
        # Extract WEIGHT, GAMMA, SOUND SPEED, VELOCITY, MACH NUMBER, c^2-U^2,
        # THERMICITY, and TEMPERATURE GRADIENT 
        #################################################################################################

        af = soundspeed_fr(gas)     # frozen sound speed
        M = output['U'][i]/af       # Mach Number in shock-fixed frame
        eta = 1-M**2                # Sonic Parameter
        sonic = af**2*eta
     
        # Assign output structure
        output['thermicity'][i] = getThermicity(gas)
        output['M'][i] = M
        output['af'][i] = af
        output['g'][i] = gas.cp/gas.cv
        output['wt'][i] = gas.mean_molecular_weight
        output['sonic'][i] = sonic
    
    
    output['gas1'] = gas1
    output['U1'] = U1
    return output
