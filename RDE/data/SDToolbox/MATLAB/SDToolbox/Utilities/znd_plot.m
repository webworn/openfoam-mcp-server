function figs = znd_plot(znd_output,varargin)
%     Creates four plots from the solution to a ZND detonation (in sdt.znd.zndsolve):
%     Temperature, pressure, Mach number, and thermicity vs. distance.
%     Optionally, also creates plots of species mass fraction vs. time, for given lists
%     of major or minor species. If major_species='All', all species will be plotted together.
%     
%     FUNCTION SYNTAX:
%         znd_plot(znd_output)
%         
%     INPUT:
%         znd_output: dictionary of outputs produced by sdt.znd.zndsolve.
%         
%     OPTIONAL INPUT:
%         xscale: 'linear' or 'log' -- how to scale the x-axis
%         
%         maxt: Maximum distance to plot to on the x-axis. By default, determined internally.
%         
%         show: Whether to display the figures. Defaults to 'True', but can set to 'False'
%               if the user wants to return the figure handles to make further modifications
%               before displaying.
% 
%         species_plt: 'mole' or 'mass'  species fraction to plot
%
%         xunits:      distance units in plot axis label, default 'mm'
%
%         xmult        multiplier for distance units,  default 1.0E3
%
%         major_species,minor_species: lists of species names defined by the user as
%                                      major or minor. Creates additional plots of mass
%                                      fraction of each given species. Names need to match
%                                      those in the Cantera model being used, and need to be
%                                      capitalized (e.g. AR for argon). Unmatched names will
%                                      be ignored for plotting, and a message displayed.
%                                      major_species='All' gives all species.
%          prnfig:   logical true to save figures as eps files, if false
%                     or missing set to logical false and figures are display
%                     only.
%         
%     OUTPUT:
%         List of figure handles: [temperature, pressure, Mach, 
%                                  thermicity, {major species}, {minor species}]

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Parse optional name-value pair arguments
    default_maxx = 0;
    default_major_species = {};
    default_minor_species = {}; 
    default_xscale = 'linear';
    default_show = true;
    default_species_plt = 'mole';
    default_xunits = 'mm';
    default_xmult = 1.0E3;
    default_prnfig = false;
%
    p = inputParser;
    addRequired(p,'znd_output');
    addParameter(p,'maxx',default_maxx,@isnumeric);
    addParameter(p,'species_plt',default_species_plt);
    addParameter(p,'major_species',default_major_species);
    addParameter(p,'minor_species',default_minor_species);
    addParameter(p,'xscale',default_xscale);
    addParameter(p,'xunits',default_xunits);
    addParameter(p,'xmult',default_xmult);
    addParameter(p,'show',default_show,@islogical);
    addParameter(p,'prnfig',default_prnfig,@islogical);
    parse(p,znd_output,varargin{:});
    maxx = p.Results.maxx;
    major_species = p.Results.major_species;
    minor_species = p.Results.minor_species;
    xscale = p.Results.xscale;
    show = p.Results.show;
    species_plt = p.Results.species_plt;
    xunits = p.Results.xunits;
    xmult = p.Results.xmult;
    prnfig = p.Results.prnfig; 
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    if maxx == 0
        [~,k] = max(znd_output.T);
        if (k == 1)
            maxx = xmult*znd_output.ind_len_ZND*5;
        else
            maxx = xmult*znd_output.distance(k);
        end
    end
    
    if strcmp(string(xscale),'linear')
        minx = 0;
    elseif strcmp(string(xscale),'log')
        minx = xmult*1e-6;
    end
    font ='TimesRoman';
    fontsize = 12;
	set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);

    if show
        visible = 'On';
    else
        visible = 'Off';
    end   
    
	% Temperature as a function of position
    figT = figure('Visible',visible);
 	plot(xmult*znd_output.distance(:),znd_output.T(:));
    x_label = ['Distance (',xunits,')'];
 	xlabel(x_label,'FontSize',fontsize);
 	ylabel('Temperature (K)','FontSize',fontsize);
