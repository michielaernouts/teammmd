#include <stdio.h>
#include <unistd.h>
#include "rpi_3_sensor_shield.h"

void main() {
	while(1)
	{
	   readmpl31152a();
	  // sleep(1);
	   readHumidity();
	}

}
