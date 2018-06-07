clear 
close all
clc

g= dlmread('LMglane');
% n = dlmread('n');

A=[0.0225 0.0225 0.0225  0.0225 0.0225 0.0225];

% cov_00 = dlmread('print/print_cov_0');
% cov_0=cat(1,A,cov_00);
cov_10 = dlmread('print/print_cov_1');
% cov_1=cat(1,A,cov_10);
s_0= dlmread('print/print_state_vec3');
% f_1= dlmread('print/FilteredBeam_1');
s_1= dlmread('print/print_state_1');
% s_0= dlmread('print/print_state_0');
ms=size(cov_10,1);

% %% PRINTING VELOCITY AMPLITUDE 
% V_1=s_1(:,3:4);
% P_1=s_1(:,1:3);
% n_v=ones(ms,1);
% n_p=n_v;
% 
% for i =1:ms
%    n_v(i)=norm(V_1(i,:));
%    n_p(i)=norm(P_1(i,:));
% end
% 
% figure1=figure('NumberTitle', 'off', 'Name', 'Compare Velocity Magnitude')
% plot(n_v, 'Linewidth',2,'MarkerSize',1,'Color',[0.200000002980232 0.200000002980232 1])
% hold on
% grid on
% plot(n_p, 'Linewidth',1.1, 'MarkerSize',1,'Color',[0.635294139385223 0.0784313753247261 0.184313729405403])
% % Create textbox
% annotation(figure1,'textbox',...
%     [0.148368421052632 0.78556263269639 0.137461538461538 0.0530785562632696],...
%     'Color',[0.400000005960464 0 0.200000002980232],...
%     'String','Q = 0.1',...
%     'FitBoxToText','off',...
%     'EdgeColor','none');
% % Create textbox
% annotation(figure1,'textbox',...
%     [0.147558704453441 0.83864118895966 0.137461538461538 0.0530785562632696],...
%     'Color',[0 0 1],...
%     'String',{'Qv = 0.1'},...
%     'FitBoxToText','off',...
%     'EdgeColor','none');
% % Create line
% annotation(figure1,'line',[0.46547447086987 0.463855037671489],...
%     [0.105986676795419 0.923396443249772],'Color',[1 0 0],'LineWidth',1,...
%     'LineStyle','--');
% xlabel('t - [simulation step]') % x-axis label
% ylabel('norm ') % y-axis label
% title( 'Velocity Magnitude Comparison - State Vector With Only Velocity and Parameter  ' ) ;
%% PRINTING PARAMETER ESTIMATION 
% 
% p_0=s_0(1:ms,4);
% p_1=s_1(1:ms,4);
% 
% figure2=figure('NumberTitle', 'off', 'Name', 'Compare Param Estimation Magnitude')
% plot(p_1, 'Linewidth',2,'MarkerSize',1,'Color',[0.200000002980232 0.200000002980232 1])
% hold on
% grid on
% plot(p_0, 'Linewidth',1.1, 'MarkerSize',1,'Color',[0.635294139385223 0.0784313753247261 0.184313729405403])
% % Create textbox
% annotation(figure2,'textbox',...
%     [0.148368421052632 0.78556263269639 0.137461538461538 0.0530785562632696],...
%     'Color',[0.400000005960464 0 0.200000002980232],...
%     'String','Q = 0.1',...
%     'FitBoxToText','off',...
%     'EdgeColor','none');
% % Create textbox
% annotation(figure2,'textbox',...
%     [0.147558704453441 0.83864118895966 0.137461538461538 0.0530785562632696],...
%     'Color',[0 0 1],...
%     'String',{'Q = 0.0001'},...
%     'FitBoxToText','off',...
%     'EdgeColor','none');
% % Create line
% annotation(figure2,'line',[0.46547447086987 0.463855037671489],...
%     [0.105986676795419 0.923396443249772],'Color',[1 0 0],'LineWidth',1,...
%     'LineStyle','--');
% xlabel('t - [simulation step]') % x-axis label
% ylabel('norm ') % y-axis label
% title( 'Parameter Estimation Comparison - State Vector With Only Velocity and Parameter ' ) ;


% PRINTING COVARIANCES 

