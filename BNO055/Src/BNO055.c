#include "main.h"
#include "BNO055.h"
#include <stdbool.h>



/*
BNO055::BNO055 (PinName p_sda, PinName p_scl, PinName p_reset) :
    _i2c(p_sda, p_scl), _res(p_reset)
{
    chip_addr = BNO055_G_CHIP_ADDR;
    chip_mode = MODE_NDOF;
    BNO055_initialize ();
}
 
BNO055::BNO055 (I2C& p_i2c, PinName p_reset, uint8_t addr, uint8_t mode) :
    _i2c(p_i2c), _res(p_reset)
{
    chip_addr = addr;
    chip_mode = mode;
    BNO055_initialize ();
}
 
BNO055::BNO055 (I2C& p_i2c, PinName p_reset) :
    _i2c(p_i2c), _res(p_reset)
{
    chip_addr = BNO055_G_CHIP_ADDR;
    chip_mode = MODE_NDOF;
    BNO055_initialize ();
}*/


void setI2CInterface_BNO055(I2C_HandleTypeDef *hi2c)
{
	hi2cLib=hi2c;
}
uint8_t WRITE_REGISTER_BNO055(uint8_t pData[], uint8_t length)
{
	uint8_t status=HAL_I2C_Master_Transmit(hi2cLib, BNO055_G_CHIP_ADDR<<1, pData, length, HAL_MAX_DELAY);
	return status;
}
uint8_t READ_REGISTER_BNO055(uint8_t buf[], uint8_t reg, uint8_t length)
{
	uint8_t status = HAL_I2C_Mem_Read(hi2cLib, BNO055_G_CHIP_ADDR<<1, reg, I2C_MEMADD_SIZE_8BIT, buf, length, HAL_MAX_DELAY);
	return status;
}

/////////////// Read data & normalize /////////////////////
void BNO055_get_Euler_Angles(BNO055_EULER_TypeDef *el)
{
    uint8_t deg_or_rad;
    int16_t h,p,r;
 
    BNO055_select_page(0);
    dt[0] = BNO055_UNIT_SEL;
    
    READ_REGISTER_BNO055(dt,BNO055_UNIT_SEL,1);
        
    if (dt[0] & 0x04) {
        deg_or_rad = 1; // Radian
    } else {
        deg_or_rad = 0; // Degree
    }
    
    dt[0] = BNO055_EULER_H_LSB;
    READ_REGISTER_BNO055(dt,BNO055_EULER_H_LSB,6);

    h = dt[1] << 8 | dt[0];
    p = dt[3] << 8 | dt[2];
    r = dt[5] << 8 | dt[4];
    if (deg_or_rad) {
        el->h = (double)h / 900;
        el->p = (double)p / 900;
        el->r = (double)r / 900;
    } else {
        el->h = (double)h / 16;
        el->p = (double)p / 16;
        el->r = (double)r / 16;
    }
}
 
void BNO055_get_quaternion(BNO055_QUATERNION_TypeDef *qua)
{
    BNO055_select_page(0);
    dt[0] = BNO055_QUATERNION_W_LSB;
    READ_REGISTER_BNO055(dt,BNO055_QUATERNION_W_LSB,8);
    
    qua->w = dt[1] << 8 | dt[0];
    qua->x = dt[3] << 8 | dt[2];
    qua->y = dt[5] << 8 | dt[4];
    qua->z = dt[7] << 8 | dt[6];
}
 
