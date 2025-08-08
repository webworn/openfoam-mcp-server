function F = hug_eq(x,gas,array)
% Computes difference between enthalpy at given temperature and volume with
% enthalpy value computed from jump conditions (equilibrium) for given pressure
% and volume. Used with root solver 'fsolve' to find states on Hugoniot.
% Note: gas state is changed by routine
%
% FUNCTION SYNTAX:
%     hug_fr(x,gas,array)
%
% USAGE:
%     [x,fval] = fsolve(@hug_eq,Ta,options,gas,array)
%
% INPUT:
%     Ta = (x) initial guess for frozen Hugoniot temperature (K)
%     options = options string for fsolve
%     gas = working gas object 
%     array = array with the following values
%       vb = desired equilibrium Hugoniot specific volume (m^3/kg)
%       h1 = enthalpy at state 1 (J/kg)
%       P1 = pressure at state 1 (Pa)
%       v1 = specific volume at state 1 (m^3/kg)
%
% OUTPUT:
%     F = [x, fval]
%         x = equilibrium Hugoniot temperature corresponding to vb (K)
%         fval = value of function at x
%

vb = array(1);
h1 = array(2);
P1 = array(3);
v1 = array(4);

set(gas,'Density',1/vb,'T',x);
equilibrate(gas,'TV');
hb1 = enthalpy_mass(gas);
Pb = gasconstant*x/(meanMolecularWeight(gas)*vb);
hb2 = h1 + 1/2*(Pb-P1)*(vb+v1);
F = hb2-hb1;

