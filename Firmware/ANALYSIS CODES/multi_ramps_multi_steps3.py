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


# Commented out IPython magic to ensure Python compatibility.
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
from scipy.signal import find_peaks, peak_prominences, peak_widths
from mpldatacursor import datacursor
from scipy.ndimage import gaussian_filter1d
from scipy import stats
# %pylab inline
import sys
from tkinter import *
import os
import tkinter
import tkinter.messagebox
from matplotlib.backends.backend_pdf import PdfPages

top=tkinter.Tk()

top.geometry('400x200')
top.title("CMC NIBP")
top.iconbitmap('D:/2020 compre time -Present/00000000 demo 2020 Jan/pulse.ico')
top.configure(background='#E5F2FE')
"""# Functions"""


def butter_highpass_filter(data, cutoff, fs, order=5):
  nyq = 0.5 * fs
  normal_cutoff = cutoff / nyq
  b, a = butter(order, normal_cutoff, btype='high', analog=False)
  y = filtfilt(b, a, data)
  return y

def butter_lowpass_filter(data, cutoff, fs, order=5):
    nyq = 0.5 * fs
    normal_cutoff = cutoff / nyq
    b, a = butter(order, normal_cutoff, btype='low', analog=False)
    y = filtfilt(b, a, data)
    return y

def peak_detect(x):
  inx, _ = find_peaks(x, prominence=0.000000001, distance=230)
  prominences = peak_prominences(x, inx)[0]
  peaks = [prominences[np.where(inx==i)] if i in inx else 0 for i in range(len(x))]
  return peaks

def peak_and_valley_detect(x, prominence=0.0025, distance=230):
  inx, _ = find_peaks(x, prominence=prominence, distance=distance)
  prominences = peak_prominences(x, inx)[0]
  peaks = [prominences[np.where(inx==i)][0] if i in inx else 0 for i in range(len(x))]
  inx, _ = find_peaks(-x, prominence=prominence, distance=distance)
  prominences = peak_prominences(-x, inx)[0]
  valleys = [-prominences[np.where(inx==i)][0] if i in inx else 0 for i in range(len(x))]
  return np.array(peaks)+np.array(valleys)

def sigmoid_5pl(x, a, b, c,d, e):
  return d + ((a-d)/(1+(x/c)**b)**e)

def sigmoid_4pl(x, a, b, c,d):
  return d + ((a-d)/1+(x/c)**b)

# removing outliers using z score
# https://stackoverflow.com/questions/23199796/detect-and-exclude-outliers-in-pandas-data-frame