void BNO055_get_linear_accel(BNO055_LIN_ACC_TypeDef *la)
{
    uint8_t ms2_or_mg;
    int16_t x,y,z;
 
    BNO055_select_page(0);
    dt[0] = BNO055_UNIT_SEL;
    READ_REGISTER_BNO055(dt,BNO055_UNIT_SEL,1);
    
    if (dt[0] & 0x01) {
        ms2_or_mg = 1; // mg
    } else {
        ms2_or_mg = 0; // m/s*s
    }
    
    dt[0] = BNO055_LINEAR_ACC_X_LSB;
    READ_REGISTER_BNO055(dt,BNO055_LINEAR_ACC_X_LSB,6);
    
    x = dt[1] << 8 | dt[0];
    y = dt[3] << 8 | dt[2];
    z = dt[5] << 8 | dt[4];
    if (ms2_or_mg) {
        la->x = (double)x;
        la->y = (double)y;
        la->z = (double)z;
    } else {
        la->x = (double)x / 100;
        la->y = (double)y / 100;
        la->z = (double)z / 100;
    }
}
 
void BNO055_get_gravity(BNO055_GRAVITY_TypeDef *gr)
{
    uint8_t ms2_or_mg;
    int16_t x,y,z;
 
    BNO055_select_page(0);
    dt[0] = BNO055_UNIT_SEL;
    READ_REGISTER_BNO055(dt,BNO055_UNIT_SEL,1);
    if (dt[0] & 0x01) {
        ms2_or_mg = 1; // mg
    } else {
        ms2_or_mg = 0; // m/s*s
    }
    dt[0] = BNO055_GRAVITY_X_LSB;
    READ_REGISTER_BNO055(dt,BNO055_GRAVITY_X_LSB,6);
    
    x = dt[1] << 8 | dt[0];
    y = dt[3] << 8 | dt[2];
    z = dt[5] << 8 | dt[4];
    if (ms2_or_mg) {
        gr->x = (double)x;
        gr->y = (double)y;
        gr->z = (double)z;
    } else {
        gr->x = (double)x / 100;
        gr->y = (double)y / 100;
        gr->z = (double)z / 100;
    }
}
 
void BNO055_get_chip_temperature(BNO055_TEMPERATURE_TypeDef *tmp)
{
    uint8_t c_or_f;
 
    BNO055_select_page(0);
    dt[0] = BNO055_UNIT_SEL;
    READ_REGISTER_BNO055(dt,BNO055_UNIT_SEL,1);
    if (dt[0] & 0x10) {
        c_or_f = 1; // Fahrenheit
    } else {
        c_or_f = 0; // degrees Celsius
    }
    dt[0] = BNO055_TEMP_SOURCE;
    dt[1] = 0;

    WRITE_REGISTER_BNO055(dt,2);
    
    HAL_Delay(1); // Do I need to wait?
    dt[0] = BNO055_TEMP;
    READ_REGISTER_BNO055(dt,BNO055_TEMP,1);
    if (c_or_f) {
        tmp->acc_chip = (int8_t)dt[0] * 2;
    } else {
        tmp->acc_chip = (int8_t)dt[0];
    }
    dt[0] = BNO055_TEMP_SOURCE;
    dt[1] = 1;
    WRITE_REGISTER_BNO055(dt,2);
    HAL_Delay(1); // Do I need to wait?
    dt[0] = BNO055_TEMP;
    READ_REGISTER_BNO055(dt,BNO055_TEMP,1);
    if (c_or_f) {
        tmp->gyr_chip = (int8_t)dt[0] * 2;
    } else {
        tmp->gyr_chip = (int8_t)dt[0];
    }
}
 
/////////////// BNO055_initialize ////////////////////////////////
void BNO055_initialize (void)
{
    //chip_addr = BNO055_G_CHIP_ADDR;
    //chip_mode = MODE_NDOF;
  
    page_flag = 0xff;
    BNO055_select_page(0);
    // Check Acc & Mag & Gyro are available of not
    BNO055_check_id();
    // Set initial data
    BNO055_set_initial_dt_to_regs();
    // Unit selection
    BNO055_unit_selection();
    // Set fusion mode
    //BNO055_change_fusion_mode(chip_mode);
    BNO055_change_fusion_mode(MODE_NDOF);

}
 
