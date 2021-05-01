% /* 
%  * File:  
%  * Author: Naveen Gangadharan
%  *
%  * Created on 12 March 2020, 00:42 ; Original copies from 2019
%  */


% Area and velocity calculation from ultrasound doppler images and audio
% signal 
clear all;
workspace;
[fileName, path] = uigetfile('*.avi');
video = VideoReader([path fileName]);  
opFolder = fullfile(cd,fileName); 
% load('VideoFrames.mat');
% load('VideoFramesDec.mat'); this was used 
% load('VideoFrames_raja12.mat');
load('VideoFrames_fq11.mat');
totalFrames = video.NumberOfFrames
% totalFrames= 82
for i = 1:totalFrames
    img= data.frames{i}; % for ith frame
    img = read(video, i);
    time(i)=video.CurrentTime;    
    figure(1);imshow(img);
    
    img_filename = strcat('frame', num2str(i),'.jpg');
    opFullFileName = fullfile(opFolder, img_filename);
    imwrite(img, opFullFileName);
    
%%  To create mask
    binImg = imbinarize(img);   %binarize the image
    binRed = binImg(:,:,1);     %consider one channel, Red here  
    edcanny = edge(binRed,'canny',0.8);%find edges in the image
%     figure(2);imshow(edcanny);
    CC = bwconncomp(edcanny); % to detect volume selector edge of all edges
    numPixels = cellfun(@numel,CC.PixelIdxList);   
    [~, indexOfMax] = max(numPixels);  
    forMask = zeros(size(edcanny));    
    forMask(CC.PixelIdxList{indexOfMax}) = 1; 
    tempMask = imfill(forMask,'holes');
%     figure(3);imshow(tempMask);
%% Resizing the mask
    imgcorner = corner(tempMask);
    maskImage = zeros(size(tempMask));
    xmin = min(imgcorner(:,1));
    xmax = max(imgcorner(:,1));
    ymin = min(imgcorner(:,2));
    ymax = max(imgcorner(:,2));
    maskImage(ymin+5:ymax-5,xmin+5:xmax-5) = 1;
    figure(4);imshow(maskImage);
%% Seperate non-binary RGB channels
    R = img(:,:,1); %Red Layer- gives mono chrome image
    G = img(:,:,2); %Green layer- monochrone 
    B = img(:,:,3); % Blue layer
%% binarize with adaptive thresholding
%     BR = imbinarize(R,'adaptive');
%     BG = imbinarize(G,'adaptive');
%     BB = imbinarize(B,'adaptive');
    
    % threshold determined from imhist()
%     BR = R > 20;
%     BG = G > 17;
%     BB = B > 20;
%     for patient rajesh 60
% for selvam 50
% pulkesh 250
    BR = R > 250;  
    BG = G > 250;
    BB = B > 250;
    
%%  Enhanced Image
    sumImage = BR + BG+BB;
    
%     sumImage = BR;
%     figure(5);imshow(sumImage);
%% Masking the Region of Interest(ROI)
    imageROI = sumImage.* maskImage;
%     figure(6);imshow(imageROI);
 %% show largest CC   
        CC = bwconncomp(imageROI);
%         disp(CC);           
        numPixels = cellfun(@numel,CC.PixelIdxList);   
        [maxPixel, indexOfMax] = max(numPixels);  
        largest = zeros(size(imageROI));    
        largest(CC.PixelIdxList{indexOfMax}) = 1;                     
        figure(2);imshow(largest);
        
        BWimg_filename = strcat('bw', num2str(i),'.jpg');
        opFullFileName1 = fullfile(opFolder, BWimg_filename);
        imwrite(largest, opFullFileName1);
%% verifying cropped original
        cropedOriginal = im2double(img).*largest;
%         figure(2);imshow(cropedOriginal);
%% area calculation
        area_img = bwarea(largest);
        r = sqrt(area_img/pi);
        r = 0.003631*r;
        cir_area = pi*r*r;
%         multiply above line by 1.3 for dbt report as calib was not known
       arr_dia(i) = cir_area;
end

frame_Numb(:,1) = 1:totalFrames;
arr_dia=arr_dia';
time=time';
% T = table(frame_Numb ,time, arr_dia);
T = table(frame_Numb, arr_dia);
excel_filename = strcat(fileName,'.xlsx');
fullFileName = fullfile(opFolder, excel_filename);
writetable (T,fullFileName);
fprintf('program ends....');
plot(frame_Numb,arr_dia)
xlabel('frame no:') 
ylabel('area in sq.cm.') 
title('Frame-wise vessel cross-sectional area')


