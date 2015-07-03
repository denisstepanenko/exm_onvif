#ifndef LEDS_H
#define LEDS_H

#include <linux/module.h>
//#include <asm/hardware.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/fcntl.h>

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>

#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/io.h>
#include "hi_gpio.h"

/*
	err:GPIO2_3
	net:GPIO2_5
	alarm:GPIO2_7
*/

#define  GPIO_0_BASE_ADDR  0x20150000
#define  GPIO_1_BASE_ADDR  0x20160000
#define  GPIO_2_BASE_ADDR  0x20170000
#define  GPIO_3_BASE_ADDR  0x20180000
#define  GPIO_4_BASE_ADDR  0x20190000
#define  GPIO_5_BASE_ADDR  0x201a0000
#define  GPIO_6_BASE_ADDR  0x201b0000
#define  GPIO_7_BASE_ADDR  0x201c0000
#define  GPIO_8_BASE_ADDR  0x201d0000


#define  GPIO_DIR_BASE   (groupbase+0x400)
#define  GPIO_INTR_MASK  (groupbase+0x410)
#define  GPIO_DATA_BASE   data_reg_base

#define WRITE_REG(Addr, Value) ((*(volatile unsigned int *)(Addr)) = (Value))
#define READ_REG(Addr)         (*(volatile unsigned int *)(Addr))


static DEFINE_SEMAPHORE(gpio_sem);  /* 根据linux3.0.y 的源码，DEFINE_SEMAPHORE取代了DECLARE_MUTEX */

//static DECLARE_MUTEX(gpio_sem);

unsigned int groupbase=-1;
unsigned int data_reg_base=0;
unsigned int gpio_2_base_addr_virtual=0;
//unsigned int gpio_9_base_addr_virtual=0;
//unsigned int gpio_10_base_addr_virtual=0;
//unsigned int gpio_11_base_addr_virtual=0;


static void gpio_calculate_data_groupbase(unsigned int groupnum, unsigned int bitnum)
{
/*
	switch (groupnum) {
	case 2:
		groupbase = gpio_2_base_addr_virtual;
		break;
	default:
		break;
	}
	*/
	if(groupnum==2)
		groupbase=gpio_2_base_addr_virtual;

//    printk("groupbase:%x !\n",groupbase);
	data_reg_base = groupbase + (1 << (bitnum + 2));
//    printk("data_reg_base:%x !\n",data_reg_base);
}

static void hi3520d_button_pin_cfg(void)
{
		/*映射复用寄存器地址*/
		unsigned int reg_virtual_addr = (unsigned int)ioremap_nocache(0x200f0000, 0x0c8);
    	if(!reg_virtual_addr)
    	{
        	printk("0x200f0000 ioremap addr failed !\n");
        	return;
    	}

    	/*配置作为gpio使用*/
        WRITE_REG(reg_virtual_addr + 0x08c,0x0);/*GPIO2_2*/

}

static __inline__ void set_net_led_value(int value)//将net灯输出指定值 gpiob1
{
	//gm8180_gpiob_setpin(GM8180_GPIOB(5),1-value);
	down_interruptible(&gpio_sem);
	gpio_groupbit_info group_bit_info;
	group_bit_info.groupnumber = 2;
	group_bit_info.bitnumber = 2;
	group_bit_info.value = value;
	gpio_calculate_data_groupbase(group_bit_info.groupnumber,group_bit_info.bitnumber);
	if(group_bit_info.value==0)
	{
	 	WRITE_REG(GPIO_DATA_BASE,0);
        //printk("1addr:0x%x vaule:%x!\n",GPIO_DATA_BASE,*((unsigned int *)GPIO_DATA_BASE));		
	}
	else if(group_bit_info.value==1)
	{
	    WRITE_REG(GPIO_DATA_BASE,1<<group_bit_info.bitnumber);
        //printk("2addr:0x%x vaule:%x!\n",GPIO_DATA_BASE,*((unsigned int *)GPIO_DATA_BASE));		
	}
	else
	{
	  	printk("write bit beyond of extent!\n");
	   	up(&gpio_sem);
	   	return -1;
	}

	up(&gpio_sem);

    return;
}

