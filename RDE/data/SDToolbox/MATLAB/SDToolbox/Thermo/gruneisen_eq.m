function G_eq = gruneisen_eq(gas)
% Computes the equilibrium Gruneisen coefficient by using a centered finite
% difference approximation and evaluating equilibrium states on the
% isentrope passing through the reference (S, V) state supplied by the gas
% object passed to the function. If the reference state is not in
% equilibrium, the result of the call is still valid but requires care in
% interpretation.
% 
% Note: Because this function uses equilibrate('SV'), it could be susceptible to the same
% problems at high pressure as soundspeed_eq. If such issues are encountered,
% this function could be altered in the same was as soundspeed_eq was.
% For details on how this could be implemented, refer to Appendix G3 of the report.
% 
% FUNCTION SYNTAX:
%     G_eq =  gruneisen_eq(gas)
% 
% INPUT:
%     gas = working gas object (restored to original state at end of function)
% 
% OUTPUT:
%     G_eq = equilibrium Gruneisen coefficient [-de/dp)_{v,eq} =
%             -(v/T)dT/dv)_{s,eq} = + (rho/T)(dT/d rho)_{s,eq}]
                
   rho0 = density(gas);
   P0 = pressure(gas);
   T0 = temperature(gas);
   s0 = entropy_mass(gas);
   x0 = moleFractions(gas);
   rho1 = 0.99*rho0;
   set(gas,'Density',rho1,'Entropy',s0,'MoleFractions',x0);
   equilibrate(gas,'SV');
   t1 = temperature(gas);
   rho2 = 1.01*rho0;
   set(gas,'Density',rho2,'Entropy',s0,'MoleFractions',x0);
   equilibrate(gas,'SV');
   t2 = temperature(gas);
   dtdrho = (t2 - t1)/(rho2 - rho1);
   rho = (rho1 + rho2)/2.;
   t = (t1+t2)/2.;
   G_eq = dtdrho*rho/t;
   set(gas,'Temperature',T0,'Pressure',P0,'MoleFractions',x0);

