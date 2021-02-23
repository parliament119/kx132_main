#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <i2c_wrapper.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>



const char *i2c_name = "/dev/i2c-1";
static int kx132_fd = -1;
const static int kx132_i2c_address = 0x1f;


int i2c_init(void){
    if((kx132_fd = open(i2c_name, O_RDWR)) < 0){
        char err[200];
        sprintf(err, "open ('%s') in i2c_init", i2c_name);
        perror(err);
        return -1;
    }

    return kx132_fd;
}

void i2c_close(void){
    close(kx132_fd);
}

int i2c_write(uint8_t reg, uint8_t data){
    int retval;
    uint8_t outbuf[2];

    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data msgset[1];

    outbuf[0] = reg;
    outbuf[1] = data;

    msgs[0].addr    = kx132_i2c_address;
    msgs[0].flags   = 0;
    msgs[0].len     = 2;
    msgs[0].buf     = outbuf;

    msgset[0].msgs  = msgs;
    msgset[0].nmsgs = 1;

    if(ioctl(kx132_fd, I2C_RDWR, &msgset) < 0){
        perror("ioctl(I2C_RDWR) in i2c_write");
        return -1;
    }

    return 0;
}


int i2c_read(uint8_t reg, uint8_t* result){
    int retval;
    uint8_t outbuf[1];

    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset[1];



    msgs[0].addr    = kx132_i2c_address;
    msgs[0].flags   = 0;
    msgs[0].len     = 1;
    msgs[0].buf     = outbuf;

    msgs[1].addr    = kx132_i2c_address;
    msgs[1].flags   = I2C_M_RD; //| I2C_M_NOSTART;
    msgs[1].len     = 1;
    msgs[1].buf     = result;

    msgset[0].msgs  = msgs;
    msgset[0].nmsgs = 2;

    outbuf[0] = reg;

    if(ioctl(kx132_fd, I2C_RDWR, &msgset) < 0){
        perror("ioctl(I2C_RDWR) in i2c_read");
        return -1;
    }

    return 0;
}


int i2c_burst_read(uint8_t reg, uint8_t* result, uint8_t size){
    int retval;
    uint8_t outbuf[1];

    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset[1];

    msgs[0].addr    = kx132_i2c_address;
    msgs[0].flags   = 0;
    msgs[0].len     = 1;
    msgs[0].buf     = outbuf;

    msgs[1].addr    = kx132_i2c_address;
    msgs[1].flags   = I2C_M_RD;
    msgs[1].len     = size;
    msgs[1].buf     = result;

    msgset[0].msgs  = msgs;
    msgset[0].nmsgs = 2;

    outbuf[0] = reg;

    *result = 0;  

    if(ioctl(kx132_fd, I2C_RDWR, &msgset) < 0){
        perror("ioctl(I2C_RDWR) in i2c_read");
        return -1;
    }

    return 0;
}

