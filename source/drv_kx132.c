/**
 * @file drv_kx132.c
 * @author awa
 * @date 19-02-2021
 * 
 * @brief  Driver for KX132-Accelerometer.
 * 
 * 	Contains functions for initialization and reading.
 * 
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <regs_kx132.h>
#include <spi_wrapper.h>
#include <config_kx132.h>
#include <macros_kx132.h>
#include <ringbuffer.h>
#include <utility.h>
#include <trigger.h>
#include <tcp.h>
#include <debug_macros.h>


static bool MAIN_LOOP = true;


/**
 * @brief Reads Data from KX132 in streaming mode and sends it over tcp to client.
 * 
 * @param readMode 			for reading with sync0 / async
 */
void kx132_streaming_mode(readMode_sw_t readMode);


/**
 * @brief Reads data from KX132 in trigger mode and writes it to ringbuffer. 
 * 
 * 	When trigger occurs, data is read as long as specified in triggerConfig->samplesAfterTrig
 * 	and then send over tcp to client.
 * 
 * @param softwareConfig 	pointer to struct containing readMode and buffersize
 * @param triggerConfig 	pointer to struct containing settings of trigger mode
 * @param triggerData 		pointer to struct containing thresholds and normalized data
 */
void kx132_trigger_mode(sw_config_t *softwareConfig, trigger_config_t *triggerConfig, trigger_data_t *triggerData);



bool kx132_init(kx132_config_t* kx132_config){

    hw_config_t*	hardwareConfig 				= (hw_config_t*) kx132_config->hardwareConfig;
    sw_config_t*	softwareConfig 				= (sw_config_t*) kx132_config->softwareConfig;

    uint8_t 		whoAmI_Register_Default 	= 0x3D;
    uint8_t 		whoAmI_Register_Value 		= 0;

    uint8_t 		manId_Register_Default[4];
    uint8_t 		manId_Register_Value[4];

    manId_Register_Default[0] = 0x4B;
    manId_Register_Default[1] = 0x69;
    manId_Register_Default[2] = 0x6F;
    manId_Register_Default[3] = 0x6E;


    spi_read(WHO_AM_I_REG_ADDR, &whoAmI_Register_Value);
    if(whoAmI_Register_Value != whoAmI_Register_Default){
        printf("[error] WHO_AM_I_REGISTER not read correctly. Should be: 0x3D\n");
    }

    spi_read_burst(MAN_ID_REG_ADDR, manId_Register_Value, 4);
    for(uint8_t i = 0; i < 4; i++){
        if(manId_Register_Default[i] != manId_Register_Value[i]){
            printf("[error] MAN_ID_REGISTER not read correctly.\n");
            break;
        }
    }


    switch (softwareConfig->readMode)
    {
        case synchronous_read_0:
            spi_write(CNTL1_REG_ADDR, 	0x00);
            spi_write(ODCNTL_REG_ADDR, 	hardwareConfig->outputDataRate_hw);
            spi_write(CNTL1_REG_ADDR, 	0xE0 | hardwareConfig->gRange_hw);

            break;

        // case synchronous_read_1:
        // 	spi_write(CNTL1_REG_ADDR,	0x00);
        // 	spi_write(INC1_REG_ADDR,	0x30);
        // 	spi_write(INC4_REG_ADDR,	0x10);
        // 	spi_write(ODCNTL_REG_ADDR,	kx132_config->hardwareConfig->outputDataRate);
        // 	spi_write(CNTL1_REG_ADDR,	0xE0 | hardwareConfig->gRange_hw);
        // 
        // 	break;

        case asynchronous_read:
            spi_write(CNTL1_REG_ADDR, 	0x00);
            spi_write(ODCNTL_REG_ADDR, 	hardwareConfig->outputDataRate_hw);
            spi_write(CNTL1_REG_ADDR, 	0xC0 | hardwareConfig->gRange_hw);

            break;
        
        default:
            printf("[error] Read Mode could not be set.\n");
            break;
    }


    return true;
}


void kx132_software_reset(void){
    spi_write(CNTL2_REG_ADDR, 0x8);
}


void kx132_async_read_raw_data(uint8_t* xyzRawData){
    spi_read_burst(XOUT_L_REG_ADDR, xyzRawData, NUMBER_OF_CHANNELS);
}


bool kx_132_sync0_read_raw_data(uint8_t* xyzRawData){
    uint8_t dataReady = 0;
    spi_read(INS2_REG_ADDR, &dataReady);

    if(dataReady & 0x10){
        spi_read_burst(XOUT_L_REG_ADDR, xyzRawData, NUMBER_OF_CHANNELS);
        return true;
    }

    return false;
}


