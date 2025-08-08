% Shock and Detonation Toolbox Demo Program
% 
% Computes ZND and CV models of detonation with the shock front
% traveling at the CJ speed.  Evaluates various measures of the reaction
% zone thickness and exothermic pulse width, effective activation energy
% and Ng stability parameter. 
%  
% ################################################################################
% Theory, numerical methods and applications are described in the following report:
% 
% “SDToolbox: Numerical Tools for Shock and Detonation Wave
% Modeling.” Explosion Dynamics Laboratory. Contributors: S.T.
% Browne, J.L. Zeigler, N.P. Bitter, B.E. Schmidt, J. Lawson, J.E.
% Shepherd. GALCIT Report FM2018.001. California Institute of
% Technology, Pasadena, CA. 2020.
% 
% Please cite this report and the website if you use these routines.
% 
% Please refer to LICENCE.txt or the above report for copyright and disclaimers.
% 
% http://shepherd.caltech.edu/EDL/PublicResources/sdt/
% 
% ################################################################################
% Updated Feb 2021 Tested with:
%     MATLAB 2020b, Cantera 2.4
% Under these operating systems:
%     Windows 10
clear;clc; close all;
disp('demo_ZND_CJ_cell')

P1 = 100000; T1 = 300; 
q = 'H2:2 O2:1 N2:3.76';
mech = 'Mevel2017.cti'; 

fname = 'h2air';

[cj_speed, curve, ~, dnew, plot_data] = CJspeed(P1, T1, q, mech);
%CJspeed_plot(2,plot_data,curve,dnew)

gas1 = Solution(mech);
set(gas1, 'T', T1, 'P', P1, 'X', q);

% FIND EQUILIBRIUM POST SHOCK STATE FOR GIVEN SPEED
[gas] = PostShock_eq(cj_speed, P1, T1, q, mech);
u_cj = cj_speed*density(gas1)/density(gas);

% FIND FROZEN POST SHOCK STATE FOR GIVEN SPEED
[gas] = PostShock_fr(cj_speed, P1, T1, q, mech);

% SOLVE ZND DETONATION ODES
[out] = zndsolve(gas,gas1,cj_speed,'advanced_output',true,'t_end',2e-3);

% Find CV parameters including effective activation energy
set(gas,'Temperature',T1,'Pressure',P1,'X',q);
gas = PostShock_fr(cj_speed, P1, T1, q, mech);
Ts = temperature(gas); Ps = pressure(gas);
Ta = Ts*(1.02);
set(gas, 'T', Ta, 'P', Ps, 'X', q);
[CVout1] = cvsolve(gas);
Tb = Ts*(0.98);
set(gas, 'T', Tb, 'P', Ps, 'X', q);
[CVout2] = cvsolve(gas);
% Approximate effective activation energy for CV explosion
taua = CVout1.ind_time;
taub = CVout2.ind_time;
if(taua==0 || taub==0)
    theta_effective_CV = 0;
else
    theta_effective_CV = 1/Ts*((log(taua)-log(taub))/((1/Ta)-(1/Tb)));
end
%  Find Gavrikov induction length based on 50% limiting species consumption,
%  fuel for lean mixtures, oxygen for rich mixtures
%  Westbrook time based on 50% temperature rise
limit_species = 'H2';
i_limit = speciesIndex(gas,limit_species);
set(gas,'Temperature',Ts,'Pressure',Ps,'X',q);
X = moleFractions(gas);
X_initial = X(i_limit);
equilibrate(gas,'UV');
X = moleFractions(gas);
X_final = X(i_limit);
T_final = temperature(gas);
X_gav = 0.5*(X_initial - X_final) + X_final;
T_west = 0.5*(T_final - Ts) + Ts;
b = length(CVout1.speciesX(:,i_limit));
for i = 1:b
    if (CVout1.speciesX(i:i,i_limit) > X_gav)
        t_gav = CVout1.time(i);
    end
end
x_gav = t_gav*out.U(1);
for i = 1:b
    if (CVout1.T(i) < T_west)
        t_west = CVout1.time(i);
    end
end
x_west = t_west*out.U(1);

max_thermicity_width_ZND = u_cj/out.max_thermicity_ZND;   %Ng et al definition
chi_ng = theta_effective_CV*out.ind_len_ZND/max_thermicity_width_ZND;
cell_gav = gavrikov(x_gav,theta_effective_CV, Ts, T1);
cell_ng = ng(out.ind_len_ZND, chi_ng);

