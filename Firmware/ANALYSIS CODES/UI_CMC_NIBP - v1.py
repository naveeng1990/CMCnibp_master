# {
#   "Default": {
# "*************************************************"
#      "author": "Naveen Gangadharan",
#      "lastModified": "Jan 25, 2021",
#      "email": "navpournami@gmail.com",
#      "desc": "CMC NIBP UI code for analysing data-ramp and step"
# "*************************************************"
#   }
# }

from numpy import math
import statistics 
from numpy import NaN, Inf, arange, isscalar, asarray, array
import scipy
import pandas as pd
import csv
from scipy.signal import savgol_filter
from matplotlib import pyplot as plt
import matplotlib.image as mpimg
from matplotlib import style
import numpy as np
from scipy.signal import butter, filtfilt
from math import pi, log
from scipy import fft, ifft
from scipy.optimize import curve_fit
from scipy.signal import butter, lfilter, freqz
from scipy.signal import find_peaks, peak_prominences
from mpldatacursor import datacursor
from scipy.ndimage import gaussian_filter1d
import sys
from tkinter import *
import os
import tkinter
import tkinter.messagebox
top=tkinter.Tk()


top.geometry('400x200')
top.title("CMC NIBP")
top.iconbitmap('D:/2020 compre time -Present/00000000 demo 2020 Jan/pulse.ico')
top.configure(background='#E5F2FE')

def _datacheck_peakdetect(x_axis, y_axis):
    if x_axis is None:
        x_axis = range(len(y_axis))
    
    if len(y_axis) != len(x_axis):
        raise (ValueError, 
                'Input vectors y_axis and x_axis must have same length')
    
    #needs to be a numpy array
    y_axis = np.array(y_axis)
    x_axis = np.array(x_axis)
    return x_axis, y_axis


def peakdetect(y_axis, x_axis = None, lookahead = 200, delta=0.0005): # added on 29th oct    # changed new for senthamiz
  
    max_peaks = []
    min_peaks = []
    dump = []  
       
    # check input data
    x_axis, y_axis = _datacheck_peakdetect(x_axis, y_axis)
    # store data length for later use
    length = len(y_axis)
    
    peaks = [0] *length
    
    #perform some checks
    if lookahead < 1:
        raise ValueError( "Lookahead must be '1' or above in value")
    if not (np.isscalar(delta) and delta >= 0):
        raise ValueError( "delta must be a positive number")

    mn, mx = np.Inf, -np.Inf
    
    #Only detect peak if there is 'lookahead' amount of points after it
    for index, (x, y) in enumerate(zip(x_axis[:-lookahead], 
                                        y_axis[:-lookahead])):
        if y > mx:
            mx = y
            mxpos = x
        if y < mn:
            mn = y
            mnpos = x
        ####look for max####
        if y < mx-delta and mx != np.Inf: 
            #Maxima peak candidate found
            #look ahead in signal to ensure that this is a peak and not jitter
            if y_axis[index:index+lookahead].max() < mx:
                peaks[mxpos] = 1
                max_peaks.append([mxpos, mx])
                dump.append(True)
                #set algorithm to only find minima now
                mx = np.Inf
                mn = np.Inf
                if index+lookahead >= length:
                    #end is within lookahead no more peaks can be found
                    break
                continue

        
        ####look for min####
        if y > mn+delta and mn != -np.Inf:

            if y_axis[index:index+lookahead].min() > mn:
                peaks[mnpos] = -1
                min_peaks.append([mnpos, mn])
                dump.append(False)
                #set algorithm to only find maxima now
                mn = -np.Inf
                mx = -np.Inf
                if index+lookahead >= length:
                
                    break
    
    peaks[0] = 0

    try:
        if dump[0]:
            max_peaks.pop(0)
        else:
            min_peaks.pop(0)
        del dump
    except IndexError:
       
        pass


    return peaks

def peak_detect(x):
  inx, _ = find_peaks(x, prominence=0.0025, distance=230)
  prominences = peak_prominences(x, inx)[0]
  peaks = [prominences[np.where(inx==i)] if i in inx else 0 for i in range(len(x))]
  return peaks


