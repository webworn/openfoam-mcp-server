% Shock and Detonation Toolbox Demo Program
% 
% Computes frozen post-shock state and isentropic expansion for specified shock speed.  
% Create plots and output file. 
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
disp('demo_shock_state_isentrope')

% set initial state of gas, avoid using precise stoichiometric ratios. 
P1 = 100000; T1 = 295; 
q = 'O2:0.2095 N2:0.7808 CO2:0.0004 Ar:0.0093';    
mech = 'airNASA9ions.cti';   
gas1 = Solution(mech);
set(gas1,'Temperature',T1,'Pressure',P1,'MoleFractions',q);
a1_fr = soundspeed_fr(gas1);
R1 = density(gas1);
gamma1_fr =  a1_fr*a1_fr*R1/P1;
disp('Initial Conditions')
disp(['   pressure ',num2str(P1),' (Pa)']);
disp(['   temperature ',num2str(T1),' (K)']);
disp(['   density ',num2str(R1),' (kg/m3)']);
disp(['a1 (frozen) ',num2str(a1_fr),' (m/s)']);
disp(['gamma1 (frozen) ',num2str(gamma1_fr),' (m/s)']);

disp(['Computing shock state and isentrope for ',q,' using ',mech]);
%
% Evaluate the frozen state of the gas behind a shock wave 
% traveling at a specified speed (must be greater than sound speed.
%
shock_speed = 1633.;
[gas] = PostShock_fr(shock_speed,P1, T1, q, mech);
%
% Evaluate properties of gas object and print out
%
T2 = temperature(gas);
P2 = pressure(gas);
R2 = density(gas);
V2 = 1/R2;
S2 = entropy_mass(gas);
w2 = density(gas1)*shock_speed/R2;
u2 = shock_speed - w2;
x2 = moleFractions(gas);
a2_fr = soundspeed_fr(gas);
gamma2_fr =  a2_fr*a2_fr*R2/P2;
disp(['Shock speed ',num2str(shock_speed),' (m/s)']);
disp('Frozen Post-Shock State');
disp(['   pressure ',num2str(P2),' (Pa)']);
disp(['   temperature ',num2str(T2),' (K)']);
disp(['   density ',num2str(R2),' (kg/m3)']);
disp(['   entropy ',num2str(S2),' (J/kg-K)']);
disp(['   w2 (wave frame) ',num2str(w2),' (m/s)']);
disp(['   u2 (lab frame) ',num2str(u2),' (m/s)']);
disp(['   a2 (frozen) ',num2str(a2_fr),' (m/s)']);
disp(['   gamma2 (frozen) ',num2str(gamma2_fr),' (m/s)']);
%
%
% Find points on the isentrope connected to the CJ state, evaluate velocity
% in Taylor wave using trapezoidal rule to evaluate the Riemann function
%
vv = V2;
V(1) = V2;
P(1) = P2;
R(1) = R2;
a(1) = a2_fr;
u(1) = u2;
T(1) = T2;
disp(['Generating points on isentrope and expansion wave velocity'])
i = 1;
while (u(i)>0) 
    i = i+1;
    vv = vv*1.01;
    sv = [S2, vv];
    setState_SV(gas,sv);
%    equilibrate(gas,'SV');
    P(i) = pressure(gas);
    R(i) = density(gas);
    V(i) = 1./R(i);
    T(i) = temperature(gas);
    a(i) = soundspeed_fr(gas);
    u(i) = u(i-1) + 0.5*(P(i)-P(i-1))*(1./(R(i)*a(i)) + 1./(R(i-1)*a(i-1)));
end
% estimate plateau conditions (state 3) by interpolating to find the u = 0 state.
nfinal = i;
P3 = P(nfinal) + u(nfinal)*(P(nfinal-1)-P(nfinal))/(u(nfinal-1)-u(nfinal));
a3 = a(nfinal)+ u(nfinal)*(a(nfinal-1)-a(nfinal))/(u(nfinal-1)-u(nfinal));
V3 = V(nfinal)+ u(nfinal)*(V(nfinal-1)-V(nfinal))/(u(nfinal-1)-u(nfinal));
T3 = T(nfinal)+ u(nfinal)*(T(nfinal-1)-T(nfinal))/(u(nfinal-1)-u(nfinal));
disp('State 3');
disp(['   pressure ',num2str(P3),' (Pa)']);
disp(['   temperature ',num2str(T3),' (K)']);
disp(['   volume ',num2str(V3),' (m3/kg)']);
% replace last data point in arrays with interpolatead point.
P(nfinal) = P3;
a(nfinal) = a3;
V(nfinal) = V3;
T(nfinal) = T3;
u(nfinal) = 0.0;
% evaluate final state to get sound speeds and effective gammas
sv = [S2, V3];
setState_SV(gas,sv);
a3_fr = soundspeed_fr(gas);
gamma3_fr =  a3_fr*a3_fr/(P3*V3);
disp(['   sound speed (frozen) ',num2str(a3_fr),' (m/s)']);
disp(['   gamma (frozen) ',num2str(gamma3_fr),' (m/s)']);
close all;

%%%%% PLOTS %%%%%
%
% Plot the isentrope
%
figure;
fontsize = 12;
plot(V(:), P(:), 'k');
xlabel('Volume (m^3/kg)','FontSize',fontsize);
ylabel('Pressure (Pa)','FontSize',fontsize);
title('Isentrope','FontSize',fontsize);
set(gca,'FontSize',12,'LineWidth',2);
% 
% Plot the sound speed
%
figure 
plot(V(:), a(:), 'k');
xlabel('Volume (m^3/kg)','FontSize',fontsize);
ylabel('Sound speed - eq (m/s)','FontSize',fontsize);
title('Isentrope','FontSize',fontsize);
set(gca,'FontSize',12,'LineWidth',2);
% 
% Plot the particle velocity
%
figure 
plot(V(:), u(:), 'k');
xlabel('Volume (m^3/kg)','FontSize',fontsize);
ylabel('Flow speed (m/s)','FontSize',fontsize);
title('Isentrope','FontSize',fontsize);
set(gca,'FontSize',12,'LineWidth',2);
% 
% Plot the temperature
%
figure 
plot(V(:), T(:), 'k');
xlabel('Volume (m^3/kg)','FontSize',fontsize);
ylabel('Temperature (K)','FontSize',fontsize);
title('Isentrope','FontSize',fontsize);
set(gca,'FontSize',12,'LineWidth',2);
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Create output file in text format with Tecplot-style variable labels for
% columns
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fn = ['shock_isentrope.txt'];
d = datestr(now,0);
fid = fopen(fn, 'w');
fprintf(fid, '# Shock State Isentrope\n');
fprintf(fid, '# Calculation run on %s\n', d );
fprintf(fid, '# Initial conditions\n');
fprintf(fid, '# Shock speed  (m/s) %4.1f\n', shock_speed);
fprintf(fid, '# Temperature (K) %4.1f\n', T1);
fprintf(fid, '# Pressure (Pa) %2.1f\n', P1);
fprintf(fid, '# Density (kg/m^3) %1.4e\n', density(gas1));
fprintf(fid, ['# Initial species mole fractions: ' q '\n']);
fprintf(fid, ['# Reaction mechanism: ' mech '\n\n']);
fprintf(fid, 'Variables = "Volume (m3/kg)", "Temperature (K)", "Pressure (atm)", "sound speed - eq (m/s)", "particle speed (m/s)"\n');
z = [V; T; P; a; u];
fprintf(fid, '%14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \n', z);
fclose(fid);


