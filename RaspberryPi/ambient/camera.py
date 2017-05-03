from picamera import PiCamera
import datetime as dt
from time import sleep

camera = PiCamera()
camera.annotate_text_size=24

#camera.rotation = 180 

##camera.start_preview()
##sleep(5)
while 1:
	camera.annotate_text = dt.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
	camera.capture('/etc/openhab2/html/picture.jpg')
#	camera.capture('/etc/openhab2/html/picture.png')	
	##camera.stop_preview()
	##camera.start_preview(fullscreen = False)
	sleep(3)
	camera.stop_preview()
