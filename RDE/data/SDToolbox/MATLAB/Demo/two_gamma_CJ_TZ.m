% two_Gamma_TZ.m
% Computes two-gamma approximation to CJ detonation wave and 
% Taylor-Zeldovich expansion.  See Appendix C.5 from SDT report:
%
% Explosion Dynamics Laboratory. 2020. 
% “SDToolbox:  Numerical Tools for Shock and Detonation Wave Modeling.” 
% GALCIT Report FM2018.001. Pasadena, CA: California Institute of Technology.
% http://shepherd.caltech.edu/EDL/PublicResources/sdt/
% 
clear all; clc; close all;
% values determined by SDT program demo_CJ_isentrope.m
%% Parameters for 
mix = 'C3H8:2 O2:5';
P1 = 101325.; %Pa
T1  = 298.15; %K
M_CJ  = 8.8378;  %CJ Mach number
W_1 = 35.4553;    % reactant molar mass
W_2 = 17.0585;    % product molar mass
gamma_1 = 1.2468; % reactant ratio of specific heats (frozen)
gamma_2 = 1.1709; % product ratio of specific heats (equilibrium)
%% CJ state (normalized by initial (state 1 = reactant) values
P_CJ = (gamma_1*M_CJ^2+1)/(gamma_2+1);   % CJ pressure ratio
rho_CJ = gamma_1*(gamma_2+1)*M_CJ^2/(1 + gamma_1*M_CJ^2)/gamma_2; % CJ density ratio
T_CJ = P_CJ*W_2/rho_CJ/W_1;  % CJ temperature rate
a_CJ = sqrt(gamma_2*W_1*T_CJ/gamma_1/W_2);   %CJ sound speed
u_CJ = M_CJ - a_CJ;  % postdetonation velocity, lab frame
q = (gamma_2+1)*a_CJ^2/(gamma_2-1)/2 - (1/(gamma_1-1) + M_CJ^2/2);  %effective combustion energy normalized by a1^2
%% Plateau state (u_3 = 0) behind TZ wave  (normalized by state 1 values)
a_3 =  (gamma_2+1)*a_CJ/2 - (gamma_2-1)*M_CJ/2;
T_3 = gamma_1*W_2*(a_3^2)/gamma_2/W_1;
P_3 = P_CJ*(T_3/T_CJ)^(gamma_2/(gamma_2-1));
rho_3 = rho_CJ*(P_3/P_CJ)^(1/gamma_2);
x_3 = a_3/M_CJ;  % location of plateau (0 < x < x_3) for x_CJ =1 (detonation location)
%%  Generating TZ wave  states (normalized by state 1 values)
%   x = Distance  from wall (x=0) to detonation (x =1)
n = 101;
x = linspace(0,1,n);  
% initialize all variables to plateau state 3 
% first do primative variables
a = a_3*ones(1,n);
p = P_3*ones(1,n);
T =  T_3*ones(1,n);
rho = rho_3*ones(1,n);
u = zeros(1,n);
% now conservative form
f_1 = rho;       
f_2 = rho.*u;
c = W_1/gamma_1/(gamma_2-1)/W_2;
f_3 = rho.*(c*T-q+u.*u/2);
% use isentropic relations to compute properties within expansion wave
for i=1:n
    X = x(i)*M_CJ/a_3;
    if (X > 1)   %expansion wave:  1 < X < M_CJ/a_3,  plateau  0 < X < 1
        a(i) = a_3*(1- (gamma_2-1)*(1-X)/(gamma_2+1));
        u(i) = u_CJ - 2*(a_CJ - a(i))/(gamma_2-1);
        T(i) = T_CJ*(a(i)/a_CJ)^2;
        P(i) = P_CJ*(T(i)/T_CJ)^(gamma_2/(gamma_2-1));
        rho(i) = rho_CJ*(P(i)/P_CJ)^(1/gamma_2);
    elseif  (X < 1)  %plateau region
        a(i) = a_3;
        u(i) = 0.;
        P(i) = P_3;
        T(i) = T_3;
        rho(i) = rho_3;
    end
    f_1(i) = rho(i);
    f_2(i) = rho(i)*u(i);
    f_3(i) =  rho(i)*(c*T(i)-q+u(i)*u(i)/2);
end
%% Plot the solution
%
fontsize = 12;
figure 
plot(x(:), u(:), 'k');
xlabel('x)','FontSize',fontsize);
ylabel('u/a_1','FontSize',fontsize);
title('Taylor-Zeldovich Flow behind Planar CJ wave','FontSize',fontsize);
set(gca,'FontSize',12,'LineWidth',2);
%
figure
plot(x(:), a(:), 'k');
xlabel('x','FontSize',fontsize);
ylabel('a/a_1','FontSize',fontsize);
title('Taylor-Zeldovich Flow behind Planar CJ wave','FontSize',fontsize);
set(gca,'FontSize',12,'LineWidth',2);
%
figure 
plot(x(:),P(:), 'k');
xlabel('x','FontSize',fontsize);
ylabel('P/P_1','FontSize',fontsize);
title('Taylor-Zeldovich Flow behind Planar CJ wave','FontSize',fontsize);
set(gca,'FontSize',12,'LineWidth',2);
%
figure 
plot(x(:), T(:), 'k');
xlabel('x','FontSize',fontsize);
ylabel('T/T_1','FontSize',fontsize);
title('Taylor-Zeldovich Flow behind Planar CJ wave','FontSize',fontsize);
set(gca,'FontSize',12,'LineWidth',2);
%
figure 
plot(x(:), rho(:), 'k');
xlabel('x','FontSize',fontsize);
ylabel('rho/rho_1','FontSize',fontsize);
title('Taylor-Zeldovich Flow behind Planar CJ wave','FontSize',fontsize);
set(gca,'FontSize',12,'LineWidth',2);

%% Create output file in text format with Tecplot-style variable labels for columns
fn = ['CJ_TZ.txt'];
d = date;
fid = fopen(fn, 'w');
fprintf(fid, ' Taylor-Zeldovich flow behinid planar CJ wave\n');
fprintf(fid, ' Calculation run on %s\n', d );
fprintf(fid, ' Initial conditions\n');
fprintf(fid, [' Mixture: ' mix '\n']);
fprintf(fid, ' Temperature (K) %4.1f\n', T1);
fprintf(fid, ' Pressure (Pa) %2.1f\n', P1);
fprintf(fid, ' 2-gamma CJ Parameters\n');
fprintf(fid, ' MCJ %1.4e\n', M_CJ);
fprintf(fid, ' gamma_1 %1.4e\n', gamma_1);
fprintf(fid, ' gamma_2 %1.4e\n', gamma_2);
fprintf(fid, ' q %1.4e\n', q);
fprintf(fid, ' x_3 %1.4e\n', a_3/M_CJ);
fprintf(fid, 'Variables = "x",  "u", "a",  "P", "T","rho","f_1","f_2","f_3"\n');
z = [x; u; a; P; T; rho; f_1; f_2; f_3];
fprintf(fid, '%14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \n', z);
fclose(fid);



    
