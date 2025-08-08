"""
Shock and Detonation Toolbox Demo Program

This program computes the ratio of specific heats and logarithmic
isentrope slope using several approaches and compares the results
graphically. See Appendix G of the report (below).

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
import matplotlib.pyplot as plt
from sdtoolbox.thermo import gruneisen_eq,gruneisen_fr,soundspeed_eq,soundspeed_fr
import numpy as np

# set composition 
#mech = 'Hong2011.cti'
#mech = 'gri30_highT.cti'
mech = 'Mevel2017.cti'
gas = ct.Solution(mech)
q = 'O2:1 H2:2.001'
T1 = 300
P1 = ct.one_atm
# find species index for plotting
iH = gas.species_index('H')
iO = gas.species_index('O')
iOH = gas.species_index('OH')
iH2O = gas.species_index('H2O')
iH2 = gas.species_index('H2')
iO2 = gas.species_index('O2')

# Use constant volume explosion as starting point for isentropic expansion
gas.TPX = T1,P1,q
gas.equilibrate('UV')
T2 = gas.T
S2 = gas.s
V2 = gas.v
V  = V2
X2 = gas.X

P = []; R = []; T = [] 
xH = []; xO = []; xOH = []; xH2 = []; xO2 = []; xH2O = []
grun_eq = []; grun_fr = []; a_eq = []; a_fr = []; kappa_fr = []; kappa_eq = [];
tvdvdt_eq = []; tvdvdt_fr = []; dpdr = []; gamma_eq = []; gamma_fr = []; 
diff = []; G_mult = []; gamma_fr_thermo = []; gamma_eq_thermo = []; P_f = []; T_f = []; ratio = [];

for i in range(20):
    # equilibrium state computations
    gas.SV = S2,V
    gas.equilibrate('SV')
    P.append(gas.P)
    R.append(gas.density)
    T.append(gas.T)
    # species
    X = gas.X
    xH.append(X[iH])
    xO.append(X[iO])
    xOH.append(X[iOH])
    xH2.append(X[iH2])
    xO2.append(X[iO2])
    xH2O.append(X[iH2O])
    # gamma computed from Gruneisen coefficient
    grun_eq.append(gruneisen_eq(gas))
    grun_fr.append(gruneisen_fr(gas))
    a_eq.append(soundspeed_eq(gas))
    a_fr.append(soundspeed_fr(gas)) 
    # logarithmic slope of isentropes in P-V coordinates
    kappa_fr.append(a_fr[-1]**2*R[-1]/P[-1])
    kappa_eq.append(a_eq[-1]**2*R[-1]/P[-1])
    # logarithmic slope of isentropes in T-V coordinates
    # equilibrium
    gas.SV = S2,1.01*V
    gas.equilibrate('SV')
    T_2 = gas.T
    gas.SV = S2,0.99*V
    gas.equilibrate('SV')
    T_1 = gas.T
    tvdvdt_eq.append(-V*(T_2-T_1)/(0.02*V)/T[-1])
    # frozen
    gas.SVX = S2,V*1.01,X2
    T_2 = gas.T
    gas.SVX = S2,V*0.99,X2
    T_1 = gas.T
    tvdvdt_fr.append(-1.*V*(T_2-T_1)/(0.02*V)/T[-1])
     # compute equilibrium values of isothermal dP/drho
    gas.TD = T[-1],1/(1.01*V)
    gas.equilibrate('TV')
    P2 = gas.P
    gas.TD = T[-1],1/(0.99*V)
    gas.equilibrate('TV')
    P1 = gas.P
    dpdr.append(-V*V*(P2-P1)/(0.02*V))
    # compute ratio of specific heats from thermodynamic identities
    gamma_eq.append(a_eq[-1]*a_eq[-1]/dpdr[-1])
    gamma_fr.append(a_fr[-1]*a_fr[-1]*R[-1]/P[-1])
    # compute the difference between equilbrium and frozen dpdrho_T
    diff.append(dpdr[-1]*R[-1]/P[-1])
    # compute equilibrium values of dvdt_P
    gas.TP = T[-1]*1.01,P[-1]
    gas.equilibrate('TP')
    V_2 = gas.v    
    gas.TP = T[-1]*0.99,P[-1]
    gas.equilibrate('TP')
    V_11 = gas.v    
    dvdt = (V_2-V_11)/(0.02*T[-1])
    G_mult.append(dvdt*T[-1]*R[-1])
    # compute frozen specific heats from thermodynamic properties
    gas.TD = T[-1],1/V
    gas.equilibrate('TV')
    gamma_fr_thermo.append(gas.cp_mass/gas.cv_mass)
    # compute equilibrium specific heats from properties and definition
    gas.TD = T[-1]*1.01,1/V
    gas.equilibrate('TV')
    U2 = gas.u
    gas.TD = T[-1]*0.99,1/V
    gas.equilibrate('TV')
    U1 = gas.u
    CVEQ = (U2-U1)/(.02*T[-1])
    gas.TP = T[-1]*1.01,P[-1]
    gas.equilibrate('TP')
    H2 = gas.h
    gas.TP = T[-1]*0.99,P[-1]
    gas.equilibrate('TP')
    H1 = gas.h
    CPEQ = (H2-H1)/(.02*T[-1])
    gamma_eq_thermo.append(CPEQ/CVEQ)
    ratio.append(gamma_eq[-1]/gamma_eq_thermo[-1])
    # frozen pressure and temperature 
    gas.SVX = S2,V,X2
    P_f.append(gas.P)
    T_f.append(gas.T)
    # next state
    V = 2.*V

# plotting
kappa_fr = np.asarray(kappa_fr); kappa_eq = np.asarray(kappa_eq)
gamma_fr = np.asarray(gamma_fr); gamma_eq = np.asarray(gamma_eq)
gamma_fr_thermo = np.asarray(gamma_fr_thermo); gamma_eq_thermo = np.asarray(gamma_eq_thermo)
R = np.asarray(R); P = np.asarray(P); P_f = np.asarray(P_f); T = np.asarray(T); T_f = np.asarray(T_f);
    
plt.figure(1)
plt.plot(T,grun_fr,':',linewidth=1)
plt.plot(T,kappa_fr-1,'-.',linewidth=1) 
plt.plot(T,gamma_fr-1,'--',linewidth=1)
plt.plot(T,gamma_fr_thermo-1,'-',linewidth=1); 
plt.title(r'Frozen $\gamma$ comparisons',fontsize=12)
plt.xlabel('Temperature (K)',fontsize=12)
plt.ylabel(r'$\gamma - 1$',fontsize=12);
plt.legend([r'$G_{fr}$',r'$\kappa_{fr}-1$',r'$\gamma_{fr}-1$',r'$(c_p/c_v)_{fr} - 1$'])
plt.xlim(min(T),max(T))

plt.figure(2)
plt.plot(T,grun_eq,':',linewidth=1)
plt.plot(T,kappa_eq-1,'-.',linewidth=1)  
plt.plot(T,gamma_eq-1,'--',linewidth=1)
plt.plot(T,gamma_eq_thermo-1,'-',linewidth=1) 
plt.title(r'Equilibrium $\gamma$ comparisons',fontsize=12)
plt.xlabel('Temperature (K)',fontsize=12)
plt.ylabel(r'$\gamma - 1$',fontsize=12)
plt.legend([r'$G_{eq}$',r'$\kappa_{fr}-1$',r'$\gamma_{fr}-1$',r'$(c_p/c_v)_{fr} - 1$'])
plt.xlim(min(T),max(T))

plt.figure(3)
plt.plot(T,gamma_fr-1,':',linewidth=1); 
plt.plot(T,gamma_eq_thermo-1,'-.',linewidth=1)
plt.plot(T,kappa_eq-1,'--',linewidth=1)
plt.title(r'Frozen vs Equilibirum $\gamma$ comparisons',fontsize=12)
plt.xlabel('Temperature (K)',fontsize=12)
plt.ylabel(r'$\gamma - 1$',fontsize=12)
plt.legend([r'$\gamma_{fr}-1$',r'$\gamma_{eq}-1$',r'$\kappa_{eq}-1$'])
plt.xlim(min(T),max(T))

plt.figure(4)
plt.plot(T,diff,'k-',linewidth=1) 
plt.title(r'Equilibrium for $(\rho/P)(dP/d\rho)_T$',fontsize=12)
plt.xlabel('Temperature (K)',fontsize=12)
plt.ylabel(r'$(\rho/P)(dP/d\rho)_T$',fontsize=12)
plt.xlim(min(T),max(T))

plt.figure(5)
plt.plot(T,G_mult,'k-',linewidth=1)
plt.title(r'Equilibrium value of $(T/v)dv/dT_p$',fontsize=12)
plt.xlabel('Temperature (K)',fontsize=12)
plt.ylabel(r'$(T/v)(dv/dT)_P$',fontsize=12)
plt.xlim(min(T),max(T))

plt.figure(6)
plt.plot(T,xH,linewidth=1)
plt.plot(T,xO,linewidth=1)
plt.plot(T,xOH,linewidth=1)
plt.plot(T,xH2,linewidth=1)
plt.plot(T,xO2,linewidth=1) 
plt.plot(T,xH2O,linewidth=1)
plt.yscale('log')
plt.ylim((1e-4,1))
plt.title('Species distribution',fontsize=12)
plt.xlabel('Temperature (K)',fontsize=12)
plt.ylabel('Mole fraction',fontsize=12)
plt.legend(['H','O','OH','H2','O2','H2O'],loc='upper left')
plt.xlim(min(T),max(T))

V_p = 1/R
plt.figure(7)
plt.plot(V_p,T_f,'k--',linewidth=1) 
plt.plot(V_p,T,'k-',linewidth=1)
plt.xscale('log')
plt.yscale('log')
plt.axis((1,1E5,100,4000))
plt.title('Frozen vs Equilibirum isentrope',fontsize=12)
plt.xlabel(r'Volume (m$^3$/kg)',fontsize=12)
plt.ylabel('Temperature (K)',fontsize=12)
plt.legend(['Frozen','Equilibrium'],loc='upper right')

plt.figure(8)
plt.plot(V_p,P_f/ct.one_atm,'k--',linewidth=1) 
plt.plot(V_p,P/ct.one_atm,'k-',linewidth=1)
plt.xscale('log')
plt.yscale('log')
plt.axis((1,1E5,1E-5,20))
plt.title('Frozen vs Equilibirum isentrope',fontsize=12)
plt.xlabel(r'Volume (m$^3$/kg)',fontsize=12)
plt.ylabel('Pressure (atm)',fontsize=12)
plt.legend(['Frozen','Equilibrium'],loc='upper right')


## fitting isentropes
# restrict fit to volumes less than 1000 times initial value
valid = list(np.transpose(np.argwhere(V_p < 1000.*V2))[0])
# first find log values
vlog = np.log(V_p[valid])
telog = np.log(T[valid])
tflog = np.log(T_f[valid])
pelog = np.log(P[valid]/ct.one_atm)
pflog = np.log(P_f[valid]/ct.one_atm)

# fit log with linear fit to get slopes.  For higher accuracy use a 2nd
# order or higher polynomial as there is enough departure from linearity
deg = 1 # polynomial degree 

f = np.poly1d(np.polyfit(vlog,telog,deg))
kfit_te = f[1] # get coefficient of x^1 i.e. slope
x = np.linspace(min(vlog),max(vlog))
plt.figure(9)
plt.plot(x,f(x))
plt.plot(vlog,telog,'o')
plt.title('Equilibrium T-V fit, slope = '+str(kfit_te))
plt.xlabel('log specific volume')
plt.ylabel('log specific temperature')

f = np.poly1d(np.polyfit(vlog,tflog,deg))
kfit_tf = f[1] # get coefficient of x^1 i.e. slope
plt.figure(10)
plt.plot(x,f(x))
plt.plot(vlog,tflog,'o')
plt.title('Frozen T-V fit, slope = '+str(kfit_tf))
plt.xlabel('log specific volume')
plt.ylabel('log specific temperature')

f = np.poly1d(np.polyfit(vlog,pelog,deg))
kfit_pe = f[1] # get coefficient of x^1 i.e. slope
plt.figure(11)
plt.plot(x,f(x))
plt.plot(vlog,pelog,'o')
plt.title('Equilibrium P-V fit, slope = '+str(kfit_pe))
plt.xlabel('log specific volume')
plt.ylabel('log specific temperature')

f = np.poly1d(np.polyfit(vlog,pflog,deg))
kfit_pf = f[1] # get coefficient of x^1 i.e. slope
plt.figure(12)
plt.plot(x,f(x))
plt.plot(vlog,pflog,'o')
plt.title('Frozen P-V fit, slope = '+str(kfit_pf))
plt.xlabel('log specific volume')
plt.ylabel('log specific temperature')

plt.show()
