/**
 * @file main.c
 * @author awa
 * @date 19-02-2021
 * 
 * @brief Main-file for KX132-Accelerometer project
 * 
 * 	For usage on a RaspberryPi connected to the KX132 through 4-Wire-SPI.
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/resource.h>

#include <regs_kx132.h>
#include <macros_kx132.h>
#include <spi_wrapper.h>
#include <config_kx132.h>
#include <ringbuffer.h>
#include <drv_kx132.h>
#include <trigger.h>
#include <utility.h>
#include <tcp.h>
#include <debug_macros.h>


//-------------------------------------------------------------------
//--- MAIN  ---------------------------------------------------------
//-------------------------------------------------------------------

int main(uint16_t argc, char* argv[]){
    printf("\n");
    printf("---------------------------------\n");
    printf("---------------------------------\n");
    printf("---------------------------------\n");
    printf("\n");

    //-------------------------------------------------------------------
    //--- Variable Declarations  ----------------------------------------
    //-------------------------------------------------------------------

    // Declaration of variables for threading
    pthread_t           threadMainLoop;
    pthread_t           threadRuntimeConfig;

    // Declaration of variables for hardware-, software- and trigger-config
    kx132_config_t      kx132_config;
    
    main_config_t       mainConfig;
    trigger_config_t    triggerConfig;
    trigger_data_t      triggerData;
    
    offsetThreshold_t   offsetThreshold;
    trigger_info_t      triggerInfo;
    
    uint16_t            offsetThresholds	[NUMBER_OF_AXES];
    int32_t             positiveThresholds	[NUMBER_OF_AXES];
    int32_t             negativeThresholds	[NUMBER_OF_AXES];

    int16_t             fixedThresholds		[NUMBER_OF_AXES];
    int16_t             xyzNormalizedValues	[NUMBER_OF_AXES];

    kx132_config.mainConfig                     = &mainConfig;
    kx132_config.triggerConfig                  = &triggerConfig;
    kx132_config.triggerData                    = &triggerData;

    offsetThreshold.offsetThresholdValues       = offsetThresholds;
    offsetThreshold.positiveThresholdValues     = positiveThresholds;
    offsetThreshold.negativeThresholdValues     = negativeThresholds;

    triggerData.fixedThresholds                 = fixedThresholds;
    triggerData.normalizedData                  = xyzNormalizedValues;
    triggerData.offsetThreshold                 = &offsetThreshold;

    triggerConfig.triggerInfo                   = &triggerInfo;


    //-------------------------------------------------------------------
    //--- Initialiazing Functions  --------------------------------------
    //-------------------------------------------------------------------

    kx132_config_init(argc, argv, &kx132_config);

    if(!spi_init()){
        printf("[main][error] SPI Init failed.\n");
        return -1;
    }

    // make sure KX132 is in a defined state after eventual program restart
    kx132_software_reset();

    if(!kx132_init(&kx132_config)){
        printf("[main][error] KX132 Accelerometer Init failed.\n");
        return -1;
    }

    normalizeThresholds(mainConfig.readMode_hw, &triggerData);
    setOffsetThresholds(&triggerData);

    #ifdef TCP_SERVER
    if(!tcp_server_init()){
        printf("[main][error] Could not establish TCP-Server connection.\n");
        return -1;
    }
    #endif //TCP_SERVER

    //-------------------------------------------------------------------
    //--- KX132 Communication - Main Program Loop  ----------------------
    //-------------------------------------------------------------------

    pthread_create( &threadMainLoop, NULL, kx132_main_loop, &kx132_config);

    #ifdef TCP_SERVER
        pthread_create( &threadRuntimeConfig, NULL, kx132_runtime_config, &kx132_config);
    #endif //TCP_SERVER

    pthread_join(threadMainLoop, NULL);

    #ifdef TCP_SERVER
        pthread_join(threadRuntimeConfig, NULL);
    #endif //TCP_SERVER



    // turn KX132 off
    kx132_software_reset();

    #ifdef TCP_SERVER
    tcp_server_close();
    #endif

    spi_deinit();

    printf("\n");
    printf("---------------------------------\n");
    printf("---  PROGRAM FINISHED  ----------\n");
    printf("---------------------------------\n");
    printf("\n");

    return 0;
}

