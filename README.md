# teammmd
Ambient Intelligence Team MMD

WORKS:
------
Barometer (MPL3115A2)
Magnetometer (MAG3110)
Accelerometer (ADXL345) more precise accelerometer
DASH7 ALP transmit over UART
BNO055 Magnetometer, Gyroscope, Accelerometer with automatic calibration

WORKS BUT NEEDS IMPROVEMENTS:
-----------------------------
PIR sensor needs to tuned.
Accelerometer (MMA8491Q) works but shows also the data when data is not ready (value 22873 for X, Y and Z)

BRANCH final:
-------------
Merging libraries:
------------------
Barometer (MPL3115A2) and Humidity (SI7021) working (RPI3 shield)
problems with magnetometer (MAG3110), gives 0,0,0 ATM