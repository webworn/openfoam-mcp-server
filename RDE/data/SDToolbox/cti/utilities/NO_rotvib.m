%% Molecular data for rotational-vibrational partition function computation
% data for ground and excited states of NO  (cm^-1 energy units)
% Comparision file
mechanism='diatomic';
cti_species = 'NO';
% Necessary input data
species = 'NO';
name = 'nitric_oxide';
dir_NAME = name;
status = mkdir(dir_NAME);
m = 30.0061;  % mass in amu
ne = 2;  % number of elements
na = 2;   % number of atoms in molecule
els = ['N';   'O' ];  % array specifying element composition
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
    molecule(1).name = 'NO(X)';
    molecule(1).deltaH_f0 = 91.123E+03; %J/mol standard heat of formation at 298.15 K,  ATC reccommendation
    molecule(1).level = 'X2Pi';  % 
    molecule(1).Te = 0.;  % electronic energy level
    molecule(1).De = 53334.;  % Depth of potential well (Park 1990)     
    molecule(1).Y0 = [1904.20, -14.075, 0.0, 0.0, 0.0, 0.0, 0.0]; % vibrational Dunham coefficients
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(1).Y0(i)*.5^i;
    end
    molecule(1).D0 =  molecule(1).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(1).theta = 2*G_0*c2;  %  characteristic vibrational temperature 
    molecule(1).Y1 = [1.67195, -0.0171, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    molecule(1).Y2 = [-5.4E-7, 0.0, 0.0, 0.0, 0.0, 0.0];  %centrifugal distortion dunham coefficients
    molecule(1).g = 4.; %degeneracy of electronic state  2(2S+1) for Pi states (2S+1) = 2
    molecule(1).ref ='NIST Webbook';
    %% first main electronic state A2Sigma+ 
    molecule(2).name = 'NO(A)';
    molecule(2).level = 'A2Sigma+';
    molecule(2).Te = 43966.; % electronic energy level  
    molecule(2).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(2).Te; %J/mol standard heat of formation at 298.15 K
    molecule(2).De = 28603.; % Depth of potential well (Park 1990) 
    molecule(2).Y0 = [2374.3, -16.11, 0.0, 0.0, 0.0, 0.0, 0.0]; % vibrational Dunham coefficients
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(2).Y0(i)*.5^i;
    end
    molecule(2).D0 =  molecule(2).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(2).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    molecule(2).Y1 = [1.9965, -0.01915, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    molecule(2).Y2 = [-5.4E-6, 0.0, 0.0, 0.0, 0.0, 0.0];  %centrifugal distortion Dunham coefficients
    molecule(2).g = 2.; %degeneracy of electronic state (2S+1) for Sigma states (2S+1) = 2
    molecule(2).ref ='Park 1990 Table 1.4, NIST chemistry webbook';
   %% electronic state a4Pi - between X and A, small well depth but important at high temperature 
    molecule(3).name = 'NO(a)';
    molecule(3).level = 'a4Pi';
    molecule(3).Te = 38807.; % electronic energy level  
    molecule(3).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(3).Te; %J/mol standard heat of formation at 298.15 K
    molecule(3).De = 14537.; % Depth of potential well (Park 1990) 
    molecule(3).Y0 = [1019., -12.8, 0.0, 0.0, 0.0, 0.0, 0.0]; 
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(3).Y0(i)*.5^i;
    end
    molecule(3).D0 =  molecule(3).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(3).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    molecule(3).Y1 = [1.12175, 0.0, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    molecule(3).Y2 = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0];  %centrifugal distortion Dunham coefficients
    molecule(3).g = 8.; %degeneracy of electronic state 2(2S+1) for Pi states 2(2S+1) = 8
    molecule(3).ref ='Park 1990 Table 1.4';
   %% electronic state B2Pi - slightly above A, small well depth but important at high temperature 
    molecule(4).name = 'NO(B)';
    molecule(4).level = 'B2Pi';
    molecule(4).Te = 45932.; % electronic energy level  
    molecule(4).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(4).Te; %J/mol standard heat of formation at 298.15 K
    molecule(4).De = 26637.; % Depth of potential well (Park 1990) 
    molecule(4).Y0 = [1042.4, -7.77, 0.0, 0.0, 0.0, 0.0, 0.0]; % vibrational Dunham coefficients
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(4).Y0(i)*.5^i;
    end
    molecule(4).D0 =  molecule(4).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(4).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    molecule(4).Y1 = [1.1244, -0.01343, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    molecule(4).Y2 = [-4.9E-6, 0.0, 0.0, 0.0, 0.0, 0.0];  %centrifugal distortion Dunham coefficients
    molecule(4).g = 4.; %degeneracy of electronic state 2(2S+1) for Pi states (2S+1) = 2
    molecule(4).ref ='Park 1990 Table 1.4, NIST webbook';
    %% electronic state C2Pi 
    molecule(5).name = 'NO(C)';
    molecule(5).level = 'C2Pi';
    molecule(5).Te = 52186.; % electronic energy level  
    molecule(5).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(5).Te; %J/mol standard heat of formation at 298.15 K
    molecule(5).De = 20383.; % Depth of potential well (Park 1990) 
    molecule(5).Y0 = [2381.3, -15.70, 0.0, 0.0, 0.0, 0.0, 0.0]; 
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(5).Y0(i)*.5^i;
    end
    molecule(5).D0 =  molecule(5).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(5).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    molecule(5).Y1 = [2.0155, -0.03244, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    D1 = 4*molecule(5).Y1(1)^3/molecule(5).Y0(1)^2;  %c Kratzer relationship (Bernath, (6.48))
    molecule(5).Y2 = [-D1, 0.0, 0.0, 0.0, 0.0, 0.0];   %centrifugal distortion correction for vibration
    molecule(5).g = 4.; %degeneracy of electronic state 2(2S+1) for Pi states (2S+1) = 2
    molecule(5).ref ='Park 1990 Table 1.5, NIST webbook';
    %% electronic state D2Sigma+ 
    molecule(6).name = 'NO(D)';
    molecule(6).level = 'D2Sigma+';
    molecule(6).Te = 53085.; % electronic energy level  
    molecule(6).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(6).Te; %J/mol standard heat of formation at 298.15 K
    molecule(6).De = 19484.; % Depth of potential well (Park 1990) 
    molecule(6).Y0 = [2323.9, -22.88, 0.0, 0.0, 0.0, 0.0, 0.0]; % vibrational Dunham coefficients
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(6).Y0(i)*.5^i;
    end
    molecule(6).D0 =  molecule(6).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(6).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    molecule(6).Y1 = [2.0026, -0.02175, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    D1 = 4*molecule(6).Y1(1)^3/molecule(6).Y0(1)^2;  % Kratzer relationship (Bernath, (6.48))
    molecule(6).Y2 = [-D1, 0.0,  0.0, 0.0, 0.0, 0.0];   %centrifugal distortion Dunham coefficients
    molecule(6).g = 2.; %degeneracy of electronic state 2(2S+1) for Pi states (2S+1) = 2
    molecule(6).ref ='Park 1990 Table 1.5, NIST webbook';
    %% electronic state E2Sigma+ 
    molecule(7).name = 'NO(E)';
    molecule(7).level = 'E2Sigma+';
    molecule(7).Te = 60629.; % electronic energy level  
    molecule(7).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(7).Te; %J/mol standard heat of formation at 298.15 K
    molecule(7).De = 11940.; % Depth of potential well (Park 1990) 
    molecule(7).Y0 = [2375.3, -16.40, 0.0, 0.0, 0.0, 0.0, 0.0]; % vibrational Dunham coefficients
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(7).Y0(i)*.5^i;
    end
    molecule(7).D0 =  molecule(7).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(7).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    molecule(7).Y1 = [1.9863, -0.01820, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    D1 = 4*molecule(7).Y1(1)^3/molecule(7).Y0(1)^2;  % Kratzer relationship (Bernath, (6.48))
    molecule(7).Y2 = [-D1, 0.0, 0.0, 0.0, 0.0, 0.0];   %centrifugal distortion Dunham coefficients
    molecule(7).g = 2.; %degeneracy of electronic state 2(2S+1) for Pi states (2S+1) = 2
    molecule(7).ref ='Park 1990 Table 1.5, NIST webbook';
    %% electronic state F2Delta 
    molecule(8).name = 'NO(F)';
    molecule(8).level = 'F2Delta';
    molecule(8).Te = 61800.; % electronic energy level  
    molecule(8).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(8).Te; %J/mol standard heat of formation at 298.15 K
    molecule(8).De = 10769.; % Depth of potential well (Park 1990) 
    molecule(8).Y0 = [2394.0, -20.00, 0.0, 0.0, 0.0, 0.0, 0.0]; 
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(8).Y0(i)*.5^i;
    end
    molecule(8).D0 =  molecule(8).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(8).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    molecule(8).Y1 = [1.9820, -0.02300, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    D1 = 4*molecule(8).Y1(1)^3/molecule(8).Y0(1)^2;  % Kratzer relationship (Bernath, (6.48))
    molecule(8).Y2 = [-D1, 0.0,0.0, 0.0, 0.0, 0.0];   %centrifugal distortion Dunham coefficients
    molecule(8).g = 4.; %degeneracy of electronic state 2(2S+1) for Pi states (2S+1) = 2
    molecule(8).ref ='Park 1990 Table 1.5, NIST webbook';
  %% electronic state b4Sigma- 
    molecule(9).name = 'NO(b4)';
    molecule(9).level = 'b4Sigma-';
    molecule(9).Te = 47950.; % electronic energy level  
    molecule(9).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(9).Te; %J/mol standard heat of formation at 298.15 K
    molecule(9).De = 21184.; % Depth of potential well (Park 1990) 
    molecule(9).Y0 = [1206.0, -15.00, 0.0, 0.0, 0.0, 0.0, 0.0]; % vibrational Dunham coefficients
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(9).Y0(i)*.5^i;
    end
    molecule(9).D0 =  molecule(9).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(9).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    molecule(9).Y1 = [1.3358, 0.0, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    D1 = 4*molecule(9).Y1(1)^3/molecule(9).Y0(1)^2;  %centrifugal distortion coefficient from Kratzer relationship (Bernath, (6.48))
    molecule(9).Y2 = [-D1, 0.0, 0.0, 0.0, 0.0, 0.0];   %centrifugal distortion correction for vibration
    molecule(9).g = 4.; %degeneracy of electronic state sigma states (2S+1) = 4
    molecule(9).ref ='Park 1990 Table 1.5, NIST webbook';
 %% electronic state b4Sigma- 
    molecule(10).name = 'NO(L''2)';
    molecule(10).level = 'L''2Phi';
    molecule(10).Te = 53657.; % electronic energy level  
    molecule(10).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(10).Te; %J/mol standard heat of formation at 298.15 K
    molecule(10).De = 18932.; % Depth of potential well (Park 1990) 
    molecule(10).Y0 = [1004.4, -11.00, 0.0, 0.0, 0.0, 0.0, 0.0]; % vibrational Dunham coefficients
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(10).Y0(i)*.5^i;
    end
    molecule(10).D0 =  molecule(10).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(10).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    molecule(10).Y1 = [1.1275, 0.0, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    D1 = 4*molecule(10).Y1(1)^3/molecule(10).Y0(1)^2;  % Kratzer relationship (Bernath, (6.48))
    molecule(10).Y2 = [-D1, 0.0, 0.0, 0.0, 0.0, 0.0]; %;   %centrifugal distortion Dunham coefficients    
    molecule(10).g = 4.; %degeneracy of electronic state sigma states (2S+1) = 4
    molecule(10).ref ='Park 1990 Table 1.5, NIST webbook';
 %% electronic state B'2Delta 
    molecule(11).name = 'NO(B''2)';
    molecule(11).level = 'B''2Delta';
    molecule(11).Te = 60364.; % electronic energy level  
    molecule(11).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(11).Te; %J/mol standard heat of formation at 298.15 K
    molecule(11).De = 12205.; % Depth of potential well (Park 1990) 
    molecule(11).Y0 = [1217.4, -15.61, 0.0, 0.0, 0.0, 0.0, 0.0]; % vibrational Dunham coefficients
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(11).Y0(i)*.5^i;
    end
    molecule(11).D0 =  molecule(11).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(11).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    molecule(11).Y1 = [1.332, -.021, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    D1 = 4*molecule(11).Y1(1)^3/molecule(11).Y0(1)^2;  % Kratzer relationship (Bernath, (6.48))
    molecule(11).Y2 = [-D1, 0.0, 0.0, 0.0, 0.0, 0.0]; %;   %centrifugal distortion Dunham coefficients    
    molecule(11).g = 4.; %degeneracy of electronic state Delta states (2S+1) = 4
    molecule(11).ref ='Park 1990 Table 1.5, NIST webbook';
 %% electronic state H2sigma 
    molecule(12).name = 'NO(H2)';
    molecule(12).level = 'H2Sigma';
    molecule(12).Te = 62473.; % electronic energy level  
    molecule(12).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(12).Te; %J/mol standard heat of formation at 298.15 K
    molecule(12).De = 10096.; % Depth of potential well (Park 1990) 
    molecule(12).Y0 = [2339.4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]; % vibrational Dunham coefficients
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(12).Y0(i)*.5^i;
    end
    molecule(12).D0 =  molecule(12).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(12).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    molecule(12).Y1 = [2.003, -.018, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    D1 = 4*molecule(12).Y1(1)^3/molecule(12).Y0(1)^2;  % Kratzer relationship (Bernath, (6.48))
    molecule(12).Y2 = [-D1, 0.0, 0.0, 0.0, 0.0, 0.0]; %;   %centrifugal distortion Dunham coefficients    
    molecule(12).g = 2.; %degeneracy of electronic state sigma states (2S+1) = 4
    molecule(12).ref ='Park 1990 Table 1.5, NIST webbook';
%% electronic state G2sigma-
    molecule(13).name = 'NO(G2)';
    molecule(13).level = 'G2Sigma-';
    molecule(13).Te = 62913.; % electronic energy level  
    molecule(13).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(13).Te; %J/mol standard heat of formation at 298.15 K
    molecule(13).De = 9656.; % Depth of potential well (Park 1990) 
    molecule(13).Y0 = [1085.5, -11.08, 0.0, 0.0, 0.0, 0.0, 0.0]; % vibrational Dunham coefficients
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(13).Y0(i)*.5^i;
    end
    molecule(13).D0 =  molecule(13).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(13).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    molecule(13).Y1 = [1.2523, -.0204, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    D1 = 4*molecule(13).Y1(1)^3/molecule(13).Y0(1)^2;  % Kratzer relationship (Bernath, (6.48))
    molecule(13).Y2 = [-D1, 0.0, 0.0, 0.0, 0.0, 0.0]; %;   %centrifugal distortion Dunham coefficients    
    molecule(13).g = 2.; %degeneracy of electronic state sigma states (2S+1) = 2
    molecule(13).ref ='Park 1990 Table 1.5, NIST webbook';
%% electronic state L2Pi
    molecule(14).name = 'NO(L2)';
    molecule(14).level = 'L2Pi';
    molecule(14).Te = 63040.; % electronic energy level  
    molecule(14).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(14).Te; %J/mol standard heat of formation at 298.15 K
    molecule(14).De = 19144.; % Depth of potential well (Park 1990) 
    molecule(14).Y0 = [952., -11.28, 0.0, 0.0, 0.0, 0.0, 0.0]; % vibrational Dunham coefficients
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(14).Y0(i)*.5^i;
    end
    molecule(14).D0 =  molecule(14).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(14).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    molecule(14).Y1 = [1.132, -.0221, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    D1 = 4*molecule(14).Y1(1)^3/molecule(14).Y0(1)^2;  % Kratzer relationship (Bernath, (6.48))
    molecule(14).Y2 = [-D1, 0.0, 0.0, 0.0, 0.0, 0.0]; %;   %centrifugal distortion Dunham coefficients    
    molecule(14).g = 4.; %degeneracy of electronic state PI states 2(2S+1) = 4
    molecule(14).ref ='Park 1990 Table 1.5, NIST webbook';
%% electronic state k2Pi
    molecule(15).name = 'NO(K2)';
    molecule(15).level = 'K2Pi';
    molecule(15).Te = 64078.; % electronic energy level  
    molecule(15).deltaH_f0 = molecule(1).deltaH_f0 + c1*molecule(15).Te; %J/mol standard heat of formation at 298.15 K
    molecule(15).De = 18106.; % Depth of potential well (Park 1990) 
    molecule(15).Y0 = [2438.3, -48.38, 0.0, 0.0, 0.0, 0.0, 0.0];  % vibrational Dunham coefficients
    % compute zero point energy
    G_0 = 0.0;
    for i = 1:7
        G_0 = G_0 +  molecule(15).Y0(i)*.5^i;
    end
    molecule(15).D0 =  molecule(15).De - G_0;  % bond dissociation energy computed from De - 1/2 hv_o
    molecule(15).theta = 2*G_0*c2;  %  characteristic vibrational temperature  
    molecule(15).Y1 = [2.034, -.056, 0.0, 0.0, 0.0, 0.0]; % rotational Dunham coefficients
    D1 = 4*molecule(15).Y1(1)^3/molecule(15).Y0(1)^2;  % Kratzer relationship (Bernath, (6.48))
    molecule(15).Y2 = [-D1, 0.0, 0.0, 0.0, 0.0, 0.0]; %;   %centrifugal distortion Dunham coefficients
    molecule(15).g = 4.; %degeneracy of electronic state PI states 2(2S+1) = 4
    molecule(15).ref ='Park 1990 Table 1.5, NIST webbook';




