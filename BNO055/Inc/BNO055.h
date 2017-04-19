#include "stm32l1xx_hal.h"

#ifndef __BNO055_H
#define __BNO055_H

//  BNO055
//  7bit address = 0b010100x(0x28 or 0x29 depends on COM3)
#define BNO055_G_CHIP_ADDR      (0x28 << 1) // COM3 = GND
#define BNO055_V_CHIP_ADDR      (0x29 << 1) // COM3 = Vdd
 
// Fusion mode
#define CONFIGMODE              0x00
#define MODE_IMU                0x08
#define MODE_COMPASS            0x09
#define MODE_M4G                0x0a
#define MODE_NDOF_FMC_OFF       0x0b
#define MODE_NDOF               0x0c
 
//  UNIT
#define UNIT_ACC_MSS            0x00    // acc m/s2
#define UNIT_ACC_MG             0x01    // acc mg
#define UNIT_GYR_DPS            0x00    // gyro Dps
#define UNIT_GYR_RPS            0x02    // gyro Rps
#define UNIT_EULER_DEG          0x00    // euler Degrees
#define UNIT_EULER_RAD          0x04    // euler Radians
#define UNIT_TEMP_C             0x00    // temperature degC
#define UNIT_TEMP_F             0x10    // temperature degF
#define UNIT_ORI_WIN            0x00    // Windows orientation
#define UNIT_ORI_ANDROID        0x80    // Android orientation
 
//  ID's
#define I_AM_BNO055_CHIP        0xa0    // CHIP ID
#define I_AM_BNO055_ACC         0xfb    // ACC ID
#define I_AM_BNO055_MAG         0x32    // MAG ID
#define I_AM_BNO055_GYR         0x0f    // GYR ID

//---------------------------------------------------------
//----- Register's definition -----------------------------
//---------------------------------------------------------
// Page id register definition
#define BNO055_PAGE_ID          0x07
 
//----- page0 ---------------------------------------------
#define BNO055_CHIP_ID          0x00
#define BNO055_ACCEL_REV_ID     0x01
#define BNO055_MAG_REV_ID       0x02
#define BNO055_GYRO_REV_ID      0x03
#define BNO055_SW_REV_ID_LSB    0x04
#define BNO055_SW_REV_ID_MSB    0x05
#define BNO055_BL_REV_ID        0x06
 
// Accel data register*/
#define BNO055_ACC_X_LSB        0x08
#define BNO055_ACC_X_MSB        0x09
#define BNO055_ACC_Y_LSB        0x0a
#define BNO055_ACC_Y_MSB        0x0b
#define BNO055_ACC_Z_LSB        0x0c
#define BNO055_ACC_Z_MSB        0x0d
 
// Mag data register
#define BNO055_MAG_X_LSB        0x0e
#define BNO055_MAG_X_MSB        0x0f
#define BNO055_MAG_Y_LSB        0x10
#define BNO055_MAG_Y_MSB        0x11
#define BNO055_MAG_Z_LSB        0x12
#define BNO055_MAG_Z_MSB        0x13
 
// Gyro data registers
#define BNO055_GYR_X_LSB        0x14
#define BNO055_GYR_X_MSB        0x15
#define BNO055_GYR_Y_LSB        0x16
#define BNO055_GYR_Y_MSB        0x17
#define BNO055_GYR_Z_LSB        0x18
#define BNO055_GYR_Z_MSB        0x19
 
// Euler data registers
#define BNO055_EULER_H_LSB      0x1a
#define BNO055_EULER_H_MSB      0x1b
 
#define BNO055_EULER_R_LSB      0x1c
#define BNO055_EULER_R_MSB      0x1d
 
#define BNO055_EULER_P_LSB      0x1e
#define BNO055_EULER_P_MSB      0x1f
 
// Quaternion data registers
#define BNO055_QUATERNION_W_LSB 0x20
#define BNO055_QUATERNION_W_MSB 0x21
#define BNO055_QUATERNION_X_LSB 0x22
#define BNO055_QUATERNION_X_MSB 0x23
#define BNO055_QUATERNION_Y_LSB 0x24
#define BNO055_QUATERNION_Y_MSB 0x25
#define BNO055_QUATERNION_Z_LSB 0x26
#define BNO055_QUATERNION_Z_MSB 0x27
 
// Linear acceleration data registers
#define BNO055_LINEAR_ACC_X_LSB 0x28
#define BNO055_LINEAR_ACC_X_MSB 0x29
#define BNO055_LINEAR_ACC_Y_LSB 0x2a
#define BNO055_LINEAR_ACC_Y_MSB 0x2b
#define BNO055_LINEAR_ACC_Z_LSB 0x2c
#define BNO055_LINEAR_ACC_Z_MSB 0x2d
 
// Gravity data registers
#define BNO055_GRAVITY_X_LSB    0x2e
#define BNO055_GRAVITY_X_MSB    0x2f
#define BNO055_GRAVITY_Y_LSB    0x30
#define BNO055_GRAVITY_Y_MSB    0x31
#define BNO055_GRAVITY_Z_LSB    0x32
#define BNO055_GRAVITY_Z_MSB    0x33
 
