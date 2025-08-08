% Shock and Detonation Toolbox Utility Program
%
% thermo_fit.m - Requires tabular thermodynamic data file (see
% twobutenal.m for format).  Plost tabular thermodynamic data and
% fits to NASA polynomials (7 term).  Compares tabular data with fit and 
% writes data file in either cti, NASA7 or NASA9 format.
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
%
clc;clear;close all;
Ra = gasconstant;
T_zero = 298.15;  % reference state temperature
P_zero = oneatm;  % reference state pressure
plot_tab = true;  % plot tabular data
fit = true;       % fit tabular data to NASA-7 polynomial form
plot_diff = true; % plot difference between tabular data and fit
cti = true;       % output data file in cti format
nasa7 = true;     % output data file in NASA-7 format
nasa9 = true;     % output data file in NASA-9 format
% data files are written to directory labeled with species name
%%
% create data structures 
run twobutenal.m
% this file contains information needed to specify the species
% composition, standard state and and the tabular data converted to
% normalized format. See example file twobutenal.m for details.
dir_NAME = name;  %  subdirectory  name for output files
status = mkdir(dir_NAME);
% Range of temperature table
imax = size(T);
Tmin = min(T(:));
Tmax = max(T(:));
%
fprintf('Species: %s\n',species);
fprintf('Total number of atoms %2.0f\n', na);
fprintf('Name: %s\n',name);
fprintf('Composition: \n');
for k =1:ne
    fprintf('%s %2.0f\n',els(k),at(k) );
end
fprintf('Tmin (K)            %5.2f\n',Tmin);
fprintf('Tmax (K)            %5.2f\n',Tmax);
fprintf('Number points       %3.0f\n',imax);
fprintf('Standard state values: \n');
fprintf('H/RT                %5.2f\n',H_RT_zero);
fprintf('S/R                 %5.2f\n',S_R_zero);

%% plot data
if (plot_tab)
    % plot specific heat vs temperature
    font ='TimesRoman';
    fontsize = 12;
	set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
    name = [' Tabular data for ' species];
    figure('Name',name);
    maxy = max(CP_R(:));
    miny = min(CP_R(:));
    maxx = Tmax;
    minx = Tmin;
    fontsize=12;
    axis([minx maxx miny maxy]);
    hold on
    plot(T(:),CP_R(:),'LineWidth',2);
    xlabel('T (K)','FontSize',fontsize);
    ylabel('C_p/R','FontSize',fontsize);
    hold off
    
    % plot enthalpy data vs temperature
    figure('Name',name);
    maxy = max(H_RT(:));
    miny = min(H_RT(:));
    maxx = Tmax;
    minx = Tmin;
    fontsize=12;
    axis([minx maxx miny maxy]);
    hold on;
    plot(T(:),H_RT(:),'LineWidth',2);
    xlabel('T (K)','FontSize',fontsize);
    ylabel('h/RT','FontSize',fontsize);
    hold off
    
    % plot entropy data vs temperature
    figure('Name',name);
    maxy = max(S_R(:));
    miny = min(S_R(:));
    maxx = Tmax;
    minx = Tmin;
    fontsize=12;
    axis([minx maxx miny maxy]);
    hold on;
    plot(T(:),S_R(:),'LineWidth',2);
    xlabel('T (K)','FontSize',fontsize);
    ylabel('S/R','FontSize',fontsize);
    hold off
