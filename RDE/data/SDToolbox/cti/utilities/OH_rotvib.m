%% Molecular data for rotational-vibrational partition function computation
% Species OH
% Comparision file
mechanism='diatomic';
cti_species = 'OH';
species ='OH';
name = 'hydroxyl';
dir_NAME = name;
status = mkdir(dir_NAME);
m = 17.0073;  % mass in amu 
ne = 2;  % number of elements
na = 2;   % number of atoms in molecule
els = ['H';   'O' ];  % array specifying element composition
at = [1; 1];  % array of element composition coefficients
%% ** All spectroscopic data for the molecule are stored in structure  'molecule(i)' for ith electronic level ***
% Notation
%Y = Dunham coefficients, these correspond to more conventional
%spectroscopic notation as follows:
% the vibrational contribution to internal energy is G(n) = sum_{i=1}^7 Y(i, 1)(n+1/2)^i
%Y0 = Y(i,0) = [omega_e,  -omega_e x_e,  omega_e y_e,  omega_e z_e,  omega_e a_e,
%omega_e b_e, omega_e c_e]  i = 1, 7 
% the rotational contribution to internal energy is F(J) = sum_{i=0}^5
% J(J=1) Y(i,1) (n+1/2)^(i-1)
% Y1 = Y(i, 1) = [B_e, -alpha_e, \gamma_e, \delta_e, constant_4 , constant_5 ] i = 0, 5
% usually there is also added a second order contribution to the rotational component of energy 
% J^2(J+1)^2Y(i,2)(n+1/2)^(i-1)
% Y2 = Y(i,2) = [-D_e, -beta_e, ...]
%% data for ground state X2Pi (cm^-1 energy units)
    molecule(1).name = 'OH(X)';
    molecule(1).deltaH_f0 = 37.3E+03; %J/mol standard heat of formation at 298.15 K,
    molecule(1).level = 'X2Pi';  % there are two electronic configurations spin 1/2 & 3/2 separated by 143 cm-1  see Carlone 1969
    molecule(1).Te = 0.;  % electronic energy level
    molecule(1).De = 37275.;  %bond dissociation energy Do plus 1/2 hv_o  
    molecule(1).Y0 = [3737.7941, -84.9156, 0.558406, -2.59739E-2, -6.38868E-4, 1.340990E-5, -4.19977E-6];  % vibrational Dunham coefficients
    molecule(1).Y1 = [18.89638, -0.725042, 8.32930E-3, -9.50223E-4, 8.04060E-5, -5.86647E-6]; % rotational Dunham coefficients
    molecule(1).Y2 = [0.0, 0., 0., 0., 0., 0.]; % centrifugal distortion Dunham coefficient
    molecule(1).g = 4.; %degeneracy of electronic state  2(2S+1) for Pi states (2S+1) = 2
    molecule(1).ref ='Luque and Crossley, The Journal of Chemical Physics 109, 439 (1998)';
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(1).Y0(i)*.5^i;
    end
    molecule(1).D0 =  molecule(1).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(1).theta = 2*G_0*c2;  %  characteristic vibrational temperature 
    %% electronic state A2Sigma+ 
    molecule(2).name = 'OH(A)';
    molecule(2).level = 'A2Sigma+';
    molecule(2).Te = 32683.97; % electronic energy level
    molecule(2).deltaH_f0 = molecule(1).deltaH_f0+c1*molecule(2).Te ;   %J/mol standard heat of formation at 298.15 K, estimated 
    molecule(2).De = 20406.2; %bond dissociation energy Do plus 1/2 hv_o
    molecule(2).D0 = 18847.;  % from Carlone 1969 
    molecule(2).Y0 = [3178.3554, -92.68141, -1.77305, 0.307923, -3.45944E-2, 0.0, 0.0]; % vibrational Dunham coefficients
    molecule(2).Y1 = [17.38922, -0.858139, 2.45302E-02, -1.0576E-02, 1.74450E-03, -1.38486E-04]; % rotational Dunham coefficients
    molecule(2).Y2 = [0.0,0.,0.,0. 0.,0.]; % centrifugal distortion Dunham coefficient
    molecule(2).g = 2.; %degeneracy of electronic state (2S+1) for Sigma states (2S+1) = 2
    molecule(2).ref ='Luque and Crossley, The Journal of Chemical Physics 109, 439 (1998)';
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(2).Y0(i)*.5^i;
    end
    molecule(2).D0 =  molecule(2).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(2).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    %% electronic state B2Sigma+   
    % This level has a very shallow well and
    % likely only a few bound vibrational states: n=0, 1 according to Carlone
    % 1969  The dissociation 
    molecule(3).name = 'OH(B)';
    molecule(3).level = 'B2Sigma+';
    molecule(3).Te = 69774.; % electronic energy level  % uncertain! 
    molecule(3).deltaH_f0 = molecule(1).deltaH_f0+c1*molecule(3).Te; 
    molecule(3).De = 1685.;  % computed from v_o and D_o  (v_o = 660 cm-1)
%    moelcule(3).D0 = 1355.; % from Carlone 1969 but very uncertain (p. 73)
    molecule(3).Y0 = [976.0, -109.3, -20.99, 0.0, 0.0, 0.0, 0.0];   % vibrational Dunham coefficients
    molecule(3).Y1 = [5.086,    0.0, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    molecule(3).Y2 = [-9.29E-04, 0.0, 0.0, 0.0, 0.0, 0.0]; % centrifugal distortion Dunham coefficient
    molecule(3).g = 2.; %degeneracy of electronic state
    molecule(4).ref ='Carlone 1969 thesis';
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(3).Y0(i)*.5^i;
    end
    molecule(3).D0 =  molecule(3).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(3).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    %% electronic state C2Sigma+
    molecule(4).name = 'OH(C)';
    molecule(4).level = 'C2Sigma+';
    molecule(4).Te = 89500; % electronic energy level  Carlone 1969
    molecule(4).deltaH_f0 = molecule(1).deltaH_f0+c1*molecule(4).Te; 
    molecule(4).De = 30084.;  % computed from v_o and D_o
%    moelcule(4).D0 = 29418.; % from Carlone 1969 
    molecule(4).Y0 = [1232.9, -19.1, 0.0, 0.0, 0.0, 0.0, 0.0];   % vibrational Dunham coefficients
    molecule(4).Y1 = [4.247, -0.078, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    molecule(4).Y2 = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]; % centrifugal distortion Dunham coefficient
    molecule(4).g = 2.; %degeneracy of electronic state
    molecule(4).ref ='Carlone Thesis 1969';
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(4).Y0(i)*.5^i;
    end
    molecule(4).D0 =  molecule(4).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(4).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
  