% Shock and Detonation Toolbox Utility Program
% partition_rotvib.m
% Computation of statistical mechanical partition function and
% thermodynamic properties using nonlinear rotational-vibrational energy
% terms and multiple electronic states for diatomic molecules.
% Requires an input file that defines electronic energy levels, vibrational
% and rotational paramters.  Three examples are available:
%
% OH_rotvib.m
% CH_rotvib.m
% NO_rotvib.m
%
% There are multiple options that can be set by editing the user specified
% parameters at the beginning of the program. These include:
%
% choosing the electronic levels to be included in the computation
% writing an output data file 
% plotting the results
% compare results to existing Cantera fit
% create a new polynomial fit and write out coefficients in Cantera, 
% NASA 7 (Chemkin-style), or NASA 9 formats. Currently only setup for two
% temperature ranges and NASA-7 style polynomial coefficients.
% 
% This version configured for a diatomic heteronuclear molecule like OH, CH or
% NO and will have to be modified for a homonuclear molecule like N2 or O2.
% There is no treatment of nuclear spin or electronic spin coupling to the
% rotational spin (spin-orbit coupling).
%
% References:
% NIST computational Chemistry Comparison http://cccbdb.nist.gov/thermo.asp
% NIST webbook  http://webbook.nist.gov/cgi/cbook.cgi?ID=C3352576&Units=SI&Mask=1000#Diatomic
% C. Carlone, Spectrum of the Hydroxyl Radical, PhD Thesis University of
% British Columbia,  1969. 
% Luque and Crossley, The Journal of Chemical Physics 109, 439 (1998)
% Boyd and Schwartzentruber, Chapter 2 and 3. 
% 
% ################################################################################
% Theory, numerical methods and applications are described in the following report:
% 
%   SDToolbox - Numerical Tools for Shock and Detonation Wave Modeling
%
%   Explosion Dynamics Laboratory
%   Graduate Aerospace Labortories
%   California Institute of Technology
%   Pasadena, CA USA 91125
%
%   GALCIT Report FM2018.001, Revised September 2019
% 
% Please cite this report and the website if you use these routines. 
% 
% http://shepherd.caltech.edu/EDL/public/sdt/SD_Toolbox/
% 
% Please refer to LICENCE.txt or the above report for copyright and disclaimers.
% #########################################
% Updated February 2020
% Tested with: 
%     MATLAB 2018a, Cantera 2.4
% Under these operating systems:
%   Windows 10
%
clc; clear; close all;
%% ****************  User specfied parameters *******************
    %Set levels to include in the partition function computation and bounds for
    %sums.  Use kmin = 1 for ground state, kmin = 2 for first electronic
    %excited state.
    kmin = 2; % lowest electronic level to include in partition function
    % 1 -  ground state
    % 2 -  first electronic state, etc.
    kmax = 2;  % highest electronic level to include in partition function
    % starting temperature and increments for partition function sums
    Tmin  = 200.; % starting temperature for evaluation of partitiion sums
    %imax = 581.;   %number of temperature values (delta T = 10 K)
    %imax = 981;
    %imax = 1481;
    imax = 1981;
    % limits on sums for energy levels
    n_max = 50; % highest vibration state to consider in initial examination
    j_max = 200; % highest rotational state to consider in initial examination
    % plot raw results 
    plot_part = true; 
    % write output file
    output = false;
    % fitting to polynomials
    fit = true;
    % comparison to data in existing cantera file
    compare = false;
    % write cantera data to csv file
    cantera_out = false;  
    cti = true;   % write cantera .cti format output file (only if fit)
    nasa7 = false;  % write nasa 7 (Chemkin II) .dat format output file  (only if fit)
    nasa9 = false; % write nasa 9  .dat format output file  (only if fit)
