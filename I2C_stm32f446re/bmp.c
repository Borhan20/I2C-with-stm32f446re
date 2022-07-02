#include "stm32f446xx.h"
#include "bmp.h"
#include "i2c.h"
#include "timer.h"
#include <stdio.h>


uint8_t chipID;

uint8_t TrimParam[36];
int32_t tRaw, pRaw, hRaw;

uint16_t dig_T1,dig_P1,dig_H1, dig_H3;

int16_t  dig_T2, dig_T3,dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9, dig_H2,  dig_H4, dig_H5, dig_H6;

#define dev_address 0x76

float Temperature, Pressure, Humidity;


void TrimRead(void)
{
	uint8_t trimdata[32];
	I2C_READ(dev_address,0x88,25,trimdata);
	Delay_ms(100);
	// Read NVM from 0x88 to 0xA1
	uint8_t temp[8];
	while(I2C1 -> SR2 & I2C_SR2_BUSY);
	I2C_READ(dev_address,0xE1,7,(uint8_t *)trimdata+25);
	Delay_ms(100);
	// Arrange the data as per the datasheet
	dig_T1 = (trimdata[1]<<8) | trimdata[0];
	dig_T2 = (trimdata[3]<<8) | trimdata[2];
	dig_T3 = (trimdata[5]<<8) | trimdata[4];
	dig_P1 = (trimdata[7]<<8) | trimdata[5];
	dig_P2 = (trimdata[9]<<8) | trimdata[6];
	dig_P3 = (trimdata[11]<<8) | trimdata[10];
	dig_P4 = (trimdata[13]<<8) | trimdata[12];
	dig_P5 = (trimdata[15]<<8) | trimdata[14];
	dig_P6 = (trimdata[17]<<8) | trimdata[16];
	dig_P7 = (trimdata[19]<<8) | trimdata[18];
	dig_P8 = (trimdata[21]<<8) | trimdata[20];
	dig_P9 = (trimdata[23]<<8) | trimdata[22];
	dig_H1 = trimdata[24];
	dig_H2 = (trimdata[26]<<8) | trimdata[25];
	dig_H3 = (trimdata[27]);
	dig_H4 = (trimdata[28]<<4) | (trimdata[29] & 0x0f);
	dig_H5 = (trimdata[30]<<4) | (trimdata[29]>>4);
	dig_H6 = (trimdata[31]);
	char res[64];
	//snprintf(res, sizeof res, "Temp: %d Press: %d Hum: %d\n", dig_T1, dig_T2, dig_T3);
	//USART2_SEND_STRING((uint8_t *)res);
	Delay_ms(100);
			
}

void BMP280Config(uint8_t osrs_t, uint8_t osrs_p, uint8_t osrs_h, uint8_t mode, uint8_t t_sb, uint8_t filter)
{
	TrimRead();
	uint8_t datatowrite = 0;
	uint8_t datacheck = 0;

	// Reset the device
	datatowrite = 0xB6;
	I2C_WRITE(dev_address,RESET_REG,datatowrite);
	Delay_ms(100);
	datatowrite = osrs_h;
	I2C_WRITE(dev_address,CTRL_HUM_REG,datatowrite);
	Delay_ms(100);
	datatowrite = (t_sb <<5) |(filter << 2);
	I2C_WRITE(dev_address,CONFIG_REG,datatowrite);
	Delay_ms(100);
	datatowrite = (osrs_t <<5) |(osrs_p << 2) | mode;
	I2C_WRITE(dev_address,CTRL_MEAS_REG,datatowrite);
	Delay_ms(100);
	
}

void BMEReadRaw(void)
{
	uint8_t RawData[8];
	// Read the Registers 0xF7 to 0xFE
	I2C_READ(dev_address,PRESS_MSB_REG,8,RawData);
		/* Calculate the Raw data for the parameters
		 * Here the Pressure and Temperature are in 20 bit format and humidity in 16 bit format*/
	pRaw = (RawData[0]<<12)|(RawData[1]<<4)|(RawData[2]>>4);
	tRaw = (RawData[3]<<12)|(RawData[4]<<4)|(RawData[5]>>4);
	hRaw = (RawData[6]<<8)|(RawData[7]);
	char tmp[10];
	//sprintf(tmp,"%s %d %d","nn ",tRaw,tRaw);
	//USART2_SEND_STRING((uint8_t *) tmp);
}



int32_t t_fine;
int32_t BMP280_compensate_T_int32(int32_t adc_T)
{
	
	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1)))>> 12) *((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}


// Returns pressure in Pa as unsigned 32 bit integer. Output value of �96386� equals 96386 Pa = 963.86 hPa
uint32_t BMP280_compensate_P_int32(int32_t adc_P)
{
	int32_t var1, var2;
	uint32_t p;
	var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)dig_P6);
	var2 = var2 + ((var1*((int32_t)dig_P5))<<1);
	var2 = (var2>>2)+(((int32_t)dig_P4)<<16);
	var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)dig_P2) *var1)>>1))>>18;
	var1 =((((32768+var1))*((int32_t)dig_P1))>>15);
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
	if (p < 0x80000000)
	{
		p = (p << 1) / ((uint32_t)var1);
	}
	else
	{
		p = (p / (uint32_t)var1) * 2;
	}
	var1 = (((int32_t)dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((int32_t)(p>>2)) * ((int32_t)dig_P8))>>13;
	p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));
	return p;
}


uint32_t bmp280_compensate_H_int32(int32_t adc_H)
{
	int32_t v_x1_u32r;
	v_x1_u32r = (t_fine - ((int32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) *\
			v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r *\
					((int32_t)dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) +\
							((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dig_H2) +\
					8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	return (uint32_t)(v_x1_u32r>>12);
}




double bmp280_compensate_H_double(uint32_t adc_H)
{
	uint32_t v_x1_u32r;
	v_x1_u32r = (t_fine - ((uint32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((uint32_t)dig_H4) << 20) - (((uint32_t)dig_H5) *
		v_x1_u32r)) + ((uint32_t)16384)) >> 15) * (((((((v_x1_u32r *
		((uint32_t)dig_H6)) >> 10) * (((v_x1_u32r * ((uint32_t)dig_H3)) >> 11) +
		((uint32_t)32768))) >> 10) + ((uint32_t)2097152)) * ((uint32_t)dig_H2) +
		8192) >> 14));
		v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
		((uint32_t)dig_H1)) >> 4));
	return (uint32_t) v_x1_u32r;
		//v_x1_u32r = ((v_x1_u32r < 0) ? 0 : v_x1_u32r);
		//v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
		//return (uint32_t)(v_x1_u32r>>12);
}






char* BME280_Measure (void)
{
	BMEReadRaw();
			double temp;
		  if (tRaw == 0x800000) Temperature = 0; // value in case temp measurement was disabled
		  else
		  {
			  Temperature = (BMP280_compensate_T_int32 (tRaw))/100.0;  // as per datasheet, the temp is x100
		  }

		  if (pRaw == 0x800000) Pressure = 0; // value in case temp measurement was disabled
		  else
		  {
			  Pressure = (BMP280_compensate_P_int32 (pRaw));  // as per datasheet, the pressure is Pa
		  }

		  if (hRaw == 0x800000) Humidity = 0; // value in case temp measurement was disabled
		  else
		  {
			  Humidity = (bmp280_compensate_H_double (hRaw))/1024.0;  // as per datasheet, the temp is x1024
		  }
			char res[64];
			snprintf(res, sizeof res, "Temp: %.2f Press: %.2f \n", Temperature, Pressure);
			return res;

			
}