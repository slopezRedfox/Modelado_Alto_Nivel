import csv
import math
import functools
import numpy as np
import scipy as sp
import matplotlib.pyplot as plt
import scipy.signal as signal
from scipy.integrate import odeint
from scipy.interpolate import interp1d
from matplotlib import rc

time=[]
current=[]
volt=[]

current_scaled=[]
volt_scaled=[]
LinearPV=[]
#------------------------------------------------
with open('SIGNALS.CSV') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count=0
    for row in csv_reader:
        t=float(row[0])
        c=float(row[1])
        v=float(row[2])
        time.append(t)
        current.append(c)
        volt.append(v)

print(f' Corriente max: {max(current)} Corriente min: {min(current)}')
print(f' Volt max: {max(volt)} Volt min: {min(volt)}')
print(f' Vp-p: {max(volt)-min(volt)}')
print(f' Ip-p: {max(current)-min(current)}')

#------------------------------------------------------------

fig1=plt.figure(1)
plt.plot(time[1:len(time)], current[1:len(time)], 'b')
#plt.plot(time[1:2482], current[1:2482], 'b')
plt.axis([0,0.006,0,5])
plt.xlabel('time (s)')
plt.ylabel('Current (A)')
plt.title('ipv scaled x 5')
plt.grid(True)
plt.savefig("ipv_scaled_x_5.png")

fig2=plt.figure(2)
plt.plot(time[1:len(time)], volt[1:len(time)], 'b')
#plt.plot(time[1:2482], volt[1:2482],'b')
plt.axis([0,0.006,11,23])
plt.xlabel('time (s)')
plt.ylabel('Volt (V)')
plt.title('Vpv scaled x 22')
plt.grid(True)
plt.savefig("vpv_scaled_x_22.png")


plt.show()