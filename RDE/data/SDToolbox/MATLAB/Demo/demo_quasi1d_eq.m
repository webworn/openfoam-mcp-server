% Shock and Detonation Toolbox Demo Program
% 
% Computes ideal quasi-one dimensional flow using equilibrium properties to 
% determine exit conditions for expansion to a specified pressure.  
% Carries out computation for a range of helium dilutions.
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
% set composition and stagnation state
mech = 'Mevel2017.cti';
gas = Solution(mech);
gas1 = Solution(mech);
for helium = 1:1:20
q = ['O2:1 H2:2.001 He:' num2str(helium)];
Tc = 300;
Pc = 1.*10^6;
set(gas, 'T', Tc, 'P', Pc, 'X', q);
disp(['Computing chamber conditions and isentropic quasi 1-d flow for ',q,' using ',mech])
% compute equilibrium properties for constant pressure burn
equilibrate(gas,'HP');
qc = moleFractions(gas);
% Isentropic expansion from stagnation state to low pressure
Tt = temperature(gas);
St = entropy_mass(gas);
Ht = enthalpy_mass(gas);
Pt = pressure(gas);
Rt = density(gas);
ct = soundspeed_eq(gas);
gammat_eq = ct^2*Rt/Pt;
disp(['Total pressure ',num2str(Pt),' (Pa)']);
disp(['Total temperature ',num2str(Tt),' (K)']);
disp(['Total density ',num2str(Rt),' (kg/m3)']);
disp(['Total entropy ',num2str(St),' (J/kg-K)']);
disp(['Total sound speed (equilibrium) ',num2str(ct),' (m/s)']);
disp(['gamma2 (equilibrium) ',num2str(gammat_eq),' (m/s)']);
% ambient pressure for impulse computation
Pa = 0.0 ; 
disp(['Computing isentropic expansion']);
pp = Pt;
imax = 200;
for i = 1:1:imax
    if i < 10
        pp = pp*0.99;
    else
        pp = pp*0.95;
    end
    sp = [St, pp];
    % compute equilibrium isentrope
    setState_SP(gas,sp);
    %equilibrate(gas,'SP');
    equilibrate(gas,'SP');
    P(i) = pressure(gas);
    R(i) = density(gas);
    T(i) = temperature(gas);
    c(i) = soundspeed_eq(gas);
    h_eq(i) = enthalpy_mass(gas);
    u_eq(i) = sqrt(2*(Ht-h_eq(i)));
    Isp_eq(i) = (u_eq(i)+ (P(i) - Pa)/(u_eq(i)*R(i)))/9.81 ; 
    M(i) = u_eq(i)/c(i);
    % compute frozen impulse  (first call is so we get a good guess for
    % entropy)
    set(gas1,'Pressure',pp,'Temperature',T(i),'MoleFractions',qc);
    set(gas1,'Pressure',pp,'Entropy',St,'MoleFractions',qc);
    h_fr(i) = enthalpy_mass(gas1);
    u_fr(i) = sqrt(2*(Ht-h_fr(i)));
    Isp_fr(i) = (u_fr(i)+ (P(i) - Pa)/(u_fr(i)*R(i)))/9.81 ; 
end
disp(['final pressure ',num2str(P(imax)),' (Pa)']);
disp(['final specific impulse (eq) ',num2str(Isp_eq(imax)),' (s)']);
disp(['final velocity (eq) ',num2str(u_eq(imax)),' (m/s)']);
disp(['final specific impulse (fr) ',num2str(Isp_fr(imax)),' (s)']);
disp(['final velocity (fr) ',num2str(u_fr(imax)),' (m/s)']);
% bracket sonic state
i = 1;
while M(i)< 1 
    i = i + 1;
end
istar = i;
% interpolate to find throat conditions
Pstar = P(istar-1) + (P(istar) - P(istar-1))*(1.0 - M(istar-1))/(M(istar)-M(istar-1));
Tstar = T(istar-1) + (T(istar) - T(istar-1))*(1.0 - M(istar-1))/(M(istar)-M(istar-1));
Rstar = R(istar-1) + (R(istar) - R(istar-1))*(1.0 - M(istar-1))/(M(istar)-M(istar-1));
cstar = c(istar-1) + (c(istar) - c(istar-1))*(1.0 - M(istar-1))/(M(istar)-M(istar-1));
rhocstar = cstar*Rstar;
disp(['Sonic state (eq) pressure ',num2str(Pstar),' (Pa)']);
disp(['Sonic state (eq) temperature ',num2str(Tstar),' (s)']);
disp(['Sonic state (eq) velocity ',num2str(cstar),' (m/s)']);
disp(['Sonic state (eq) mass flux ',num2str(rhocstar),' (kg/m^2s)']);
disp('-------------------------------------------'); disp(''); disp('');

% compute the area ratio A/A* based on equilibrium flow properties
for i = 1:1:imax
    Area(i) = rhocstar/(u_eq(i)*R(i));
end
end
