
#include "MPL3115A2.h"

void setI2CInterface_MPL3115A2(I2C_HandleTypeDef *hi2c)
{
	hi2cLib=hi2c;
}
uint8_t WRITE_REGISTER_MPL3115A2(uint8_t pData[],uint8_t length)
{
	uint8_t status=HAL_I2C_Master_Transmit(hi2cLib, MPL3115A2_ADDRESS<<1, pData,length, HAL_MAX_DELAY);
	return status;
}
uint8_t READ_REGISTER_MPL3115A2(uint8_t buf[],uint8_t reg,uint8_t length)
{
	uint8_t status = HAL_I2C_Mem_Read(hi2cLib, MPL3115A2_ADDRESS<<1, reg, I2C_MEMADD_SIZE_8BIT, buf, length, HAL_MAX_DELAY);
	return status;
}

/*
uint8_t ReadBar_MPL3115A2(uint32_t *data)
{
	uint8_t status=0;
	uint8_t msb[1];
	uint8_t csb[1];
	uint8_t lsb[1];
	READ_REGISTER_MPL3115A2(msb,OUT_P_MSB,1);
	READ_REGISTER_MPL3115A2(csb,OUT_P_CSB,1);
	status=READ_REGISTER_MPL3115A2(lsb,OUT_P_LSB,1);
	*data=msb[0]<<16|csb[0]<<8|lsb[0];
	return status;
}
*/
uint8_t ReadBar_MPL3115A2_v2(uint32_t *data)
{
	uint8_t status=0;
	uint8_t bar[3];
	READ_REGISTER_MPL3115A2(bar,OUT_P_MSB,3);
	*data=bar[0]<<16|bar[1]<<8|bar[2];
	return status;
}

int parseBar_MPL3115A2(uint32_t data)
{
	uint32_t alt_m = data >> 6;
	uint32_t alt_l = data & 0x30;
	return alt_m + alt_l / 64.0;
}
uint8_t newValue_MPL3115A2()
{
	uint8_t c[1];
	READ_REGISTER_MPL3115A2(c,INT_SOURCE,1);
	if((c[0] && (0x80)))
		return 1;
	else
		return 0;
}

void Init_Bar_int_MPL3115A2()
{
	uint8_t who[1];
	READ_REGISTER_MPL3115A2(who,WHO_AM_I,1);
	if (who[0] == 0xc4)
	{
		uint8_t a[2]={CTRL_REG1,0x38};
		uint8_t b[2]={PT_DATA_CFG,0x07};
		uint8_t c[2]={CTRL_REG3,0x11};
		uint8_t d[2]={CTRL_REG4,0x80};
		uint8_t e[2]={CTRL_REG1,0x39};
		WRITE_REGISTER_MPL3115A2(a,2);
		WRITE_REGISTER_MPL3115A2(b,2);
		WRITE_REGISTER_MPL3115A2(c,2);
		WRITE_REGISTER_MPL3115A2(d,2);
		WRITE_REGISTER_MPL3115A2(e,2);
	}
}

void Init_Bar_MPL3115A2()
{
	uint8_t who[1];
	READ_REGISTER_MPL3115A2(who,WHO_AM_I,1);
	if (who[0] == 0xc4)
	{
		uint8_t a[2]={0x26, 0x38};
		uint8_t b[2]={0x27, 0x00};
		uint8_t c[2]={0x28, 0x11};
		uint8_t d[2]={0x29, 0x00};
		uint8_t e[2]={0x2a, 0x00};
		uint8_t f[2]={0x13, 0x07};
		WRITE_REGISTER_MPL3115A2(a,2);
		WRITE_REGISTER_MPL3115A2(b,2);
		WRITE_REGISTER_MPL3115A2(c,2);
		WRITE_REGISTER_MPL3115A2(d,2);
		WRITE_REGISTER_MPL3115A2(e,2);
		WRITE_REGISTER_MPL3115A2(f,2);
		Active_MPL3115A2();
	}
}
void Active_MPL3115A2()
{
	uint8_t v[1];
	READ_REGISTER_MPL3115A2(v,0x26,1);
	v[0] = v[0] | 0x01;
	uint8_t v2[2]={0x26, v[0]};
	WRITE_REGISTER_MPL3115A2(v2,2);
}

void Standby_MPL3115A2()
{
	uint8_t v[1];
	READ_REGISTER_MPL3115A2(v,0x26,1);
	v[0] = v[0] & ~0x01;
	uint8_t v2[2]={0x26, v[0]};
	WRITE_REGISTER_MPL3115A2(v2,2);
}
uint16_t Read_Temp_MPL3115A() 
{
	uint16_t t = 0;
	uint8_t t1[1];
	READ_REGISTER_MPL3115A2(t1,0x04,1);
	uint8_t t2[1];
	READ_REGISTER_MPL3115A2(t2,0x05,1);
	t = (t1[0] << 8) | t2[0];
	return t;
}
double parseTemp_MPL3115A(uint16_t temp) 
{
	uint16_t t_m = (temp >> 8) & 0xFF;
	uint16_t t_l = temp & 0xFF;
	if (t_m > 0x7f) t_m = t_m - 256;
	return t_m + t_l / 256.0;
}