#ifndef IIC_SIM_API_H
#define IIC_SIM_API_H

#define FALSE	0
#define TRUE		1
#define DEV_ADDR	0x88
//////io control command 
#define I2C_CMD_BASE_ADDR		0x9000
#define I2C_BUS_INIT				I2C_CMD_BASE_ADDR + 1
#define I2C_READ_BYTE			I2C_CMD_BASE_ADDR + 2
#define I2C_WRITE_BYTE			I2C_CMD_BASE_ADDR + 3
//#define I2C_BUS_STOP			I2C_CMD_BASE_ADDR + 4
struct iic_dev_struct{
	unsigned char dev_addr;
	unsigned char reg_addr;
	unsigned char buf[2];
};
#endif