%%Spectroscopic constants
    q = 1.60217662E-19;  % charge on electron
    h = 6.62607004E-34 ;  %Planck constant (J-s)
    kb = 1.38064852E-23; %Boltzmann constant  (J/K)
    c0 = 2.99792458E+8;  %speed of light in vacuum (m/s)
    c2 = 1E2*h*c0/kb;  %conversion from cm-1 to temperature (K), second spectroscopic constant
    Na =  6.022140857E+23; %Avogadro constant (molecules per gram mole)
    Ra = kb*Na;  %gas constant  (J/g-mol K)
    c1 = 1E2*h*c0*Na;  %  conversion from cm-1 to energy J/mol
    amu = 1.660539040E-27;  % kg
    c3 = q*1e-2/h/c0;   % conversion from eV to cm-1
    c4 = q*Na;    % conversion from eV to J/mol
%%  Cantera constants
    Rc = gasconstant;  %what Cantera uses for gas constant  J/kmol K
    T_zero = 298.15;  %thermochemical standard state 
    P_zero = oneatm;  %thermochemical standard state 
%%  Molecular data file
run NO_rotvib.m
[a b] = size(molecule);
display(['Molecule ',name, ' ', species])
display(['Number of electronic states ',num2str(b)]);
if (kmax > b)
    kmax = b;
end
%% initialize variables
species = molecule(kmin).name;
DeltaH_f0 = molecule(kmin).deltaH_f0;
U = zeros(1,imax);
U_RT = zeros(1,imax);
CV_R = zeros(1,imax);
CP_R = zeros(1,imax);
S_R = zeros(1,imax);
q = zeros(1,imax);
dqdT = zeros(1,imax);
d2qdT2 = zeros(1,imax);
nmax = zeros(1,kmax);
JMAX = zeros(kmax,n_max);
E = zeros(kmax,n_max,j_max);
T = zeros(1,imax);
cv_r = zeros(1,imax);
u_rt = zeros(1,imax);
s_r = zeros(1,imax);
q = zeros(1,imax);
dqdT = zeros(1,imax);
d2qdT2 = zeros(1,imax);
%% Find maximum value of vibrational level based on difference between energy levels becoming negative
%  or vibration energy exceeding dissociation energy D0.  The second
%  criteria is important or shallow potential wells with poorly known
%  vibrational frequency constants (2BSigma+ state of OH for example)
for k = kmin:kmax
    d_sum = 0.;
    for n=1:n_max
        v = n-1; % vibrational quantum number
        G = 0.;
        G2 = 0.;
        for i=1:7
            G = G + molecule(k).Y0(i)*(v+0.5)^i;
            G2 = G2 + molecule(k).Y0(i)*(v+1.5)^i;
        end
        if ((G2-G) < 0)
            break
        elseif (G >= molecule(k).De)
            break
        end
     nmax(k) = n - 1;       
    end
    display(['Electronic level ',num2str(k),' vibrational energy ',num2str(G), ' nmax ', num2str(nmax(k))]);
    v_max =  -1*molecule(k).Y0(1)/ molecule(k).Y0(2)/2 - 0.5;
    display(['Maximum level by Irwin''s Eqn (19) ',num2str(v_max)]);
    v_max2 = -1*molecule(k).Y1(1)/ molecule(k).Y1(2)/2 - 0.5;
    display(['Maximum level by Irwin''s Eqn (20) ',num2str(v_max2)]);
end
%% find maximum value of rotational level for each vibrational level
% this is only reliable if rotational constants are well known, otherwise
% a cutoff should be used to prevent nonphysical rotational levels. 
for k=kmin:kmax
    for n = 1:nmax(k)
        v = n-1;
        % vibrational energy
        G = 0.;
        for i=1:7
            G = G + molecule(k).Y0(i)*(v +.5)^i ;
        end
        % rotational energy
        for j=1:j_max
            J = j-1;   % rotational quantum number
            F = 0.;
            F1 = 0.;
            for i=1:6
                F = F + J*(J+1)*(molecule(k).Y1(i)+J*(J+1)*molecule(k).Y2(i))*(v+0.5)^(i-1);
                F1 = F1 + (J+1)*(J+2)*(molecule(k).Y1(i)+(J+1)*(J+2)*molecule(k).Y2(i))*(v+0.5)^(i-1);
            end
             if ((F1-F) < 0)
                JMAX(k,n) = j-1;
                disp('Max J due to Delta F <0');
                break
            elseif (F + G >  molecule(k).De)
                disp('Max J due to E > De');
                 JMAX(k,n) = j-1;
                break
            end
        end
        display(['Electronic level ',num2str(k),' vibrational level ',num2str(n), ' Jmax ', num2str(JMAX(k,n))]);
    end
