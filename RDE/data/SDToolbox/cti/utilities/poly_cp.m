function [x, cp_fit, index_mid, a_0_6, a_1_6, s_fit, a_0_7, a_1_7, h_fit] = poly_cp(z, Tmin, Tmid, Tmax,CP_R_zero,DCPRDT,S_R_zero,H_RT_zero)
% partition.m - A function that fits a two segment polynomial to specific
% heat, use NASA 7 term format from SP-272. Gordon and McBride 1971.
% Coefficients a6 and a7 for enthalpy and entropy are also computed.
%
%Eq. 90: cp0/R=a1+a2T+a3*T^2+a4*T^3+a5*T^4 
%Eq. 91: H0_T/(RT)=a1+a2/2*T+a3/3*T^2+a4/4*T^3+a5/5*T^4+a6/T 
%Eq. 92: S0_T/R=a1*ln(T)+a2*T+a3/2*T^2+a4/3*T^3+a5/4*T^4+a7
%
% This program fits the specific heat only then uses standard state data
% and the analytical integration to obtain the additional coefficients for
% entropy and enthalpy.  The specific heat fit constrains the derivative to
% vanish at the lowest temperature, the value and derivative are continuous
% at the knot at T_mid and the value of specific heat at 298.15 K is
% constrained to be the specified value of the standard state. The value of
% the entropy and entropy are set to standard state values at 298.15 K. The
% entropy and enthalpy values at the midpoints are continuous.  This
% procedure produces high quality fits as long as the input entropy and enthalpy
% are thermodynamically consistent with the specific heat data.  This will
% always be the case if the thermodynamic data have been derived from
% partition functions.   
%
% Matlab R2018a
% JES 7/14/2018
%
T_zero = 298.15;
T = z(1,:);
cp_over_R = z(2,:);
h_over_RT = z(3,:);
s_over_R = z(4,:);
index_max = length(T);
Tmin = min(T);
Tmax = max(T);
index_mid = max(find(T(:)<=Tmid));   %find closest temperature in vector
Tmid = T(index_mid:index_mid);
%
% set up coefficient matrix
C = zeros(index_max+1,10);
for i = 1:5
    for k =1:index_mid
        C(k,i) = T(k)^(i-1);
    end
end
for i = 1:5
    for k = index_mid:index_max
        C(k+1,i+5) = T(k)^(i-1);
    end
end
% initial guess for coefficients
for i = 1:10
    x0(i)  = 1.;
end
% right hand side
for k = 1:index_mid
    d(k) = cp_over_R(k);
end
for k = index_mid:index_max
    d(k+1) = cp_over_R(k);
end
% constraint values 
for i=1:5
    Aeq(1,i) = T_zero^(i-1);   %fix standard state value for poly 1
    Aeq(1,i+5) = 0.0;
    %
    Aeq(2,i) = T(index_mid)^(i-1);   %fix midpoint value  for poly 1
    Aeq(2,i+5) = 0.0;
    %
    Aeq(3,i) = 0.;                    %fix midpoint value  for poly 2
    Aeq(3,i+5) = T(index_mid)^(i-1);
    %
    Aeq(4,i) = (i-1)*T(index_mid)^(i-2);  %fix derivative for poly 1
    Aeq(4,i+5) = 0.0;
    %
    Aeq(5,i) = 0.;                       %fix derivative for poly 2                 
    Aeq(5,i+5) = (i-1)*T(index_mid)^(i-2);
    %
%    Aeq(6,i) = (i-1)*T(1)^(i-2);    % sero derivative at lowest temperature
%    Aeq(6,i+5) = 0.0;
end
beq(1) = CP_R_zero;  % match standard state, polynomial 1
beq(2) = cp_over_R(index_mid);  % continuity at knot, polynomial 1
beq(3) = cp_over_R(index_mid);  % continuity at knot, polynomial 2
beq(4) = DCPRDT;   % match derivative at knot, polynomial 1
beq(5) = DCPRDT;   % match derivative at knot, polynomial 2
%beq(6) = 0.;        % zero derivative at lowest temperature in poly 1
% least squares fit, diagnostics can be turned on if necessary
%options = optimoptions('lsqlin','Algorithm','active-set','Diagnostics','on');
options = optimoptions('lsqlin','Algorithm','interior-point','Diagnostics','off');
x = lsqlin(C,d,[],[],Aeq,beq,[],[],x0,options);
% evaluate polynomial for specific heat
cp_fit = C*x;
% value at standard state
% check fit at standard state and eliminate any jump at midpoint
display(['Result of initial optimization']);
sum = 0.;
for i = 1:5
    sum = sum + x(i)*T_zero^(i-1);
