#ifndef PN532_H
#define PN532_H

#define PN532_PREAMBLE                      (0x00)
#define PN532_STARTCODE1                    (0x00)
#define PN532_STARTCODE2                    (0xFF)
#define PN532_POSTAMBLE                     (0x00)

#define PN532_HOSTTOPN532                   (0xD4)
#define PN532_PN532TOHOST                   (0xD5)
#define PN532_COMMAND_GETFIRMWAREVERSION    (0x02)
#define PN532_I2C_ADDRESS                   (0x48 >> 1)
#define PN532_I2C_READY                     (0x01)
#define PN532_MIFARE_ISO14443A              (0x00)
#define PN532_COMMAND_INLISTPASSIVETARGET   (0x4A)
#define PN532_COMMAND_SAMCONFIGURATION      (0x14)
#define PN532_PACKBUFFSIZ                     64

uint8_t pn532_packetbuffer[PN532_PACKBUFFSIZ];
uint8_t _irq, _reset;
uint8_t _uid[7];       // ISO14443A uid
uint8_t _uidLen;       // uid len
uint8_t _key[6];       // Mifare Classic key
uint8_t _inListedTag;  // Tg number of inlisted tag.
uint8_t writeBuffer[1];
  
uint8_t WRITE_REGISTER_PN532(uint8_t pData[],uint8_t length);  
uint8_t READ_REGISTER_PN532(uint8_t buf[],uint8_t reg,uint8_t length); 
void setI2CInterface_PN532(I2C_HandleTypeDef *hi2c);
uint8_t isready(void);
uint8_t waitready(uint16_t timeout);
void writecommand(uint8_t* cmd, uint8_t cmdlen);
void readdata(uint8_t buff[], uint8_t n); 
uint8_t readack2();
uint8_t sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout);
uint32_t getFirmwareVersion(void);
uint8_t SAMConfig(void);
//uint8_t readPassiveTargetID(uint8_t cardbaudrate, uint8_t * uid, uint8_t * uidLength, uint16_t timeout);
uint32_t readPassiveTargetID(uint8_t cardbaudrate, uint8_t * uid, uint8_t * uidLength, uint16_t timeout);
#endif