#include <stdint.h>

void I2C_INIT(void);
void I2C_READ_BYTE(char saddr, char maddr, uint8_t *data);
void I2C_READ(char saddr, char maddr,int n, uint8_t *data);
void I2C_WRITE(char saddr, char maddr, uint8_t data);
void I2C_WRITE_MULTI(char saddr, char maddr, int n, uint8_t *data);