end
delta_cp = CP_R_zero-sum;
display(['Difference in CP at T_zero = ',num2str(delta_cp)]);
% check for jump at T mid
cp_jump = cp_fit(index_mid+1)-cp_fit(index_mid);
display(['Jump in CP at midpoint = ',num2str(cp_jump)]);
% remove jump
x(6) = x(6) - cp_jump;
% update initial guess and re-run optimization
x1 = x;
x = lsqlin(C,d,[],[],Aeq,beq,[],[],x1,options);
x2 = x;
x = lsqlin(C,d,[],[],Aeq,beq,[],[],x2,options);
display(['Result of re-running optimization']);
%evaluate fit again
cp_fit = C*x;
sum = 0.;
for i = 1:5
    sum = sum + x(i)*T_zero^(i-1);
end
delta_cp = CP_R_zero-sum;
display(['Difference at T_zero in CP = ',num2str(delta_cp)]);
% check for jump at T mid
cp_jump = cp_fit(index_mid+1)-cp_fit(index_mid);
display(['Jump in CP at midpoint = ',num2str(cp_jump)]);
%
% evaluate the enthalpy fit
%
C2 = zeros(index_max+1,10);
for i = 1:5
    for k =1:index_mid
        C2(k,i) = T(k)^(i-1)/i;
    end
end
for i = 1:5
    for k = index_mid:index_max
        C2(k+1,i+5) = T(k)^(i-1)/i;
    end
end
% fix poly 1 constant by matching enthalpy at 298.15 
sum = 0.;
for i = 1:5
   sum = sum + x(i)*T_zero^(i-1)/i;
end
a_0_6 = T_zero*(H_RT_zero - sum);
% fix poly 2 constant by matching entropy at knot 
sum1 = 0.;
for i = 1:5
   sum1 = sum1 + x(i)*T(index_mid)^(i-1)/i;
end
sum1 = sum1 + a_0_6/T(index_mid);
% match polynomials at knot
sum2 = 0.;
for i = 1:5
   sum2 = sum2 + x(5+i)*T(index_mid)^(i-1)/i;
end
a_1_6 = T(index_mid)*(sum1 - sum2);
% evaluate corrected fit
h_fit=C2*x; 
for k = 1:index_mid
    h_fit(k) = h_fit(k) + a_0_6/T(k);
end
for k = index_mid:index_max
    h_fit(k+1) = h_fit(k+1) + a_1_6/T(k);
end
%
% evaluate entropy fit
% 
C1 = zeros(index_max+1,10);
for i = 1:5
    for k =1:index_mid
        if (i==1)
            C1(k,i) = log(T(k));
        else
            C1(k,i) = T(k)^(i-1)/(i-1);
        end
    end
end
for i = 1:5
    for k = index_mid:index_max
        if (i==1)
            C1(k+1,i+5) = log(T(k));
        else
            C1(k+1,i+5) = T(k)^(i-1)/(i-1);
        end
    end
end
% fix poly 1 constant by matching entropy at 300 K, gives lower error than
% matching interpolated value at T_zero
s_fit=C1*x; 
index_300 = find(T(:)==300);
S_R_300 = s_over_R(index_300);
a_0_7 = S_R_300 - s_fit(index_300) ;
% fix poly 2 constant by matching entropy at knot 
a_1_7 =(s_fit(index_mid) + a_0_7) - s_fit(index_mid+1);
% evaluate corrected fit
for k = 1:index_mid
    s_fit(k) = s_fit(k) + a_0_7;
end
for k = index_mid:index_max
    s_fit(k+1) = s_fit(k+1) + a_1_7;
end
end


