% Shock and Detonation Toolbox Demo Program
% 
% Calculate the EQUILIBRIUM post shock state based on the initial gas
% state and the shock speed.  Evaluates the shock jump conditions using a fixed
% composition using SDToolbox function PostShock_eq.
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
disp('demo_PSeq')
%% Initial state specification:
%  P1 = Initial Pressure  
%  T1 = Initial Temperature 
%  U = Shock Speed 
%  q = Initial Composition 
%  mech = Cantera mechanism File name
P1 = 100000; T1 = 300; U = 2000;
q = 'H2:2 O2:1 N2:3.76';    
mech = 'Mevel2017.cti'; 
%% Compute equilibrium postshock state
[gas] = PostShock_eq(U, P1, T1, q, mech);
% Output 
% gas - Cantera gas object for postshock state
P1atm = P1/oneatm;
Ps = pressure(gas);
Psatm = Ps/oneatm;
Ts = temperature(gas);
disp(['Initial state: ',q, ', P1 = ',num2str(P1atm),' atm, T1 = ',num2str(T1),' K'])
disp(['Mechanism: ', mech])
disp(['Equilibrium postshock state: Ps = ',num2str(Psatm),' atm, Ts = ', num2str(Ts),' K'])

