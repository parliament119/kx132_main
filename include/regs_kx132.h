/**
 * @file regs_kx132.h
 * @author awa
 * @date 19-02-2021
 * 
 * @brief This file contains the definitions of registers for the Kionix KX132-Accelerometer.
 * 
 */

#ifndef REGS_KX132_H
#define REGS_KX132_H

/**
 * @brief KX132 Registers
 * 
 */
typedef enum {
    MAN_ID_REG_ADDR         	= 0x00,
    PART_ID_REG_ADDR        	= 0x01,
    XADP_L_REG_ADDR         	= 0x02,
    XADP_H_REG_ADDR         	= 0x03,
    YADP_L_REG_ADDR         	= 0x04,
    YADP_H_REG_ADDR         	= 0x05,
    ZADP_L_REG_ADDR         	= 0x06,
    ZADP_H_REG_ADDR         	= 0x07,
    XOUT_L_REG_ADDR         	= 0x08,
    XOUT_H_REG_ADDR         	= 0x09,
    YOUT_L_REG_ADDR         	= 0x0A,
    YOUT_H_REG_ADDR         	= 0x0B,
    ZOUT_L_REG_ADDR         	= 0x0C,
    ZOUT_H_REG_ADDR         	= 0x0D,
    // 0x0E - 0x11
    COTR_REG_ADDR           	= 0x12,
    WHO_AM_I_REG_ADDR       	= 0x13,
    TSCP_REG_ADDR           	= 0x14,
    TSPP_REG_ADDR           	= 0x15,
    INS1_REG_ADDR           	= 0x16,
    INS2_REG_ADDR           	= 0x17,
    INS3_REG_ADDR           	= 0x18,
    STATUS_REG_REG_ADDR     	= 0x19,
    INT_REL_REG_ADDR        	= 0x1A,
    CNTL1_REG_ADDR          	= 0x1B,
    CNTL2_REG_ADDR          	= 0x1C,
    CNTL3_REG_ADDR          	= 0x1D,
    CNTL4_REG_ADDR          	= 0x1E,
    CNTL5_REG_ADDR          	= 0x1F,
    CNTL6_REG_ADDR          	= 0x20,
    ODCNTL_REG_ADDR         	= 0x21,
    INC1_REG_ADDR           	= 0x22,
    INC2_REG_ADDR           	= 0x23,
    INC3_REG_ADDR           	= 0x24,
    INC4_REG_ADDR           	= 0x25,
    INC5_REG_ADDR           	= 0x26,
    INC6_REG_ADDR           	= 0x27,
    // 0x28 
    TILT_TIMER_REG_ADDR     	= 0x29,
    TDTRC_REG_ADDR          	= 0x2A,
    TDTC_REG_ADDR           	= 0x2B,
    TTH_REG_ADDR            	= 0x2C,
    TTL_REG_ADDR            	= 0x2D,
    FTD_REG_ADDR            	= 0x2E,
    STD_REG_ADDR            	= 0x2F,
    TLT_REG_ADDR            	= 0x30,
    TWS_REG_ADDR            	= 0x31,
    FFTH_REG_ADDR           	= 0x32,
    FFC_REG_ADDR            	= 0x33,
    FFCNTL_REG_ADDR         	= 0x34,
    // 0x35 - 0x36   
    TILT_ANGLE_LL_REG_ADDR  	= 0x37,
    TILT_ANGLE_HL_REG_ADDR  	= 0x38,
    HYST_SET_REG_ADDR       	= 0x39,
    LP_CNTL1_REG_ADDR       	= 0x3A,
    LP_CNTL2_REG_ADDR       	= 0x3B,
    // 0x3C - 0x48
    WUFTH_REG_ADDR          	= 0x49,
    BTSWUFTH_REG_ADDR       	= 0x4A,
    BTSTH_REG_ADDR          	= 0x4B,
    BTSC_REG_ADDR           	= 0x4C,
    WUFC_REG_ADDR           	= 0x4D,
    // 0x4E - 0x5C  
    SELF_TEST_REG_ADDR      	= 0x5D,
    BUF_CNTL1_REG_ADDR      	= 0x5E,
    BUF_CNTL2_REG_ADDR      	= 0x5F,
    BUF_STATUS1_REG_ADDR    	= 0x60,
    BUF_STATUS2_REG_ADDR    	= 0x61,
    BUF_CLEAR_REG_ADDR      	= 0x62,
    BUF_READ_REG_ADDR       	= 0x63,
    ADP_CNTL1_REG_ADDR      	= 0x64,
    ADP_CNTL2_REG_ADDR      	= 0x65,
    ADP_CNTL3_REG_ADDR      	= 0x66,
    ADP_CNTL4_REG_ADDR      	= 0x67,
    ADP_CNTL5_REG_ADDR      	= 0x68,
    ADP_CNTL6_REG_ADDR      	= 0x69,
    ADP_CNTL7_REG_ADDR      	= 0x6A,
    ADP_CNTL8_REG_ADDR      	= 0x6B,
    ADP_CNTL9_REG_ADDR      	= 0x6C,
    ADP_CNTL10_REG_ADDR     	= 0x6D,
    ADP_CNTL11_REG_ADDR     	= 0x6E,
    ADP_CNTL12_REG_ADDR     	= 0x6F,
    ADP_CNTL13_REG_ADDR     	= 0x70,
    ADP_CNTL14_REG_ADDR     	= 0x71,
    ADP_CNTL15_REG_ADDR     	= 0x72,
    ADP_CNTL16_REG_ADDR     	= 0x73,
    ADP_CNTL17_REG_ADDR     	= 0x74,
    ADP_CNTL18_REG_ADDR     	= 0x75,
    ADP_CNTL19_REG_ADDR     	= 0x76
    // 0x77 - 0x7F
} kx132_reg_t;




#endif //REGS_KX132_H