// Temperature data register
#define BNO055_TEMP             0x34
 
// Status registers
#define BNO055_CALIB_STAT       0x35
#define BNO055_SELFTEST_RESULT  0x36
#define BNO055_INTR_STAT        0x37
#define BNO055_SYS_CLK_STAT     0x38
#define BNO055_SYS_STAT         0x39
#define BNO055_SYS_ERR          0x3a
 
// Unit selection register
#define BNO055_UNIT_SEL         0x3b
#define BNO055_DATA_SELECT      0x3c
 
// Mode registers
#define BNO055_OPR_MODE         0x3d
#define BNO055_PWR_MODE         0x3e
#define BNO055_SYS_TRIGGER      0x3f
#define BNO055_TEMP_SOURCE      0x40
 
// Axis remap registers
#define BNO055_AXIS_MAP_CONFIG  0x41
#define BNO055_AXIS_MAP_SIGN    0x42
 
// SIC registers
#define BNO055_SIC_MTRX_0_LSB   0x43
#define BNO055_SIC_MTRX_0_MSB   0x44
#define BNO055_SIC_MTRX_1_LSB   0x45
#define BNO055_SIC_MTRX_1_MSB   0x46
#define BNO055_SIC_MTRX_2_LSB   0x47
#define BNO055_SIC_MTRX_2_MSB   0x48
#define BNO055_SIC_MTRX_3_LSB   0x49
#define BNO055_SIC_MTRX_3_MSB   0x4a
#define BNO055_SIC_MTRX_4_LSB   0x4b
#define BNO055_SIC_MTRX_4_MSB   0x4c
#define BNO055_SIC_MTRX_5_LSB   0x4d
#define BNO055_SIC_MTRX_5_MSB   0x4e
#define BNO055_SIC_MTRX_6_LSB   0x4f
#define BNO055_SIC_MTRX_6_MSB   0x50
#define BNO055_SIC_MTRX_7_LSB   0x51
#define BNO055_SIC_MTRX_7_MSB   0x52
#define BNO055_SIC_MTRX_8_LSB   0x53
#define BNO055_SIC_MTRX_8_MSB   0x54
 
// Accelerometer Offset registers
#define ACCEL_OFFSET_X_LSB      0x55
#define ACCEL_OFFSET_X_MSB      0x56
#define ACCEL_OFFSET_Y_LSB      0x57
#define ACCEL_OFFSET_Y_MSB      0x58
#define ACCEL_OFFSET_Z_LSB      0x59
#define ACCEL_OFFSET_Z_MSB      0x5a
 
// Magnetometer Offset registers
#define MAG_OFFSET_X_LSB        0x5b
#define MAG_OFFSET_X_MSB        0x5c
#define MAG_OFFSET_Y_LSB        0x5d
#define MAG_OFFSET_Y_MSB        0x5e
#define MAG_OFFSET_Z_LSB        0x5f
#define MAG_OFFSET_Z_MSB        0x60
 
// Gyroscope Offset registers
#define GYRO_OFFSET_X_LSB       0x61
#define GYRO_OFFSET_X_MSB       0x62
#define GYRO_OFFSET_Y_LSB       0x63
#define GYRO_OFFSET_Y_MSB       0x64
#define GYRO_OFFSET_Z_LSB       0x65
#define GYRO_OFFSET_Z_MSB       0x66
 
// Radius registers
#define ACCEL_RADIUS_LSB        0x67
#define ACCEL_RADIUS_MSB        0x68
#define MAG_RADIUS_LSB          0x69
#define MAG_RADIUS_MSB          0x6a
 
//----- page1 ---------------------------------------------
// Configuration registers
#define ACCEL_CONFIG            0x08
#define MAG_CONFIG              0x09
#define GYRO_CONFIG             0x0a
#define GYRO_MODE_CONFIG        0x0b
#define ACCEL_SLEEP_CONFIG      0x0c
#define GYRO_SLEEP_CONFIG       0x0d
#define MAG_SLEEP_CONFIG        0x0e
 
// Interrupt registers
#define INT_MASK                0x0f
#define INT                     0x10
#define ACCEL_ANY_MOTION_THRES  0x11
#define ACCEL_INTR_SETTINGS     0x12
#define ACCEL_HIGH_G_DURN       0x13
#define ACCEL_HIGH_G_THRES      0x14
#define ACCEL_NO_MOTION_THRES   0x15
#define ACCEL_NO_MOTION_SET     0x16
#define GYRO_INTR_SETING        0x17
#define GYRO_HIGHRATE_X_SET     0x18
#define GYRO_DURN_X             0x19
#define GYRO_HIGHRATE_Y_SET     0x1a
#define GYRO_DURN_Y             0x1b
#define GYRO_HIGHRATE_Z_SET     0x1c
#define GYRO_DURN_Z             0x1d
#define GYRO_ANY_MOTION_THRES   0x1e
#define GYRO_ANY_MOTION_SET     0x1f
