% Shock and Detonation Toolbox Demo Program
% 
% Computes detonation and reflected shock wave pressure for overdriven waves.   
% Varies density of initial state and detonation wave speed. Creates an output file.
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
disp('Precompression Detonation Series')

% set initial state, composition, and gas object
P0 = 100000; T0 = 300; 
q = 'H2:0.31 N2O:0.69';    
mech = 'Mevel2015.cti';
gas0 = Solution(mech);
gas1 = Solution(mech);
gas3 = Solution(mech);
set(gas0,'Temperature',T0,'Pressure',P0,'MoleFractions',q);

% Find compressed state (isentropic)
rho0 = density(gas0);
s0 = entropy_mass(gas0);
x0 = moleFractions(gas0);

% open output file
% print output file
fn = ['precompressed_detonation_reflection.txt'];
d = date;
fid = fopen(fn, 'w');
fprintf(fid, '# Reflection of Overdriven Detonation\n');
fprintf(fid, '# Calculation run on %s\n', d );
fprintf(fid, ['# Initial species mole fractions: ' q '\n']);
fprintf(fid, ['# Reaction mechanism: ' mech '\n\n']);
rho0 = 2*rho0;
% loop through initial state with isentropic compression
for rho1=(rho0:.5*rho0:4*rho0)
    set(gas1,'Density',rho1,'Entropy',s0,'MoleFractions',x0); 
    P1 = pressure(gas1);
    T1 = temperature(gas1);

    disp(['Density ',num2str(rho1),' (kg/m^3)']);
    fprintf(fid, '# Initial conditions\n');
    fprintf(fid, '# Temperature (K) %4.1f\n', T1);
    fprintf(fid, '# Pressure (Pa) %2.1f\n', P1);
    fprintf(fid, '# Density (kg/m^3) %1.4e\n', rho1);

    % Find CJ speed
    [cj_speed] = CJspeed(P1, T1, q, mech);
    disp(['CJspeed ',num2str(cj_speed),' (m/s)']);
    [gas] = PostShock_eq(cj_speed,P1, T1, q, mech);
    P2 = pressure(gas);
 
    % Evaluate overdriven detonations and reflected shocks
    i=1;
    for f = (1:0.05:1.50)
        u_shock = f*cj_speed;
        speed(i) = u_shock;
        disp(['   Detonation Speed ',num2str(speed(i)),' (m/s)']);
        [gas] = PostShock_eq(u_shock,P1, T1, q, mech);
        % Evaluate properties of gas object 
        vs(i) = 1./density(gas);
        ps(i) = pressure(gas);
        [p3,UR,gas3] = reflected_eq(gas1,gas,gas3,u_shock);
        pr(i) = p3;
        vr(i) = 1./density(gas3);
        i = i+1;
    end

    disp('-------------------------------------------');        
    fprintf(fid, 'Variables = "detonation speed (m/s)", "detonation pressure (Pa)", "detonation volume (m3/kg)", "reflected shock pressure (Pa)", "reflected shock volume (m3/kg)"\n');
    z = [speed; ps; vs; pr; vr];
    fprintf(fid, ' %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \n', z);
end
fclose(fid);