def ramp1():
  """# Ramp
  Peak detection for both ppg, remove outliers, sigmoid fit (median for 0 pressure)
  """

  nibp = 'C:/CMC_NIBP/ramp.txt'
  # time, ch1, ch2, ch3, ch4, ch5, ch6, ch7= np.loadtxt(nibp, dtype=('f4, f4, f4, f4, f4, f4,f4, f4'),unpack=True,delimiter = '\t')
  time, ch1, ch2, ch3= np.loadtxt(nibp, dtype=('f4, f4, f4, f4'),unpack=True,delimiter = '\t')
  ip = pd.DataFrame()
  ip['time'] = time
  ip['ref_ppg'] = ch2 + 2
  ip['occ_ppg'] = ch3 + 2
  ip['pressure'] = ch1
  # Finding peaks and valleys


  # Finding peaks and valleys
  ip['ref_peaks'] = peak_and_valley_detect(ip['ref_ppg'])
  ip['occ_peaks'] = peak_and_valley_detect(ip['occ_ppg'], prominence=0.001)

  ip = ip[ip['pressure']>0]


  # Making a list of ref ppg cycles
  cycles = []
  valleys = (ip[ip['ref_peaks']==-1].index).tolist()
  for i in range(1, len(valleys)):
    cycles.append(ip.loc[valleys[i-1]:valleys[i]])
  # to check if there are cycles with no peaks in between in ref arm, such cycles are ignored.
  cycles = [cycle for cycle in cycles if len(cycle[(cycle['ref_peaks']!=0)&(cycle['ref_peaks']!=-1)])!=0]
  begin = ip.loc[:valleys[0]]

  # Generating amplitude, auc, mean cycle pressure and mean plateau pressure for each cycle
  amplitude = []
  cycle_pressure = []
  plateau_pressure = []
  ref_amplitude=[]


  for i in range(len(cycles)):
    # if (cycles[i]['mean_pressure'].values[0]==cycles[i]['mean_pressure'].values).all():
      ref_amplitude.append(cycles[i][(cycles[i]['ref_peaks']!=0)&(cycles[i]['ref_peaks']!=-1)]['ref_peaks'].values[0])
      # plateau_pressure.append(cycles[i]['mean_pressure'].values[0])
      cycle_pressure.append(cycles[i]['pressure'].mean())
      if len(cycles[i][(cycles[i]['occ_peaks']!=0)&(cycles[i]['occ_peaks']!=-1)])!=0:
        amplitude.append(cycles[i][(cycles[i]['occ_peaks']!=0)&(cycles[i]['occ_peaks']!=-1)]['occ_peaks'].values[0])
      else:
        amplitude.append(0)

  # Putting the output variables in another dataframe
  op = pd.DataFrame()
  # op['plateau_pressure'] = plateau_pressure
  op['amplitude'] = amplitude
  op['cycle_pressure'] = cycle_pressure
  op['ref_amplitude'] = ref_amplitude
  op['ratio'] = np.divide(np.array(amplitude), np.array(ref_amplitude))

  # Removing outliers (helps fitting)
  op = op[(np.abs(stats.zscore(op)) < 3).all(axis=1)]

  

  op = op.sort_values('cycle_pressure')
  try:
    (a, b, c, d, e), _ = curve_fit(sigmoid_5pl, op['cycle_pressure'], op['ratio'], maxfev=5000, absolute_sigma=False)
  except:
    op['temp'] = [int(x) for x in op['cycle_pressure']]
    row = pd.DataFrame(op[op['temp']==0].median()).transpose()
    op = op[op['temp']>0]
    op = pd.concat([row, op])
    try:
      (a, b, c, d, e), _ = curve_fit(sigmoid_5pl, op['cycle_pressure'], op['ratio'], maxfev=5000)


    except:
      op.to_csv('C:/CMC_NIBP/'+'cmc_ramp1_profile.csv', index=False)
      w = plt.scatter(op['cycle_pressure'], op['ratio'])
      datacursor(w)
      textstr = 'cannot fit'
      # plt.subplots_adjust(bottom=0.3)
      plt.gcf().text(0.8, 0.9, textstr, fontsize=10)

      axes = plt.gca()
      num = (op['ratio'].max()-op['ratio'].min())/5
      axes.set_ylim([op['ratio'].min()-num, op['ratio'].max()+num ])
      axes.set_xlim([0, 200])

      plt.xlabel('Pressures in mmHg');
      plt.ylabel('PPG ratios');
      plt.title('CMC_NIBP Ramp Protocol')
      plt.savefig('C:/CMC_NIBP/'+'CMC_NIBP Ramp 1 Plot')
      plt.tight_layout(h_pad=1)
      plt.show()
      return


  op['fit'] = sigmoid_5pl(op['cycle_pressure'], a, b, c, d, e)
  op.to_csv('C:/CMC_NIBP/'+'cmc_ramp1_profile.csv', index=False)

  fit = pd.DataFrame()
  fit['x'] = list(range(int(op['cycle_pressure'].min()), int(op['cycle_pressure'].max())))
  fit['y'] = np.interp(fit['x'], op['cycle_pressure'], op['fit']) 
  order = (fit['x'].max() - fit['x'].min())/10
  fit['d1'] = np.gradient(gaussian_filter1d(fit['y'], order))
  fit['d2'] = np.gradient(fit['d1'])

  plt.figure(figsize=(7.5, 7.5))
  # Max and min of 2nd derivative
  dias = fit[fit['d2']==fit['d2'].min()]['x'].values[0]
  sys = fit[fit['d2']==fit['d2'].max()]['x'].values[0]
  infls = [sys] #,dias]

  plt.subplot(2,1,1)
  w = plt.scatter(op['cycle_pressure'], op['ratio'])
  datacursor(w)
  plt.plot(fit['x'], fit['y'])
  for i, inf in enumerate(infls, 1):
    plt.axvline(x=inf, color='k', label=f'Inflection Point {i}')

  textstr = 'approx. systolic = '+str(sys)+'mmHg'
  # +'\n\napprox. diastolic = '+str(dias)+'mmHg'
  plt.gcf().text(0.6, 0.9, textstr, fontsize=10)

  axes = plt.gca()
  num = (op['ratio'].max()-op['ratio'].min())/5
  axes.set_ylim([op['ratio'].min()-num, op['ratio'].max()+num ])
  axes.set_xlim([0, 200])

  plt.xlabel('Pressures in mmHg');
  plt.ylabel('PPG');
  plt.title('CMC_NIBP Ramp Protocol')

  f = open("C:/CMCdaqV22_06Jan2021/config/currentselection/rec_PeripheralSendList02_cur.cfg", "w")
  f.write("2" + "\n" + str(sys))
  f.close()

  # Intersection of lines through max, min, infl of fit
  infl = [x for x in fit[np.sign(fit['d2']).diff()!=0]['x'].values if (x>dias)&(x<sys)][0]

  x1 = fit['x'].min()
  y1 = fit[fit['x']==fit['x'].min()]['y'].values[0]
  m = fit[fit['x']==fit['x'].min()]['d1'].values[0]
  fit['line_high'] = (fit['x'] - x1)*m + y1

  x1 = infl
  y1 = fit[fit['x']==infl]['y'].values[0]
  m = fit[fit['x']==infl]['d1'].values[0]
  fit['line_infl'] = (fit['x'] - x1)*m + y1

  x1 = fit['x'].max()
  y1 = fit[fit['x']==fit['x'].max()]['y'].values[0]
  m = fit[fit['x']==fit['x'].max()]['d1'].values[0]
  fit['line_low'] = (fit['x'] - x1)*m + y1

  plt.subplot(2,1,2)
  w = plt.scatter(op['cycle_pressure'], op['ratio'])
  datacursor(w)
  plt.plot(fit['x'], fit['y'])
  plt.plot(fit['x'], fit['line_high'], '--', linewidth=1.2)
  plt.plot(fit['x'], fit['line_infl'], '--', linewidth=1.2)
  plt.plot(fit['x'], fit['line_low'], '--', linewidth=1.2)

  plt.plot(infl, fit[fit['x']==infl]['y'].values[0], 'ro')

  idx1 = np.argwhere(np.diff(np.sign(fit['line_high'] - fit['line_infl']))).flatten()[0]
  plt.plot(fit['x'].values[idx1], fit['line_high'].values[idx1], 'ro')

  idx2 = np.argwhere(np.diff(np.sign(fit['line_infl'] - fit['line_low']))).flatten()[0]
  plt.plot(fit['x'].values[idx2], fit['line_low'].values[idx2], 'ro')

  textstr = 'approx. systolic = '+str(fit['x'].values[idx2])+'mmHg'
  # +'\napprox. diastolic = '+str(fit['x'].values[idx1])+'mmHg'
  plt.subplots_adjust(bottom=0.3)
  plt.gcf().text(0.6, 0.4, textstr, fontsize=10)

  axes = plt.gca()
  num = (op['ratio'].max()-op['ratio'].min())/5
  axes.set_ylim([op['ratio'].min()-num, op['ratio'].max()+num ])
  axes.set_xlim([0, 200])

  plt.xlabel('Pressures in mmHg');
  plt.ylabel('PPG');
  plt.title('CMC_NIBP Ramp Protocol')
  plt.savefig('C:/CMC_NIBP/'+'CMC_NIBP Ramp 1 Plot')
  plt.tight_layout(h_pad=1)
  plt.show()