end
if (fit)
    Tmid = 1000.;
    % find specific heat derivative at midpoint
    index_mid = max(find(T(:)<=Tmid));   %find closest temperature to selected value
    Tmid = T(index_mid:index_mid);
    display(['Adjusted temperature for midpoint of fit = ',num2str(Tmid)]);
    display(['Adjusted temperature for minimum of fit = ',num2str(Tmin)]);
    display(['Fitting...']);
    z = [T CP_R H_RT S_R]';
    DCPRDT = (CP_R(index_mid+1)-CP_R(index_mid-1))/(T(index_mid+1) - T(index_mid-1));
    [x, cp_fit, index_mid, a_0_6, a_1_6, s_fit,  a_0_7, a_1_7, h_fit] = poly_cp(z,Tmin,Tmid,Tmax,CP_R_zero,DCPRDT,S_R_zero,H_RT_zero);
    %% Check for jumps and maximum differences with original data
    cp_jump = cp_fit(index_mid+1)-cp_fit(index_mid);
    h_jump = h_fit(index_mid+1)-h_fit(index_mid);
    s_jump = h_fit(index_mid+1)-h_fit(index_mid);
    display(['Result of optimization after adjustment']);
    display(['Jump at Tmid in Cp/R ', num2str(cp_jump)]);
    display(['Jump at Tmid in H/RT ', num2str(h_jump)]);
    display(['Jump at Tmid  in S/R ', num2str(s_jump)]);
    %Compute maximum errors (remove duplicate value at Tmid)
    cp_fit(index_mid:index_mid) = [];
    h_fit(index_mid:index_mid) = [];
    s_fit(index_mid:index_mid) = [];
    cp_diff = cp_fit-CP_R;
    h_diff = h_fit- H_RT;
    s_diff = s_fit-S_R;
    cp_max_diff = max(abs(cp_diff));
    h_max_diff = max(abs(h_diff));
    s_max_diff = max(abs(s_diff));
    display(['Maximum difference in Cp/R ', num2str(cp_max_diff)]);
    display(['Maximum difference in H/RT ', num2str(h_max_diff)]);
    display(['Maximum difference in S/R ', num2str(s_max_diff)]);
    if (plot_diff)
        font ='TimesRoman';
        fontsize = 12;
    	set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
        % plot differences
        figure('Name',' Tabular vs fit data');
        hold on;
        set(gca,'FontSize',fontsize,'LineWidth',2);
        plot(T(:),cp_diff(:));
        plot(T(:), h_diff(:));
        plot(T(:), s_diff(:));
        xlabel('T (K)','FontSize',fontsize);
        ylabel('Differences','FontSize',fontsize);
        legend('Cp/R', 'H/RT', 'S/R','Location','southeast');
        title(['Thermodynamic data ', species],'FontSize',fontsize);
        hold off
        
        % plot thermo data vs temperature
        figure('Name','Fit of tabular data' );
        maxy = max(cp_fit(:));
        miny = min(cp_fit(:));
        maxx = max(T(:));
        minx = min(T(:));
        fontsize=12;
        axis([minx maxx miny maxy]);
        hold on
        set(gca,'FontSize',fontsize,'LineWidth',2);
        plot(T(:),cp_fit(:));
        plot(T(1:2:imax),CP_R(1:2:imax),'ok');
        xlabel('T (K)','FontSize',fontsize);
        ylabel('C_p/R','FontSize',fontsize);
        title(['Thermodynamic data ', species ],'FontSize',fontsize);
        legend('Fit','Tabular','Location','southeast');
        hold off
        
        % plot thermo data vs temperature
        figure('Name','Fit of tabular data' );
        maxy = max(h_fit(:));
        miny = min(h_fit(:));
        maxx = max(T(:));
        minx = min(T(:));
        fontsize=12;
        axis([minx maxx miny maxy]);
        hold on;
        set(gca,'FontSize',fontsize,'LineWidth',2);
        plot(T(:),h_fit(:));
        plot(T(1:2:imax),H_RT(1:2:imax),'ok');
        xlabel('T (K)','FontSize',fontsize);
        ylabel('h/RT','FontSize',fontsize);
        title(['Thermodynamic data ', species ],'FontSize',fontsize);
        legend('Fit','Tabular','Location','southeast');
        hold off
        
        % plot thermo data vs temperature
        figure('Name','Fit of tabular data' );
        maxy = max(s_fit(:));
        miny = min(s_fit(:));
        maxx = max(T(:));
        minx = min(T(:));
        fontsize=12;
        axis([minx maxx miny maxy]);
        hold on;
        set(gca,'FontSize',fontsize,'LineWidth',2);
        plot(T(:),s_fit(:));
        plot(T(1:2:imax),S_R(1:2:imax),'ok');
        xlabel('T (K)','FontSize',fontsize);
        ylabel('S/R','FontSize',fontsize);
        title(['Thermodynamic data ', species ],'FontSize',fontsize);
        legend('Fit','Tabular','Location','southeast');
        hold off
        display(['Paused, CR to continue']);
        pause;
        close all;
    end
    %% create coefficient in correct order for NASA-7 format
    %The NASA-7 polynomials are of the form:
    %Cp/R = a1 + a2 T + a3 T^2 + a4 T^3 + a5 T^4
    %H/RT = a1 + a2 T /2 + a3 T^2 /3 + a4 T^3 /4 + a5 T^4 /5 + a6/T
    %S/R  = a1 lnT + a2 T + a3 T^2 /2 + a4 T^3 /3 + a5 T^4 /4 + a7
    % a = [x(1:5)',a_0_6,a_0_7,x(6:10)',a_1_6,a_1_7];
    
    a = [x(1:5)',a_0_6,a_0_7,x(6:10)',a_1_6,a_1_7];
    %% Cantera .cti format output file
    if (cti)
        display(['Writing Cantera format .cti file for species ',species]);
        format = ' %c:%d ';
        atoms = '';
        for k = 1:ne
            if (at(k) > 0)
                str = sprintf(format,els(k),at(k));
                atoms = strcat(atoms,str);
            end
        end
        Cantera_output_text = ['name = "',species,'",\n',...
            '        atoms = "',atoms, '",' ];
        species_clean = strrep(species,'*','star');
        outputfilename =[dir_NAME,'/',species_clean,'.cti'];
        outputfile = fopen(outputfilename,'w');
        fprintf(outputfile, ['species(', Cantera_output_text,'\n', ...
            '        thermo=(NASA([%.3f, %.3f],\n',...
            '                     ['], Tmin, Tmid);
        for i = 1:6
            fprintf(outputfile, '%+.8E, ', a(i));
        end
        fprintf(outputfile, '%+.8E]),\n', a(7));
        fprintf(outputfile, ['                NASA([%.3f, %.3f],\n',...
            '                     ['], Tmid, Tmax);
        for i = 8:13
            fprintf(outputfile, '%+.8E, ',a(i));
        end
        fprintf(outputfile, '%+.8E])', a(14));
        fprintf(outputfile, [ '),\n',...
            '        note = ''Fit by JES ', datestr(now),''')']);
        fclose(outputfile);
    end
    %% NASA (Chemkin II) format output file
    if (nasa7)
        display(['Writing NASA-7 (Chemkin II) format .dat file for species ',species]);
        format = '%2c%3i';
        atoms = '';
        for k = 1:ne
            if (at(k) > 0)
                str = sprintf(format,els(k),at(k));
                atoms = strcat(atoms,str);
            end
        end
        % pad species names and atom fields to fixed length
        name_field = species;
        name_char_number = size(species,2);
        number_spaces = 18 - name_char_number;
        for i = 1:number_spaces
            name_field =[name_field,' '];
        end
        c_size = size(atoms,2);
        n_spaces = 20 -c_size;
        for i = 1:n_spaces
            atoms =[atoms,' '];
        end
        %NASA format is particular about columns alignment
        species_clean = strrep(species,'*','star');
        outputfilename =[dir_NAME,'/',species_clean,'_NASA7.dat'];
        outputfile = fopen(outputfilename, 'w');
        %            fprintf(outputfile,'THERMO\n%10.3f%10.3f%10.2f\n',Tmin, Tmax,Tmid);
        space = '      ';
        fprintf(outputfile, '%s%s%s%s%10.2f%10.2f%8.2f%s1\n',name_field,' SDT18',atoms,'G',Tmin, Tmax,Tmid,space);
        for i = 8:12
            fprintf(outputfile, '%+.8E', a(i));
        end
        fprintf(outputfile, '    2\n');
        for i = 13:14
            fprintf(outputfile, '%+.8E', a(i));
        end
        for i = 1:3
            fprintf(outputfile, '%+.8E', a(i));
        end
        fprintf(outputfile, '    3\n');
        for i = 4:7
            fprintf(outputfile, '%+.8E', a(i));
        end
        fprintf(outputfile,'                   4\n');
        %            fprintf(outputfile,'END\n');
        fclose(outputfile);
    end
    %% NASA 9 term (1996) format output file
    %
    %Record     Constants     Format     Column 
    %1  Species name or formula A24 1 to 24
    %   Comments (data source) A56 25-80 
    %2  Number of T intervals I2 2 
    %   Optional identification code A6 4-9 
    %   Chemical formulas, symbols, and numbers 5(A2,F6.2) 11-50 
    %   Zero for gas and nonzero for condensed phases I1 52
    %   Molecular weight F13.5 53-65 
    %   Heat of formation at 298.15 K, J/mol F13.5 66-80 
    %3  Temperature range 2F10.3 2-21 
    %   Number of coefficients for Cp0/R I1 23 
    %   T exponents in empirical equation for Cp0/R 8F5.1 24-63
    %   {H0(298.15)-H0(0)}, J/mol F15.3 66-80 
    %4  First five coefficients for Cp0/R 5D16.8 1-80 
    %5  Last two coefficients for Cp0/R 3D16.8 1-48
    %   Integration constants b1 and b2 2D16.8 49-80 
    %... Repeat 3, 4, and 5 for each interval
    %
    if (nasa9)
        display(['Writing NASA-9 format .dat file for species ',species]);
        format = '%2c%6.2f';
        atoms = '';
        for k = 1:5
            if (k <= ne)
                if (at(k) > 0)
                    str = sprintf(format,strjust(els(k),'left'),at(k));
                    atoms = strcat(atoms,str);
                else
                    str = sprintf(format,[' '],0.0);
                    atoms = strcat(atoms,str);
                end
            else
                str = sprintf(format,[' '],0.0);
                atoms = strcat(atoms,str);
            end
        end
        % pad species names and atom fields to fixed length
        name_field = species;
        name_char_number = size(species,2);
        number_spaces = 24 - name_char_number;
        for i = 1:number_spaces
            name_field =[name_field,' '];
        end
        c_size = size(atoms,2);
        n_spaces = 40 -c_size;
        for i = 1:n_spaces
            atoms =[atoms,' '];
        end
        %NASA format is particular about columns alignment
        species_clean = strrep(species,'*','star');
        output_file_name =[dir_NAME,'/',species_clean,'_NASA9.dat'];
        outputfile = fopen(output_file_name, 'w');
        %            fprintf(outputfile,'thermo nasa9\n);
        space = '      ';
        % record 1
        fprintf(outputfile, '%s%s%s\n',name_field,'Data fit by JES ',datestr(now));
        % record 2
        fprintf(outputfile,' %1d %s %s %1d%13.5f%13.5f\n',2,space,atoms,0,m,DeltaH_f0);
        % record 3
        dummy = 0;  % need to set this to  H(298.15) - H(0)
        fprintf(outputfile,' %10.3f%10.3f %1d%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f  %15.3f\n', Tmin, Tmid,7,-2.,-1.,0.,1.,2.,3.,4.,0.,dummy);
        %record 4
        fprintf(outputfile, '%16.8E%16.8E%16.8E%16.8E%16.8E\n', 0,0,a(1),a(2),a(3));
        %Record 5
        fprintf(outputfile, '%16.8E%16.8E%16.8E%16.8E%16.8E\n', a(4),a(5),0.0,a(6),a(7));
        fprintf(outputfile,' %10.3f%10.3f %1d%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f  %15.3f\n', Tmid, Tmax,7,-2.,-1.,0.,1.,2.,3.,4.,0.,dummy);
        %record 4
        fprintf(outputfile, '%16.8E%16.8E%16.8E%16.8E%16.8E\n', 0,0,a(8),a(9),a(10));
        %Record 5
        fprintf(outputfile, '%16.8E%16.8E%16.8E%16.8E%16.8E\n', a(11),a(12),0.0,a(13),a(14));
        fclose(outputfile);
    end
end
    
function [x, cp_fit, index_mid, a_0_6, a_1_6, s_fit, a_0_7, a_1_7, h_fit] = poly_cp(z, Tmin, Tmid, Tmax,CP_R_zero,DCPRDT,S_R_zero,H_RT_zero)
% poly_cp.m - A function that fits a two segment polynomial to specific
% heat, use NASA 7 term format from SP-272. Gordon and McBride 1971.
% Coefficients a6 and a7 for enthalpy and entropy are also computed.
%
%Eq. 90: cp0/R=a1+a2T+a3*T^2+a4*T^3+a5*T^4 
%Eq. 91: H0_T/(RT)=a1+a2/2*T+a3/3*T^2+a4/4*T^3+a5/5*T^4+a6/T 
%Eq. 92: S0_T/R=a1*ln(T)+a2*T+a3/2*T^2+a4/3*T^3+a5/4*T^4+a7
%
% This program fits the specific heat only then uses standard state data
% and the analytical integration to obtain the additional coefficients for
% entropy and enthalpy.  The specific heat fit constrains the derivative to
% vanish at the lowest temperature, the value and derivative are continuous
% at the knot at T_mid and the value of specific heat at 298.15 K is
% constrained to be the specified value of the standard state. The value of
% the entropy and entropy are set to standard state values at 298.15 K. The
% entropy and enthalpy values at the midpoints are continuous.  This
% procedure produces high quality fits as long as the input entropy and enthalpy
% are thermodynamically consistent with the specific heat data.  This will
% always be the case if the thermodynamic data have been derived from
% partition functions.   
%
% Matlab R2018a
% JES 7/14/2018
%
T_zero = 298.15;
T = z(1,:);
cp_over_R = z(2,:);
h_over_RT = z(3,:);
s_over_R = z(4,:);
index_max = length(T);
Tmin = min(T);
Tmax = max(T);
index_mid = max(find(T(:)<=Tmid));   %find closest temperature in vector
Tmid = T(index_mid:index_mid);
%
% set up coefficient matrix
C = zeros(index_max+1,10);
for i = 1:5
    for k =1:index_mid
        C(k,i) = T(k)^(i-1);
    end
end
for i = 1:5
    for k = index_mid:index_max
        C(k+1,i+5) = T(k)^(i-1);
    end
end
% initial guess for coefficients
for i = 1:10
    x0(i)  = 1.;
end
% right hand side
for k = 1:index_mid
    d(k) = cp_over_R(k);
end
for k = index_mid:index_max
    d(k+1) = cp_over_R(k);
end
% constraint values 
for i=1:5
    Aeq(1,i) = T_zero^(i-1);   %fix standard state value for poly 1
    Aeq(1,i+5) = 0.0;
    %
    Aeq(2,i) = T(index_mid)^(i-1);   %fix midpoint value  for poly 1
    Aeq(2,i+5) = 0.0;
    %
    Aeq(3,i) = 0.;                    %fix midpoint value  for poly 2
    Aeq(3,i+5) = T(index_mid)^(i-1);
    %
    Aeq(4,i) = (i-1)*T(index_mid)^(i-2);  %fix derivative for poly 1
    Aeq(4,i+5) = 0.0;
    %
    Aeq(5,i) = 0.;                       %fix derivative for poly 2                 
    Aeq(5,i+5) = (i-1)*T(index_mid)^(i-2);
    %
%    Aeq(6,i) = (i-1)*T(1)^(i-2);    % sero derivative at lowest temperature
%    Aeq(6,i+5) = 0.0;
end
beq(1) = CP_R_zero;  % match standard state, polynomial 1
beq(2) = cp_over_R(index_mid);  % continuity at knot, polynomial 1
beq(3) = cp_over_R(index_mid);  % continuity at knot, polynomial 2
beq(4) = DCPRDT;   % match derivative at knot, polynomial 1
beq(5) = DCPRDT;   % match derivative at knot, polynomial 2
%beq(6) = 0.;        % zero derivative at lowest temperature in poly 1
% least squares fit, diagnostics can be turned on if necessary
%options = optimoptions('lsqlin','Algorithm','active-set','Diagnostics','on');
options = optimoptions('lsqlin','Algorithm','interior-point','Diagnostics','off');
x = lsqlin(C,d,[],[],Aeq,beq,[],[],x0,options);
% evaluate polynomial for specific heat
cp_fit = C*x;
% value at standard state
% check fit at standard state and eliminate any jump at midpoint
display(['Result of initial optimization']);
sum = 0.;
for i = 1:5
    sum = sum + x(i)*T_zero^(i-1);
end
delta_cp = CP_R_zero-sum;
display(['Difference in CP at T_zero = ',num2str(delta_cp)]);
% check for jump at T mid
cp_jump = cp_fit(index_mid+1)-cp_fit(index_mid);
display(['Jump in CP at midpoint = ',num2str(cp_jump)]);
% remove jump
x(6) = x(6) - cp_jump;
% update initial guess and re-run optimization
x1 = x;
x = lsqlin(C,d,[],[],Aeq,beq,[],[],x1,options);
x2 = x;
x = lsqlin(C,d,[],[],Aeq,beq,[],[],x2,options);
display(['Result of re-running optimization']);
%evaluate fit again
cp_fit = C*x;
sum = 0.;
for i = 1:5
    sum = sum + x(i)*T_zero^(i-1);
end
delta_cp = CP_R_zero-sum;
display(['Difference at T_zero in CP = ',num2str(delta_cp)]);
% check for jump at T mid
cp_jump = cp_fit(index_mid+1)-cp_fit(index_mid);
display(['Jump in CP at midpoint = ',num2str(cp_jump)]);
%
% evaluate the enthalpy fit
%
C2 = zeros(index_max+1,10);
for i = 1:5
    for k =1:index_mid
        C2(k,i) = T(k)^(i-1)/i;
    end
end
for i = 1:5
    for k = index_mid:index_max
        C2(k+1,i+5) = T(k)^(i-1)/i;
    end
end
% fix poly 1 constant by matching enthalpy at 298.15 
sum = 0.;
for i = 1:5
   sum = sum + x(i)*T_zero^(i-1)/i;
end
a_0_6 = T_zero*(H_RT_zero - sum);
% fix poly 2 constant by matching entropy at knot 
sum1 = 0.;
for i = 1:5
   sum1 = sum1 + x(i)*T(index_mid)^(i-1)/i;
end
sum1 = sum1 + a_0_6/T(index_mid);
% match polynomials at knot
sum2 = 0.;
for i = 1:5
   sum2 = sum2 + x(5+i)*T(index_mid)^(i-1)/i;
end
a_1_6 = T(index_mid)*(sum1 - sum2);
% evaluate corrected fit
h_fit=C2*x; 
for k = 1:index_mid
    h_fit(k) = h_fit(k) + a_0_6/T(k);
end
for k = index_mid:index_max
    h_fit(k+1) = h_fit(k+1) + a_1_6/T(k);
end
%
% evaluate entropy fit
% 
C1 = zeros(index_max+1,10);
for i = 1:5
    for k =1:index_mid
        if (i==1)
            C1(k,i) = log(T(k));
        else
            C1(k,i) = T(k)^(i-1)/(i-1);
        end
    end
end
for i = 1:5
    for k = index_mid:index_max
        if (i==1)
            C1(k+1,i+5) = log(T(k));
        else
            C1(k+1,i+5) = T(k)^(i-1)/(i-1);
        end
    end
end
% fix poly 1 constant by matching entropy at 300 K, gives lower error than
% matching interpolated value at T_zero
s_fit=C1*x; 
index_300 = find(T(:)==300);
S_R_300 = s_over_R(index_300);
a_0_7 = S_R_300 - s_fit(index_300) ;
% fix poly 2 constant by matching entropy at knot 
a_1_7 =(s_fit(index_mid) + a_0_7) - s_fit(index_mid+1);
% evaluate corrected fit
for k = 1:index_mid
    s_fit(k) = s_fit(k) + a_0_7;
end
for k = index_mid:index_max
    s_fit(k+1) = s_fit(k+1) + a_1_7;
end
end
