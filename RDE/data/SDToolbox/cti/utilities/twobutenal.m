% This an example of the input file data structure for the program
% thermo_fit.m 
% JES July 8, 2018
%%
% the following variables are required
name = '2-Butenal' ;      
species = 'CH3CHCHCHO';
% composition
na = 11;  % number of atoms
ne = 3;   % number of elements
els = ['C';   'H';   'O' ];  %array of element symbols
at = [4; 6; 1];  %quantity of each element
% standard state values
%T_zero = 298.15;
S_R_zero =  40.06;  % so/R at 298.15 K (From Wang 2000)
DeltaH_f0 = -109.78E3 ;  % heat of formation J/mol
H_RT_zero = -DeltaH_f0/8.314472/298.15; % ho/RT at 298.15 K (enthalpy of formation)
m = 70.091  % molar mass  g/mol
%%
% The following data structure is an example of how to use output from
% LINGRAF and convert to arrays required for thermo_fit.m 
Rgas = 1.987; % cal/mol K  - this should be in the units of the Table
% Pressure = 1.0 atm
% Input from LINGRAF (Caltech 2007)
% 2-Butenal minimized with QEq and DREIDING II
% Columns are : TEMP(K)   CP (cal/mol K)   S (cal/mol K)   H (kcal/mol)   G (kcal/mol)
DATA = [200.0          14.4456         64.7666         2.1247       -10.8286
300.0          18.2506         71.3319         3.7567       -17.6429
400.0          22.4497         77.1501         5.7892       -25.0709
500.0          26.7302         82.6217         8.2492       -33.0616
600.0          30.7590         87.8566        11.1266       -41.5874
700.0          34.3955         92.8766        14.3878       -50.6258
800.0          37.6098         97.6840        17.9916       -60.1556
900.0          40.4184        102.2798        21.8962       -70.1556
1000.0          42.8563        106.6675        26.0629       -80.6046
1100.0          44.9649        110.8535        30.4565       -91.4823
1200.0          46.7863        114.8460        35.0463      -102.7689
1300.0          48.3601        118.6546        39.8055      -114.4454
1400.0          49.7220        122.2895        44.7113      -126.4940
1500.0          50.9033        125.7612        49.7439      -138.8979
1600.0          51.9310        129.0799        54.8868      -151.6411
1700.0          52.8280        132.2558        60.1258      -164.7091
1800.0          53.6138        135.2981        65.4487      -178.0879
1900.0          54.3047        138.2157        70.8453      -191.7645
2000.0          54.9143        141.0171        76.3069      -205.7272
2100.0          55.4543        143.7097        81.8259      -219.9644
2200.0          55.9342        146.3007        87.3958      -234.4656
2300.0          56.3623        148.7967        93.0110      -249.2213
2400.0          56.7454        151.2037        98.6667      -264.2222
2500.0          57.0894        153.5273       104.3588      -279.4594
2600.0          57.3992        155.7725       110.0835      -294.9250
2700.0          57.6792        157.9441       115.8376      -310.6113
2800.0          57.9328        160.0464       121.6185      -326.5115
2900.0          58.1632        162.0834       127.4234      -342.6184
3000.0          58.3731        164.0588       133.2504      -358.9261
3100.0          58.5648        165.9760       139.0974      -375.4283
3200.0          58.7402        167.8382       144.9628      -392.1194
3300.0          58.9012        169.6483       150.8450      -408.9942
3400.0          59.0492        171.4089       156.7426      -426.0476
3500.0          59.1856        173.1226       162.6545      -443.2745
3600.0          59.3116        174.7916       168.5794      -460.6704
3700.0          59.4281        176.4183       174.5165      -478.2311
3800.0          59.5360        178.0046       180.4647      -495.9526
3900.0          59.6362        179.5524       186.4234      -513.8310
4000.0          59.7294        181.0634       192.3918      -531.8619
4100.0          59.8163        182.5394       198.3691      -550.0424
4200.0          59.8973        183.9818       204.3549      -568.3687
4300.0          59.9729        185.3921       210.3484      -586.8375
4400.0          60.0437        186.7717       216.3493      -605.4463
4500.0          60.1101        188.1218       222.3569      -624.1912
4600.0          60.1723        189.4436       228.3711      -643.0695
4700.0          60.2308        190.7384       234.3913      -662.0790
4800.0          60.2858        192.0070       240.4172      -681.2166
4900.0          60.3375        193.2506       246.4484      -700.4796
5000.0          60.3863        194.4700       252.4846      -719.8656];
%%
% Convert data to normalized form, correct to standard state 
% thermo_fit.m expects to have four numerica arrays of the same dimension
% T       -  temperature in Kelvin
% CP_R    -  normalized specific heat Cp/R
% S_R     -  normalized entropy S/R
% H_RT    -  normalized enthalpy H/RT
T = DATA(:,1);
CP_R = DATA(:,2)/Rgas;
CP_R_zero = interp1(T,CP_R,T_zero);
S_R = DATA(:,3)/Rgas;
% correct entropy and enthalpy to standard state
s_temp = interp1(T,S_R,T_zero);
S_R = S_R+(S_R_zero-s_temp);
h_temp = interp1(T,DATA(:,4),T_zero);
H_RT = (DATA(:,4)-h_temp + H_RT_zero*Rgas*T_zero/1000.)*1000/Rgas./T;