def ramp2():
  """# Ramp

  Std of occ_ppg amplitudes, approx - pressure at 1.5 of min std
  """

  nibp = 'C:/CMC_NIBP/ramp.txt'
  # it doesnt work for 1.5 , work only for ttr, unless you change to 5 or 10 factor for scaling
  # time, ch1, ch2, ch3, ch4, ch5, ch6, ch7= np.loadtxt(nibp, dtype=('f4, f4, f4, f4, f4, f4,f4, f4'),unpack=True,delimiter = '\t')
  time, ch1, ch2, ch3= np.loadtxt(nibp, dtype=('f4, f4, f4, f4'),unpack=True,delimiter = '\t')
  ip = pd.DataFrame()
  ip['time'] = time
  ip['ref_ppg'] = ch2 + 2
  ip['occ_ppg'] = ch3 + 2
  ip['pressure'] = ch1
  # Finding peaks and valleys


  # Finding peaks and valleys
  ip['ref_peaks'] = peak_and_valley_detect(ip['ref_ppg'])
  ip['occ_peaks'] = peak_and_valley_detect(ip['occ_ppg'], prominence=0.001)

  ip = ip[(ip['pressure']>0) & (ip['pressure']<190)]

  ip = ip.sort_values('pressure')
  ip['occ_ppg'] = ip['occ_ppg'] - ip['occ_ppg'].mean()
  ip['std'] = ip['occ_ppg'].rolling(1000).std()
  ip = ip[ip['std']>0]
  ip.to_csv('C:/CMC_NIBP/'+'cmc_ramp2_profile.csv', index=False)

  sys = ip[ip['std']<ip['std'].min()*10]['pressure'].min()

  f = open("C:/CMCdaqV22_06Jan2021/config/currentselection/rec_PeripheralSendList02_cur.cfg", "w")
  f.write("2" + "\n" + str(int(round(sys))))
  f.close()

  # plt.figure(figsize=(5,5))
  w = plt. scatter(ip['pressure'], ip['std'])
  datacursor(w)
  plt.plot(sys, ip[ip['pressure']==sys]['std'].values[0], 'ro')
  textstr = 'approx. systolic = '+str(int(round(sys)))+'mmHg'
  plt.subplots_adjust(bottom=0.2)
  plt.gcf().text(0.5, 0.8, textstr, fontsize=15)
  axes = plt.gca()
  axes.set_ylim([ip['std'].min() - 0.0001, ip['std'].max() + 0.0001])
  axes.set_xlim([0, 200])
  plt.xlabel('Pressures in mmHg');
  plt.ylabel('PPG');
  plt.title('CMC_NIBP Ramp Protocol')
  plt.savefig('C:/CMC_NIBP/'+'CMC_NIBP Ramp 2 Plot')
  plt.show()

