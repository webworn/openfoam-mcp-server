% Shock and Detonation Toolbox Demo Program
% 
% This is a demostration of how to compute basic explosion parameters (UV,
% HP and ZND) as a function of fuel mole fraction for a specified fuel,
% oxider and diluent. Creates a set of plots and an output file.
% 'ExplosionParameters.txt'.
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
disp('Explosion Parameters')

P1 = oneatm; T1 = 300;
mech = 'Mevel2017.cti';
gas  = Solution(mech);
gas1 = Solution(mech);
gas2 = Solution(mech);
nsp  = nSpecies(gas);

% plots?
plots = true;
% output file
output = true;

% set initial species to be varied
fuel = 'H2';
oxidizer = 'O2';
diluent = 'N2';

% find species indices
ifuel = speciesIndex(gas,fuel);
ioxidizer  = speciesIndex(gas,oxidizer);
idiluent = speciesIndex(gas,diluent);

disp('Initial Conditions');
disp(['   Pressure = ',num2str(P1), ' (Pa) Temperature = ', num2str(T1), ' (K)']);
disp(['   Mechanism: ',mech]);
disp(['Initial Species in Mixture ']);
disp(['   fuel ',fuel]);
disp(['   oxidizer ',oxidizer]);
disp(['   diluent ',diluent]);
disp('--------------------------------------');

%% define number of points, range of equivalence ratio, stoichiometric
% fuel-oxidizer molar ratio, diluent-oxidizer ratio
npoints = 10;
fuelmin = 0.15;
fuelmax = 0.6;
deltafuel = (fuelmax - fuelmin)/(npoints-1);
stoich = 2.0;   %ratio of fuel to oxidizer for stoichiometric mixture
beta = 3.76;  %ratio of diluent to oxidizer 

