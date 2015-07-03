/* buzzer driver, by lc 2013.4 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/types.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/ioctl.h>
#include <asm/segment.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>

#include <linux/proc_fs.h>
#include <linux/workqueue.h>

#include "hi_gpio.h"

#if HZ==100
#define RETURN_TIME 50//单位:1/100秒
#elif HZ==200
#define RETURN_TIME 100
#else
	#error "unsupport HZ value"
#endif


#define BUZZER_DEV       "buzzer" //设备节点名称
#define BUZZER_MAJOR     255 	//changed by shixin 254
#define BUZZER_NAME      "BUZZER"
#define BUZZER_VERSION   "Version 0.1"


static void buzzer_end(unsigned long arg);
static struct timer_list buzzer_timerlist=TIMER_INITIALIZER(buzzer_end, 0, 0);



#define  GPIO_1_BASE_ADDR  0x20160000
#define  GPIO_9_BASE_ADDR  0x201e0000

#define  GPIO_DIR_BASE   (groupbase+0x400)
#define  GPIO_INTR_MASK  (groupbase+0x410)
#define  GPIO_DATA_BASE   data_reg_base

#define WRITE_REG(Addr, Value) ((*(volatile unsigned int *)(Addr)) = (Value))
#define READ_REG(Addr)         (*(volatile unsigned int *)(Addr))


static DEFINE_SEMAPHORE(gpio_sem);  /* 根据linux3.0.y 的源码，DEFINE_SEMAPHORE取代了DECLARE_MUTEX */

//static DECLARE_MUTEX(gpio_sem);

static unsigned int groupbase=-1;
static unsigned int data_reg_base=0;
static unsigned int gpio_1_base_addr_virtual=0;


/*配置管脚复用*/
static void hi3520a_button_pin_cfg(void)
{
		/*映射复用寄存器地址*/
		unsigned int reg_virtual_addr = (unsigned int)ioremap_nocache(0x200f0000, 0x1d8);
    	if(!reg_virtual_addr)
    	{
        	printk("0x200f0000 ioremap addr failed !\n");
        	return;
    	}

    	/*配置作为gpio使用*/
        //WRITE_REG(reg_virtual_addr + 0x03c,0x1);/*GPIO1_4*/
	//WRITE_REG(reg_virtual_addr + 0x178,0x1);/*GPIO9_6*/
	WRITE_REG(reg_virtual_addr + 0x170,0x1);/*GPIO9_4   IR*/
}


static void gpio_calculate_data_groupbase(unsigned int groupnum, unsigned int bitnum)
{
	switch (groupnum) {
	case 1:
		groupbase = gpio_1_base_addr_virtual;
		break;
	default:
		break;
	}

//    printk("groupbase:%x !\n",groupbase);
	data_reg_base = groupbase + (1 << (bitnum + 2));
//    printk("data_reg_base:%x !\n",data_reg_base);
}

static __inline__ void set_gpio_output(void)  //设置为输出
{
	unsigned int reg_tmp;
	gpio_groupbit_info group_bit_info;
	//group_bit_info.groupnumber = 9;   //1;
	//group_bit_info.bitnumber = 6;    //4//;
	group_bit_info.groupnumber = 9;
	group_bit_info.bitnumber = 4;
	group_bit_info.value = 1;
	gpio_calculate_data_groupbase(group_bit_info.groupnumber,group_bit_info.bitnumber);
    reg_tmp=READ_REG(GPIO_DIR_BASE);
   	reg_tmp |=(1<<group_bit_info.bitnumber);
    WRITE_REG(GPIO_DIR_BASE,reg_tmp);
}


static void set_gpio_value(int value)
{
	down_interruptible(&gpio_sem);
	gpio_groupbit_info group_bit_info;
	group_bit_info.groupnumber = 9;   //1;
	group_bit_info.bitnumber = 4;    //4//;
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

static void buzzer_end(unsigned long arg)
{   
	//lc do 一次性输出后，停止继续输出
	printk("buzz stop beep!\n");
	set_gpio_value(0);
}

static int buzzer_open (struct inode *inode, struct file *file)
{
	return 0;
}


static int buzzer_release (struct inode *inode, struct file *file)
{
	return 0;
}

static struct file_operations buzzer_fops = {
	owner:THIS_MODULE,
	open:		buzzer_open,
    //unlocked_ioctl:		leds_ioctl,/* linux3.0.y 不再使用ioctl，采用unlocked_ioctl */
	//ioctl:		leds_ioctl,
	release:    buzzer_release,
};

static struct miscdevice buzzer_dev = {
    MISC_DYNAMIC_MINOR,
    "buzdrv",
    &buzzer_fops,
};

static int __init buzzer_init(void)
{
	printk("start init the buzzer %s(%s)...\n",BUZZER_VERSION,"lc buzzer");
	int  ret=0;
	ret = misc_register(&buzzer_dev);
    if (ret)
    {
        printk(KERN_ERR "register misc dev for buzdrv fail!\n");
  		return ret;
 	}
	gpio_1_base_addr_virtual=(unsigned int) ioremap_nocache(GPIO_1_BASE_ADDR,0x40000);

	if(!gpio_1_base_addr_virtual)
 	{
     		printk("ioremap gpio group1 failed!\n");
     		return -1;
 	}

	hi3520a_button_pin_cfg();
	set_gpio_output();
	//lc do 先至1_4为1，然后在timer到时后置为0
	printk("buzzer beep!\n");
	set_gpio_value(1);
	
	buzzer_timerlist.expires = jiffies + RETURN_TIME; //1second
    buzzer_timerlist.function = buzzer_end; 
    buzzer_timerlist.data = 0;

	add_timer(&buzzer_timerlist);
	
	return 0; 
}

static void __exit buzzer_exit(void)	
{
	printk("buzzer driver %s(%s) removed!\n",BUZZER_VERSION,"lc buzzer");

	del_timer(&buzzer_timerlist);
	
	misc_deregister(&buzzer_dev);
   	iounmap((void*)gpio_1_base_addr_virtual);
}

MODULE_LICENSE("GPL");
module_init(buzzer_init);
module_exit(buzzer_exit);