#if 0
static __inline__ void set_error_led_value(int value)//将error灯输出指定值 Gpiob0
{
	//gm8180_gpiob_setpin(GM8180_GPIOB(2),1-value);
	down_interruptible(&gpio_sem);
	gpio_groupbit_info group_bit_info;
	group_bit_info.groupnumber = 2;
	group_bit_info.bitnumber = 5;
	group_bit_info.value = value;
	gpio_calculate_data_groupbase(group_bit_info.groupnumber,group_bit_info.bitnumber);
	if(group_bit_info.value==0)
	{
	 	WRITE_REG(GPIO_DATA_BASE,0);
        //printk("1addr:0x%x vaule:%x!\n",GPIO_DATA_BASE,*((unsigned int *)GPIO_DATA_BASE));		
	}
	else if(group_bit_info.value==1)
	{
	    WRITE_REG(GPIO_DATA_BASE,1<<group_bit_info.bitnumber);
       // printk("2addr:0x%x vaule:%x!\n",GPIO_DATA_BASE,*((unsigned int *)GPIO_DATA_BASE));		
	}
	else
	{
	  	printk("write bit beyond of extent!\n");
	   	up(&gpio_sem);
	   	return;
	}

	up(&gpio_sem);

    return;
}

static __inline__ void set_alarm_led_value(int value)//将alarm灯输出指定值 gpiob2
{
	//gm8180_gpiob_setpin(GM8180_GPIOB(3),1-value);
	down_interruptible(&gpio_sem);
	gpio_groupbit_info group_bit_info;
	group_bit_info.groupnumber = 2;
	group_bit_info.bitnumber = 7;
	group_bit_info.value = value;
	gpio_calculate_data_groupbase(group_bit_info.groupnumber,group_bit_info.bitnumber);
	if(group_bit_info.value==0)
	{
	 	WRITE_REG(GPIO_DATA_BASE,0);
        //printk("1addr:0x%x vaule:%x!\n",GPIO_DATA_BASE,*((unsigned int *)GPIO_DATA_BASE));		
	}
	else if(group_bit_info.value==1)
	{
	    WRITE_REG(GPIO_DATA_BASE,1<<group_bit_info.bitnumber);
        //printk("2addr:0x%x vaule:%x!\n",GPIO_DATA_BASE,*((unsigned int *)GPIO_DATA_BASE));		
	}
	else
	{
	  	printk("write bit beyond of extent!\n");
	   	up(&gpio_sem);
	   	return;
	}

	up(&gpio_sem);

    return;
}
#endif
static __inline__ void set_leds_output(void)//将管脚都设为输出并上拉
{
	unsigned int reg_tmp;
	
	gpio_groupbit_info group_bit_info;
	group_bit_info.groupnumber = 2;
	group_bit_info.bitnumber = 2;
	group_bit_info.value = 1;
	gpio_calculate_data_groupbase(group_bit_info.groupnumber,group_bit_info.bitnumber);
    reg_tmp=READ_REG(GPIO_DIR_BASE);
   	reg_tmp |=(1<<group_bit_info.bitnumber);
    WRITE_REG(GPIO_DIR_BASE,reg_tmp);
#if 0
	group_bit_info.groupnumber = 2;
	group_bit_info.bitnumber = 5;
	group_bit_info.value = 1;
	gpio_calculate_data_groupbase(group_bit_info.groupnumber,group_bit_info.bitnumber);
    reg_tmp=READ_REG(GPIO_DIR_BASE);
   	reg_tmp |=(1<<group_bit_info.bitnumber);
    WRITE_REG(GPIO_DIR_BASE,reg_tmp);

	group_bit_info.groupnumber = 2;
	group_bit_info.bitnumber = 7;
	group_bit_info.value = 1;
	gpio_calculate_data_groupbase(group_bit_info.groupnumber,group_bit_info.bitnumber);
    reg_tmp=READ_REG(GPIO_DIR_BASE);
   	reg_tmp |=(1<<group_bit_info.bitnumber);
    WRITE_REG(GPIO_DIR_BASE,reg_tmp);
#endif	
}
	


#endif