%% loop through cases
%
phi = [];
xfuel = [];
xoxidizer = [];
xdiluent = [];
disp(['Computing parameters for  ', num2str(npoints), ' cases']);
disp('--------------------------------------');
for i = 1:npoints
    % define composition as xfuel*F + xoxidizer*(O + beta*D)
    x = zeros(nsp, 1);
    xfuel(i) = fuelmin + (i-1)*deltafuel;
    xoxidizer(i) = (1-xfuel(i))/(1+beta);
    xdiluent(i) = beta*xoxidizer(i);
    phi(i) = xfuel(i)/(stoich*xoxidizer(i));
    x(ifuel,1) = xfuel(i);
    x(ioxidizer,1) = xoxidizer(i);
    x(idiluent,1) = xdiluent(i);
    disp(['Case ', num2str(i),' Fuel concentration: ',num2str(x(ifuel,1)*100),'%' ]);
    disp('Molefractions');
    disp(['   fuel (',fuel, '): ', num2str(x(ifuel,1))]);
    disp(['   oxidizer (',oxidizer,'): ', num2str(x(ioxidizer,1))]);
    disp(['   diluent (',diluent,'): ', num2str(x(idiluent,1))]);
    disp(['   Phi = ', num2str(phi(i))]);
    set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',x);
    init_rho = density(gas);
    init_af = soundspeed_fr(gas);
    
    % Constant Pressure Explosion State
    set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',x);
    equilibrate(gas,'HP');
    hp_rho(i) = density(gas);
    hp_exp(i) = init_rho/hp_rho(i);
    hp_T(i) = temperature(gas);
    hp_ae(i) = soundspeed_eq(gas);
    
    % Constant Volume Explosion State
    set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',x);
    equilibrate(gas,'UV');
    uv_P(i) = pressure(gas);
    uv_T(i) = temperature(gas);
    uv_ae(i) = soundspeed_eq(gas);
    
    % CJ speed
    %set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',x);
    [Ucj(i)] = CJspeed(P1, T1, x, mech);

    % vN state
    %set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',x);
    [gas1] = PostShock_fr(Ucj(i), P1, T1, x, mech);
    vn_T(i) = temperature(gas1);
    vn_P(i) = pressure(gas1);
    vn_rho(i) = density(gas1);
    vn_af(i) = soundspeed_fr(gas1);

    % ZND Structure
    set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',x);
    [ZNDout] = zndsolve(gas1,gas,Ucj(i),'advanced_output',true);
    ind_len_ZND(i) = ZNDout.ind_len_ZND;
    exo_len_ZND(i) = ZNDout.exo_len_ZND;

    % CJ state
    %set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',x);
    [gas1] = PostShock_eq(Ucj(i),P1, T1,x, mech);
    cj_T(i) = temperature(gas1);
    cj_P(i) = pressure(gas1);
    cj_rho(i) = density(gas1);
    cj_af(i) = soundspeed_fr(gas1);
    % Reflected CJ state
    [ref_P(i),Uref(i),gas2] = reflected_eq(gas,gas1,gas2,Ucj(i));

    % State 3 - Plateau at end of Taylor wave
    %disp(['Generating points on isentrope and computing Taylor wave velocity'])
    w2 = density(gas)*Ucj(i)/cj_rho(i);
    S2 = entropy_mass(gas1);
    u2 = Ucj(i) - w2;
    u(1) = u2;
    P(1) = pressure(gas1);
    R(1) = density(gas1);
    V(1) = 1./R(1);
    T(1) = temperature(gas1);
    a(1) = soundspeed_eq(gas1);
    vv = 1/cj_rho(i);
    k = 1;
    while (u(k)>0)
        k = k+1;
        vv = vv*1.01;
        sv = [S2, vv];
        setState_SV(gas1,sv);
        equilibrate(gas1,'SV');
        P(k) = pressure(gas1);
        R(k) = density(gas1);
        V(k) = 1./R(k);
        a(k) = soundspeed_eq(gas1);
        u(k) = u(k-1) + 0.5*(P(k)-P(k-1))*(1./(R(k)*a(k)) + 1./(R(k-1)*a(k-1)));
    end
    % estimate plateau conditions (state 3) by interpolating to find the u = 0 state.
    nfinal = k;
    P3(i) = P(nfinal) + u(nfinal)*(P(nfinal-1)-P(nfinal))/(u(nfinal-1)-u(nfinal));
    if i < npoints
        clear P R V a u;
    end
    %Approximate the effective activation energy using finite differences
    set(gas,'Temperature',T1,'Pressure',P1,'MoleFractions',x);
    gas = PostShock_fr(Ucj(i), P1, T1, x, mech);
    Ts = temperature(gas); Ps = pressure(gas);
    Ta = Ts*(1.02);
    set(gas, 'T', Ta, 'P', Ps, 'X', x);
    [CVout1] = cvsolve(gas);
    Tb = Ts*(0.98);
    set(gas, 'T', Tb, 'P', Ps, 'X', x);
    [CVout2] = cvsolve(gas);    
    % Approximate effective activation energy for CV explosion
    taua = CVout1.ind_time;
    taub = CVout2.ind_time;
    if(taua==0 || taub==0)
        theta_effective_CV(i) = 0;
    else
        theta_effective_CV(i) = 1/Ts*((log(taua)-log(taub))/((1/Ta)-(1/Tb))); 
    end
    disp('--------------------------------------');
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% CREATE OUTPUT TEXT FILE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if (output)
    fn = ['ExplosionParameters.txt'];
    d = datestr(now);
    fid = fopen(fn, 'w');
    fprintf(fid, '# Explosion Parameters\n');
    fprintf(fid, '# Computation done on %s\n', d);
    fprintf(fid, '# Mechanism %s\n',mech);
    fprintf(fid, '# Initial Species in Mixture \n');
    fprintf(fid, '# fuel %s\n',fuel);
    fprintf(fid, '# oxidizer %s\n',oxidizer);
    fprintf(fid, '# diluent %s\n',diluent);
    fprintf(fid, '# Initial conditions\n');
    fprintf(fid, '# Temperature (K) %4.1f\n', T1);
    fprintf(fid, '# Pressure (Pa) %2.1f\n\n', P1);
    fprintf(fid, ['Variables = "Equivalence Ratio","X fuel","X oxidizer","X diluent","CV Pressure","CV Temperature",' ...
        '"HP expansion","HP soundspeed","CJ Speed","CJ Pressure ","CJ sound speed","vN Pressure ",' ...
        '"Reflected Pressure","Reflected Speed","Plateau Pressure","Ind Length","Energy Length","Theta Effective"\n']);
    z = [phi; xfuel; xoxidizer; xdiluent; uv_P; uv_T; hp_exp; hp_ae; Ucj; cj_P; cj_af; vn_P; ref_P; Uref; P3; ind_len_ZND;...
        exo_len_ZND; theta_effective_CV];
    fprintf(fid, '%14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e \t %14.5e  \t %14.5e  \t %14.5e  \t %14.5e  \t %14.5e  \t %14.5e \t %14.5e \t %14.5e \t %14.5e\n', z);
    fclose(fid);
