/*
MIFARE 1k classic card exists out of 16 sectors each secor has 4 blocks, each block has 16 bytes in it.
64 blocks*16 bytes per block = 1024 bytes
The UID can be found at the 4 first bytes of sector 0, block 0*/

#include "main.h"
#include <string.h>
#include "stm32l1xx_hal.h"
#include "PN532.h"

I2C_HandleTypeDef *hi2cLib;


uint8_t WRITE_REGISTER_PN532(uint8_t pData[],uint8_t length)
{
	uint8_t status=HAL_I2C_Master_Transmit(hi2cLib, PN532_I2C_ADDRESS <<1, pData,length, HAL_MAX_DELAY);
	return status;
}
uint8_t READ_REGISTER_PN532(uint8_t buf[],uint8_t reg,uint8_t length)
{
	uint8_t status = HAL_I2C_Mem_Read(hi2cLib, PN532_I2C_ADDRESS<<1, reg, I2C_MEMADD_SIZE_8BIT, buf, length, HAL_MAX_DELAY);
	return status;
}

void setI2CInterface_PN532(I2C_HandleTypeDef *hi2c)
{
	hi2cLib=hi2c;
}

uint8_t isready()
{
	// I2C check if status is ready by IRQ line being pulled low.
	uint8_t x = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8);
	return x == 0;
}

uint8_t waitready(uint16_t timeout)
{
	  uint16_t timer = 0;
	  while(!isready()) {
	    if (timeout != 0) {
	      timer += 10;
	      if (timer > timeout) {

	        return 0;
	      }
	    }
	    HAL_Delay(10);
	  }
	  return 1;
}

void writecommand(uint8_t* cmd, uint8_t cmdlen)
{
	uint8_t checksum;
	uint8_t temp=0;
	cmdlen++;
	HAL_Delay(2) ;    // or whatever the delay is for waking up the board

	checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
	writeBuffer[temp]=PN532_PREAMBLE;
	temp++;
	writeBuffer[temp]=PN532_PREAMBLE;
	temp++;
	writeBuffer[temp]=PN532_STARTCODE2;
	temp++;
	writeBuffer[temp]=cmdlen;
	temp++;
	writeBuffer[temp]=~cmdlen+1;
	temp++;
	writeBuffer[temp]=PN532_HOSTTOPN532;
	temp++;

	checksum += PN532_HOSTTOPN532;
	uint8_t i=0;
	for (i=0; i<cmdlen-1; i++)
	{
		writeBuffer[temp]=cmd[i];
		temp++;
		checksum += cmd[i];
	}
	writeBuffer[temp]=~checksum;
	temp++;
	writeBuffer[temp]=PN532_POSTAMBLE;
	temp++;
	WRITE_REGISTER_PN532(writeBuffer,temp);
}

void readdata(uint8_t buff[], uint8_t n)
{
	HAL_Delay(2);
	HAL_I2C_Master_Receive(hi2cLib, PN532_I2C_ADDRESS<<1, buff, n, HAL_MAX_DELAY); // notmal n+2
}
uint8_t readack()
{
	//uint8_t pn532ack[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
        uint8_t pn532ack[] = {0x01, 0x00, 0x00, 0xFF, 0x00, 0xFF};
	uint8_t ackbuff[7];
	readdata(ackbuff, 6);
        
	uint8_t temp=(0 == strncmp((char *)ackbuff, (char *)pn532ack, 7)); // changed ackbuff to (ackbuff>>8)
        
	return (0 == strncmp((char *)ackbuff, (char *)pn532ack, 7));
}

uint8_t sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout)
{
	// write the command
	writecommand(cmd, cmdlen);
	// Wait for chip to say its ready!
	if (!waitready(timeout))
	{
            return 0;
	}
	// read acknowledgement
	if (!readack())
	{
	    return 0;
	}

	return 1; // ack'd command
}

