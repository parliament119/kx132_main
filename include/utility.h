/**
 * @file utility.h
 * @author awa
 * @date 20-02-2021
 * 
 * @brief header for utility.c
 *  
 *  function declarations needed for utility.c
 * 
 */

#ifndef HELPER_H
#define HELPER_H


/**
 * @brief Converts low + high 8-Bit values to one signed 16-Bit value
 * 
 * @param lowVal        lower 8-Bit value 
 * @param highVal       higher 8-Bit value
 * @return int16_t      signed 16-Bit value
 */
int16_t convertRaw(uint8_t lowVal, uint8_t highVal);


/**
 * @brief Converts an array consisiting of raw data to array of signed 16-Bit values
 * 
 * @param xyzRawData    pointer to array holding raw data (low + high)
 * @param data          pointer to array where signed 16-Bit values should be saved
 */
void convertRawArray(uint8_t* xyzRawData, int16_t* data);



#endif // HELPER_H