% figure3=figure('NumberTitle', 'off', 'Name', 'Covariance Test_0')
% subplot(3,1,1)
% plot(cov_1(1:ms,1))
% hold on
% plot(0,cov_1(1,1),'r*')
% grid on
% % legend(' x','pos y','pos z','Location','NorthEastOutside');
% xlabel('t - [simulation step]') % x-axis label
% ylabel('covariance') % y-axis label
% % xlim([-1000 8000]);
% ylim([0 0.1])
% title( 'Velocity X Covariance - State Vector With Only Velocity and Parameter ' ) ;
% subplot(3,1,2)
% plot(cov_1(1:ms,2))
% hold on
% plot(0,cov_1(1,2),'r*')
% % xlim([1:ms]);
% ylim([0 0.1])
% grid on
% xlabel('t - [simulation step]') % x-axis label
% ylabel('covariance') % y-axis label
% title( 'Velocity Y Covariance - State Vector With Only Velocity and Parameter' ) ;
% subplot(3,1,3)
% plot(cov_1(1:ms,3))
% hold on
% plot(0,cov_1(1,3),'r*')
% % xlim([-1000 8000]);
% grid on
% ylim([0 0.1])
% xlabel('t - [simulation step]') % x-axis label
% ylabel('covariance') % y-axis label
% title( 'Velocity Z Covariance- State Vector With Only Velocity and Parameter ' ) ;
% % Create textbox
% annotation(figure3,'textbox',...
%     [0.406107408386963 0.95273809188744 0.319647773279352 0.0260960334029209],...
%     'String','Analysis of Velicity Covariance when Q = 0.0001',...
%     'FontWeight','bold',...
%     'FitBoxToText','off',...
%     'EdgeColor','none');
% 
% 
% figure4=figure('NumberTitle', 'off', 'Name', 'Covariance Test_0')
% subplot(3,1,1)
% plot(cov_0(1:ms,1))
% hold on
% plot(0,cov_0(1,1),'r*')
% grid on
% % legend(' x','pos y','pos z','Location','NorthEastOutside');
% xlabel('t - [simulation step]') % x-axis label
% ylabel('covariance') % y-axis label
% % xlim([-1000 8000]);
% ylim([0 0.1])
% title( 'Velocity X Covariance - State Vector With Only Velocity and Parameter ' ) ;
% subplot(3,1,2)
% plot(cov_0(1:ms,2))
% hold on
% plot(0,cov_0(1,2),'r*')
% % xlim([1:ms]);
% ylim([0 0.1])
% grid on
% xlabel('t - [simulation step]') % x-axis label
% ylabel('covariance') % y-axis label
% title( 'Velocity Y Covariance - State Vector With Only Velocity and Parameter' ) ;
% subplot(3,1,3)
% plot(cov_0(1:ms,3))
% hold on
% plot(0,cov_0(1,3),'r*')
% % xlim([-1000 8000]);
% grid on
% ylim([0 0.1])
% xlabel('t - [simulation step]') % x-axis label
% ylabel('covariance') % y-axis label
% title( 'Velocity Z Covariance- State Vector With Only Velocity and Parameter ' ) ;
% % Create textbox
% annotation(figure4,'textbox',...
%     [0.406107408386963 0.95273809188744 0.319647773279352 0.0260960334029209],...
%     'String','Analysis of Velicity Covariance when Q = 0.1',...
%     'FontWeight','bold',...
%     'FitBoxToText','off',...
%     'EdgeColor','none');
%% PRINTING DISTANCE FILTER GROUND TRUTH 

F_1=s_1(:,1:3);
F_0=s_0(:,1:3);
% F_1=f_1(1:ms,2:4);
% F_0=s_0(:,1:3);
% F_5=f_5(:,2:4);
G=g(1:size(F_0,1),2:4);
% F_1=F_1(1:size(10000,1),:);
% F_0=F_0(1:size(10000,1),:);


figure1=figure('NumberTitle', 'off', 'Name', 'Distance Test 3 ')
axes1 = axes('Parent',figure1);
hold(axes1,'on');
p1=[-1,-3,-0.8]';
p2=[-1,10,-0.8]';
p3=[1.8,-3,-0.8]';
p4=[1.8,10,-0.8]';
x = [p1(1) p2(1) p4(1) p3(1)];
y = [p1(2) p2(2) p4(2) p3(2)];
z = [p1(3) p2(3) p4(3) p3(3)];
fill3(x, y, z,[0.992156863212585 0.917647063732147 0.796078443527222]);
% hold on
% ob1=[0.7654,-3,-0.4]';
% ob2=[0.7654,10,-0.4]';
% ob3=[0.7654,-3,-0.6]';
% ob4=[0.7654,10,-0.6]';
% A = [ob1(1) ob2(1) ob4(1) ob3(1)];
% B = [ob1(2) ob2(2) ob4(2) ob3(2)];
% C = [ob1(3) ob2(3) ob4(3) ob3(3)];
% fill3(A, B, C,[255 255 204 1]);
% grid on
% hold on
plot3(0,0,0, 'ko')
% plot3(F_1(:,3),F_1(:,1),F_1(:,2), 'Linewidth',2,'MarkerSize',1,'Color',[0.200000002980232 0.200000002980232 1])
% hold on
plot3(G(:,3),G(:,1),G(:,2),'-g','LineWidth',1.1,'Color',[1 0 0])
hold on
plot3(F_0(:,3),F_0(:,1),F_0(:,2), 'Linewidth',1.1, 'MarkerSize',1,'Color',[0 1 0])
hold on
plot3(F_1(:,3),F_1(:,1),F_1(:,2), 'Linewidth',1.1, 'MarkerSize',1,'Color',[0.0 0.0 1])

