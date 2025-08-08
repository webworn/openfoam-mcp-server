function afrozen = soundspeed_fr(gas)
% Computes the frozen sound speed by using a centered finite
% difference approximation and evaluating frozen composition states on the
% isentrope passing through the reference (S, V) state supplied by the gas
% object passed to the function. 
% 
% FUNCTION SYNTAX:
%     afrozen =  soundspeed_fr(gas)
% 
% INPUT:
%     gas = working gas object (restored to original state at end of function)
% 
% OUTPUT:
%     afrozen = frozen sound speed = sqrt({d P/d rho)_{s,x0}) 

   rho0 = density(gas);
   T0 = temperature(gas);
   P0 = pressure(gas);
   s0 = entropy_mass(gas);
   x0 = moleFractions(gas);
   rho1 = 0.99*rho0;
   set(gas,'Density',rho1,'Entropy',s0,'MoleFractions',x0);
   p1 = pressure(gas);
   rho2 = 1.01*rho0;
   set(gas,'Density',rho2,'Entropy',s0,'MoleFractions',x0);
   p2 = pressure(gas);
   dpdrho_s = (p2 - p1)/(rho2 - rho1);
   afrozen = sqrt(dpdrho_s);
   set(gas,'Temperature',T0,'Pressure',P0,'MoleFractions',x0);
