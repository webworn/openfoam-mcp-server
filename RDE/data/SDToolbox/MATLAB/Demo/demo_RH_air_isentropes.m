% Shock and Detonation Toolbox Demo Program
% 
% Creates arrays for frozen Hugoniot curve for shock wave in air, Rayleigh Line 
% with specified shock speed, and four representative isentropes.  
% Options to create plot and output file.
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
disp('demo_RH_air_isentropes')
%% input parameters
P1 = 100000; T1 = 300; 
q = 'O2:0.2095 N2:0.7808 CO2:0.0004 Ar:0.0093';    
mech = 'airNASA9ions.cti';   
fname = 'air';
            
%Shock speed for Rayleigh line, must be greater than initial state sound
%speed
U1 = 1000.;

% Flag for creating an output file (choose ZERO for NO output file)
outfile = 1;

% flag for creating a plot (= 0 no plot, = 1 plot)
plotfig = 1;
%% Initial state
gas1 = Solution(mech);
set(gas1, 'T', T1, 'P', P1, 'X', q);

h1 = enthalpy_mass(gas1);
r1 = density(gas1);
v1 = 1/density(gas1);
s1 = entropy_mass(gas1);
 
%Postshock state
[gas] = PostShock_fr(U1, P1, T1, q, mech);
v_ps = 1/density(gas);

%% RAYLEIGH LINE 
i = 0;
for v2=0.9*v_ps:0.01*v1:0.999*v1
    i = i + 1;
    vR(i) = v2; 
   
    PRpa = (P1 - r1^2*U1^2*(v2 - v1));
    PR(i) = PRpa/oneatm;                    %Pressure in atmospheres    
end

disp('Rayleigh Line Array Created')

%% REACTANT (frozen) HUGONIOT 
set(gas, 'T', T1, 'P', P1, 'X', q);    
Ta = temperature(gas);
va = 1/density(gas);

PH1(1) = pressure(gas)/oneatm;
vH1(1) = va;

i = 0;
vb = va;
while(vb >= 0.9*v_ps)
    i = i + 1;
    vb = va - i*0.01;
    %Always starts at new volume and previous temperature to find next state
    array = [vb;h1;P1;v1];
    options=optimset('Display','off');
    [x,fval] = fsolve(@hug_fr,Ta,options,gas,array);
    
    set(gas, 'T', x, 'Density', 1/vb);
    PH1(i+1) = pressure(gas)/oneatm;
    vH1(i+1) = vb;
end

disp('Reactant Hugoniot Array Created')

[gas] = PostShock_fr(U1, P1, T1, q, mech);
vsj = 1/density(gas);
Psj = pressure(gas)/oneatm;
%% Compute four frozen isentropes 
% one passing through initial state
% one passing through postshock state
% two in between
Ph = pressure(gas);
Sh = entropy_mass(gas);
Th = temperature(gas);
qh = moleFractions(gas);
mult = 0.95;
imax = log(P1/Ph)/log(mult);
deltas = (Sh - s1)/3.;
s = s1;
for j=1:1:4
    pp = Ph*1.15;
    for i = 1:1:imax
        set(gas,'Pressure',pp,'Entropy',s,'MoleFractions',qh);
        Ps(i,j) = pressure(gas)/oneatm;
        vs(i,j) = 1/density(gas);
%        if vs(i,j) > v1, break, end
        pp = pp*mult;
    end
    s = s + deltas;
    disp(['Isentrope Array Created (s = ' num2str(s) ')'])
end

%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% CREATE OUTPUT TEXT FILE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if(outfile == 0)
    disp('No output files created ')
else
    fn = [fname, '_', num2str(U1), '_RH_air_isentrope.txt'];
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
	fprintf(fid, 'Variables = Volume (R)', 'Pressure (R)'\n');
	
	y = [vR; PR];
	fprintf(fid, '%1.4e \t %1.4e \n', y);
	fclose(fid);
	
	fn = [fname, '_RH_air_ReacHug.txt'];
	fid = fopen(fn, 'w');
	fprintf(fid, '# REACTANT HUGONIOT FOR GIVEN MIXTURE\n');
	fprintf(fid, '# CALCULATION RUN ON %s\n\n', d);
	
	fprintf(fid, '# INITIAL CONDITIONS\n');
	fprintf(fid, '# TEMPERATURE (K) %4.1f\n', T1);
	fprintf(fid, '# PRESSURE (ATM) %2.1f\n', P1);
	fprintf(fid, '# DENSITY (KG/M^3) %1.4e\n', r1);
	fprintf(fid, ['# SPECIES MOLE FRACTIONS: ' q '\n']);
	
	fprintf(fid, '# THE OUTPUT DATA COLUMNS ARE:\n');
	fprintf(fid, 'Variables = Volume (H1), Pressure (H1)\n');
	
	y = [vH1; PH1];
	fprintf(fid, '%1.4e \t %1.4e\n', y);
	fclose(fid);
end
%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% CREATE PLOT
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if(plotfig == 0)
    disp('Plot not requested ')
else
    maxP = max(PR) + 0.1*max(PR);
    minP = 0;
    maxV = max([vR,vH1]) + 0.1*max([vR,vH1]);
    minV = min([vR,vH1]) - 0.1*max([vR,vH1]);
	figure(plotfig); clf;
	plot(vR(:),PR(:),'k:','LineWidth',2);  
	hold on;
	plot(vH1(:),PH1(:),'r','LineWidth',2);
    for j=1:1:4
    hold on;
	plot(vs(:,j),Ps(:,j),'b','LineWidth',2);
    end
	hold off;
	axis([minV maxV minP maxP]);
	xlabel('specific volume (m^3/kg)','FontSize', 14);
	ylabel('pressure (atm)','FontSize', 14);
	set(gca,'FontSize',12,'LineWidth',2);
    legend('Rayleigh','Hugoniot','Isentropes');
end

