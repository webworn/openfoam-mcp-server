"""
Shock and Detonation Toolbox
"cv" module

Calculates constant-volume explosions.
 
This module defines the following functions:

    cvsolve
    
and the following classes:
    
    CVSys

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
Updated February 2021
Tested with: 
    Python 3.79 and Cantera 2.4
Under these operating systems:
    Windows 10, Linux (Ubuntu)
"""
import cantera as ct
import numpy as np
from scipy.integrate import solve_ivp

class CVSys(object):
    def __init__ (self,gas):
        self.gas = gas
        
    def __call__(self,t,y):
        """
        Evaluates the system of ordinary differential equations for an adiabatic, 
        constant-volume, zero-dimensional reactor. 
        It assumes that the 'gas' object represents a reacting ideal gas mixture.
    
        INPUT:
            t = time
            y = solution array [temperature, species mass 1, 2, ...]
            gas = working gas object
        
        OUTPUT:
            An array containing time derivatives of:
                temperature and species mass fractions, 
            formatted in a way that the integrator in cvsolve can recognize.
            
        """
        # Set the state of the gas, based on the current solution vector.
        self.gas.TDY = y[0],self.gas.density,y[1:]
        
        # Energy/temperature equation (terms separated for clarity)  
        a = self.gas.standard_enthalpies_RT - np.ones(self.gas.n_species)
        b = self.gas.net_production_rates /(self.gas.density * self.gas.cv_mass)
        dTdt = -self.gas.T * ct.gas_constant * np.dot(a,b)
        
        # Species equations
        dYdt = self.gas.net_production_rates*self.gas.molecular_weights/self.gas.density
        
        return np.hstack((dTdt, dYdt))
    
   
def cvsolve(gas,
            t_end=1e-6,max_step=1e-5,t_eval=None,
            relTol=1e-5,absTol=1e-8,Method='LSODA'):
    """
    Solves the ODE system defined in CVSys, taking the gas object input as the
    initial state.
    
    Uses scipy.integrate.solve_ivp. The 'Radau' solver is used as this is a stiff system.
    From the scipy documentation:
        
        Implicit Runge-Kutta method of the Radau IIA family of order 5. 
        The error is controlled with a third-order accurate embedded formula. 
        A cubic polynomial which satisfies the collocation conditions 
        is used for the dense output.
        
    
    FUNCTION SYNTAX:
        output = cvsolve(gas,**kwargs)
    
    INPUT:
        gas = working gas object
        
    OPTIONAL INPUT:
        t_end = end time for integration, in sec
        max_step = maximum time step for integration, in sec
        t_eval = array of time values to evaluate the solution at.
                    If left as 'None', solver will select values.
                    Sometimes these may be too sparse for good-looking plots.
        relTol = relative tolerance
        absTol = absolute tolerances
        Method = method of integration, 'LSODA' is default.  
    
    OUTPUT:
        output = a dictionary containing the following results:
            time = time array
            T = temperature profile array
            P = pressure profile array
            speciesY = species mass fraction array
            speciesX = species mole fraction array
            
            gas = working gas object
            
            exo_time = pulse width (in secs) of temperature gradient (using 1/2 max)
            ind_time = time to maximum temperature gradient
            ind_time_10 = time to 10% of maximum temperature gradient
            ind_time_90 = time to 90% of maximum temperature gradient
            
    """
    r0 = gas.density
    y0 = np.hstack((gas.T,gas.Y))    

    tel = [0.,t_end] # Timespan

    output = {}
    
    out = solve_ivp(CVSys(gas),tel,y0,method=Method,
                    atol=absTol,rtol=relTol,max_step=max_step,t_eval=t_eval)

    
    output['time'] = out.t
    output['T'] = out.y[0,:]
    output['speciesY'] = out.y[1:,:]
    
    # Initialize additional output matrices where needed
    b = len(output['time'])
    output['P'] = np.zeros(b)
    output['dTdt'] = np.zeros(b)
    output['speciesX'] = np.zeros(output['speciesY'].shape)
    output['ind_time'] = 0
    output['ind_time_90'] = 0
    output['ind_time_10'] = 0
    output['exo_time'] = 0    
    temp_grad = np.zeros(b)
    
    #############################################################################
    # Extract PRESSSURE and TEMPERATURE GRADIENT
    #############################################################################
    
    # Have to loop for operations involving the working gas object
    for i,T in enumerate(output['T']):
        gas.TDY = T,r0,output['speciesY'][:,i]        
        wt = gas.mean_molecular_weight        
        s = 0
        for z in range(gas.n_species):
            w = gas.molecular_weights[z]
            e = ct.gas_constant*T*(gas.standard_enthalpies_RT[z]/w - 1/wt)
            s = s + e*w*gas.net_production_rates[z]
            
        temp_grad[i] = -s/(r0*gas.cv_mass)
        output['P'][i] = gas.P
        output['speciesX'][:,i] = gas.X
        output['dTdt'][i] = temp_grad[i]

    n = temp_grad.argmax()
    
    if n == b:
        print('Error: Maximum temperature gradient occurs at the end of the reaction zone')
        print('       Your final integration length may be too short,')
        print('       your mixture may be too rich/lean, or something else may be wrong')
        print(' ')
        output['ind_time'] = output['time'][b] 
        output['ind_time_10'] = output['time'][b]
        output['ind_time_90'] = output['time'][b] 
        output['exo_time'] = 0
        print('Induction Time: '+str(output['ind_time']))
        print('Exothermic Pulse Time: '+str(output['exo_time']))
        return output
    elif n == 0:
        print('Error: Maximum temperature gradient occurs at the beginning of the reaction zone')
        print('       Your final integration length may be too short,')
        print('       your mixture may be too rich/lean, or something else may be wrong')
        print(' ')
        output['ind_time'] = output['time'][0] 
        output['ind_time_10'] = output['time'][0]
        output['ind_time_90'] = output['time'][0] 
        output['exo_time'] = 0 
        print('Induction Time: '+str(output['ind_time']))
        print('Exothermic Pulse Time: '+str(output['exo_time']))
        return output
    else:
        output['ind_time'] = output['time'][n]
        
        k = 0
        MAX10 = 0.1*max(temp_grad)
        d = temp_grad[0]        
        while d < MAX10 and k < n:
            k = k + 1
            d = temp_grad[k]
        output['ind_time_10'] = output['time'][k]
        
        k = 0
        MAX90 = 0.9*max(temp_grad)
        d = temp_grad[0]
        while d < MAX90 and k < n:
            k = k + 1
            d = temp_grad[k]
        output['ind_time_90'] = output['time'][k]

        # find exothermic time
        tstep2 = 0
        half_T_flag1 = 0
        half_T_flag2 = 0
        # Go into a loop to find two times when temperature is half its maximum
        for j,tgrad in enumerate(list(temp_grad)):
            if half_T_flag1 == 0:
                if tgrad > 0.5*max(temp_grad):
                    half_T_flag1 = 1
                    tstep1 = j
                    
            elif half_T_flag2 == 0:
                if tgrad < 0.5*max(temp_grad):
                    half_T_flag2 = 1
                    tstep2 = j
                else:
                    tstep2 = 0


    # Exothermic time for CV explosion
    if tstep2 == 0:
        print('Error: No pulse in the temperature gradient')
        print('       Your final integration length may be too short,')
        print('       your mixture may be too rich/lean, or something else may be wrong') 
        output['exo_time'] = 0
    else:
        output['exo_time'] = output['time'][tstep2] - output['time'][tstep1]

    output['gas'] = gas 
    return output
