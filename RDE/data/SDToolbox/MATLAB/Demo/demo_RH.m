% Shock and Detonation Toolbox Demo Program
% 
% Creates arrays for Rayleigh Line with specified shock speed, Reactant, and Product 
% Hugoniot Curves for  H2-air mixture.  Options to create output file and plots.
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
disp('demo_RH')
%% User editable data
% Initial state specification
% P1 = Initial Pressure
% T1 = Initial Temperature
% q = Initial Composition MUST use capital letters - STRING
% mech = Mechanism File name in CTI format - STRING 
P1 = 100000; T1 = 300; 
q = 'H2:2 O2:1 N2:3.76';    
mech = 'Mevel2017.cti';   
% fname = Mixture name used in plot titles and output file name
% od = Overdrive factor used for Rayleigh Line, shock speed is od*cj_speed
% gbound = Overdrive for Gamma Bound -- mulitiplied by CJ speed to find the maximum gamma 
fname = 'h2air';
od = 1.; 
gbound = 1.4;
%
% outfile = Enter a non-zero number to generate a .txt file
%           OR enter '0' (the number zero) to bypass generating an output file
outfile = 1;
% Plot numbers for the two optional plots (choose ZERO for NO plot), if
% both are non-zero, they should be distinct values
% gplot =   Enter a non-zero number to generate a plot of Rayleigh Line,
%           Reactant Hugoniot, and Product Hugoniot with
%           minimum density limit based on ideal gas value (g-1)/(g+1)
% nogamma = Enter a non-zero number to get a plain plot of Rayleigh Line,
%           Reactant Hugoniot, and Product Hugoniot
nogamma = 1; %figure number for plot without minimum density limit
gplot = 0; % figure number for plot  with minimum density limit displayed
%%
%initial state
gas1 = Solution(mech);
set(gas1, 'T', T1, 'P', P1, 'X', q);
h1 = enthalpy_mass(gas1);
r1 = density(gas1);
v1 = 1/density(gas1);
%Compute CJ point
[cj_speed] = CJspeed(P1, T1, q, mech);
[gas] = PostShock_eq(cj_speed,P1, T1, q, mech);
vcj = 1/density(gas);
Pcj = pressure(gas)/oneatm;
% set shock speed
U1 = od*cj_speed;
%%
%FIND POSTSHOCK SPECIFIC VOLUME FOR specified U1
[gas] = PostShock_fr(U1, P1, T1, q, mech);
v_ps = 1/density(gas);

%FIND GAMMA CORRESPONDING TO gbound*U_CJ
[gas] = PostShock_fr(gbound*cj_speed, P1, T1, q, mech);
g = cp_mass(gas)/cv_mass(gas);


%% RAYLEIGH LINE, SJUMP MIN & MAX, GAMMA CONSTRAINT 

i = 0;
for v2=0.1*vcj:0.01*vcj:2*vcj
    i = i + 1;
    vR(i) = v2; 
   
    PRpa = (P1 - r1^2*U1^2*(v2 - v1));
    PR(i) = PRpa/oneatm;                    %Pressure in atmospheres
    
    min_line(i) = 1/40;                     %Sjump minimum
    max_line(i) = 1/1.005;                  %Sjump maximum
    gamma(i) = (g-1)*r1/(g+1);              %gamma constraint
    
end

disp('Rayleigh Line Array Created')

%%%%%%%%%%%%%%%%%%%%%%%
%% REACTANT HUGONIOT %%
%%%%%%%%%%%%%%%%%%%%%%%
set(gas, 'T', T1, 'P', P1, 'X', q);    
Ta = temperature(gas);
va = 1/density(gas);

PH1(1) = pressure(gas)/oneatm;
vH1(1) = va;

i = 0;
vb = va;
while(vb >= v_ps)
    i = i + 1;

    %Always starts at new volume and previous temperature to find next state
    array = [vb;h1;P1;v1];
    options=optimset('Display','off');
    [x,fval] = fsolve(@hug_fr,Ta,options,gas,array);
    
    set(gas, 'T', x, 'Density', 1/vb);
    PH1(i+1) = pressure(gas)/oneatm;
    vH1(i+1) = vb;

    vb = va - i*0.01;
end

disp('Reactant Hugoniot Array Created')

[gas] = PostShock_fr(U1, P1, T1, q, mech);
vsj = 1/density(gas);
Psj = pressure(gas)/oneatm;

%%%%%%%%%%%%%%%%%%%%%%
%% PRODUCT HUGONIOT %%
%%%%%%%%%%%%%%%%%%%%%%

%GET FIRST POINT ON PRODUCT HUGOINIOT - CV Combustion
set(gas, 'T', T1, 'P', P1, 'X', q);    
equilibrate(gas, 'UV');
Ta = temperature(gas);
va = 1/density(gas);

PH2(1) = pressure(gas)/oneatm;
vH2(1) = va;

i = 0;
vb = va;
while(vb > 0.35*vcj)
    i = i + 1;

    %Always starts at new volume and previous temperature to find next state
    array = [vb;h1;P1;v1];
    options=optimset('Display','off');
    [x,fval] = fsolve(@hug_eq,Ta,options,gas,array);
    
    set(gas, 'T', x, 'Density', 1/vb);
    PH2(i+1) = pressure(gas)/oneatm;
    vH2(i+1) = vb;

    vb = va - i*0.01;
end
  
