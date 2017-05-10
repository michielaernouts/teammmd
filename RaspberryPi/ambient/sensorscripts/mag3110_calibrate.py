#!/usr/bin/env python
import math
import time
import threading
import Queue
import termios, fcntl, sys, os
from ctypes import *
#cdll.LoadLibrary("./bcm2835.so")

sensor = CDLL("./sensor.so")

class mag3110:
		
	def __init__(self):
		self.x_off = -565
		self.y_off = 700
		self.z_off = 0
		
		if (0 == sensor.bcm2835_init()):
			print "bcm3835 driver init failed."
			return
		
	def writeRegister(self, register, value):
	    sensor.MAG3110_WRITE_REGISTER(register, value)

	def readRegister(self, register):
		return sensor.MAG3110_READ_REGISTER(register)
	
	def __str__(self):
		ret_str = ""
		(x, y, z) = self.getAxes()
		ret_str += "X: "+str(x) + "  "
		ret_str += "Y: "+str(y) + "  "
		ret_str += "Z: "+str(z)
		
		return ret_str

	def init(self):
		sensor.MAG3110_Init()
	def readRawData_x(self):
		return sensor.MAG3110_ReadRawData_x()

	def readRawData_y(self):
		return sensor.MAG3110_ReadRawData_y()

	def readRawData_z(self):
		return sensor.MAG3110_ReadRawData_z()

	def getAxes(self):
		x = self.readRawData_x()
		y = self.readRawData_y()
		z = self.readRawData_z()

		return (x, y, z)

	def readAsInt(self):
		x = self.readRawData_x() / 40
		y = self.readRawData_y() / 40 
		z = self.readRawData_z() / 40

		return (x, y, z)
		 		
	def getHeading(self):
		(x, y, z) = self.getAxes()
		
		heading = math.atan2((y-self.y_off), (x-self.x_off)) * 180/math.pi + 180
		
		return heading

	def calibrate(self, x, y, z):
		max_x = max(x)
		max_y = max(y)
		max_z = max(z)

		min_x = min(x)
		min_y = min(y)
		min_z = min(z)
		
		self.x_off = (max_x + min_x) / 2
		self.y_off = (max_y + min_y) / 2 
		self.z_off = 0 #(max_z + min_z) / 2


def calibrate_thread(x, y, z):
	while (q.get() != 0):
		x.append(mag.readRawData_x())
		y.append(mag.readRawData_y())
		z.append(mag.readRawData_z())

		time.sleep(0.2)	
		
mag = mag3110()
mag.init()	

x = []
y = []
z = []

q = Queue.Queue(0)
q.put(1)
print "\nCalibrate your mag3110 sensor, Now horizontally rotate your board for 360 degrees\n"
T = threading.Thread(target=calibrate_thread, args=(x, y, z))
T.start()
while 1:
	c = int(input("If you have done, press 0 to exit: "))
	if (c == 0):
		q.put(0)
		break

mag.calibrate(x, y ,z)

f_cal = file("mag_calibration.data", 'w')
print >> f_cal, mag.x_off, mag.y_off, mag.z_off
f_cal.close()

print "calibrated, now the new calibration data is:\n"
print mag.x_off, mag.y_off, mag.z_off


