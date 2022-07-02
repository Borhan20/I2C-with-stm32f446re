#include "lcd.h"
#include "timer.h"
#include "uart.h"

void send_to_lcd (char data, int rs)
{
	RS_GPIO_Port->ODR |= (rs << RS_Pin);
	/* write the data to the respective pin */
	D7_GPIO_Port->ODR |=(((data>>3)&0x01)<< D7_Pin);
	D6_GPIO_Port->ODR |=(((data>>2)&0x01)<< D6_Pin);
	D5_GPIO_Port->ODR |=(((data>>1)&0x01)<< D5_Pin);
	D4_GPIO_Port->ODR |=(((data>>0)&0x01)<< D4_Pin);


	/* Toggle EN PIN to send the data
	 * if the HCLK > 100 MHz, use the  20 us delay
	 * if the LCD still doesn't work, increase the delay to 50, 80 or 100..
	 */
	EN_GPIO_Port->ODR |= (1U << EN_Pin);
	Delay_us (50);
	EN_GPIO_Port->ODR |= (0 << EN_Pin);
	Delay_us (50);
}

void lcd_send_cmd (char cmd)
{
    char datatosend;

    /* send upper nibble first */
    datatosend = ((cmd>>4)&0x0f);
    send_to_lcd(datatosend,0);  // RS must be 0 while sending command

    /* send Lower Nibble */
    datatosend = ((cmd)&0x0f);
	send_to_lcd(datatosend, 0);
}

void lcd_send_data (char data)
{
	char datatosend;

	/* send higher nibble */
	datatosend = ((data>>4)&0x0f);
	send_to_lcd(datatosend, 1);  // rs =1 for sending data

	/* send Lower nibble */
	datatosend = ((data)&0x0f);
	send_to_lcd(datatosend, 1);
}

void lcd_clear (void)
{
	lcd_send_cmd(0x01);
	Delay_ms(2);
}

void lcd_put_cur(int row, int col)
{
    switch (row)
    {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0xC0;
            break;
    }

    lcd_send_cmd (col);
}


void lcd_init (void)
{
	RCC->AHB1ENR|=RCC_AHB1ENR_GPIOBEN;
	RCC->AHB1ENR|=RCC_AHB1ENR_GPIOCEN;
	GPIOC->MODER|= (1U<<0)|(1U<<2);
	GPIOB->MODER|= (1U<<0)|(1U<<6)|(1U<<8)|(1U<<10)|(1U<<20);
	
	// 4 bit initialisation
	Delay_ms(50);  // wait for >40ms
	lcd_send_cmd (0x30);
	Delay_ms(5);  // wait for >4.1ms
	lcd_send_cmd (0x30);
	Delay_ms(1);  // wait for >100us
	lcd_send_cmd (0x30);
	Delay_ms(10);
	lcd_send_cmd (0x20);  // 4bit mode
	Delay_ms(10);

  // display initialisation
	lcd_send_cmd (0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	Delay_ms(1);
	lcd_send_cmd (0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	Delay_ms(1);
	lcd_send_cmd (0x01);  // clear display
	Delay_ms(1);
	Delay_ms(1);
	lcd_send_cmd (0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	Delay_ms(1);
	lcd_send_cmd (0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
}

void lcd_send_string (char *str)
{
	while (*str) lcd_send_data (*str++);
}