end

if (plots)
    % make plots
    xfuelp = 100*xfuel;
    figure('Name', 'Explosion Parameters')
    maxy = max([max(cj_T),max(hp_T),max(uv_T)]);
    miny = min([min(cj_T),min(hp_T),min(uv_T)]);
    maxx = max(xfuelp);
    minx = min(xfuelp);
    fontsize=12;
    hold on;
    xlabel('Fuel fraction (%)','FontSize',fontsize);
    ylabel('Temperature (K)','FontSize',fontsize);
    title('Explosion Temperature','FontSize',fontsize);
    axis([minx maxx miny maxy])
    set(gca,'FontSize',fontsize,'LineWidth',2);
    plot(xfuelp(:),cj_T(:),'r',xfuelp(:),hp_T(:),'g',xfuelp(:),uv_T(:),'b');
    legend('CJ', 'HP', 'UV');
    hold off;

    figure('Name', 'Explosion Parameters')
    maxy = max([max(cj_P),max(uv_P),max(ref_P),max(P3)]);
    miny = min([min(cj_P),min(uv_P),min(ref_P),min(P3)]);
    maxx = max(xfuelp);
    minx = min(xfuelp);
    fontsize=12;
    hold on;
    xlabel('Fuel fraction (%)','FontSize',fontsize);
    ylabel('Pressure (MPa)','FontSize',fontsize);
    title('Explosion Pressure','FontSize',fontsize);
    axis([minx maxx miny/1E6 maxy/1E6])
    set(gca,'FontSize',fontsize,'LineWidth',2);
    plot(xfuelp(:),ref_P(:)/1E6,'r',xfuelp(:),cj_P(:)/1E6,'b',xfuelp(:),uv_P(:)/1E6,'g',xfuelp(:),P3(:)/1E6,'c');
    legend('CJ ref','CJ', 'UV','P3');
    hold off;

    figure('Name', 'Explosion Parameters')
    maxy = max([max(Ucj),max(Uref),max(hp_ae)]);
    miny = min([min(Ucj),min(Uref),min(hp_ae)]);
    maxx = max(xfuelp);
    minx = min(xfuelp);
    fontsize=12;
    hold on;
    xlabel('Fuel fraction (%)','FontSize',fontsize);
    ylabel('speed (m/s)','FontSize',fontsize);
    title('Explosion Speed','FontSize',fontsize);
    axis([minx maxx miny maxy])
    set(gca,'FontSize',fontsize,'LineWidth',2);
    plot(xfuelp(:),Uref(:),'r',xfuelp(:),Ucj(:),'b',xfuelp(:),hp_ae(:),'g');
     legend('CJ ref','CJ', 'a_e (HP)');
    hold off;

    figure('Name', 'Explosion Parameters')
    maxy = max([max(ind_len_ZND),max(exo_len_ZND)]);
    miny = min([min(ind_len_ZND),min(exo_len_ZND)]);
    maxx = max(xfuelp);
    minx = min(xfuelp);
    fontsize=12;    
    xlabel('Fuel fraction (%)','FontSize',fontsize);
    ylabel('length (m)','FontSize',fontsize);
    title('Explosion Lengths','FontSize',fontsize);
    set(gca,'FontSize',fontsize,'LineWidth',2);
    semilogy(xfuelp(:),ind_len_ZND(:),'r',xfuelp(:),exo_len_ZND(:),'b');
    axis([minx maxx miny maxy])
    legend('\Delta_{\sigma}_{max}','\Delta_{\sigma}');
end

