/**
 * @file drv_kx132.h
 * @author awa
 * @date 20-02-2021
 * 
 * @brief header for drv_kx132.c
 * 
 *  function declarations needed for drv_kx132.c
 * 
 */

#ifndef DRV_KX132_H
#define DRV_KX132_H

///cond
#include <stdint.h>
#include <stdbool.h>
///endcond

#include <config_kx132.h>


/**
 * @brief Initializes KX132 accelerometer with set parameters  
 * 
 * @return true         if successful
 * @return false        if error
 */
bool kx132_init(kx132_config_t* config);


/**
 * @brief Software Resets the KX132.
 * 
 */
void kx132_software_reset(void);


/**
 * @brief Reads the 6 Normal Axis Output Register of KX132 in async-mode 
 * 
 * @attention           Asynchronous reading is discouraged as it leads to a lot of duplicates in data.
 * 
 * @param xyzRawData    pointer to buffer where raw data should be saved
 */
void kx132_async_read_raw_data(uint8_t* xyzRawData);


/**
 * @brief  Reads the 6 Normal Axis Output Register of KX132 in software-sync-mode 
 * 
 * @param xyzRawData    pointer to buffer where raw data should be saved
 * @return true         if data was ready and could be read
 * @return false        if data was not ready
 */
bool kx_132_sync0_read_raw_data(uint8_t* xyzRawData);


/**
 * @brief Processes user input from tcp and changes trigger settings durting runtime.
 * 
 * @param kx_config pointer to struct containing needed triggerConfig- and triggerData-struct
 */
void *kx132_runtime_config(void *kx_config);


/**
 * @brief Calls either kx132_streaming_mode() or kx132_trigger_mode() for reading data from KX132.
 * 
 * @param kx_config pointer to main config-struct containing all relevant settings
 */
void *kx132_main_loop(void *kx_config);

#endif //DRV_KX132_H