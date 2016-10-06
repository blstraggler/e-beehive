#!/usr/bin/env python
# -*- coding: iso-8859-15 -*-

# exec(open('e-hive.py').read()) ou 
# exec(compile(open('e-hive.py', "rb").read(),'e-hive.py', 'exec'))

# to eliminate events in a range of time
# keen.delete_events("Behive_MyDatas", timeframe={"start":"2016-02-26T10:26:03.197Z","end":"2016-02-29T10:12:03.197Z"})
# BEWARE of offset vs UT !!!!
import os, sys

def deglitch(vect):
	"function deglitch of a vector"
	ne = len(vect)
	medv = num.median(vect)
	pseudosig = num.percentile(vect,95.) -  num.percentile(vect,5.)
	iglitch = num.where(abs(vect - medv) > 3.* pseudosig)[0]
	ngl = len(iglitch)
	dvect = vect
	#dvect[iglitch] = (vect[iglitch[0]-1] + vect[iglitch[0]+1]) / 2.
	for i in range(ngl):
		  dvect[iglitch[i]] = num.median(vect[max(iglitch[0]-3,0):min(iglitch[0]+3,ne-1)])
	return [dvect,iglitch]
import keen
import sys
import csv
import matplotlib.pyplot as plt
import numpy as num
import jdcal
import dateutil
import astropy.time

plt.ion()

duration = input("Duration of analysis until now in days or hours (nd or nh) : ")
if duration[-1:] == 'd' or duration[-1:] == 'D':
	tf = "this_"+duration[:-1]+"_days"
if duration[-1:] == 'h' or duration[-1:] == 'H':
	tf = "this_"+duration[:-1]+"_hours"	
c = keen.extraction("Beehive_MyDatas", property_names="data", timeframe=tf) 
ds = keen.extraction("Beehive_MyDatas", property_names="published_at", timeframe=tf) 
ne = len(c)
tb = num.empty([ne,5],num.float) 
tt = num.empty([ne],num.float) 

for i in range(ne):
	for j in range(5):
		tb[i,j] = float(c[i]['data'].split(" ")[2*j+1])
	# parsing de la date pour convertir en julien
	dss = ds[i]['published_at']
	dt1 = dss.split('-')
	dt2 = dt1[-1].split('T')
	dt3 = dt2[-1].split(':')
	dt4 = dt3[-1].split('Z')
	sd = dt1[0]+'.'+dt1[1]+'.'+dt2[0] # year, month, day
	dt = dateutil.parser.parse(sd) 
	jdt = astropy.time.Time(dt).jd # conversion in julian
	jdt += float(dt3[0])/24.+float(dt3[1])/60./24.+float(dt4[0])/3600./24.
	# add hour, min, sec
	tt[i] = jdt
tt -=tt[0]	# substract jd[0]

dsi = ds[0]['published_at'][:-8]
dse = ds[ne-1]['published_at'][:-8]
plt.ion()
tempint = tb[0:ne,0]
z = deglitch(tempint)
dtempint = z[0]
pc = tb[0:ne,1] + .7*(- dtempint[0:ne] + 15.)/9. # weight corrected from temperature drift

gl = z[1]
plt.figure(1)
plt.suptitle('My Beehive : '+dsi+'   -->  '+dse)
plt.subplot(221)
#plt.plot(tt[0:ne],tb[0:ne,1],marker='o')
plt.plot(tt[0:ne],dtempint[0:ne],marker='v')
plt.plot(tt[gl],dtempint[gl],marker='s')
plt.title('Temp ')
plt.subplot(222)
plt.plot(tt[0:ne],tb[0:ne,1],marker='o')
plt.plot(tt[0:ne],pc,marker='v')
plt.title('Weight ')
plt.subplot(223)
#plt.plot(tt[0:ne],tb[0:ne,3],marker='o')
#plt.title('Hygro')
#plt.subplot(234)
plt.plot(tt[0:ne],tb[0:ne,2],marker='v')
plt.title('Luminosity')
plt.subplot(224)
plt.plot(tt[0:ne],tb[0:ne,3],marker='o')
plt.title('Noise ')
#plt.subplot(236)
#plt.plot(tt[0:ne],tb[0:ne,4],marker='o')
#plt.title('tension alim')
plt.show()

poids_median = num.median(pc)
str_date_poids = dsi+' --> '+ dse +'  weight : ' + '%.2f' % poids_median + ' kg \n'

f = open('e-hive.txt', 'a')
f.write( str_date_poids)
f.close()


fin = input("OK ? ")