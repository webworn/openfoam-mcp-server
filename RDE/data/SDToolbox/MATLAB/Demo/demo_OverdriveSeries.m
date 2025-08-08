% Shock and Detonation Toolbox Demo Program
% 
% This is a demostration of how to vary the Overdrive (U/UCJ)
% in a loop for constant volume explosions and the ZND detonation simulations.
% 
% NOTE: The convention in this demo is overdrive = shock_speed/CJspeed,
%        Another convention is (U/UCJ)^2, so be careful when using this demo
% 
% Here the explosion functions are called for varying conditions, new properties are calculated, 
% plots are displayed, and output files are written.
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
clear; clc; close all;
disp('Overdrive (U/UCJ) Series')

P1 = oneatm; T1 = 300; eq = 1.;
disp('Initial Conditions')
disp(['Equivalence Ratio = ',num2str(eq),' Temperature = ',num2str(T1),' K, Pressure = ',num2str(P1),' Pa']);
mech = 'Mevel2017.cti';   
gas = Solution(mech); 
gas1 = Solution(mech);
nsp = nSpecies(gas);
phi = [];

% plots: 1 = make plots, otherwise no plots
plots = 1;

% find Hydrogen nitrogen, and oxygen indices
ih2 = speciesIndex(gas,'H2');
io2  = speciesIndex(gas,'O2');
in2  = speciesIndex(gas,'N2');
x = zeros(nsp, 1);
x(ih2,1) = eq*2.0;
x(io2,1) = 1.0;
x(in2,1) = 3.76;

[cj_speed] = CJspeed(P1, T1, x, mech);   

npoints=10;
disp(['For ', num2str(npoints), ' values of U/U_cj'])
for i = 1:npoints
   %%%Constant Volume Explosion Data%%%
   % FIND POST SHOCK STATE FOR GIVEN SPEED
   overdrive(i) = (1.0 +0.6/npoints*(i-1)); %Overdrive = U/Ucj
   disp([' ', num2str(i), ': overdrive = ', num2str(overdrive(i))])
   set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',x);
   
   %%%Constant Volume Explosion Data%%%
   % FIND POST SHOCK STATE FOR GIVEN SPEED
   [gas] = PostShock_fr(cj_speed*overdrive(i), P1, T1, x, mech);
   % SOLVE CONSTANT VOLUME EXPLOSION ODES
   [CVout] = cvsolve(gas);
   exo_time_CV(i) = CVout.exo_time;
   ind_time_CV(i) = CVout.ind_time;
   
   %%%%%ZND Detonation Data%%%%%
   % FIND POST SHOCK STATE FOR GIVEN SPEED
   set(gas1, 'T', T1, 'P', P1, 'X', x);
   [gas] = PostShock_fr(cj_speed*overdrive(i), P1, T1, x, mech);
   Ts(i) = temperature(gas); %frozen shock temperature   
   Ps(i) = pressure(gas); %frozen shock pressure
   % SOLVE ZND DETONATION ODES
   [ZNDout] = zndsolve(gas,gas1,cj_speed*overdrive(i),'advanced_output',true);
   exo_time_ZND(i) = ZNDout.exo_time_ZND;
   exo_len_ZND(i) = ZNDout.exo_len_ZND;
   ind_time_ZND(i) = ZNDout.ind_time_ZND;
   ind_len_ZND(i) = ZNDout.ind_len_ZND;
   tsteps = size(ZNDout.T);
   Tf_ZND(i) = ZNDout.T(tsteps(1));
   
   %%Calculate CJstate Properties%%%
   [gas] = PostShock_eq(cj_speed*overdrive(i),P1, T1,x, mech);
   T2(i) = temperature(gas);
   P2(i) = pressure(gas);
   rho2(i) = density(gas);

   %Approximate the effective activation energy using finite differences
    Ta = Ts(i)*(1.02);
    set(gas, 'T', Ta, 'P', Ps(i), 'X', x);
    [CVout1] = cvsolve(gas);
    taua = CVout1.ind_time;
    Tb = Ts(i)*(0.98);
    set(gas, 'T', Tb, 'P', Ps(i), 'X', x);
    [CVout2] = cvsolve(gas);    
    taub = CVout2.ind_time;
    % Approximate effective activation energy for CV explosion    
    if(taua==0 || taub==0)
        theta_effective_CV(i) = 0;         
    else
        theta_effective_CV(i) = 1/Ts(i)*((log(taua)-log(taub))/((1/Ta)-(1/Tb))); 
    end
      
    [gas] = PostShock_fr(cj_speed*overdrive(i)*1.02,P1, T1,x, mech);
    Ta= temperature(gas);
    [ZNDout1] = zndsolve(gas,gas1,cj_speed*overdrive(i)*1.02,'advanced_output',true);
    ind_len_a = ZNDout1.ind_len_ZND;
    [gas] = PostShock_fr(cj_speed*overdrive(i)*0.98,P1, T1,x, mech);
    Tb = temperature(gas);
    [ZNDout2] = zndsolve(gas,gas1,cj_speed*overdrive(i)*0.98,'advanced_output',true);    
    ind_len_b = ZNDout2.ind_len_ZND;
    % Approximate effective activation energy for ZND Detonation
    if(ind_len_a==0 || ind_len_b==0)
        theta_effective_ZND(i) = 0;         
    else
        theta_effective_ZND(i) = 1/Ts(i)*((log(ind_len_a)-log(ind_len_b))/((1/Ta)-(1/Tb))); 
    end
