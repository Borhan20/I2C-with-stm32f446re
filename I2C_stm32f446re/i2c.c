#include "i2c.h"
#include <stdint.h>
#include <stm32f446xx.h>
#define GPIOBEN					(1U<<1)
#define I2C1EN					(1U<<21)
#define CR1_PE          (1U<<0)
#define SR2_BUSY				(1U<<1)
#define CR1_START       (1U<<8)
#define CR1_STOP				(1U<<9)
#define SR1_SB				  (1U<<0)
#define SR1_ADDR        (1U<<1)
#define SR1_TXE 				(1U<<7)
#define CR1_ACK					(1U<<10)
#define SR1_RXNE				(1U<<6)
#define	SR1_BTF	    		(1U<<2)


void I2C_INIT(void){
	//ENABLE CLOCK ACCESS TO GPIOB
	RCC->AHB1ENR |= GPIOBEN;
	//ENABLE CLOCK ACCESS TO I2C1
	RCC->APB1ENR |= I2C1EN;
	
	//CONFIGURE THE I2C PINS FOR ALTERNATE FUNCTIONS
	GPIOB->MODER |= (2<<16) | (2<<18);  // Bits (17:16)= 1:0 --> Alternate Function for Pin PB8; Bits (19:18)= 1:0 --> Alternate Function for Pin PB9
	GPIOB->OTYPER |= (1<<8) | (1<<9);  //  Bit8=1, Bit9=1  output open drain
	GPIOB->OSPEEDR |= (3<<16) | (3<<18);  // Bits (17:16)= 1:1 --> High Speed for PIN PB8; Bits (19:18)= 1:1 --> High Speed for PIN PB9
	GPIOB->PUPDR |= (1<<16) | (1<<18);  // Bits (17:16)= 0:1 --> Pull up for PIN PB8; Bits (19:18)= 0:1 --> pull up for PIN PB9
	GPIOB->AFR[1] |= (4<<0) | (4<<4);  // Bits (3:2:1:0) = 0:1:0:0 --> AF4 for pin PB8;  Bits (7:6:5:4) = 0:1:0:0 --> AF4 for pin PB9
	
	//SOFTWARE RESET FOR I2C1
	I2C1->CR1 |=(1U<<15);
	//COME OUT OF RESET MODE
	I2C1->CR1 &=~(1U<<15);
	
	//SET PERIPHERAL CLOCK FREQUENCY
	I2C1->CR2=(45<<0); //45 Mhz
	
	//CONFIGURE CLOCK CONTROL TEGISTER
	I2C1->CCR = (225<<0);
	//SET MAXIMUM RISE TIME
	I2C1->TRISE = 46;
	
	//ENABLE I2C1 MODULE
	I2C1->CR1 |= CR1_PE;
		
}

void I2C_READ_BYTE(char saddr, char maddr, uint8_t *data){
	volatile uint32_t temp;
	
	//WAIT UNTIL I2C BUS IS FREE
	while(I2C1->SR2 & SR2_BUSY){}
	
	//GENERATE START CONDITION
	I2C1->CR1 |= CR1_START;
	
	//WAIT UNTIL START FLAG IS SET
	while(!(I2C1->SR1 & SR1_SB)){}
		
	//TRANSMIT SLAVE ADDRESS + WRITE
	I2C1->DR = saddr << 1;
	
	//WAIT UNTIL ADDR FLAG IS SET
	while(!(I2C1->SR1 & SR1_ADDR)){}
	
	//READ SR2 TO CLEAR ADDR FLAG
	temp = I2C1->SR2;
	
	//SEND MEMORY ADDRESS TO READ FROM
	I2C1->DR = maddr;
		
	//WAIT UNTIL THE TRANSMISSION
	while(!(I2C1->SR1 & SR1_TXE)){}
		
	//GENERATE RESTART CONDITION
	I2C1->CR1 |= CR1_START;
	
	//WAIT UNTIL START FLAG IS SET
	while(!(I2C1->SR1 & SR1_SB)){}
		
	//TRANSMIT SLAVE ADDRESS + READ
	I2C1->DR = saddr << 1 | 1;
		
	//WAIT UNTIL ADDR FLAG IS SET
	while(!(I2C1->SR1 & SR1_ADDR)){}
		
	//NO ACKNOWLEDGE RETURNED
	I2C1->CR1 &= ~CR1_ACK;
	
	//READ SR2 TO CLEAR ADDR FLAG
	temp = I2C1->SR2;
		
	//GENERATE STOP AFTER DATA RECEIVED
	I2C1->CR1 |= CR1_STOP;
		
	//WAIT UNTIL RXNE IS SET
	while(!(I2C1->SR1 & SR1_RXNE)){}
	
	//READ DATA FROM DR
	*data++ = I2C1->DR;
		

}

