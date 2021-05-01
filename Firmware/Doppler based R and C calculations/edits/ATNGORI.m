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
[fileName, path] = uigetfile('*.avi');
video = VideoReader([path fileName]);  
opFolder = fullfile(cd,fileName); 
if ~exist(opFolder, 'dir')            %if  not existing 
% mkdir ../UltraSound_RA Op_US_Frames; %make directory & execute as indicated in opfolder variable
mkdir (opFolder);
end
global totalFrames;
totalFrames = video.NumberOfFrames; %#ok<*VIDREAD>
fprintf('Number of frames in current ultrasound video %3.0f \n',totalFrames);
global arr_dia;
global p_val;
arr_dia = zeros(totalFrames,1);
p_val = zeros(totalFrames,1);
area_img=0;
r=0;
dia=0;
cir_area=0;
for count = 388 : 388
    img_filename = strcat('frame', num2str(count),'.jpg');
    frame_f = read(video, count);
    opFullFileName = fullfile(opFolder, img_filename);
    imwrite(frame_f, opFullFileName);
    img = imread(fullfile(opFolder,img_filename));
    img1=img;
  
    % % 
% R = img(:,:,1);
% G = img(:,:,2);
% B = img(:,:,3);
% Rmean = filter2(fspecial('average',3),R)/255;
% Gmean = filter2(fspecial('average',3),G)/255;
% Bmean = filter2(fspecial('average',3),B)/255;
% CL1 = adapthisteq(Rmean,'clipLimit',0.02,'Distribution','rayleigh');
% CL2 = adapthisteq(Gmean,'clipLimit',0.02,'Distribution','rayleigh');
% CL3 = adapthisteq(Bmean,'clipLimit',0.02,'Distribution','rayleigh');
% image1 = cat(3, CL1, CL2, CL3);
% img=image1
% 
    img = imbinarize(img);
    R = img(:,:,1); %Red Layer- gives mono chrome image
    G = img(:,:,2); %Green layer- monochrone 
    B = img(:,:,3); % Blue layer
    F = R-G;

       
    %% locations of white pixels
    d = size(F);
    p = 1;
    for r = 3:d(1)-3
        for c = 3: d(2)-3
            subimage = F(r-2:r+2, c-2:c+2);
            [w,x] = imhist(subimage);
            %to store location of pixels whose adjacent 2 pixels are white
            if w(end) >= 4
                pos(p,1) = r;
                pos(p,2) = c;
                p = p+1;
            end
        end
    end
    
%     p=50;

    

    if ( (p~=1))

        rmax = max(pos(:,1));
        rmin = min(pos(:,1));
        cmax = max(pos(:,2));
        cmin = min(pos(:,2));
    
        R = img1(:,:,1); %Red Layer- gives mono chrome image
        G = img1(:,:,2); %Green layer- monochrone 
        B = img1(:,:,3); 
%         R = min(R, [], 3); 
%         G = min(G, [], 3); 
%         B = min(B, [], 3); 
        R=R<30;
        G=G<90;
        B=B<90;
%         ar1=R+G+B;
        
        ar1 = R(rmin-10:rmax+10,cmin-10:cmax+10);
        ar2 = G(rmin-10:rmax+10,cmin-10:cmax+10);
        ar3 = B(rmin-10:rmax+10,cmin-10:cmax+10);
        ar1=ar1+ar2+ar3;
        ar1=imcomplement(ar1);
        ar1=imclearborder(ar1); 
%         imshow(ar1);
        CC = bwconncomp(ar1);
        disp(CC);           
        numPixels = cellfun(@numel,CC.PixelIdxList);   
        [maxPixel, indexOfMax] = max(numPixels);  
        largest = zeros(size(ar1));    
        largest(CC.PixelIdxList{indexOfMax}) = 1;                     
        figure;
        imshow(largest); 
        BWimg_filename = strcat('bw', num2str(count),'.jpg');
        opFullFileName1 = fullfile(opFolder, BWimg_filename);
        imwrite(largest, opFullFileName1);
        
        area_img = bwarea(largest);
        r = sqrt(area_img/pi);
        r = 0.003631*r;
        cir_area = pi*r*r;
        arr_dia(count) = cir_area;
        p_val(count)=p;

%     elseif ((p>=300))
%         rmax = max(pos(:,1));
%         rmin = min(pos(:,1));
%         cmax = max(pos(:,2));
%         cmin = min(pos(:,2));
% %         %%
% %         R = img1(:,:,1); %Red Layer- gives mono chrome image
% %         G = img1(:,:,2); %Green layer- monochrone 
% %         B = img1(:,:,3);
% %         R=R<90;B=B<90;G=G<90;
%         
%         ar1 = R(rmin-50:rmax+50,cmin-50:cmax+50);
%         ar2 = G(rmin-50:rmax+50,cmin-50:cmax+50);
%         ar3 = B(rmin-50:rmax+50,cmin-50:cmax+50);
%     
%         ar1=  ar1+ar2+ar3;
% 
%         CC = bwconncomp(ar1);
%         disp(CC);           
%         numPixels = cellfun(@numel,CC.PixelIdxList);   
%         [maxPixel, indexOfMax] = max(numPixels);  
%         largest = zeros(size(ar1));    
%         largest(CC.PixelIdxList{indexOfMax}) = 1;                     
%         
%         BWimg_filename = strcat('bw', num2str(count),'.jpg');
%         opFullFileName1 = fullfile(opFolder, BWimg_filename);
%         imwrite(largest, opFullFileName1);
%         
%         area_img = bwarea(largest);
%         r = sqrt(area_img/pi);
% %         dia = 2*r;
%         r = 0.003631*r;
% %         fprintf('diameter of cropped circle %fcm \n',dia1)
% %         r1 = dia1/2;
%         cir_area = pi*r*r;
%         arr_dia(count) = cir_area;
%         p_val(count)=p;
%         fprintf('Area of circle = %fcm2\n',cir_area);

    elseif p==1
        arr_dia(count) = cir_area;
        p_val(count)=p;

    end

      
end
frame_No(:,1) = 1:totalFrames;
T = table(frame_No , arr_dia,p_val);
% excel_filename = 'RadialArtery_Dia.xlsx';
excel_filename = strcat(fileName,'.xlsx');
fullFileName = fullfile(opFolder, excel_filename);
writetable (T,fullFileName);
fprintf('program ends....');
return


