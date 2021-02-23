/**
 * @file spi_wrapper.c
 * @author awa
 * @date 19-02-2021
 * 
 * @brief Contains functions for communicating over 4-Wire-SPI.
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <bcm2835.h>

#include <regs_kx132.h>
#include <spi_wrapper.h>


#define BCM2835_SPI_CUSTOM_CLK_DIVIDER_40   40      ///< RPi core_freq and core_freq_min in /boot/coinfig.txt both set to 400 MHz
#define READ_SPI                            0x80    ///< Reading from KX132-Register requires sending the register number combined with the read-command 0x80 through an OR
#define WRITE_SPI                           0x00    ///< Writing to KX132-Register does not require any extra command. Only here for clarity.

#define SPI_READ_EXTRA_BYTE                 1       ///<  Reading over SPI needs one "extra" byte.
                                                    ///<  For example: sending the register-address for reading 6 values through auto-increment. 
                                                    ///<  [reg-address + value1 + value2 + ... + value6]


bool spi_init(void){

    if (!bcm2835_init()){
        return false;
    }

    bcm2835_spi_begin                   ();
    bcm2835_spi_setBitOrder             (BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode             (BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider         (BCM2835_SPI_CUSTOM_CLK_DIVIDER_40);
    bcm2835_spi_chipSelect              (BCM2835_SPI_CS0);
    bcm2835_spi_setChipSelectPolarity   (BCM2835_SPI_CS0, LOW);

    return true;
}


void spi_deinit(void){
    bcm2835_spi_end();
}


void spi_write(kx132_reg_t reg, uint8_t data){

    uint8_t buffer[2];
    buffer[0] = reg | WRITE_SPI;
    buffer[1] = data;

    bcm2835_spi_transfern(buffer, 2);
    return;
}


void spi_read(kx132_reg_t reg, uint8_t* data){

    uint8_t buffer[2];
    buffer[0] = reg | READ_SPI;

    bcm2835_spi_transfern(buffer,2);
    *data = buffer[1];
    return;
}


void spi_read_burst(kx132_reg_t reg, uint8_t* data, uint16_t len){ 

    uint8_t buffer[len + SPI_READ_EXTRA_BYTE]; // 
    buffer[0] = reg | READ_SPI;

    bcm2835_spi_transfern(buffer, (len + SPI_READ_EXTRA_BYTE) );
    
    memcpy(data, &buffer[1], len);

}