def ramp3():
  """# Ramp

  max- min of occ ppg
  """

  nibp = 'C:/CMC_NIBP/ramp.txt'
  # time, ch1, ch2, ch3, ch4, ch5, ch6, ch7= np.loadtxt(nibp, dtype=('f4, f4, f4, f4, f4, f4,f4, f4'),unpack=True,delimiter = '\t')
  time, ch1, ch2, ch3= np.loadtxt(nibp, dtype=('f4, f4, f4, f4'),unpack=True,delimiter = '\t')
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

  # Generating amplitude, auc, mean cycle pressure and mean plateau pressure for each cycle
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

  # Putting the output variables in another dataframe
  op = pd.DataFrame()
  op['cycle_pressure'] = cycle_pressure
  op['amplitude'] = amplitude
  op['ref_amplitude'] = ref_amplitude
  op['ratio'] = np.divide(np.array(amplitude), np.array(ref_amplitude))

  # Removing outliers (helps fitting)
  op = op[(np.abs(stats.zscore(op)) < 3).all(axis=1)]

  op = op.sort_values('cycle_pressure')
  try:
    (a, b, c, d, e), _ = curve_fit(sigmoid_5pl, op['cycle_pressure'], op['ratio'], maxfev=5000, absolute_sigma=False)
  except:
    op['temp'] = [int(x) for x in op['cycle_pressure']]
    row = pd.DataFrame(op[op['temp']==0].median()).transpose()
    op = op[op['temp']>0]
    op = pd.concat([row, op])
    try:
      (a, b, c, d, e), _ = curve_fit(sigmoid_5pl, op['cycle_pressure'], op['ratio'], maxfev=5000)
    except:
      op.to_csv('C:/CMC_NIBP/'+'cmc_ramp3_profile.csv', index=False)
      w = plt.scatter(op['cycle_pressure'], op['ratio'])
      datacursor(w)
      textstr = 'cannot fit'
      # plt.subplots_adjust(bottom=0.3)
      plt.gcf().text(0.8, 0.9, textstr, fontsize=10)

      axes = plt.gca()
      num = (op['ratio'].max()-op['ratio'].min())/5
      axes.set_ylim([op['ratio'].min()-num, op['ratio'].max()+num ])
      axes.set_xlim([0, 200])

      plt.xlabel('Pressures in mmHg');
      plt.ylabel('PPG ratios');
      plt.title('CMC_NIBP Ramp Protocol')
      plt.savefig('C:/CMC_NIBP/'+'CMC_NIBP Ramp 3 Plot')
      plt.tight_layout(h_pad=1)
      plt.show()
      return
  op['fit'] = sigmoid_5pl(op['cycle_pressure'], a, b, c, d, e)
  op.to_csv('C:/CMC_NIBP/'+'cmc_ramp3_profile.csv', index=False)

  fit = pd.DataFrame()
  fit['x'] = list(range(int(op['cycle_pressure'].min()), int(op['cycle_pressure'].max())))
  fit['y'] = np.interp(fit['x'], op['cycle_pressure'], op['fit']) 
  order = (fit['x'].max() - fit['x'].min())/10
  fit['d1'] = np.gradient(gaussian_filter1d(fit['y'], order))
  fit['d2'] = np.gradient(fit['d1'])

  plt.figure(figsize=(7.5, 7.5))
  # Max and min of 2nd derivative
  dias = fit[fit['d2']==fit['d2'].min()]['x'].values[0]
  sys = fit[fit['d2']==fit['d2'].max()]['x'].values[0]
  infls = [sys] #,dias]

  f = open("C:/CMCdaqV22_06Jan2021/config/currentselection/rec_PeripheralSendList02_cur.cfg", "w")
  f.write("2" + "\n" + str(sys))
  f.close()

  plt.subplot(2,1,1)
  w = plt.scatter(op['cycle_pressure'], op['ratio'])
  datacursor(w)
  plt.plot(fit['x'], fit['y'])
  for i, inf in enumerate(infls, 1):
    plt.axvline(x=inf, color='k', label=f'Inflection Point {i}')

  textstr = 'approx. systolic = '+str(sys)+'mmHg'
  # +'\napprox. diastolic = '+str(dias)+'mmHg'
  plt.gcf().text(0.6, 0.9, textstr, fontsize=10)

  axes = plt.gca()
  num = (op['ratio'].max()-op['ratio'].min())/5
  axes.set_ylim([op['ratio'].min()-num, op['ratio'].max()+num ])
  axes.set_xlim([0, 200])

  plt.xlabel('Pressures in mmHg');
  plt.ylabel('PPG');
  plt.title('CMC_NIBP Ramp Protocol')

  # Intersection of lines through max, min, infl of fit
  infl = [x for x in fit[np.sign(fit['d2']).diff()!=0]['x'].values if (x>dias)&(x<sys)][0]

  x1 = fit['x'].min()
  y1 = fit[fit['x']==fit['x'].min()]['y'].values[0]
  m = fit[fit['x']==fit['x'].min()]['d1'].values[0]
  fit['line_high'] = (fit['x'] - x1)*m + y1

  x1 = infl
  y1 = fit[fit['x']==infl]['y'].values[0]
  m = fit[fit['x']==infl]['d1'].values[0]
  fit['line_infl'] = (fit['x'] - x1)*m + y1

  x1 = fit['x'].max()
  y1 = fit[fit['x']==fit['x'].max()]['y'].values[0]
  m = fit[fit['x']==fit['x'].max()]['d1'].values[0]
  fit['line_low'] = (fit['x'] - x1)*m + y1

  plt.subplot(2,1,2)
  w = plt.scatter(op['cycle_pressure'], op['ratio'])
  datacursor(w)
  plt.plot(fit['x'], fit['y'])
  plt.plot(fit['x'], fit['line_high'], '--', linewidth=1.2)
  plt.plot(fit['x'], fit['line_infl'], '--', linewidth=1.2)
  plt.plot(fit['x'], fit['line_low'], '--', linewidth=1.2)

  plt.plot(infl, fit[fit['x']==infl]['y'].values[0], 'ro')

  idx1 = np.argwhere(np.diff(np.sign(fit['line_high'] - fit['line_infl']))).flatten()[0]
  plt.plot(fit['x'].values[idx1], fit['line_high'].values[idx1], 'ro')

  idx2 = np.argwhere(np.diff(np.sign(fit['line_infl'] - fit['line_low']))).flatten()[0]
  plt.plot(fit['x'].values[idx2], fit['line_low'].values[idx2], 'ro')

  textstr = 'approx. systolic = '+str(fit['x'].values[idx2])+'mmHg'
  # +'\napprox. diastolic = '+str(fit['x'].values[idx1])+'mmHg'
  plt.subplots_adjust(bottom=0.3)
  plt.gcf().text(0.6, 0.4, textstr, fontsize=10)

  axes = plt.gca()
  num = (op['ratio'].max()-op['ratio'].min())/5
  axes.set_ylim([op['ratio'].min()-num, op['ratio'].max()+num ])
  axes.set_xlim([0, 200])

  plt.xlabel('Pressures in mmHg');
  plt.ylabel('PPG');
  plt.title('CMC_NIBP Ramp Protocol')
  plt.savefig('C:/CMC_NIBP/'+'CMC_NIBP Ramp 3 Plot')
  plt.tight_layout(h_pad=1)
  plt.show()


