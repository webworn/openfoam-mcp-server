function aequil = soundspeed_eq(gas)
% Computes the equilibrium sound speed by using a centered finite
% difference approximation. Directly evaluating pressure at two density/specific
% volume states along an isentrope requires use of equilibrate('SV'). However,
% this may not always converge at high pressure. Instead, a more robust method
% using equilibrate('TP') is used that employs thermodynamic identities detailed
% further in Appendix G2 of the report.
% 
% The old version of this code that used 'SV' instead of 'TP' is commented out
% below for reference.
% 
% FUNCTION SYNTAX:
%     aequil =  soundspeed_eq(gas)
% 
% INPUT:
%     gas = working gas object (restored to original state at end of function)
% 
% OUTPUT:
%     aequil = equilibrium sound speed = sqrt({d P/d rho)_s, eq) (m/s)

%%
%    Old method which is not reliable, 'SV' will not converge at high pressure
%    rho0 = density(gas);
%    T0 = temperature(gas);
%    P0 = pressure(gas);
%    s0 = entropy_mass(gas);
%    x0 = moleFractions(gas);
%    rho1 = 0.99*rho0;
%    set(gas,'Density',rho1,'Entropy',s0,'MoleFractions',x0);
%    equilibrate(gas,'SV');
%    p1 = pressure(gas);
%    rho2 = 1.01*rho0;
%    set(gas,'Density',rho2,'Entropy',s0,'MoleFractions',x0);
%    equilibrate(gas,'SV');
%    p2 = pressure(gas);
%    dpdrho_s = (p2 - p1)/(rho2 - rho1);
%    aequil = sqrt(dpdrho_s);
%    set(gas,'Temperature',T0,'Pressure',P0,'MoleFractions',x0);

%%
    % New method based on Taylor series expansion, centered differences using thermodynamic identities
    T0 = temperature(gas);
    P0 = pressure(gas);
    x0 = moleFractions(gas);
   
    T2 = 1.01*T0;
    T1 = 0.99*T0;
    set(gas,'Temperature',T1,'Pressure',P0);
    equilibrate(gas,'TP');
    s1 = entropy_mass(gas);

    set(gas,'Temperature',T2,'Pressure',P0);
    equilibrate(gas,'TP');
    s2 = entropy_mass(gas);

    DSDT = (s2 - s1)/(T2 - T1);

    P1 = 0.99*P0;
    P2 = 1.01*P0;
    set(gas,'Temperature',T0,'Pressure',P1);
    equilibrate(gas,'TP');
    s1 = entropy_mass(gas);

    set(gas,'Temperature',T0,'Pressure',P2);
    equilibrate(gas,'TP');
    s2 = entropy_mass(gas);

    DSDP = (s2 - s1)/(P2 - P1);

    DTDP = -DSDP/DSDT;

    TA = T0 + DTDP*(P1-P0);
    set(gas,'Temperature',TA,'Pressure',P1);
    equilibrate(gas,'TP');
    rhoA = density(gas);

    TB = T0 + DTDP*(P2-P0);
    set(gas,'Temperature',TB,'Pressure',P2);
    equilibrate(gas,'TP');
    rhoB = density(gas);

    DRHODP = (P2-P1)/(rhoB-rhoA);

    aequil = sqrt(DRHODP);

    % Restore gas object to original state
    set(gas,'Temperature',T0,'Pressure',P0,'MoleFractions',x0);

