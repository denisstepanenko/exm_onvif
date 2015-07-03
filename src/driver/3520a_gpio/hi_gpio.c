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
unsigned int gpio_0_base_addr_virtual=0;
unsigned int gpio_2_base_addr_virtual=0;
unsigned int gpio_6_base_addr_virtual=0;
unsigned int gpio_8_base_addr_virtual=0;


/*配置管脚复用*/
static void hi3520d_button_pin_cfg(void)
{
		/*映射复用寄存器地址*/
		unsigned int reg_virtual_addr = (unsigned int)ioremap_nocache(0x200f0000, 0x0c8);
    	if(!reg_virtual_addr)
    	{
        	printk("0x200f0000 ioremap addr failed !\n");
        	return -1;
    	}

    	/*配置作为gpio使用*/

	//雄迈主板gpio输入
	WRITE_REG(reg_virtual_addr + 0x098,0x0);/*GPIO0_0  */
	WRITE_REG(reg_virtual_addr + 0x09c,0x0);/*GPIO0_1  */
	WRITE_REG(reg_virtual_addr + 0x0a0,0x0);/*GPIO0_2  */
	WRITE_REG(reg_virtual_addr + 0x0a4,0x0);/*GPIO0_3  */

	WRITE_REG(reg_virtual_addr + 0x030,0x0);/*GPIO8_0 */
	WRITE_REG(reg_virtual_addr + 0x038,0x0);/*GPIO8_2 */
	WRITE_REG(reg_virtual_addr + 0x03c,0x0);/*GPIO8_3 */
	WRITE_REG(reg_virtual_addr + 0x040,0x0);/*GPIO8_4 */


	//雄迈主板gpio输出
	//0->GPIO2_3  hw cfg
	//1->GPIO6_2
	WRITE_REG(reg_virtual_addr + 0x070,0x0);/*GPIO6_2*/
	//2->GPIO2_5  hw cfg
	//3->GPIO2_6  hw cfg

	//雄迈UART1/2设置复用为UART
	//WRITE_REG(reg_virtual_addr + 0x04c,0x1);   /*GPIO5_0 UART1*/
	WRITE_REG(reg_virtual_addr + 0x050,0x1);   /*GPIO5_1 UART1*/
	//WRITE_REG(reg_virtual_addr + 0x054,0x1);   /*GPIO5_2 UART1*/
	WRITE_REG(reg_virtual_addr + 0x058,0x1);   /*GPIO5_3 UART1*/
	WRITE_REG(reg_virtual_addr + 0x05c,0x1);   /*GPIO5_4 UART2*/
	WRITE_REG(reg_virtual_addr + 0x060,0x1);   /*GPIO5_5 UART2*/
	//WRITE_REG(reg_virtual_addr + 0x008,0x10);   /*GPIO12_3 */
	//WRITE_REG(reg_virtual_addr + 0x054,0x10);   /*GPIO12_5 */
	//WRITE_REG(reg_virtual_addr + 0x1C0,0x1);   /*GPIO11_1 */
	//WRITE_REG(reg_virtual_addr + 0x1BC,0x1);   /*GPIO11_0 */
	
	/*配置VIU0_VS为VIU0_CLKA*/ 
 	//WRITE_REG(reg_virtual_addr + 0x004,0x2);
	//printk("read:%#X\n",READ_REG(reg_virtual_addr + 0x000) );

	
}

static void gpio_calculate_data_groupbase(unsigned int groupnum, unsigned int bitnum)
{
	switch (groupnum) {
	case 0:
		groupbase = gpio_0_base_addr_virtual;
		break;
	case 2:
		groupbase = gpio_2_base_addr_virtual;
		break;
	case 6:
		groupbase = gpio_6_base_addr_virtual;
		break;
	case 8:
		groupbase = gpio_8_base_addr_virtual;
		break;
	default:
		break;
	}

//    printk("groupbase:%x !\n",groupbase);
	data_reg_base = groupbase + (1 << (bitnum + 2));
//    printk("data_reg_base:%x !\n",data_reg_base);
}

 

static int gpio_open(struct inode *inode, struct file *filp)
{
   return 0;  
}

static int gpio_release(struct inode *inode, struct file *filp)
{
 return 0; 
}


