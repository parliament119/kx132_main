/**
 * @file trigger.c
 * @author awa
 * @date 19-02-2021
 * 
 * @brief Contains functions for trigger-detection.
 * 
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <trigger.h>
#include <config_kx132.h>
#include <macros_kx132.h>
#include <regs_kx132.h>
#include <spi_wrapper.h>
#include <drv_kx132.h>
#include <utility.h>



bool detectOffsetTrigger(axis_t axis, int16_t formattedData, edge_detection_t edgeDetection ,offsetThreshold_t *offsetThreshold){
        
    bool triggerDetected = false;

    switch (edgeDetection)
        {
            case detectPositive:
                triggerDetected = formattedData >= offsetThreshold->positiveThresholdValues[axis];
                break;
            
            case detectNegative:
                triggerDetected = formattedData <= offsetThreshold->negativeThresholdValues[axis];
                break;
            
            case detectBoth:
                /// XOR, because if both are true, something propably went wrong anyways :)
                triggerDetected =  (formattedData >= offsetThreshold->positiveThresholdValues[axis]) ^
                        (formattedData <= offsetThreshold->negativeThresholdValues[axis]);
                break;

            default:
                triggerDetected = false;
                break;
        }

    return triggerDetected;
}


bool detectFixedTrigger(axis_t axis, int16_t formattedData, edge_detection_t edgeDetection, trigger_data_t *triggerData){
    
    bool triggerDetected = false;

    switch (edgeDetection)
        {
            /// Condition for detecting negative edge: Threshold needs to be bigger than normalized data.
            case detectPositive:
                if(triggerData->fixedThresholds[axis] > triggerData->normalizedData[axis]){
                    triggerDetected = formattedData >= triggerData->fixedThresholds[axis];
                }
                break;
            
            /// Condition for detecting negative edge: Threshold needs to be smaller than normalized data.
            case detectNegative:
                if(triggerData->fixedThresholds[axis] < triggerData->normalizedData[axis]){
                    triggerDetected = formattedData <= triggerData->fixedThresholds[axis];
                }
                break;
            
            /// Works with absolute values, hence detects both directions.
            case detectBoth:
                if(abs(triggerData->fixedThresholds[axis]) > abs(triggerData->normalizedData[axis])){
                    triggerDetected = abs(formattedData) >= abs(triggerData->fixedThresholds[axis]);
                }
                else if(abs(triggerData->fixedThresholds[axis]) < abs(triggerData->normalizedData[axis])){
                    triggerDetected = abs(formattedData) <= abs(triggerData->fixedThresholds[axis]);
                }
                break;
            
            default:
                triggerDetected = false;
                break;
        }

    return triggerDetected;
}


bool detectAllTriggers(int16_t *formattedData, trigger_config_t* triggerConfig, trigger_data_t *triggerData){

    uint8_t triggerDetected = 0b000;
    bool    triggeredAxis   [NUMBER_OF_AXES];

    if(triggerConfig->triggerMode == fixedTriggerMode){

        triggeredAxis[X_INDEX] = detectFixedTrigger(X_AXIS, formattedData[X_INDEX], triggerConfig->edgeDetection, triggerData);
        triggeredAxis[Y_INDEX] = detectFixedTrigger(Y_AXIS, formattedData[Y_INDEX], triggerConfig->edgeDetection, triggerData);
        triggeredAxis[Z_INDEX] = detectFixedTrigger(Z_AXIS, formattedData[Z_INDEX], triggerConfig->edgeDetection, triggerData);
    }

    else if(triggerConfig->triggerMode == offsetTriggerMode){

        triggeredAxis[X_INDEX] = detectOffsetTrigger(X_AXIS, formattedData[X_INDEX], triggerConfig->edgeDetection, triggerData->offsetThreshold);
        triggeredAxis[Y_INDEX] = detectOffsetTrigger(Y_AXIS, formattedData[Y_INDEX], triggerConfig->edgeDetection, triggerData->offsetThreshold);
        triggeredAxis[Z_INDEX] = detectOffsetTrigger(Z_AXIS, formattedData[Z_INDEX], triggerConfig->edgeDetection, triggerData->offsetThreshold);
    }


    triggerDetected = (triggeredAxis[Z_INDEX] << 2) | (triggeredAxis[Y_INDEX] << 1) | triggeredAxis[X_INDEX]; /// shift trigger-values for bitmasking
    

    /// check trigger-values against bitmask
    if(triggerConfig->triggerLogic == and_logic){
        return ((triggerDetected & triggerConfig->triggerBitmask) == triggerConfig->triggerBitmask);
    }
    else if(triggerConfig->triggerLogic == or_logic){
        return ((triggerDetected & triggerConfig->triggerBitmask) != 0);
    }

    /// function should never get here
    printf("[trigger][error] Logic for trigger not set correctly. Triggers can't be detected.\n");
    return false;
}