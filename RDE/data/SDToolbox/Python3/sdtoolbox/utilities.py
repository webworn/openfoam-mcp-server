"""
Shock and Detonation Toolbox
"utilities" module

Utility tools for creating pre-defined plots and output files from the outputs
of the CJspeed, cvsolve, cp_solve, and zndsolve functions.
 
This module defines the following functions:

    CJspeed_plot
    cv_plot
    znd_plot
    znd_fileout
    cp_plot
    
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
Updated February 12, 2021
Tested with: 
    Python 3.79 and Cantera 2.4
Under these operating systems:
    Windows 10, Linux (Ubuntu)
"""

import numpy as np
import matplotlib.pyplot as plt
from cycler import cycler

masterFontSize = 12
defaultColors = ['#1f77b4',
                 '#ff7f0e',
                 '#2ca02c',
                 '#d62728',
                 '#9467bd',
                 '#8c564b',
                 '#e377c2',
                 '#7f7f7f',
                 '#bcbd22',
                 '#17becf']
plt.rc('font',size=masterFontSize)
# Change linestyle once all colors cycled through
plt.rc('axes', prop_cycle=(cycler('linestyle', ['-', '--', '-.',':'])*
                           cycler('color',defaultColors[:4])))

def CJspeed_plot(plot_data,cj_speed):
    """
    Creates two plots of the CJspeed fitting routine: both display density ratio
    vs. speed. The first is very 'zoomed in' around the minimum, and shows the
    quadratic fit plotted through the calculated points. The second shows the 
    same fit on a wider scale, with the minimum and its corresponding speed
    indicated.
    
    FUNCTION SYNTAX:
        CJspeed_plot(plot_data,cj_speed)
        
    INPUT:
        plot_data = tuple (rr,w1,dnew,a,b,c) produced by sdtoolbox.postshock.CJspeed
                    rr = density ratio
                    w1 = speed
                    dnew = minimum density
                    a,b,c = quadratic fit coefficients
                    
        cj_speed = CJ speed from same calculation as plot_data
        
    OUTPUT:
        (none, but displays plots)
        
    """
    # Unpack tuple of plot data
    rr,w1,dnew,a,b,c = plot_data
    
    # Generate plots
    xmin = np.min(rr)
    xmax = np.max(rr)
    x = np.linspace(xmin,xmax)
    y = a*x*x+b*x+c
    fig, ax = plt.subplots()
    line1, = ax.plot(rr,w1,marker='s',linestyle='none')
    line2, = ax.plot(x,y)
    ax.ticklabel_format(style='plain',useOffset=False)
    ax.set_title('CJspeed fitting routine output, CJ speed ='+str(cj_speed))
    ax.set_xlabel('density ratio')
    ax.set_ylabel('speed (m/s)')
    
    x = np.linspace(1.5,2.0)
    y = a*x*x+b*x+c
    fig, ay = plt.subplots()
    line1, = ay.plot(x,y)
    line1, = ay.plot(dnew,cj_speed,marker='s',linestyle='none')
    ay.set_title('CJspeed fitting routine output, CJ speed ='+str(cj_speed))
    ay.set_xlabel('density ratio')
    ay.set_ylabel('speed (m/s)')
    plt.show()
    
