/* leds driver, by lc 2013.4 */
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

//#include <asm/hardware.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/ioctl.h>
#include <asm/segment.h>
#include <asm/irq.h>
//#include <asm/arch/irqs.h>
#include <asm/io.h>

#include <linux/fs.h>
#include <linux/device.h>
//#include <linux/devfs_fs_kernel.h>
//#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/init.h>




#include "audioout.h"
#include "audioout_api.h"
//#include "compile_info.h"

/* 原驱动模型
static void release_leds_dev(struct class_device *dev)
{
	return ;
}

static struct class leds_class = {
	.name		= "leds",
	.release	= &release_leds_dev,
};

struct class_device leds_class_dev;
*/

#define AUDIOOUT_DEV       "/dev/audioout" //设备节点名称


//static int leds_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
static int audioout_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	//unsigned long buffer[100];
	unsigned long buffer[2];
	unsigned char *buf;
	buf=(unsigned char *)buffer;
	int state;

	//printk("leds_ioctl cmd is %d\n",cmd);
	
	switch (cmd){
		
		case	SET_0:
				//printk("leds_ioctl errorstate is %ld\n",*(unsigned long *)arg);
				copy_from_user(buf,(unsigned char *)arg,1);
				state=buf[0];
				//printk("audioout_ioctl set_0_value is %d\n",state);
				set_0_value(state);
		break;
		case	SET_1:	
				copy_from_user(buf,(unsigned char *)arg,1);
				state=buf[0];
				//printk("audioout_ioctl set_1_value is %d\n",state);
				set_1_value(state);
				//printk("leds_ioctl alarmstate is %d\n",alarmstate);
		break;
		case	SET_2:
				copy_from_user(buf,(unsigned char *)arg,1);
				state=buf[0];
				//printk("audioout_ioctl set_2_value is %d\n",state);
				set_2_value(state);
				//printk("leds_ioctl netstate is %d\n",netstate);
		break;
		default:
			#ifdef DEBUG
				printk("\naudioout drv recv a unknow cmd :0x%08x\n",(int)cmd);
			#endif
		break;
		}

	return 0;
}

static int audioout_open (struct inode *inode, struct file *file)
{
	//初始化工作
	return 0;
}


static int audioout_release (struct inode *inode, struct file *file)
{
	//clean up
	return 0;
}

static struct file_operations audioout_fops = {
	owner:THIS_MODULE,
	open:		audioout_open,
    unlocked_ioctl:		audioout_ioctl,/* linux3.0.y 不再使用ioctl，采用unlocked_ioctl */
	//ioctl:		leds_ioctl,
	release:    audioout_release,
};

static struct miscdevice audioout_dev = {
    MISC_DYNAMIC_MINOR,
    "audioout",
    &audioout_fops,
};

static int __init audioout_init(void)
{
	printk("start init the audioout %s(%s)...\n",AUDIOOUT_VERSION,"lc audioout");
	int  ret=0;
	unsigned int reg_tmp=0;
/*
	if (register_chrdev(LEDS_MAJOR,LEDS_NAME, &leds_fops)) {
                        printk("audio: can't register devfs leds\n");
                        return -ENODEV;
                }
    class_register(&leds_class);	
    		
	//leds_class_dev.dev = &adap->dev;
	leds_class_dev.class = &leds_class;
	leds_class_dev.devt = MKDEV(LEDS_MAJOR,0);
	sprintf(leds_class_dev.class_id, "audio");
	class_device_register(&leds_class_dev);
	//class_device_create_file(&leds_class_dev, &class_device_attr_name);
	*/
	ret = misc_register(&audioout_dev);
    	if (ret)
    	{
        printk(KERN_ERR "register misc dev for audioout fail!\n");
  		return ret;
 	}
	gpio_1_base_addr_virtual=(unsigned int) ioremap_nocache(GPIO_1_BASE_ADDR,0x10000);
 	if(!gpio_1_base_addr_virtual)
 	{
     		printk("ioremap gpio group1 failed!\n");
     		return -1;
 	}
	gpio_2_base_addr_virtual=(unsigned int) ioremap_nocache(GPIO_2_BASE_ADDR,0x10000);
 	if(!gpio_2_base_addr_virtual)
 	{
     		printk("ioremap gpio group2 failed!\n");
			iounmap((void*)gpio_2_base_addr_virtual);
     		return -1;
 	}
	hi3520d_button_pin_cfg();
	
	gpio_calculate_data_groupbase(2, 4);
	reg_tmp=READ_REG(GPIO_DIR_BASE);
	reg_tmp |=(1<<4);
	WRITE_REG(GPIO_DIR_BASE,reg_tmp);
			
	gpio_calculate_data_groupbase(1, 0);
	reg_tmp=READ_REG(GPIO_DIR_BASE);
	reg_tmp |=(1<<0);
	reg_tmp |=(1<<1);
	WRITE_REG(GPIO_DIR_BASE,reg_tmp);
	
	set_0_value(0);
	set_1_value(0);
	set_2_value(0);

	return 0; 
}

static void __exit audioout_exit(void)	
{
	printk("audioout driver %s(%s) removed!\n",AUDIOOUT_VERSION,"lc audioout");
/*	
	class_unregister(&leds_class);
	devfs_remove("leds");
	unregister_chrdev(LEDS_MAJOR,LEDS_NAME);
	*/
	misc_deregister(&audioout_dev);
   	iounmap((void*)gpio_1_base_addr_virtual);
	iounmap((void*)gpio_2_base_addr_virtual);


}

MODULE_LICENSE("GPL");
module_init(audioout_init);
module_exit(audioout_exit);