//static int gpio_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
static int gpio_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{         
    	gpio_groupbit_info group_bit_info;
    	unsigned int reg_tmp=0;
	relay_struct  relay;
	int tmp_set_delay;
    	down_interruptible(&gpio_sem);
	static unsigned long in_state=0xffff;//报警输入常开常闭属性，位1常闭，位0常开;默认常开
	static unsigned long in; //输入状态
	static TimeDelay delay={3,3};//转为3,3	//延时
	static unsigned int on_delay[8]={3,3,3,3,3,3,3,3};  //无效到有效时间
	static unsigned int off_delay[8]={0};//有效到无效时间
	
    	switch(cmd)
    	{
	
	case GPIO_SET_INPUT:/*1表示常闭0表示常开*/
		copy_from_user(&in_state,(unsigned long*)arg, sizeof(unsigned long));
		in=0;
		//printk("in_state:%#X\n",in_state);
		break;
	case GPIO_GET_INPUT:/*1表示常闭0表示常开*/
		 copy_to_user((unsigned long *)arg, &in_state, sizeof(unsigned long));
		break;
	case GPIO_GET_DWORD:
#if 1
		/*设置gpio为输入*/
		//gpio_calculate_data_groupbase(4, 4);//些函数后面一个参数好像没有意义
		//WRITE_REG(GPIO_DIR_BASE,0x0);//写0为输入
		//读输入状态
		reg_tmp=0;
		unsigned in_tmp;
		//reg_tmp &=(1<<group_bit_info.bitnumber);
		{
			unsigned int ch,ch_tmp;
			for(ch=0; ch<8; ch++)
			{
				//ch小于4是第0组的
				if(ch<4)
				{
				gpio_calculate_data_groupbase(0, ch);//些函数后面一个参数好像没有意义
				//WRITE_REG(GPIO_DIR_BASE,0x0);//写0为输入
				ch_tmp=ch;
				}
				else//第8组的
				{
					switch (ch)
					{
					case 4:
						ch_tmp = 0;
						break;
					case 5:
						ch_tmp = 2;
						break;
					case 6:
						ch_tmp = 3;
						break;
					case 7:
						ch_tmp = 4;
						break;
					default:
						break;
					}
					gpio_calculate_data_groupbase(8,ch_tmp);
					// yk WRITE_REG(GPIO_DIR_BASE,0x0);//写0为输入
				}

				reg_tmp=READ_REG(GPIO_DATA_BASE);
				//printk("reg_tmp:%#X,in_state:%#X\n",reg_tmp,in_state);
				unsigned int on=reg_tmp&(1<<ch_tmp);
				//printk("on:%#X,reg_tmp:%#X\n",on,reg_tmp);


				if(in_state&(1<<ch))/*in_state=1*///根据每一位的常开常闭属性设置有效无效时间
				{

					if(on!=0)/*高电平*/
					{
						if(on_delay[ch]>0)
						{
							on_delay[ch]=0;//0->1
						}
						off_delay[ch]++;
					}
					else//低电平
					{
					
						if(off_delay[ch]>0)
						{
							off_delay[ch]=0;//1->0
						}
						on_delay[ch]++;
					}
					if(off_delay[ch]>delay.off_delay)//3次无报警后才算无报警
					{
						on_delay[ch]=0;
						in &= (~(1<<ch));	
					}
					if(on_delay[ch]>delay.on_delay)//1次有报警就有报警
					{
						off_delay[ch]=0;
						in |= (1<<ch);
					}
					
				}
				else/*in_state=0*/
				{
					if(on!=0)/*高电平*/
					{
						if(off_delay[ch]>0)
						{
							off_delay[ch]=0;//1->0
						}
						on_delay[ch]++;						
					}
					else//低电平
					{
						if(on_delay[ch]>0)
						{
							on_delay[ch]=0;//0->1
						}
						off_delay[ch]++;
					}	

					if(off_delay[ch]>delay.off_delay)	//3次无报警才算无报警
					{
						off_delay[ch]=0;
						in |= (1<<ch);

					}
					if(on_delay[ch]>delay.on_delay)//1次有报警就算有报警
					{
						on_delay[ch]=0;
						in &= (~(1<<ch));
					}					
				}
			}
		}
		//printk("in:%#X,in_state:%#X\n",in,in_state);
		in_tmp = (in)^(in_state);
		in_tmp &=0xff;
		//printk("end in:%#X\n",in_tmp);
		copy_to_user((unsigned long *)arg, &in_tmp, sizeof(unsigned long));
#endif
		break;
	case GPIO_SET_DELAY:
		copy_from_user(&delay,(TimeDelay*)arg, sizeof(TimeDelay));
		for(tmp_set_delay=0; tmp_set_delay < 8; tmp_set_delay++)
		{
			on_delay[tmp_set_delay] = delay.on_delay;
			off_delay[tmp_set_delay] = delay.off_delay;
		}
		break;
	case GPIO_GET_DELAY:
		copy_to_user((TimeDelay*)arg,&delay, sizeof(TimeDelay));
		break;
	case GPIO_SET_OUTPUT_VALUE:
		copy_from_user((void *)&relay,arg, sizeof(relay_struct));
		//printk("ch=%d,num=%d\n",relay.ch,relay.result);
		unsigned int groupbit = 0;
		switch(relay.ch)
		{
			case 0:
				groupbit = 3;
				break;
			case 1:
				groupbit = 2;
				break;
			case 2:
				groupbit = 5;
				break;
			case 3:
				groupbit = 6;
				break;
			default:
				up(&gpio_sem);
				return -1;
		}
		//写值		
		if(relay.ch==1)
			gpio_calculate_data_groupbase(6,groupbit);
		else
			gpio_calculate_data_groupbase(2,groupbit);
		if(relay.result==0)
		{
			//WRITE_REG(GPIO_DATA_BASE,0x0);
			reg_tmp=READ_REG(GPIO_DATA_BASE);
    		reg_tmp &=(0<<groupbit);
    		WRITE_REG(GPIO_DATA_BASE,reg_tmp);
			printk("1addr:0x%x vaule:%x!\n",GPIO_DATA_BASE,*((unsigned int *)GPIO_DATA_BASE));		
		}
		else if(relay.result==1)
		{
			//WRITE_REG(GPIO_DATA_BASE,1<<groupbit);
			reg_tmp=READ_REG(GPIO_DATA_BASE);
    		reg_tmp |=(1<<groupbit);
    		WRITE_REG(GPIO_DATA_BASE,reg_tmp);
			printk("2addr:0x%x vaule:%x!\n",GPIO_DATA_BASE,*((unsigned int *)GPIO_DATA_BASE));		
		}
		else
		{
			printk("write bit beyond of extent!\n");
			up(&gpio_sem);
			return -1;
		}

		break;
	case GPIO_SET_HEARTBEAT:
		//gpio_calculate_data_groupbase(9, 0);//些函数后面一个参数好像没有意义
		//WRITE_REG(GPIO_DIR_BASE,0x1);//写1为输出
		copy_from_user(&reg_tmp,arg, sizeof(int));

		//gpio_calculate_data_groupbase(9,4);
		//WRITE_REG(GPIO_DATA_BASE,reg_tmp);
		break;
	case GPIO_GET_POWER:
		//printk("gpio get power!\n");
		gpio_calculate_data_groupbase(2,7);
		reg_tmp=READ_REG(GPIO_DATA_BASE);
		//printk("reg_tmp:%#X,in_state:%#X\n",reg_tmp,in_state);
		unsigned int on=(reg_tmp>>7);
		on&=0xff;
		//printk("on:%#X,reg_tmp:%#X\n",on,reg_tmp);
		copy_to_user((unsigned long *)arg, &on, sizeof(unsigned long));
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
 // ioctl:gpio_ioctl,
  unlocked_ioctl:gpio_ioctl,  /* linux3.0.y 不再使用ioctl，采用unlocked_ioctl */
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

	//printk("asdadasdasdad\n");
	//hi3520a_button_pin_cfg();

 	signed int  ret=0;
        ret = misc_register(&gpio_dev);
        if (ret)
        {
                printk(KERN_ERR "register misc dev for hi_gpio fail!\n");
  		return ret;
 	}
 
 	gpio_0_base_addr_virtual=(unsigned int)ioremap_nocache(GPIO_0_BASE_ADDR,0x10000);
 	if(!gpio_0_base_addr_virtual)
 	{
     		printk("ioremap gpio group0 failed!\n");
     	return -1;
 	}
 	//gpio_2_base_addr_virtual=gpio_0_base_addr_virtual+0x20000;
	gpio_2_base_addr_virtual=(unsigned int)ioremap_nocache(GPIO_2_BASE_ADDR,0x10000);
	if(!gpio_2_base_addr_virtual)
 	{
     		printk("ioremap gpio group2 failed!\n");
     		/*说明gpio_0 和gpio_4都映射成功了，但gpio_8没映射成功，所以将之前映射的释放*/
     		iounmap((void*)gpio_0_base_addr_virtual);
     		return -1;
 	}
	gpio_6_base_addr_virtual=(unsigned int)ioremap_nocache(GPIO_6_BASE_ADDR,0x10000);
	if(!gpio_6_base_addr_virtual)
 	{
     		printk("ioremap gpio group6 failed!\n");
     		/*说明gpio_0 和gpio_4都映射成功了，但gpio_8没映射成功，所以将之前映射的释放*/
     		iounmap((void*)gpio_0_base_addr_virtual);
     		iounmap((void*)gpio_2_base_addr_virtual);
     		return -1;
 	}
	gpio_8_base_addr_virtual=(unsigned int) ioremap_nocache(GPIO_8_BASE_ADDR,0x10000);
 	if(!gpio_8_base_addr_virtual)
 	{
     		printk("ioremap gpio group8 failed!\n");
     		/*说明gpio_0 和gpio_4都映射成功了，但gpio_8没映射成功，所以将之前映射的释放*/
     		iounmap((void*)gpio_0_base_addr_virtual);
     		iounmap((void*)gpio_2_base_addr_virtual);
     		iounmap((void*)gpio_6_base_addr_virtual);
     		return -1;
 	}

 	printk("gpio config buttonpin!\n");
 	hi3520d_button_pin_cfg();

	unsigned int i;
	unsigned int reg_tmp=0;

		gpio_calculate_data_groupbase(0, 0);
		reg_tmp=READ_REG(GPIO_DIR_BASE);
		reg_tmp &=(0);
		reg_tmp &=(0<<1);
		reg_tmp &=(0<<2);
		reg_tmp &=(0<<3);
    	WRITE_REG(GPIO_DIR_BASE,reg_tmp);
		
		gpio_calculate_data_groupbase(8, 0);
		reg_tmp=READ_REG(GPIO_DIR_BASE);
		reg_tmp &=(0);
		reg_tmp &=(0<<2);
		reg_tmp &=(0<<3);
		reg_tmp &=(0<<4);
    	WRITE_REG(GPIO_DIR_BASE,reg_tmp);
		
		//将gpio2_3/5/6 设为输出
		gpio_calculate_data_groupbase(2, 0);
		reg_tmp=READ_REG(GPIO_DIR_BASE);
		//gpio2_7 作为输入
		reg_tmp &=(0<<7);
		
		reg_tmp |=(1<<3);
		reg_tmp |=(1<<5);
		reg_tmp |=(1<<6);
    	WRITE_REG(GPIO_DIR_BASE,reg_tmp);
		gpio_calculate_data_groupbase(2, 5);
		reg_tmp=READ_REG(GPIO_DATA_BASE);
		reg_tmp &=(0<<5);
		WRITE_REG(GPIO_DATA_BASE,reg_tmp);
		gpio_calculate_data_groupbase(2, 6);
		reg_tmp=READ_REG(GPIO_DATA_BASE);
		reg_tmp &=(0<<6);
		WRITE_REG(GPIO_DATA_BASE,reg_tmp);

		gpio_calculate_data_groupbase(6,0);
		reg_tmp=READ_REG(GPIO_DIR_BASE);
		reg_tmp |=(1<<2);
		WRITE_REG(GPIO_DIR_BASE,reg_tmp);

		return 0;         
}


static void __exit hi_gpio_exit(void)
{
    	misc_deregister(&gpio_dev);
    	iounmap((void*)gpio_0_base_addr_virtual);
    	iounmap((void*)gpio_2_base_addr_virtual);
    	iounmap((void*)gpio_6_base_addr_virtual);
    	iounmap((void*)gpio_8_base_addr_virtual);
}

module_init(hi_gpio_init);
module_exit(hi_gpio_exit);

MODULE_AUTHOR("lc,gtalarm ");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gpio for 3520d"); 