void *kx132_runtime_config(void *kx_config){
    
    kx132_config_t		*kx132_config 	= (kx132_config_t*) kx_config;
    trigger_config_t 	*triggerConfig 	= kx132_config->triggerConfig;
    trigger_data_t   	*triggerData 	= kx132_config->triggerData;

    #ifdef TCP_SERVER
        printf("[drv_kx132] Runtime Config listening.\n");
        char data[256];

        while(MAIN_LOOP){
            tcp_recv(data);

            if(processRuntimeFlags(data, triggerConfig, triggerData)){
                printf("[drv_kx132] Client terminated connection. Exiting Program.\n");
                MAIN_LOOP = false;
                break;
            }

            strcpy(data, "");
        }
    #endif //TCP_SERVER

    return NULL;
}


void *kx132_main_loop(void *kx_config){

    kx132_config_t 		*kx132_config 	= (kx132_config_t*) kx_config;

    sw_config_t 		*softwareConfig = kx132_config->softwareConfig;
    trigger_config_t 	*triggerConfig 	= kx132_config->triggerConfig;
    trigger_data_t 		*triggerData 	= kx132_config->triggerData;

    if(softwareConfig->useMode == streaming_mode){
        kx132_streaming_mode(softwareConfig->readMode);

    }
    else if(softwareConfig->useMode == triggered_mode){
        kx132_trigger_mode(softwareConfig, triggerConfig, triggerData);
    }

    return NULL;
}


void kx132_streaming_mode(readMode_sw_t readMode){


    //-------------------------------------------------------------------
    //--- Variable Declarations  ----------------------------------------
    //-------------------------------------------------------------------

    uint8_t 	xyzRawData		[NUMBER_OF_CHANNELS];
    int16_t 	xyzFormatted	[2][NUMBER_OF_AXES];

    uint64_t 	count 			= 0;
    uint8_t 	activeBuffer 	= 0;


    for(uint8_t i = 0; i < NUMBER_OF_AXES; i++){
        xyzFormatted[0][i] = 0;
        xyzFormatted[1][i] = 0;
    }


    //-------------------------------------------------------------------
    //--- Reading Loop  -------------------------------------------------
    //-------------------------------------------------------------------

    while(MAIN_LOOP)
    {
        if(readMode == synchronous_read_0){
            if(!kx_132_sync0_read_raw_data(xyzRawData)){
                continue;
            }
        }
        else if(readMode == asynchronous_read){
            kx132_async_read_raw_data(xyzRawData);
        }
        
        convertRawArray(xyzRawData, xyzFormatted[activeBuffer]);
        

        #ifdef DEBUG_PRINT_STREAM_DATA
            printf("|X: %6.d   |Y: %6.d   |Z: %6.d   | #%d\n",
                    xyzFormatted[!activeBuffer][X_INDEX],
                    xyzFormatted[!activeBuffer][Y_INDEX],
                    xyzFormatted[!activeBuffer][Z_INDEX], count);
        #endif //DEBUG_PRINT_STREAM_DATA


        #ifdef TCP_SERVER
            tcp_send(xyzFormatted[!activeBuffer]);
        #endif //TCP_SERVER


        activeBuffer = !activeBuffer;

        count++;
    }

    return;
}