def step1():
  """# Step
  using ref and occ peak_detect
  """
  pdf = PdfPages('debug.pdf')

  # Reading input
  nibp = 'C:/CMC_NIBP/nibp.txt'
  time, ch1, ch2, ch3 = np.loadtxt(nibp, dtype=('f4, f4, f4, f4'), unpack=True, delimiter='\t')
  ip_full = pd.DataFrame()
  ip_full['time'] = time
  ip_full['ref_ppg'] = ch2 + 2
  ip_full['occ_ppg'] = ch3 + 2
  ip_full['pressure'] = ch1

  # Finding peaks and valleys
  ip_full['ref_peaks'] = peak_and_valley_detect(ip_full['ref_ppg'])
  ip_full['occ_peaks'] = peak_and_valley_detect(ip_full['occ_ppg'], prominence=0.001)

  plateau_width = 3000
  # Making a list of plateaus
  hts = np.linspace(0.0001, 1, 100)
  peaks, _ = find_peaks(ip_full['pressure'], width=plateau_width)

  plateaus = []
  for peak in peaks:
  	for ht in hts:
  		width, height, left, right = peak_widths(ip_full['pressure'], np.array([peak]), rel_height=ht)
  		if width > plateau_width:
  			break

  			
  	if right[0]>left[0]+plateau_width:
  		plateaus.append(ip_full.loc[left[0]:left[0]+plateau_width])
  	else:
  		plateaus.append(ip_full.loc[left[0]:right[0]])

  # Finding mean pressure for each plateau
  mean_pressures = []
  for plateau in plateaus:
    plateau.drop(plateau.head(1000).index, inplace=True) 
    # plateau.drop(plateau.tail(500).index, inplace=True)
    mean_pressures.extend([plateau['pressure'].mean()] * len(plateau))

  # Separating out just the plateaus
  ip = pd.concat(plateaus)
  ip['mean_pressure'] = mean_pressures
  ip = ip[np.invert(ip.index.duplicated())]

  detected_plateaus = ip['mean_pressure'].reindex(ip_full.index, fill_value=0).values
  fig, ax = plt.subplots()
  ax.plot(ip_full['pressure'])
  ax.plot(detected_plateaus)
  pdf.savefig(fig)

  for i,plateau in ip.groupby('mean_pressure'):
    fig, ax = plt.subplots()
    f1 = fig.add_subplot(2,1,1)
    f1.set_title('ref ppg at '+str(int(i))+'mmHg')
    f1.plot(plateau['ref_ppg'])
    f1.plot(0.5*plateau['ref_peaks']+plateau['ref_ppg'].mean(), 'r')


    f2 = fig.add_subplot(2,1,2)
    f2.set_title('occ ppg at '+str(int(i))+'mmHg')
    f2.plot(plateau['occ_ppg'])
    f2.plot(0.5*plateau['occ_peaks']+plateau['occ_ppg'].mean(), 'r')
    fig.tight_layout()
    pdf.savefig(fig)

  pdf.close()
  # Making a list of ref ppg cycles
  cycles = []
  valleys = (ip[ip['ref_peaks'] < 0].index).tolist()
  for i in range(1, len(valleys)):
    cycles.append(ip.loc[valleys[i - 1]:valleys[i]])
  # to check if there are cycles with no peaks in between in ref arm, such cycles are ignored.
  cycles = [cycle for cycle in cycles if len(cycle[(cycle['ref_peaks'] > 0)]) != 0]
  begin = ip.loc[:valleys[0]]

  # Generating amplitude, auc, mean cycle pressure and mean plateau pressure for each cycle
  amplitude = []
  cycle_pressure = []
  plateau_pressure = []
  ref_amplitude = []

  for i in range(len(cycles)):
    if (cycles[i]['mean_pressure'].values[0] == cycles[i]['mean_pressure'].values).all():
      ref_amplitude.append(
        cycles[i][(cycles[i]['ref_peaks'] >0)]['ref_peaks'].values[0])
      plateau_pressure.append(cycles[i]['mean_pressure'].values[0])
      cycle_pressure.append(cycles[i]['pressure'].mean())
      if len(cycles[i][(cycles[i]['occ_peaks'] >0)]) != 0:
        amplitude.append(
          cycles[i][(cycles[i]['occ_peaks'] >0)]['occ_peaks'].values[0])
      else:
        amplitude.append(0)

  # Putting the output variables in another dataframe
  op = pd.DataFrame()
  op['plateau_pressure'] = plateau_pressure
  op['amplitude'] = amplitude
  op['cycle_pressure'] = cycle_pressure
  op['ref_amplitude'] = ref_amplitude
  op['ratio'] = np.divide(np.array(amplitude), np.array(ref_amplitude))

  # Removing outliers (helps fitting)
  op = op[(np.abs(stats.zscore(op)) < 3).all(axis=1)]

  # Removing first and last peaks from each plateau
  # groups = []
  # for i, group in op.groupby(['plateau_pressure']):
  #   group = group.drop(group.index[[0, 1,-1]])
  #   groups.append(group)
  # op = pd.concat(groups)

  op = op.sort_values('plateau_pressure')
  try:
    (a, b, c, d, e), _ = curve_fit(sigmoid_5pl, op['plateau_pressure'], op['ratio'], maxfev=5000)
    # except:
    #   op['temp'] = [int(x) for x in op['plateau_pressure']]
    #   row = pd.DataFrame(op[op['temp']==0].median()).transpose()
    #   op = op[op['temp']>0]
    #   op = pd.concat([row, op])
    #   (a, b, c, d, e), _ = curve_fit(sigmoid_5pl, op['plateau_pressure'], op['ratio'], maxfev=5000)
    op['fit'] = sigmoid_5pl(op['plateau_pressure'], a, b, c, d, e)
    op.to_csv('C:/CMC_NIBP/'+'cmc_step1_profile.csv', index=False)

    fit = pd.DataFrame()
    fit['x'] = list(range(int(op['plateau_pressure'].min()), int(op['plateau_pressure'].max())))
    fit['y'] = np.interp(fit['x'], op['plateau_pressure'], op['fit'])
    order = (fit['x'].max() - fit['x'].min()) / 10
    fit['d1'] = np.gradient(gaussian_filter1d(fit['y'], order))
    fit['d2'] = np.gradient(fit['d1'])
    fit.to_csv('C:/CMC_NIBP/'+'slopes.csv', index=False)

    temp = pd.DataFrame()
    temp['pressures'] = np.sort(list(set(op['plateau_pressure'])))
    temp['variances'] = [op[op['plateau_pressure'] == pressure]['ratio'].var() for pressure in temp['pressures']]

    plt.figure(figsize=(7.5, 7.5))
    # Max and min of 2nd derivative
    up_dias = fit[fit['d2'] == fit['d2'].min()]['x'].values[0]
    low_sys = fit[fit['d2'] == fit['d2'].max()]['x'].values[0]

    x = temp[temp['pressures'] < up_dias]
    low_dias = int(x[x['variances'] == x['variances'].min()]['pressures'].values[0])
    x = temp[temp['pressures'] > low_sys]
    up_sys = int(x[x['variances'] == x['variances'].min()]['pressures'].values[0])

    infls = [low_dias, up_dias, low_sys, up_sys]

    plt.subplot(2, 1, 1)
    w = plt.scatter(op['plateau_pressure'], op['ratio'])
    datacursor(w)
    plt.plot(fit['x'], fit['y'])
    for i, inf in enumerate(infls, 1):
      plt.axvline(x=inf, color='k', label=f'Inflection Point {i}')

    textstr = 'systolic range: ' + str(low_sys) + ' - ' + str(up_sys) + ' mmHg\ndiastolic range: ' + str(
      low_dias) + ' - ' + str(up_dias) + ' mmHg'
    plt.gcf().text(0.6, 0.9, textstr, fontsize=10)

    axes = plt.gca()
    num = (op['ratio'].max()-op['ratio'].min())/5
    axes.set_ylim([op['ratio'].min()-num, op['ratio'].max()+num ])
    axes.set_xlim([0, 200])

    plt.xlabel('Pressures in mmHg');
    plt.ylabel('PPG');
    plt.title('CMC_NIBP Step Protocol')

    # Intersection of lines through max, min, infl of fit
    infl = [x for x in fit[np.sign(fit['d2']).diff() != 0]['x'].values if (x > up_dias) & (x < low_sys)][0]

    x1 = fit['x'].min()
    y1 = fit[fit['x'] == fit['x'].min()]['y'].values[0]
    m = fit[fit['x'] == fit['x'].min()]['d1'].values[0]
    fit['line_high'] = (fit['x'] - x1) * m + y1

    x1 = infl
    y1 = fit[fit['x'] == infl]['y'].values[0]
    m = fit[fit['x'] == infl]['d1'].values[0]
    fit['line_infl'] = (fit['x'] - x1) * m + y1

    x1 = fit['x'].max()
    y1 = fit[fit['x'] == fit['x'].max()]['y'].values[0]
    m = fit[fit['x'] == fit['x'].max()]['d1'].values[0]
    fit['line_low'] = (fit['x'] - x1) * m + y1

    plt.subplot(2, 1, 2)
    w = plt.scatter(op['plateau_pressure'], op['ratio'])
    datacursor(w)
    plt.plot(fit['x'], fit['y'])
    plt.plot(fit['x'], fit['line_high'], '--', linewidth=1.2)
    plt.plot(fit['x'], fit['line_infl'], '--', linewidth=1.2)
    plt.plot(fit['x'], fit['line_low'], '--', linewidth=1.2)

    plt.plot(infl, fit[fit['x'] == infl]['y'].values[0], 'ro')

    idx1 = np.argwhere(np.diff(np.sign(fit['line_high'] - fit['line_infl']))).flatten()[0]
    up_dias = fit['x'].values[idx1]
    plt.plot(up_dias, fit['line_high'].values[idx1], 'ro')

    idx2 = np.argwhere(np.diff(np.sign(fit['line_infl'] - fit['line_low']))).flatten()[0]
    low_sys = fit['x'].values[idx2]
    plt.plot(low_sys, fit['line_low'].values[idx2], 'ro')

    x = temp[temp['pressures'] < up_dias]
    low_dias = int(x[x['variances'] == x['variances'].min()]['pressures'].values[0])
    x = temp[temp['pressures'] > low_sys]
    up_sys = int(x[x['variances'] == x['variances'].min()]['pressures'].values[0])

    plt.plot(low_dias, fit[fit['x'] > low_dias]['y'].max(), 'ro')
    plt.plot(up_sys, fit[fit['x'] < up_sys]['y'].min(), 'ro')

    textstr = 'systolic range: ' + str(low_sys) + ' - ' + str(up_sys) + ' mmHg\ndiastolic range: ' + str(
      low_dias) + ' - ' + str(up_dias) + ' mmHg'
    plt.subplots_adjust(bottom=0.3)
    plt.gcf().text(0.6, 0.4, textstr, fontsize=10)

    axes = plt.gca()
    num = (op['ratio'].max()-op['ratio'].min())/5
    axes.set_ylim([op['ratio'].min()-num, op['ratio'].max()+num ])
    axes.set_xlim([0, 200])

    plt.xlabel('Pressures in mmHg');
    plt.ylabel('PPG');
    plt.title('CMC_NIBP Step Protocol')
    plt.savefig('C:/CMC_NIBP/'+'CMC_NIBP Step 1 Plot')
    plt.tight_layout(h_pad=1)
    plt.show()
  except:
    op.to_csv('C:/CMC_NIBP/'+'cmc_step1_profile.csv', index=False)
    w = plt.scatter(op['plateau_pressure'], op['ratio'])
    datacursor(w)
    textstr = 'cannot fit'
    # plt.subplots_adjust(bottom=0.3)
    plt.gcf().text(0.8, 0.9, textstr, fontsize=10)

    axes = plt.gca()
    num = (op['ratio'].max()-op['ratio'].min())/5
    axes.set_ylim([op['ratio'].min()-num, op['ratio'].max()+num ])
    axes.set_xlim([0, 200])

    plt.xlabel('Pressures in mmHg');
    plt.ylabel('PPG');
    plt.title('CMC_NIBP Step Protocol')
    plt.savefig('C:/CMC_NIBP/'+'CMC_NIBP Step 1 Plot')
    plt.tight_layout(h_pad=1)
    plt.show()



