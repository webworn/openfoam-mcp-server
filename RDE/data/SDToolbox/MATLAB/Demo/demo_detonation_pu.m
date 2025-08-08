% Shock and Detonation Toolbox Demo Program
% 
% Calculate the hugoniot and p-u relationship for shock waves centered on
% the CJ state. Generates an output file.
%  
% ################################################################################
% Theory, numerical methods and applications are described in the following report:
% 
%     Numerical Solution Methods for Shock and Detonation Jump Conditions, S.
%     Browne, J. Ziegler, and J. E. Shepherd, GALCIT Report FM2006.006 - R3,
%     California Institute of Technology Revised September, 2018.
% 
% Please cite this report and the website if you use these routines. 
% 
% Please refer to LICENCE.txt or the above report for copyright and disclaimers.
% 
% http://shepherd.caltech.edu/EDL/PublicResources/sdt/
% 
% ################################################################################ 
% Updated September 2018
% Tested with: 
%     MATLAB 2017b and 2018a, Cantera 2.3 and 2.4
% Under these operating systems:
%     Windows 8.1, Windows 10, Linux (Debian 9)
%%
clear;clc;
disp('demo_detonation_pu')

% set initial state, composition, and gas object
P1 = 100000; T1 = 300; 
q = 'H2:0.31 N2O:0.69';    
mech = 'Mevel2015.cti';
gas1 = Solution(mech);
set(gas1,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
rho1 = density(gas1);

% Find CJ speed
[cj_speed] = CJspeed(P1, T1, q, mech);

% CJ state
[gas] = PostShock_eq(cj_speed,P1, T1, q, mech);
disp(['CJ computation for ',mech,' with composition ',q])
P2 = pressure(gas);
T2 = temperature(gas);
q2 = moleFractions(gas);
rho2 = density(gas);
w2 = rho1/rho2*cj_speed;
u2= cj_speed-w2;
Umin = soundspeed_eq(gas);
disp(['CJ speed ',num2str(cj_speed),' (m/s)']);
disp('CJ State');
disp(['   Pressure ',num2str(P2),' (Pa)']);
disp(['   Particle velocity ',num2str(u2),' (m/s)']);

% reflected shock from CJ detonation
gas3 = Solution(mech);
[p3,Ur,gas3] = reflected_eq(gas1,gas,gas3,cj_speed);
v3 = 1./density(gas);
disp(['Reflected shock (equilibrium) computation for ',mech,' with composition ',q])
disp(['   Reflected wave speed ',num2str(Ur),' (m/s)']);
disp(['   Reflected shock pressure ',num2str(pressure(gas3)),' (Pa)']);
% Bounds for reflected shock speed
Umax = Ur+u2;
disp(['   Maximum reflected shock speed ',num2str(Umax),' (m/s)']);
disp(['   Minimum reflected shock speed ',num2str(Umin),' (m/s)']);
disp('Compute shock adiabat and wave curve starting at the CJ state');
i = 1;

for ushock=(Umin:10:Umax)
[gas] = PostShock_eq(ushock, P2, T2, q2, mech);
p(i) = pressure(gas);
v(i) = 1./density(gas);
u3(i) = u2-(ushock - ushock*rho2/density(gas));
disp(['   shock velocity ',num2str(ushock),' (m/s)']);
i=i+1;
end

disp('Final Point (should be rigid wall reflection point)');
p(i) = p3;
v(i) = v3;
u3(i) = 0.;
disp(['   shock velocity ',num2str(ushock),' (m/s)']);
disp(['   post shock pressure ',num2str(p(i)),' (Pa)']);
disp(['   post shock volume ',num2str(v(i)),' (m3/kg)']);
disp(['   post shock velocity ',num2str(u3(i)),' (m/s)']);

fn = 'cj-pu.txt';
d = date;
fid = fopen(fn, 'w');
fprintf(fid, '# Shock wave centered on CJ Detonation State\n');
fprintf(fid, '# Calculation run on %s\n', d );
fprintf(fid, '# Initial conditions\n');
fprintf(fid, '# Temperature (K) %4.1f\n', T1);
fprintf(fid, '# Pressure (Pa) %2.1f\n', P1);
fprintf(fid, '# Density (kg/m^3) %1.4e\n', density(gas1));
fprintf(fid, ['# Initial species mole fractions: ' q '\n']);
fprintf(fid, ['# Reaction mechanism: ' mech '\n\n']);
fprintf(fid, 'Variables = "particle velocity (m/s)", "pressure (Pa)", "volume (m3/kg)"\n');
z = [u3; p; v];
fprintf(fid, '%14.5e \t %14.5e \t %14.5e \n', z);
fclose(fid);

