"""
Shock and Detonation Toolbox Demo Program

Compare propagating shock and stagnation point profiles.
 
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
import pickle
from numpy import exp
import matplotlib.pyplot as plt
import sys

failMsg = """
       This demo compares the saved outputs of demo_STGshk and demo_RZshk.
       At least one of the output files 'stg.p' and 'rzn.p' are missing.
       Please run the above demos first in order to generate these outputs.
           """

try:
    stg = pickle.load(open('stg.p','rb'))
    rzn = pickle.load(open('rzn.p','rb'))
except FileNotFoundError:
    sys.exit(failMsg)

U1 = stg['U1']
Delta = stg['Delta']

# relationship between stagnation point distance and 
x = Delta*(1 - exp(-rzn['distance']/Delta))

masterFontSize = 12
plt.rc('font',size=masterFontSize)

figTitle = 'Reaction Zone Structure U = '+str(U1)+' m/s'
rznLabel = 'shock tube (x transformation)'
stgLabel = 'stagnation flow'

eps = 1e-6

plt.figure()
plt.title(figTitle,y=1.08)
maxy = max(stg['P'])
miny = min(stg['P'])
minx = min(stg['distance'])+eps
maxx = max(stg['distance'])
plt.semilogx(x,rzn['P'],color='k',marker='o',mfc='none',ms=8,markevery=30,label=rznLabel)
plt.semilogx(stg['distance'],stg['P'],'b',label=stgLabel)
plt.axis([minx,maxx,miny,maxy])
plt.xlabel('distance (m)')
plt.ylabel('pressure (Pa)')
plt.legend()
plt.savefig('shk-stg-P.eps',bbox_inches='tight')

plt.figure()
plt.title(figTitle,y=1.08)
maxy = max(stg['T'])
miny = min(stg['T'])
plt.semilogx(x,rzn['T'],color='k',marker='o',mfc='none',ms=8,markevery=30,label=rznLabel)
plt.semilogx(stg['distance'],stg['T'],'b',label=stgLabel)
plt.axis([minx,maxx,miny,maxy])
plt.xlabel('distance (m)')
plt.ylabel('temperature (K)')
plt.legend()
plt.savefig('shk-stg-T.eps',bbox_inches='tight')

plt.figure()
plt.title(figTitle,y=1.08)
maxy = max(stg['rho'])
miny = min(stg['rho'])
plt.semilogx(x,rzn['rho'],color='k',marker='o',mfc='none',ms=8,markevery=50,label=rznLabel)
plt.semilogx(stg['distance'],stg['rho'],'b',label=stgLabel)
plt.axis([minx,maxx,miny,maxy])
plt.ticklabel_format(style='sci',scilimits=(0,0),axis='y')
plt.xlabel('distance (m)')
plt.ylabel('density (kg/m$^3$)')
plt.legend()
plt.savefig('shk-stg-rho.eps',bbox_inches='tight')

plt.figure()
plt.title(figTitle,y=1.08)
plt.loglog(x,rzn['species'][8].T,color='k',marker='o',mfc='none',ms=8,markevery=50,label=rznLabel)
plt.loglog(stg['distance'],stg['species'][8,:].T,'b',label=stgLabel)
plt.axis([minx,maxx,1e-3,1])
plt.xlabel('distance (m)')
plt.yticks([1e-3,1e-2,1e-1,1e0])
plt.ylabel('CO$_2$ mass fraction')
plt.legend()
plt.savefig('shk-stg-co2.eps',bbox_inches='tight')

plt.figure()
plt.title(figTitle,y=1.08)
maxx = max(rzn['distance'])
minx = min(rzn['distance'])+1E-6
maxy = max(stg['rho'])
miny = min(stg['rho'])
plt.semilogx(rzn['distance'],rzn['rho'],color='k',marker='o',mfc='none',ms=8,markevery=50,label=rznLabel)
plt.semilogx(stg['distance'],stg['rho'],'b',label=stgLabel)
plt.axis([minx,maxx,miny,maxy])
plt.ticklabel_format(style='sci',scilimits=(0,0),axis='y')
plt.xlabel('distance (m)')
plt.ylabel('density (kg/m$^3$)')
plt.legend()
plt.savefig('shk-stg-rho-comp-distance.eps',bbox_inches='tight')

plt.figure()
plt.title(figTitle,y=1.08)
maxx = max(stg['time'])
minx = min(stg['time'])+eps
maxy = max(stg['rho'])
miny = min(stg['rho'])
plt.semilogx(rzn['time'],rzn['rho'],color='k',marker='o',mfc='none',ms=8,markevery=50,label=rznLabel)
plt.semilogx(stg['time'],stg['rho'],'b',label=stgLabel)
plt.axis([minx,maxx,miny,maxy])
plt.ticklabel_format(style='sci',scilimits=(0,0),axis='y')
plt.xlabel('time (s)')
plt.ylabel('density (kg/m$^3$)')
plt.legend()
plt.savefig('shk-stg-rho-comp-time.eps',bbox_inches='tight')

plt.show()

