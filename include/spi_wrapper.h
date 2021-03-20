/**
 * @file spi_wrapper.h
 * @author awa
 * @date 19-02-2021
 * 
 * @brief Header for spi_wrapper.c
 * 
 *  function declarations needed for SPI-communication.
 * 
 */

#ifndef SPI_WRAPPER_H
#define SPI_WRAPPER_H

///\cond
#include <stdint.h>
#include <stdbool.h>
///\endcond

#include <regs_kx132.h>


/**
 * @brief Initiliazes the SPI-connection.
 *  
 * @return true     if success
 * @return false    if error
 */
bool spi_init(void);


/**
 * @brief Closes the SPI-connection.
 * 
 */
void spi_deinit(void);


/**
 * @brief Writes a single byte over SPI to a register.
 * 
 * @param reg       destination register
 * @param data      byte for writing to register
 */
void spi_write(kx132_reg_t reg, uint8_t data);


/**
 * @brief Reads a single byte over SPI from a register.
 * 
 * @param reg       source register
 * @param data      pointer to buffer where read byte should be saved 
 */
void spi_read(kx132_reg_t reg, uint8_t* data);


/**
 * @brief Reads n bytes over SPI through auto-increment, starting at provided register
 * 
 * @note Used for reading the Axis-Data-Outputs of the KX132.
 * 
 * @param reg       first source register
 * @param data      pointer to buffer where read bytes should be saved
 * @param len       number of register to read
 */
void spi_read_burst(kx132_reg_t reg, uint8_t* data, uint16_t len);


#endif //SPI_WRAPPER