end

if(plots==1)
    % make plots
    fontsize=12;
    figure;
    plot(overdrive,T2(:),'k');
    xlabel('Overdrive = U/U_{CJ}','FontSize',fontsize);
    ylabel('Temperature (K)','FontSize',fontsize);
    title('Post CJ State Temperature','FontSize',fontsize);
    set(gca,'FontSize',12,'LineWidth',2);

    figure;
    plot(overdrive,P2(:),'k');
    xlabel('Overdrive = U/U_{CJ}','FontSize',fontsize);
    ylabel('Pressure (Pa)','FontSize',fontsize);
    title('Post CJ State Pressure','FontSize',fontsize);
    set(gca,'FontSize',12,'LineWidth',2);

    figure;
    plot(overdrive,rho2(:),'k');
    xlabel('Overdrive = U/U_{CJ}','FontSize',fontsize);
    ylabel('Density (kg/m^3)','FontSize',fontsize);
    title('Post CJ State Density','FontSize',fontsize);
    set(gca,'FontSize',12,'LineWidth',2);

    %%% Plots for the Induction Zone (times and lengths)
    figure;
    plot(overdrive,ind_time_CV(:),'k');
    xlabel('Overdrive = U/U_{CJ}','FontSize',fontsize);
    ylabel('\tau_{CV_i} (s)','FontSize',fontsize);
    title('Induction time for CV explosion','FontSize',fontsize);
    set(gca,'FontSize',12,'LineWidth',2);

    figure;
    plot(overdrive,ind_time_ZND(:),'k');
    xlabel('Overdrive = U/U_{CJ}','FontSize',fontsize);
    ylabel('\tau_{ZND_i} (s)','FontSize',fontsize);
    title('Induction time for ZND detonation','FontSize',fontsize);
    set(gca,'FontSize',12,'LineWidth',2);

    figure;
    plot(overdrive,ind_len_ZND(:),'k');
    xlabel('Overdrive = U/U_{CJ}','FontSize',fontsize);
    ylabel('\Delta_{ZND_i} (m)','FontSize',fontsize);
    title('Induction length for ZND detonation','FontSize',fontsize);
    set(gca,'FontSize',12,'LineWidth',2);

    %%% Plots for the Exothermic Zone (times and lengths)
    figure;
    plot(overdrive,exo_time_CV(:),'k');
    xlabel('Overdrive = U/U_{CJ}','FontSize',fontsize);
    ylabel('tau_{CV_e} (s)','FontSize',fontsize);
    title('Exothermic time for CV explosion','FontSize',fontsize);
    set(gca,'FontSize',12,'LineWidth',2);

    figure;
    plot(overdrive,exo_time_ZND(:),'k');
    xlabel('Overdrive = U/U_{CJ}','FontSize',fontsize);
    ylabel('tau_{ZND_e} (s)','FontSize',fontsize);
    title('Exothermic time for ZND detonation','FontSize',fontsize);
    set(gca,'FontSize',12,'LineWidth',2);

    figure;
    plot(overdrive,exo_len_ZND(:),'k');
    xlabel('Overdrive = U/U_{CJ}','FontSize',fontsize);
    ylabel('Delta_{ZND_e} (m)','FontSize',fontsize);
    title('Exothermic length for ZND detonation','FontSize',fontsize);
    set(gca,'FontSize',12,'LineWidth',2);

    %Ts and Tf for ZND detonation
    figure;
    plot(overdrive,Ts(:),'k');
    xlabel('Overdrive = U/U_{CJ}','FontSize',fontsize);
    ylabel('T_s (K)','FontSize',fontsize);
    title('Frozen shock Temperature for ZND detonation','FontSize',fontsize);
    set(gca,'FontSize',12,'LineWidth',2);

    figure;
    plot(overdrive,Tf_ZND(:),'k');
    xlabel('Overdrive = U/U_{CJ}','FontSize',fontsize);
    ylabel('T_f (K)','FontSize',fontsize);
    title('Final Temperature for ZND detonation','FontSize',fontsize);
    set(gca,'FontSize',12,'LineWidth',2);

    %Approximation of the effective activation energy for CV explosion
    figure;
    plot(overdrive, theta_effective_CV);
    xlabel('Overdrive = U/U_{CJ}','FontSize',fontsize);
    ylabel('Theta_{CV} (J)','FontSize',fontsize);
    title('Effective activation energy for CV explosion','FontSize',fontsize);
    set(gca,'FontSize',12,'LineWidth',2);

    %Approximation of the effective activation energy for ZND detonation
    figure;
    plot(overdrive, theta_effective_ZND);
    xlabel('Overdrive = U/U_{CJ}','FontSize',fontsize);
    ylabel('Theta_{ZND} (J)','FontSize',fontsize);
    title('Effective activation energy for ZND detonation','FontSize',fontsize);
    set(gca,'FontSize',12,'LineWidth',2)
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% CREATE OUTPUT TEXT FILE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fn = 'Overdrive_Series.txt';
d = date;
P = P1/oneatm;
	