void BNO055_unit_selection(void)
{
    BNO055_select_page(0);
    dt[0] = BNO055_UNIT_SEL;
    dt[1] = UNIT_ORI_WIN + UNIT_ACC_MSS + UNIT_GYR_DPS + UNIT_EULER_DEG + UNIT_TEMP_C;
    WRITE_REGISTER_BNO055(dt,2);
}
 
uint8_t BNO055_select_page(uint8_t page)
{
    if (page != page_flag){
        dt[0] = BNO055_PAGE_ID;
        if (page == 1) {
            dt[1] = 1;  // select page 1
        } else {
            dt[1] = 0;  // select page 0
        }
        WRITE_REGISTER_BNO055(dt,2);
        dt[0] = BNO055_PAGE_ID;
        READ_REGISTER_BNO055(dt,BNO055_PAGE_ID,1);
        page_flag = dt[0];
    }
    return page_flag;
}
 
uint8_t BNO055_reset(void)
{
     /*_res = 0;
     HAL_Delay(1);   // Reset 1mS
     _res = 1;
     wait(0.7);  // Need to wait at least 650mS*/
     
    page_flag = 0xff;
    BNO055_select_page(0);
    BNO055_check_id();
    if (chip_id != I_AM_BNO055_CHIP){
        return 1;
    } else {
        BNO055_initialize();
        return 0;
    }
}
 
////// Set BNO055_initialize data to related registers ///////////
void BNO055_set_initial_dt_to_regs(void)
{
    // BNO055_select_page(0);
    // current setting is only used default values
}
 
/////////////// Check Who am I? ///////////////////////////
void BNO055_check_id(void)
{
    BNO055_select_page(0);
    // ID
    dt[0] = BNO055_CHIP_ID;
    READ_REGISTER_BNO055(dt,BNO055_CHIP_ID,7);
    chip_id = dt[0];
    if (chip_id == I_AM_BNO055_CHIP) {
        ready_flag = 1;
    } else {
        ready_flag = 0;
    }
    acc_id = dt[1];
    if (acc_id == I_AM_BNO055_ACC) {
        ready_flag |= 2;
    }
    mag_id = dt[2];
    if (mag_id == I_AM_BNO055_MAG) {
        ready_flag |= 4;
    }
    gyr_id = dt[3];
    if (mag_id == I_AM_BNO055_MAG) {
        ready_flag |= 8;
    }
    bootldr_rev_id = dt[5]<< 8 | dt[4];
    sw_rev_id = dt[6];
}
 
void BNO055_read_id_inf(BNO055_ID_INF_TypeDef *id)
{
    id->chip_id = chip_id;
    id->acc_id = acc_id;
    id->mag_id = mag_id;
    id->gyr_id = gyr_id;
    id->bootldr_rev_id = bootldr_rev_id;
    id->sw_rev_id = sw_rev_id;
}
 
/////////////// Check chip ready or not  //////////////////
uint8_t BNO055_chip_ready(void)
{
    if (ready_flag == 0x0f) {
        return 1;
    }
    return 0;
}
 
/////////////// Read Calibration status  //////////////////
uint8_t BNO055_read_calib_status(void)
{
    BNO055_select_page(0);
    dt[0] = BNO055_CALIB_STAT;
    READ_REGISTER_BNO055(dt,BNO055_CALIB_STAT,1);
    return dt[0];
}
 
/////////////// Change Fusion mode  ///////////////////////
void BNO055_change_fusion_mode(uint8_t mode)
{
    uint8_t current_mode;
 
    BNO055_select_page(0);
    current_mode = BNO055_check_operating_mode();
    switch (mode) {
        case CONFIGMODE:
            dt[0] = BNO055_OPR_MODE;
            dt[1] = mode;
            WRITE_REGISTER_BNO055(dt,2);
            HAL_Delay(19);    // wait 19mS
            break;
        case MODE_IMU:
        case MODE_COMPASS:
        case MODE_M4G:
        case MODE_NDOF_FMC_OFF:
        case MODE_NDOF:
            if (current_mode != CONFIGMODE) {   // Can we change the mode directry?
                dt[0] = BNO055_OPR_MODE;
                dt[1] = CONFIGMODE;
                WRITE_REGISTER_BNO055(dt,2);
                HAL_Delay(19);    // wait 19mS
            }
            dt[0] = BNO055_OPR_MODE;
            dt[1] = mode;
            WRITE_REGISTER_BNO055(dt,2);
            HAL_Delay(7);    // wait 7mS
            break;
        default:
            break;
    }
}
 
