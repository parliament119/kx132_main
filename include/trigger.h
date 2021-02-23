/**
 * @file trigger.h
 * @author awa
 * @date 19-02-2021
 * 
 * @brief Header for trigger.c
 * 
 *  function declarations needed for trigger.c and 
 *  typedefs and structs used for trigger-mode
 * 
 */

#ifndef TRIGGER_H
#define TRIGGER_H

#include <stdint.h>
#include <stdbool.h>

#include <macros_kx132.h>

///< enum for trigger mode
typedef enum{
    fixedTriggerMode    = 0,
    offsetTriggerMode   = 1,
} trigger_mode_t;


///< enum for edge detection
typedef enum{
    detectPositive      = 0,
    detectNegative      = 1,
    detectBoth          = 2,
} edge_detection_t;


/// struct for offset-trigger-mode
typedef struct{
    uint16_t*           offsetThresholdValues;      ///< offset to normalized values, set by user
    int32_t*            positiveThresholdValues;    ///< holds values for positive Edge, so it doesnt need to be calculated every time. 
    int32_t*            negativeThresholdValues;    ///< holds values for negative Edge, so it doesnt need to be calculated every time. 
} offsetThreshold_t;


/// @brief enum for bitmask for detecting trigger
///
/// X : 0b001
/// Y : 0b010
/// Z : 0b100
///
typedef enum{
    x_trigger           = 0b001,
    y_trigger           = 0b010,
    xy_trigger          = 0b011,
    z_trigger           = 0b100,
    xz_trigger          = 0b101,
    yz_trigger          = 0b110,
    xyz_trigger         = 0b111,
} trigger_bitmask_t;


///< enum for detect-logic 
typedef enum{
    and_logic           = 0,
    or_logic            = 1,
} trigger_logic_t;


/// struct for time/sample-based information of trigger
typedef struct{
    uint32_t            timeBeforeTrig;             ///< time before trigger in Milliseconds; set by user
    uint32_t            timeAfterTrig;              ///< time after trig in Milliseconds; set by user
    uint32_t            samplesBeforeTrig;          ///< samples to read from ringbuffer before trigger; calculated based on frequency and timeBeforeTrig
    uint32_t            samplesAfterTrig;           ///< samples to read from ringbuffer after trigger; calculated based on frequency and timeAfterTrig
    uint32_t            numberOfSamples;            ///< sum of number of samples to be read from ringbuffer and send over tcp
    uint32_t            triggerIndex;               ///< holds info about index of samples which triggered
    double              outputDataRate;             ///< outputDataRate for calculating needed samples for reading from ringbuffer
} trigger_info_t;


/// struct for configuration of trigger
typedef struct{
    trigger_mode_t      triggerMode;                ///< fixed / offset
    trigger_info_t*     triggerInfo;                ///< struct containing relevant information for reading data from ringbuffer
    edge_detection_t    edgeDetection;              ///< pos | neg | both
    trigger_bitmask_t   triggerBitmask;       	    ///< bitmask for detecting trigger
    trigger_logic_t     triggerLogic;               ///< whether to apply AND- or OR- Logic to trigger
} trigger_config_t;


/// struct for threshold-values and normalized data 
typedef struct{
    int16_t*            normalizedData;             ///< holds normalized data for each axis
    int16_t*            fixedThresholds;            ///< holds fixedThreshold values for each axis
    offsetThreshold_t*  offsetThreshold;            ///< struct containing relevant values for offset-trigger-mode
} trigger_data_t;




/**
 * @brief Checks whether axis triggered.
 * 
 * @note only called in Offset-Threshold-Mode
 * 
 * @param axis              which axis should be checked for trigger
 * @param formattedData     formatted int16_t data read from KX132 for corresponding axis
 * @param edgeDetection     positive / negative / both
 * @param offsetThreshold   pointer to struct containing threshold values
 * @return true             if trigger was detected
 * @return false            if trigger was not detected
 */
bool detectOffsetTrigger(axis_t axis, int16_t formattedData, edge_detection_t edgeDetection ,offsetThreshold_t *offsetThreshold);


/**
 * @brief Checks whether axis triggered.
 * 
 * @note only called in Fixed-Threshold-Mode
 * 
 * @param axis              which axis should be checked for trigger
 * @param formattedData     formatted int16_t data read from KX132 for corresponding axis 
 * @param edgeDetection     positive / negative / both
 * @param triggerData       pointer to struct containing threshold values and normalized axis data
 * @return true             if trigger was detected
 * @return false            if trigger was detected
 */
bool detectFixedTrigger(axis_t axis, int16_t formattedData, edge_detection_t edgeDetection, trigger_data_t *triggerData);


/**
 * @brief Checks all axes for trigger.
 * 
 * Calls detectOffsetTrigger() or detectFixedTrigger() depending on configuration.
 * 
 * @param formattedData     pointer to array of formatted int16_t data read from KX132 for all axes
 * @param triggerConfig     pointer to struct containing configuration for the trigger-mode
 * @param triggerData       pointer to struct containing threshold-values and normalized axis data 
 * @return true             if configured bitmask-logic-condition was met 
 * @return false            if configured bitmask-logic-condition was not met 
 */
bool detectAllTriggers(int16_t *formattedData, trigger_config_t* triggerConfig, trigger_data_t *triggerData);



#endif // TRIGGER_H