b = length(out.T);
disp(['ZND computation results; ']);
disp(['Mixture ', q]);
disp(['Mechanism ', mech]);
disp(['Initial temperature ',num2str(T1,'%8.3e'),' K']);
disp(['Initial pressure ',num2str(P1,'%8.3e'),' Pa']);
disp(['CJ speed ',num2str(cj_speed,'%8.3e'),' m/s']);
disp([' ']);
disp(['Reaction zone computation end time = ',num2str(out.tfinal,'%8.3e'),' s']);
disp(['Reaction zone computation end distance = ',num2str(out.xfinal,'%8.3e'),' m']);
disp([' ']);
disp(['T (K), initial = ' num2str(out.T(1),5) ', final ' num2str(out.T(b),5) ', max ' num2str(max(out.T(:)),5)]);
disp(['P (Pa), initial = ' num2str(out.P(1),3) ', final ' num2str(out.P(b),3) ', max ' num2str(max(out.P(:)),3)]);
disp(['M, initial = ' num2str(out.M(1),3) ', final ' num2str(out.M(b),3) ', max ' num2str(max(out.M(:)),3)]);
disp(['u (m/s), initial = ' num2str(out.U(1),5) ', final ' num2str(out.U(b),5) ', cj ' num2str(u_cj,5)]);
disp([' ']);
disp(['Reaction zone thermicity half-width = ',num2str(out.exo_len_ZND,'%8.3e'),' m']);
disp(['Reaction zone maximum thermicity distance = ',num2str(out.ind_len_ZND,'%8.3e'),' m']);
disp(['Reaction zone thermicity half-time = ',num2str(out.exo_time_ZND, '%8.3e'),' s']);
disp(['Reaction zone maximum thermicity time = ',num2str(out.ind_time_ZND,'%8.3e'),' s']);
disp(['Reaction zone width (u_cj/sigmadot_max) = ',num2str(max_thermicity_width_ZND,'%8.3e'),' m']);
disp([' ']);
disp(['CV computation results; ']);
disp(['Time to dT/dt_max = ',num2str(CVout1.ind_time, '%8.3e'),' s']);
disp(['Distance to dT/dt_max = ',num2str(CVout1.ind_time*out.U(1), '%8.3e'),' m']);
disp(['Reduced activation energy) = ',num2str(theta_effective_CV,'%8.3e')]);
disp(['Time to 50% consumption = ',num2str(t_gav, '%8.3e'),' s']);
disp(['Distance to 50% consumption = ',num2str(x_gav, '%8.3e'),' m']);
disp(['Time to 50% temperature rise = ',num2str(t_west, '%8.3e'),' s']);
disp(['Distance to 50% temperature = ',num2str(x_west, '%8.3e'),' m']);
disp([' ']);
disp(['Cell size predictions ']);
disp(['Gavrikov correlation ',num2str(cell_gav, '%8.3e'),' m']);
disp(['Ng et al Chi Parameter ',num2str(chi_ng, '%8.3e'),' m']);
disp(['Ng et al correlation ',num2str(cell_ng, '%8.3e'),' m']);
disp(['Westbrook correlation ',num2str(29*x_west, '%8.3e'),' m']);


znd_plot(out,'maxx',1,'major_species',{'H2', 'O2', 'H2O'},...
    'minor_species',{'H', 'O', 'OH', 'H2O2', 'HO2'});

function lambda = gavrikov(delta,theta, Tvn, T0)
% Correlation function for detonation cell width 
% proposed by Gavrikov et al COMBUSTION AND FLAME 120:19�33 (2000)
% based on using a reaction zone length based on time to 50% limiting
% reactant consumption in constant volume explosion approximation using vn
% postshock velocity to convert time to distance.   Tested against a range
% of fuel-oxidizer diluent mixtures
%
% Inputs:
% delta = reaction zone length based on time to 50% consumption of limiting
% reactant from CV computation and delta = time * w_VN
% theta = Ea/RT_VN,  effective reduced activation energy based on CV
% computation
% Tvn = von Neumann (postshock temperature behind CJ shock wave)
% T0 = initial temperature
%
% Constants
a = -0.007843787493;
b = 0.1777662961;
c = 0.02371845901;
d = 1.477047968;
e = 0.1545112957;
f = 0.01547021569;
g = -1.446582357;
h = 8.730494354;
i = 4.599907939;
j = 7.443410379;
k = 0.4058325462;
m = 1.453392165;
%  define nondimensional parameters
X = theta;
Y = Tvn/T0;
z = Y*(a*Y-b) + X*(c*X-d + (e-f*Y)*Y) + g*log(Y)+ h*log(X) + Y*(i/X - k*Y/X^m) - j;
lambda = delta*10^(z);
end

function lambda = ng(delta,chi)
%correlation function for detonation cell size from
% Ng, Hoi Dick, Yiguang Ju, and John H. S. Lee. 2007. Assessment of
% Detonation Hazards in High-Pressure Hydrogen Storage from Chemical
% Sensitivity Analysis. INTERNATIONAL JOURNAL OF HYDROGEN ENERGY 32 (1):
% 93-99.
% Tested only against low pressure H2-air data
% Inputs:
% delta = reaction zone length based on peak thermicity in ZND simulation
% chi = theta*Delta_i/Delta_r where 
%       theta = reduced effective activation energy from CV computation
%       Delta_i = distance to peak thermicity from ZND computation
%       Delta_r = w_vN/\sigmadot_max from ZND computation
% See Ng et al.  Combustion Theory and Modeling 2005 for a discussion of
% the chi parameter.  
%
% Constants
A0 = 30.465860763763;
a1 = 89.55438805808153;
a2 = -130.792822369483;
a3 = 42.02450507117405;
b1 = -0.02929128383850;
b2 = 1.0263250730647101E-5;
b3 = -1.031921244571857E-9;
lambda = delta*(A0 + ((a3/chi + a2)/chi + a1)/chi + ((b3*chi + b2)*chi + b1)*chi);
end