def cv_plot(cv_output,xscale='linear',maxt=None,
            major_species=None,minor_species=None,show=True):
    """
    Creates two subplots from the solution to a CV explosion (in sdtoolbox.cv.cvsolve):
    Temperature vs. time, and pressure vs. time.
    Optionally, also creates plots of species mass fraction vs. time, for given lists
    of major or minor species. If major_species='All', all species will be plotted together.
    
    FUNCTION SYNTAX:
        cv_plot(cv_output)
        
    INPUT:
        cv_output: dictionary of outputs produced by sdtoolbox.cv.cvsolve.
        
    OPTIONAL INPUT:
        xscale: 'linear' or 'log' -- how to scale the x-axis
        
        maxt: Maximum time to plot to on the x-axis. By default, determined internally.
        
        show: Whether to display the figures. Defaults to 'True', but can set to 'False'
              if the user wants to return the figure handles to make further modifications
              before displaying.
        
        major_species,minor_species: lists of species names defined by the user as
                                     major or minor. Creates additional plots of mass
                                     fraction of each given species. Names need to match
                                     those in the Cantera model being used, and need to be
                                     capitalized (e.g. AR for argon). Unmatched names will
                                     be ignored for plotting, and a message displayed.
                                     major_species='AllSDT' gives all species.
                
    OUTPUT:
        List of figure handles: [temperature, pressure, {major species}, {minor species}]
        
    """
    ###########################################################
	 # PLOT TEMPERATURE PROFILE - MAX TIME = MAX TEMP + 10%
    ###########################################################    
    k = cv_output['T'].argmax()
    
    if maxt is None:
        if cv_output['time'][k] == 0:
            maxt = cv_output['ind_time']*5
        elif cv_output['time'][k] >= cv_output['ind_time']*50:
            maxt = cv_output['ind_time']*5
        else:
            maxt = cv_output['time'][k] + 0.1*cv_output['time'][k]
    
    if xscale == 'linear':
        mint = 0
    elif xscale == 'log':
        mint = 1e-9
    	

    # Temperature as a function of time
    figT = plt.figure()	
    plt.plot(cv_output['time'],cv_output['T'])
    plt.xlabel('Time (s)')
    plt.ylabel('Temperature (K)')
    plt.xscale(xscale)
    plt.xlim((mint,maxt))
    plt.title('CV Structure')
    if xscale == 'linear':
        plt.ticklabel_format(style='sci', axis='x', scilimits=(0,0))
    plt.tight_layout()
    	
    # Pressure as a function of time
    figP = plt.figure()	
    plt.plot(cv_output['time'],cv_output['P'])
    plt.xlabel('Time (s)')
    plt.ylabel('Pressure (Pa)')
    plt.xscale(xscale)
    plt.xlim((mint,maxt))
    plt.title('CV Structure')
    if xscale == 'linear':
        plt.ticklabel_format(style='sci', axis='both', scilimits=(0,0))    
    plt.tight_layout()
    
    figs = [figT,figP]
    
    if major_species is not None:
        if major_species == 'All':
            major_species = cv_output['gas'].species_names
        major_k = []; major_labels = [];
        for s in major_species:
            if s in cv_output['gas'].species_names:
                major_k.append(cv_output['gas'].species_index(s))
                major_labels.append(s)
            else:
                print(s+' is not a species in the current gas model.')
        if major_k:
            figMaj = plt.figure()
            plt.semilogy(cv_output['time'],cv_output['speciesY'].T[:,major_k])
            plt.xscale(xscale)
            plt.xlim((mint,maxt))
            plt.title('CV Structure')
            plt.xlabel('Time (s)')
            plt.ylabel('Species mass fraction')
            plt.legend(major_labels,loc='center left',bbox_to_anchor=(1, 0.5),
                       ncol=max(len(major_labels)//10,1))
            plt.tight_layout()
            figs.append(figMaj)
        
    if minor_species is not None:
        minor_k = []; minor_labels = [];
        for s in minor_species:
            if s in cv_output['gas'].species_names:
                minor_k.append(cv_output['gas'].species_index(s))
                minor_labels.append(s)
            else:
                print(s+' is not a species in the current gas model.')
        if minor_k:
            figMin = plt.figure()
            plt.semilogy(cv_output['time'],cv_output['speciesY'].T[:,minor_k])
            plt.xscale(xscale)
            plt.xlim((mint,maxt))
            plt.title('CV Structure')
            plt.xlabel('Time (s)')
            plt.ylabel('Species mass fraction')
            plt.legend(minor_labels,loc='center left',bbox_to_anchor=(1, 0.5),
                       ncol=max(len(minor_labels)//10,1))
            plt.tight_layout()
            figs.append(figMin)

    if show:
        plt.show()
    return figs
    
def znd_plot(znd_output,xscale='linear',maxx=None,
             major_species=None,minor_species=None,show=True):
    """
    Creates four plots from the solution to a ZND detonation (in sdtoolbox.znd.zndsolve):
    Temperature, pressure, Mach number, and thermicity vs. distance.
    Optionally, also creates plots of species mass fraction vs. time, for given lists
    of major or minor species. If major_species='All', all species will be plotted together.
    
    FUNCTION SYNTAX:
        znd_plot(znd_output)
        
    INPUT:
        znd_output: dictionary of outputs produced by sdtoolbox.znd.zndsolve.
        
    OPTIONAL INPUT:
        xscale: 'linear' or 'log' -- how to scale the x-axis
        
        maxt: Maximum distance to plot to on the x-axis. By default, determined internally.
        
        show: Whether to display the figures. Defaults to 'True', but can set to 'False'
              if the user wants to return the figure handles to make further modifications
              before displaying.

        major_species,minor_species: lists of species names defined by the user as
                                     major or minor. Creates additional plots of mass
                                     fraction of each given species. Names need to match
                                     those in the Cantera model being used, and need to be
                                     capitalized (e.g. AR for argon). Unmatched names will
                                     be ignored for plotting, and a message displayed.
                                     major_species='All' gives all species.
        
    OUTPUT:
        List of figure handles: [temperature, pressure, Mach, 
                                 thermicity, {major species}, {minor species}]
        
    """
    k = znd_output['T'].argmax()
    
    if maxx is None:
        if znd_output['time'][k] == 0 and 'ind_len_ZND' in znd_output:
            maxx = znd_output['ind_len_ZND']*5
        else:
            maxx = znd_output['distance'][k]
    
    if xscale == 'linear':
        minx = 0
    elif xscale == 'log':
        minx = 1e-6
    		
    # Temperature as a function of position
    figT = plt.figure()
    plt.plot(znd_output['distance'],znd_output['T'])
    plt.xlabel('Distance (m)')
    plt.ylabel('Temperature (K)')
    plt.xscale(xscale)
    plt.xlim((minx,maxx))
    plt.title('ZND structure')
    if xscale == 'linear':
        plt.ticklabel_format(style='sci', axis='x', scilimits=(0,0))
    plt.title('ZND structure')
    plt.tight_layout()
    	
    # Pressure as a function of position
    figP = plt.figure()
    plt.plot(znd_output['distance'],znd_output['P'])
    plt.xlabel('Distance (m)')
    plt.ylabel('Pressure (Pa)')
    plt.xscale(xscale)
    plt.xlim((minx,maxx))
    plt.title('ZND structure')
    if xscale == 'linear':
        plt.ticklabel_format(style='sci', axis='both', scilimits=(0,0))
    plt.tight_layout()
    
    # Mach number as a function of position    
    figM = plt.figure()
    plt.plot(znd_output['distance'],znd_output['M'])
    plt.xlabel('Distance (m)')
    plt.ylabel('Mach number')
    plt.xscale(xscale)
    plt.xlim((minx,maxx))
    plt.title('ZND structure')
    if xscale == 'linear':
        plt.ticklabel_format(style='sci', axis='x', scilimits=(0,0))    
    plt.tight_layout()   
    
    # Thermicity as a function of position    
    figS = plt.figure()
    plt.plot(znd_output['distance'],znd_output['thermicity'])
    plt.xlabel('Distance (m)')
    plt.ylabel('Thermicity (1/s)')
    plt.xscale(xscale)
    plt.xlim((minx,maxx))
    plt.title('ZND structure')
    if xscale == 'linear':
        plt.ticklabel_format(style='sci', axis='both', scilimits=(0,0))    
    plt.tight_layout()
    
    figs = [figT,figP,figM,figS]
    
    if major_species is not None:
        if major_species == 'All':
            major_species = znd_output['gas1'].species_names
        major_k = []; major_labels = [];
        for s in major_species:
            if s in znd_output['gas1'].species_names:
                major_k.append(znd_output['gas1'].species_index(s))
                major_labels.append(s)
            else:
                print(s+' is not a species in the current gas model.')
        if major_k:
            figMaj = plt.figure()
            plt.semilogy(znd_output['distance'],znd_output['species'].T[:,major_k])
            plt.xscale(xscale)
            plt.xlim((minx,maxx))
            plt.title('ZND structure')
            plt.xlabel('Distance (m)')
            plt.ylabel('Species mass fraction')
            plt.legend(major_labels,loc='center left',bbox_to_anchor=(1, 0.5),
                       ncol=max(len(major_labels)//10,1))
            if xscale == 'linear':
                plt.ticklabel_format(style='sci', axis='x', scilimits=(0,0))
                
            plt.tight_layout()
            figs.append(figMaj)
        
    if minor_species is not None:
        minor_k = []; minor_labels = [];
        for s in minor_species:
            if s in znd_output['gas1'].species_names:
                minor_k.append(znd_output['gas1'].species_index(s))
                minor_labels.append(s)
            else:
                print(s+' is not a species in the current gas model.')
        if minor_k:
            figMin = plt.figure()
            plt.semilogy(znd_output['distance'],znd_output['species'].T[:,minor_k])
            plt.xscale(xscale)
            plt.xlim((minx,maxx))
            plt.title('ZND structure')
            plt.xlabel('Distance (m)')
            plt.ylabel('Species mass fraction')
            plt.legend(minor_labels,loc='center left',bbox_to_anchor=(1, 0.5),
                       ncol=max(len(minor_labels)//10,1))
            if xscale == 'linear':
                plt.ticklabel_format(style='sci', axis='x', scilimits=(0,0))

            plt.tight_layout()
            figs.append(figMin)
   
    
    if show:
        plt.show()
    return figs
    
def znd_fileout(fname,znd_output):
    """
    Creates 2 formatted text files to store the output of the solution to a ZND
    detonation (from sdtoolbox.znd.zndsolve). Includes a timestamp of when the file was created,
    input conditions, and tab-separated columns of output data.
    
    FUNCTION SYNTAX:
        znd_fileout(fname,znd_output)
        
    INPUT:
        fname: filename (appended by detonation velocity and '_znd' or '_znd2') (str)
        znd_output: dictionary of outputs produced by sdtoolbox.znd.zndsolve.
        
    OUTPUT:
        (none, but generates files)
    """
    import datetime
    from cantera import one_atm

    P1 = znd_output['gas1'].P
    T1 = znd_output['gas1'].T
    r1 = znd_output['gas1'].density
    
    U1 = znd_output['U1']

    fid = open(fname+'_'+str(U1)+'_znd.txt','w')
    d = datetime.date.today().strftime("%B %d, %Y")

    P = P1/one_atm

    fid.write('# ZND: DETONATION STRUCTURE CALCULATION\n')
    fid.write('# CALCULATION RUN ON %s\n\n' % d)
	
    fid.write('# INITIAL CONDITIONS\n')
    fid.write('# TEMPERATURE (K) %4.1f\n' % T1)
    fid.write('# PRESSURE (atm) %2.1f\n' % P)
    fid.write('# DENSITY (kg/m^3) %1.4e\n' % r1)
	
    fid.write('# SHOCK SPEED (m/s) %5.2f\n\n' % U1)
	
    fid.write('# Induction Times\n')
    fid.write('# Time to Peak Thermicity =   %1.4e s\n' % znd_output['ind_time_ZND'])
    fid.write('# Distance to Peak Thermicity =   %1.4e m\n' % znd_output['ind_len_ZND'])

    fid.write('\n# Exothermic/Reaction Times\n')
    fid.write('# Exothermic Pulse Time =   %1.4e s\n' % znd_output['exo_time_ZND'])
    fid.write('# Exothermic Pulse Distance =   %1.4e m\n' % znd_output['exo_len_ZND'])
	
    fid.write('# REACTION ZONE STRUCTURE\n\n')
	
    fid.write('# THE OUTPUT DATA COLUMNS ARE:\n')
    fid.write('Variables = "Distance (m)", "Mach Number", "Time (s)", "Pressure (Pa)", "Temperature (K)", "Density (kg/m^3)", "Thermicity (1/s)"\n')
	
   
    for val in zip(znd_output['distance'],znd_output['M'],znd_output['time'],znd_output['P'],
                   znd_output['T'],znd_output['rho'],znd_output['thermicity']):
        fid.write('%14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e\n' % val)
	
    fid.close()
	
    fid = open(fname+'_'+str(U1)+'_znd2.txt','w')

    fid.write('# ZND: DETONATION STRUCTURE CALCULATION\n')
    fid.write('# CALCULATION RUN ON %s\n\n' % d)

    fid.write('# INITIAL CONDITIONS\n')
    fid.write('# TEMPERATURE (K) %4.1f\n' % T1)
    fid.write('# PRESSURE (atm) %2.1f\n' % P)
    fid.write('# DENSITY (kg/m^3) %1.4e\n' % r1)
	
    fid.write('# SHOCK SPEED (M/S) %5.2f\n\n' % U1)
    fid.write('# REACTION ZONE STRUCTURE\n\n')

    fid.write('# Induction Times\n')
    fid.write('# Time to Peak Thermicity =   %1.4e s\n' % znd_output['ind_time_ZND'])
    fid.write('# Distance to Peak Thermicity =   %1.4e m\n' % znd_output['ind_len_ZND'])

    fid.write('\n# Exothermic/Reaction Times\n')
    fid.write('# Time of Reaction based on Thermicity Pulse Width =   %1.4e s\n' % znd_output['exo_time_ZND'])
    fid.write('# Length of Reaction based on Thermicity Pulse Width =   %1.4e m\n' % znd_output['exo_len_ZND'])
	
    fid.write('# THE OUTPUT DATA COLUMNS ARE:\n')
    fid.write('Variables = "Distance (m)", "Velocity (m/s)", "Sound Speed (m/s)", "Gamma", "Weight (kg/mol)","c^2-U^2 (m/s)"\n')
	
    
    for val in zip(znd_output['distance'],znd_output['U'],znd_output['af'],znd_output['g'],
                   znd_output['wt'],znd_output['sonic']):
        fid.write('%14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e\n'  % val)
	
    fid.close()

def cp_plot(cp_output,xscale='linear',maxt=None,
            major_species=None,minor_species=None,show=True):
    """
    Creates two subplots from the solution to a CP explosion (in sdtoolbox.cp.cpsolve):
    Temperature vs. time, and density vs. time.
    Optionally, also creates plots of species mass fraction vs. time, for given lists
    of major or minor species. If major_species='All', all species will be plotted together.
    
    FUNCTION SYNTAX:
        cp_plot(cp_output)
        
    INPUT:
        cp_output: dictionary of outputs produced by sdtoolbox.cp.cpsolve.
        
    OPTIONAL INPUT:
        xscale: 'linear' or 'log' -- how to scale the x-axis
        
        maxt: Maximum time to plot to on the x-axis. By default, determined internally.
        
        show: Whether to display the figures. Defaults to 'True', but can set to 'False'
              if the user wants to return the figure handles to make further modifications
              before displaying.
        
        major_species,minor_species: lists of species names defined by the user as
                                     major or minor. Creates additional plots of mass
                                     fraction of each given species. Names need to match
                                     those in the Cantera model being used, and need to be
                                     capitalized (e.g. AR for argon). Unmatched names will
                                     be ignored for plotting, and a message displayed.
                                     major_species='AllSDT' gives all species.
                
    OUTPUT:
        List of figure handles: [temperature, pressure, {major species}, {minor species}]
        
    """
    ###########################################################
	 # PLOT TEMPERATURE PROFILE - MAX TIME = MAX TEMP + 10%
    ###########################################################    
    k = cp_output['T'].argmax()
    
    if maxt is None:
        if cp_output['time'][k] == 0:
            maxt = cp_output['ind_time']*5
        elif cp_output['time'][k] >= cp_output['ind_time']*50:
            maxt = cp_output['ind_time']*5
        else:
            maxt = cp_output['time'][k] + 0.1*cp_output['time'][k]
    
    if xscale == 'linear':
        mint = 0
    elif xscale == 'log':
        mint = 1e-9
    	

    # Temperature as a function of time
    figT = plt.figure()	
    plt.plot(cp_output['time'],cp_output['T'])
    plt.xlabel('Time (s)')
    plt.ylabel('Temperature (K)')
    plt.xscale(xscale)
    plt.xlim((mint,maxt))
    plt.title('CP structure')
    if xscale == 'linear':
        plt.ticklabel_format(style='sci', axis='x', scilimits=(0,0))
    plt.tight_layout()
    	
    # density as a function of time
    figD = plt.figure()	
    plt.plot(cp_output['time'],cp_output['D'])
    plt.xlabel('Time (s)')
    plt.ylabel('Density (kg/m3)')
    plt.xscale(xscale)
    plt.xlim((mint,maxt))
    plt.title('CP structure')
    if xscale == 'linear':
        plt.ticklabel_format(style='sci', axis='both', scilimits=(0,0))    
    plt.tight_layout()
    
    figs = [figT,figD]
    
    if major_species is not None:
        if major_species == 'All':
            major_species = cp_output['gas'].species_names
        major_k = []; major_labels = [];
        for s in major_species:
            if s in cp_output['gas'].species_names:
                major_k.append(cp_output['gas'].species_index(s))
                major_labels.append(s)
            else:
                print(s+' is not a species in the current gas model.')
        if major_k:
            figMaj = plt.figure()
            plt.semilogy(cp_output['time'],cp_output['speciesY'].T[:,major_k])
            plt.xscale(xscale)
            plt.xlim((mint,maxt))
            plt.xlabel('Time (s)')
            plt.ylabel('Species mass fraction')
            plt.legend(major_labels,loc='center left',bbox_to_anchor=(1, 0.5),
                       ncol=max(len(major_labels)//10,1))
            plt.tight_layout()
            figs.append(figMaj)
        
    if minor_species is not None:
        minor_k = []; minor_labels = [];
        for s in minor_species:
            if s in cp_output['gas'].species_names:
                minor_k.append(cp_output['gas'].species_index(s))
                minor_labels.append(s)
            else:
                print(s+' is not a species in the current gas model.')
        if minor_k:
            figMin = plt.figure()
            plt.semilogy(cp_output['time'],cp_output['speciesY'].T[:,minor_k])
            plt.xscale(xscale)
            plt.xlim((mint,maxt))
            plt.xlabel('Time (s)')
            plt.ylabel('Species mass fraction')
            plt.legend(minor_labels,loc='center left',bbox_to_anchor=(1, 0.5),
                       ncol=max(len(minor_labels)//10,1))
            plt.tight_layout()
            figs.append(figMin)

    if show:
        plt.show()
    return figs
    

