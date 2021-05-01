% /* 
%  * File:  
%  * Author: Naveen Gangadharan
%  *
%  * Created on 12 March 2020, 00:42 ; Original copies from 2019
%  */

clc;   
close all;  
clear;  
imtool close all;
%%
[data.input.fileName, data.input.path] = uigetfile('*.avi');
data.video = VideoReader([data.input.path,data.input.fileName]);
% data.video.NumberOfFrames

for i = 1: data.video.NumberOfFrames
   data.frames{i} = read(data.video, i); 
%    imshow(data.frames{i});
end
%%
save('VideoFrames_fq11.mat','data');