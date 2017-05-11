#ifndef RPI_3_SENSOR_SHIELD_H_
#define RPI_3_SENSOR_SHIELD_H_

#define MPL3115A2_ADDRESS                0x60
#define MPL3115A2_PT_DATA_CFG            0x13
#define MPL3115A2_CTRL_REG1              0x26
#define MPL3115A2_STATUS                 0x00

#define SI7021_ADDRESS                   0x40
#define SI7021_MEASRH_HOLD_CMD           0xE5
#define SI7021_MEASRH_NOHOLD_CMD         0xF5
#define SI7021_MEASTEMP_HOLD_CMD         0xE3
#define SI7021_MEASTEMP_NOHOLD_CMD       0xF3
#define SI7021_READPREVTEMP_CMD          0xE0
#define SI7021_RESET_CMD                 0xFE
#define SI7021_WRITERHT_REG_CMD          0xE6
#define SI7021_READRHT_REG_CMD           0xE7
#define SI7021_WRITEHEATER_REG_CMD       0x51
#define SI7021_READHEATER_REG_CMD        0x11
#define SI7021_ID1_CMD                   0xFA0F
#define SI7021_ID2_CMD                   0xFCC9
#define SI7021_FIRMVERS_CMD              0x84B8

void readmpl31152a(void);
void readHumidity(void);

#endif
