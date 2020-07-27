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

alpha=0.625
Psi=5.1387085e-6                #Is current (saturation)
b=math.log(Psi)

stoptime = 5
time=[]
p_1=[]
p_2=[]

time_cpp=[]
p_1_cpp=[]
p_2_cpp=[]

with open('PARAMS.CSV') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    for row in csv_reader:
    	t=float(row[0])
    	q1=float(row[1])
    	q2=float(row[2])
    	time.append(t)
    	p_1.append(q1)
    	p_2.append(q2)

markers_1 = [100,200,300,400,500,600,700,800,900,1000,1100,1200,1300,1400]
markers_2 = [50,150,250,350,450,550,650,750,850,950,1050,1150,1250,1350,1450]

fig3=plt.figure(3);                      #figure plots
ax=fig3.add_subplot(211)
ax.axis([0,stoptime,0.55,0.66])                 #set axis
ax.set_xlabel('tiempo [s]')
ax.set_ylabel(r'$\theta_1$')                    #Theta_1 estimated
ax.plot(time,p_1,lw=2)
ax.plot([0,stoptime],[alpha, alpha],'k--',lw=2);
ax.grid(True)

ax=fig3.add_subplot(212)
ax.axis([0,stoptime,-13.5,-11])
ax.set_xlabel('t [s]')
ax.set_ylabel(r'$\theta_2$')
ax.plot(time,p_2,lw=2)                   #Theta_2 estimated
ax.plot([0,stoptime],[b, b],'k--',lw=2);
ax.grid(True)
plt.savefig("theta1_theta2_estimadas_euler_python.png")

fig4=plt.figure(4);                     #Phaseplane Theta_1 vs Theta_2
ax=fig4.add_subplot(111)
ax.axis([0.45,0.8,-15,-9])
ax.set_xlabel(r'$\theta_1$')
ax.set_ylabel(r'$\theta_2$')
ax.plot(p_1,p_2,'.b',ms=8)
plt.grid(True)
plt.axis.hold=0
vv=17.4
yy=alpha*vv+b
print(yy)                           #print Ipv
print(b)                            #print ln(Is)
aa=np.zeros(2)
bb=np.zeros(2)                          #trace line on phaseplane
aa[0]=0
aa[1]=0.8   
bb[0]=yy
bb[1]=-aa[1]*vv+yy
ax.plot(aa,bb,'-k',lw=2)
ax.plot(alpha,b,'ro',ms=8);
plt.savefig("theta1_theta2_phaseplane_euler_python.png")
plt.show()
