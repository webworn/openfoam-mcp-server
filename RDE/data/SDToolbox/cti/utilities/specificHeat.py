# coding: utf-8
"""
Compute specific heat coefficients to extend NASA9 data to low temperature.
Use statistical mechanics RRHO model to create low temperature data for
diatomic species from nuclear-rotational vibrational partition functions.
Approximate nuclear-rotational specific heat for polyatomics with high-temperature value
and assume RRHO with indpendent modes for polyatomics
JES 10/23/2021
"""

# Dependencies:
import scipy
from scipy.optimize import curve_fit, minimize, leastsq
import numpy as np
from sdtoolbox.thermo import soundspeed_eq,soundspeed_fr
import cantera as ct
print('Running Cantera version: ' + ct.__version__)
ct.basis = 'mass'
import matplotlib
matplotlib.rcParams['text.usetex'] = True
import matplotlib.pyplot as plt

def Cp_R(T,species_Name):
     """
     Low temperature heat capacities (approximate vibration) and rotation fully excited except for H2, O2, N2, OH)
     Ground electronic state only.  
     """
     if species_Name == 'O2':
          """ McQuarrie p. 95"""       
          Tv = 2256
          if (T/Tv > .01):
               Cp_R = 5/2 + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2                  
          else:
               Cp_R = 5/2
               
          #  nuclear-rotational partition for homonuclear integer spin nuclei with antisymmetric electronic ground state
          nubar = 1.437
          Tr = 1.43877*nubar
          I = 0  #16O nuclear spin
          J_max = 50

          q = 0
          for J in range(0,10*J_max+1):
               if J % 2:
                    q = q + (I+1)*(2*I+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)    # odd terms
               
          dqdT = 0
          for J in range(0,10*J_max+1):
               if J % 2:
                    dqdT = dqdT + (I+1)*(2*I+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)*J*(J+1)*Tr/T**2
  
          dqdT_2 = 0
          for J in range(0,10*J_max+1):
               if J % 2:
                    dqdT_2 = dqdT_2 + (I+1)*(2*I+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)*((J*(J+1)*Tr/T**2)**2 -2*J*(J+1)*Tr/T**3)

          Cp_rot = 2*T*dqdT/q + T**2*dqdT_2/q - (T*dqdT/q)**2
          Cp_R = Cp_R + Cp_rot
     elif species_Name == 'N2':
          """ McQuarrie p. 95 and p. 104 for rotational partition function"""       
          Tv = 3374
          if (T/Tv > .01):
               Cp_R = 5/2 + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          else:
               Cp_R = 5/2
               
          #  nuclear-rotational partition for homonuclear integer spin nuclei with symmetric electronic ground state
          nubar = 2.00
          Tr = 1.43877*nubar
          I = 1  #14N nuclear spin
          J_max = 10

          q = 0
          for J in range(0,10*J_max+1):
               if J % 2:
                    q = q + I*(2*I+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)    # odd terms
               else:
                    q = q + (I+1)*(2*I+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)    # even terms

          dqdT = 0
          for J in range(0,10*J_max+1):
               if J % 2:
                    dqdT = dqdT + I*(2*I+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)*J*(J+1)*Tr/T**2
               else:
                    dqdT = dqdT + (I+1)*(2*I+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)*J*(J+1)*Tr/T**2

          dqdT_2 = 0
          for J in range(0,10*J_max+1):
               if J % 2:
                    dqdT_2 = dqdT_2 + I*(2*I+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)*((J*(J+1)*Tr/T**2)**2 -2*J*(J+1)*Tr/T**3)
               else:
                    dqdT_2 = dqdT_2 + (I+1)*(2*I+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)*((J*(J+1)*Tr/T**2)**2 -2*J*(J+1)*Tr/T**3)

          Cp_rot = 2*T*dqdT/q + T**2*dqdT_2/q - (T*dqdT/q)**2
          Cp_R = Cp_R + Cp_rot
     elif species_Name == "NO":
          """ nubar Data from NIST webbook in cm-1 units"""       
          nubar = 1890.
          Tv = 1.43877*nubar
          if (T/Tv > .01):
               Cp_R = 5/2 + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          else:
               Cp_R = 5/2
               
          #  nuclear-rotational partition for heteronuclear diatomic with symmetric electronic ground state
          nubar = 1.695
          Tr = 1.43877*nubar
          I_1 = 1    #14N nuclear spin
          I_2 = 0    #16O nuclear spin
          J_max = 10

          q = 0
          for J in range(0,10*J_max+1):
                    q = q + (2*I_1+1)*(2*I_2+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)    # 
   
          dqdT = 0
          for J in range(0,10*J_max+1):
                    dqdT = dqdT + (2*I_1+1)*(2*I_2+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)*J*(J+1)*Tr/T**2
 
          dqdT_2 = 0
          for J in range(0,10*J_max+1):
                    dqdT_2 = dqdT_2 + (2*I_1+1)*(2*I_2+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)*((J*(J+1)*Tr/T**2)**2 -2*J*(J+1)*Tr/T**3)

          Cp_rot = 2*T*dqdT/q + T**2*dqdT_2/q - (T*dqdT/q)**2
          Cp_R = Cp_R + Cp_rot
     elif species_Name == "OH":
          """ nubar Data from NIST webbook in cm-1 units"""       
          nubar = 3737.761
          Tv = 1.43877*nubar
          if (T/Tv > .01):
               Cp_R = 5/2 + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          else:
               Cp_R = 5/2
               
          #  nuclear-rotational partition for heteronuclear diatomic with symmetric electronic ground state
          nubar = 18.9
          I_1 = 1/2  #1H nuclear spin
          I_2 = 0    #16O nuclear spin
          Tr = 1.43877*nubar
          J_max = 10

          q = 0
          for J in range(0,10*J_max+1):
                    q = q + (2*I_1+1)*(2*I_2+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)    
   
          dqdT = 0
          for J in range(0,10*J_max+1):
                    dqdT = dqdT + (2*I_1+1)*(2*I_2+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)*J*(J+1)*Tr/T**2
 
          dqdT_2 = 0
          for J in range(0,10*J_max+1):
                    dqdT_2 = dqdT_2 + (2*I_1+1)*(2*I_2+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)*((J*(J+1)*Tr/T**2)**2 -2*J*(J+1)*Tr/T**3)

          Cp_rot = 2*T*dqdT/q + T**2*dqdT_2/q - (T*dqdT/q)**2
          Cp_R = Cp_R + Cp_rot    
     elif species_Name == "CH":
          """ nubar Data from NIST webbook in cm-1 units"""       
          nubar = 2858.5
          Tv = 1.43877*nubar
          if (T/Tv > .01):
               Cp_R = 5/2 + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          else:
               Cp_R = 5/2
               
          #  nuclear-rotational partition for heteronuclear diatomic with symmetric electronic ground state
          nubar = 14.457
          I_1 = 1/2  #1H nuclear spin
          I_2 = 0    #12C nuclear spin
          Tr = 1.43877*nubar
          J_max = 10

          q = 0
          for J in range(0,10*J_max+1):
                    q = q + (2*I_1+1)*(2*I_2+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)    
   
          dqdT = 0
          for J in range(0,10*J_max+1):
                    dqdT = dqdT + (2*I_1+1)*(2*I_2+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)*J*(J+1)*Tr/T**2
 
          dqdT_2 = 0
          for J in range(0,10*J_max+1):
                    dqdT_2 = dqdT_2 + (2*I_1+1)*(2*I_2+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)*((J*(J+1)*Tr/T**2)**2 -2*J*(J+1)*Tr/T**3)

          Cp_rot = 2*T*dqdT/q + T**2*dqdT_2/q - (T*dqdT/q)**2
          Cp_R = Cp_R + Cp_rot    
     elif species_Name == "CO":
          """ McQuarrie p. 95 and p. 104 for rotational partition function"""       
          Tv = 3103
          if (T/Tv > .005):
               Cp_R = 5/2 + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2 
          else:
               Cp_R = 5/2
               
          # nuclear-rotational partition for homonuclear fractional spin nuclei with symmetric electronic ground state
          nubar = 1.925
          Tr = 1.43877*nubar
          I = 1/2
          J_max = 25
          I_1 = 0    #12C nuclear spin
          I_2 = 0    #16O nuclear spin

          q = 0
          for J in range(0,10*J_max+1):
                    q = q + (2*I_1+1)*(2*I_2+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)    # odd terms
  
          dqdT = 0
          for J in range(0,10*J_max+1):
                     dqdT = dqdT + (2*I_1+1)*(2*I_2+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)*J*(J+1)*Tr/T**2
 
          dqdT_2 = 0
          for J in range(0,10*J_max+1):
                    dqdT_2 = dqdT_2 + (2*I_1+1)*(2*I_2+1)*(2*J+1)*np.exp(-J*(J+1)*Tr/T)*((J*(J+1)*Tr/T**2)**2 -2*J*(J+1)*Tr/T**3)
 
          Cp_rot = 2*T*dqdT/q + T**2*dqdT_2/q - (T*dqdT/q)**2
          Cp_R = Cp_R + Cp_rot
     elif species_Name == "CO2":
          """ Data from p. 137 of McQuarrie, high temperature limit of rotational heat capacity"""
          Tv = 954 # bending 
          Cp_R = 5/2 + 2*(Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2   
          Tv = 3360 # asymmetric stretch
          Cp_R = Cp_R + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          Tv = 1890 # symmetric stretch
          Cp_R = Cp_R + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          Cp_rot = 1
          Cp_R = Cp_R + Cp_rot
     elif species_Name == "H2O":
          """ Data from p. 137 of McQuarrie"""
          Tv = 2290 # bending
          Cp_R = 5/2 + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2   
          Tv = 5160 # asymmetric stretch
          Cp_R = Cp_R + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          Tv = 5360 # symmetric stretch
          Cp_R = Cp_R + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          """high temperature limit of rotational heat capacity"""
          Cp_rot = 3/2
          Cp_R = Cp_R + Cp_rot
     elif species_Name == "HO2":
          """ nubar Data from NIST webbook in cm-1 units"""
          nubar = 3415.1  # OH stretch
          Tv = 1.43877*nubar
          Cp_R = 5/2 + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          nubar = 1397.8  # Bend
          Tv = 1.43877*nubar
          Cp_R = Cp_R + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          nubar = 1100.3  #OO stretch
          Tv = 1.43877*nubar
          Cp_R = Cp_R + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          """high temperature limit of rotational heat capacity"""
          Cp_rot = 3/2
          Cp_R = Cp_R + Cp_rot          
     elif species_Name == "H2O2":
          """ nubar Data from NIST webbook in cm-1 units"""
          nubar = 3617.1  # OH symmetric stretch
          Tv = 1.43877*nubar
          Cp_R = 5/2 + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          nubar = 1385  # OH symmetric bend
          Tv = 1.43877*nubar
          Cp_R = Cp_R + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          nubar = 865  # OO stretch
          Tv = 1.43877*nubar
          Cp_R = Cp_R + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          nubar = 371  # Torsion
          Tv = 1.43877*nubar
          Cp_R = Cp_R + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          nubar = 3610  # OH asymmetric stretch
          Tv = 1.43877*nubar
          Cp_R = Cp_R + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          nubar = 1270  # OH asymmetric bend
          Tv = 1.43877*nubar
          Cp_R = Cp_R + (Tv/T)*(Tv/T)*np.exp(Tv/T)/(np.exp(Tv/T)-1)**2
          """high temperature limit of rotational heat capacity"""
          Cp_rot = 3/2
          Cp_R = Cp_R + Cp_rot
     elif species_Name == "HE":
          """translation only"""
          Cp_R = 5/2
     elif species_Name == "AR":
          """translation only"""
          Cp_R = 5/2
     else:
          print(' Unknown species '+species_Name)
          exit()

     return Cp_R

# Define the mechanism and initial state
##ideal_gas = ct.Solution('H2O2-LT-RK.yaml','H2O2_IG')
##X1 = 'H2:1 O2:0.5'
ideal_gas = ct.Solution('chem-HC-Air.yaml')
X1 = 'N2:1'

T1 = 300
P1 = ct.one_atm
ideal_gas.TPX = T1, P1, X1

#select species to examine
speciesName ='CO'
species = ideal_gas.species_index(speciesName)  #index number of species
## Get species thermo input data  (NASA9 coefficients)
thermo_data = ideal_gas.species(species).thermo.coeffs
# temperature ranges for NASA data in file
T_num = int(thermo_data[0])
print('Number of temperature ranges ',str(T_num))
T_min = thermo_data[1]
print('Min temperature ', str(T_min))
T_max = thermo_data[(T_num-1)*11+2]
print('Max temperature ', str(T_max))
T_range = np.arange(T_min,T_max,step=2)
T_match = 200

# select options
low_temperature  = 1    # set to 1 to make comparisons with low temperature estimate
low_temperature_fit = 0 # set to 1 to fit low temperature estimate to NASA9 format

# loop through initial pressures and evaluate specific heat capacities
CP_IG = np.zeros(len(T_range))
HRT_IG = np.zeros(len(T_range))
SR_IG = np.zeros(len(T_range))
for i, T in enumerate(T_range):
     ideal_gas.TPX = T, P1, X1
     CP_IG[i] = ideal_gas.standard_cp_R[species]
     HRT_IG[i] = ideal_gas.standard_enthalpies_RT[species]
     SR_IG[i] = ideal_gas.standard_entropies_R[species]

if low_temperature_fit:
     #coefficients for lowest temperature range
     a = thermo_data[3:10]
     T_low = 30
     ideal_gas.TPX = T_match, P1, X1
     Cp_R_match = ideal_gas.standard_cp_R[species]
     S_R_match = ideal_gas.standard_entropies_R[species]
     H_RT_match = ideal_gas.standard_enthalpies_RT[species]
     
     def poly_eval(a,z):
          # evaluate NASA9 heat capacity and derivative at temperature T after multiplying by T**2
          cp = a[0] + z*(a[1] + z*(a[2] + z*(a[3]+z*(a[4]+z*(a[5]+z*a[6])))))
          dcpdt =  z*(a[1]+ z*(2*a[2]+z*(3*a[3]+z*(4*a[4]+z*(5*a[5]+6*z*a[6])))))
          return cp, dcpdt
     #constraints for fittings low temperature data to lowest NASA point
     A, B = poly_eval(a,T_match)
     C = T_low**2*Cp_R(T_low,speciesName)

     ### create target for matching low temperature data  
     p =  np.ones(7)
     x = np.linspace(T_low, T_match, 50)
     y = np.zeros(len(x))
     for j, T in enumerate(x):
          y[j] = T**2*Cp_R(T,speciesName)

     ### Polynomial to fit
     def F(x,a0,a1,a2,a3,a4,a5,a6):
          F =  a0 + x*(a1 + x*(a2+x*(a3+x*(a4+x*(a5+x*a6)))))
          return F

     def residuals(p,x,y):
          b =  x[-1]*(p[1]+ x[-1]*(2*p[2]+x[-1]*(3*p[3]+x[-1]*(4*p[4]+x[-1]*(5*p[5]+6*x[-1]*p[6])))))  
          penalty = abs(A-F(x[-1],p[0],p[1],p[2],p[3],p[4],p[5],p[6]))*10000 #match T^2*cp at T_match                           
          penalty = penalty + abs(C-F(x[0],p[0],p[1],p[2],p[3],p[4],p[5],p[6]))*10000 #match T^2*cp at T_low
          penalty = penalty + abs(B-b)*10000 # match derivative at T_match                 
          return abs(y - F(x,p[0],p[1],p[2],p[3],p[4],p[5],p[6])) + penalty

     #Fit data

     popt1, pcov1 = curve_fit(F, x, y)
     popt2, pcov2 = leastsq(func=residuals, x0=popt1, args=(x,y))
     y_fit1 = F(x, *popt1)
     y_fit2 = F(x, *popt2)

     ## plot fits
     fig, ax = plt.subplots(1)
     ax.scatter(x,y)
     ax.plot(x,y_fit1, color='g', alpha=0.75, label='curve_fit')
     ax.plot(x,y_fit2, color='b', alpha=0.75, label='leastsq')
     plt.ylabel(r'$T^2C_p/R$', fontsize=14)
     plt.xlabel(r'$T$ (K)', fontsize=14)
     plt.title(r' Species '+speciesName, fontsize=14)
     plt.legend()

     # compute coefficients for H, S formulas
     z = T_match
     def match(z,a):
          a7 = z*H_RT_match-z*((-a[0]/z +np.log(z)*a[1])/z + a[2] + z*(a[3]/2 + z*(a[4]/3 + z*(a[5]/4 + z*a[6]/5))))
          a8 = S_R_match - ((-a[0]/z/2 - a[1])/z + np.log(z)*a[2] + z*(a[3] + z*(a[4]/2 + z*(a[5]/3 + z*a[6]/4))))
          return a7, a8
     a7, a8 = match(T_match,popt2)


# Plot data

plt.rcParams['xtick.labelsize'] = 12
plt.rcParams['ytick.labelsize'] = 12
plt.rcParams['figure.autolayout'] = True
plt.rcParams['axes.labelsize'] = 14
plt.rcParams['font.family'] = 'serif'
plt.figure(num='comparion')
plt.axis([0, 300, 2,6])   # adjust limits as needed
plt.plot(T_range, CP_IG, '-', linewidth=2.0, color='b',label='NASA')
# low temperature values from partition fucntions
if low_temperature:
     T_LOW = np.arange(0.01,200,step=1)
     CP_LOW = np.zeros(len(T_LOW))
     for k, T in enumerate(T_LOW):
          CP_LOW[k] = Cp_R(T,speciesName)   #

     plt.plot(T_LOW, CP_LOW, '-', linewidth=2.0, color='r',label='Partition Function')
if low_temperature_fit:
     plt.plot(x,y_fit2/x**2,'.',color='k',linewidth=2.0,label='Least squares')
plt.ylabel(r'$C_p/R$', fontsize=14)
plt.xlabel(r'$T$ (K)', fontsize=14)
#plt.title(r' Species '+speciesName, fontsize=14)
plt.legend()
# save plot
#plt.savefig('H2-O2_Cp_R.pdf', dpi=350, format='pdf')

##plt.figure(num='enthalpy')
##plt.axis([5, 500, -50, 10])
##plt.plot(T_range, HRT_IG, '-', linewidth=2.0, color='b',label='NASA')
##plt.ylabel(r'$H/RT$', fontsize=14)
##plt.xlabel(r'$T$ (K)', fontsize=14)
##plt.title(r' Species '+speciesName, fontsize=14)
### save plot
###plt.savefig('H2-O2_H_RT.pdf', dpi=350, format='pdf')
##
##plt.figure(num='entropy')
##plt.axis([5, 500, 0, 25])
##plt.plot(T_range, SR_IG, '-', linewidth=2.0, color='b',label='NASA')
##plt.ylabel(r'$S/R$', fontsize=14)
##plt.xlabel(r'$T$ (K)', fontsize=14)
##plt.title(r' Species '+speciesName, fontsize=14)
##
### save plot
###plt.savefig('H2-O2_S_R.pdf', dpi=350, format='pdf')
plt.show()
