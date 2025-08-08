%% Molecular data for rotational-vibrational partition function computation
% Comparision file  (Cantera .cti file)
mechanism='diatomic';    
cti_species = 'CH';
% data for ground and first four excited states of CH  (cm^-1 energy units)
species = 'CH';
name = 'methylidyne';
dir_NAME = name;
status = mkdir(dir_NAME);
m = 13.0186;  % mass in amu
ne = 2;  % number of elements
na = 2;   % number of atoms in molecule
els = ['C';   'H' ];  % array specifying element composition
at = [1; 1];  % array of element composition coefficients
%% ** All spectroscopic data for the molecule are stored in structure  'molecule(i)' for ith electronic level ***
% Notation
%Y = Dunham coefficients, these correspond to more conventional
%spectroscopic notation as follows:
% the vibrational contribution to internal energy is G(n) = sum_{i=1}^7 Y(i, 1)(n+1/2)^i
%Y0 = Y(i,0) = [omega_e,  -omega_e x_e,  omega_e y_e,  omega_e z_e,  omega_e a_e,
%omega_e b_e, omega_e c_e]  i = 1, 7
% the rotational contribution to internal energy is F(J) = sum_{i=0}^5
% J(J=1) Y(1,i) (n+1/2)^i
% Y1 = Y(i, 1) = [B_e, -alpha_e, \gamma_e, \delta_e, constant_4 , constant_5 ] i = 0, 5
% usually there is also added a second order contribution to the rotational component of energy
% J^2(J+1)^2Y(i,2)(n+1/2)^(i-1)
% Y2 = Y(i,2) = [-D_e, -beta_e, ...]
%% ground electronic state X2Pi
molecule(1).name = 'CH(X)';
molecule(1).deltaH_f0 = 594.13E+03; %J/mol standard heat of formation at 298.15 K,
molecule(1).level = 'X2Pi';  % there are two electronic configurations spin 1/2 & 3/2 separated by 143 cm-1  see Carlone 1969
molecule(1).Te = 0.;  % electronic energy level 
molecule(1).D0 = 32517.7;  %bond dissociation energy from table
molecule(1).Y0 = [2860.7508, -64.4387, 0.3634, 0., 0., 0., 0.]; % vibrational Dunham coefficients
molecule(1).Y1 = [14.45988, -0.53654, 0., 0., 0., 0.]; % rotational Dunham coefficients
molecule(1).Y2 = [-1.45E-3, 0.0, 0., 0., 0., 0.];  % centrifugal distortion Dunham coefficients
molecule(1).g = 4.; %degeneracy of electronic state  2(2S+1) for Pi states (2S+1) = 2
molecule(1).ref ='https://webbook.nist.gov/cgi/cbook.cgi?ID=C3315375&Units=SI&Mask=1000#Diatomic';
% compute zero point energy
G_0 = 0.0;
for i = 1:7
    G_0 = G_0 +  molecule(1).Y0(i)*.5^i;
end
molecule(1).De =  molecule(1).D0 + G_0;  % well depth computed from D0 + 1/2 hv_o
molecule(1).theta = 2*G_0*c2;  %  characteristic vibrational temperature
%% electronic state a4Sigma-
molecule(2).name = 'CH(a)';
molecule(2).level = 'a4Sigma-';
molecule(2).Te = 6025.; % electronic energy level
molecule(2).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(2).Te; %J/mol standard heat of formation at 298.15 K
molecule(2).De = 2.3092e+04; %Well depth
molecule(2).Y0 = [3090.9, -102.17, 0., 0., 0., 0., 0.]; % vibrational Dunham coefficients
molecule(2).Y1 = [15.4, -0.55, 0., 0., 0., 0.]; % rotational Dunham coefficients
molecule(2).Y2 = [-1.519E-3, 0., 0., 0., 0., 0.];  % centrifugal distortion Dunham coefficients
molecule(2).g = 4.; %degeneracy of electronic state (2S+1) for Sigma states  2S+1 = 4
molecule(2).ref ='https://webbook.nist.gov/cgi/cbook.cgi?ID=C3315375&Units=SI&Mask=1000#Diatomic and Kalemos 1999';
% compute zero point energy
G_0 = 0.0;
for i = 1:7
    G_0 = G_0 +  molecule(2).Y0(i)*.5^i;
