"""
Shock and Detonation Toolbox Utility Program

thermo_check.py  -  Scans a Cantera .cti mechanism file to determine size
of jumps in thermodynamic properties and derivatives. Identifies species
with largest Cp/R jump, writes species names and temperature limits to
file for species jumps in Cp/R exceeding the tolerance. Provides
routines for findings all jumps and plotting thermodynamic properties of
individual species. Only works with NASA-7 polynomials with current
version of Cantera 2.3 and 2.4.  Writes output file BadSpecies.txt containing
names and temperature limit of species with jumps exceeding user specified criteria.

Note the NASA-7 polynomials are of the form:
Cp/R = a1 + a2 T + a3 T^2 + a4 T^3 + a5 T^4    
H/RT = a1 + a2 T /2 + a3 T^2 /3 + a4 T^3 /4 + a5 T^4 /5 + a6/T    
S/R  = a1 lnT + a2 T + a3 T^2 /2 + a4 T^3 /3 + a5 T^4 /4 + a7

################################################################################
 Theory, numerical methods and applications are described in the following report:
 
     Numerical Solution Methods for Shock and Detonation Jump Conditions, S.
     Browne, J. Ziegler, and J. E. Shepherd, GALCIT Report FM2006.006 - R3,
     California Institute of Technology Revised September, 2018.
 
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
import numpy as np
from numpy.polynomial import Polynomial as P
import matplotlib.pyplot as plt

def hFit(a,T):
    """
    a = [a1 ... a6]
    T is numpy array
    Returns h and dh/dT (where by h, H/RT is meant)
    """
    h = a[0] + a[1]*T/2 + a[2]*T**2/3 + a[3]*T**3/4 + a[4]*T**4/5 + a[5]/T
    dhdT = a[1]/2 + a[2]*2*T/3 + a[3]*3*T**2/4 + a[4]*4*T**3/5 - a[5]/T**2
    return h,dhdT

def sFit(a,T):
    """
    a = [a1 ... a7]
    T is numpy array
    Returns s and ds/dT (where by s, S/R is meant)
    """
    s = a[0]*np.log(T) + a[1]*T + a[2]*T**2/2 + a[3]*T**3/3 + a[4]*T**4/4 + a[6]
    dsdT = a[0]/T + a[1] + a[2]*T + a[3]*T**2 + a[4]*T**3
    return s,dsdT
    

def coeffImport(species):
    """
    Takes a Cantera species object
    """
    midT = species.thermo.coeffs[0]
    minT = species.thermo.min_temp
    maxT = species.thermo.max_temp
    highCoeffs = species.thermo.coeffs[1:8]
    lowCoeffs = species.thermo.coeffs[8:15]      

    return minT,midT,maxT,lowCoeffs,highCoeffs

def continuity(mech,absTol=1e-5):
    """
    Takes a string representing Cantera mechanism, e.g. 'gri30.cti'
    
    absTol is the absolute tolerance used when checking for C0 and C1
    continuity. Can vary the default value if desired.
    Absolute rather than relative tolerance used to avoid division by zero
    since some species (e.g. Ar) have zero slope for all T.
    """
    print('Mechanism file '+mech)
    gas = ct.Solution(mech)
    ns = gas.n_species
    ne= gas.n_elements
    nr = gas.n_reactions
    """
    Compute gas standard state properties for each species
    """
    T0 = 298.15
    P0 = 101325.
    gas.TP = T0, P0
    R = ct.gas_constant          # universal gas constant
    w = np.zeros(ns)
    hs = np.zeros(ns)
    cps = np.zeros(ns)
    ss = np.zeros(ns)
    deltahf = np.zeros(ns)
    for k in range(gas.n_species):
        w[k]=gas.molecular_weights[k]           # molecular weight of the ith species
        hs[k] = gas.standard_enthalpies_RT[k]   #specific enthalpy of the ith species
        deltahf[k] = 1E-6*hs[k]*R*gas.T         # heat of formation  (kJ/mol)
        cps[k] = gas.standard_cp_R[k]           # heat capacity of the ith species
        ss[k] = gas.standard_entropies_R[k]     # entropy of the ith species
    
    print('Number of elements = '+str(ne))
    print('Number of species = '+str(ns))
    print('Number of reactions = '+str(nr))
    S = ct.Species.listFromFile(mech) # list of species objects
    nCoeffs = int((S[0].thermo.n_coeffs-1)/2) # number of coefficients per polynomial, i.e. identifies NASA-7 vs. NASA-9s
    globalMinT = gas.min_temp
    globalMaxT = gas.max_temp
    print('NASA-'+str(nCoeffs)+' polynomials')
    print('Range of valid temperatures for all fits:')
    print('Max(Min. T) for all species = '+str(globalMinT)+' K')
    print('Min(Max. T) for all species = '+str(globalMaxT)+' K')  
    print("Absolute error tolerance for C0 and C1 {:.2e}".format(absTol))
    print('\n')
    f = open("BadSpecies.txt","w+")
    print('Species\tCp C0\tCp C1\th C0\th C1\ts C0\ts C1\tMin T\tMid T\tMax T\tW\tCp/R\tS/R\tH/RT\tDeltaHf')
    cpjumpmax = 0.
    ibad = 0
    for i,species in enumerate(S): 
        minT,midT,maxT,lowCoeffs,highCoeffs = coeffImport(species)    

        # Polynomial class takes coefficients in ascending powers of x
        lowPolyCp = P(lowCoeffs[0:5])
        highPolyCp = P(highCoeffs[0:5])      
        
        # Check for continuity
        cpjump = abs(lowPolyCp(midT)-highPolyCp(midT))
        if cpjump > cpjumpmax:
            cpjumpmax = cpjump
            spmax = species
        cpc0 =  cpjump < absTol
        if not cpc0:
            f.write('{0:18} {1:10.2f} {2:10.2f} {3:10.2f}  \n'.format(species.name,minT,midT,maxT))
            ibad = ibad + 1
            
        cpc1 = abs(lowPolyCp.deriv()(midT)-highPolyCp.deriv()(midT)) < absTol    
        
        hc0 = abs(hFit(lowCoeffs[0:6],midT)[0]-hFit(highCoeffs[0:6],midT)[0]) < absTol
        hc1 = abs(hFit(lowCoeffs[0:6],midT)[1]-hFit(highCoeffs[0:6],midT)[1]) < absTol
        
        sc0 = abs(sFit(lowCoeffs[0:7],midT)[0]-sFit(highCoeffs[0:7],midT)[0]) < absTol
        sc1 = abs(sFit(lowCoeffs[0:7],midT)[1]-sFit(highCoeffs[0:7],midT)[1]) < absTol 
       
        print(species.name+'\t'+str(cpc0)+'\t'+str(cpc1)+'\t'+str(hc0)+'\t'+str(hc1)+'\t'+str(sc0)+'\t'+ \
              str(sc1)+'\t'+str(minT)+'\t'+str(midT)+'\t'+str(maxT)+'\t{:.2f}'.format(w[i])+'\t{:.2f}'.format(cps[i])+ \
              '\t{:.2f}'.format(ss[i])+'\t{:7.2f}'.format(hs[i])+'\t{:7.2f}'.format(deltahf[i]))      

    print('\n'"Wrote {:d} species to BadSpecies.txt file".format(ibad))
    print('\n'+spmax.name+" has the maximum Cp/R jump of {:.2e}".format(cpjumpmax))
    jumps(spmax)
    plotter(spmax)
    plt.show()
    f.close()
    return
    
def plotter(species):
    """
    Takes a Cantera species object
    """
    minT,midT,maxT,lowCoeffs,highCoeffs = coeffImport(species)
    lowPolyCp = P(lowCoeffs[0:5])
    highPolyCp = P(highCoeffs[0:5])    
    lowTrange = np.linspace(minT,midT)
    highTrange = np.linspace(midT,maxT) 

    plt.figure()
    plt.plot(lowTrange,lowPolyCp(lowTrange),label='Low')
    plt.plot(highTrange,highPolyCp(highTrange),label='High')
    plt.legend()
    plt.title(r'$C_p$ fits for '+species.name)
    plt.xlabel('T [K]')
    plt.ylabel(r'$C_p$/R')

    plt.figure()
    plt.plot(lowTrange,hFit(lowCoeffs[0:6],lowTrange)[0],label='Low')
    plt.plot(highTrange,hFit(highCoeffs[0:6],highTrange)[0],label='High')
    plt.legend()
    plt.title(r'h fits for '+species.name)
    plt.xlabel('T [K]')
    plt.ylabel('h/RT')

    plt.figure()
    plt.plot(lowTrange,sFit(lowCoeffs[0:7],lowTrange)[0],label='Low')
    plt.plot(highTrange,sFit(highCoeffs[0:7],highTrange)[0],label='High')
    plt.legend()
    plt.title(r's fits for '+species.name)
    plt.xlabel('T [K]')
    plt.ylabel('s/R')
    return

def jumps(species):
    """
    Takes a cantera species object
    """
    
    minT,midT,maxT,lowCoeffs,highCoeffs = coeffImport(species)
    lowPolyCp = P(lowCoeffs[0:5])
    highPolyCp = P(highCoeffs[0:5])    
    lowTrange = np.linspace(minT,midT)
    highTrange = np.linspace(midT,maxT) 

    CpJump = lowPolyCp(midT)-highPolyCp(midT)
    CpTJump = lowPolyCp.deriv()(midT)-highPolyCp.deriv()(midT)
    HJump = hFit(lowCoeffs[0:6],midT)[0]-hFit(highCoeffs[0:6],midT)[0]
    HTJump = hFit(lowCoeffs[0:6],midT)[1]-hFit(highCoeffs[0:6],midT)[1]
    SJump  = sFit(lowCoeffs[0:7],midT)[0]-sFit(highCoeffs[0:7],midT)[0]
    STJump = sFit(lowCoeffs[0:7],midT)[1]-sFit(highCoeffs[0:7],midT)[1] 

    print('\n')
    print(species.name+" discontinuities at {:.2f} K".format(midT))
    print("Cp/R  {:.2e}".format(CpJump))
    print("Cp'/R {:.2e}".format(CpTJump))
    print("H/RT  {:.2e}".format(HJump))
    print("H/RT' {:.2e}".format(HTJump))
    print("S/R   {:.2e}".format(SJump))
    print("S'/R  {:.2e}".format(STJump))

    Tzero = 298.15
    R = ct.gas_constant
    Cp = R*lowPolyCp(Tzero)/1000.
    H = R*Tzero*hFit(lowCoeffs[0:6],Tzero)[0]/1.E06
    S = R*sFit(lowCoeffs[0:7],Tzero)[0]/1000.
    print('\n')
    print("Minimum temperature  {:.2f} K".format(minT))
    print("Midpoint temperature {:.2f} K".format(midT))
    print("Maximum temperature  {:.2f} K".format(maxT))
    print('\n'+species.name+" properties at {:.2f} K".format(Tzero))
    print("Cp     {:.3f} J/mol-K   {:.3f} cal/mol-K".format(Cp, Cp/4.184))
    print("H       {:.3f} kJ/mol    {:.3f} cal/K".format(H,H/4.184))
    print("S     {:.3f} J/mol-K  {:.3f} cal/mol-K".format(S,S/4.184))

    return

# Example of scanning an entire mechanism:
# mech = 'OHstar-partition.cti'
#mech = 'Hong2011.cti'
#mech = 'Burke2012.cti'
#mech = 'Davis2005.cti'
mech = 'Keromnes2013.cti'
#mech = 'Li2015.cti'
#mech = 'Davis2005.cti'
#mech = 'Mevel2018.cti'
#mech = 'Mevel2015.cti'
#mech = 'Mevel2017.cti'
#mech = 'hexaneReduced.cti'
#mech = 'hexaneFull.cti'
#mech = 'hexanePartial.cti'
#mech = 'Dagaut_Ori.cti'
#mech ='JetSurf2.cti'
#mech = 'chem.cti'
#mech = 'sandiego20161214.cti'
#mech = 'sandiego20161214_H2only.cti'
#mech='Blanquart2018.cti'
gas = ct.Solution(mech)
continuity(mech,1e-5)

#example of examining a single species
##print('Mechanism file '+mech)
##speciesName = 'CH*'
##species = gas.species(speciesName)
##jumps(species)
##plotter(species)
##plt.show()






