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
data = dlmread([path fileName2]);

global totalFrames;
totalFrames = video.NumberOfFrames; %#ok<*VIDREAD>
% totalFrames  =300;
fprintf('Number of frames in current ultrasound video %3.0f \n',totalFrames);
global arr_dia;
global f_vel;
global f_phase;
global frate;
arr_dia = zeros(totalFrames,1);
f_vel = zeros(totalFrames,1); 
f_vel1 = zeros(totalFrames,1); 
f_phase =zeros(totalFrames,1);
time1 =zeros(totalFrames,1);
frate=video.FrameRate;
time(1)=0.0333;

for count = 2 : totalFrames
    img_filename = strcat('frame', num2str(count),'.jpg');
    frame_f = read(video, count);
    
    time1(count)=video.CurrentTime;
    x = data(:,1);
    y1 = data(:,2); 
%     y1=abs(y1);  % added new
%     y1=y1+2;  % added new
    index1=knnsearch(x,time1(count-1));
    index2=knnsearch(x,time1(count));
    x=x(index1:index2);
    y1=y1(index1:index2);
    
    fs = 4000;
    x=y1;
    ax = real(x);
    Y=fft(ax);
    Z=fftshift(Y);
    L=length(ax);
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
        f1 = (-(ly-1)/2:(ly-1)/2-1)/ly*fs;
        
     

    end
    P1(2:end-1) = 2*P1(2:end-1);
%     P4(2:end-1) = 2*P4(2:end-1);
    f = fs*(0:(L/2))/L;
    pks=[];
    locs=[];
    pks1=[];
    locs1=[];
    [pks,locs]=findpeaks(P1,f','MinPeakProminence',0.0001);
%     [pks1,locs1]=findpeaks(P3/pi,f1','MinPeakProminence',0.0001);
    
    sum=0;
    num=0;
    mean_f=0;
    for n=1: length(locs)
        sum = sum + (pks(n,1)*locs(n,1));
        num = num+pks(n,1);
    mean_f = sum/num;
    end
%     
%     A=[pks,locs];
%     [u,v,s]=svd(A);
%     f_vel(count)= max(pks);
    f_vel(count)= mean_f *0.013967468175389; 
    %calculating vel assuming angle of known deg 45 see excel and 8M transmitted freq.
% D:\2020 compre time -Present\Obj 2 Ultrasound exp\0000 Ultrasouund reccent _ using versalab Z cLCULtion\Aug20 pgm\00000 correct impedance calc\
% 0000 latest analysis dec 2020 and details in diary nov 4 page
    
    
%     f_phase(count)= max(locs1);
%     plot(f,P1);
%     stem(f1,P3/pi);
   
%     opFullFileName = fullfile(opFolder, img_filename);
%     imwrite(frame_f, opFullFileName);
%     img = imread(fullfile(opFolder,img_filename));
%     grayImage = min(img, [], 3);  
%     BW = grayImage<30;    
%     BW = imfill(BW, 'holes'); 
%     BW = imclearborder(BW);
% %     imshow(BW);
%     CC = bwconncomp(BW); 
%     numPixels = cellfun(@numel,CC.PixelIdxList);  
%     [maxPixel, indexOfMax] = max(numPixels);  
%     largest = zeros(size(BW));  
%     largest(CC.PixelIdxList{indexOfMax}) = 1;          
% %     imshow(largest); 
%     BWimg_filename = strcat('bw', num2str(count),'.jpg');
%     opFullFileName1 = fullfile(opFolder, BWimg_filename);
%     imwrite(largest, opFullFileName1);
%     area_img = bwarea(largest);    
%     r = sqrt(area_img/pi);
%     dia = 2*r;
%     dia = 0.003631*dia;
%     arr_dia(count) = dia;
    
    
end
frame_Num(:,1) = 1:totalFrames;
frame_Num=frame_Num';
arr_dia=arr_dia';
T = table(frame_Num , arr_dia);
T1 = table(frame_Num , f_vel);
T2 = table(frame_Num, f_phase);
fileName3="phase";
% excel_filename = 'RadialArtery_Dia.xlsx';
excel_filename1 = strcat(fileName1,'.xlsx');
excel_filename2 = strcat(fileName2,'.xlsx');
excel_filename3 = strcat(fileName3,'.xlsx');
fullFileName1 = fullfile(opFolder, excel_filename1);
fullFileName2 = fullfile(opFolder, excel_filename2);
fullFileName3 = fullfile(opFolder, excel_filename3);
writetable (T,fullFileName1);
writetable (T1,fullFileName2);
writetable (T2,fullFileName3);
fprintf('program ends....');

% f_vel1=f_vel
plot(frame_Num,f_vel)
xlabel('frame no:') 
ylabel('flow velocity in cm/s') 
title('Frame-wise blood flow velocity')
% 
% x = data(:,1);
% y1 = data(:,2); 
% 
% index1=knnsearch(x,time1);
% index2=knnsearch(x,time1+(1/frate));
% x=x(index1:index2);
% y1=y1(index1:index2);
% 
% fs = 4000;
% t=x;
% x=y1;
% ax = real(x);
% [t1,x1]=instfreq(ax,fs,'Method','hilbert')
% plot(x1,abs(t1))

