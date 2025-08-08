% Shock and Detonation Toolbox Demo Program
% 
% Computes detonation and reflected shock wave pressure for overdriven
% waves. Both the post-initial-shock and the post-reflected-shock states
% are equilibrium states.  Creates output file.
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
disp('demo_overdriven')

%% set initial state, composition, and gas object
P1 = 100000; T1 = 300; 
q = 'H2:0.31 N2O:0.69';    
mech = 'Mevel2015.cti';
gas1 = Solution(mech);
set(gas1,'Temperature',T1,'Pressure',P1,'MoleFractions',q);

%% Find CJ speed
[cj_speed] = CJspeed(P1, T1, q, mech);
[gas] = PostShock_eq(cj_speed,P1, T1, q, mech);
disp(['CJ computation for ',mech,' with composition ',q])
P2 = pressure(gas);
disp(['   CJ speed ',num2str(cj_speed),' (m/s)']);
disp(['   CJ pressure ',num2str(P2),' (Pa)']);

%% Evaluate gas state
i=1;
for f = (1:0.05:1.50)
    u_shock = f*cj_speed;
    speed(i) = u_shock;
    [gas] = PostShock_eq(u_shock,P1, T1, q, mech);
    % Evaluate properties of gas object 
    vs(i) = 1./density(gas);
    ps(i) = pressure(gas);
    disp(['   U/U_cj ',num2str(f)]);
     % Print out
    gas3 = Solution(mech);
    [p3,UR,gas3] = reflected_eq(gas1,gas,gas3,u_shock);
    pr(i) = p3;
    vr(i) = 1./density(gas3);
    i = i+1;
end

%% print output file
fn = ['overdriven_reflection.txt'];
d = datestr(now,0);
fid = fopen(fn, 'w');
fprintf(fid, '# Reflection of Overdriven Detonation\n');
fprintf(fid, '# Calculation run on %s\n', d );
fprintf(fid, '# Initial conditions\n');
fprintf(fid, '# Temperature (K) %4.1f\n', T1);
fprintf(fid, '# Pressure (Pa) %2.1f\n', P1);
fprintf(fid, '# Density (kg/m^3) %1.4e\n', density(gas1));
fprintf(fid, ['# Initial species mole fractions: ' q '\n']);
fprintf(fid, ['# Reaction mechanism: ' mech '\n\n']);
fprintf(fid, 'Variables = "detonation speed (m/s)", "detonation pressure (Pa)", "detonation volume (m3/kg)", "reflected shock pressure (Pa)", "reflected shock volume (m3/kg)"\n');
z = [speed; ps; vs; pr; vr];
fprintf(fid, '%14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \n', z);
fclose(fid);