void I2C_READ(char saddr, char maddr, int n, uint8_t *data){
	
	volatile uint32_t temp;
	//WAIT UNTIL I2C BUS IS FREE
	while(I2C1->SR2 & SR2_BUSY){}
	
	//GENERATE START CONDITION
	I2C1->CR1 |= CR1_START;
	
	//WAIT UNTIL START FLAG IS SET
	while(!(I2C1->SR1 & SR1_SB)){}
		
	//TRANSMIT SLAVE ADDRESS + WRITE
	I2C1->DR = saddr << 1;
	
	//WAIT UNTIL ADDR FLAG IS SET
	while(!(I2C1->SR1 & SR1_ADDR)){}
	//READ SR2 TO CLEAR ADDR FLAG
	temp = I2C1->SR2;
	
		
	//WAIT UNTIL THE TRANSMISSION
	while(!(I2C1->SR1 & SR1_TXE)){}
	
	//SEND MEMORY ADDRESS TO READ FROM
	I2C1->DR = maddr;
		
	//WAIT UNTIL THE TRANSMISSION
	while(!(I2C1->SR1 & SR1_TXE)){}
		
	//GENERATE RESTART CONDITION
	I2C1->CR1 |= CR1_START;
	//WAIT UNTIL START FLAG IS SET
	while(!(I2C1->SR1 & SR1_SB)){}
		
	//TRANSMIT SLAVE ADDRESS + READ
	I2C1->DR = saddr << 1 | 1;
		
	//WAIT UNTIL ADDR FLAG IS SET
	while(!(I2C1->SR1 & SR1_ADDR)){}
	//READ SR2 TO CLEAR ADDR FLAG
	temp = I2C1->SR2;
		
	//ACKNOWLEDGE RETURNED
	I2C1->CR1 |= CR1_ACK;
		

	while(n > 0)
			{
				//IF ONE BYTE
				if(n == 1)
				{
					// ACKNOWLEDGE NOT RETURNED
					I2C1->CR1 &= ~CR1_ACK;
					// GENERATE STOP CONDITION
					I2C1->CR1 |= CR1_STOP;

					// WAIT UNTIL RXNE IS NOT SET 
					while (!(I2C1->SR1 & SR1_RXNE)){}
					//READ DATA FROM DR
            *data++ = I2C1->DR;
            break;
				}
				else
				{
       	 // WAIT UNTIL RXNE IS NOT SET 
           while (!(I2C1->SR1 & SR1_RXNE)){}
           //READ DATA FROM DR
           (*data++) = I2C1->DR;
           n--;
				}
	}
}
	
void I2C_WRITE(char saddr, char maddr, uint8_t data){
	
	volatile uint32_t temp;
	//WAIT UNTIL I2C BUS IS FREE
	while(I2C1->SR2 & SR2_BUSY){}
	
	//GENERATE START CONDITION
	I2C1->CR1 |= CR1_START;
	
	//WAIT UNTIL START FLAG IS SET
	while(!(I2C1->SR1 & SR1_SB)){}
		
	//TRANSMIT SLAVE ADDRESS + WRITE
	I2C1->DR = saddr << 1;
	
	//WAIT UNTIL ADDR FLAG IS SET
	while(!(I2C1->SR1 & SR1_ADDR)){}
	//READ SR2 TO CLEAR ADDR FLAG
	temp = I2C1->SR2;
	
		
	//WAIT UNTIL THE TRANSMISSION
	while(!(I2C1->SR1 & SR1_TXE)){}
	
	//SEND MEMORY ADDRESS TO WRITE
	I2C1->DR = maddr;
			
	//WAIT UNTIL DATA REGISTER EMPTY
  while (!(I2C1->SR1 & (SR1_TXE))){}
  //SEND DATA
  I2C1->DR = data;
  //WAIT UNTIL TRANSFER FINISHED
  while (!(I2C1->SR1 & (SR1_BTF))){}

  // GENERATE STOP CONDITION
  I2C1->CR1 |= CR1_STOP;
}

void I2C_WRITE_MULTI(char saddr, char maddr, int n, uint8_t *data){
	
	volatile uint32_t temp;
	//WAIT UNTIL I2C BUS IS FREE
	while(I2C1->SR2 & SR2_BUSY){}
	
	//GENERATE START CONDITION
	I2C1->CR1 |= CR1_START;
	
	//WAIT UNTIL START FLAG IS SET
	while(!(I2C1->SR1 & SR1_SB)){}
		
	//TRANSMIT SLAVE ADDRESS + WRITE
	I2C1->DR = saddr << 1;
	
	//WAIT UNTIL ADDR FLAG IS SET
	while(!(I2C1->SR1 & SR1_ADDR)){}
	//READ SR2 TO CLEAR ADDR FLAG
	temp = I2C1->SR2;
	
		
	//WAIT UNTIL THE TRANSMISSION
	while(!(I2C1->SR1 & SR1_TXE)){}
	
	//SEND MEMORY ADDRESS TO WRITE
	I2C1->DR = maddr;
		

	for (int i = 0; i < n; i++) {
			
			//WAIT UNTIL DATA REGISTER EMPTY
       while (!(I2C1->SR1 & (SR1_TXE))){}
      //SEND DATA
      I2C1->DR = *data++;
   }

   //WAIT UNTIL TRANSFER FINISHED
   while (!(I2C1->SR1 & (SR1_BTF))){}

   // GENERATE STOP CONDITION
  I2C1->CR1 |= CR1_STOP;
}







