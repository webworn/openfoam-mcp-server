function CJspeed_plot(plot_num,plot_data,curve,dnew)
% Creates two plots of the CJspeed fitting routine: both display density ratio
% vs. speed. The first is very 'zoomed in' around the minimum, and shows the
% quadratic fit plotted through the calculated points. The second shows the 
% same fit on a wider scale, with the minimum and its corresponding speed
% indicated.        
        
    % Plots how initial velocity varies with density ratio
    figure(plot_num);
    plot(plot_data.rr(:),plot_data.w1(:),'ks');
    hold on;
    plot(curve,'k')
    hold off;
    title(['Initial Velocity vs. Density Ratio - CJ Speed is ',num2str(plot_data.cj_speed,6),' \pm',num2str(plot_data.upbound,3),],'FontSize', 14);
    xlabel(['Density Ratio  [Fit: ',num2str(curve.a,4),'x^2 + ',num2str(curve.b,4),'x + ',num2str(curve.c,4),']'],'FontSize', 14);
    ylabel('Velocity','FontSize', 14);
    set(gca,'FontSize',12);
    
    x = linspace(1.5,2.0);
    big_curve = curve(x);
    figure(plot_num+1);
    plot(x(:),big_curve(:),'k')
    hold on;
    plot(dnew,plot_data.cj_speed,'k*')
    hold off;
    title(['Initial Velocity vs. Density Ratio - CJ Speed is ',num2str(plot_data.cj_speed,8),' \pm',num2str(plot_data.upbound,3),],'FontSize', 14);
    xlabel(['Density Ratio  [Fit: ',num2str(curve.a,4),'x^2 + ',num2str(curve.b,4),'x + ',num2str(curve.c,4),']'],'FontSize', 14);
    ylabel('Velocity','FontSize', 14);
    set(gca,'FontSize',12);