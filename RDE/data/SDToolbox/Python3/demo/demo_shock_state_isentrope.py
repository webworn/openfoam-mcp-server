"""
Shock and Detonation Toolbox Demo Program

Computes frozen post-shock state and isentropic expansion for specified shock speed.  
Create plots and output file. 

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
from sdtoolbox.thermo import soundspeed_fr
from sdtoolbox.postshock import PostShock_fr
import datetime
import matplotlib.pyplot as plt

# set initial state of gas, avoid using precise stoichiometric ratios. 
P1 = 100000; T1 = 295
q = 'O2:0.2095 N2:0.7808 CO2:0.0004 Ar:0.0093' 
mech = 'airNASA9ions.cti' 
gas1 = ct.Solution(mech)
gas1.TPX = T1,P1,q
a1_fr = soundspeed_fr(gas1)
R1 = gas1.density
gamma1_fr =  a1_fr**2*R1/P1

print('Initial Conditions')
print('   pressure '+str(P1)+' (Pa)')
print('   temperature '+str(T1)+' (K)')
print('   density '+str(R1)+' (kg/m3)')
print('a1 (frozen) '+str(a1_fr)+' (m/s)')
print('gamma1 (frozen) '+str(gamma1_fr)+' (m/s)')

print('Computing shock state and isentrope for '+q+' using '+mech)

# Evaluate the frozen state of the gas behind a shock wave 
# traveling at a specified speed (must be greater than sound speed)
shock_speed = 1633.
gas = PostShock_fr(shock_speed,P1, T1, q, mech)

# Evaluate properties of gas object and print out
T2 = gas.T
P2 = gas.P
R2 = gas.density
V2 = 1/R2
S2 = gas.entropy_mass
w2 = gas1.density*shock_speed/R2
u2 = shock_speed - w2
x2 = gas.X
a2_fr = soundspeed_fr(gas)
gamma2_fr =  a2_fr**2*R2/P2

print('Shock speed '+str(shock_speed)+' (m/s)')
print('Frozen Post-Shock State')
print('   pressure '+str(P2)+' (Pa)')
print('   temperature '+str(T2)+' (K)')
print('   density '+str(R2)+' (kg/m3)')
print('   entropy '+str(S2)+' (J/kg-K)')
print('   w2 (wave frame) '+str(w2)+' (m/s)')
print('   u2 (lab frame) '+str(u2)+' (m/s)')
print('   a2 (frozen) '+str(a2_fr)+' (m/s)')
print('   gamma2 (frozen) '+str(gamma2_fr)+' (m/s)')

# Find points on the isentrope connected to the CJ state, evaluate velocity
# in Taylor wave using trapezoidal rule to evaluate the Riemann function

vv = V2
V = [V2]
P = [P2]
R = [R2]
a = [a2_fr]
u = [u2]
T = [T2]
print('Generating points on isentrope and expansion wave velocity')

while u[-1] > 0:
    vv = vv*1.01
    gas.SVX = S2,vv,gas.X
    P.append(gas.P)
    R.append(gas.density)
    V.append(1./R[-1])
    T.append(gas.T)
    a.append(soundspeed_fr(gas))
    u.append(u[-1] + 0.5*(P[-1]-P[-2])*(1/(R[-1]*a[-1]) + 1/(R[-2]*a[-2])))

# estimate plateau conditions (state 3) by interpolating to find the u = 0 state.
P3 = P[-1] + u[-1]*(P[-2]-P[-1])/(u[-2]-u[-1])
a3 = a[-1]+ u[-1]*(a[-2]-a[-1])/(u[-2]-u[-1])
V3 = V[-1]+ u[-1]*(V[-2]-V[-1])/(u[-2]-u[-1])
T3 = T[-1]+ u[-1]*(T[-2]-T[-1])/(u[-2]-u[-1])
print('State 3');
print('   pressure '+str(P3)+' (Pa)')
print('   temperature '+str(T3)+' (K)')
print('   volume '+str(V3)+' (m3/kg)')

# replace last data point in arrays with interpolatead point.
P[-1] = P3
a[-1] = a3
V[-1] = V3
T[-1] = T3
u[-1] = 0.0

# evaluate final state to get sound speeds and effective gammas
gas.SVX = S2,V3,gas.X
a3_fr = soundspeed_fr(gas)
gamma3_fr =  a3_fr**2/(P3*V3)
print('   sound speed (frozen) '+str(a3_fr)+' (m/s)')
print('   gamma (frozen) '+str(gamma3_fr)+' (m/s)')

##### PLOTS #####
fontsize = 12

# Plot the isentrope
plt.figure(1)
plt.plot(V, P, 'k')
plt.xlabel(r'Volume (m$^3$/kg)',fontsize=fontsize)
plt.ylabel('Pressure (Pa)',fontsize=fontsize)
plt.title('Isentrope',fontsize=fontsize)
plt.tick_params(labelsize=12)
plt.ticklabel_format(style='sci',axis='y',scilimits=(0,0))
plt.xlim(xmin=0)
plt.ylim(ymin=0)

# Plot the sound speed
plt.figure(2)
plt.plot(V, a, 'k');
plt.xlabel(r'Volume (m$^3$/kg)',fontsize=fontsize)
plt.ylabel('Sound speed - eq (m/s)',fontsize=fontsize)
plt.title('Isentrope',fontsize=fontsize)
plt.tick_params(labelsize=12)
plt.xlim(xmin=0)

# Plot the particle velocity
plt.figure(3) 
plt.plot(V, u, 'k');
plt.xlabel(r'Volume (m$^3$/kg)',fontsize=fontsize)
plt.ylabel('Flow speed (m/s)',fontsize=fontsize)
plt.title('Isentrope',fontsize=fontsize)
plt.tick_params(labelsize=12)
plt.xlim(xmin=0)
plt.ylim(ymin=0)

# Plot the temperature
plt.figure(4) 
plt.plot(V, T, 'k')
plt.xlabel(r'Volume (m$^3$/kg)',fontsize=fontsize)
plt.ylabel('Temperature (K)',fontsize=fontsize)
plt.title('Isentrope',fontsize=fontsize)
plt.tick_params(labelsize=12)
plt.xlim(xmin=0)

##################################################################################################
# Create output file in text format with Tecplot-style variable labels for
# columns
##################################################################################################
fn = 'shock_isentrope.txt'
d = datetime.date.today()
fid = open(fn, 'w')
fid.write('# Shock State Isentrope\n')
fid.write('# Calculation run on %s\n' % d )
fid.write('# Initial conditions\n')
fid.write('# Shock speed  (m/s) %4.1f\n' % shock_speed)
fid.write('# Temperature (K) %4.1f\n' % T1)
fid.write('# Pressure (Pa) %2.1f\n' % P1)
fid.write('# Density (kg/m^3) %1.4e\n' % gas1.density)
fid.write('# Initial species mole fractions: '+q+'\n')
fid.write('# Reaction mechanism: '+mech+'\n\n')
fid.write('Variables = "Volume (m3/kg)", "Temperature (K)", "Pressure (atm)", "sound speed - eq (m/s)", "particle speed (m/s)"\n')
for i in range(len(V)):
    fid.write('%14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \n' % (V[i], T[i], P[i], a[i], u[i]))
fid.close()

plt.show()