uint32_t getFirmwareVersion()
{
	uint32_t response;
	uint8_t pn532response_firmwarevers[] = {0x01,0x00, 0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};
	pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;
	if (! sendCommandCheckAck(pn532_packetbuffer, 1,1000))
	{
		return 0;
	}
	// read data packet
	readdata(pn532_packetbuffer, 12); //ok
	/*char str[80];
  	sprintf(str, " >> Firmware: 0x%02X,0x%02X,0x%02X,0x%02X", pn532_packetbuffer[8],pn532_packetbuffer[9],pn532_packetbuffer[10],pn532_packetbuffer[11]);
	HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);*/
        

	// check some basic stuff 0x01 0x00 0x00 0xFF 0x00 0xFF
	if (0 != strncmp((char *)pn532_packetbuffer, (char *)pn532response_firmwarevers, 8)) // must be equal
	{
		return 0;
	}

	int offset = 8;  // Skip a response byte when using I2C to ignore extra data.//+1 for not discarding leading 0x01
	response = pn532_packetbuffer[offset++];
	response <<= 8;
	response |= pn532_packetbuffer[offset++];
	response <<= 8;
	response |= pn532_packetbuffer[offset++];
	response <<= 8;
	response |= pn532_packetbuffer[offset++];
        
	return response;
}

  /*CARDS FUNCTIONS*/
  uint8_t SAMConfig() {  // executes ok
  pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
  pn532_packetbuffer[1] = 0x01; // normal mode;
  pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
  pn532_packetbuffer[3] = 0x01; // use IRQ pin!
  
  if (!sendCommandCheckAck(pn532_packetbuffer, 4, 1000))
  { 
          return 0;
  }
   

  // read data packet
  readdata(pn532_packetbuffer, 8);
   
   /*	char str[80];
	sprintf(str, " - samconf: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X - ",pn532_packetbuffer[0], pn532_packetbuffer[1], pn532_packetbuffer[2], pn532_packetbuffer[3], pn532_packetbuffer[4], pn532_packetbuffer[5], pn532_packetbuffer[6], pn532_packetbuffer[7]);
	HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
*/
	return  (pn532_packetbuffer[6] == 0x15); //changed 6 to 7 for not discarding leading 0x01
}


/*! 
    Waits for an ISO14443A target to enter the field
    
    @param  cardBaudRate  Baud rate of the card
    @param  uid           Pointer to the array that will be populated
                          with the card's UID (up to 7 bytes)
    @param  uidLength     Pointer to the variable that will hold the
                          length of the card's UID.
    
    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
//uint8_t readPassiveTargetID(uint8_t cardbaudrate, uint8_t * uid, uint8_t * uidLength, uint16_t timeout) {
uint32_t readPassiveTargetID(uint8_t cardbaudrate, uint8_t * uid, uint8_t * uidLength, uint16_t timeout) {
  pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET; //4A
  pn532_packetbuffer[1] = 1;  // max 1 cards at once (we can set this to 2 later) //01
  pn532_packetbuffer[2] = cardbaudrate; //00
  
  if (! sendCommandCheckAck(pn532_packetbuffer, 3, timeout)) //execute ok
  {
    return 0x0;  // no cards read
  }
  
	// wait for a card to enter the field (only possible with I2C) TODO
	if (!waitready(0))
	{
		return 0x0;
	}

  // read data packet
    readdata(pn532_packetbuffer, 20);
  
  // check some basic stuff
  /* ISO14443A card response should be in the following format:
  
    byte            Description
    -------------   ------------------------------------------
    b1              leading byte (0x01)
    b2              PREAMBLE
    b3...b4         START CODE (0x00 and 0xFF)
    b5              LEN, 1 byte indicating the number of bytes in the data field (TFI and PD0 to PDn) (0x0C) 12 bytes
    b6              LCS,  1 Packet Length Checksum LCS byte that satisfies the relation:  Lower byte of [LEN + LCS] = 0x00, 
    b7              Tags Found, TFI (0xD5 in case of a frame from the PN532 to the host controller)
    
    b8-b18         DATA
    b8             command code (4b)
    b9             always (0x01)
    b10            always (0x01)
    b11..12        SENS_RES (0x00 0x04)         
    b13            SEL_RES (0x08)
    b14            NFCID Length (0x04)
    b15..b18       UID (depend from tag or card)
  
    b19            DCS,  data checksum (0x00)
    b20            POSTAMBLE (0x00)*/
 
  
  
  if (pn532_packetbuffer[8] != 1) 
    return 0;
    
  uint16_t sens_res = pn532_packetbuffer[9];
  sens_res <<= 8;
  sens_res |= pn532_packetbuffer[10];
  
  /* Card appears to be Mifare Classic */
  *uidLength = pn532_packetbuffer[13]; // was 12 but +1

  for (uint8_t i=0; i < pn532_packetbuffer[12]; i++) 
  {
    uid[i] = pn532_packetbuffer[13+i];
  }
   
  uint16_t a = (pn532_packetbuffer[14]<<8) | pn532_packetbuffer[15];
  uint16_t b = (pn532_packetbuffer[16]<<8) | pn532_packetbuffer[17];
  uint32_t out = (a<<16) | b ;
  return out;
  //return 1;
}