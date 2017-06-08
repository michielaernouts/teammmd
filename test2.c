/*
install paho:
https://eclipse.org/paho/clients/c/ 

extra optie voor compilen
-lpaho-mqtt3c
*/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"

#include <unistd.h>
#include <bcm2835.h>
#include <unistd.h>
#include <python2.7/Python.h>
#include "rpi_3_sensor_shield.h"
#define PIN RPI_GPIO_P1_15

#define ADDRESS     "tcp://143.129.37.79:1883"
#define CLIENTID    "Example"
#define TOPIC_AIR       "sensors/airquality"
#define TOPIC_HUM       "sensors/humidity"
#define TOPIC_TEMP      "sensors/temperature"
#define TOPIC_PRES      "sensors/pressure"
#define TOPIC_ALT       "sensors/altitude"
#define QOS         1
#define TIMEOUT     10000L

uint16_t CO21;

void readAirQuality(uint16_t* CO211) {
	if(bcm2835_gpio_lev(PIN)==0){
		uint32_t data;
		uint16_t tVOC1;
		getData_CCS811(&data);
		parseResult_CCS811(&data,CO211,&tVOC1);
	}
}

int main(int argc, char* argv[])
{
	bcm2835_init();
	bcm2835_i2c_setClockDivider(25000); //10kHz
	configure_CCS811(1);
	enableInterrupts_CCS811();
	bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_INPT);
	sleep(1);
	char str1[80];
	char str2[80];
	char str3[80];
	char str4[80];
	char str5[80];
	//uint16_t CO21;
	double humidity;
	double oldhumidity;
	double oldairquality;
	double temperatureMPL;
	double oldtemperatureMPL;
	double pressure;
	double oldpressure;
	float altitude;
	float oldaltitude;
	
	
	
	Py_Initialize();
	PyObject* main_module = PyImport_AddModule("__main__");
	// Get the main module's dictionary
      	// and make a copy of it.
      	PyObject* main_dict = PyModule_GetDict(main_module);
      	PyObject* main_dict_copy = PyDict_Copy(main_dict);
	char pythonvar[80];

	while(1)
	{
        //sleep(1);
		//oldairquality = readAirQuality(&CO21);
		readAirQuality(&CO21);
//		PyRun_SimpleString(pythonvar);
//		sprintf(pythonvar,"CO21=%ld",CO21);

		/*if(oldairquality == CO21)
		{
			printf("Same value\n");
		}
		else
		{	*/


			if(CO21 > 0 && CO21 < 8192)
			{
				sprintf(str1, "%d", CO21);
				printf("CO2: %s\n",str1);

				
				MQTTClient client;
				MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
				MQTTClient_message pubmsg = MQTTClient_message_initializer;
				MQTTClient_deliveryToken token;
				int rc;

				MQTTClient_create(&client, ADDRESS, CLIENTID,MQTTCLIENT_PERSISTENCE_NONE, NULL);
				conn_opts.keepAliveInterval = 20;
				conn_opts.cleansession = 1;	
				if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
				{
					printf("Failed to connect, return code %d\n", rc);
					exit(-1);
				}
				pubmsg.payload = str1;
				pubmsg.payloadlen = strlen(str1);
				pubmsg.qos = QOS;
				pubmsg.retained = 0;
				MQTTClient_publishMessage(client, TOPIC_AIR, &pubmsg, &token);
				printf("Waiting for up to %d seconds for publication of %s\n"
						"on topic %s\n",
						(int)(TIMEOUT/1000), str1, TOPIC_AIR);
				rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
				printf("Message with delivery token %d delivered\n", token);
				MQTTClient_disconnect(client, 10000);
				MQTTClient_destroy(&client);	
			}
		//}
		
		
		sleep(1);
		oldhumidity = humidity;
		humidity = readHumidity();
		sprintf(str2, "%.2f", humidity);
		printf("humidity: %s\n",str2);
      
		if (oldhumidity == humidity)
		{
			printf("Same humidity value\n");
		}
		else
		{
			MQTTClient client2;
			MQTTClient_connectOptions conn_opts2 = MQTTClient_connectOptions_initializer;
			MQTTClient_message pubmsg2 = MQTTClient_message_initializer;
			MQTTClient_deliveryToken token2;
			int rc2;

			MQTTClient_create(&client2, ADDRESS, CLIENTID,MQTTCLIENT_PERSISTENCE_NONE, NULL);
			conn_opts2.keepAliveInterval = 20;
			conn_opts2.cleansession = 1;	
		
			if ((rc2 = MQTTClient_connect(client2, &conn_opts2)) != MQTTCLIENT_SUCCESS)
				{
					printf("Failed to connect, return code %d\n", rc2);
					exit(-1);
				}
				
			pubmsg2.payload = str2;
			pubmsg2.payloadlen = strlen(str2);
			pubmsg2.qos = QOS;
			pubmsg2.retained = 0;
			MQTTClient_publishMessage(client2, TOPIC_HUM, &pubmsg2, &token2);
			printf("Waiting for up to %d seconds for publication of %s\n"
					"on topic %s\n",
					(int)(TIMEOUT/1000), str2, TOPIC_HUM);
			rc2 = MQTTClient_waitForCompletion(client2, token2, TIMEOUT);
			printf("Message with delivery token %d delivered\n", token2);
			MQTTClient_disconnect(client2, 10000);
			MQTTClient_destroy(&client2);
		}
		
		
		
		
		sleep(1);
		oldtemperatureMPL = temperatureMPL;
		temperatureMPL = readTemperatureMPL();
		sprintf(str3, "%.2f", temperatureMPL);
		printf("Temperature: %s\n",str3);

		if(oldtemperatureMPL == temperatureMPL)
		{
			printf("Same temperature value\n");
		}
		else
		{
				sprintf(pythonvar,"temp=%f",temperatureMPL);
				PyRun_SimpleString(pythonvar);

			if(temperatureMPL > 0)
			{
//				sleep(6);		

//				sprintf(pythonvar,"temp=%f",temperatureMPL);
//				PyRun_SimpleString(pythonvar);
//				sleep(6);		
			
				MQTTClient client3;
				MQTTClient_connectOptions conn_opts3 = MQTTClient_connectOptions_initializer;
				MQTTClient_message pubmsg3 = MQTTClient_message_initializer;
				MQTTClient_deliveryToken token3;
				int rc3;

				MQTTClient_create(&client3, ADDRESS, CLIENTID,MQTTCLIENT_PERSISTENCE_NONE, NULL);
				conn_opts3.keepAliveInterval = 20;
				conn_opts3.cleansession = 1;	
			
				if ((rc3 = MQTTClient_connect(client3, &conn_opts3)) != MQTTCLIENT_SUCCESS)
				{
					printf("Failed to connect, return code %d\n", rc3);
					exit(-1);
				}
					
				pubmsg3.payload = str3;
				pubmsg3.payloadlen = strlen(str3);
				pubmsg3.qos = QOS;
				pubmsg3.retained = 0;
				MQTTClient_publishMessage(client3, TOPIC_TEMP, &pubmsg3, &token3);
				printf("Waiting for up to %d seconds for publication of %s\n"
						"on topic %s\n",
						(int)(TIMEOUT/1000), str3, TOPIC_TEMP);
				rc3 = MQTTClient_waitForCompletion(client3, token3, TIMEOUT);
				printf("Message with delivery token %d delivered\n", token3);
				MQTTClient_disconnect(client3, 10000);
				MQTTClient_destroy(&client3);
			}
		}
		
		sleep(1);
		oldaltitude = altitude;
		altitude = readAltitude();
		sprintf(str4, "%.2f", altitude);
		printf("Altitude %s\n", str4);
		
		if(oldaltitude == altitude)
		{
			printf("Same altitude value\n");
		}
		else
		{		
		MQTTClient client4;
			MQTTClient_connectOptions conn_opts4 = MQTTClient_connectOptions_initializer;
			MQTTClient_message pubmsg4 = MQTTClient_message_initializer;
			MQTTClient_deliveryToken token4;
			int rc4;

			MQTTClient_create(&client4, ADDRESS, CLIENTID,MQTTCLIENT_PERSISTENCE_NONE, NULL);
			conn_opts4.keepAliveInterval = 20;
			conn_opts4.cleansession = 1;	
		
			if ((rc4 = MQTTClient_connect(client4, &conn_opts4)) != MQTTCLIENT_SUCCESS)
			{
				printf("Failed to connect, return code %d\n", rc4);
				exit(-1);
			}
			pubmsg4.payload = str4;
			pubmsg4.payloadlen = strlen(str4);
			pubmsg4.qos = QOS;
			pubmsg4.retained = 0;
			MQTTClient_publishMessage(client4, TOPIC_ALT, &pubmsg4, &token4);
			printf("Waiting for up to %d seconds for publication of %s\n"
					"on topic %s\n",
					(int)(TIMEOUT/1000), str4, TOPIC_ALT);
			rc4 = MQTTClient_waitForCompletion(client4, token4, TIMEOUT);
			printf("Message with delivery token %d delivered\n", token4);
			MQTTClient_disconnect(client4, 10000);
			MQTTClient_destroy(&client4);		

		}
		
		sleep(1);	
		oldpressure = pressure;		
		pressure = readPressure();
		sprintf(str5, "%.2f", pressure);
		printf("Pressure %s\n", str5);           
		
	
		if(oldpressure == pressure)
		{
			printf("Same pressure value\n");
		}
		else
		{
			MQTTClient client5;
			MQTTClient_connectOptions conn_opts5 = MQTTClient_connectOptions_initializer;
			MQTTClient_message pubmsg5 = MQTTClient_message_initializer;
			MQTTClient_deliveryToken token5;
			int rc5;

			MQTTClient_create(&client5, ADDRESS, CLIENTID,MQTTCLIENT_PERSISTENCE_NONE, NULL);
			conn_opts5.keepAliveInterval = 20;
			conn_opts5.cleansession = 1;	
			
			if ((rc5 = MQTTClient_connect(client5, &conn_opts5)) != MQTTCLIENT_SUCCESS)
			{
				printf("Failed to connect, return code %d\n", rc5);
				exit(-1);
			}

			pubmsg5.payload = str5;
			pubmsg5.payloadlen = strlen(str5);
			pubmsg5.qos = QOS;
			pubmsg5.retained = 0;
			MQTTClient_publishMessage(client5, TOPIC_PRES, &pubmsg5, &token5);
			printf("Waiting for up to %d seconds for publication of %s\n"
						"on topic %s\n",
						(int)(TIMEOUT/1000), str5, TOPIC_PRES);
			rc5 = MQTTClient_waitForCompletion(client5, token5, TIMEOUT);
			printf("Message with delivery token %d delivered\n", token5);
			MQTTClient_disconnect(client5, 10000);
			MQTTClient_destroy(&client5);		
		}
		
		
	}
  
}
