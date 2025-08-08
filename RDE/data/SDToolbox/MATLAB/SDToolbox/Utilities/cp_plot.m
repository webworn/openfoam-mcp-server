function figs = cp_plot(output,varargin)
%     Creates two subplots from the solution to a CP explosion (in sdt.cp.cvsolve):
%     Temperature vs. time, and desnity vs. time.
%     Optionally, also creates plots of species mass fraction vs. time, for given lists
%     of major or minor species. If major_species='All', all species will be plotted together.
%     
%     FUNCTION SYNTAX:
%         figs = cp_plot(output)
%         
%     INPUT:
%         output: dictionary of outputs produced by cp solve or cvsolve.
%         
%     OPTIONAL INPUT:
%         xscale: 'linear' or 'log' -- how to scale the x-axis
%         
%         maxt: Maximum time to plot to on the x-axis. By default, determined internally.
%
%         species_plt:   'mole' or "mass" -- species fractions to display,
%                       default is 'mole"
%         
%       P_scale =  scaling factor for pressure, default = 1
%
%       t_scale = scaled factor for time, default = 1
%
%       P_unit =  label on pressure, default = P (Pa)
%
%       t_unit =   label on time axis,  default = t  (s)
% 
%         show: Whether to display the figures. Defaults to 'True', but can set to 'False'
%               if the user wants to return the figure handles to make further modifications
%               before displaying.
%
%       prnfig = logical variable determining if .eps figures are produced
%       as part of output.  
%         
%         major_species,minor_species: lists of species names defined by the user as
%                                      major or minor. Creates additional plots of mass
%                                      fraction of each given species. Names need to match
%                                      those in the Cantera model being used, and need to be
%                                      capitalized (e.g. AR for argon). Unmatched names will
%                                      be ignored for plotting, and a message displayed.
%                                      major_species='All' gives all species.
%                 
%     OUTPUT:
%         List of figure handles: [temperature, pressure, density, {major species}, {minor species}]

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Parse optional name-value pair arguments
    default_maxt = 0;
    default_major_species = {};
    default_minor_species = {}; 
    default_xscale = 'linear';
    default_show = true;   
    default_species_plt = 'mole';
    default_P_scale = 1.;
    default_rho_scale = 1.;
    default_t_scale = 1.;
    default_t_unit = 't (s)';
    default_P_unit ='P (Pa)';
    default_rho_unit = '\rho (kg/m3)';
    default_prnfig = false;
    
    p = inputParser;
    addRequired(p,'cv_output');
    addParameter(p,'maxt',default_maxt,@isnumeric);
    addParameter(p,'species_plt',default_species_plt);
    addParameter(p,'major_species',default_major_species);
    addParameter(p,'minor_species',default_minor_species);
    addParameter(p,'xscale',default_xscale);
    addParameter(p,'show',default_show,@islogical);
    addParameter(p,'P_scale',default_P_scale,@isnumeric);
    addParameter(p,'rho_scale',default_rho_scale,@isnumeric);
    addParameter(p,'t_scale',default_P_scale,@isnumeric);
    addParameter(p,'P_unit',default_P_unit);
    addParameter(p,'rho_unit',default_rho_unit);
    addParameter(p,'t_unit',default_t_unit);
    addParameter(p,'prnfig',default_prnfig,@islogical);

    parse(p,output,varargin{:});
    maxt = p.Results.maxt;
    species_plt = p.Results.species_plt;
    major_species = p.Results.major_species;
    minor_species = p.Results.minor_species;
    xscale = p.Results.xscale;
    show = p.Results.show;
    P_scale = p.Results.P_scale;
    rho_scale = p.Results.rho_scale;
    t_scale = p.Results.t_scale;
    P_unit = p.Results.P_unit;
    rho_unit = p.Results.rho_unit;
    t_unit = p.Results.t_unit;
    prnfig = p.Results.prnfig; 
   
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	%PLOT TEMPERATURE PROFILE - MAX TIME = MAX TEMP + 10%
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	
    if maxt == 0
        k = 1;
        MAX = max(output.T);
        d = output.T(1);
        while d < MAX
            k = k + 1;
            d = output.T(k);
        end
        
        if (output.time(1,k) == 0)
            maxt = output.ind_time*5;
        elseif (output.time(1,k) >= output.ind_time*50)
            maxt = output.ind_time*5;
        else
            maxt = output.time(1,k) + 0.1*output.time(1,k);
        end
    end
    
    if strcmp(string(xscale),'linear')
        mint = 0;
    elseif strcmp(string(xscale),'log')
        mint = 1e-9;
    end
    
    fontsize = 12;
    if show
        visible = 'On';
    else
        visible = 'Off';
    end
    font ='TimesRoman';
    set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);

    %Temperature as a function of time
 	figT = figure('Visible',visible);
 	plot(output.time(:)/t_scale,output.T(:),'k');
 	xlabel(t_unit,'FontSize',fontsize);
 	ylabel('Temperature (K)','FontSize',fontsize);
