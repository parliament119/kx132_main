/**
 * @file config_kx132.c
 * @author awa
 * @date 19-02-2021
 * 
 * @brief Contains functions for configuration of hardware, software and trigger.
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <config_kx132.h>
#include <trigger.h>
#include <macros_kx132.h>
#include <drv_kx132.h>
#include <utility.h>


//-------------------------------------------------------------------
//--- Macros  -------------------------------------------------------
//-------------------------------------------------------------------

#define ZERO                    0

#define TRIGGER_INDEX_VALUE     1

#define DEFAULT_NORMALIZED      0

#define DEFAULT_THRESHOLD       8000

#define DEFAULT_TIME            10

#define DEFAULT_BUFFER_SIZE     BUFFER_SIZE_2048_KB

#define NUM_NORMALIZE_SAMPLES   (5000)

#define G_RANGE_2               2
#define G_RANGE_4               4
#define G_RANGE_8               8
#define G_RANGE_16              16


//-------------------------------------------------------------------
//--- Static Globals  -----------------------------------------------
//-------------------------------------------------------------------

// Static Globals for processing User Input
static const char* trigMode_Flag       = "-trig";
static const char* trigOffset_Arg      = "offset";
static const char* trigFixed_Arg       = "fixed";

static const char* edge_Flag           = "-edge";
static const char* edgePos_Arg         = "pos";
static const char* edgeNeg_Arg         = "neg";
static const char* edgeBoth_Arg        = "both";

static const char* timeBefore_Flag     = "-t1";
static const char* timeAfter_Flag      = "-t2";

static const char* triggerLogic_Flag   = "-logic";
static const char* bitmask_Flag        = "-axes";

static const char* bitmask_Arg_List[7] = {"x", "y", "xy", "z", "xz", "yz", "xyz"};

static const char* xOffsetThres_Flag   = "-xO";
static const char* yOffsetThres_Flag   = "-yO";
static const char* zOffsetThres_Flag   = "-zO";
static const char* xFixedThres_Flag    = "-xF";
static const char* yFixedThres_Flag    = "-yF";
static const char* zFixedThres_Flag    = "-zF";


static const double outputDataRate_double[16] = {0.781, 1.563, 3.125, 6.25, 12.5, 25, 50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600};

static const trigger_bitmask_t triggerBitmaskList[7] = {
    x_trigger,
    y_trigger,
    xy_trigger,
    z_trigger,
    xz_trigger,
    yz_trigger,
    xyz_trigger,
};



//-------------------------------------------------------------------
//--- Function Definitions  -----------------------------------------
//-------------------------------------------------------------------

void kx132_config_init(uint16_t argc, char *argv[], kx132_config_t *kx132_config){

    hw_config_t*        hardwareConfig   =  kx132_config->hardwareConfig;
    sw_config_t*        softwareConfig   =  kx132_config->softwareConfig;
    trigger_config_t*   triggerConfig    =  kx132_config->triggerConfig;
    trigger_data_t*     triggerData      =  kx132_config->triggerData;


    hardwareConfig->outputDataRate_hw                               = odr_25600_Hz;
    hardwareConfig->resolution_hw                                   = resolution_16bit;
    hardwareConfig->readMode_hw                                     = synchronous_read_hw_0;
    hardwareConfig->gRange_hw                                       = g_range_8g;

    softwareConfig->useMode                                         = triggered_mode;
    softwareConfig->readMode                                        = synchronous_read_0;
    softwareConfig->bufferSize                                      = DEFAULT_BUFFER_SIZE;

    triggerConfig->triggerMode                                      = fixedTriggerMode;
    triggerConfig->edgeDetection                                    = detectBoth;
    triggerConfig->triggerBitmask                                   = xyz_trigger;
    triggerConfig->triggerLogic                                     = or_logic;

    triggerConfig->triggerInfo->timeBeforeTrig                      = DEFAULT_TIME;
    triggerConfig->triggerInfo->timeAfterTrig                       = DEFAULT_TIME;
    triggerConfig->triggerInfo->triggerIndex                        = ZERO;
    triggerConfig->triggerInfo->outputDataRate                      = outputDataRate_double[hardwareConfig->outputDataRate_hw];

    triggerData->normalizedData[X_AXIS]                             = DEFAULT_NORMALIZED;
    triggerData->normalizedData[Y_AXIS]                             = DEFAULT_NORMALIZED;
    triggerData->normalizedData[Z_AXIS]                             = DEFAULT_NORMALIZED;

    triggerData->fixedThresholds[X_AXIS]                            = DEFAULT_THRESHOLD;
    triggerData->fixedThresholds[Y_AXIS]                            = DEFAULT_THRESHOLD;
    triggerData->fixedThresholds[Z_AXIS]                            = DEFAULT_THRESHOLD;

    triggerData->offsetThreshold->offsetThresholdValues[X_AXIS]     = DEFAULT_THRESHOLD;
    triggerData->offsetThreshold->offsetThresholdValues[Y_AXIS]     = DEFAULT_THRESHOLD;
    triggerData->offsetThreshold->offsetThresholdValues[Z_AXIS]     = DEFAULT_THRESHOLD;


    // process user input coming from console and set config accordingly
    if(argc > 1){
        processInitFlags(argc, argv, hardwareConfig, softwareConfig, triggerConfig, triggerData);
    }

    setTriggerTimeSamples(triggerConfig->triggerInfo);

}


void processInitFlags(  uint16_t            argc,
                        char                *argv[],
                        hw_config_t         *hardwareConfig,
                        sw_config_t         *softwareConfig,
                        trigger_config_t    *triggerConfig,
                        trigger_data_t      *triggerData)
{

    if( ((argc-1) % 2) != 0){
        printf("[config][warning] Flags might not be set correctly. Did you forget an argument?\n");
    }

    // for finding corresponding value to flag
    outputDataRate_hw_t outputDataRateList[16] = {
        odr_0_781_Hz,
        odr_1_563_Hz,
        odr_3_125_Hz,
        odr_6_25_Hz,
        odr_12_5_Hz,
        odr_25_Hz,
        odr_50_Hz,
        odr_100_Hz,
        odr_200_Hz,
        odr_400_Hz,
        odr_800_Hz,
        odr_1600_Hz,
        odr_3200_Hz,
        odr_6400_Hz,
        odr_12800_Hz,
        odr_25600_Hz,
    };

    const char* mode_Flag           = "-mode";
    const char* modeStream_Arg      = "stream";
    const char* modeTrig_Arg        = "trig";

    const char* odr_Flag            = "-odr";
//  const char* resolution_Flag     = "-res";
    const char* gRange_Flag         = "-g";

    const char* readMode_Flag       = "-read";
    const char* readSync0_Arg       = "sync0";
//  const char* readSync1_Arg       = "sync1";
    const char* readAsync_Arg       = "async";


    uint32_t intArgValue = 0;


    // start at 1, first argv is call to program
    for(int i = 1; i < argc; i++){

        //---------------------
        //--- Use Mode  -------
        //---------------------
        if (!strncmp(argv[i], mode_Flag, strlen(mode_Flag))){
            if(!strncmp(argv[i+1], modeStream_Arg, strlen(modeStream_Arg))){
                softwareConfig->useMode = streaming_mode;
                i++; // increment i by 1, in case it was a correct argument for the flag
            }
            else if(!strncmp(argv[i+1], modeTrig_Arg, strlen(modeTrig_Arg))){
                softwareConfig->useMode = triggered_mode;
                i++;
            }
        }

        //---------------------
        //--- Frequency  ------
        //---------------------
        if(!strncmp(argv[i], odr_Flag, strlen(odr_Flag))){
            if(sscanf(argv[i+1], "%d", &intArgValue) == 1){
                hardwareConfig->outputDataRate_hw          = outputDataRateList[intArgValue];
                triggerConfig->triggerInfo->outputDataRate = outputDataRate_double[intArgValue];
                i++;
            }
        }

        // Left, in case it's needed later.
        //---------------------
        //--- Resolution  -----
        //---------------------
        // if(!strncmp(argv[i], resolution_Flag, strlen(resolution_Flag))){
        //     if(sscanf(argv[i+1], "%d", &intArgValue) == 1){
        //         if(intArgValue == RESO_8_BIT){
        //             harwareConfig->resolution = resolution_8bit;
        //             i++; 
        //         }
        //         else if(intArgValue == RESO_16_BIT){
        //             hardwareConfig->resolution = resolution_16bit;
        //             i++; 
        //         }
        //     }
        // }

        //---------------------
        //--- Read Mode  ------
        //---------------------
        if (!strncmp(argv[i], readMode_Flag, strlen(readMode_Flag))){

            if(!strncmp(argv[i+1], readSync0_Arg, strlen(readSync0_Arg))){
                hardwareConfig->readMode_hw = synchronous_read_hw_0;
                softwareConfig->readMode    = synchronous_read_0;
                i++;
            }

            // else if(!strncmp(argv[i+1], readSync1_Arg, strlen(readSync1_Arg))){
            //     hardwareConfig->readMode_hw = synchronous_read_hw_1;
            //     softwareConfig->readMode    = synchronous_read_1;
            //     i++;
            // }

            else if(!strncmp(argv[i+1], readAsync_Arg, strlen(readAsync_Arg))){
                hardwareConfig->readMode_hw = asynchronous_hw_read;
                softwareConfig->readMode    = asynchronous_read;
                i++;
            }
        }

        //---------------------
        //--- G-Range  --------
        //---------------------
        if(!strncmp(argv[i], gRange_Flag, strlen(gRange_Flag))){
            if(sscanf(argv[i+1], "%d", &intArgValue) == 1){
                if(intArgValue == G_RANGE_2){
                    hardwareConfig->gRange_hw = g_range_2g;
                    i++;
                }
                else if(intArgValue == G_RANGE_4){
                    hardwareConfig->gRange_hw = g_range_4g;
                    i++;
                }
                else if(intArgValue == G_RANGE_8){
                    hardwareConfig->gRange_hw = g_range_8g;
                    i++;
                }
                else if(intArgValue == G_RANGE_16){
                    hardwareConfig->gRange_hw = g_range_16g;
                    i++;
                }
            }
        }

        //---------------------
        //--- Trig Mode  ------
        //---------------------
        if (!strncmp(argv[i], trigMode_Flag, strlen(trigMode_Flag))){
            if(!strncmp(argv[i+1], trigOffset_Arg, strlen(trigOffset_Arg))){
                triggerConfig->triggerMode = offsetTriggerMode;
                i++; // increment i by 1, in case it was a correct argument for the flag
            }
            else if(!strncmp(argv[i+1], trigFixed_Arg, strlen(trigFixed_Arg))){
                triggerConfig->triggerMode = fixedTriggerMode;
                i++;
            }
        }

        //---------------------
        //--- Edge Detection --
        //---------------------
        if (!strncmp(argv[i], edge_Flag, strlen(edge_Flag))){
            if(!strncmp(argv[i+1], edgePos_Arg, strlen(edgePos_Arg))){
                triggerConfig->edgeDetection = detectPositive;
                i++;
            }
            else if(!strncmp(argv[i+1], edgeNeg_Arg, strlen(edgeNeg_Arg))){
                triggerConfig->edgeDetection = detectNegative;
                i++;
            }
            else if(!strncmp(argv[i+1], edgeBoth_Arg, strlen(edgeBoth_Arg))){
                triggerConfig->edgeDetection = detectBoth;
                i++;
            }
        }

        //---------------------
        //--- Logic  ----------
        //---------------------
        if(!strncmp(argv[i], triggerLogic_Flag, strlen(triggerLogic_Flag))){
                if(sscanf(argv[i+1], "%d", &intArgValue) == 1){
                    if(intArgValue == and_logic){
                        triggerConfig->triggerLogic = and_logic;
                    }
                    else if(intArgValue == or_logic){
                        triggerConfig->triggerLogic = or_logic;
                    }
                    i++;
                }
        }

        //---------------------
        //--- Axes Logic  -----
        //---------------------
        if (!strncmp(argv[i], bitmask_Flag, strlen(bitmask_Flag))){
            for(uint8_t u = 0; u < 7; u++){
                if(!strncmp(argv[i+1], bitmask_Arg_List[u], 3)){
                    triggerConfig->triggerBitmask = triggerBitmaskList[u];
                    i++; // increment i by 1, in case it was a correct argument for the flag
                    break;
                }
            }
        }

        //---------------------
        //--- Time  -----------
        //---------------------
        if(!strncmp(argv[i], timeBefore_Flag, strlen(timeBefore_Flag))){
                if(sscanf(argv[i+1], "%d", &intArgValue) == 1){
                    triggerConfig->triggerInfo->timeBeforeTrig = intArgValue;
                    i++;
                }
        }

        if(!strncmp(argv[i], timeAfter_Flag, strlen(timeAfter_Flag))){
                if(sscanf(argv[i+1], "%d", &intArgValue) == 1){
                    triggerConfig->triggerInfo->timeAfterTrig = intArgValue;
                    i++;
                }
        }


        //---------------------
        //--- Offset Thres  ---
        //---------------------
        if(!strncmp(argv[i], xOffsetThres_Flag, strlen(xOffsetThres_Flag))){
            if(sscanf(argv[i+1], "%d", &intArgValue) == 1){
                if((intArgValue >= ZERO) && (intArgValue <= UINT16_MAX)){
                    triggerData->offsetThreshold->offsetThresholdValues[X_INDEX] = intArgValue;
                    i++;
                }
            }
        }
        if(!strncmp(argv[i], yOffsetThres_Flag, strlen(yOffsetThres_Flag))){
            if(sscanf(argv[i+1], "%d", &intArgValue) == 1){
                if((intArgValue >= ZERO) && (intArgValue <= UINT16_MAX)){
                    triggerData->offsetThreshold->offsetThresholdValues[Y_INDEX] = intArgValue;
                    i++;
                }
            }
        }
        if(!strncmp(argv[i], zOffsetThres_Flag, strlen(zOffsetThres_Flag))){
            if(sscanf(argv[i+1], "%d", &intArgValue) == 1){
                if((intArgValue >= ZERO) && (intArgValue <= UINT16_MAX)){
                    triggerData->offsetThreshold->offsetThresholdValues[Z_INDEX] = intArgValue;
                    i++;
                }
            }
        }

        //---------------------
        //--- Fixed Thres  ----
        //---------------------
        if(!strncmp(argv[i], xFixedThres_Flag, strlen(xFixedThres_Flag))){
            if(sscanf(argv[i+1], "%d", &intArgValue) == 1){
                if((intArgValue >= INT16_MIN) && (intArgValue <= INT16_MAX)){
                    triggerData->fixedThresholds[X_INDEX]  = intArgValue;
                    i++;
                }
            }
        }
        if(!strncmp(argv[i], yFixedThres_Flag, strlen(yFixedThres_Flag))){
            if(sscanf(argv[i+1], "%d", &intArgValue) == 1){
                if((intArgValue >= INT16_MIN) && (intArgValue <= INT16_MAX)){
                    triggerData->fixedThresholds[Y_INDEX]  = intArgValue;
                    i++;
                }
            }
        }
        if(!strncmp(argv[i], zFixedThres_Flag, strlen(zFixedThres_Flag))){
            if(sscanf(argv[i+1], "%d", &intArgValue) == 1){
                if((intArgValue >= INT16_MIN) && (intArgValue <= INT16_MAX)){
                    triggerData->fixedThresholds[Z_INDEX]  = intArgValue;
                    i++;
                }
            }
        }
    }

    return;
}


bool processRuntimeFlags(char *data, trigger_config_t* triggerConfig, trigger_data_t* triggerData){

    const char* exit_Flag       = "exit";
    uint32_t    intArgValue     = 0;


    char *strPtr = strtok (data," ");
    while (strPtr != NULL)
    {
        //---------------------
        //--- Exit  -----------
        //---------------------
        if(!strncmp(strPtr, exit_Flag, strlen(exit_Flag))){
            return true;
        }

        //---------------------
        //--- Trig Mode  ------
        //---------------------
        if (!strncmp(strPtr, trigMode_Flag, strlen(trigMode_Flag))){
            strPtr = strtok (NULL, " ");
            if(!strncmp(strPtr, trigOffset_Arg, strlen(trigOffset_Arg))){
                triggerConfig->triggerMode = offsetTriggerMode;
            }
            else if(!strncmp(strPtr, trigFixed_Arg, strlen(trigFixed_Arg))){
                triggerConfig->triggerMode = fixedTriggerMode;
            }
        }

        //---------------------
        //--- Edge Detection --
        //---------------------
        if (!strncmp(strPtr, edge_Flag, strlen(edge_Flag))){
            strPtr = strtok (NULL, " ");
            if(!strncmp(strPtr, edgePos_Arg, strlen(edgePos_Arg))){
                triggerConfig->edgeDetection = detectPositive;
            }
            else if(!strncmp(strPtr, edgeNeg_Arg, strlen(edgeNeg_Arg))){
                triggerConfig->edgeDetection = detectNegative;
            }
            else if(!strncmp(strPtr, edgeBoth_Arg, strlen(edgeBoth_Arg))){
                triggerConfig->edgeDetection = detectBoth;
            }
        }

        //---------------------
        //--- Time  -----------
        //---------------------
        if(!strncmp(strPtr, timeBefore_Flag, strlen(timeBefore_Flag))){
            strPtr = strtok (NULL, " ");
            if(sscanf(strPtr, "%d", &intArgValue) == 1){
                triggerConfig->triggerInfo->timeBeforeTrig = intArgValue;
                setTriggerTimeSamplesBefore(triggerConfig->triggerInfo);
            }
        }

        if(!strncmp(strPtr, timeAfter_Flag, strlen(timeAfter_Flag))){
            strPtr = strtok (NULL, " ");
            if(sscanf(strPtr, "%d", &intArgValue) == 1){
                triggerConfig->triggerInfo->timeAfterTrig = intArgValue;
                setTriggerTimeSamplesAfter(triggerConfig->triggerInfo);
            }
        }

        //---------------------
        //--- Logic  ----------
        //---------------------
        if(!strncmp(strPtr, triggerLogic_Flag, strlen(triggerLogic_Flag))){
            strPtr = strtok (NULL, " ");
            if(sscanf(strPtr, "%d", &intArgValue) == 1){
                if(intArgValue == and_logic){
                    triggerConfig->triggerLogic = and_logic;
                }
                else if(intArgValue == or_logic){
                    triggerConfig->triggerLogic = or_logic;
                }
            }
        }

        //---------------------
        //--- Axes Logic  -----
        //---------------------
        if (!strncmp(strPtr, bitmask_Flag, strlen(bitmask_Flag))){
            strPtr = strtok (NULL, " ");
            for(uint8_t u = 0; u < 7; u++){
                if(!strncmp(strPtr, bitmask_Arg_List[u], 3)){
                    triggerConfig->triggerBitmask = triggerBitmaskList[u];
                    break;
                }
            }
        }

        //---------------------
        //--- Offset Thres ----
        //---------------------
        if(!strncmp(strPtr, xOffsetThres_Flag, strlen(xOffsetThres_Flag))){
            strPtr = strtok (NULL, " ");
            if(sscanf(strPtr, "%d", &intArgValue) == 1){
                if((intArgValue >= ZERO) && (intArgValue <= UINT16_MAX)){
                   triggerData->offsetThreshold->offsetThresholdValues[X_INDEX] = intArgValue;
                }
            }
        }
        if(!strncmp(strPtr, yOffsetThres_Flag, strlen(yOffsetThres_Flag))){
            strPtr = strtok (NULL, " ");
            if(sscanf(strPtr, "%d", &intArgValue) == 1){
                if((intArgValue >= ZERO) && (intArgValue <= UINT16_MAX)){
                    triggerData->offsetThreshold->offsetThresholdValues[Y_INDEX] = intArgValue;
                }
            }
        }
        if(!strncmp(strPtr, zOffsetThres_Flag, strlen(zOffsetThres_Flag))){
            strPtr = strtok (NULL, " ");
            if(sscanf(strPtr, "%d", &intArgValue) == 1){
                if((intArgValue >= ZERO) && (intArgValue <= UINT16_MAX)){
                    triggerData->offsetThreshold->offsetThresholdValues[Z_INDEX] = intArgValue;
                }
            }
        }

        //---------------------
        //--- Fixed Thres ----
        //---------------------
        if(!strncmp(strPtr, xFixedThres_Flag, strlen(xFixedThres_Flag))){
            strPtr = strtok (NULL, " ");
            if(sscanf(strPtr, "%d", &intArgValue) == 1){
                if((intArgValue >= INT16_MIN) && (intArgValue <= INT16_MAX)){
                    triggerData->fixedThresholds[X_INDEX]  = intArgValue;
                }
            }
        }
        if(!strncmp(strPtr, yFixedThres_Flag, strlen(yFixedThres_Flag))){
            strPtr = strtok (NULL, " ");
            if(sscanf(strPtr, "%d", &intArgValue) == 1){
                if((intArgValue >= INT16_MIN) && (intArgValue <= INT16_MAX)){
                    triggerData->fixedThresholds[Y_INDEX]  = intArgValue;
                }
            }
        }
        if(!strncmp(strPtr, zFixedThres_Flag, strlen(zFixedThres_Flag))){
            strPtr = strtok (NULL, " ");
            if(sscanf(strPtr, "%d", &intArgValue) == 1){
                if((intArgValue >= INT16_MIN) && (intArgValue <= INT16_MAX)){
                    triggerData->fixedThresholds[Z_INDEX]  = intArgValue;
                }
            }
        }

        strPtr = strtok (NULL, " ");
    }


    return false;
}


void setTriggerTimeSamplesBefore(trigger_info_t *triggerInfo){
    triggerInfo->samplesBeforeTrig   = (uint32_t) ( ceil( triggerInfo->outputDataRate * triggerInfo->timeBeforeTrig / 1000) );
    triggerInfo->numberOfSamples     = triggerInfo->samplesBeforeTrig + TRIGGER_INDEX_VALUE + triggerInfo->samplesAfterTrig;
}


void setTriggerTimeSamplesAfter(trigger_info_t *triggerInfo){
    triggerInfo->samplesAfterTrig   = (uint32_t) ( ceil( triggerInfo->outputDataRate * triggerInfo->timeAfterTrig / 1000) );
    triggerInfo->numberOfSamples     = triggerInfo->samplesBeforeTrig + TRIGGER_INDEX_VALUE + triggerInfo->samplesAfterTrig;
}


void setTriggerTimeSamples(trigger_info_t *triggerInfo){
    setTriggerTimeSamplesBefore(triggerInfo);
    setTriggerTimeSamplesAfter(triggerInfo);
}


void setOffsetThresholds(trigger_data_t* triggerData){
    for(axis_t axis = 0; axis < NUMBER_OF_AXES; axis++){
        triggerData->offsetThreshold->positiveThresholdValues[axis] = triggerData->normalizedData[axis] + 
                                                                      triggerData->offsetThreshold->offsetThresholdValues[axis];
        
        triggerData->offsetThreshold->negativeThresholdValues[axis] = triggerData->normalizedData[axis] - 
                                                                      triggerData->offsetThreshold->offsetThresholdValues[axis];
    }
}


void normalizeThresholds(readMode_sw_t readMode ,trigger_data_t* triggerData){

	uint8_t     xyzRawData      [NUMBER_OF_CHANNELS];
	int16_t     xyzFormatted    [NUMBER_OF_AXES];

	int32_t     xSum            = 0;
	int32_t     ySum            = 0;
	int32_t     zSum            = 0;
    uint32_t    count           = 0;

    if(readMode == synchronous_read_0){
        while(count <= NUM_NORMALIZE_SAMPLES){
            if(!kx_132_sync0_read_raw_data(xyzRawData)){
                continue;
            }

            // converting of raw xyzRawData to signed 16-Bit
            convertRawArray(xyzRawData, xyzFormatted);

            xSum += xyzFormatted[X_INDEX];
            ySum += xyzFormatted[Y_INDEX];
            zSum += xyzFormatted[Z_INDEX];

            count++;
        }
    }
    else if(readMode == asynchronous_read){
        while(count <= NUM_NORMALIZE_SAMPLES){
            kx132_async_read_raw_data(xyzRawData);

            // converting of raw xyzRawData to signed 16-Bit
            convertRawArray(xyzRawData, xyzFormatted);

            xSum += xyzFormatted[X_INDEX];
            ySum += xyzFormatted[Y_INDEX];
            zSum += xyzFormatted[Z_INDEX];

            count++;
        }
    }

	triggerData->normalizedData[X_INDEX] = xSum / NUM_NORMALIZE_SAMPLES;
	triggerData->normalizedData[Y_INDEX] = ySum / NUM_NORMALIZE_SAMPLES;
	triggerData->normalizedData[Z_INDEX] = zSum / NUM_NORMALIZE_SAMPLES;

}