fid = fopen(fn, 'w');
fprintf(fid, '# ZND DETONATION STRUCTURE CALCULATION AND CONSTANT VOLUME EXPLOSION\n');
fprintf(fid, '# CALCULATION RUN ON %s\n\n', d);

fprintf(fid, '# INITIAL CONDITIONS\n');
fprintf(fid, '# TEMPERATURE (K) %4.1f\n', T1);
fprintf(fid, '# PRESSURE (Pa) %2.1f\n', P1);
fprintf(fid, '# DENSITY (KG/M^3) %1.4e\n', density(gas1));
q = 'H2:2 O2:1 N2:3.76';    
fprintf(fid, ['# SPECIES MOLE FRACTIONS: ' q '\n']);
	
%	fprintf(fid, '# REACTION ZONE STRUCTURE\n\n');
	
fprintf(fid, '# THE OUTPUT DATA COLUMNS ARE:\n');
fprintf(fid, 'Variables = "Overdrive U/U_CJ", "Temperature state 2", "Pressure state 2", "density state 2", "Temperature Post Shock", "Pressure Post Shock", "Induction time CV", "Exothermic time CV", "Effective Activation Energy CV ",  "Effective Activation Energy ZND ", "Induction  time ZND", "Induction length ZND", "Exothermic time ZND", "Exothermic length ZND", "Final Temperature ZND"\n');
	
z = [overdrive; T2; P2; rho2; Ts; Ps; ind_time_CV;  exo_time_CV; theta_effective_CV; theta_effective_ZND; ind_time_ZND; ind_len_ZND; exo_time_ZND; exo_len_ZND; Tf_ZND];
fprintf(fid, '%14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e  \t %14.5e  \t %14.5e  \t %14.5e  \t %14.5e  \t %14.5e \t %14.5e  \t %14.5e \n', z);
	
fclose(fid);
