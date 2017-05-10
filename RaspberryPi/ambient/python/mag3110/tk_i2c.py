#!/usr/bin/python

import smbus

# ============================================================
# Antonio Casini I2C Class - Thanks to Adafruit
# ============================================================

Bus_n = 1

# Address = hex address device - es. 0x0e for mag3110

class Tk_I2C :

   def __init__(self, address, bus=smbus.SMBus(Bus_n)):
     self.address = address
     self.bus = bus

   def write8(self, reg, value):
     "Writes an 8-bit value to an register/address"
     try:
       self.bus.write_byte_data(self.address, reg, value)
     except IOError, err:
       print "Error accessing 0x%02" % self.address
       return -1
   
   def readU8(self, reg):
     "Reads an unsigned byte from the I2C device"
     try:
       result = self.bus.read_byte_data(self.address, reg)
       return result
     except IOError, err:
       print "Error accessing 0x%02X" % self.address
       return -1

   def readS8(self, reg):
     "Reads a signed byte from the I2C device"
     try:
       result = self.bus.read_byte_data(self.address, reg)
       if (result > 127):
         return result - 256
       else:
         return result
     except IOError, err:
       print "Error accessing 0x%02X" % self.address
       return -1

   def readU16(self, reg):
     "Reads a unsigned 16-bit value from the I2C device"
     try:
       hibyte = self.bus.read_byte_data(self.address, reg)
       result = (hibyte << 8) + self.bus.read_byte_data(self.address, reg+1)
       return result
     except IOError, err:
       print "Error accessing 0x%02X" % self.address 
       return -1

   def readS16(self, reg):
     "Reads a signed 16-bit value from the I2C device"
     try:
       hibyte = self.bus.read_byte_data(self.address, reg)
       if (hibyte > 127):
         hibyte -= 256
       result = (hibyte << 8) + self.bus.read_byte_data(self.address, reg+1)
       return result
     except IOError, err:
       print "Error accessing 0x%02X" % self.address
       return -1