end
%%
display(['computing energy levels and partition function']);
%% energy levels
% for each electronic state k
for k=kmin:kmax
    % for  each vibrational level n
    for n = 1:nmax(k)
        v = n-1;
        G = 0;
        % vibrational contribution
        for i=1:7
            G = G + molecule(k).Y0(i)*(v+0.5)^i;
        end
        % for each rotational level J
        for j = 1:JMAX(k,n)
            % rotational contributions with vibrational corrections
            F = 0.;
            J = j-1;
            for i=1:6
                F = F + J*(J+1)*(molecule(k).Y1(i)+J*(J+1)*molecule(k).Y2(i))*(v+0.5)^(i-1);
            end
            E(k,n,j) = G + F + molecule(k).Te;
        end
    end
    font ='TimesRoman';
    fontsize = 12;
	set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
    figure('Name', ['Electronic energy Level ', num2str(k),' ', molecule(k).level ]);
    JJ = 0:1:j_max;
    hold on
    for n=1:nmax(k)
        EE(1,:) = E(k,n,:);
        plot(JJ(1:JMAX(k,n)),EE(1,1:JMAX(k,n)),'.');
    end
    Diss = molecule(k).Te + molecule(k).De;
    plot([0,JJ(JMAX(k,1):JMAX(k,1))], [E(k,1,1), E(k,1,1)]);
        plot([0,JJ(JMAX(k,1):JMAX(k,1))], [Diss, Diss]);
    xlabel('rotational quantum number J')
    ylabel('energy (cm^{-1})')
    hold off
end
%% partition function and temperature derivatives as a function of temperature
for i=1:imax
    T(i) = Tmin + (i-1)*10;
    sum1 = 0.;
    sum2 = 0.;
    sum3 = 0.;
    for k= kmin:kmax
        ge = molecule(k).g;
        for n=1:nmax(k)
            for j = 1:JMAX(k,n)
                J = j-1;
                sum1 = sum1 + ge*(2*J+1)*exp(-E(k,n,j)*c2/T(i));
                sum2 = sum2 + ge*(2*J+1)*E(k,n,j)*c2*exp(-E(k,n,j)*c2/T(i))/T(i)/T(i);
                sum3 = sum3 + ge*(2*J+1)*(E(k,n,j)*c2/T(i))^2*exp(-E(k,n,j)*c2/T(i))/T(i)/T(i);
            end
        end
    end
    q(i) = sum1;
    dqdT(i) = sum2;
    d2qdT2(i) = sum3 - 2*sum2/T(i);
end
Tmax =  max(T);
%% internal (exclusive of translation) energy as a function of temperature
U = kb*T.*T.*dqdT./q ;
%% partition function at reference temperature
sum = 0;
for k=kmin:kmax
    ge = molecule(k).g;
    for n=1:nmax(k)
        for j =1:JMAX(k,n)
            J = j-1;
            sum = sum + ge*(2*J+1)*exp(-E(k,n,j)*c2/T_zero);
        end
    end
end
q_zero = sum;
%% internal energy at reference temperature
sum = 0;
for k=kmin:kmax
    ge = molecule(k).g;
    for n=1:nmax(k)
        for j =1:JMAX(k,n)    
            J = j-1;
           sum = sum + ge*(2*J+1)*E(k,n,j)*c2*exp(-E(k,n,j)*c2/T_zero);
        end
    end
