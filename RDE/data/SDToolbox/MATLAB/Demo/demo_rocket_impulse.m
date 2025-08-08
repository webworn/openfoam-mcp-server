% Shock and Detonation Toolbox Demo Program
% 
% Computes rocket performance using quasi-one dimensional isentropic flow using 
% both frozen and equilibrium properties for a range of helium dilutions in a 
% hydrogen-oxygen mixture.  Plots impulse as a function of dilution.
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
for helium = 1:1:40
he(helium) = (0.1*(helium-1))/(0.1*(helium-1)+1.5);
q = ['O2:0.5 H2:1.001 HE:' num2str(0.1*(helium-1))]; 
if helium == 1
    q = 'O2:0.5 H2:1.001';
end
Tc = 300;
Pc = 1.e7;
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
imax = 90;
for i = 1:1:imax
     pp = pp*0.95;
     sp = [St, pp];
    % compute equilibrium isentrope
    setState_SP(gas,sp);
    %equilibrate(gas,'SP','maxsteps 10000','maxiter 10000');
    equilibrate(gas,'SP');
    P(i) = pressure(gas);
    R(i) = density(gas);
    T(i) = temperature(gas);
    h_eq(i) = enthalpy_mass(gas);
    u_eq(i) = sqrt(2*(Ht-h_eq(i)));
    Isp_eq(i) = (u_eq(i)+ (P(i) - Pa)/(u_eq(i)*R(i)))/9.81 ; 
    % compute frozen impulse  (first call is so we get a good guess for
    % entropy 
    set(gas1,'Pressure',pp,'Temperature',T(i),'MoleFractions',qc);
    set(gas1,'Pressure',pp,'Entropy',St,'MoleFractions',qc);
    h_fr(i) = enthalpy_mass(gas1);
    R_fr(i) = density(gas1);
    T_fr(i) = temperature(gas1);
    u_fr(i) = sqrt(2*(Ht-h_fr(i)));
    Isp_fr(i) = (u_fr(i)+ (P(i) - Pa)/(u_fr(i)*R_fr(i)))/9.81 ; 
end
I_fr(helium) = Isp_fr(imax);
I_eq(helium) = Isp_eq(imax);
disp(['final pressure ',num2str(P(imax)),' (Pa)']);
disp(['final specific impulse (eq) ',num2str(Isp_eq(imax)),' (s)']);
disp(['final velocity (eq) ',num2str(u_eq(imax)),' (m/s)']);
disp(['final specific impulse (fr) ',num2str(Isp_fr(imax)),' (s)']);
disp(['final velocity (fr) ',num2str(u_fr(imax)),' (m/s)']);
disp('-------------------------------------------'); disp(''); disp('');
end
%
% Plot the impulses
%
figure; clf;
fontsize = 12;
plot(he(:), I_fr(:), 'r', he(:), I_eq(:), 'b');
xlabel('helium mole fraction ','FontSize',fontsize);
ylabel('Isp (s)','FontSize',fontsize);
title('Warm gas thruster performance','FontSize',fontsize);
set(gca,'FontSize',12,'LineWidth',2);
legend('Frozen','Equilibrium')
% 

