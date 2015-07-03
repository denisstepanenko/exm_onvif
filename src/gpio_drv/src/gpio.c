#include <linux/module.h>
#include <asm/hardware.h>
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
#include <linux/kernel.h>
#include "hi_gpio.h"

#define  GPIO_0_BASE_ADDR  0x20150000
#define  GPIO_1_BASE_ADDR  0x20160000
#define  GPIO_2_BASE_ADDR  0x20170000
#define  GPIO_3_BASE_ADDR  0x20180000
#define  GPIO_4_BASE_ADDR  0x20190000
#define  GPIO_5_BASE_ADDR  0x201a0000
#define  GPIO_6_BASE_ADDR  0x201b0000
#define  GPIO_7_BASE_ADDR  0x201c0000

#define  GPIO_DIR_BASE   (groupbase+0x400)
#define  GPIO_INTR_MASK  (groupbase+0x410)
#define  GPIO_DATA_BASE   data_reg_base


#define WRITE_REG(Addr, Value) ((*(volatile unsigned int *)(Addr)) = (Value))
#define READ_REG(Addr)         (*(volatile unsigned int *)(Addr))

static DECLARE_MUTEX(gpio_sem);

unsigned int groupbase=-1;
unsigned int data_reg_base=0;
unsigned int gpio_0_base_addr_virtual=0;
unsigned int gpio_1_base_addr_virtual=0;
unsigned int gpio_2_base_addr_virtual=0;
unsigned int gpio_3_base_addr_virtual=0;
unsigned int gpio_4_base_addr_virtual=0;
unsigned int gpio_5_base_addr_virtual=0;
unsigned int gpio_6_base_addr_virtual=0;
unsigned int gpio_7_base_addr_virtual=0;

static unsigned char gpio_port_newvalue = 0xff;
//static unsigned char gpio_port_oldvalue = 0xff;
static unsigned int old_bit_value[8];   //保存每一个端子上次一的值
static unsigned int old_bit_value_1[8]; //当有报警动作时候保存端子的值

static unsigned char alarmFlag[8] = {0}; //每一个端子的报警标志
static int beginToEndFlag[8];		//设置开始到恢复的标志(0-start to end  1-end to start)
static TimeDelay timeDelay[8];		//存储报警触发的延时时间
static unsigned char timeRemain[8]; //报警触发的延时时间


static void hi3515_button_pin_cfg(void)
{

unsigned int reg_virtual_addr = (unsigned int)ioremap_nocache(0x200f0000,0x4000);
if(!reg_virtual_addr)
{
    printk("0x200f0000 ioremap addr failed !\n");
    return -1;
}
	/*配置作为普通输入*/
	WRITE_REG(reg_virtual_addr + 0x08,0x1);/*reg2管脚复用配置gpio3_0,按键1*/
	WRITE_REG(reg_virtual_addr + 0x0c,0x1);/*reg3管脚复用配置gpio3_1,按键1*/
	WRITE_REG(reg_virtual_addr + 0x10,0x1);/*reg4管脚复用配置gpio3_2,按键1*/
	WRITE_REG(reg_virtual_addr + 0x14,0x1);/*reg5管脚复用配置gpio3_3，按键2*/
	WRITE_REG(reg_virtual_addr + 0x18,0x1);/*reg5管脚复用配置gpio3_3，按键2*/
	WRITE_REG(reg_virtual_addr + 0x1c,0x1);/*reg5管脚复用配置gpio3_3，按键2*/
	WRITE_REG(reg_virtual_addr + 0x20,0x1);/*reg5管脚复用配置gpio3_3，按键2*/
	WRITE_REG(reg_virtual_addr + 0x24,0x1);/*reg5管脚复用配置gpio3_3，按键2*/

	WRITE_REG(reg_virtual_addr + 0x30,0x1);    	//ret4-0
	WRITE_REG(reg_virtual_addr + 0x34,0x1);
	WRITE_REG(reg_virtual_addr + 0x38,0x1);
	WRITE_REG(reg_virtual_addr + 0x3c,0x1);
	WRITE_REG(reg_virtual_addr + 0x40,0x1);
	WRITE_REG(reg_virtual_addr + 0x48,0x1);
	WRITE_REG(reg_virtual_addr + 0x4c,0x1);
	WRITE_REG(reg_virtual_addr + 0x50,0x1);
	WRITE_REG(reg_virtual_addr + 0x54,0x1); 	//reg5-0
	WRITE_REG(reg_virtual_addr + 0x58,0x1);
	WRITE_REG(reg_virtual_addr + 0x5c,0x1);
	WRITE_REG(reg_virtual_addr + 0x60,0x1);
	WRITE_REG(reg_virtual_addr + 0x64,0x1);
	WRITE_REG(reg_virtual_addr + 0x68,0x1);
	WRITE_REG(reg_virtual_addr + 0x6c,0x1);
}