def ramp():



    nibpp = 'C:/CMC_NIBP/ramp.txt'
    time, ch1, ch2, ch3= np.loadtxt(nibpp, dtype=('f4, f4, f4, f4'),unpack=True,delimiter = '\t')
    ip = pd.DataFrame()
    ip['time'] = time
    ip['ref_ppg'] = ch2 + 2
    ip['occ_ppg'] = ch3 + 2
    ip['pressure'] = ch1   
    ip['ref_peaks'] = peak_detect(-ip['ref_ppg'])
    ip = ip[ip['pressure']>0]
    cycles = []
    peaks = (ip[ip['ref_peaks'] != 0].index).tolist()
    for i in range(1, len(peaks)):
        cycles.append(ip.loc[peaks[i - 1]:peaks[i]])
    amplitude = []
    auc = []
    cycle_pressure = []
    plateau_pressure = []
    ref_amplitude = []

    for i in range(len(cycles)):
        ref_amplitude.append(
            (cycles[i]['ref_ppg'].values[0]) + (cycles[i]['ref_ppg'].values[-1])/2)
        cycle_pressure.append(cycles[i]['pressure'].mean())
        amplitude.append(cycles[i]['occ_ppg'].max() - cycles[i]['occ_ppg'].min())

    op = pd.DataFrame()
    op['cycle_pressure'] = cycle_pressure
    op['amplitude'] = amplitude
    op['ref_amplitude'] = ref_amplitude
    op['ratio'] = np.divide(np.array(amplitude), np.array(ref_amplitude))
    op = op.sort_values('cycle_pressure')
    op.to_csv('C:/CMC_NIBP/'+'cmc_nibp_ramp_profile.csv', index=False)


    (a, b, c, d, e), _ = curve_fit(sigmoid, op['cycle_pressure'], op['ratio'], maxfev=5000)
    op['fit'] = sigmoid(op['cycle_pressure'], a, b, c, d, e)
    xfit = list(range(int(op['cycle_pressure'].min()), int(op['cycle_pressure'].max())))
    yfit = np.interp(xfit, op['cycle_pressure'], op['fit']) 
    slope = np.gradient(yfit)
    diff = np.gradient(np.gradient(gaussian_filter1d(yfit, 50)))
    infls = [xfit[np.where(diff==diff.min())[0][0]], xfit[np.where(diff==diff.max())[0][0]]]
    infls.extend(np.where(np.diff(np.sign(diff)))[0])

    infls = np.sort(infls)
    inx = np.where(xfit==infls[1])
    y_infl = ((np.array(xfit)-infls[1])*slope[inx][0])+yfit[inx]


    max = np.where(yfit==yfit.max())[0][0]
    y_high = ((np.array(xfit)-xfit[max])*slope[max])+yfit.max()

    min = np.where(yfit==yfit.min())[0][0]
    y_low = ((np.array(xfit)-xfit[min])*slope[min])+yfit.min()

    idx1 = np.argwhere(np.diff(np.sign(y_high - y_infl))).flatten()[0]

    idx2 = np.argwhere(np.diff(np.sign(y_infl - y_low))).flatten()[0]


    

    f = open("C:/CMCdaqV22_06Jan2021/config/currentselection/rec_PeripheralSendList02_cur.cfg", "w")

    apr_sys_pressure=int( xfit[idx2] )
    f.write("2" + "\n" + str(xfit[idx2]) )
    f.close()
    f= plt.figure()
    ax= f.add_subplot(111)
    w=plt.scatter(op['cycle_pressure'],op['ratio']);
    plt.plot(op['cycle_pressure'], op['fit'])
    plt.plot(y_infl)
    plt.plot(y_high)
    plt.plot(y_low)
    plt.plot(xfit[idx1], y_high[idx1], 'ro')
    plt.plot(xfit[idx2], y_low[idx2], 'ro')
    plt.xlabel('Pressures in mmHg');
    plt.ylabel('PPG');
    plt.title('CMC_NIBP Ramp Protocol')
    
    textstr='apprx sys:'+str(apr_sys_pressure)+'mmHg' 

    props = dict(boxstyle='round', facecolor='wheat', alpha=0.5)

    ax.text(0.02, 0.95, textstr, transform=ax.transAxes, fontsize=10,
            verticalalignment='top', bbox=props)

    plt.savefig('CMC_NIBP ramp Plot')
    datacursor(w)
    plt.show();


def step():

    # Reading input
    nibp = 'C:/CMC_NIBP/nibp.txt'
    # time, ch1, ch2, ch3, ch4, ch5, ch6, ch7= np.loadtxt(nibp, dtype=('f4, f4, f4, f4, f4, f4,f4, f4'),unpack=True,delimiter = '\t')
    time, ch1, ch2, ch3= np.loadtxt(nibp, dtype=('f4, f4, f4, f4'),unpack=True,delimiter = '\t')
    ip = pd.DataFrame()


    ip['time'] = time
    ip['ref_ppg'] = ch2 + 2
    ip['occ_ppg'] = ch3 + 2    
    ip['pressure'] = ch1

    # Finding peaks and valleys
    ip['ref_peaks'] = peakdetect(ip['ref_ppg'], lookahead=100, delta=0.005)
    ip['occ_peaks'] = peakdetect(ip['occ_ppg'], lookahead=100, delta=0.005)



    # yfit = np.interp(range(int(op['plateau_pressure'].min()), int(op['plateau_pressure'].max())), op['plateau_pressure'], op['fit']) 
    # f1 = np.array(yfit)
    # f2 = np.insert(f1, 0, np.zeros(3))[:-3]
    # slope = np.diff(f1)






def butter_lowpass_filter(data, cutoff, fs, order=5):
    nyq = 0.5 * fs
    normal_cutoff = cutoff / nyq
    b, a = butter(order, normal_cutoff, btype='low', analog=False)
    y = filtfilt(b, a, data)
    return y


def butter_highpass_filter(data, cutoff, fs, order=5):
    nyq = 0.5 * fs
    normal_cutoff = cutoff / nyq
    b, a = butter(order, normal_cutoff, btype='high', analog=False)
    y = filtfilt(b, a, data)
    return y





def sigmoid(x, a, b, c,d, e):
  return d + ((a-d)/(1+(x/c)**b)**e)





def helloCallBackStep():
  step()

def helloCallBackRamp():
  ramp()



B1=tkinter.Button(text="run_ramp",fg='red',command= helloCallBackRamp)
# B1.pack(side=tkinter.LEFT)
B1.place(x=140,y=100)

B2=tkinter.Button(text="run_step",fg='blue',command= helloCallBackStep)
# B2.pack(side=tkinter.RIGHT)
B2.place(x=220,y=100)
top.mainloop()