end
molecule(2).D0 =  molecule(2).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
molecule(2).theta = 2*G_0*c2;  %  characteristic vibrational temperature
%%  electronic state A2Delta
molecule(3).name = 'CH(A)';
molecule(3).level = 'A2Delta';
molecule(3).Te = 23189.8; % electronic energy level
molecule(3).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(3).Te; %J/mol standard heat of formation at 298.15 K
molecule(3).De = 1.5929e+04; %well depth
molecule(3).Y0 = [2930.7, -96.65, 0., 0., 0., 0., 0.]; % vibrational Dunham coefficients
molecule(3).Y1 = [14.934, -0.697, 0., 0., 0., 0.]; % rotational Dunham coefficients
molecule(3).Y2 = [-0.00154, 0., 0., 0., 0., 0.];  % centrifugal distortion Dunham coefficients
molecule(3).g = 4.; %degeneracy of electronic state 2(2S+1) for Delta states  2S+1 = 2
molecule(3).ref ='https://webbook.nist.gov/cgi/cbook.cgi?ID=C3315375&Units=SI&Mask=1000#Diatomic';
% compute zero point energy
G_0 = 0.0;
for i = 1:7
    G_0 = G_0 +  molecule(3).Y0(i)*.5^i;
end
molecule(3).D0 =  molecule(3).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
molecule(3).theta = 2*G_0*c2;  %  characteristic vibrational temperature
%% electronic state B2Sigma+
molecule(4).name = 'CH(B)';
molecule(4).level = 'B2Sigma-';
molecule(4).Te = 26140.; % electronic energy level
molecule(4).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(4).Te; %J/mol standard heat of formation at 298.15 K
molecule(4).De = 3000.;  % well depth from Kalemos et al  1999
molecule(4).Y0 = [2167.1, -173.72, 0.0, 0.0, 0.0, 0.0, 0.0]; % vibrational Dunham  coefficients
molecule(4).Y1 = [12.645, -1.11, 0.0, 0.0, 0.0, 0.0];  % rotational Dunham coefficients
molecule(4).Y2 = [-2.22E-3, 0.0, 0.0, 0.0, 0.0, 0.0];  % centrifugal distortion Dunham coefficients
molecule(4).g = 2.; %degeneracy of electronic state
molecule(4).ref ='Kalemos et al  1999';
% compute zero point energy
G_0 = 0.0;
for i = 1:7
    G_0 = G_0 +  molecule(4).Y0(i)*.5^i;
end
molecule(4).D0 =  molecule(4).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
molecule(4).theta = 2*G_0*c2;  %  characteristic vibrational temperature
%% C2Sigma+
molecule(5).name = 'CH(C)';
molecule(5).level = 'C2Sigma+';
molecule(5).Te = 32125; % electronic energy level  Kalemos et al  1999
molecule(5).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(5).Te; %J/mol standard heat of formation at 298.15 K
molecule(5).De = 7339.6;  % Kalemos et al  1999
molecule(5).Y0 = [2840.2, -125.96, 13.55, 0.0, 0.0, 0.0, 0.0];   % vibrational Dunham  coefficients
molecule(5).Y1 = [14.603, -0.7185, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
molecule(5).Y2 = [-1.555E-3, 0.0, 0.0, 0.0, 0.0, 0.0];  % centrifugal distortion Dunham coefficients
molecule(5).g = 2.; %degeneracy of electronic state
molecule(5).ref ='Kalemos et al  1999';
% compute zero point energy
G_0 = 0.0;
for i = 1:7
    G_0 = G_0 +  molecule(5).Y0(i)*.5^i;
end
molecule(5).D0 =  molecule(5).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
molecule(5).theta = 2*G_0*c2;  %  characteristic vibrational temperature
%% E2Pi
molecule(6).name = 'CH(E)';
molecule(6).level = 'E2Pi';
molecule(6).Te = 5.9346e+04; % electronic energy level  Kalemos et al  1999
molecule(6).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(6).Te; %J/mol standard heat of formation at 298.15 K
molecule(6).De = 7339.6;  % Kalemos et al  1999  THIS IS VALUE FOR C STATE E is unknown
molecule(6).Y0 = [2743.0, -125.96, 0., 0.0, 0.0, 0.0, 0.0];   % vibrational Dunham  coefficients
molecule(6).Y1 = [12.6, -0.7185, 0.0, 0.0, 0.0, 0.0];  %  rotational Dunham coefficients, alpha_e VALUE FOR C STATE, values for E is unknown
D1 = 4*molecule(5).Y1(1)^3/molecule(5).Y0(1)^2;  % Kratzer relationship (Bernath, (6.48))
molecule(6).Y2 = [-D1, 0.0, 0.0, 0.0, 0.0, 0.0];   % centrifugal distortion Dunham coefficients
molecule(6).g = 4.; %degeneracy of electronic state
molecule(6).ref ='NIST webbook, Kalemos et al  1999';
% compute zero point energy
G_0 = 0.0;
for i = 1:7
    G_0 = G_0 +  molecule(6).Y0(i)*.5^i;
end
molecule(6).D0 =  molecule(6).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
molecule(6).theta = 2*G_0*c2;  %  characteristic vibrational temperature
