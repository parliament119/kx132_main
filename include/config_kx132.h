/**
 * @file config_kx132.h
 * @author awa
 * @date 19-02-2021
 * 
 * @brief header for config_kx132.h
 * 
 *  function declarations needed for config_kx132.c and 
 *  typedefs and structs used for config
 * 
 */


//-------------------------------------------------------------------
//--- Macros  -------------------------------------------------------
//-------------------------------------------------------------------

#ifndef CONFIG_KX132_H
#define CONFIG_KX132_H

#include <stdint.h>
#include <trigger.h>


#define RESO_8_BIT              8
#define RESO_16_BIT             16

#define G_RANGE_HW_2            0x00
#define G_RANGE_HW_4            0x08
#define G_RANGE_HW_8            0x10
#define G_RANGE_HW_16           0x18

#define BUFFER_SIZE_128_KB      131072     // 2^17
#define BUFFER_SIZE_256_KB      262144     // 2^18
#define BUFFER_SIZE_512_KB      524288     // 2^19
#define BUFFER_SIZE_1024_KB     1048576    // 2^20
#define BUFFER_SIZE_2048_KB     2097152    // 2^21
#define BUFFER_SIZE_4096_KB     4194304    // 2^22
#define BUFFER_SIZE_8192_KB     8388608    // 2^23


//-------------------------------------------------------------------
//--- Typedefs  -----------------------------------------------------
//-------------------------------------------------------------------

///< emum for hardware config of output data rate (frequency)
typedef enum{
    odr_0_781_Hz            = 0x0,
    odr_1_563_Hz            = 0x1,
    odr_3_125_Hz            = 0x2,
    odr_6_25_Hz             = 0x3,
    odr_12_5_Hz             = 0x4,
    odr_25_Hz               = 0x5,
    odr_50_Hz               = 0x6,
    odr_100_Hz              = 0x7,
    odr_200_Hz              = 0x8,
    odr_400_Hz              = 0x9,
    odr_800_Hz              = 0xA,
    odr_1600_Hz             = 0xB,
    odr_3200_Hz             = 0xC,
    odr_6400_Hz             = 0xD,
    odr_12800_Hz            = 0xE,
    odr_25600_Hz            = 0xF,
} outputDataRate_hw_t;


///< enum for hardware config of resolution (not implemented)
typedef enum{
    resolution_8bit         = RESO_8_BIT,
    resolution_16bit        = RESO_16_BIT,
} resolution_hw_t;


///< enum for hardware config of read mode (async/sync)
typedef enum{
    synchronous_read_0   = 0,                       ///< No Hardware Interrupt. Syncing through SPI. Uses DataReady-Bit in INS2_REG.
//  synchronous_read_1   = 1,                       ///< Hardware Interrupt. Syncing through INT1-PIN of KX132. //! Not implemented.
    asynchronous_read    = 2,                       ///< Asynchronous Read of Raw Data.
} readMode_hw_t;


///< enum for hardware config of g-range (sensitivity)
typedef enum{
    g_range_2g              = G_RANGE_HW_2,
    g_range_4g              = G_RANGE_HW_4,
    g_range_8g              = G_RANGE_HW_8,
    g_range_16g             = G_RANGE_HW_16,
} gRange_hw_t;


///< enum for software config of use mode (streaming/trigger)
typedef enum{
    streaming_mode          = 0,
    triggered_mode          = 1
} useMode_t;


/// struct holding configuration used for initializing KX132 and software configuration during runtime
typedef struct{
    outputDataRate_hw_t     outputDataRate_hw;      ///< frequency-setting for KX132 (Range: 0x0 - 0xF)
    resolution_hw_t         resolution_hw;          ///< resolution of KX132 output (8-Bit/16-Bit) //! Not implemented.
    readMode_hw_t           readMode_hw;            ///< depending on config turns "Data Ready Engine" on or off
    gRange_hw_t             gRange_hw;              ///< sensitivity of KX132 (higher g >> lower sensitivity)
    useMode_t               useMode;                ///< streaming / trigger
    uint32_t                bufferSize;             ///< buffersize for allocating memory of ringbuffer
} main_config_t;


/// struct holding every other config struct for passing to threaded functions
typedef struct{
    main_config_t*          mainConfig;
    trigger_config_t*       triggerConfig;
    trigger_data_t*         triggerData;
} kx132_config_t;



//-------------------------------------------------------------------
//--- Function Declarations  ----------------------------------------
//-------------------------------------------------------------------

/**
 * @brief Initialiazes all settings corresponding to user input or default values. 
 * 
 * @param argc              console input argument count passed from main()
 * @param argv              console input arguments passed from main()
 * @param kx132_config      pointer to struct containg all configuration settings
 */
void kx132_config_init(uint16_t argc, char *argv[], kx132_config_t *kx132_config);


/**
 * @brief Parser for console user input at start of program.
 * 
 * @param argc              console input argument count passed from main()
 * @param argv              console input arguments passed from main()
 * @param mainConfig        pointer to struct containing hardware + software setting
 * @param triggerConfig     pointer to struct containing trigger settings
 * @param triggerData       pointer to struct containing trigger data (thresholds + normalized)
 */
void processInitFlags(  uint16_t            argc,
                        char                *argv[],
                        main_config_t       *mainConfig,
                        trigger_config_t    *triggerConfig,
                        trigger_data_t      *triggerData);


/**
 * @brief Parser for tcp user input during runtime.
 * 
 * @param data              pointer to string containing user input
 * @param triggerConfig     pointer to struct containing trigger setting for changing
 * @param triggerData       pointer to struct containing trigger data for changing
 * @return true             if an "exit"-message was sent, will terminate program
 * @return false            if no "exit"-message was sent
 */
bool processRuntimeFlags(char *data, trigger_config_t *triggerConfig, trigger_data_t *triggerData);


/**
 * @brief Calculate and set the offset-thresholds.
 * 
 *  Takes normalized data and applies the threshold as an offset
 *      to create the positive and negative thresholds.
 * 
 * @param triggerData       pointer to struct containing the normalized axis data and thresholds
 */
void setOffsetThresholds(trigger_data_t* triggerData);


/**
 * @brief Calculates normalized values for each axis.
 * 
 *  Reads n (Default=5000) samples and creates an average.
 * 
 * @param readMode          readMode Flag to read through sync0/async
 * @param triggerData       pointer to struct containing the normalized data
 */
void normalizeThresholds(readMode_hw_t readMode ,trigger_data_t* triggerData);



#endif // CONFIG_KX132_H