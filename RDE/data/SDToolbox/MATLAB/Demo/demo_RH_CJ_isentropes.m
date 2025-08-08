% Shock and Detonation Toolbox Demo Program
% 
% Creates plot for equilibrium product Hugoniot curve near CJ point, 
% shows Rayleigh Line with slope Ucj and four isentropes bracketing CJ point. 
% Creates plot showing Gruneisen coefficient, denominator in Jouguet's rule, isentrope slope.
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
disp('demo_RH_CJ_isentropes')
%% Input data
P1 = 100000; T1 = 300; 
q = 'H2:2 O2:1 N2:3.76';    
mech = 'Mevel2017.cti';   
%% compute initial state
gas1 = Solution(mech);
set(gas1, 'T', T1, 'P', P1, 'X', q);
h1 = enthalpy_mass(gas1);
r1 = density(gas1);
v1 = 1/density(gas1);
%% compute CJ speed
[cj_speed] = CJspeed(P1, T1, q, mech);
disp(['CJ speed ', num2str(cj_speed)]);
%% compute CJ state
[gas] = PostShock_eq(cj_speed,P1, T1, q, mech);
vcj = 1/density(gas);
pcj = pressure(gas);
Pcj = pcj/oneatm;
tcj = temperature(gas);
scj = entropy_mass(gas);
qcj = moleFractions(gas);
U1 = cj_speed;
%% Compute RAYLEIGH Line
vmin = 0.3*vcj;
vmax = 1.7*vcj;
vinc = .01*vcj;
i = 0;
for v2=vmin:vinc:vmax
    i = i + 1;
    vR(i) = v2; 
    PRpa = (P1 - r1^2*U1^2*(v2 - v1));
    PR(i) = PRpa/oneatm;                    %Pressure in atmospheres  
end
disp('Rayleigh Line Array Created')
%% Compute product Hugoniot 
%
% Use CV combust as initial state 
set(gas, 'T', T1, 'P', P1, 'X', q);    
equilibrate(gas, 'UV');
Ta = temperature(gas);
va = 1/density(gas);

PH2(1) = pressure(gas)/oneatm;
vH2(1) = va;
Grun(1) = gruneisen_eq(gas);
gamma(1) = (soundspeed_eq(gas))^2/(vH2(1)*pressure(gas));
denom(1) = 1 +  Grun(1)*(vH2(1)-v1)/(2*vH2(1));
i = 0;
vb = va;
while(vb > vmin)
    i = i + 1;
    vb = va - i*.01;
    %use new volume and previous temperature to find next state
    array = [vb;h1;P1;v1];
    options=optimset('Display','off');
    [x,fval] = fsolve(@hug_eq,Ta,options,gas,array);
    set(gas, 'T', x, 'Density', 1/vb);
    PH2(i+1) = pressure(gas)/oneatm;
    vH2(i+1) = vb;
    Grun(i+1) = gruneisen_eq(gas);
    gamma(i+1) = (soundspeed_eq(gas))^2/(vH2(i+1)*pressure(gas));
    denom(i+1) = 1 +  Grun(i+1)*(vH2(i+1)-v1)/(2*vH2(i+1));
end
disp('Product Hugoniot Array Created')
%% isentropes near CJ point
set(gas, 'T', tcj, 'P', pcj, 'X', qcj);
s = scj*.98;
deltas = 0.01*scj;
for j=1:1:4
   i = 1; 
    for pp=PR*oneatm
        % frozen (CJ composition)
        set(gas,'Pressure',pp,'Entropy',s,'MoleFractions',qcj);
        Ps_f(i,j) = pressure(gas)/oneatm;
        vs_f(i,j) = 1/density(gas);
        % equilibrium 
        sp = [s, pp];
        setState_SP(gas,sp);
        equilibrate(gas,'SP');
        Ps_e(i,j) = pressure(gas)/oneatm;
        vs_e(i,j) = 1/density(gas);
        i = i + 1;
    end
    s = s + deltas;
end
disp('Isentropes created')
%% plot creation
maxP = max(PR) ;
minP = min(PR);
maxV = max(vR);
minV = min(vR);

	figure(1); clf;
	plot(vR(:),PR(:),'k:','LineWidth',2);  
	hold on;
	plot(vcj,Pcj,'ko','LineWidth',2)
    hold on;
	plot(vH2,PH2,'r','LineWidth',2)
        for j=1:1:4
    hold on;
	plot(vs_e(:,j),Ps_e(:,j),'b','LineWidth',2);
    end
	hold off;
	axis([minV maxV minP maxP]);
	xlabel('specific volume (m^3/kg)','FontSize', 14);
	ylabel('pressure (atm)','FontSize', 14);
    legend('Rayleigh','CJ','Hugoniot','Isentropes');
	set(gca,'FontSize',12,'LineWidth',2);
    
    
	figure(2); clf;
	plot(vH2(:),Grun(:),'r','LineWidth',2);
    hold on;
    plot(vH2(:),denom(:),'b','LineWidth',2);
    hold on;
    plot(vH2(:),gamma(:),'g','LineWidth',2);
    hold off;
	axis([minV maxV 0 1.5]);
	xlabel('specific volume (m^3/kg)','FontSize', 14);
	ylabel('Coefficent','FontSize', 14);
    legend('Gruneisen', 'denominator', 'gamma');
	set(gca,'FontSize',12,'LineWidth',2);