static void gpio_calculate_data_groupbase(unsigned int groupnum, unsigned int bitnum)
{
	switch(groupnum)
    {
	case 0: 
	     groupbase =gpio_0_base_addr_virtual;
	     break;
	case 1: 
	     groupbase =gpio_1_base_addr_virtual;
	     break;
	case 2: 
	     groupbase =gpio_2_base_addr_virtual;
	     break;
	case 3: 
	     groupbase =gpio_3_base_addr_virtual;
	     break;
	case 4: 
	     groupbase =gpio_4_base_addr_virtual;
	     break;
	case 5: 
	     groupbase =gpio_5_base_addr_virtual;
	     break;
	case 6: 
	     groupbase =gpio_6_base_addr_virtual;
	     break;
	case 7: 
	     groupbase =gpio_7_base_addr_virtual;
	     break;
	default:
	     break;
    }

    //printk("groupbase:%x !\n",groupbase);
    data_reg_base=groupbase+(1<<(bitnum+2));
    //printk("data_reg_base:%x !\n",data_reg_base);
}

static int gpio_open(struct inode *inode, struct file *filp)
{
   return 0;		
}

static int gpio_release(struct inode *inode, struct file *filp)
{
	return 0;	
}

static int gpio_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	gpio_groupbit_info group_bit_info;
    unsigned int reg_tmp=0;
	unsigned int reg_tmp1=0;
	TimeDelay delay_time;		
    down_interruptible(&gpio_sem);
    switch(cmd)
    {
        case GPIO_SET_DIR:
            copy_from_user(&group_bit_info,(gpio_groupbit_info*)arg, sizeof(gpio_groupbit_info));
		    if((group_bit_info.groupnumber>7)||(group_bit_info.bitnumber>7))
		    {
				printk("group number or bitnum beyond extent!\n");
				up(&gpio_sem);
				return -1;
		    }
	        gpio_calculate_data_groupbase(group_bit_info.groupnumber,group_bit_info.bitnumber);
		    reg_tmp=READ_REG(GPIO_DIR_BASE);

		    if(group_bit_info.value==0)
		    {
				reg_tmp &=~(1<<group_bit_info.bitnumber);
				WRITE_REG(GPIO_DIR_BASE,reg_tmp);
		    }
		    else if(group_bit_info.value==1)
		    {
				reg_tmp |=(1<<group_bit_info.bitnumber);
				WRITE_REG(GPIO_DIR_BASE,reg_tmp);
		    }
		    else
		    {
				printk("dir beyond of extent!\n");
				up(&gpio_sem);
				return -1;
		    }
	    	break;
        case GPIO_GET_DIR:
            copy_from_user(&group_bit_info,(gpio_groupbit_info*)arg, sizeof(gpio_groupbit_info));
		    if((group_bit_info.groupnumber>7)||(group_bit_info.bitnumber>7))
		    {
				printk("group number or bitnum beyond extent!\n");
				up(&gpio_sem);
				return -1;
		    }
            gpio_calculate_data_groupbase(group_bit_info.groupnumber,group_bit_info.bitnumber);
		    reg_tmp=READ_REG(GPIO_DIR_BASE);
			reg_tmp &=(1<<group_bit_info.bitnumber);
			if(reg_tmp!=0)
		    {
				group_bit_info.value=1;
		    }
		    else
		    {
				group_bit_info.value=0;
		    }
            copy_to_user((void __user *)arg, &group_bit_info, sizeof(gpio_groupbit_info));
			break;
		case GPIO_READ_BIT:
	    	copy_from_user(&group_bit_info,(gpio_groupbit_info*)arg, sizeof(gpio_groupbit_info));
	    	if((group_bit_info.groupnumber>7)||(group_bit_info.bitnumber>7))
	    	{
				printk("group number or bitnum beyond extent!\n");
				up(&gpio_sem);
				return -1;
	   	 	}
	    	gpio_calculate_data_groupbase(group_bit_info.groupnumber,group_bit_info.bitnumber);
	    	reg_tmp=READ_REG(GPIO_DATA_BASE);
			reg_tmp1 = reg_tmp;
			if (group_bit_info.bitnumber == 4)
			{
				printk(KERN_INFO "old_bit_value:%02x group_bit_info.bitnumber:%02x reg_tmp:%02x\n", old_bit_value[group_bit_info.bitnumber], group_bit_info.bitnumber, reg_tmp);
			}
			if (old_bit_value[group_bit_info.bitnumber] != reg_tmp)
			{
				//printk(KERN_INFO "timeRemain: %02x  group_bit_info.bitnumber: %02x  beginToEndFlag[group_bit_info.bitnumber]: %02x\n", timeRemain[group_bit_info.bitnumber], group_bit_info.bitnumber, beginToEndFlag[group_bit_info.bitnumber]);
				if (timeRemain[group_bit_info.bitnumber] != 0)        //防抖动处理
				{
					alarmFlag[group_bit_info.bitnumber] = 0;
					timeRemain[group_bit_info.bitnumber] = 0;
				}
				else
				{
					if (beginToEndFlag[group_bit_info.bitnumber] == 0)
					{
						if (gpio_port_newvalue & (1 << group_bit_info.bitnumber))
						{
							timeRemain[group_bit_info.bitnumber] = timeDelay[group_bit_info.bitnumber].beginToEnd;
						}
						else
						{
							timeRemain[group_bit_info.bitnumber] = timeDelay[group_bit_info.bitnumber].endToBegin;
						}
					}
					else
					{
						if (gpio_port_newvalue & (1 << group_bit_info.bitnumber))
						{
							timeRemain[group_bit_info.bitnumber] = timeDelay[group_bit_info.bitnumber].endToBegin;
						}
						else
						{
							timeRemain[group_bit_info.bitnumber] = timeDelay[group_bit_info.bitnumber].beginToEnd;
						}
					}
					alarmFlag[group_bit_info.bitnumber] = 1;
				}
			}

			if (timeRemain[group_bit_info.bitnumber] != 0)
			{
				timeRemain[group_bit_info.bitnumber]--;
			}
			/*
			if (group_bit_info.bitnumber == 4)
			{
				printk(KERN_INFO "timeRemain[group_bit_info.bitnumber]: %02x alarmFlag: %02x     a:%02x\n", timeRemain[group_bit_info.bitnumber], alarmFlag[group_bit_info.bitnumber], gpio_port_newvalue & (1 << group_bit_info.bitnumber));
			}
			*/
			if (gpio_port_newvalue & (1 << group_bit_info.bitnumber))  //此位为常闭
			{
				old_bit_value[group_bit_info.bitnumber] = reg_tmp;
				
				if ((alarmFlag[group_bit_info.bitnumber] == 1) && (timeRemain[group_bit_info.bitnumber] == 0))
				{
					if (beginToEndFlag[group_bit_info.bitnumber] == 0)
					{
						beginToEndFlag[group_bit_info.bitnumber] = 1;
					}
					else
					{
						beginToEndFlag[group_bit_info.bitnumber] = 0;
					}
					alarmFlag[group_bit_info.bitnumber] = 0;
					old_bit_value_1[group_bit_info.bitnumber] = reg_tmp;
					reg_tmp &= (gpio_port_newvalue & (1 << group_bit_info.bitnumber));
				}
				else
				{
					reg_tmp = old_bit_value_1[group_bit_info.bitnumber] & (gpio_port_newvalue & (1 << group_bit_info.bitnumber));
				}

				reg_tmp &= (gpio_port_newvalue & (1 << group_bit_info.bitnumber));
			}
			else
			{
				old_bit_value[group_bit_info.bitnumber] = reg_tmp;

				if ((alarmFlag[group_bit_info.bitnumber] == 1) && (timeRemain[group_bit_info.bitnumber] == 0))
				{
					if (beginToEndFlag[group_bit_info.bitnumber] == 0)
					{
						beginToEndFlag[group_bit_info.bitnumber] = 1;
					}
					else
					{
						beginToEndFlag[group_bit_info.bitnumber] = 0;
					}
					alarmFlag[group_bit_info.bitnumber] = 0;
					old_bit_value_1[group_bit_info.bitnumber] = reg_tmp;
					
					reg_tmp = 1 << group_bit_info.bitnumber;
					printk(KERN_INFO "#####:%02x %02x\n", gpio_port_newvalue, reg_tmp);
				}
				else
				{
					reg_tmp = old_bit_value_1[group_bit_info.bitnumber] & (gpio_port_newvalue & (1 << group_bit_info.bitnumber));
				}
				if (((reg_tmp1 == 0) && ((gpio_port_newvalue & (1 << group_bit_info.bitnumber)) == 0)) || (timeRemain[group_bit_info.bitnumber] != 0))
				{
					reg_tmp = 1 << group_bit_info.bitnumber;
				}
				
			}
			/*
			if (group_bit_info.bitnumber == 4)
			{
				printk(KERN_INFO "ssss:%02x\n", reg_tmp);
			}
			*/
		    group_bit_info.value = (unsigned char)reg_tmp;
	        copy_to_user((void __user *)arg, &group_bit_info, sizeof(gpio_groupbit_info));
			break; 
		case GPIO_WRITE_BIT:
		    copy_from_user(&group_bit_info,(gpio_groupbit_info*)arg, sizeof(gpio_groupbit_info));

		    if((group_bit_info.groupnumber>7)||(group_bit_info.bitnumber>7))
		    {
				printk(KERN_INFO "group number or bitnum beyond extent!\n");
				up(&gpio_sem);
				return -1;
		    }
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
			break;
		case GPIO_SET_STATUS:
			copy_from_user(&gpio_port_newvalue, arg, sizeof(unsigned char));
			break;
		case GPIO_SET_DELAY:
			copy_from_user(&delay_time, (TimeDelay *)arg, sizeof(TimeDelay));

			//printk(KERN_INFO "delay_time.beginToEnd:%02x delay_time.endToBegin:%02x\n", delay_time.beginToEnd, delay_time.endToBegin);
			timeDelay[delay_time.bitnumber].beginToEnd = delay_time.beginToEnd;
			timeDelay[delay_time.bitnumber].endToBegin = delay_time.endToBegin;

			break;
	    default:
			up(&gpio_sem);
	        return -1;           
    }
    up(&gpio_sem);

    return 0;

}

