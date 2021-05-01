    %method 2 - Crop Method 
% find diameter by Croping the image a  aznd then process the cropped image
% adjust threshold value(grayImage < x) if circle or image area is not properly deducted
%Steps:
%1 - calculate area in pixel
%2 - calculate radius from area and then diameter in pix
%3 - calculate diameter in cm by multiplying dia in pixel with cm/pxl value (where cm/pixel value is calibrated with known circle diameter)
%4 - again area is calculated from diamter in cm to get area in cm2.
%%
clc;
clear;
close all;
[fileName, path] = uigetfile('*.png; *.jpg');
img = imread([path fileName]);
imshow(img);
hold on;
hRect = imcrop;
%crop and hit enter or right click and select crop image
figure, imshow(hRect);
grayImage = min(hRect, [], 3); 
try
% 97 or 60 for dbs , US color is 40
   BW = grayImage<50;
   imshow(BW);
catch e
   BW = grayImage<93;
%    BW = grayImage<93;
end

edges_depth_distance=edge(BW,'canny',[0 0.5],4);
% edges_depth_distance=edge(BW,'sobel',.03,'horizontal');
BW = bwmorph(edges_depth_distance, 'bridge')

se = strel('line', 1, 180); % Structuring element for dilation
BW = imdilate(BW, se);
figure, imshow(BW);

% BW = grayImage<40;
imshow(BW);
BW = imfill(BW, 'holes'); 

   
imshow(BW); 
BW = imclearborder(BW); 


imshow(BW); 
CC = bwconncomp(BW);
disp(CC);           
numPixels = cellfun(@numel,CC.PixelIdxList);   
[maxPixel, indexOfMax] = max(numPixels);  
largest = zeros(size(BW));    
largest(CC.PixelIdxList{indexOfMax}) = 1;                     
figure;
imshow(largest); 
area_img = bwarea(largest);
r = sqrt(area_img/pi);
dia = 2*r;
% dia1 = 0.027093*dia;
dia1 = 0.003631*dia;
% dia1 = 0.0107508*dia;  for dbs
% dia1 = 0.008626*dia;
% dia1 = 2.7/dia;

% disp(dia1);
fprintf('diameter of cropped circle %fcm \n',dia1)

r1 = dia1/2;
cir_area = pi*r1*r1;
fprintf('Area of circle = %fcm2',cir_area);
% disp(cir_area);
%%

