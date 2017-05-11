#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "rpi_3_sensor_shield.h"

void readHumidity(void)
{
	// Create I2C bus
	int file;
	char *bus = "/dev/i2c-1";
	if((file = open(bus, O_RDWR)) < 0) 
	{
		printf("Failed to open the bus. \n");
		exit(1);
	}
	// Get I2C device, SI7021 I2C address is 0x40(64)
	ioctl(file, I2C_SLAVE, SI7021_ADDRESS);

	// Send humidity measurement command(0xF5)
	char config[1] = {0xF5};
	write(file, config, 1);
	sleep(1);

	// Read 2 bytes of humidity data
	// humidity msb, humidity lsb
	char data[2] = {0};
	if(read(file, data, 2) != 2)
	{
		printf("Error : Input/output Error \n");
	}
	else
	{
                short a = 0;
                a = (data[0]<<8) | (data[1]);

//		printf("raw: %s, %c \n",data[0], data[0]);
		// Convert the data
//		float humidity = (((data[0] * 256 + data[1]) * 125.0) / 65536.0) - 6;
		float humidity = ((a * 125.0) / 65536.0) - 6;

                
		// Output data to screen
		printf(">>>SI7021:<<<\n");
		printf("Relative Humidity : %.2f RH \n", humidity);
		printf(">>>SI7021<<<\n\n");
	}

	// Send temperature measurement command(0xF3)
	config[0] = 0xF3;
	write(file, config, 1); 
	sleep(1);

	// Read 2 bytes of temperature data
	// temp msb, temp lsb
	if(read(file, data, 2) != 2)
	{
		printf("Error : Input/output Error \n");
	}
	else
	{
		// Convert the data
		float cTemp = (((data[0] * 256 + data[1]) * 175.72) / 65536.0) - 46.85;
		float fTemp = cTemp * 1.8 + 32;

		// Output data to screen		
		//printf("Temperature in Celsius : %.2f C \n", cTemp);
		//printf("Temperature in Fahrenheit : %.2f F \n", fTemp);
		
	}
}


void readmpl31152a() 
{
	// Create I2C bus
	int file;
	char *bus = "/dev/i2c-1";
	if((file = open(bus, O_RDWR)) < 0) 
	{
		printf("Failed to open the bus. \n");
		exit(1);
	}
	// Get I2C device, TSL2561 I2C address is 0x60(96)
	ioctl(file, I2C_SLAVE, MPL3115A2_ADDRESS);

	// Select control register(0x26)
	// Active mode, OSR = 128, altimeter mode(0xB9)
	char config[2] = {0};
	config[0] = MPL3115A2_CTRL_REG1;
	config[1] = 0xB9;
	write(file, config, 2);
	// Select data configuration register(0x13)
	// Data ready event enabled for altitude, pressure, temperature(0x07)
	config[0] = MPL3115A2_PT_DATA_CFG;
	config[1] = 0x07;
	write(file, config, 2);
	// Select control register(0x26)
	// Active mode, OSR = 128, altimeter mode(0xB9)
	config[0] = MPL3115A2_CTRL_REG1;
	config[1] = 0xB9;
	write(file, config, 2);
	sleep(1);

	// Read 6 bytes of data from address 0x00(00)
	// status, tHeight msb1, tHeight msb, tHeight lsb, temp msb, temp lsb
	char reg[1] = {0x00};
	write(file, reg, 1);
	char data[6] = {0};
	if(read(file, data, 6) != 6)
	{
		printf("Error : Input/Output error \n");
		exit(1);
	}

	// Convert the data
	int tHeight = ((data[1] * 65536) + (data[2] * 256 + (data[3] & 0xF0)) / 16);
	int temp = ((data[4] * 256) + (data[5] & 0xF0)) / 16;
	float altitude = tHeight / 16.0;
	float cTemp = (temp / 16.0);
	float fTemp = cTemp * 1.8 + 32;

	// Select control register(0x26)
	// Active mode, OSR = 128, barometer mode(0x39)
	config[0] = MPL3115A2_CTRL_REG1;
	config[1] = 0x39;
	write(file, config, 2);
	sleep(1);

	// Read 4 bytes of data from register(0x00)
	// status, pres msb1, pres msb, pres lsb
	reg[0] = MPL3115A2_STATUS;
	write(file, reg, 1);
	read(file, data, 4);

	// Convert the data to 20-bits
	int pres = ((data[1] * 65536) + (data[2] * 256 + (data[3] & 0xF0))) / 16;
	float pressure = (pres / 4.0) / 1000.0;
	
	// Output data to screen
	printf(">>>MPL3115A2:<<<\n");
	printf("Pressure : %.2f kPa \n", pressure);
	printf("Altitude : %.2f m \n", altitude);
	printf("Temperature in Celsius : %.2f C \n", cTemp);
	printf("Temperature in Fahrenheit : %.2f F \n", fTemp);
	printf(">>>MPL3115A2<<<\n\n");
}