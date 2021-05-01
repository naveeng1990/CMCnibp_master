
% /* 
%  * File:  
%  * Author: Naveen Gangadharan
%  *
%  * Created on 12 March 2020, 00:42 ; Original copies from 2019
%  */
%%
% prerequisite :
% D folder(work place)should be unlocked and Program/output path should be opened in matlab before execution 
% Input Data - Ultrasound image should be taken without erroneous noise like structures(no extra color indications near artery border).
% Grayscale of ultrasound background should be adjusted properly without any black background near the artery border(prefer white/gray)
% clear output field before each run
%%
clc;   
close all;  
clear;  
imtool close all;
workspace;
% [fileName, path] = uigetfile('*.avi');
[fileName1, path] = uigetfile('*.avi');
video = VideoReader([path fileName1]);  
opFolder = fullfile(cd,fileName1); 
if ~exist(opFolder, 'dir')            %if  not existing 
% mkdir ../UltraSound_RA Op_US_Frames; %make directory & execute as indicated in opfolder variable
mkdir (opFolder);
end

[fileName2, path] = uigetfile('*.txt');
disp([path fileName2]);
data1 = dlmread([path fileName2]);


[fileName3, path] = uigetfile('*.txt');
disp([path fileName3]);
data2 = dlmread([path fileName3]);

global totalFrames;
totalFrames = video.NumberOfFrames; %#ok<*VIDREAD>
totalFrames  =800;
fprintf('Number of frames in current ultrasound video %3.0f \n',totalFrames);
global arr_dia;
global f_vel;
global f_phase;
global frate;
arr_dia = zeros(totalFrames,1);
f_vel = zeros(totalFrames,1); 
f_vel = zeros(totalFrames,1); 
f_phase =zeros(totalFrames,1);
frate=video.FrameRate;


% ar = data1(:,2); %#vol flow, 10 Hz 
ar = data1(:,1);

% pr= data2(:,1);  %#second excel with only pressure, more smaples 1KHz file used
pr= data2(:,2);


fs = 1000;
Y=fft(pr);
Z=fftshift(Y);
L=length(pr);
if mod(L,2) == 1
    P2 = abs(Y/L);
    P1 = P2(1:(L+1)/2);
    P3= angle(Z);
%         P4 = P3(1:(L+1)/2);
    ly = length(Y);
    f1 = (-ly/2:ly/2-1)/ly*fs;
else
    P2 = abs(Y/L);
    P1 = P2(1:L/2+1);
    P3= angle(Z);
%         P4 = P3(1:L/2+1);
    ly = length(Y);
%     f1 = (-(ly-1)/2:(ly-1)/2-1)/ly*fs;
        
end

P1(2:end-1) = 2*P1(2:end-1);
%     P4(2:end-1) = 2*P4(2:end-1);
f = fs*(0:(L/2))/L;

ly = length(Y);
% f1 = (-ly/2:ly/2-1)/ly*fs;

pks=[];
locs=[];
pks1=[];
locs1=[];
[pks,locs]=findpeaks(P1,f','MinPeakProminence',0.0001);
% [pks1,locs1]=findpeaks(P3/pi,f1','MinPeakProminence',0.0001);
    
sum=0;
num=0;
mean_f=0;
for n=1: length(locs)
    sum = sum + (pks(n,1)*locs(n,1));
    num = num+pks(n,1);
mean_f = sum/num;
end

% f_vel(count)= mean_f;
% f_phase(count)= max(locs1);
% plot(f(1:250),P1(1:250));
plot(f,P1);
% stem(f1,P3/pi);




fss = 17;   % need atleast 30fps

Y1=fft(ar);
Z1=fftshift(Y1);
L1=length(ar);
if mod(L1,2) == 1
    P22 = abs(Y1/L1);
    P11 = P22(1:(L1+1)/2);
    P33= angle(Z1);
else
    P22 = abs(Y1/L1);
    P11 = P22(1:L1/2+1);
    P33= angle(Z1);
end

P11(2:end-1) = 2*P11(2:end-1);
f1 = fss*(0:(L1/2))/L1;

% lyy = length(Y1);
% f11 = (-lyy/2:lyy/2-1)/lyy*fss;

pkss=[];
locss=[];
pkss1=[];
locss1=[];
[pkss,locss]=findpeaks(P11,f1','MinPeakProminence',0.0001);
% [pkss1,locss1]=findpeaks(P33/pi,f11','MinPeakProminence',0.0001);
    
sum1=0;
num1=0;
mean_f1=0;
for n=1: length(locss)
    sum1 = sum1 + (pkss(n,1)*locss(n,1));
    num1 = num1+pkss(n,1);
mean_f1 = sum1/num1;
end
xdft=P11;
plot(f1(1:100),P11(1:100));
% stem(f1,P3/pi);


lpad1 = length(pr);
xdft1 = fft(ar,lpad1);
xdft1 = xdft1(1:lpad1/2+1);
xdft1 = xdft1/length(ar);
xdft1(2:end-1) = 2*xdft1(2:end-1);
freq1 = 0:fss/lpad1:fss/2;

plot(freq1,abs(xdft1))
% dfts=envelope(abs(xdft1),1000,'peak')
% plot(freq1,dfts)

% f=freq1'

% xt=ifft(xdft1);

hold on
% plot(freq1,ones(2*length(ar)/2+1,1),'LineWidth',2)
xlabel('Hz')
ylabel('Amplitude')
hold off


flowfft=abs(xdft1);
pressfft=abs(P1(1:250))
f=freq1';


% xtt=fft(xt);
E= abs(P1)/abs(xdft1);
E= envelope(abs(E),1000,'peak')

P1= P1(1:length(xdft1))
plot(freq1,(E));





% frame_No(:,1) = 1:totalFrames;
% T = table(frame_No , arr_dia);
% T1 = table(frame_No , f_vel);
% T2 = table(frame_No , f_phase);
% fileName3="phase";
% % excel_filename = 'RadialArtery_Dia.xlsx';
% excel_filename1 = strcat(fileName1,'.xlsx');
% excel_filename2 = strcat(fileName2,'.xlsx');
% excel_filename3 = strcat(fileName3,'.xlsx');
% fullFileName1 = fullfile(opFolder, excel_filename1);
% fullFileName2 = fullfile(opFolder, excel_filename2);
% fullFileName3 = fullfile(opFolder, excel_filename3);
% writetable (T,fullFileName1);
% writetable (T1,fullFileName2);
% writetable (T2,fullFileName3);
% fprintf('program ends....');



