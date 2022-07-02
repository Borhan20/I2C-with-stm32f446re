#include <stdint.h>
#include "stm32f4xx.h"
void USART2_RX_INTERRUPT_CONFIG(void);
char USART2_GET_CHAR(void);
void USART2_SEND_CHAR(uint8_t c);
void USART2_SEND_STRING(uint8_t* c);
void USART2_SEND_INT(int n);
static void USART_SET_BAUDRATE(USART_TypeDef *USARTx, uint32_t PeriphClk,  uint32_t BaudRate);
static uint16_t COMPUTE_USART_BAUDRATE(uint32_t PeriphClk, uint32_t BaudRate);
