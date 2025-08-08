% demo_ShockTube
% Shock and Detonation Toolbox Demo Program
% 
% Calculates the solution to ideal shock tube problem. Three cases possible: 
%     conventional nonreactive driver (gas), 
%     constant volume combustion driver (uv),
%     forward CJ detonation (initiated at closed end opposite diaphragm)
%     reverse CJ detonation (initiated at diaphragm) driver (cj)
% 
% Creates P-u relationship for shock wave and expansion wave, plots results
% and interpolates to find shock speed and states 2 and 3 given states 1
% and 4.  Creates tabulation of relevant states for solution.
% 
% Requires installation of Shock and Detonation Toolbox package
% #########################################################################
% Theory, numerical methods and applications are described in:
% 
% SDToolbox: Numerical Tools for Shock and Detonation Wave Modeling,
% Explosion Dynamics Laboratory, Contributors: S.T. Browne, J.L. Ziegler,
% N.P. Bitter, B.E. Schmidt, J. Lawson and J.E. Shepherd, GALCIT
% Technical Report FM2018.001 Revised February 2021.
%
% Report and installation instructions available at  
% http://shepherd.caltech.edu/EDL/PublicResources/sdt/
% Please cite this report and the website if you use these routines. 
% Please refer to LICENCE.txt or the above report for copyright and disclaimers.
% #########################################################################
% Updated April 11 2021
%%
clear all;clc;close all;
%% select plotting options
shock_plot = false;
exp_plot = false;
rfl_plot = false;
T0 = 298.15; 
%% Select case for the driver operations
%driver = 'gas';  % nonreactive gas driver
driver = 'uv';   % constant pressure gas driver
%driver = 'cj_r';  % detonation driver - reverse (initiation at diaphragm)
%driver = 'cj_f';  % detonation driver - forward (initiation at end of driver)
switch driver
    case 'gas'
        P_driver = 3.3E6; 
        T_driver = T0;
        q4 = 'He:1.0';
        driver_mech = 'Mevel2017.cti';
        driver_gas = Solution(driver_mech);
        expansion = 'fr';
    case {'uv','cj_r','cj_f'}
        P_driver_fill = 0.25*oneatm; 
        T_driver_fill = T0;
        q4 = 'C3H8:1.0 O2:5.0';
        driver_mech = 'gri30_highT.cti';
        driver_gas = Solution(driver_mech);
        expansion = 'eq';
    end
