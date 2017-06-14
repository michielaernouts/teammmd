/*
 * PN532.c
 *
 *  Created on: 20 Apr 2017
 *      Author: Fifth
 */
#include "PN532.h"

void setI2CInterface_PN532(I2C_HandleTypeDef *hi2c)
{
	hi2cLib=hi2c;
}

uint8_t WRITE_REGISTER_PN532(uint8_t pData[],uint8_t length)
{
	uint8_t status=HAL_I2C_Master_Transmit(hi2cLib, PN532_I2C_ADDRESS<<1 , pData,length, HAL_MAX_DELAY);
		return status;
	}
uint8_t READ_REGISTER_PN532(uint8_t buf[],uint8_t reg,uint8_t length)
{
	uint8_t status = HAL_I2C_Mem_Read(hi2cLib, PN532_I2C_ADDRESS<<1, reg, I2C_MEMADD_SIZE_8BIT, buf, length, HAL_MAX_DELAY);
		return status;
	}

void begin()
{
/*
 *    // Reset the PN532
    digitalWrite(_reset, HIGH);
    digitalWrite(_reset, LOW);
    delay(400);
    digitalWrite(_reset, HIGH);
    delay(10);  // Small delay required before taking other actions after reset.
                // See timing diagram on page 209 of the datasheet, section 12.23.
 */
}

  // Generic PN532 functions