end
U_zero = Na*kb*sum/q_zero;    %J/mol
% compute correction needed to adjust internal energy to thermodynamic
% reference value accounting for heat of formation of atoms and translation
U_c = DeltaH_f0 -2.5*Ra*T_zero - U_zero;
% correct energy and add translational component
U = Na*U +U_c + Ra*T*1.5;
U_RT = U./T/Ra;
H_RT = U_RT +1;
%% specific heats
CV_R =  2*T.*dqdT./q - T.*T.*(dqdT./q).^2 + T.*T.*d2qdT2./q  + 3./2. ;
CP_R = CV_R + 1.;    % compute C_P for comparison with JANNAF data and polynomial fits
%% entropy
% compute constants for translational component (chemical constant is
% determined by degeneracy of electronic state and included with the
% partition function)
s_st = 2.5 + 1.5*log(2*pi*amu*m/h/h)-log(P_zero);
S_R = T.*dqdT./q +  log(q) + s_st + 2.5*log(kb*T) ;
%% interpolate values at standard state
H_RT_zero = interp1(T,H_RT,T_zero,'pchip');
CP_R_zero = interp1(T,CP_R,T_zero,'pchip');
S_R_zero = interp1(T,S_R,T_zero,'pchip');
H_zero = H_RT_zero*Ra*T_zero;
%% Plotting partition function vs temperature
if (plot_part)
    figure('Name', 'Diatomic Molecule - statistical mechanics');
    maxy = max(q(:));
    %miny = min(q(:));
    fontsize=12;
    axis([Tmin Tmax 1E-5 maxy]);
    semilogy(T(:),q(:),'k','LineWidth',2 );
    xlabel('T (K)','FontSize',fontsize);
    ylabel('q','FontSize',fontsize);
    title('Partition function - internal only','FontSize',fontsize);
    
    % plot internal energy vs temperature
    figure('Name', 'Diatomic Molecule - statistical mechanics');
%     maxy = max(U_RT(1,:));
%     miny = min(U_RT(1,:));
     U_int = U - 1.5*Ra*T;
     maxy = max(U_int(1,:));
     miny = min(U_int(1,:));
    fontsize=12;
    axis([Tmin Tmax miny maxy]);
    plot(T(:)',U_int(:)/1E6','k','LineWidth',2);
    xlabel('T (K)','FontSize',fontsize);
    ylabel('U_int (MJ/kmol)','FontSize',fontsize);
    title('internal energy','FontSize',fontsize);
%     
    % plot specific heat vs temperature
    figure('Name', 'Diatomic Molecule - statistical mechanics');
    maxy = max(CV_R(:));
    miny = min(CV_R(:));
    fontsize=12;
    axis([Tmin Tmax miny maxy]);
    set(gca,'FontSize',fontsize,'LineWidth',2);
    plot(T(:),CV_R(:),'k');
    xlabel('T (K)','FontSize',fontsize);
    ylabel('CV_/R','FontSize',fontsize);
    title('internal specific heat','FontSize',fontsize);
