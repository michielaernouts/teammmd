import sys
import threading
import time
from datetime import datetime
from neopixel import *
from PeakMonitor import PeakMonitor


SINK_NAME = 'alsa_output.usb-C-Media_Electronics_Inc._USB_PnP_Sound_Device-00-Device.analog-stereo'
METER_RATE = 344
strip = Adafruit_NeoPixel(12, 18, 800000, 5, False, 255)



def fadeIn(color):
    strip.setBrightness(0)
    strip.show()
    for i in range(strip.numPixels()):
        strip.setPixelColor(i, color)
    for j in range(255):
        strip.setBrightness(j)
        strip.show()
        time.sleep(2 / 1000.0)


def fadeOut(color):
    strip.setBrightness(255)
    strip.show()
    for i in range(strip.numPixels()):
        strip.setPixelColor(i, color)
    for j in range(255, -1, -1):
        strip.setBrightness(j)
        strip.show()
        time.sleep(2 / 1000.0)


def errorRed():
    for i in range(strip.numPixels()):
        strip.setPixelColor(i, Color(0, 255, 0))
    strip.show()


class SetupGreen(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.done = False

    def run(self):
        while not self.done:
            for q in range(3):
                for i in range(0, strip.numPixels(), 3):
                    strip.setPixelColor(i + q, Color(255, 0, 0))
                strip.show()
                time.sleep(50 / 1000.0)
                for i in range(0, strip.numPixels(), 3):
                    strip.setPixelColor(i + q, 0)
        turnOff()


def rotateBlue():
    for i in range(0, 11, 1):
        strip.setPixelColor(i, Color(0, 0, 255))
        if i + 1 > 11:
            strip.setPixelColor(i + 1 - 12, Color(0, 0, 160))
        else:
            strip.setPixelColor(i + 1, Color(0, 0, 160))
        if i + 2 > 11:
            strip.setPixelColor(i + 2 - 12, Color(0, 0, 100))
        else:
            strip.setPixelColor(i + 2, Color(0, 0, 100))
        if i + 3 > 11:
            strip.setPixelColor(i + 3 - 12, Color(0, 0, 50))
        else:
            strip.setPixelColor(i + 3, Color(0, 0, 50))
        if i + 4 > 11:
            strip.setPixelColor(i + 4 - 12, Color(0, 0, 100))
        else:
            strip.setPixelColor(i + 4, Color(0, 0, 100))
        if i + 5 > 11:
            strip.setPixelColor(i + 5 - 12, Color(0, 0, 160))
        else:
            strip.setPixelColor(i + 5, Color(0, 0, 160))
        if i + 6 > 11:
            strip.setPixelColor(i + 6 - 12, Color(0, 0, 255))
        else:
            strip.setPixelColor(i + 6, Color(0, 0, 255))
        if i + 7 > 11:
            strip.setPixelColor(i + 7 - 12, Color(0, 0, 160))
        else:
            strip.setPixelColor(i + 7, Color(0, 0, 160))
        if i + 8 > 11:
            strip.setPixelColor(i + 8 - 12, Color(0, 0, 100))
        else:
            strip.setPixelColor(i + 8, Color(0, 0, 100))
        if i + 9 > 11:
            strip.setPixelColor(i + 9 - 12, Color(0, 0, 50))
        else:
            strip.setPixelColor(i + 9, Color(0, 0, 50))
        if i + 10 > 11:
            strip.setPixelColor(i + 10 - 12, Color(0, 0, 100))
        else:
            strip.setPixelColor(i + 10, Color(0, 0, 100))
        if i + 11 > 11:
            strip.setPixelColor(i + 11 - 12, Color(0, 0, 160))
        else:
            strip.setPixelColor(i + 11, Color(0, 0, 160))
        strip.show()
        time.sleep(0.2)


def setupNeoPixel():
    print "STARTING NEOPIXEL"
    try:
        strip.begin()
        print "Neopixel started..."
        print '-' * 60
    except:
        print "Neopixel could not be started..."
        print '-' * 60



def turnOff():
    for i in range(strip.numPixels()):
        strip.setPixelColor(i, Color(0, 0, 0))
        strip.setBrightness(255)
    strip.show()


def setupPulseAudio():
    try:
        monitor = PeakMonitor(SINK_NAME, METER_RATE)
        return monitor
    except:
        global setupDone
        setupDone = True
        errorRed()
        time.sleep(3)
        sys.exit()


class AudioReact(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.listening = True

    def run(self):
        fadeIn(Color(0, 0, 255))
        print datetime.now().strftime('%H:%M:%S') + " - Listening and NeoPixel Reacting"
        monitor = setupPulseAudio()
        fadeIn(Color(0, 0, 255))
        for sample in monitor:
            for i in range(strip.numPixels()):
                strip.setPixelColor(i, Color(0, 0, (sample - 128) * 2))
            strip.show()
            if not self.listening:
                fadeOut(Color(0, 0, 255))
                print datetime.now().strftime('%H:%M:%S') + " - Stopped Listening and NeoPixel Reacting"
                return



