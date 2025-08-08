function G_fr = gruneisen_fr(gas)
% Computes the frosen Gruneisen coefficient by using a centered finite
% difference approximation and evaluating frozen states on the
% isentrope passing through the reference (S, V) state supplied by the gas
% object passed to the function. 
% 
% FUNCTION SYNTAX:
%     G_fr =  gruneisen_fr(gas)
% 
% INPUT:
%     gas = working gas object (not modified in function)
% 
% OUTPUT:
%     G_eq = frozen Gruneisen coefficient [-de/dp)_{v,x0} =
%             -(v/T)dT/dv)_{s,x0} = + (rho/T)(dT/d rho)_{s,x0}]
                
   rho0 = density(gas);
   P0 = pressure(gas);
   T0 = temperature(gas);
   s0 = entropy_mass(gas);
   x0 = moleFractions(gas);
   rho1 = 0.99*rho0;
   set(gas,'Density',rho1,'Entropy',s0,'MoleFractions',x0);
   t1 = temperature(gas);
   rho2 = 1.01*rho0;
   set(gas,'Density',rho2,'Entropy',s0,'MoleFractions',x0);
   t2 = temperature(gas);
   dtdrho = (t2 - t1)/(rho2 - rho1);
   rho = (rho1 + rho2)/2.;
   t = (t1+t2)/2.;
   G_fr = dtdrho*rho/t;
   set(gas,'Temperature',T0,'Pressure',P0,'MoleFractions',x0);