end
%% compare with NASA polynomial fits in specified mechanism file
if (compare)
    mech = [mechanism '.cti'];
    gas = Solution(mech);
    i_cti_species = speciesIndex(gas,cti_species);
    X = [cti_species,':1.0'];
    for i=1:imax
        set(gas, 'T', T(i), 'P', oneatm, 'X', X);
        cp = cp_R(gas);
        cp_r(i) = cp(i_cti_species);
        h_r = enthalpies_RT(gas);
        h_rt(i)=h_r(i_cti_species);
        S_r = entropies_R(gas);
        u_rt(i) = h_rt(i)-1;
        s_r(i) = S_r(i_cti_species);
    end
    cv_r = cp_r -1;
    set(gas, 'T', T_zero, 'P', oneatm, 'X', X);
    cp = cp_R(gas);
    cp_r_zero = cp(i_cti_species);
    h_r = enthalpies_RT(gas);
    h_rt_zero = h_r(i_cti_species);
    DH_f0 = h_rt_zero*Ra*T_zero;
    h_zero = DH_f0;
    S_r = entropies_R(gas);
    s_r_zero = S_r(i_cti_species);
    u_rt_zero = h_rt_zero-1;
       % plot thermo data vs temperature 
    index_max = length(T);
    figure('Name','Cantera thermo vs statistical mechanics' );
    a = max(cp_r(:));
    b = max(CP_R(:));
    maxy = max(a,b);
    miny = min(cp_r(:));
    maxx = max(T(:));
    minx = min(T(:));
    fontsize=12;
    axis([minx maxx miny maxy]);
    hold on
    set(gca,'FontSize',fontsize,'LineWidth',2);
    plot(T(:),cp_r(:));
    plot(T(1:20:index_max),CP_R(1:20:index_max),'ok');
    xlabel('T (K)','FontSize',fontsize);
    ylabel('C_p/R','FontSize',fontsize);
    legend('Reference data', 'statistical mechanics','Location','southeast');
    title(['Cantera data ', mechanism, ' species ', cti_species],'FontSize',fontsize);
    hold off
    
    % plot thermo data vs temperature
    figure('Name','Cantera thermo vs statistical mechanics' );
    maxy = max(H_RT(:));
    miny = min(min(H_RT(:)),0.);
    maxx = max(T(:));
    minx = min(T(:));
    fontsize=12;
    axis([minx maxx miny maxy]);
    hold on;
    set(gca,'FontSize',fontsize,'LineWidth',2);
    plot(T(:),h_rt(:));
    plot(T(1:20:index_max),H_RT(1:20:index_max),'ok');
    xlabel('T (K)','FontSize',fontsize);
    ylabel('h/RT','FontSize',fontsize);
    legend('Reference data', 'statistical mechanics','Location','northeast');
    title(['Cantera data ', mechanism, ' species ', cti_species],'FontSize',fontsize);
    hold off
    
    % plot thermo data vs temperature
    figure('Name','Cantera thermo vs statistical mechanics' );
    maxy = max(s_r(:));
    miny = min(s_r(:));
    maxx = max(T(:));
    minx = min(T(:));
    fontsize=12;
    axis([minx maxx miny maxy]);
    hold on;
    set(gca,'FontSize',fontsize,'LineWidth',2);
    plot(T(:),s_r(:));
    plot(T(1:20:index_max),S_R(1:20:index_max),'ok');
    xlabel('T (K)','FontSize',fontsize);
    ylabel('S/R','FontSize',fontsize);
    legend('Reference data', 'statistical mechanics','Location','southeast');
    title(['Cantera data ', mechanism, ' species ', cti_species],'FontSize',fontsize);
    hold off
    
    % plot errors in specific heat, internal energy and entropy
    Err_Cv =  (CV_R - cv_r)./CV_R;
    Err_U =  (U_RT-u_rt)./U_RT;
    Err_S = (S_R-s_r)./S_R;
    % plot thermo data errors vs temperature
    figure('Name','Cantera thermo vs statistical mechanics' );
    % miny=min(Err_U);
    % maxy=max(Err_U);
    % maxx = max(T(:));
    % minx = min(T(:));
    fontsize=12;
    % axis([minx maxx miny maxy]);
    hold on;
    set(gca,'FontSize',fontsize,'LineWidth',2);
    plot(T(:),Err_Cv(:));
    plot(T(:), Err_U(:));
    plot(T(:), Err_S(:));
    xlabel('T (K)','FontSize',fontsize);
    ylabel('Relative error','FontSize',fontsize);
    legend('Cv', 'U', 'S','Location','southeast');
    title(['Cantera data ', mechanism, ' species ', cti_species],'FontSize',fontsize);
    hold off
    cp_max_err = max(abs(cp_r-CP_R));
    u_max_err = max(abs(u_rt- U_RT));
    s_max_err = max(abs(s_r-S_R));
    display(['Difference between cantera and paritiion function:']);
    display(['Maximum difference in Cp/R ', num2str(cp_max_err)]);
    display(['Maximum difference in U/RT ', num2str(u_max_err)]);
    display(['Maximum difference in S/R ', num2str(s_max_err)]);
    if (cantera_out)
        display(['Writing cantera thermodynamics to csv text file']);
        d = datestr(now);
        species_clean = strrep(species,'*','star');
        fn =[dir_NAME,'/',species_clean,'-cantera','.csv'];
        fid = fopen(fn, 'w');
        fprintf(fid, '# Cantera Thermodynamics\n');
        fprintf(fid, '# Mechanism %s\n',mechanism);
        fprintf(fid, '# Computation done on %s\n', d);
        fprintf(fid, '# Species %s\n', species);
        fprintf(fid, '# Standard state T = %14.5e (K) P = %14.5e (Pa)\n',T_zero,P_zero);
        fprintf(fid, '# C_P        %14.5e (J/K mol)\n',cp_r_zero*Ra);
        fprintf(fid, '# H          %14.5e (J/mol)\n',h_zero);
        fprintf(fid, '# S          %14.5e (J/K mol)\n',s_r_zero*Ra);
        fprintf(fid, '# C_P/R      %14.5e\n',cp_r_zero);
        fprintf(fid, '# H/RT       %14.5e\n',h_rt_zero);
        fprintf(fid, '# S/R        %14.5e\n',s_r_zero);
        fprintf(fid, '#     T            C_P/R           H/RT            S/R\n');
        z = [T; cp_r; h_rt; s_r];
        fprintf(fid, '%14.5e, %14.5e, %14.5e, %14.5e\n',z); 
        fclose(fid);
    end