% 	title('ZND structure','FontSize',fontsize);
    xlim([minx maxx]);
 	set(gca,'FontSize',12,'XScale',xscale);
    if (prnfig) print('temperature','-depsc'); end
	
	% Pressure as a function of position
    figP = figure('Visible',visible);
	plot(xmult*znd_output.distance(:),znd_output.P(:));
 	xlabel(x_label,'FontSize',fontsize);
	ylabel('Pressure (Pa)','FontSize',fontsize);
%	title('ZND structure','FontSize',fontsize);
	xlim([minx maxx]);
    set(gca,'FontSize',12,'XScale',xscale);
    if (prnfig) print('pressure','-depsc'); end
    
    % Mach number as a function of position
    figM = figure('Visible',visible);
    plot(xmult*znd_output.distance(:),znd_output.M(:));
 	xlabel(x_label,'FontSize',fontsize);
    ylabel('M ','FontSize',fontsize);
%    title('ZND structure','FontSize',fontsize);
    xlim([minx maxx]);
    set(gca,'FontSize',12,'XScale',xscale);
    if (prnfig) print('Mach','-depsc'); end
    
    % Thermicity as a function of position
    figS = figure('Visible',visible);
    plot(xmult*znd_output.distance(:),znd_output.thermicity(:));
 	xlabel(x_label,'FontSize',fontsize);
    ylabel('Thermicity (s^{-1})','FontSize',fontsize);
%    title('ZND structure','FontSize',fontsize);
    xlim([minx maxx]);
    set(gca,'FontSize',12,'XScale',xscale);
    if (prnfig) print('thermicity','-depsc'); end
   
    figs = [figT figP figM figS];
    
    % Major species mass fractions as a function of position
    if ~isempty(major_species)
        if strcmp(string(major_species),'All')
            major_species = speciesNames(znd_output.gas1);
        end
        major_k = []; major_labels = {};
        species = speciesNames(znd_output.gas1);
        for i = 1:length(major_species)
            if any(ismember(species,major_species{i}))
                major_k(end+1) = speciesIndex(znd_output.gas1,major_species(i));
                major_labels{end+1} = major_species{i};
            end
        end
        if ~isempty(major_k)
            figMaj = figure('Visible',visible);
            set(gca,'linestyleorder',{'-','-.','--',':'},'nextplot','add','XScale',xscale,'yscale','log');
            if strcmp(string(species_plt),'mass')
                plot(xmult*znd_output.distance(:),znd_output.species(:,major_k));
                ylabel('Species mass fraction','FontSize',fontsize);
            elseif strcmp(string(species_plt),'mole')
                plot(xmult*znd_output.distance(:),znd_output.speciesX(:,major_k));
                ylabel('Species mole fraction','FontSize',fontsize);
            end
            axis([minx maxx 1.0E-10 1]);
           	xlabel(x_label,'FontSize',fontsize);
 %           title('ZND structure','FontSize',fontsize);
            legend(major_labels,'Location','southeast');
            if (prnfig) print('major_species','-depsc'); end
            figs(end+1) = figMaj;
        end
    end
    
    % Minor species mass fractions as a function of position
    if ~isempty(minor_species)
        minor_k = []; minor_labels = {};
        species = speciesNames(znd_output.gas1);
        for i = 1:length(minor_species)
            if any(ismember(species,minor_species{i}))
                minor_k(end+1) = speciesIndex(znd_output.gas1,minor_species(i));
                minor_labels{end+1} = minor_species{i};
            end
        end
        if ~isempty(minor_k)
            figMin = figure('Visible',visible);
            set(gca,'linestyleorder',{'-','-.','--',':'},'nextplot','add','XScale',xscale,'yscale','log');
            if strcmp(string(species_plt),'mass')
                plot(xmult*znd_output.distance(:),znd_output.species(:,minor_k));
                ylabel('Species mass fraction','FontSize',fontsize);
            elseif strcmp(string(species_plt),'mole')
                plot(xmult*znd_output.distance(:),znd_output.speciesX(:,minor_k));
                ylabel('Species mole fraction','FontSize',fontsize);
            end
            axis([minx maxx 1.0E-10 1]);
            xlabel(x_label,'FontSize',fontsize);
            %           title('ZND structure','FontSize',fontsize);
            legend(minor_labels,'Location','southeast');
            if (prnfig) print('minor_species','-depsc'); end
            figs(end+1) = figMin;
        end
    end
    