xlabel('z') % x-axis label
ylabel('x') % y-axis label
zlabel('y')
% hold on
title('Distance between Fiter Ball and Ground Truth Ball')
view(axes1,[50.8000000000001 6]);
grid(axes1,'on');
% Create textbox
annotation(figure1,'textbox',...
    [0.389534176075146 0.459593480428779 0.137461538461538 0.0530785562632697],...
    'Color',[1 0 0],...
    'String','FILTER RESULT',...
    'FontWeight','bold',...
    'FitBoxToText','off',...
    'EdgeColor','none');

% Create textbox
annotation(figure1,'textbox',...
    [0.411807162939794 0.428613718013435 0.137461538461538 0.0530785562632695],...
    'Color',[0 1 0],...
    'String','GROUND TRUTH',...
    'FontWeight','bold',...
    'FitBoxToText','off',...
    'EdgeColor','none');

% Create textbox
annotation(figure1,'textbox',...
    [0.188148902316401 0.305404300733441 0.11478947368421 0.0510638297872341],...
    'String',{'Collision Plane'},...
    'FitBoxToText','off',...
    'EdgeColor','none');

% Create arrow
annotation(figure1,'arrow',[0.354083380925186 0.354083380925186],...
    [0.810099252934899 0.711846318036287]);

% Create textbox
annotation(figure1,'textbox',...
    [0.333381496287836 0.737459978655284 0.0229862935465449 0.0405549626467454],...
    'String',{'v ?'},...
    'FitBoxToText','off',...
    'EdgeColor','none');

%% PRINTTING PARAMETER ESTIMATION 
% p_0=s_0(:,7);
% % p_0=s_0(:,7);
% 
% figure('NumberTitle', 'off', 'Name', 'Distance Test 5 ')
% plot(p_0)
% grid on
% title({'Pos, Vel, Param Q = R','Plane Distance Estimation '})
% xlabel('t - [simulation step]') % x-axis label
% ylabel('Distance - [m]') % y-axis label
% set(gcf, 'PaperUnits', 'centimeters');
% set(gcf, 'PaperPosition', [0 0 60 30]); %x_width=10cm y_width=15cm
% saveas(gcf,'print/param_0.bmp')
% % 
% % figure('NumberTitle', 'off', 'Name', 'Distance Test 5 ')
% % plot(p_0)
% % grid on
% % title({'Asynchronous Pos, Vel, Param Q = R','Plane Distance Estimation '})
% % xlabel('t - [simulation step]') % x-axis label
% % ylabel('Distance - [m]') % y-axis label
% % set(gcf, 'PaperUnits', 'centimeters');
% % set(gcf, 'PaperPosition', [0 0 60 30]); %x_width=10cm y_width=15cm
% % saveas(gcf,'print/param_0.bmp')
% 
% 
% figure('NumberTitle', 'off', 'Name', 'Distance Test 5 ')
% plot(s_0(4:6))
% grid on
% title({'Asynchronous Pos, Vel, Param Q = R','Velocity Estimation '})
% xlabel('t - [simulation step]') % x-axis label
% ylabel('Velocity - [m/s]') % y-axis label
% set(gcf, 'PaperUnits', 'centimeters');
% set(gcf, 'PaperPosition', [0 0 60 30]); %x_width=10cm y_width=15cm
% saveas(gcf,'print/param_0.bmp')
% 
% 
% % figure('NumberTitle', 'off', 'Name', 'parameter')
% % plot(s_5(:,7:9))
% % grid on
% % hold on
% % plot(d(:,2))
% % legend('force x','force y','force z','Expected Force Null','Location','NorthEastOutside');
% % title({'Pos, Vel Q < R','Correction Forces Magnitude'})
% % xlabel('t - [simulation step]') % x-axis label
% % ylabel('magnitude - [m]') % y-axis label