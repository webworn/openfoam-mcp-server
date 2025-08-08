%% plotter. - opens csv file and plots data. Edit to suit.
clc;clear all; close;
filenamea = 'NO(X)-partition.csv';
filenameb = 'NO(A)-partition.csv';
filenamec = 'NO-partition.csv';
filenamed = 'NO-cantera.csv';
row_start = 11;
A = csvread(filenamea,row_start,0);
B = csvread(filenameb,row_start,0);
C = csvread(filenamec,row_start,0);
row_start = 12;
D = csvread(filenamed,row_start,0);
font ='TimesRoman';
fontsize = 12;
set(0,'DefaultTextFontName',font,'DefaultTextFontSize',fontsize,'DefaultAxesFontName',font,'DefaultAxesFontSize',fontsize,'DefaultLineLineWidth',1,'DefaultLineMarkerSize',7.75);
% plot temperature
figure('Name','plot 1' );
minx = min(A(:,1));
maxx = max(A(:,1));
% maxy = max(A(:,2));
% miny = min(A(:,2));
maxy = 6;
miny = 3.;
fontsize=12;
set(gca,'FontSize',fontsize,'LineWidth',2);
plot(A(:,1),A(:,2),'k','linewidth',1);
hold on
plot(B(:,1),B(:,2),'--','linewidth',1);
plot(C(:,1),C(:,2),'-.','linewidth',1,'MarkerSize',1,'MarkerIndices',5);
plot(D(:,1),D(:,2),':','linewidth',1,'MarkerSize',1,'MarkerIndices',5);
axis([minx maxx miny maxy]);
xlab = xlabel('$T$ (K)','FontSize',fontsize);
ylab = ylabel('$C_p/R$ ','FontSize',fontsize);
set(xlab,'Interpreter','latex');
set(ylab,'Interpreter','latex');
str_1 = ['NO(X)'];
str_2 = ['NO(A)'];
str_3 = ['NO (First 15 states)'];
str_4 = ['NO (NASA 20000 K fit))'];
leg = legend(str_1,str_2,str_3,str_4','Location','northwest');
set(leg,'Interpreter','latex');
hold off

