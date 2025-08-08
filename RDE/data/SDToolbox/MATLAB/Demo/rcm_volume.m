function [V, dVdt] = rcm_volume(t,Z)
% returns normalized volume V (V(t)/V(0)) and volume time derivative dVdt(t)/V(0)
% by interpolating tabular data to time t (which could be an array).
%
% designed to be used with smoothed data from a rapid compression machine
% Z(:,1) - time, t
% Z(:,2) - volume V(t)
% Z(:,3) - volume derivative, dVdt(t)
%%
n = length(t);
m = length(Z);
for i = 1:n
        k = find((Z(:,1)>t(i)),1,'first');
        if (k < m)
        s = (t(i) - Z(k,1))/(Z(k+1,1)-Z(k,1));
        a = Z(k,2) + s*(Z(k+1,2)-Z(k,2));
        V(i) = a/Z(1,2);
        b = Z(k,3) + s*(Z(k+1,3)-Z(k,3));
        dVdt(i) = b/Z(1,2);
    else
        V(i) = Z(m,2)/Z(1,2);
        dVdt(i) = Z(m,3)/Z(1,2);
    end
end
end