uint8_t BNO055_check_operating_mode(void)
{
    BNO055_select_page(0);
    dt[0] = BNO055_OPR_MODE;
    READ_REGISTER_BNO055(dt,BNO055_OPR_MODE,1);
    return dt[0];
}
 
/////////////// Set Mouting position  /////////////////////
void BNO055_set_mounting_position(uint8_t position)
{
    uint8_t remap_config;
    uint8_t remap_sign;
    uint8_t current_mode;
 
    current_mode = BNO055_check_operating_mode();
    BNO055_change_fusion_mode(CONFIGMODE);
    switch (position) {
        case MT_P0:
            remap_config = 0x21;
            remap_sign = 0x04;
            break;
        case MT_P2:
            remap_config = 0x24;
            remap_sign = 0x06;
            break;
        case MT_P3:
            remap_config = 0x21;
            remap_sign = 0x02;
            break;
        case MT_P4:
            remap_config = 0x24;
            remap_sign = 0x03;
            break;
        case MT_P5:
            remap_config = 0x21;
            remap_sign = 0x01;
            break;
        case MT_P6:
            remap_config = 0x21;
            remap_sign = 0x07;
            break;
        case MT_P7:
            remap_config = 0x24;
            remap_sign = 0x05;
            break;
        case MT_P1:
        default:
            remap_config = 0x24;
            remap_sign = 0x00;
            break;
    }
    dt[0] = BNO055_AXIS_MAP_CONFIG;
    dt[1] = remap_config;
    dt[2] = remap_sign;
    WRITE_REGISTER_BNO055(dt,3);
    BNO055_change_fusion_mode(current_mode);
}
 
/////////////// I2C Freq. /////////////////////////////////
/*void BNO055_frequency(int hz)
{
    _i2c.frequency(hz);
}*/
 
/////////////// Read/Write specific register //////////////
uint8_t BNO055_read_reg0(uint8_t addr)
{
    BNO055_select_page(0);
    dt[0] = addr;

    READ_REGISTER_BNO055(dt,addr,1);
    return (uint8_t)dt[0];
}
 
uint8_t BNO055_write_reg0(uint8_t addr, uint8_t data)
{
    uint8_t current_mode;
    uint8_t d;
 
    current_mode = BNO055_check_operating_mode();
    BNO055_change_fusion_mode(CONFIGMODE);
    dt[0] = addr;
    dt[1] = data;
    WRITE_REGISTER_BNO055(dt,2);
    d = dt[0];
    BNO055_change_fusion_mode(current_mode);
    return d;
}
 
uint8_t BNO055_read_reg1(uint8_t addr)
{
    BNO055_select_page(1);
    dt[0] = addr;
    READ_REGISTER_BNO055(dt,addr,1);
    return (uint8_t)dt[0];
}
 
uint8_t BNO055_write_reg1(uint8_t addr, uint8_t data)
{
    uint8_t current_mode;
    uint8_t d;
 
    current_mode = BNO055_check_operating_mode();
    BNO055_change_fusion_mode(CONFIGMODE);
    BNO055_select_page(1);
    dt[0] = addr;
    dt[1] = data;
    WRITE_REGISTER_BNO055(dt,2);
    d = dt[0];
    BNO055_change_fusion_mode(current_mode);
    return d;
}
 
/* USER CODE END 0 */

