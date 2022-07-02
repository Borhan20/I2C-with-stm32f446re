
#include "stm32f4xx.h"
/*********** Define the LCD PINS below ****************/

#define RS_Pin 0
#define RS_GPIO_Port GPIOC
#define RW_Pin 1
#define RW_GPIO_Port GPIOC
#define EN_Pin 0
#define EN_GPIO_Port GPIOB
#define D4_Pin 3
#define D4_GPIO_Port GPIOB
#define D5_Pin 5
#define D5_GPIO_Port GPIOB
#define D6_Pin 4
#define D6_GPIO_Port GPIOB
#define D7_Pin 10
#define D7_GPIO_Port GPIOB

void lcd_init (void);   // initialize lcd

void lcd_send_cmd (char cmd);  // send command to the lcd

void lcd_send_data (char data);  // send data to the lcd

void lcd_send_string (char *str);  // send string to the lcd

void lcd_put_cur(int row, int col);  // put cursor at the entered position row (0 or 1), col (0-15);

void lcd_clear (void);