#ifndef I2C_WRAPPER_H
#define I2C_WRAPPER_H

#include <stdbool.h>


int i2c_init(void);
void i2c_close(void);
int i2c_write(uint8_t reg, uint8_t data);
int i2c_read(uint8_t reg, uint8_t* result);
int i2c_burst_read(uint8_t reg, uint8_t* result, uint8_t size);


#endif //I2C_WRAPPER_H