disp('Product Hugoniot Array Created')

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% CREATE OUTPUT TEXT FILE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if(outfile == 0)
    disp('No output files created')
else
    fn = [fname, '_', num2str(U1), '_RH.txt'];
    d = date;
	P = P1/oneatm;
	
	fid = fopen(fn, 'w');
	fprintf(fid, '# RAYLEIGH LINE, SJUMP MAX & MIN, AND GAMMA CONSTRAINT FOR GIVEN SHOCK SPEED\n');
	fprintf(fid, '# CALCULATION RUN ON %s\n\n', d);
	
	fprintf(fid, '# INITIAL CONDITIONS\n');
	fprintf(fid, '# TEMPERATURE (K) %4.1f\n', T1);
	fprintf(fid, '# PRESSURE (ATM) %2.1f\n', P1);
	fprintf(fid, '# DENSITY (KG/M^3) %1.4e\n', r1);
	fprintf(fid, ['# SPECIES MOLE FRACTIONS: ' q '\n']);
	
	fprintf(fid, '# SHOCK SPEED (M/S) %5.2f\n\n', U1);
	
	fprintf(fid, '# THE OUTPUT DATA COLUMNS ARE:\n');
	fprintf(fid, 'Variables = "Volume (R)", "Pressure (R)", "Max Line", "Min Line", "Gamma Line"\n');
	
	y = [vR; PR; min_line; max_line; gamma];
	fprintf(fid, '%1.4e \t %1.4e \t %1.4e \t %1.4e \t %1.4e\n', y);
	fclose(fid);
	
	fn = [fname, '_ReacHug.txt'];
	fid = fopen(fn, 'w');
	fprintf(fid, '# REACTANT HUGONIOT FOR GIVEN MIXTURE\n');
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
	
	fn = [fname, '_ProdHug.txt'];
	fid = fopen(fn, 'w');
	fprintf(fid, '# PRODUCT HUGONIOT FOR GIVEN MIXTURE\n');
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
	
	fn = [fname, '_', num2str(U1), '_RH_points.txt'];
	fid = fopen(fn, 'w');
	fprintf(fid, '# SPECIFIC POINTS FOR GIVEN SHOCK SPEED\n');
	fprintf(fid, '# CALCULATION RUN ON %s\n\n', d);
	
	fprintf(fid, '# INITIAL CONDITIONS\n');
	fprintf(fid, '# TEMPERATURE (K) %4.1f\n', T1);
	fprintf(fid, '# PRESSURE (ATM) %2.1f\n', P1);
	fprintf(fid, '# DENSITY (KG/M^3) %1.4e\n', r1);
	fprintf(fid, ['# SPECIES MOLE FRACTIONS: ' q '\n']);
	
	fprintf(fid, '# SHOCK SPEED (M/S) %5.2f\n\n', U1);
	
	fprintf(fid, '# THE OUTPUT DATA COLUMNS ARE:\n');
	fprintf(fid, 'Variables = "Volume (H)", "Pressure (H)"\n');
	
	y = [[v1 vsj vcj]; [P1/oneatm Psj Pcj]];
	fprintf(fid, '%1.4e \t %1.4e\n', y);
	fclose(fid);
end

maxP = max(PR) + 0.1*max(PR);
minP = 0;
maxV = max([vR,vH1,vH2]) + 0.1*max([vR,vH1,vH2]);
minV = min([vR,vH1,vH2]) - 0.1*max([vR,vH1,vH2]);

if(nogamma == 0)
    disp('Plot with out gamma constraint not requested')
else
	figure(nogamma); clf;
	plot(vR(:),PR(:),'k:','LineWidth',2);  
	hold on;
	plot(vH1(:),PH1(:),'b','LineWidth',2)
	hold on;
	plot(vcj,Pcj,'ks','LineWidth',2)
    hold on;
	plot(vH2,PH2,'r','LineWidth',2)
	hold off;
	axis([minV maxV minP maxP]);
	title(['Rayliegh Line & Hugoniot Curves (',fname,' U = ',num2str(U1,5),')'],'FontSize', 14);
	xlabel('specific volume','FontSize', 14);
	ylabel('pressure','FontSize', 14);
	set(gca,'FontSize',12,'LineWidth',2);
    legend('Rayleigh','Fr Hugoniot','CJ','Eq Hugoniot')
end

if(gplot == 0)
    disp('Plot with density constraints not requested')
else
	figure(gplot); clf;
	plot(vR(:),PR(:),'k:','LineWidth',2);  
	hold on;
	plot(vH1(:),PH1(:),'r','LineWidth',2)
	hold on;
	plot(vcj,Pcj,'ks','LineWidth',2)
    hold on;
   	plot(vsj,Psj,'b*','LineWidth',2)
    hold on;
	plot(vH2,PH2,'b','LineWidth',2)
    hold on;
%     plot(max_line(:),PR(:),'k','LineWidth',3);
%     hold on;
%     plot(min_line(:),PR(:),'k','LineWidth',3)
%     hold on;
    plot(gamma(:),PR(:),'k:','LineWidth',3)
    hold off;
	axis([minV maxV minP maxP]);
	title(['Rayliegh Line & Hugoniot Curves with Constraints (',fname,' U = ',num2str(U1,5),')'],'FontSize', 14);
	xlabel('specific volume','FontSize', 14);
	ylabel('pressure','FontSize', 14);
	set(gca,'FontSize',12,'LineWidth',2);
end
