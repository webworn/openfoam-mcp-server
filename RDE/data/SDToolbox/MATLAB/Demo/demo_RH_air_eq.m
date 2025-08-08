% Shock and Detonation Toolbox Demo Program
% 
% Creates arrays for Rayleigh Line with specified shock speed in air, frozen and 
% equilibrium Hugoniot curves. Options to create output file and plot.
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
disp('demo_RH_air_eq')

%Specify initial state
P1 = 100000; T1 = 300; 
q = 'O2:0.2095 N2:0.7808 CO2:0.0004 Ar:0.0093';    
mech = 'airNASA9ions.cti';   
fname = 'air';
%Specify shock speed  (must be greater than sound speed of initial state
U1 = 3500.;
% Flag for creating an output file (choose ZERO for NO output file)
outfile = 1;
% flag for creating a plot (= 0 no plot, = 1 plot)
plotfig = 1;
% EDIT VALUES ABOVE THIS LINE
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
gas1 = Solution(mech);
set(gas1, 'T', T1, 'P', P1, 'X', q);

h1 = enthalpy_mass(gas1);
r1 = density(gas1);
v1 = 1/density(gas1);
 
%FIND POSTSHOCK SPECIFIC VOLUME FOR U1
[gas] = PostShock_eq(U1, P1, T1, q, mech);
v_ps = 1/density(gas);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% RAYLEIGH LINE %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
i = 0;
for v2=0.9*v_ps:0.01*v1:0.999*v1
    i = i + 1;
    vR(i) = v2; 
    PRpa = (P1 - r1^2*U1^2*(v2 - v1));
    PR(i) = PRpa/oneatm;                    %Pressure in atmospheres    
end
disp('Rayleigh Line Array Created')
%%%%%%%%%%%%%%%%%%%%%%%
%% EQUILIBRIUM HUGONIOT %%
%%%%%%%%%%%%%%%%%%%%%%%
i = 0;
for U=0.2*U1:0.05*U1:1.1*U1
    i = i + 1;
    [gas] = PostShock_eq(U, P1, T1, q, mech);
    PH1(i) = pressure(gas)/oneatm;
    vH1(i) = 1./density(gas);
end

disp('Equilibrium Hugoniot Array Created')

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% FROZEN HUGONIOT %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
i = 0;
for U=0.01*U1:0.05*U1:1.1*U1
    i = i + 1;
    [gas] = PostShock_fr(U, P1, T1, q, mech);
    PH2(i) = pressure(gas)/oneatm;
    vH2(i) = 1./density(gas);
end

disp('Frozen Hugoniot Array Created')
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% CREATE OUTPUT TEXT FILE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if(outfile == 0)
    disp('No output files created ')
else
    fn = [fname, '_', num2str(U1), '_air_RH.txt'];
    d = date;
	P = P1/oneatm;
	
	fid = fopen(fn, 'w');
	fprintf(fid, '# RAYLEIGH LINE FOR GIVEN SHOCK SPEED\n');
	fprintf(fid, '# CALCULATION RUN ON %s\n\n', d);
	
	fprintf(fid, '# INITIAL CONDITIONS\n');
	fprintf(fid, '# TEMPERATURE (K) %4.1f\n', T1);
	fprintf(fid, '# PRESSURE (ATM) %2.1f\n', P1);
	fprintf(fid, '# DENSITY (KG/M^3) %1.4e\n', r1);
	fprintf(fid, ['# SPECIES MOLE FRACTIONS: ' q '\n']);
	
	fprintf(fid, '# SHOCK SPEED (M/S) %5.2f\n\n', U1);
	
	fprintf(fid, '# THE OUTPUT DATA COLUMNS ARE:\n');
	fprintf(fid, 'Variables = "Volume (R)", "Pressure (R)"\n');
	
	y = [vR; PR];
	fprintf(fid, '%1.4e \t %1.4e \n', y);
	fclose(fid);
	
	fn = [fname, '_', num2str(U1),'_air_EqHug.txt'];
	fid = fopen(fn, 'w');
	fprintf(fid, '# EQUILIBRIUM HUGONIOT FOR GIVEN MIXTURE\n');
	fprintf(fid, '# CALCULATION RUN ON %s\n\n', d);
	
	fprintf(fid, '# INITIAL CONDITIONS\n');
	fprintf(fid, '# TEMPERATURE (K) %4.1f\n', T1);
	fprintf(fid, '# PRESSURE (ATM) %2.1f\n', P1);
	fprintf(fid, '# DENSITY (KG/M^3) %1.4e\n', r1);
	fprintf(fid, ['# SPECIES MOLE FRACTIONS: ' q '\n']);
	
	fprintf(fid, '# THE OUTPUT DATA COLUMNS ARE:\n');
	fprintf(fid, 'Variables = "Volume (H1)", "Pressure (H1)"\n');
	
	y = [vH1; PH1];
	fprintf(fid, '%1.4e \t %1.4e\n', y);
	fclose(fid);
    
    fn = [fname, '_', num2str(U1),'_air_FrHug.txt'];
	fid = fopen(fn, 'w');
	fprintf(fid, '# FROZEN HUGONIOT FOR GIVEN MIXTURE\n');
	fprintf(fid, '# CALCULATION RUN ON %s\n\n', d);
	
	fprintf(fid, '# INITIAL CONDITIONS\n');
	fprintf(fid, '# TEMPERATURE (K) %4.1f\n', T1);
	fprintf(fid, '# PRESSURE (ATM) %2.1f\n', P1);
	fprintf(fid, '# DENSITY (KG/M^3) %1.4e\n', r1);
	fprintf(fid, ['# SPECIES MOLE FRACTIONS: ' q '\n']);
	
	fprintf(fid, '# THE OUTPUT DATA COLUMNS ARE:\n');
	fprintf(fid, 'Variables = "Volume (H2)", "Pressure (H2)"\n');
	
	y = [vH2; PH2];
	fprintf(fid, '%1.4e \t %1.4e\n', y);
	fclose(fid);

end
%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% CREATE PLOT
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if(plotfig == 0)
    disp('Plot not requested')
else
    maxP = max(PR) + 0.1*max(PR);
    minP = 0;
    maxV = max([vR,vH1]) + 0.1*max([vR,vH1]);
    minV = min([vR,vH1]) - 0.1*max([vR,vH1]);
	figure(plotfig); clf;
	plot(vR(:),PR(:),'k:','LineWidth',2);  
	hold on;
	plot(vH1(:),PH1(:),'r','LineWidth',2);
    hold on;
	plot(vH2(:),PH2(:),'b','LineWidth',2);
	hold off;
	axis([minV maxV minP maxP]);
	xlabel('specific volume (m3/kg)','FontSize', 14);
	ylabel('pressure (atm)','FontSize', 14);
	set(gca,'FontSize',12,'LineWidth',2);
    legend('Rayleigh','Eq Hugoniot','Fr Hugoniot');
end

