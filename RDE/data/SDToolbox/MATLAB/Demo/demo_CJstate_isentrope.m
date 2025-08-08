% Shock and Detonation Toolbox Demo Program
% 
% Calculates  points on the isentrope and Taylor-Zeldovich expansion 
% behind a CJ detonation.  Prints output file and makes plots.
%  
% ################################################################################
% Theory, numerical methods and applications are described in the following report:
% 
% Explosion Dynamics Laboratory. 2020. 
% “SDToolbox:  Numerical Tools for Shock and Detonation Wave Modeling.” 
% GALCIT Report FM2018.001. Pasadena, CA: California Institute of Technology.
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
disp('demo_CJstate_isentrope')
%% set initial state of gas, avoid using precise stoichiometric ratios. 
P1 = 101325; T1 = 298.15; 
%q = 'C3H8:2 O2:5';  
%mech = 'gri30_highT.cti'; 
q = 'NC5H12:1 O2:8 N2:30.08';
mech = 'JetSurf2.cti';
gas1 = Solution(mech);
set(gas1,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
a1_fr = soundspeed_fr(gas1);
R1 = density(gas1);
gamma1_fr =  a1_fr*a1_fr*R1/P1;
W1 = meanMolecularWeight(gas1);
disp('Initial State');
disp(['   Pressure ',num2str(P1),' (Pa)']);
disp(['   Temperature ',num2str(T1),' (K)']);
disp(['   Density ',num2str(R1),' (kg/m3)']);
disp(['   a1 (frozen) ',num2str(a1_fr),' (m/s)']);
disp(['   gamma1 (frozen) ',num2str(gamma1_fr)]);
disp(['   W1 ',num2str(W1),' (kg/kmol)']);
%% Calculate the CJSpeed using the Minimum Wave Speed Method
%
disp(['Computing CJ state and isentrope for ',q,' using ',mech]);
[cj_speed] = CJspeed(P1, T1, q, mech);
%% Evaluate the equilibrium state of the gas behind a shock wave 
% traveling at the CJ speed
[gas] = PostShock_eq(cj_speed,P1, T1, q, mech);
%% Evaluate properties of gas object and print out
%
T2 = temperature(gas);
P2 = pressure(gas);
R2 = density(gas);
V2 = 1/R2;
S2 = entropy_mass(gas);
w2 = density(gas1)*cj_speed/R2;
u2 = cj_speed - w2;
x2 = moleFractions(gas);
a2_eq = soundspeed_eq(gas);
a2_fr = soundspeed_fr(gas);
gamma2_fr =  a2_fr*a2_fr*R2/P2;
gamma2_eq =  a2_eq*a2_eq*R2/P2;
W2 = meanMolecularWeight(gas);
disp(['CJ speed ',num2str(cj_speed),' (m/s)']);
disp('CJ State');
disp(['   Pressure ',num2str(P2),' (Pa)']);
disp(['   Temperature ',num2str(T2),' (K)']);
disp(['   Density ',num2str(R2),' (kg/m3)']);
disp(['   Entropy ',num2str(S2),' (J/kg-K)']);
disp(['   w2 (wave frame) ',num2str(w2),' (m/s)']);
disp(['   u2 (lab frame) ',num2str(u2),' (m/s)']);
disp(['   a2 (frozen) ',num2str(a2_fr),' (m/s)']);
disp(['   a2 (equilibrium) ',num2str(a2_eq),' (m/s)']);
disp(['   Gamma2 (frozen) ',num2str(gamma2_fr)]);
disp(['   Gamma2 (equilibrium) ',num2str(gamma2_eq)]);
disp(['   W2 ',num2str(W2),' (kg/kmol)']);
%% compute the effective value of q based on two-gamma model
M1 = cj_speed/a1_fr;
eparam = a1_fr*a1_fr*(M1^(-2)*(gamma2_eq/gamma1_fr)^2*(1+gamma1_fr*M1*M1)^2/(2*(gamma2_eq*gamma2_eq-1))-1/(gamma1_fr-1)-M1*M1/2);
disp(['Detonation CJ Mach number ',num2str(M1)]);
disp(['2-gamma energy parameter q ',num2str(eparam),' (J/kg)']);
%% Find points on the isentrope connected to the CJ state, evaluate velocity
% in Taylor wave using trapezoidal rule to evaluate the Riemann function
%
npoints=100;
vv = V2;
V(1) = V2;
P(1) = P2;
R(1) = R2;
a(1) = a2_eq;
u(1) = u2;
T(1) = T2;
disp(['Generating points on isentrope and computing Taylor wave velocity'])
i = 1;
while (u(i)>0) 
    i = i+1;
    vv = vv*1.01;
    sv = [S2, vv];
    setState_SV(gas,sv);
    equilibrate(gas,'SV');
    P(i) = pressure(gas);
    R(i) = density(gas);
    V(i) = 1./R(i);
    T(i) = temperature(gas);
    a(i) = soundspeed_eq(gas);
    u(i) = u(i-1) + 0.5*(P(i)-P(i-1))*(1./(R(i)*a(i)) + 1./(R(i-1)*a(i-1)));
end
%% estimate plateau conditions (state 3) by interpolating to find the u = 0 state.
nfinal = i;
P3 = P(nfinal) + u(nfinal)*(P(nfinal-1)-P(nfinal))/(u(nfinal-1)-u(nfinal));
a3 = a(nfinal)+ u(nfinal)*(a(nfinal-1)-a(nfinal))/(u(nfinal-1)-u(nfinal));
V3 = V(nfinal)+ u(nfinal)*(V(nfinal-1)-V(nfinal))/(u(nfinal-1)-u(nfinal));
T3 = T(nfinal)+ u(nfinal)*(T(nfinal-1)-T(nfinal))/(u(nfinal-1)-u(nfinal));
disp('State 3 ');
disp(['   Pressure ',num2str(P3),' (Pa)']);
disp(['   Temperature ',num2str(T3),' (K)']);
disp(['   Volume ',num2str(V3),' (m3/kg)']);
% replace last data point in arrays with interpolated point.
p(nfinal) = P3;
a(nfinal) = a3;
V(nfinal) = V3;
T(nfinal) = T3;
u(nfinal) = 0.0;
%% evaluate final state to get sound speeds and effective gammas
sv = [S2, V3];
setState_SV(gas,sv);
equilibrate(gas,'SV');
a3_fr = soundspeed_fr(gas);
gamma3_fr =  a3_fr*a3_fr/(P3*V3);
gamma3_eq =  a3*a3/(V3*P3);
disp(['   Sound speed (frozen) ',num2str(a3_fr),' (m/s)']);
disp(['   Sound speed (equilibrium) ',num2str(a3),' (m/s)']);
disp(['   Gamma (frozen) ',num2str(gamma3_fr)]);
disp(['   Gamma (equilibrium) ',num2str(gamma3_eq)]);
close all;
%% Plot the isentrope
%
figure
fontsize = 12;
plot(V(:), P(:), 'k');
xlabel('Volume (m^3/kg)','FontSize',fontsize);
ylabel('Pressure (Pa)','FontSize',fontsize);
title('Isentrope through CJ point','FontSize',fontsize);
set(gca,'FontSize',12,'LineWidth',2);
%% Plot the sound speed
%
figure 
plot(V(:), a(:), 'k');
xlabel('Volume (m^3/kg)','FontSize',fontsize);
ylabel('Sound speed - eq (m/s)','FontSize',fontsize);
title('Isentrope through CJ point','FontSize',fontsize);
set(gca,'FontSize',12,'LineWidth',2);
%% Plot the particle velocity
%
figure 
plot(V(:), u(:), 'k');
xlabel('Volume (m^3/kg)','FontSize',fontsize);
ylabel('Flow speed (m/s)','FontSize',fontsize);
title('Isentrope through CJ point','FontSize',fontsize);
set(gca,'FontSize',12,'LineWidth',2);
%% Plot the temperature
%
figure 
plot(V(:), T(:), 'k');
xlabel('Volume (m^3/kg)','FontSize',fontsize);
ylabel('Temperature (K)','FontSize',fontsize);
title('Isentrope through CJ point','FontSize',fontsize);
set(gca,'FontSize',12,'LineWidth',2);
%% Create output file in text format with Tecplot-style variable labels for columns
fn = ['cj_isentrope.txt'];
d = date;
fid = fopen(fn, 'w');
fprintf(fid, '# CJ State Isentrope\n');
fprintf(fid, '# Calculation run on %s\n', d );
fprintf(fid, '# Initial conditions\n');
fprintf(fid, '# Temperature (K) %4.1f\n', T1);
fprintf(fid, '# Pressure (Pa) %2.1f\n', P1);
fprintf(fid, '# Density (kg/m^3) %1.4e\n', density(gas1));
fprintf(fid, ['# Initial species mole fractions: ' q '\n']);
fprintf(fid, ['# Reaction mechanism: ' mech '\n\n']);
fprintf(fid, 'Variables = "Volume (m3/kg)", "Temperature (K)", "Pressure (atm)", "sound speed - eq (m/s)", "particle speed (m/s)"\n');
z = [V; T; P; a; u];
fprintf(fid, '%14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \n', z);
fclose(fid);