% 	title('Constant volume explosion','FontSize',fontsize);
    xlim([mint/t_scale maxt/t_scale]);
 	set(gca,'FontSize',12,'XScale',xscale);
    if (prnfig) print('temperature','-depsc'); end
	
	%Pressure as a function of time
    if (P_scale ~= 0)
        figP = figure('Visible',visible);
        plot(output.time(:)/t_scale,output.P(:)/P_scale,'k');
        xlabel(t_unit,'FontSize',fontsize);
        ylabel(P_unit,'FontSize',fontsize);
        xlim([mint/t_scale maxt/t_scale]);
        set(gca,'FontSize',12,'XScale',xscale);
        if (prnfig) print('pressure','-depsc'); end
    end
    
    %density as a function of time
	figrho = figure('Visible',visible);
	plot(output.time(:)/t_scale,output.rho(:)/rho_scale,'k');
	xlabel(t_unit,'FontSize',fontsize);
	ylabel(rho_unit,'FontSize',fontsize);
	xlim([mint/t_scale maxt/t_scale]);
    set(gca,'FontSize',12,'XScale',xscale);
    if (prnfig) print('density','-depsc'); end

    if (P_scale == 0)
        figs = [figT figrho];
    elseif (rho_scale == 0)
        figs = [figT figP];
    else 
        figs = [figT figP figrho];
    end
    % Major species mass fractions as a function of position
    if ~isempty(major_species)
        if strcmp(string(major_species),'All')
            major_species = speciesNames(output.gas);
        end
        major_k = []; major_labels = {};
        species = speciesNames(output.gas);
        for i = 1:length(major_species)
            if any(ismember(species,major_species{i}))
                major_k(end+1) = speciesIndex(output.gas,major_species(i));
                major_labels{end+1} = major_species{i};
            end
        end
        if ~isempty(major_k)
            figMaj = figure('Visible',visible);
            set(gca,'linestyleorder',{'-','-.','--',':'},'nextplot','add','XScale',xscale,'yscale','log');
            box on;
            if strcmp(string(species_plt),'mass')
                plot(output.time(:)/t_scale,output.species(:,major_k));
                ylabel('Species mass fraction','FontSize',fontsize);
            elseif strcmp(string(species_plt),'mole')
                plot(output.time(:)/t_scale,output.speciesX(:,major_k));
                ylabel('Species mole fraction','FontSize',fontsize);
            end
            axis([mint/t_scale maxt/t_scale 1.0E-10 1]);
            xlabel(t_unit,'FontSize',fontsize);
             legend(major_labels,'Location','southeast');
            if (prnfig) print('major_species','-depsc'); end
            figs(end+1) = figMaj;
        end
    end
    
    % Minor species mass fractions as a function of position
    if ~isempty(minor_species)
        minor_k = []; minor_labels = {};
        species = speciesNames(output.gas);
        for i = 1:length(minor_species)
            if any(ismember(species,minor_species{i}))
                minor_k(end+1) = speciesIndex(output.gas,minor_species(i));
                minor_labels{end+1} = minor_species{i};
            end
        end
        if ~isempty(minor_k)
            figMin = figure('Visible',visible);
            set(gca,'linestyleorder',{'-','-.','--',':'},'nextplot','add','XScale',xscale,'yscale','log');
            box on;
            if strcmp(string(species_plt),'mass')
                plot(output.time(:)/t_scale,output.species(:,minor_k));
                ylabel('Species mass fraction','FontSize',fontsize);
            elseif strcmp(string(species_plt),'mole')
                plot(output.time(:)/t_scale,output.speciesX(:,minor_k));
                ylabel('Species mole fraction','FontSize',fontsize);
            end
            axis([mint/t_scale maxt/t_scale 1.0E-10 1]);
            xlabel(t_unit,'FontSize',fontsize);
            legend(minor_labels,'Location','southeast');
            if (prnfig) print('minor_species','-depsc'); end
            figs(end+1) = figMin;
        end
    end 

