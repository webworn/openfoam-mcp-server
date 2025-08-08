function [V, dVdt] = f_volume(t,tau,V_min,t_min)
% f_volume
% returns normalized volume V (V(t)/V(0)) and volume time derivative dVdt(t)/V(0)
% parameter tau, t_min and V_min need to be defined in main program
% V_min  ratio of minimum to maximum volume (1/compression ratio)
% t_min should be a minimum of 3*tau and the computation end time should be
% at least 6*tau to have a smooth volume function.  
%  
% ################################################################################
% Theory, numerical methods and applications are described in the following report:
%
% IGNITION MODELING AND THE  CRITICAL DECAY RATE CONCEPT
%
% J. E. SHEPHERD
% GRADUATE AEROSPACE LABORATORIES
% CALIFORNIA INSTITUTE OF TECHNOLOGY
% PASADENA, CA 91125
% GALCIT REPORT EDL2019.002  July 2019
%
% Please cite this report and the website if you use these routines.
%
% Please refer to LICENCE.txt or the above report for copyright and disclaimers.
%
% http://shepherd.caltech.edu/EDL/PublicResources/sdt/
%
% ################################################################################
% Updated July 2019
% Tested with:
%     MATLAB 2017b and 2018a, Cantera 2.3 and 2.4
% Under these operating systems:
%     Windows 8.1, Windows 10, Linux (Debian 9)
%%

%%
V = ( 1 - V_min)*(1 - exp(-((t-t_min)/tau)^2))/(1- exp(-((t_min)/tau)^2)) + V_min;
dVdt = (1-V_min)*exp(-((t-t_min)/tau)^2)*2*(t-t_min)/tau^2/(1- exp(-((t_min)/tau)^2));
end

