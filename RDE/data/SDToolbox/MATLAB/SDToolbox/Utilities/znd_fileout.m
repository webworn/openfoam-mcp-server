function znd_fileout(fname,znd_output)
% Creates 2 formatted text files to store the output of the solution to a ZND
% detonation (from sdt.znd.zndsolve). Includes a timestamp of when the file was created,
% input conditions, and tab-separated columns of output data.

    P1 = pressure(znd_output.gas1);
    T1 = temperature(znd_output.gas1);
    r1 = density(znd_output.gas1);
    
    U1 = znd_output.U1;

    fn = [fname, '_', num2str(U1), '_znd.txt'];
	d = date;

	P = P1/oneatm;
	
	fid = fopen(fn, 'w');
	fprintf(fid, '# ZND: DETONATION STRUCTURE CALCULATION\n');
	fprintf(fid, '# CALCULATION RUN ON %s\n\n', d);
	
	fprintf(fid, '# INITIAL CONDITIONS\n');
	fprintf(fid, '# TEMPERATURE (K) %4.1f\n', T1);
	fprintf(fid, '# PRESSURE (atm) %2.1f\n', P);
	fprintf(fid, '# DENSITY (kg/m^3) %1.4e\n', r1);
	
	fprintf(fid, '# SHOCK SPEED (m/s) %5.2f\n\n', U1);
	
	fprintf(fid, '# Induction Times\n');
	fprintf(fid, '# Time to Peak Thermicity =   %1.4e s\n', znd_output.ind_time_ZND);
	fprintf(fid, '# Distance to Peak Thermicity =   %1.4e m\n', znd_output.ind_len_ZND);
    
    fprintf(fid, '\n# Exothermic/Reaction Times\n');    
	fprintf(fid, '# Exothermic Pulse Time =   %1.4e s\n', znd_output.exo_time_ZND);
	fprintf(fid, '# Exothermic Pulse Distance =   %1.4e m\n', znd_output.exo_len_ZND);
	
	fprintf(fid, '# REACTION ZONE STRUCTURE\n\n');
	
	fprintf(fid, '# THE OUTPUT DATA COLUMNS ARE:\n');
	fprintf(fid, 'Variables = Distance (m), Mach Number, Time (s), Pressure (atm), Temperature (K), Density (kg/m^3), Thermicity (1/s)\n');
	
    z = [znd_output.distance';znd_output.M';znd_output.time';znd_output.P';znd_output.T';znd_output.rho';znd_output.thermicity'];
	fprintf(fid, '%14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e\n', z);
	
	fclose(fid);
	
	fn = [fname, '_', num2str(U1), '_znd2.txt'];

	fid = fopen(fn, 'w');
	fprintf(fid, '# ZND: DETONATION STRUCTURE CALCULATION\n');
	fprintf(fid, '# CALCULATION RUN ON %s\n\n', d);
	
	fprintf(fid, '# INITIAL CONDITIONS\n');
	fprintf(fid, '# TEMPERATURE (K) %4.1f\n', T1);
	fprintf(fid, '# PRESSURE (atm) %2.1f\n', P);
	fprintf(fid, '# DENSITY (kg/m^3) %1.4e\n', r1);
	
	fprintf(fid, '# SHOCK SPEED (m/s) %5.2f\n\n', U1);
	
    fprintf(fid, '# REACTION ZONE STRUCTURE\n\n');
        
	fprintf(fid, '# Induction Times\n');
	fprintf(fid, '# Time to Peak Thermicity =   %1.4e s\n', znd_output.ind_time_ZND);
	fprintf(fid, '# Distance to Peak Thermicity =   %1.4e m\n', znd_output.ind_len_ZND);
    
    fprintf(fid, '\n# Exothermic/Reaction Times\n');
	fprintf(fid, '# Time of Reaction based on Thermicity Pulse Width =   %1.4e s\n', znd_output.exo_time_ZND);
	fprintf(fid, '# Length of Reaction based on Thermicity Pulse Width =   %1.4e m\n', znd_output.exo_len_ZND);
	
	fprintf(fid, '# THE OUTPUT DATA COLUMNS ARE:\n');
	fprintf(fid, 'Variables = Distance (m), Velocity (m/s), Sound Speed (m/s), Gamma, Weight (kg/mol),c^2-U^2 (m/s)\n');
	
	z = [znd_output.distance';znd_output.U';znd_output.af';znd_output.g';znd_output.wt';znd_output.sonic'];
	fprintf(fid, '%14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e\n', z);
	
	fclose(fid);
