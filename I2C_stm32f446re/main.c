#include <stdio.h>
#include <string.h>
#include <math.h>
#include "clock.h"
#include "timer.h"
#include "uart.h"
#include "i2c.h"
#include "bmp.h"
#include "lcd.h"


int main(void)
{
	SysClockConfig();
	TIM6Config();
	USART2_RX_INTERRUPT_CONFIG();
	I2C_INIT();
	BMP280Config(OSRS_2, OSRS_16, OSRS_1, MODE_NORMAL, T_SB_0p5, IIR_16);
	USART2_SEND_STRING((uint8_t *) "START\n");
	lcd_init ();

	while(1){	
  lcd_put_cur(0, 0);
  lcd_send_string("HELLO ");
  lcd_send_string("WORLD ");
	
		char *res = BME280_Measure();
		USART2_SEND_STRING((uint8_t *) res);
		USART2_SEND_STRING((uint8_t *) "\n");
		Delay_ms(2000);
	  lcd_clear();
	}
}





	
	
	
	
	
	
	
	