end
%% write out data file 
if (output)
    display(['Writing partition function thermodynamics to text file']);
    d = datestr(now);
    species_clean = strrep(species,'*','star');
    fn =[dir_NAME,'/',species_clean,'-partition','.csv'];
    fid = fopen(fn, 'w');
    fprintf(fid, '# Statistical Thermodynamics\n');
    fprintf(fid, '# Computation done on %s\n', d);
    fprintf(fid, '# Species %s\n', species);
    fprintf(fid, '# Standard state T = %14.5e (K) P = %14.5e (Pa)\n',T_zero,P_zero);
    fprintf(fid, '# C_P        %14.5e (J/K mol)\n',CP_R_zero*Ra);
    fprintf(fid, '# H          %14.5e (J/mol)\n',H_zero);
    fprintf(fid, '# S          %14.5e (J/K mol)\n',S_R_zero*Ra);
    fprintf(fid, '# C_P/R      %14.5e\n',CP_R_zero);
    fprintf(fid, '# H/RT       %14.5e\n',H_RT_zero);
    fprintf(fid, '# S/R        %14.5e\n',S_R_zero);
    fprintf(fid, '#     T            C_P/R           H/RT            S/R\n');
    z = [T; CP_R; H_RT; S_R];
    fprintf(fid, '%14.5e, %14.5e, %14.5e, %14.5e\n',z); 
    fclose(fid);