uint8_t SAMConfig()
{
	pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
	pn532_packetbuffer[1] = 0x01; // normal mode;
	pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
	pn532_packetbuffer[3] = 0x01; // use IRQ pin!

	if (!sendCommandCheckAck(pn532_packetbuffer, 4,1000))
		return 0;

	// read data packet
	readdata(pn532_packetbuffer, 8);

	int offset = 6+1; //+1 for not discarding leading 0x01
	return  (pn532_packetbuffer[offset] == 0x15);
}
uint32_t getFirmwareVersion()
{
	uint32_t response;
	uint8_t pn532response_firmwarevers[] = {0x01,0x00,0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};
	pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

	if (! sendCommandCheckAck(pn532_packetbuffer, 1,1000))
	{

		return 0;
	}

	// read data packet
	readdata(pn532_packetbuffer, 12);
		// check some basic stuff
	if (0 != strncmp((char *)pn532_packetbuffer, (char *)pn532response_firmwarevers, 8))
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
uint8_t sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout)
{
	uint16_t timer = 0;

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

uint8_t setPassiveActivationRetries(uint8_t maxRetries)
{
	pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
	pn532_packetbuffer[1] = 5;    // Config item 5 (MaxRetries)
	pn532_packetbuffer[2] = 0xFF; // MxRtyATR (default = 0xFF)
	pn532_packetbuffer[3] = 0x01; // MxRtyPSL (default = 0x01)
	pn532_packetbuffer[4] = maxRetries;

	if (! sendCommandCheckAck(pn532_packetbuffer, 5,1000))
		return 0;  // no ACK

	return 1;
}

// ISO14443A functions
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
uint8_t readPassiveTargetID(uint8_t cardbaudrate, uint8_t * uid, uint8_t * uidLength, uint16_t timeout)
{
	pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
	pn532_packetbuffer[1] = 1;  // max 1 cards at once (we can set this to 2 later)
	pn532_packetbuffer[2] = cardbaudrate;

	if (!sendCommandCheckAck(pn532_packetbuffer, 3, timeout))
	{
		return 0x0;  // no cards read
	}

	// wait for a card to enter the field (only possible with I2C) TODO

	if (!waitready(timeout))
	{
		return 0x0;
	}

	// read data packet
	readdata(pn532_packetbuffer, 20);
	// check some basic stuff

	/* ISO14443A card response should be in the following format:

	byte            Description
	-------------   ------------------------------------------
	b0..6           Frame header and preamble
	b7              Tags Found
	b8              Tag Number (only one used in this example)
	b9..10          SENS_RES
	b11             SEL_RES
	b12             NFCID Length
	b13..NFCIDLen   NFCID                                      */

	if (pn532_packetbuffer[8] != 1)////+1 for not discarding leading 0x01
		return 0;

	uint16_t sens_res = pn532_packetbuffer[10];
	sens_res <<= 8;
	sens_res |= pn532_packetbuffer[10];

	/* Card appears to be Mifare Classic */
	*uidLength = pn532_packetbuffer[12];

	for (uint8_t i=0; i < pn532_packetbuffer[12]; i++)
	{
		uid[i] = pn532_packetbuffer[13+i];
	}

	return 1;
}//timeout 0 means no timeout - will block forever.
/*!
    @brief  Exchanges an APDU with the currently inlisted peer

    @param  send            Pointer to data to send
    @param  sendLength      Length of the data to send
    @param  response        Pointer to response data
    @param  responseLength  Pointer to the response data length
*/
uint8_t inDataExchange(uint8_t * send, uint8_t sendLength, uint8_t * response, uint8_t * responseLength)
{
	if (sendLength > PN532_PACKBUFFSIZ-2)
	{
		return 0; //APDU length too long for packet buffer
	}
	uint8_t i;

	pn532_packetbuffer[0] = 0x40; // PN532_COMMAND_INDATAEXCHANGE;
	pn532_packetbuffer[1] = _inListedTag;
	for (i=0; i<sendLength; ++i)
	{
		pn532_packetbuffer[i+2] = send[i];
	}

	if (!sendCommandCheckAck(pn532_packetbuffer,sendLength+2,1000))
	{
		return 0;//Could not send ADPU
	}

	if (!waitready(1000))
	{
		return 0;//Response never received for ADPU..
	}

	readdata(pn532_packetbuffer,sizeof(pn532_packetbuffer));

	if (pn532_packetbuffer[0] == 0 && pn532_packetbuffer[1] == 0 && pn532_packetbuffer[2] == 0xff)
	{
		uint8_t length = pn532_packetbuffer[3];
		if (pn532_packetbuffer[4]!=(uint8_t)(~length+1))
		{
			return 0;//Length check invalid
		}
		if (pn532_packetbuffer[5]==PN532_PN532TOHOST && pn532_packetbuffer[6]==PN532_RESPONSE_INDATAEXCHANGE)
		{
			if ((pn532_packetbuffer[7] & 0x3f)!=0)
			{
				return 0;//Status code indicates an error
			}

			length -= 3;

			if (length > *responseLength)
			{
			length = *responseLength; // silent truncation...
			}

			for (i=0; i<length; ++i)
			{
				response[i] = pn532_packetbuffer[8+i];
			}
			*responseLength = length;

			return 1;
		}
		else
		{
			return 0; //Don't know how to handle this command
		}
	}
	else
	{
		return 0;//Preamble missing
	}
}
/*!
    @brief  'InLists' a passive target. PN532 acting as reader/initiator,
            peer acting as card/responder.
*/
uint8_t inListPassiveTarget()
{
	pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
	pn532_packetbuffer[1] = 1;
	pn532_packetbuffer[2] = 0;
	//About to inList passive target
	if (!sendCommandCheckAck(pn532_packetbuffer,3,1000))
	{
		return 0;//Could not send inlist message
	}

	if (!waitready(30000))
	{
		return 0;
	}

	readdata(pn532_packetbuffer,sizeof(pn532_packetbuffer));

	if (pn532_packetbuffer[0] == 0 && pn532_packetbuffer[1] == 0 && pn532_packetbuffer[2] == 0xff)
	{
		uint8_t length = pn532_packetbuffer[3];
		if (pn532_packetbuffer[4]!=(uint8_t)(~length+1))
		{
			return 0;//Length check invalid
		}
		if (pn532_packetbuffer[5]==PN532_PN532TOHOST && pn532_packetbuffer[6]==PN532_RESPONSE_INLISTPASSIVETARGET)
		{
			if (pn532_packetbuffer[7] != 1)
			{
				return 0;//Unhandled number of targets inlisted
			}
			_inListedTag = pn532_packetbuffer[8];
		return 1;
		}
		else
		{
			return 0;//Unexpected response to inlist passive host
		}
	}
	else
	{
		return 0;//Preamble missing
	}
	return 1;
}
// Mifare Classic functions
/*!
      Indicates whether the specified block number is the first block
      in the sector (block 0 relative to the current sector)
*/
uint8_t mifareclassic_IsFirstBlock (uint32_t uiBlock)
{
	 // Test if we are in the small or big sectors
	if (uiBlock < 128)
		return ((uiBlock) % 4 == 0);
	else
		return ((uiBlock) % 16 == 0);
}
/*!
      Indicates whether the specified block number is the sector trailer
*/
uint8_t mifareclassic_IsTrailerBlock (uint32_t uiBlock)
{
	  // Test if we are in the small or big sectors
	  if (uiBlock < 128)
	    return ((uiBlock + 1) % 4 == 0);
	  else
	    return ((uiBlock + 1) % 16 == 0);
}
/*!
    Tries to authenticate a block of memory on a MIFARE card using the
    INDATAEXCHANGE command.  See section 7.3.8 of the PN532 User Manual
    for more information on sending MIFARE and other commands.

    @param  uid           Pointer to a byte array containing the card UID
    @param  uidLen        The length (in bytes) of the card's UID (Should
                          be 4 for MIFARE Classic)
    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  keyNumber     Which key type to use during authentication
                          (0 = MIFARE_CMD_AUTH_A, 1 = MIFARE_CMD_AUTH_B)
    @param  keyData       Pointer to a byte array containing the 6 byte
                          key value

    @returns 1 if everything executed properly, 0 for an error
*/
uint8_t mifareclassic_AuthenticateBlock (uint8_t * uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t * keyData)
{
	//uint8_t len;
	uint8_t i;

	// Hang on to the key and uid data
	memcpy (_key, keyData, 6);
	memcpy (_uid, uid, uidLen);
	_uidLen = uidLen;

	// Prepare the authentication command //
	pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;   /* Data Exchange Header */
	pn532_packetbuffer[1] = 1;                              /* Max card numbers */
	pn532_packetbuffer[2] = (keyNumber) ? MIFARE_CMD_AUTH_B : MIFARE_CMD_AUTH_A;
	pn532_packetbuffer[3] = blockNumber;                    /* Block Number (1K = 0..63, 4K = 0..255 */
	memcpy (pn532_packetbuffer+4, _key, 6);
	for (i = 0; i < _uidLen; i++)
	{
		pn532_packetbuffer[10+i] = _uid[i];                /* 4 byte card ID */
	}

	if (! sendCommandCheckAck(pn532_packetbuffer, 10+_uidLen,1000))
		return 0;

	// Read the response packet
	readdata(pn532_packetbuffer, 12);

	// check if the response is valid and we are authenticated???
	// for an auth success it should be bytes 5-7: 0xD5 0x41 0x00
	// Mifare auth error is technically byte 7: 0x14 but anything other and 0x00 is not good
	if (pn532_packetbuffer[7] != 0x00)
	{
		return 0; //"Authentification failed
	}

	return 1;
}
/*!
    Tries to read an entire 16-byte data block at the specified block
    address.

    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  data          Pointer to the byte array that will hold the
                          retrieved data (if any)

    @returns 1 if everything executed properly, 0 for an error
*/
uint8_t mifareclassic_ReadDataBlock (uint8_t blockNumber, uint8_t * data)
{
	/* Prepare the command */
	pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
	pn532_packetbuffer[1] = 1;                      /* Card number */
	pn532_packetbuffer[2] = MIFARE_CMD_READ;        /* Mifare Read command = 0x30 */
	pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */

	/* Send the command */
	if (! sendCommandCheckAck(pn532_packetbuffer, 4,1000))
	{
	  return 0; //Failed to receive ACK for read command
	}

	/* Read the response packet */
	readdata(pn532_packetbuffer, 26);

	/* If byte 8 isn't 0x00 we probably have an error */
	if (pn532_packetbuffer[7] != 0x00)
	{
	  return 0; //Unexpected response
	}

	/* Copy the 16 data bytes to the output buffer        */
	/* Block content starts at byte 9 of a valid response */
	memcpy (data, pn532_packetbuffer+8, 16);

	return 1;
}
/*!
    Tries to write an entire 16-byte data block at the specified block
    address.

    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  data          The byte array that contains the data to write.

    @returns 1 if everything executed properly, 0 for an error
*/
uint8_t mifareclassic_WriteDataBlock (uint8_t blockNumber, uint8_t * data)
{
	 /* Prepare the first command */
	 pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
	 pn532_packetbuffer[1] = 1;                      /* Card number */
	 pn532_packetbuffer[2] = MIFARE_CMD_WRITE;       /* Mifare Write command = 0xA0 */
	 pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */
	 memcpy (pn532_packetbuffer+4, data, 16);          /* Data Payload */

	 /* Send the command */
	 if (! sendCommandCheckAck(pn532_packetbuffer, 20,1000))
	 {
	   return 0;//Failed to receive ACK for write command
	 }
	HAL_Delay(10);
	 /* Read the response packet */
	 readdata(pn532_packetbuffer, 26);

	 return 1;
}
/*!
    Formats a Mifare Classic card to store NDEF Records

    @returns 1 if everything executed properly, 0 for an error
*/
uint8_t mifareclassic_FormatNDEF (void)
{
	uint8_t sectorbuffer1[16] = {0x14, 0x01, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
	uint8_t sectorbuffer2[16] = {0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
	uint8_t sectorbuffer3[16] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0x78, 0x77, 0x88, 0xC1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	// Note 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 must be used for key A
	// for the MAD sector in NDEF records (sector 0)

	// Write block 1 and 2 to the card
	if (!(mifareclassic_WriteDataBlock (1, sectorbuffer1)))
		return 0;
	if (!(mifareclassic_WriteDataBlock (2, sectorbuffer2)))
		return 0;
	// Write key A and access rights card
	if (!(mifareclassic_WriteDataBlock (3, sectorbuffer3)))
		return 0;
	// Seems that everything was OK (?!)
	return 1;
}
/*!
    Writes an NDEF URI Record to the specified sector (1..15)

    Note that this function assumes that the Mifare Classic card is
    already formatted to work as an "NFC Forum Tag" and uses a MAD1
    file system.  You can use the NXP TagWriter app on Android to
    properly format cards for this.

    @param  sectorNumber  The sector that the URI record should be written
                          to (can be 1..15 for a 1K card)
    @param  uriIdentifier The uri identifier code (0 = none, 0x01 =
                          "http://www.", etc.)
    @param  url           The uri text to write (max 38 characters).

    @returns 1 if everything executed properly, 0 for an error
*/
uint8_t mifareclassic_WriteNDEFURI (uint8_t sectorNumber, uint8_t uriIdentifier, const char * url)
{
	 // Figure out how long the string is
	  uint8_t len = strlen(url);

	  // Make sure we're within a 1K limit for the sector number
	  if ((sectorNumber < 1) || (sectorNumber > 15))
	    return 0;

	  // Make sure the URI payload is between 1 and 38 chars
	  if ((len < 1) || (len > 38))
	    return 0;

	  // Note 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 must be used for key A
	  // in NDEF records

	  // Setup the sector buffer (w/pre-formatted TLV wrapper and NDEF message)
	  uint8_t sectorbuffer1[16] = {0x00, 0x00, 0x03, len+5, 0xD1, 0x01, len+1, 0x55, uriIdentifier, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	  uint8_t sectorbuffer2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	  uint8_t sectorbuffer3[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	  uint8_t sectorbuffer4[16] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7, 0x7F, 0x07, 0x88, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	  if (len <= 6)
	  {
	    // Unlikely we'll get a url this short, but why not ...
	    memcpy (sectorbuffer1+9, url, len);
	    sectorbuffer1[len+9] = 0xFE;
	  }
	  else if (len == 7)
	  {
	    // 0xFE needs to be wrapped around to next block
	    memcpy (sectorbuffer1+9, url, len);
	    sectorbuffer2[0] = 0xFE;
	  }
	  else if ((len > 7) && (len <= 22))
	  {
	    // Url fits in two blocks
	    memcpy (sectorbuffer1+9, url, 7);
	    memcpy (sectorbuffer2, url+7, len-7);
	    sectorbuffer2[len-7] = 0xFE;
	  }
	  else if (len == 23)
	  {
	    // 0xFE needs to be wrapped around to final block
	    memcpy (sectorbuffer1+9, url, 7);
	    memcpy (sectorbuffer2, url+7, len-7);
	    sectorbuffer3[0] = 0xFE;
	  }
	  else
	  {
	    // Url fits in three blocks
	    memcpy (sectorbuffer1+9, url, 7);
	    memcpy (sectorbuffer2, url+7, 16);
	    memcpy (sectorbuffer3, url+23, len-24);
	    sectorbuffer3[len-22] = 0xFE;
	  }

	  // Now write all three blocks back to the card
	  if (!(mifareclassic_WriteDataBlock (sectorNumber*4, sectorbuffer1)))
	    return 0;
	  if (!(mifareclassic_WriteDataBlock ((sectorNumber*4)+1, sectorbuffer2)))
	    return 0;
	  if (!(mifareclassic_WriteDataBlock ((sectorNumber*4)+2, sectorbuffer3)))
	    return 0;
	  if (!(mifareclassic_WriteDataBlock ((sectorNumber*4)+3, sectorbuffer4)))
	    return 0;

	  // Seems that everything was OK (?!)
	  return 1;
}

// Mifare Ultralight functions
/*!
    Tries to read an entire 4-byte page at the specified address.

    @param  page        The page number (0..63 in most cases)
    @param  buffer      Pointer to the byte array that will hold the
                        retrieved data (if any)
*/
uint8_t mifareultralight_ReadPage (uint8_t page, uint8_t * buffer)
{
	 if (page >= 64)
	  {
	    return 0;//Page value out of range
	  }
	  /* Prepare the command */
	  pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
	  pn532_packetbuffer[1] = 1;                   /* Card number */
	  pn532_packetbuffer[2] = MIFARE_CMD_READ;     /* Mifare Read command = 0x30 */
	  pn532_packetbuffer[3] = page;                /* Page Number (0..63 in most cases) */

	  /* Send the command */
	  if (! sendCommandCheckAck(pn532_packetbuffer, 4,1000))
	  {
	    return 0;//Failed to receive ACK for write command
	  }

	  /* Read the response packet */
	  readdata(pn532_packetbuffer, 26);

	  /* If byte 8 isn't 0x00 we probably have an error */
	  if (pn532_packetbuffer[7] == 0x00)
	  {
	    /* Copy the 4 data bytes to the output buffer         */
	    /* Block content starts at byte 9 of a valid response */
	    /* Note that the command actually reads 16 byte or 4  */
	    /* pages at a time ... we simply discard the last 12  */
	    /* bytes                                              */
	    memcpy (buffer, pn532_packetbuffer+8, 4);
	  }
	  else
	  {
	    return 0;//Unexpected response reading block
	  }

	  // Return OK signal
	  return 1;
}
/*!
    Tries to write an entire 4-byte page at the specified block
    address.

    @param  page          The page number to write.  (0..63 for most cases)
    @param  data          The byte array that contains the data to write.
                          Should be exactly 4 bytes long.

    @returns 1 if everything executed properly, 0 for an error
*/
uint8_t mifareultralight_WritePage (uint8_t page, uint8_t * data)
{
	if (page >= 64)
	{
		return 0; //Page value out of range
	}

	/* Prepare the first command */
	pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
	pn532_packetbuffer[1] = 1;                      /* Card number */
	pn532_packetbuffer[2] = MIFARE_ULTRALIGHT_CMD_WRITE;       /* Mifare Ultralight Write command = 0xA2 */
	pn532_packetbuffer[3] = page;            /* Page Number (0..63 for most cases) */
	memcpy (pn532_packetbuffer+4, data, 4);          /* Data Payload */

	/* Send the command */
	if (! sendCommandCheckAck(pn532_packetbuffer, 8,1000))
	{
		return 0;//Failed to receive ACK for write command
	}
	HAL_Delay(10);
	/* Read the response packet */
	readdata(pn532_packetbuffer, 26);

	// Return OK Signal
	return 1;
}

// NTAG2xx functions
/*!
    Tries to read an entire 4-byte page at the specified address.

    @param  page        The page number (0..63 in most cases)
    @param  buffer      Pointer to the byte array that will hold the
                        retrieved data (if any)
*/
uint8_t ntag2xx_ReadPage (uint8_t page, uint8_t * buffer)
{
	// TAG Type       PAGES   USER START    USER STOP
	  // --------       -----   ----------    ---------
	  // NTAG 203       42      4             39
	  // NTAG 213       45      4             39
	  // NTAG 215       135     4             129
	  // NTAG 216       231     4             225

	  if (page >= 231)
	  {
	    return 0;//Page value out of range
	  }
	  /* Prepare the command */
	  pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
	  pn532_packetbuffer[1] = 1;                   /* Card number */
	  pn532_packetbuffer[2] = MIFARE_CMD_READ;     /* Mifare Read command = 0x30 */
	  pn532_packetbuffer[3] = page;                /* Page Number (0..63 in most cases) */

	  /* Send the command */
	  if (! sendCommandCheckAck(pn532_packetbuffer, 4,1000))
	  {
	    return 0;//Failed to receive ACK for write command
	  }

	  /* Read the response packet */
	  readdata(pn532_packetbuffer, 26);

	  /* If byte 8 isn't 0x00 we probably have an error */
	  if (pn532_packetbuffer[7] == 0x00)
	  {
	    /* Copy the 4 data bytes to the output buffer         */
	    /* Block content starts at byte 9 of a valid response */
	    /* Note that the command actually reads 16 byte or 4  */
	    /* pages at a time ... we simply discard the last 12  */
	    /* bytes                                              */
	    memcpy (buffer, pn532_packetbuffer+8, 4);
	  }
	  else
	  {
	    return 0;//Unexpected response reading block:
	  }
	  // Return OK signal
	  return 1;
}
/*!
    Tries to write an entire 4-byte page at the specified block
    address.

    @param  page          The page number to write.  (0..63 for most cases)
    @param  data          The byte array that contains the data to write.
                          Should be exactly 4 bytes long.

    @returns 1 if everything executed properly, 0 for an error
*/
uint8_t ntag2xx_WritePage (uint8_t page, uint8_t * data)
{
	// TAG Type       PAGES   USER START    USER STOP
	// --------       -----   ----------    ---------
	// NTAG 203       42      4             39
	// NTAG 213       45      4             39
	// NTAG 215       135     4             129
	// NTAG 216       231     4             225

	if ((page < 4) || (page > 225))
	{
		return 0;//Page value out of range
	}

	/* Prepare the first command */
	pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
	pn532_packetbuffer[1] = 1;                              /* Card number */
	pn532_packetbuffer[2] = MIFARE_ULTRALIGHT_CMD_WRITE;    /* Mifare Ultralight Write command = 0xA2 */
	pn532_packetbuffer[3] = page;                           /* Page Number (0..63 for most cases) */
	memcpy (pn532_packetbuffer+4, data, 4);                 /* Data Payload */

	/* Send the command */
	if (! sendCommandCheckAck(pn532_packetbuffer, 8,1000))
	{
		return 0;//Failed to receive ACK for write comman
	}
	HAL_Delay(10);

	/* Read the response packet */
	readdata(pn532_packetbuffer, 26);

	// Return OK Signal
	return 1;
}
/*!
    Writes an NDEF URI Record starting at the specified page (4..nn)

    Note that this function assumes that the NTAG2xx card is
    already formatted to work as an "NFC Forum Tag".

    @param  uriIdentifier The uri identifier code (0 = none, 0x01 =
                          "http://www.", etc.)
    @param  url           The uri text to write (null-terminated string).
    @param  dataLen       The size of the data area for overflow checks.

    @returns 1 if everything executed properly, 0 for an error
*/
uint8_t ntag2xx_WriteNDEFURI (uint8_t uriIdentifier, char * url, uint8_t dataLen)
{
	  uint8_t pageBuffer[4] = { 0, 0, 0, 0 };

	  // Remove NDEF record overhead from the URI data (pageHeader below)
	  uint8_t wrapperSize = 12;

	  // Figure out how long the string is
	  uint8_t len = strlen(url);

	  // Make sure the URI payload will fit in dataLen (include 0xFE trailer)
	  if ((len < 1) || (len+1 > (dataLen-wrapperSize)))
	    return 0;

	  // Setup the record header
	  // See NFCForum-TS-Type-2-Tag_1.1.pdf for details
	  uint8_t pageHeader[12] =
	  {
	    /* NDEF Lock Control TLV (must be first and always present) */
	    0x01,         /* Tag Field (0x01 = Lock Control TLV) */
	    0x03,         /* Payload Length (always 3) */
	    0xA0,         /* The position inside the tag of the lock bytes (upper 4 = page address, lower 4 = byte offset) */
	    0x10,         /* Size in bits of the lock area */
	    0x44,         /* Size in bytes of a page and the number of bytes each lock bit can lock (4 bit + 4 bits) */
	    /* NDEF Message TLV - URI Record */
	    0x03,         /* Tag Field (0x03 = NDEF Message) */
	    len+5,        /* Payload Length (not including 0xFE trailer) */
	    0xD1,         /* NDEF Record Header (TNF=0x1:Well known record + SR + ME + MB) */
	    0x01,         /* Type Length for the record type indicator */
	    len+1,        /* Payload len */
	    0x55,         /* Record Type Indicator (0x55 or 'U' = URI Record) */
	    uriIdentifier /* URI Prefix (ex. 0x01 = "http://www.") */
	  };

	  // Write 12 byte header (three pages of data starting at page 4)
	  memcpy (pageBuffer, pageHeader, 4);
	  if (!(ntag2xx_WritePage (4, pageBuffer)))
	    return 0;
	  memcpy (pageBuffer, pageHeader+4, 4);
	  if (!(ntag2xx_WritePage (5, pageBuffer)))
	    return 0;
	  memcpy (pageBuffer, pageHeader+8, 4);
	  if (!(ntag2xx_WritePage (6, pageBuffer)))
	    return 0;

	  // Write URI (starting at page 7)
	  uint8_t currentPage = 7;
	  char * urlcopy = url;
	  while(len)
	  {
	    if (len < 4)
	    {
	      memset(pageBuffer, 0, 4);
	      memcpy(pageBuffer, urlcopy, len);
	      pageBuffer[len] = 0xFE; // NDEF record footer
	      if (!(ntag2xx_WritePage (currentPage, pageBuffer)))
	        return 0;
	      // DONE!
	      return 1;
	    }
	    else if (len == 4)
	    {
	      memcpy(pageBuffer, urlcopy, len);
	      if (!(ntag2xx_WritePage (currentPage, pageBuffer)))
	        return 0;
	      memset(pageBuffer, 0, 4);
	      pageBuffer[0] = 0xFE; // NDEF record footer
	      currentPage++;
	      if (!(ntag2xx_WritePage (currentPage, pageBuffer)))
	        return 0;
	      // DONE!
	      return 1;
	    }
	    else
	    {
	      // More than one page of data left
	      memcpy(pageBuffer, urlcopy, 4);
	      if (!(ntag2xx_WritePage (currentPage, pageBuffer)))
	        return 0;
	      currentPage++;
	      urlcopy+=4;
	      len-=4;
	    }
	  }

	  // Seems that everything was OK (?!)
	  return 1;
}

// Help functions to display formatted text


// Low level communication functions that handle both SPI and I2C.
/*!
    @brief  Reads n bytes of data from the PN532 via SPI or I2C.

    @param  buff      Pointer to the buffer where data will be written
    @param  n         Number of bytes to be read
*/
void readdata(uint8_t buff[], uint8_t n)
{
	uint16_t timer = 0;
	HAL_Delay(2);

	HAL_I2C_Master_Receive(hi2cLib, PN532_I2C_ADDRESS<<1, buff, n+2, HAL_MAX_DELAY);
}
/*!
    @brief  Writes a command to the PN532, automatically inserting the
            preamble and required frame details (checksum, len, etc.)

    @param  cmd       Pointer to the command buffer
    @param  cmdlen    Command length in bytes
*/
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
/*!
    @brief  Return true if the PN532 is ready with a response.
*/
uint8_t isready()
{
	// I2C check if status is ready by IRQ line being pulled low.
	uint8_t x = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_13); //TODO
	return x == 0;
}
/*!
    @brief  Waits until the PN532 is ready.

    @param  timeout   Timeout before giving up
*/
uint8_t waitready(uint16_t timeout)
{
	  uint16_t timer = 0;

	  while(!isready())
	  {
	    if (timeout != 0)
	    {
	      timer += 5;
	      if (timer > timeout)
	      {
	        return 0;
	      }
	    }
	    HAL_Delay(5);
	  }
	  return 1;
}
/*!
    @brief  Tries to read the  I2C ACK signal
*/
uint8_t readack()
{

	uint8_t pn532ack[] = {0x01, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
	uint8_t ackbuff[7];
	readdata(ackbuff, 6);
	return (0 == strncmp((char *)ackbuff, (char *)pn532ack, 7));
}
void setUART_PN532(UART_HandleTypeDef *huart)
{
	huartP=huart;
	}
