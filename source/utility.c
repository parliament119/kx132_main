/**
 * @file utility.c
 * @author awa
 * @date 20-02-2021
 * 
 * @brief Contains function for converting raw data to signed 16-Bit
 * 
 */

///\cond
#include <stdint.h>
#include <stdio.h>
///\endcond

#include <utility.h>
#include <macros_kx132.h>



int16_t convertRaw(uint8_t lowVal, uint8_t highVal){
    return ((highVal & 0xFF) << 8) | lowVal;	//! why ( & 0xFF ) ????
}


void convertRawArray(uint8_t* xyzRawData, int16_t* data){
    data[X_INDEX] = convertRaw(xyzRawData[X_LOW_CHANNEL], xyzRawData[X_HIGH_CHANNEL]);
    data[Y_INDEX] = convertRaw(xyzRawData[Y_LOW_CHANNEL], xyzRawData[Y_HIGH_CHANNEL]);
    data[Z_INDEX] = convertRaw(xyzRawData[Z_LOW_CHANNEL], xyzRawData[Z_HIGH_CHANNEL]);
    return;
}



void timer(void){
    //! #include <sys/time.h>

    // struct timespec startTimeTotal, endTimeTotal;
    // long timeDiffTotal;
    // clock_gettime(CLOCK_MONOTONIC_RAW, &startTimeTotal);
    // clock_gettime(CLOCK_MONOTONIC_RAW, &endTimeTotal);
    // timeDiffTotal = ((endTimeTotal.tv_sec-startTimeTotal.tv_sec)*(1000*1000*1000) + (endTimeTotal.tv_nsec-startTimeTotal.tv_nsec)) / 1000 ;
    // printf("%lu usec passed total\n",timeDiffTotal) ;

    return;
}