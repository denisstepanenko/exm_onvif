#ifndef __HI_GPIO_H__ 
#define __HI_GPIO_H__

#define GPIO_DIR_IN 0
#define GPIO_DIR_OUT 1

#define GPIO_SET_DIR 0x1
#define GPIO_GET_DIR 0x2
#define GPIO_READ_BIT 0x3
#define GPIO_WRITE_BIT 0x4


//yk add 报警输入
#define GPIO_SET_INPUT 0x5 //设置报警输入常开常闭属性
#define GPIO_GET_INPUT 0X6 //读取报警输入常开常闭属性
#define GPIO_GET_DWORD 0X7 //读取报警输入状态
#define GPIO_SET_DELAY 0X8 //设置报警输入延时
#define GPIO_GET_DELAY 0X9 //读取报警输入延时
#define GPIO_SET_OUTPUT_VALUE 0x10 //设置报警输出io的值

typedef struct {
	unsigned int  groupnumber;
	unsigned int  bitnumber;
	unsigned int  value;
}gpio_groupbit_info;


typedef struct{
	unsigned int on_delay;//io变为高后的延时
	unsigned int off_delay;//io变为低后的延时
}TimeDelay;
#endif