static struct file_operations gpio_fops = {
  owner:THIS_MODULE,
  open:gpio_open,
  ioctl:gpio_ioctl,
  release:gpio_release,
};

static struct miscdevice gpio_dev = {
    MISC_DYNAMIC_MINOR,
    "hi_gpio",
    &gpio_fops,
};



//gpio的复用关系要放在uboot下面；
static int __init hi_gpio_init(void)
{
	signed int  ret=0;
	int i;
    ret = misc_register(&gpio_dev);
    if (ret)
    {
        printk(KERN_ERR "register misc dev for i2c fail!\n");
		return ret;
	}
	
	hi3515_button_pin_cfg();    //将寄存器芯片的地址映射到内存只能在模块加载内核中完成
	gpio_0_base_addr_virtual=(unsigned int)ioremap_nocache(GPIO_0_BASE_ADDR,0x40000);
	if(!gpio_0_base_addr_virtual)
	{
	    printk("ioremap gpio group0 failed!\n");
	    return -1;
	}
	gpio_1_base_addr_virtual=gpio_0_base_addr_virtual+0x10000;
	gpio_2_base_addr_virtual=gpio_0_base_addr_virtual+0x20000;
	gpio_3_base_addr_virtual=gpio_0_base_addr_virtual+0x30000;
	gpio_4_base_addr_virtual=(unsigned int) ioremap_nocache(GPIO_4_BASE_ADDR,0x40000);
	if(!gpio_4_base_addr_virtual)
	{
	    printk("ioremap gpio group0 failed!\n");
	    iounmap((void*)gpio_0_base_addr_virtual);
	    return -1;
	}

	gpio_5_base_addr_virtual=gpio_4_base_addr_virtual+0x10000;
	gpio_6_base_addr_virtual=gpio_4_base_addr_virtual+0x20000;
	gpio_7_base_addr_virtual=gpio_4_base_addr_virtual+0x30000;

	for (i = 0; i < 8; i++)
	{
		old_bit_value[i] = 1 << i;
		old_bit_value_1[i] = 1 << i;
	}
	return 0;         
}


static void __exit hi_gpio_exit(void)
{
    misc_deregister(&gpio_dev);
    iounmap((void*)gpio_0_base_addr_virtual);
    iounmap((void*)gpio_4_base_addr_virtual);

}

module_init(hi_gpio_init);
module_exit(hi_gpio_exit);

MODULE_AUTHOR("Digital Media Team ,Hisilicon crop ");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Real Time Clock interface for HI3511");