def step2():
  """# Step
  between valleys, max-min

  """

  # Reading input
  nibp = 'C:/CMC_NIBP/nibp.txt'
  time, ch1, ch2, ch3= np.loadtxt(nibp, dtype=('f4, f4, f4, f4'),unpack=True,delimiter = '\t')
  ip = pd.DataFrame()
  ip['time'] = time
  ip['ref_ppg'] = ch2 + 2
  ip['occ_ppg'] = ch3 + 2
  ip['pressure'] = ch1


  # Finding peaks and valleys
  ip['ref_peaks'] = peak_detect(-ip['ref_ppg'])

  # Making a list of plateaus
  hts = np.linspace(0.0001,1,100)
  peaks, _ = find_peaks(ip['pressure'], width=3000)

  plateaus = []
  for peak in peaks:
    for ht in hts:
      width, height, left, right = peak_widths(ip['pressure'], np.array([peak]), rel_height=ht)
      if width>3000:
        break
    plateaus.append(ip.loc[left[0]:right[0]])



  mean_pressures = []
  for plateau in plateaus:
    plateau.drop(plateau.head(1000).index, inplace=True)   # removing first 2s
    plateau.drop(plateau.tail(500).index, inplace=True)
    mean_pressures.extend([plateau['pressure'].mean()]*len(plateau))
  # Separating out just the plateaus
  ip = pd.concat(plateaus)
  ip['plateau'] = mean_pressures
  ip = ip[np.invert(ip.index.duplicated())]

  # Making a list of ref ppg cycles
  cycles = []
  peaks = (ip[ip['ref_peaks']!=0].index).tolist()
  for i in range(1, len(peaks)):
    cycles.append(ip.loc[peaks[i-1]:peaks[i]])

  # Generating amplitude, auc, mean cycle pressure and mean plateau pressure for each cycle
  amplitude = []
  cycle_pressure = []
  plateau_pressure = []
  ref_amplitude=[]

  for i in range(len(cycles)):
    if (cycles[i]['plateau'].values[0]==cycles[i]['plateau'].values).all():
      ref_amplitude.append(
          (cycles[i]['ref_ppg'].values[0]) + (cycles[i]['ref_ppg'].values[-1])/2)
      cycle_pressure.append(cycles[i]['pressure'].mean())
      plateau_pressure.append(cycles[i]['plateau'].values[0])
      amplitude.append(cycles[i]['occ_ppg'].max() - cycles[i]['occ_ppg'].min())

  # Putting the output variables in another dataframe
  op = pd.DataFrame()
  op['plateau_pressure'] = plateau_pressure
  op['amplitude'] = amplitude
  op['ref_amplitude'] = ref_amplitude
  op['cycle_pressure'] = cycle_pressure
  op['ref_amplitude'] = ref_amplitude

  op['ratio'] = np.divide(np.array(amplitude), np.array(ref_amplitude))

  # Removing outliers (helps fitting)
  op = op[(np.abs(stats.zscore(op)) < 3).all(axis=1)]

  # Removing first and last peaks from each plateau
  # groups = []
  # for i, group in op.groupby(['plateau_pressure']):
  #   group = group.drop(group.index[[0, 1,-1]])
  #   groups.append(group)
  # op = pd.concat(groups)

  op = op.sort_values('plateau_pressure')
  try:

    (a, b, c, d, e), _ = curve_fit(sigmoid_5pl, op['plateau_pressure'], op['ratio'], maxfev=5000)
    # except:
    #   op['temp'] = [int(x) for x in op['plateau_pressure']]
    #   row = pd.DataFrame(op[op['temp']==0].median()).transpose()
    #   op = op[op['temp']>0]
    #   op = pd.concat([row, op])
    #   (a, b, c, d, e), _ = curve_fit(sigmoid_5pl, op['plateau_pressure'], op['ratio'], maxfev=5000)
    op['fit'] = sigmoid_5pl(op['plateau_pressure'], a, b, c, d, e)
    op.to_csv('C:/CMC_NIBP/'+'cmc_step2_profile.csv', index=False)

    fit = pd.DataFrame()
    fit['x'] = list(range(int(op['plateau_pressure'].min()), int(op['plateau_pressure'].max())))
    fit['y'] = np.interp(fit['x'], op['plateau_pressure'], op['fit']) 
    order = (fit['x'].max() - fit['x'].min())/10
    fit['d1'] = np.gradient(gaussian_filter1d(fit['y'], order))
    fit['d2'] = np.gradient(fit['d1'])
    fit.to_csv('C:/CMC_NIBP/'+'slopes.csv', index=False)

    temp = pd.DataFrame()
    temp['pressures'] = np.sort(list(set(op['plateau_pressure'])))
    temp['variances'] = [op[op['plateau_pressure']==pressure]['ratio'].var() for pressure in temp['pressures']]

    plt.figure(figsize=(7.5, 7.5))
    # Max and min of 2nd derivative
    up_dias = fit[fit['d2']==fit['d2'].min()]['x'].values[0]
    low_sys = fit[fit['d2']==fit['d2'].max()]['x'].values[0]

    x = temp[temp['pressures']<up_dias]
    low_dias = int(x[x['variances']==x['variances'].min()]['pressures'].values[0])
    x = temp[temp['pressures']>low_sys]
    up_sys = int(x[x['variances']==x['variances'].min()]['pressures'].values[0])

    infls = [low_dias, up_dias, low_sys, up_sys]

    plt.subplot(2,1,1)
    w = plt.scatter(op['plateau_pressure'], op['ratio'])
    datacursor(w)
    plt.plot(fit['x'], fit['y'])
    for i, inf in enumerate(infls, 1):
      plt.axvline(x=inf, color='k', label=f'Inflection Point {i}')

    textstr = 'systolic range: '+str(low_sys)+' - '+str(up_sys)+' mmHg\ndiastolic range: '+str(low_dias)+' - '+str(up_dias)+' mmHg'
    plt.gcf().text(0.6, 0.9, textstr, fontsize=10)

    axes = plt.gca()
    num = (op['ratio'].max()-op['ratio'].min())/5
    axes.set_ylim([op['ratio'].min()-num, op['ratio'].max()+num ])
    axes.set_xlim([0, 200])

    plt.xlabel('Pressures in mmHg');
    plt.ylabel('PPG');
    plt.title('CMC_NIBP Step Protocol')

    # Intersection of lines through max, min, infl of fit
    infl = [x for x in fit[np.sign(fit['d2']).diff()!=0]['x'].values if (x>up_dias)&(x<low_sys)][0]

    x1 = fit['x'].min()
    y1 = fit[fit['x']==fit['x'].min()]['y'].values[0]
    m = fit[fit['x']==fit['x'].min()]['d1'].values[0]
    fit['line_high'] = (fit['x'] - x1)*m + y1

    x1 = infl
    y1 = fit[fit['x']==infl]['y'].values[0]
    m = fit[fit['x']==infl]['d1'].values[0]
    fit['line_infl'] = (fit['x'] - x1)*m + y1

    x1 = fit['x'].max()
    y1 = fit[fit['x']==fit['x'].max()]['y'].values[0]
    m = fit[fit['x']==fit['x'].max()]['d1'].values[0]
    fit['line_low'] = (fit['x'] - x1)*m + y1

    plt.subplot(2,1,2)
    w = plt.scatter(op['plateau_pressure'], op['ratio'])
    datacursor(w)
    plt.plot(fit['x'], fit['y'])
    plt.plot(fit['x'], fit['line_high'], '--', linewidth=1.2)
    plt.plot(fit['x'], fit['line_infl'], '--', linewidth=1.2)
    plt.plot(fit['x'], fit['line_low'], '--', linewidth=1.2)

    plt.plot(infl, fit[fit['x']==infl]['y'].values[0], 'ro')

    idx1 = np.argwhere(np.diff(np.sign(fit['line_high'] - fit['line_infl']))).flatten()[0]
    up_dias = fit['x'].values[idx1]
    plt.plot(up_dias, fit['line_high'].values[idx1], 'ro')

    idx2 = np.argwhere(np.diff(np.sign(fit['line_infl'] - fit['line_low']))).flatten()[0]
    low_sys = fit['x'].values[idx2]
    plt.plot(low_sys, fit['line_low'].values[idx2], 'ro')

    x = temp[temp['pressures']<up_dias]
    low_dias = int(x[x['variances']==x['variances'].min()]['pressures'].values[0])
    x = temp[temp['pressures']>low_sys]
    up_sys = int(x[x['variances']==x['variances'].min()]['pressures'].values[0])

    plt.plot(low_dias, fit[fit['x']>low_dias]['y'].max(), 'ro')
    plt.plot(up_sys, fit[fit['x']<up_sys]['y'].min(), 'ro')

    textstr = 'systolic range: '+str(low_sys)+' - '+str(up_sys)+' mmHg\ndiastolic range: '+str(low_dias)+' - '+str(up_dias)+' mmHg'
    plt.subplots_adjust(bottom=0.3)
    plt.gcf().text(0.6, 0.4, textstr, fontsize=10)

    axes = plt.gca()
    num = (op['ratio'].max()-op['ratio'].min())/5
    axes.set_ylim([op['ratio'].min()-num, op['ratio'].max()+num ])
    axes.set_xlim([0, 200])

    plt.xlabel('Pressures in mmHg');
    plt.ylabel('PPG');
    plt.title('CMC_NIBP Step Protocol')
    plt.savefig('C:/CMC_NIBP/'+'CMC_NIBP Step 2 Plot')
    plt.tight_layout(h_pad=1)
    plt.show()
  except:
    op.to_csv('C:/CMC_NIBP/'+'cmc_step2_profile.csv', index=False)
    w = plt.scatter(op['plateau_pressure'], op['ratio'])
    datacursor(w)
    textstr = 'cannot fit'
    # plt.subplots_adjust(bottom=0.3)
    plt.gcf().text(0.8, 0.9, textstr, fontsize=10)

    axes = plt.gca()
    num = (op['ratio'].max()-op['ratio'].min())/5
    axes.set_ylim([op['ratio'].min()-num, op['ratio'].max()+num ])
    axes.set_xlim([0, 200])

    plt.xlabel('Pressures in mmHg');
    plt.ylabel('PPG');
    plt.title('CMC_NIBP Step Protocol')
    plt.savefig('C:/CMC_NIBP/'+'CMC_NIBP Step 2 Plot')
    plt.tight_layout(h_pad=1)
    plt.show()



def helloCallBackStep1():
  step1()

def helloCallBackStep2():
  step2()  

def helloCallBackRamp1():
  ramp1()

def helloCallBackRamp2():
  ramp2()

def helloCallBackRamp3():
  ramp3()    



B1=tkinter.Button(text="ramp1",fg='red',command= helloCallBackRamp1)
B1.place(x=80,y=100)

B2=tkinter.Button(text="ramp2",fg='blue',command= helloCallBackRamp2)
B2.place(x=120,y=100)

B3=tkinter.Button(text="ramp3",fg='red',command= helloCallBackRamp3)
B3.place(x=160,y=100)

B4=tkinter.Button(text="step1",fg='blue',command= helloCallBackStep1)
B4.place(x=80,y=80)

B5=tkinter.Button(text="step2",fg='red',command= helloCallBackStep2)
B5.place(x=140,y=80)

top.mainloop()