end
%% Fit data to polynomial format
if (fit)
    display(['Fitting partition function thermodynamics to NASA-7 polynomial']);
    Tmid = 1500.;  % conventional choice
    index_mid = find(T(:)<=Tmid, 1, 'last' );   %find closest temperature to selected value
    Tmid = T(index_mid:index_mid);
    display(['Adjusted temperature for midpoint of fit = ',num2str(Tmid)]);
    display(['Adjusted temperature for minimum of fit = ',num2str(Tmin)]);
    display(['Refitting...']);
    z = [T; CP_R; H_RT; S_R];
    % find specific heat derivative at midpoint
    DCPRDT = (CP_R(index_mid+1)-CP_R(index_mid-1))/(T(index_mid+1) - T(index_mid-1));
    [x, cp_fit, index_mid, a_0_6, a_1_6, s_fit,  a_0_7, a_1_7, h_fit] = poly_cp(z,Tmin,Tmid,Tmax,CP_R_zero,DCPRDT,S_R_zero,H_RT_zero);
    %% check fit against input data
    figure('Name','Polynomial fit vs statistical mechanics' );
    maxy = max(CP_R);
    miny = min(CP_R);
    maxx = max(T);
    minx = min(T);
    index_max = length(T);
    fontsize=12;
    axis([minx maxx miny maxy]);
    hold on
    set(gca,'FontSize',fontsize,'LineWidth',2);
    plot(T(1:index_mid),cp_fit(1:index_mid));
    plot(T(index_mid:index_max),cp_fit(index_mid+1:index_max+1));
    plot(T(1:20:index_max),CP_R(1:20:index_max),'ok');
    xlabel('T (K)','FontSize',fontsize);
    ylabel('C_p/R','FontSize',fontsize);
    legend('poly 1','poly 2','statistical mechanics','Location','southeast');
    title(['Partition vs Fit, species ', species],'FontSize',fontsize);
    hold off
    %
    % plot fits and compare to partition function results
    figure('Name','Polynomial fit vs statistical mechanics' );
    maxy = max(S_R);
    miny = min(S_R);
    maxx = max(T);
    minx = min(T);
    index_max = length(T);
    fontsize=12;
    axis([minx maxx miny maxy]);
    hold on
    set(gca,'FontSize',fontsize,'LineWidth',2);
    plot(T(1:index_mid),s_fit(1:index_mid));
    plot(T(index_mid:index_max),s_fit(index_mid+1:index_max+1));
    plot(T(1:20:index_max),S_R(1:20:index_max),'ok');
    xlabel('T (K)','FontSize',fontsize);
    ylabel('S/R','FontSize',fontsize);
    legend('poly 1','poly 2','statistical mechanics','Location','southeast');
    title(['Partition vs Fit, species ', species],'FontSize',fontsize);
    hold off
    
    figure('Name','Polynomial fit vs statistical mechanics' );
    maxy = max(H_RT);
    miny = min(H_RT);
    maxx = max(T);
    minx = min(T);
    index_max = length(T);
    fontsize=12;
    axis([minx maxx miny maxy]);
    hold on
    set(gca,'FontSize',fontsize,'LineWidth',2);
    plot(T(1:index_mid),h_fit(1:index_mid));
    plot(T(index_mid:index_max),h_fit(index_mid+1:index_max+1));
    plot(T(1:20:index_max),H_RT(1:20:index_max),'ok');
    xlabel('T (K)','FontSize',fontsize);
    ylabel('H/RT','FontSize',fontsize);
    legend('poly 1','poly 2','statistical mechanics','Location','southeast');
    title(['Partition vs Fit, species ', species],'FontSize',fontsize);
    hold off
    
    %% Check for jumps and maximum errors
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
    cp_max_err = max(abs(cp_fit'-CP_R));
    h_max_err = max(abs(h_fit'- H_RT));
    s_max_err = max(abs(s_fit'-S_R));
    display(['Maximum error in Cp/R ', num2str(cp_max_err)]);
    display(['Maximum error in H/RT ', num2str(h_max_err)]);
    display(['Maximum error in S/R ', num2str(s_max_err)]);
    %% write coefficient files for Cantera and Chemkin
    % create coefficient in correct order for NASA-7 format
    %The NASA-7 polynomials are of the form:
    %Cp/R = a1 + a2 T + a3 T^2 + a4 T^3 + a5 T^4
    %H/RT = a1 + a2 T /2 + a3 T^2 /3 + a4 T^3 /4 + a5 T^4 /5 + a6/T
    %S/R  = a1 lnT + a2 T + a3 T^2 /2 + a4 T^3 /3 + a5 T^4 /4 + a7
    % a = [x(1:5)',a_0_6,a_0_7,x(6:10)',a_1_6,a_1_7];
    a = [x(1:5)',a_0_6,a_0_7,x(6:10)',a_1_6,a_1_7];
    %% Cantera output file
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
        outputfilename =[dir_NAME,'/',species_clean,'-partition','.cti'];
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
            '        note = ''Partition fit by JES ', datestr(now),''')']);
        fclose(outputfile);
    end
    %% NASA 7 (Chemkin II) format output file
    if (nasa7)
        display(['Writing NASA (Chemkin II) format .dat file for species ',species]);
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
        outputfilename =[dir_NAME,'/',species_clean,'-partition-NASA7','.dat'];
        outputfile = fopen(outputfilename, 'w');
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
        outputfilename =[dir_NAME,'/',species_clean,'-partition-NASA9','.dat'];
        outputfile = fopen(outputfilename, 'w');
        %            fprintf(outputfile,'thermo nasa9\n);
        space = '      ';
        % record 1
        fprintf(outputfile, '%s%s%s\n',name_field,'Partition fit by JES ', datestr(now));
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
    

