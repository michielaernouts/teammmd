# SHOOB AI
A developing virtual assistant for your home to run on a Raspberry PI. 

## Setup
* clone this git in your preferred location ```git clone --recursive https://github.com/WouterJansen/Shoob_AI.git```
* Install AdaFruit python library by running ```sudo apt-get install build-essential python-dev git scons swig```
* Go to ```rpi_ws281x``` and run ```sudo scons``` 
* Go to ```rpi_ws281x/python```and run ```sudo python setup.py install```
* Install PulseAudio ```sudo apt-get install pulseaudio```
* Go to ```python-pulseaudio``` and run ```sudo python setup.py install```
* Make sure PulseAudio starts on launch as follows ```sudo pulseaudio --daemonize=no --system --realtime``` (for example with ``` sudo crontab -e```)
* Set your audio device correctly in ```NeoPixel.py``` on line 9. In this example we use an audio USB device but yours can differ. To find out what the name is of yours you can run ```pactl list short sinks```
* Run ```ShoobAI.py``` as ```sudo python ShoobAI.py``` to test if all works. It will print out any problems found.

## Requirements
* Raspberry PI running Raspbian
* a 12 LED Neopixel ring connected as in the picture below:
  ![raspberry-pi-neopixel]
  
  
## Notes
* If not using onboard audio it's recommended to disable the sound driver. Otherwise this can interfere with the NeoPixel PWM signal:
  * Alter  ```/lib/modprobe.d/aliases.conf``` to have ```options snd-usb-audio index=-2``` commented out.
  * create a ```blacklist-rgb-matrix.conf``` file in the same folder and in ```/etc/modprobe.d``` and add the following lines:
  ```  
  blacklist snd_bcm2835
  blacklist snd_pcm
  blacklist snd_timer
  blacklist snd_pcsp
  blacklist snd

  ```
  * Set default audio device by editing ```~/.asoundrc``` to have:
  ``` 
  pcm.!default {
        type hw
        card 0
  }

  ctl.!default {
        type hw
        card 0
  }
  ```


## References
* Using PulseAudio in Python: http://freshfoo.com/posts/pulseaudio_monitoring/
* Using NeoPixel on Raspberry Pi: https://learn.adafruit.com/neopixels-on-raspberry-pi/overview  
  
  
  
  
  
  
  
  
  [raspberry-pi-neopixel]: https://cdn.raspberrytips.nl/wp-content/uploads/2016/05/neopixel-raspberry-pi-led-ws281x-600x292.png "from  :https://raspberrytips.nl/neopixel-ws2811-raspberry-pi/"