%% Select case for the driven gas operations (use equilibrium only for very strong shocks
%driven = 'frozen';  %non reactive driven gas
driven = 'equilibrium';  %reactive driven gas
% initial state, composition, and gas object for driver section
% set initial state, composition, and gas object for driven section
P1 = oneatm; T1 = T0;
q1 = 'O2:0.2095 N2:0.7808 Ar:0.0093 CO2:0.0004';   % standard atmosphere composition
%q1 = 'O2:1 N2:3.76';  %combustion air composition approximation
%q1 = 'CO2:1.';
%q1 = 'Ar:1.0';
driven_mech = 'airNASA9noions.cti';
driven_gas = Solution(driven_mech);
reflected_gas = Solution(driven_mech);
set(driven_gas,'Temperature',T1,'Pressure',P1,'MoleFractions',q1);
%
disp(['demo_ShockTube - driver case: ',driver,'; driven case: ',driven]);
%% Evaluate initial state 
rho1 = density(driven_gas);
a1 = soundspeed_fr(driven_gas);
gamma1 = a1^2*rho1/P1;
%% Evaluate post-shock state (frozen) for a range of shock speeds 
i = 0;
Ms_max = 15;  %  THIS VALUE MUST BE ADJUSTED IN SOME CASES, INCREASE IF NO SOLUTION, DECREASE IF PLOTTING RANGE TOO LARGE
disp(['Generating points on shock P-u curve']);
for U = a1*1.01:25:Ms_max*a1
    i = i +1;
    switch driven
        case 'frozen'
            [shocked_gas] = PostShock_fr(U, P1, T1, q1, driven_mech);
        case 'equilibrium'
            [shocked_gas] = PostShock_eq(U, P1, T1, q1, driven_mech);
    end
    a2(i) = soundspeed_fr(shocked_gas);
    P2(i) = pressure(shocked_gas);
    T2(i) = temperature(shocked_gas);
    rho2(i) = density(shocked_gas);
    w2 = rho1*U/density(shocked_gas);
    u2(i) = U - w2;
    Us(i) = U;
    P_sig(i) = 1 + 2*gamma1*((U/a1)^2 - 1)/(gamma1 + 1);
    rho_sig(i) = (gamma1+1)/(gamma1-1 +2/(U/a1)^2);
    T_sig(i) = P_sig(i)/rho_sig(i);
end
ismax = length(Us);
if (shock_plot)
    font ='TimesRoman';
    fontsize = 12;
    set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
    figure('Name','shock P(Us)');
    plot(Us/a1,(P2/P1));
    hold on
    plot(Us/a1,P_sig)
    xlab = xlabel('Shock Mach number $M_s$','FontSize', 12);
    ylab = ylabel('Shock pressure ratio $P_2/P_1$','FontSize', 12);
    set(xlab,'Interpreter','latex');
    set(ylab,'Interpreter','latex');
    leg = legend('Real gas',['Ideal gas $\gamma$ ', num2str(gamma1)],'location','Northwest');
    set(leg,'Interpreter','latex');
    hold off
    figure('Name','shock P(u)');
    plot(u2/a1,(P2/P1));
    hold on
    plot(u2/a1,P_sig)
    xlab = xlabel('postshock velocity $u_2/a_1$','FontSize', 12);
    ylab = ylabel('Shock pressure ratio $P_2/P_1$','FontSize', 12);
    set(xlab,'Interpreter','latex');
    set(ylab,'Interpreter','latex');
    leg = legend('Real gas',['Ideal gas $\gamma$ ', num2str(gamma1)],'location','Northwest');
    set(leg,'Interpreter','latex');
    hold off
    figure('Name','shock T(u)');
    plot(Us/a1,(T2/T1));
    hold on
    plot(Us/a1,T_sig)
    xlab = xlabel('Shock Mach number $M_s$','FontSize', 12);
    ylab = ylabel('Shock temperature ratio $T_2/T_1$','FontSize', 12);
    set(xlab,'Interpreter','latex');
    set(ylab,'Interpreter','latex');
    leg = legend('Real gas',['Ideal gas $\gamma$ ', num2str(gamma1)],'location','Northwest');
    set(leg,'Interpreter','latex');
    hold off
    figure('Name','shock rho(u)');
    plot(Us/a1,(rho2/rho1));
    hold on
    plot(Us/a1,rho_sig)
    xlab = xlabel('Shock Mach number $M_s$','FontSize', 12);
    ylab = ylabel('Shock density ratio $\rho_2/\rho_1$','FontSize', 12);
    set(xlab,'Interpreter','latex');
    set(ylab,'Interpreter','latex');
    leg = legend('Real gas',['Ideal gas $\gamma$ ', num2str(gamma1)],'location','Northwest');
    set(leg,'Interpreter','latex');
    hold off
end
switch driver
    %% Set initial state for driver section - pressurized gas and no reaction
    % for nonreacting driver, use frozen expansion
    case 'gas'
        set(driver_gas,'Temperature',T_driver,'Pressure',P_driver,'MoleFractions',q4);
        P4 = pressure(driver_gas);
        T4 = temperature(driver_gas);
        rho4 = density(driver_gas);
        a4 = soundspeed_fr(driver_gas);      
        u3(1) = 0.;
        X4 = moleFractions(driver_gas);
    case 'uv'
        %% Set initial state for driver section - CV combustion driven shock tube
        % using constant volume state approximation
        % for reacting driver, use equilibrium expansion
        set(driver_gas,'Temperature',T_driver_fill,'Pressure',P_driver_fill,'MoleFractions',q4);
        equilibrate(driver_gas,'UV');
        P4 = pressure(driver_gas);
        T4 = temperature(driver_gas);
        rho4 = density(driver_gas);
        a4 = soundspeed_fr(driver_gas);      
        u3(1) = 0.;
        X4 = moleFractions(driver_gas);
    case 'cj_r'
        %% Detonation driver reverse, CJ wave moving away from diaphragm
        set(driver_gas,'Temperature',T_driver_fill,'Pressure',P_driver_fill,'MoleFractions',q4);
        rho_driver_fill = density(driver_gas);
        [cj_speed] = CJspeed(P_driver_fill, T_driver_fill, q4, driver_mech);
        [driver_gas] = PostShock_eq(cj_speed,P_driver_fill, T_driver_fill, q4, driver_mech);
        P4 = pressure(driver_gas);
        P_CJ = P4;
        T4 = temperature(driver_gas);
        T_CJ = temperature(driver_gas);
        w3 = cj_speed*rho_driver_fill/density(driver_gas);
        u3(1) = w3-cj_speed;
        a4 = soundspeed_fr(driver_gas);
        rho4 = density(driver_gas);
        X4 = moleFractions(driver_gas);
    case 'cj_f'
        %% Detonation driver forward, CJ wave moving toward diaphragm
        set(driver_gas,'Temperature',T_driver_fill,'Pressure',P_driver_fill,'MoleFractions',q4);
        rho_driver_fill = density(driver_gas);
        [cj_speed] = CJspeed(P_driver_fill, T_driver_fill, q4, driver_mech);
        [driver_gas] = PostShock_eq(cj_speed,P_driver_fill, T_driver_fill, q4, driver_mech);
        P4 = pressure(driver_gas);
        P_CJ = P4;
        T4 = temperature(driver_gas);
        T_CJ = T4;
        a4 = soundspeed_eq(driver_gas);
        rho4 = density(driver_gas);
        w3 = cj_speed*rho_driver_fill/density(driver_gas);
        u3(1) = cj_speed-w3;
        X4 = moleFractions(driver_gas);
end
%% Evaluate initial state for expansion computation
% The driver_gas state used as initial conditions is
% 'uv' equilibrium state with u3(1) = 0 for constant volume combustion
% CJ state with u3(1) = wcj-Ucj < 0 for 'cj_r'  reverse propagation detonation
% Cj state with u3(1) = Ucj - wc > 0 for 'cj_f' forward propagation detonation
rho3(1) = density(driver_gas);
P3(1) = pressure(driver_gas);
T3(1) = temperature(driver_gas);
S4 = entropy_mass(driver_gas);
a3(1) = soundspeed_eq(driver_gas);
gamma4 = a3(1)^2*rho3(1)/P3(1);
%% compute unsteady expansion states from driver
disp(['Generating points on driver isentrope P-u curve']);
i = 1;
vv = 1/rho3(1);
P_eig(1) = 1.;
while (u3(i)<u2(ismax) && T3(i) > 15)
    vv = vv*1.01;
    sv = [S4, vv];
    setState_SV(driver_gas,sv);
    switch expansion
        case 'eq'
            %required for equilibrium expansion
            equilibrate(driver_gas,'SV');
            a3(i) = soundspeed_eq(driver_gas);
        case 'fr'
            %use this for frozen expansion
            a3(i) = soundspeed_fr(driver_gas);
    end
    P3(i+1) = pressure(driver_gas);
    T3(i+1) = temperature(driver_gas);
    rho3(i+1) = density(driver_gas);
    a3(i+1) = soundspeed_eq(driver_gas);
    % left facing expansion wave
    u3(i+1) = u3(i) - 0.5*(P3(i+1)-P3(i))*(1./(rho3(i)*a3(i)) + 1./(rho3(i+1)*a3(i+1)));
    P_eig(i+1) = (1- (gamma4-1)*(u3(i+1)-u3(1))/a3(1)/2)^(2*gamma4/(gamma4-1));
    i = i+1;
end
%% compute driver reflected shock states 
i = 0;
Md_max = 1.5;  %  May need to adjust this value
disp(['Generating points on driver reflected shock P-u curve']);
driver_shocked_gas = Solution(driver_mech);
set(driver_shocked_gas,'T',T4,'P',P4,'MoleFractions',X4);
for Ud = a4*1.01:25:Md_max*a4
    i = i +1;
    switch driven
        case 'frozen'
            [driver_shocked_gas] = PostShock_fr(Ud, P4, T4, X4, driver_mech);
        case 'equilibrium'
            [driver_shocked_gas] = PostShock_eq(Ud, P4, T4, X4, driver_mech);
    end
    a3r(i) = soundspeed_fr(driver_shocked_gas);
    P3r(i) = pressure(driver_shocked_gas);
    T3r(i) = temperature(driver_shocked_gas);
    rho3r(i) = density(driver_shocked_gas);
    u3r(i) = u3(1) -(1-rho4/density(driver_shocked_gas))*Ud;
end
%
if (rfl_plot) % plot reflected shock P-u
    font ='TimesRoman';
    fontsize = 12;
    set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
    figure('Name','Driver shock P-u relation');
    plot(u3r,P3r/P1);
    xlab = xlabel('Flow Mach number $u_{3,r}$','FontSize', 12);
    ylab = ylabel('Pressure ratio $P_r/P_1$','FontSize', 12);
    set(xlab,'Interpreter','latex');
    set(ylab,'Interpreter','latex');
end
%
if (exp_plot) % plot reflected expansion p-u
    font ='TimesRoman';
    fontsize = 12;
    set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
    figure('Name','Driver expansion P-u relation');
    plot(u3/a3(1),(P3/P3(1)));
    hold on
    plot(u3/a3(1),P_eig)
    xlab = xlabel('Flow Mach number $u/a_4$','FontSize', 12);
    ylab = ylabel('Pressure ratio $P/P_4$','FontSize', 12);
    set(xlab,'Interpreter','latex');
    set(ylab,'Interpreter','latex');
    leg = legend('Real gas',['Ideal gas $\gamma$ ', num2str(gamma4)],'location','Northeast');
    set(leg,'Interpreter','latex');
    hold off
end
%% computing intersection of the expansion wave curve with the shock wave
%  Input limits for finding intersection of polars
pmin = 1.01;
pmax = min(max(P3),max(P2))/P1;
% define intersection function for pressure match
fun = @(x) (interp1(P2/P1,u2,x,'pchip')-interp1(P3/P1,u3,x,'pchip'))^2;
% solve for pressure match at contact surface
disp(['Seeking solution for pressure matched contact surface with driver expansion']);
P_contact = fminbnd(@(x) fun(x), pmin, pmax);
% check for velocities match at contact surface 
u2_contact = interp1(P2/P1,u2,P_contact,'pchip');
u3_contact = interp1(P3/P1,u3,P_contact,'pchip');
delta_u=  u2_contact-u3_contact;
if abs(delta_u) < 0.1  %this number is arbitary but appears to be reasonable
    disp(['Pressure matched solution for driver expansion found, u3-u2 ',num2str(delta_u)]);
    driver_wave = 'exp';
else
    disp(['Pressure matched solution for driver expansion not found, u3-u2 ',num2str(delta_u)]);
    disp(['Need to recompute match using driver shock'])
    driver_wave = 'shk';
    pmin = P_CJ/P1;
    pmax = min(max(P3r),max(P2))/P1;
    % define intersection function for pressure match
    fun = @(x) (interp1(P2/P1,u2,x,'pchip')-interp1(P3r/P1,u3r,x,'pchip'))^2;
    % solve for pressure match at contact surface
    disp(['Solving for pressure matched contact surface with driver shock']);
    P_contact = fminbnd(@(x) fun(x), pmin, pmax);
    % check for velocities match at contact surface
    u2_contact = interp1(P2/P1,u2,P_contact,'pchip');
    u3_contact = interp1(P3r/P1,u3r,P_contact,'pchip');
    delta_u=  u2_contact-u3_contact;
    disp(['Pressure matched solution for driver shock found, u3-u2 ',num2str(delta_u)]);
end
% find shock speed
U_shock = interp1(P2/P1,Us,P_contact,'pchip');
M_shock = U_shock/a1;
% find velocity and states on driven side of contact surface
T2_contact = interp1(P2/P1,T2,P_contact,'pchip');
a2_contact = interp1(P2/P1,a2,P_contact,'pchip');
rho2_contact = interp1(P2/P1,rho2,P_contact,'pchip');
M2 = u2_contact/a2_contact;
% find velocity and states on driver side of contact surface
if driver_wave == 'exp'
T3_contact = interp1(P3/P1,T3,P_contact,'pchip');
a3_contact = interp1(P3/P1,a3,P_contact,'pchip');
rho3_contact = interp1(P3/P1,rho3,P_contact,'pchip');
M3 = u3_contact/a3_contact;
elseif driver_wave == 'shk'
T3_contact = interp1(P3r/P1,T3r,P_contact,'pchip');
a3_contact = interp1(P3r/P1,a3r,P_contact,'pchip');
rho3_contact = interp1(P3r/P1,rho3r,P_contact,'pchip');
M3 = u3_contact/a3_contact;
end   
%%
% Display results on screen
disp(['Driven State (1)']);
disp(['   Fill gas ',q1]);
disp(['   Pressure ',num2str(P1/1E3),' (kPa)']);
disp(['   Temperature ',num2str(T1),' (K)']);
disp(['   Sound speed ',num2str(a1),' (m/s)']);
disp(['   Density ',num2str(rho1),' (kg/m3)']);
switch driver
    case 'cj_f'
        disp(['Driver State (CJ) - forward']);
        disp(['   Fill gas ',q4]);
        disp(['   Fill Pressure ',num2str(P_driver_fill/1E3),' (kPa)']);
        disp(['   Fill Temperature ',num2str(T_driver_fill),' (K)']);
        disp(['   CJ wave speed ',num2str(cj_speed),' (m/s)']);
        disp(['   CJ pressure ',num2str(P_CJ/1E3),' (kPa)']);
        disp(['   CJ temperature ',num2str(T_CJ),' (K)']);
        disp(['   CJ gas speed ',num2str(u3(1)),' (m/s)']);
    case 'cj_r'
        disp(['Driver State (CJ) - reverse']);
        disp(['   Fill gas ',q4]);
        disp(['   Fill Pressure ',num2str(P_driver_fill/1E3),' (kPa)']);
        disp(['   Fill Temperature ',num2str(T_driver_fill),' (K)']);
        disp(['   CJ wave speed ',num2str(cj_speed),' (m/s)']);
        disp(['   CJ pressure ',num2str(P_CJ/1E3),' (kPa)']);
         disp(['   CJ temperature ',num2str(T_CJ),' (K)']);
       % determine plateau state
        P4 = interp1(u3,P3,0,'pchip');  
        T4 = interp1(u3,T3,0,'pchip');  
        rho4 = interp1(u3,rho3,0,'pchip');
        a4 = interp1(u3,a3,0,'pchip');
        disp(['   Plateau state ']);
     case 'uv'
        disp(['Driver State (UV)']);
        disp(['   Fill gas ',q4]);
        disp(['   Fill Pressure ',num2str(P_driver_fill/1E3),' (kPa)']);
        disp(['   Fill Temperature ',num2str(T_driver_fill),' (K)']);
    case 'gas'
        disp(['Driver State (4)']);
        disp(['   Fill gas ',q4]);
end
disp(['   Pressure ',num2str(P4/1E3),' (kPa)']);
disp(['   Temperature ',num2str(T4),' (K)']);
disp(['   Sound speed ',num2str(a4),' (m/s)']);
disp(['   Density ',num2str(rho4),' (kg/m3)']);
disp(['Solution matching P-u for states 2 & 3']);
disp(['Shock speed ',num2str(U_shock),' (m/s)']);
disp(['Shock Mach number ',num2str(M_shock),' ']);
disp(['Shock Pressure ratio ',num2str(P_contact),' ']);
disp(['Postshock state (2) in driven section']);
disp(['   Pressure ',num2str(P1*P_contact/1E3),' (kPa)']);
disp(['   Velocity ',num2str(u2_contact),' (m/s)']);
disp(['   Temperature ',num2str(T2_contact),' (K)']);
disp(['   Sound speed ',num2str(a2_contact),' (m/s)']);
disp(['   Density ',num2str(rho2_contact),' (kg/m3)']);
disp(['   Mach number ',num2str(M2),' ']);
set(shocked_gas,'Temperature',T2_contact,'Pressure',P1*P_contact,'MoleFractions',q1);
%% compute reflected shock state  (optional)
% disp('Frozen post-reflected-shock state (5) in driven section');
% [p3,UR,reflected_gas] = reflected_fr(driven_gas,shocked_gas,reflected_gas,U_shock);
% % Output
% % UR - reflected shock wave speed relative to reflecting surface
% % p3 - pressure behind reflected shock wave
% % gas3 - Cantera gas object for equilibrium state behind reflectes shock wave
% disp(['   Reflected Shock Speed UR = ', num2str(UR), ' (m/s)']);
% disp(['   Pressure ',num2str(p3/1000.),' (kPa)']);
% disp(['   Temperature ',num2str(temperature(reflected_gas)),' (K)']);
% disp(['   Density ',num2str(density(reflected_gas)),' (kg/m3)']);
%
disp(['Expansion or shock state (3) in driver section']);
disp(['   Pressure ',num2str(P1*P_contact/1E3),' (kPa)']);
disp(['   Velocity ',num2str(u3_contact),' (m/s)']);
disp(['   Temperature ',num2str(T3_contact),' (K)']);
disp(['   Sound speed ',num2str(a3_contact),' (m/s)']);
disp(['   Density ',num2str(rho3_contact),' (kg/m3)']);
disp(['   Mach number ',num2str(M3),' ']);
Z = rho2_contact*a2_contact/(rho3_contact*a3_contact);
disp(['Impedance ratio at contact surface (rho a)_2/(rho a)_3 ',num2str(Z),' ']);
%%
% plot pressure-velocity diagram
title=['P-u diagram for case ',driver];
figure('Name',title); 
font ='TimesRoman';
fontsize = 12;
set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
plot(u2(:),P2(:)/P1,'k','LineWidth',1);
hold on
switch driver
     case 'cj_r'
        axis([min(u3) u2(ismax) 0 1.2*max(P3)/P1]);
        plot(u3(1),P_CJ/P1,'ok');  % CJ point
    case 'cj_f'
        axis([0 u2(ismax) 0 1.5*P4/P1]); 
        plot(u3(1),P_CJ/P1,'ok');  % CJ_point
        u3_tz = 2*u3(1)-u3 ;   % these are states on the Taylor-Zeldovich wave behind the detonation.
        plot(u3_tz,P3(:)/P1,'--r','LineWidth',1);  
         % find the plateau state at the end of t
        P4 = interp1(u3_tz,P3,0,'pchip');
        T4 = interp1(u3_tz,T3,0,'pchip');
        rho4 = interp1(u3_tz,rho3,0,'pchip');
        a4 = interp1(u3_tz,a3,0,'pchip');
        disp(['Plateau state on TZ wave']);
        disp(['   Pressure ',num2str(P4/1E3),' (kPa)']);
        disp(['   Temperature ',num2str(T4),' (K)']);
        disp(['   Sound speed ',num2str(a4),' (m/s)']);
        disp(['   Density ',num2str(rho4),' (kg/m3)']);
        plot(u3-2*u3(1),P3(:)/P1,'-.b','LineWidth',1)  % p-u from plateau state
        plot(u3r,P3r/P1,':m','LineWidth',1);  %p-u reflected shock solution
        % plateau state expansion-shock solution
        pmax = min(max(P4),max(P2))/P1;
        pmin = 1.0;
        % define intersection function
        fun = @(x) (interp1(P2/P1,u2,x,'pchip')-interp1(P3/P1,u3-2*u3(1),x,'pchip'))^2;
        % solve for intersection of P-u curves to find pressure at contact surface
        P_contact_tz = fminbnd(@(x) fun(x), pmin, pmax);
        U_shock = interp1(P2/P1,Us,P_contact_tz,'pchip');
        M_shock = U_shock/a1;
        % find velocity and states on driven side of contact surface
        u2_contact_tz = interp1(P2/P1,u2,P_contact_tz,'pchip');
        T2_contact_tz = interp1(P2/P1,T2,P_contact_tz,'pchip');
        a2_contact_tz = interp1(P2/P1,a2,P_contact_tz,'pchip');
        rho2_contact_tz = interp1(P2/P1,rho2,P_contact_tz,'pchip');
        M2_tz = u2_contact_tz/a2_contact_tz;
        disp(['Shock driven by plateau state:']);
        disp(['   Shock speed ',num2str(U_shock),' (m/s)']);
        disp(['   Shock Mach number ',num2str(M_shock),' ']);
        disp(['   Shock Pressure ratio ',num2str(P_contact),' ']);
        disp(['   Postshock state (2) in driven section']);
        disp(['     Pressure ',num2str(P1*P_contact_tz/1E3),' (kPa)']);
        disp(['     Velocity ',num2str(u2_contact_tz),' (m/s)']);
        disp(['     Temperature ',num2str(T2_contact_tz),' (K)']);
        disp(['     Sound speed ',num2str(a2_contact_tz),' (m/s)']);
        disp(['     Density ',num2str(rho2_contact_tz),' (kg/m3)']);
        disp(['     Flow Mach number ',num2str(M2_tz),' ']);
    otherwise
        axis([0 u2(ismax) 0 1.2*P4/P1]);
end
plot(u3(:),P3(:)/P1,'g','LineWidth',1);
plot(u2_contact,P_contact,'sk')
xlab = xlabel('velocity (m/s)','FontSize', 12);
ylab = ylabel('pressure $P/P_1$','FontSize', 12);
set(xlab,'Interpreter','latex');
set(ylab,'Interpreter','latex');
set(gca,'FontSize',10,'LineWidth',1);
switch driver
    case 'cj_r'
        leg = legend('Driven shock','CJ','Driver expansion (deceleration)','CS','location','Northeast');
    case 'cj_f'
        leg = legend('Driven shock','CJ','Driver expansion (deceleration)','TZ Plateau expansion (acceleration)','Driver reflected shock','Driver expansion (acceleration)','CS','location','Northeast');
    case 'uv'
        leg = legend('Driven shock','Driver expansion (uv)','CS','location','Northeast');
    case 'gas'
        leg = legend('Driven shock','Driver expansion (gas)','CS','location','Northeast');
end
set(leg,'Interpreter','latex');
hold off
