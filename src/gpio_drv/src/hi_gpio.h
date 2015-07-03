#ifndef __HI_GPIO_H__ 
#define __HI_GPIO_H__

#define GPIO_DIR_IN 0
#define GPIO_DIR_OUT 1

#define GPIO_SET_DIR 0x1
#define GPIO_GET_DIR 0x2
#define GPIO_READ_BIT 0x3
#define GPIO_WRITE_BIT 0x4
#define GPIO_SET_STATUS 0x5
#define GPIO_SET_DELAY 0x6

typedef struct {
	unsigned int  groupnumber;
	unsigned int  bitnumber;
	unsigned int  value;
}gpio_groupbit_info;

typedef struct TimeDelay
{
	unsigned char bitnumber;
        unsigned char beginToEnd;         //开始到恢复的延时
        unsigned char endToBegin;         //恢复到开始的延时
}TimeDelay;


#endif