void kx132_trigger_mode(sw_config_t *softwareConfig, trigger_config_t* triggerConfig, trigger_data_t *triggerData){

    //-------------------------------------------------------------------
    //--- Variable Declarations & Memmory Allocation --------------------
    //-------------------------------------------------------------------

    uint8_t 		xyzRawData		[NUMBER_OF_CHANNELS];
    int16_t 		xyzFormatted	[NUMBER_OF_AXES];
    ringbuffer_t 	xyzRingbuffer	[NUMBER_OF_AXES];
    int16_t* 		xyzBuffer		[NUMBER_OF_AXES];
    int16_t*		xyzReadBuffer	[NUMBER_OF_AXES];

    uint32_t 		samplesRead 	= 0;
    bool 			triggerDetected = false;


    for(axis_t axis = 0; axis < NUMBER_OF_AXES ; axis++){

        xyzBuffer[axis] 	= (int16_t*) malloc(softwareConfig->bufferSize * sizeof(int16_t));
        xyzReadBuffer[axis] = (int16_t*) malloc(softwareConfig->bufferSize * sizeof(int16_t));

        if(xyzBuffer[axis] == NULL){
            printf("[drv_kx132][error] Buffer could not be allocated!\n", axis+1);
            return; //TODO
        }

        if(xyzReadBuffer[axis] == NULL){
            printf("[drv_kx132][error] Buffer could not be allocated!\n", axis+1);
            return; //TODO
        }
    }

    
    if(!rb_xyz_init(xyzRingbuffer, xyzBuffer, softwareConfig->bufferSize)){
        printf("[drv_kx132][error] Ringbuffer could not be initialized.\n");
        return;
    }

    //-------------------------------------------------------------------
    //--- Reading Loop  -------------------------------------------------
    //-------------------------------------------------------------------

    while(MAIN_LOOP)
    {
        if(softwareConfig->readMode == synchronous_read_0){
            if(!kx_132_sync0_read_raw_data(xyzRawData)){
                continue; // repeatedly try to read until new axis data is available 
            }
        }
        else if(softwareConfig->readMode == asynchronous_read){
            kx132_async_read_raw_data(xyzRawData);
        }
        
        convertRawArray(xyzRawData, xyzFormatted);


        //-------------------------------------------------------------------
        //---Trigger Detection  ----------------------------------------------
        //-------------------------------------------------------------------
        
        triggerDetected = detectAllTriggers(xyzFormatted, triggerConfig, triggerData);

        if(triggerDetected){

            // push values that triggered the threshold and get index for reading data from ringbuffer
            triggerConfig->triggerInfo->triggerIndex	=	rb_push(&xyzRingbuffer[X_INDEX], xyzFormatted[X_INDEX]);
                                                            rb_push(&xyzRingbuffer[Y_INDEX], xyzFormatted[Y_INDEX]);
                                                            rb_push(&xyzRingbuffer[Z_INDEX], xyzFormatted[Z_INDEX]);
            

            while(samplesRead <= triggerConfig->triggerInfo->samplesAfterTrig){
                if(softwareConfig->readMode == synchronous_read_0){
                    if(!kx_132_sync0_read_raw_data(xyzRawData)){
                        continue; // repeatedly try to read until new axis data is available 
                    }
                }
                else if(softwareConfig->readMode == asynchronous_read){
                    kx132_async_read_raw_data(xyzRawData);
                }

                convertRawArray(xyzRawData, xyzFormatted);

                rb_push(&xyzRingbuffer[X_INDEX], xyzFormatted[X_INDEX]);
                rb_push(&xyzRingbuffer[Y_INDEX], xyzFormatted[Y_INDEX]);
                rb_push(&xyzRingbuffer[Z_INDEX], xyzFormatted[Z_INDEX]);

                samplesRead++;
            }

            samplesRead = 0;

            rb_read_chunk(&xyzRingbuffer[X_INDEX], xyzReadBuffer[X_INDEX], triggerConfig->triggerInfo);
            rb_read_chunk(&xyzRingbuffer[Y_INDEX], xyzReadBuffer[Y_INDEX], triggerConfig->triggerInfo);
            rb_read_chunk(&xyzRingbuffer[Z_INDEX], xyzReadBuffer[Z_INDEX], triggerConfig->triggerInfo);


            #ifdef DEBUG_PRINT_TRIG_DATA
                printf("-------------------------------\n");
                printf("-------------------------------\n");
                for(int u = 0; u < triggerConfig->triggerInfo->numberOfSamples; u++){
                    printf("X:%6.d  |Y:%6.d  |Z:%6.d    --- #%d\n", 
                            xyzReadBuffer[X_INDEX][u],
                            xyzReadBuffer[Y_INDEX][u],
                            xyzReadBuffer[Z_INDEX][u],
                            u);
                }
                printf("-------------------------------\n\n\n\n");
            #endif //DEBUG_PRINT_TRIG_DATA


            #ifdef TCP_SERVER
                tcp_send_trig_buffer(xyzReadBuffer, triggerConfig->triggerInfo, triggerData->normalizedData);
            #endif //TCP_SERVER

            continue; // jump back to reading Data.
        }

        // if no trigger was detected, just push the values onto ringbuffer
        rb_push(&xyzRingbuffer[X_INDEX], xyzFormatted[X_INDEX]);
        rb_push(&xyzRingbuffer[Y_INDEX], xyzFormatted[Y_INDEX]);
        rb_push(&xyzRingbuffer[Z_INDEX], xyzFormatted[Z_INDEX]);

    }


    for(int i = 0; i < NUMBER_OF_AXES ; i++){
        free(xyzBuffer[i]);
    }


    return;
}