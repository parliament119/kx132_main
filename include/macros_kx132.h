/**
 * @file macros_kx132.h
 * @author awa
 * @date 20-02-2021
 * 
 * @brief Macros for accessing arrays & data of KX132 
 * 
 */

#ifndef MACROS_KX132_H
#define MACROS_KX132_H


#define ARRAY_SIZE(array)   ( (sizeof(array)) / (sizeof(array[0])) )

#define X_INDEX				0
#define Y_INDEX				1
#define Z_INDEX				2
#define NUMBER_OF_AXES		3
#define NUMBER_OF_CHANNELS	6


#define X_LOW_CHANNEL		0
#define X_HIGH_CHANNEL		1
#define Y_LOW_CHANNEL		2
#define Y_HIGH_CHANNEL		3
#define Z_LOW_CHANNEL		4
#define Z_HIGH_CHANNEL		5

///< enum for 3 axes
typedef enum{
    X_AXIS = X_INDEX,
    Y_AXIS = Y_INDEX,
    Z_AXIS = Z_INDEX
} axis_t;


#endif // MACROS_KX132_H