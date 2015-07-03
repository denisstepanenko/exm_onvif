#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <commonlib.h>
#include <gtthread.h>
#include <tw9903api.h>
#include <iic_sim_api.h>
#define VERSION "V0.07"
/*
	V0.07 added change H_DELAY function, changed background color ro blue
	V0.06 changed background color ro black
	V0.05 changed stavation register setting function
	V0.04 reset 1x0c register before setup 9903  
	v0.03 interface was changed into simmulating iic port more stable.
	v0.02 added thread mutex lock control 
	added init_iic_dev() to initialize I2C device return the handler of I2C device
*/
//////// IIC related parameters
//#define I2C_DEV_NAME		"/dev/i2c-0"
#define I2C_DEV_NAME		"/dev/iicsim"	//lsk 2006 -11-30
#if 0
/* this is for i2c-dev.c        */
#define I2C_SLAVE       0x0703  /* Change slave address                 */
                                /* Attn.: Slave address is 7 or 10 bits */
#define I2C_SLAVE_FORCE 0x0706  /* Change slave address                 */
                                /* Attn.: Slave address is 7 or 10 bits */
                                /* This changes the address, even if it */
                                /* is already taken!                    */
#define I2C_TENBIT      0x0704  /* 0 for 7 bit addrs, != 0 for 10 bit   */

#define I2C_FUNCS       0x0705  /* Get the adapter functionality */
#define I2C_RDWR        0x0707  /* Combined R/W transfer (one stop only)*/
#define I2C_PEC         0x0708  /* != 0 for SMBus PEC                   */
#endif

////tw9903  寄存器地址定义
#define TW9903_REG_STATUS		0x01	//状态寄存器video loss
#define TW9903_REG_BRT			0x10	//亮度
#define TW9903_REG_CNT			0x11	//对比度
#define TW9903_REG_SHP1		0x12	//锐化寄存器1
#define TW9903_REG_STA_V		0x13	//饱和度寄存器1
#define TW9903_REG_STA_U		0x14	//饱和度寄存器2
#define TW9903_REG_HUE			0x15	//色度
#define TW9903_REG_SHP2	 	0x16	//锐化寄存器2
#define IIC_LOCK_FILE 			"/lock/iic_dev"  ////文件锁

//互斥体用来防止多线程对I2C总线进行控制
//static pthread_mutex_t	IIC_ctrl_mutex=PTHREAD_MUTEX_INITIALIZER;		
//static int fd_lockfile;
//初始化9903,加载参数表
////初始化D1 和HD1 模式的参数表
////前面的是寄存器地址後面的是参数值
const char techwell9903_720x576[] =
{
        0x02,  0x40,
        0x03,  0x50,
        0x04,  0x00,
        0x05,  0x08,
        0x06,  0x40,

// test  set 0x1c first 
//        0x1c,  0x01,	// lsk test 0x01->0x09
        0x07,  0x22,	// lsk test 0x22->0x22
        0x08,  0x18,	// lsk test 0x15->0x13
        0x09,  0xf0,	// lsk test 0x40->0x50
        0x0a,  0x82,	// lsk test 0x85->0xa5
        0x0b,  0xd0,	// lsk test 0xd0->0xe0
        0x0c,  0x8c,
        0x0d,  0x00,
        0x0e,  0x11,
        0x0f,  0x00,
        0x10,  0x10,
        0x11,  0x70,
  	0x12,  0x0f,
  	0x13,  0x7f,
  	0x14,  0x5a,
  	0x15,  0x30,
        0x16,  0xc3,
  
        0x18,  0x00,
        0x19,  0x58,
        0x1a,  0x80,
        0x1b,  0x80,
        0x1c,  0x01,	// lsk test 0x01->0x09
        0x1d,  0x7f,
        0x1e,  0x00,
        0x1f,  0x00,
        0x20,  0x50,
        0x21,  0x82,
        0x22,  0xf0,
        0x23,  0xfe,
        0x24,  0x3c,
        0x25,  0x38,
        0x26,  0x48,
        0x27,  0x20,
        0x28,  0x00,
        0x29,  0x15,
        0x2a,  0xa0,
        0x2b,  0x44,
        0x2c,  0x07,
        0x2d,  0x00,
        0x2e,  0xa9,
        0x2f,  0xe6,	//lsk 2007 -1-26 change 0xe6->0xe4
        0x30,  0x00,
        0x31,  0x30,
        0x32,  0xa0,
        0x33,  0x22,
        0x34,  0x11,
        0x35,  0x35,
	 0x36,  0x72,

        0x08,  0x18,	// lsk reset this register
        0x0a,  0x81,	// lsk reset this register

};
////初始化CIF模式的参数表
////前面的是寄存器地址後面的是参数值
const char techwell9903_352x288[] = 
{
	0x02,  0x40,
	0x03,  0x50,
	0x04,  0x00,
	0x05,  0x08,
	0x06,  0x40,
	0x07,  0x12,
	0x08,  0x15,
	0x09,  0x20,
	0x0a,  0x92,
	0x0b,  0xb5,
	0x0c,  0x8c,
	0x0d,  0x00,
	0x0e,  0x11,
	0x0f,  0xf8,
	0x10,  0x20,
	0x11,  0x60,
	0x12,  0x01,
	0x13,  0x7f,
	0x14,  0x5a,
	0x15,  0x00,
	0x16,  0xc3,
	
	0x18,  0x00,
	0x19,  0x58,
	0x1a,  0x80,
	0x1b,  0x80,
	0x1c,  0x01,
	0x1d,  0x7f,
	0x1e,  0x00,
	0x1f,  0x00,
	0x20,  0x50,
	0x21,  0x82,
	0x22,  0xf0,
	0x23,  0xfe,
	0x24,  0x3c,
	0x25,  0x38,
	0x26,  0x48,
	0x27,  0x20,
	0x28,  0x00,
	0x29,  0x15,
	0x2a,  0xa0,
	0x2b,  0x44,
	0x2c,  0x07,
	0x2d,  0x00,
	0x2e,  0xa9,
        0x2f,  0xe6,	//lsk 2007 -1-26 change 0xe6->0xe4
	0x30,  0x00,
	0x31,  0x30,
	0x32,  0xa0,
	0x33,  0x22,
	0x34,  0x11,
	0x35,  0x35,
	0x36,  0x72,

        0x08,  0x16,	// lsk reset this register
        0x0a,  0x92,	// lsk reset this register
};
////初始化QCIF模式的参数表
////前面的是寄存器地址後面的是参数值
const char techwell9903_176x144[] = 
{
	0x02,  0x40,
	0x03,  0x50,
	0x04,  0x00,
	0x05,  0x08,
	0x06,  0x40,
	0x07,  0x12,
	0x08,  0x13,
	0x09,  0x20,
	0x0a,  0x92,
	0x0b,  0xb5,
	0x0c,  0x8c,
	0x0d,  0x00,
	0x0e,  0x23,
	0x0f,  0xf0,
	0x10,  0x20,
	0x11,  0x60,
	0x12,  0x01,
	0x13,  0x7f,
	0x14,  0x5a,
	0x15,  0x00,
	0x16,  0xc3,
	
	0x18,  0x00,
	0x19,  0x58,
	0x1a,  0x80,
	0x1b,  0x80,
	0x1c,  0x01,
	0x1d,  0x7f,
	0x1e,  0x00,
	0x1f,  0x00,
	0x20,  0x50,
	0x21,  0x82,
	0x22,  0xf0,
	0x23,  0xfe,
	0x24,  0x3c,
	0x25,  0x38,
	0x26,  0x48,
	0x27,  0x20,
	0x28,  0x00,
	0x29,  0x15,
	0x2a,  0xa0,
	0x2b,  0x44,
	0x2c,  0x07,
	0x2d,  0x00,
	0x2e,  0xa9,
        0x2f,  0xe6,	//lsk 2007 -1-26 change 0xe6->0xe4
	0x30,  0x00,
	0x31,  0x30,
	0x32,  0xa0,
	0x33,  0x22,
	0x34,  0x11,
	0x35,  0x35,
	0x36,  0x72,

        0x08,  0x15,	// lsk reset this register
        0x0a,  0x92,	// lsk reset this register
};
#if 1
static __inline__ int open_IIC_lockfile(void)
{
	int ret;
	int fd_lockfile = -1;
	fd_lockfile = open(IIC_LOCK_FILE,O_RDONLY);
	if(fd_lockfile<0)
	{
		return -1;
	}
	ret = lock_file(fd_lockfile, 1);
	if (ret<0)
	{
		close(fd_lockfile);
		return ret;
	}
	return fd_lockfile;
}

static __inline__ void close_IIC_lockfile(int fd_lockfile)
{
		unlock_file(fd_lockfile);
		close(fd_lockfile);
}
#endif
/*
************************************************************************
*函数名	:init_iic_dev
*功能	:初始化IIC设备
*输入	:  无
*返回值	:成功返回设备控制句柄, 负值表示失败
*修改日志:
*************************************************************************
*/
int init_iic_dev(void)
{
	int I2c_fd;
	int ret;
	int fd_lockfile;
	fd_lockfile= open(IIC_LOCK_FILE,O_RDWR|O_CREAT,0640);
	if(fd_lockfile <0)
	{
		return -1;
	}
	ret = lock_file(fd_lockfile, 1);
	if (ret<0)
	{
		close(fd_lockfile);
		return ret;
	}
	if ((I2c_fd = open(I2C_DEV_NAME, O_RDWR)) < 0) 
	{
		perror("open I2C device error: ");
		close_IIC_lockfile(fd_lockfile);
   		return -1;
    	}
	close_IIC_lockfile(fd_lockfile);
	return I2c_fd;
}
/*
************************************************************************
*函数名	:release_iic_dev
*功能	:关闭/释放IIC设备
*输入	:  无
*返回值	:成功返回0, 负值表示失败
*修改日志:
*************************************************************************
*/
int release_iic_dev(int iic_fd)
{
	if(iic_fd>=0)
	{
		close(iic_fd);
		return 0;
	}
	return -1;
}
#if 0
/*
************************************************************************
*函数名	:set_iic_chip_addr
*功能	:设置I2C 设备地址
*输入	:  
	  int iic_fd;		//打开设备返回的控制句柄	
	  int chip_addr;	//设备地址
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
static __inline__ int set_iic_chip_addr(int iic_fd,int chip_addr)
{
	if(ioctl(iic_fd,I2C_SLAVE, chip_addr) < 0)
	{
		perror("set I2C device address error: ");
    		return -1;
  	}
//	printf("set addr ok \n");// test
	return 0;
}
/*
************************************************************************
*函数名	:iic_bus_init
*功能	:I2C 总线初始化
*输入	:  
	  int iic_fd;		//打开设备返回的控制句柄	
	  int chip_addr;	//设备地址
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
static __inline__ int iic_bus_init(int iic_fd,int chip_addr)
{
	if(iic_fd<0)
	{
		printf("I2C handler error iic_fd = %d\n", iic_fd);
		return -1;
	}
	if((chip_addr<0)||(chip_addr>0x7f))
	{
		printf("chip_addr out of range (0-0x7f) \n");
		return -1;
	}
	if(set_iic_chip_addr(iic_fd, chip_addr)<0)
	{
		return -1;
	}
	return 0;
}
#endif 
/*
************************************************************************
*函数名	:write_iic_reg
*功能	:写tw9903设备的寄存器
*输入	:  
	  int iic_fd;			//打开设备返回的控制句柄	
	  unsigned char reg;	//寄存器地址
* 	  int val;				//寄存器参数值
*返回值	:无 
*修改日志:
*************************************************************************
*/
static __inline__ int write_iic_reg(int iic_fd, unsigned char chip_addr, unsigned char reg , unsigned char value)
{
	struct iic_dev_struct info;
	
	memset(&info, 0 ,sizeof(info));
	info.dev_addr = chip_addr;
	info.reg_addr = reg;
	info.buf[0] = value;
	if(ioctl(iic_fd, I2C_WRITE_BYTE, &info))
	{
		perror("I2C write error:");
		return -1;
	}		
	return 0;
}
/*
************************************************************************
*函数名	:read_iic_reg
*功能	:读取tw9903 寄存器值
*输入	:  
	  int iic_fd;		//打开设备返回的控制句柄	
	  unsigned char reg;			//寄存器的地址
	  unsigned char *val;		//存放寄存器值的指针
*返回值	:成功返回 0 寄存器的值存放在val中，失败返回-1
*修改日志:
*************************************************************************
*/
static __inline__ int read_iic_reg(int iic_fd, int chip_addr, unsigned char reg, unsigned char *val)
{
	struct iic_dev_struct info;
	if(val==NULL)
	{
		return -1;
	}
	memset(&info, 0 ,sizeof(info));
	info.dev_addr = (unsigned char)chip_addr;
	info.reg_addr = reg;

	if(ioctl(iic_fd, I2C_READ_BYTE, &info))
	{
		perror("I2C read error:");
		return -1;
	}
	*val =  info.buf[0];
	return 0;
}

/*
************************************************************************
*函数名	:write_tw9903_reg
*功能	:写tw9903设备的寄存器
*输入	:  
	  int iic_fd;			//打开设备返回的控制句柄	
	  int chip_addr;		//设备地址
	  unsigned char reg;	//寄存器地址
* 	  int val;				//寄存器参数值
*返回值	:0表示成功, 负值表示失败
*修改日志:
*************************************************************************
*/
int write_tw9903_reg(int iic_fd, int chip_addr, unsigned char reg , int value)
{
	int ret;
	int fd_lockfile;
	fd_lockfile = open_IIC_lockfile();
	if(fd_lockfile<0)
	{
		return -1;
	}
	ret = write_iic_reg(iic_fd, chip_addr, reg, value);
	close_IIC_lockfile(fd_lockfile);
	return ret;
}
/*
************************************************************************
*函数名	:read_tw9903_reg
*功能	:读取tw9903 寄存器值
*输入	:  
	  int iic_fd;		//打开设备返回的控制句柄	
	  int chip_addr;	//设备地址
	  int reg;			//寄存器的地址
*返回值	:成功返回寄存器的值，失败返回0xff
*修改日志:
*************************************************************************
*/
unsigned char read_tw9903_reg(int iic_fd, int chip_addr,unsigned char reg)
{
	unsigned char val;
	int ret;
	int fd_lockfile;
	fd_lockfile = open_IIC_lockfile();
	if(fd_lockfile<0)
	{
		return -1;
	}
	ret = read_iic_reg(iic_fd, chip_addr, reg, &val);
	close_IIC_lockfile(fd_lockfile);
	if(ret)
	{
		return ret;
	}
	return val;
}
/*
************************************************************************
*函数名	:write_iic_table_pair
*功能	:写寄存器参数表
*输入	:  
	  int iic_fd;			//打开设备返回的控制句柄	
	  unsigned char *buf;	//参数表指针
* 	  int len;				//参数表的大小
*返回值	:0表示成功负值表示失败
*修改日志:
*注意:
参数表的格式必须为:
	寄存器地址，寄存器值,
	寄存器地址，寄存器值,
	寄存器地址，寄存器值,
	寄存器地址，寄存器值,
	寄存器地址，寄存器值,
					......
					
*************************************************************************
*/
static __inline__ void write_iic_table_pair(int iic_fd , int chip_addr, unsigned char *buf, int len)
{
	int i, ret;
//	unsigned char test;
	
	unsigned char *p=NULL;
	p = buf;
	for(i=0;i<len/2;i++)
	{
		ret = write_iic_reg(iic_fd, chip_addr, *p, *(p+1));
//		printf("write reg %02x  val = %02x \n", *p,*(p+1));

		if(ret)
		{
			printf("write reg %02x error \n", *p);
		}
#if 0
		// test 

		ret = read_iic_reg(iic_fd, chip_addr,*p, &test);
		if(ret)
		{
			printf("read reg %02x error \n", *p);
		}
		if(test!=(*(p+1)))
		{
			printf("read reg %02x error write val = %02x read val =%02x \n",*p, *(p+1), test);
		}
		ret = read_iic_reg(iic_fd,chip_addr, 0x07,  &test);
		if(ret)
		{
			printf("read reg %02x error \n", 0x07);
		}
		printf("reg 0x07 = %02x\n", test);
		///// end of test
#endif
		p+=2;

	}
}

/*
************************************************************************
*函数名	:init_tw9903
*功能	:tw9903 初始化
*输入	:  
	  int iic_fd;		//打开设备返回的控制句柄	
	  int chip_addr;	//设备地址
* 	  int mode;		//工作模式
*返回值	:0表示成功，负值表示失败
*修改日志:
*************************************************************************
*/
int init_tw9903(int iic_fd,int chip_addr,int mode)
{
	int fd_lockfile;
	//// set chip address 
	fd_lockfile = open_IIC_lockfile();
	if(fd_lockfile<0)
	{
		return -1;
	}
	//// setup tw9903
	// test lsk 2006 -12-30
#if 1	
	write_iic_reg(iic_fd, chip_addr, 0x1c, 0x08);// reset 0x1c register to avoid error 
#endif 	
//lsk 2007 -1-17

////enb of change
	switch (mode)
	{
		case SCR_MODE_D1:
		write_iic_table_pair(iic_fd, chip_addr, (unsigned char*)techwell9903_720x576, sizeof(techwell9903_720x576));
		break;
		case SCR_MODE_HD1:
		write_iic_table_pair(iic_fd, chip_addr, (unsigned char*)techwell9903_720x576, sizeof(techwell9903_720x576));
		break;
		case SCR_MODE_CIF:
		write_iic_table_pair(iic_fd, chip_addr, (unsigned char*)techwell9903_352x288, sizeof(techwell9903_352x288));
		break;
		case SCR_MODE_QCIF:
		write_iic_table_pair(iic_fd, chip_addr, (unsigned char*)techwell9903_176x144, sizeof(techwell9903_176x144));
		break;
	}
	close_IIC_lockfile(fd_lockfile);
	return 0;
}
/*
************************************************************************
*函数名	:read_tw9903_vloss
*功能	:读取9903视频丢失状态
*输入	:  
	  int iic_fd;		//打开设备返回的控制句柄	
	  int chip_addr;	//设备地址
* 	  
*返回值	:0表示没有视频丢失，1 表示有视频丢失，负值表示出错
*修改日志:
*************************************************************************
*/
int read_tw9903_vloss(int iic_fd,int chip_addr)
{
	unsigned char val;
	int temp;
	int fd_lockfile;
	fd_lockfile = open_IIC_lockfile();
	if(fd_lockfile<0)
	{
		return -1;
	}
	temp = read_iic_reg(iic_fd, chip_addr, TW9903_REG_STATUS, &val);
	if(temp)
	{
		close_IIC_lockfile(fd_lockfile);
		return -1;
	}
	val &=0x80;
	close_IIC_lockfile(fd_lockfile);
	return val>>7;
}
/*
************************************************************************
*函数名	:set_tw9903_brightness
*功能	:设置tw9903 亮度
*输入	:  
	  int iic_fd;		//打开设备返回的控制句柄	
	  int chip_addr;	//设备地址
* 	  int val;			//亮度参数值
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
int set_tw9903_brightness(int iic_fd,int chip_addr,int val)
{
	int value;
	int ret;
	int fd_lockfile;
	fd_lockfile = open_IIC_lockfile();
	if(fd_lockfile<0)
	{
		return -1;
	}
	value = get_value(-128,127,val);
	ret = write_iic_reg(iic_fd, chip_addr, TW9903_REG_BRT,value);
	close_IIC_lockfile(fd_lockfile);
	return ret;
}
/*
************************************************************************
*函数名	:set_tw9903_hue
*功能	:设置tw9903 色度
*输入	:  
	  int iic_fd;		//打开设备返回的控制句柄	
	  int chip_addr;	//设备地址
* 	  int val;			//色度参数值
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
int set_tw9903_hue(int iic_fd,int chip_addr,int val)
{
	int value;
	int ret;
	int fd_lockfile;
	fd_lockfile = open_IIC_lockfile();
	if(fd_lockfile<0)
	{
		return -1;
	}
	value = get_value(-128,127,val);
	ret = write_iic_reg(iic_fd, chip_addr, TW9903_REG_HUE,value);
	close_IIC_lockfile(fd_lockfile);
	return ret;
}
/*
************************************************************************
*函数名	:set_tw9903_contrast
*功能	:设置tw9903 对比度
*输入	:  
	  int iic_fd;		//打开设备返回的控制句柄	
	  int chip_addr;	//设备地址
* 	  int val;			//对比度参数值
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
int set_tw9903_contrast(int iic_fd,int chip_addr,int val)
{
	int value;
	int ret;
	int fd_lockfile;
	fd_lockfile = open_IIC_lockfile();
	if(fd_lockfile<0)
	{
		return -1;
	}
	value = get_value(0,255,val);
	ret = write_iic_reg(iic_fd, chip_addr, TW9903_REG_CNT,value);
	close_IIC_lockfile(fd_lockfile);
	return ret;
}
/*
************************************************************************
*函数名	:set_tw9903_saturation
*功能	:设置tw9903 饱和度
*输入	:  
	  int iic_fd;		//打开设备返回的控制句柄	
	  int chip_addr;	//设备地址
* 	  int val;			//饱和度参数值
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
int set_tw9903_saturation(int iic_fd,int chip_addr,int val)
{
	int value;
	int ret;
	int fd_lockfile;
	fd_lockfile = open_IIC_lockfile();
	if(fd_lockfile<0)
	{
		return -1;
	}
	value = get_value(0,200,val);
	ret = write_iic_reg(iic_fd, chip_addr, TW9903_REG_STA_V,value);
	if(ret<0)
	{
		close_IIC_lockfile(fd_lockfile);
		return ret;
	}
//	ret = write_iic_reg(iic_fd, chip_addr, TW9903_REG_STA_U,value+37); //lsk 2007 -1-18
	ret = write_iic_reg(iic_fd, chip_addr, TW9903_REG_STA_U,value-36); //lsk 2007 -1-18
	close_IIC_lockfile(fd_lockfile);
	return ret;
}
/*
************************************************************************
*函数名	:set_tw9903_sharp2
*功能	:设置锐化寄存器1
*输入	:  
	  int iic_fd;		//打开设备返回的控制句柄	
	  int chip_addr;	//设备地址
* 	  int val;			//锐化寄存器1的参数值
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
int set_tw9903_sharp1(int iic_fd,int chip_addr,int val)
{
	int ret;
	int fd_lockfile;
	fd_lockfile = open_IIC_lockfile();
	if(fd_lockfile<0)
	{
		return -1;
	}
	ret = write_iic_reg(iic_fd, chip_addr, TW9903_REG_SHP1,val);
	close_IIC_lockfile(fd_lockfile);
	return ret;
}
/*
************************************************************************
*函数名	:set_tw9903_sharp2
*功能	:设置锐化寄存器2
*输入	:  
	  int iic_fd;		//打开设备返回的控制句柄	
	  int chip_addr;	//设备地址
* 	  int val;			//锐化寄存器2的参数值
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
int set_tw9903_sharp2(int iic_fd,int chip_addr,int val)
{
	int ret;
	int fd_lockfile;
	fd_lockfile = open_IIC_lockfile();
	if(fd_lockfile<0)
	{
		return -1;
	}
	ret = write_iic_reg(iic_fd, chip_addr,TW9903_REG_SHP2,val);
	close_IIC_lockfile(fd_lockfile);
	return ret;
}
/*
************************************************************************
*函数名	:set_tw9903_H_range
*功能	:设置H_DELAY寄存器
*输入	:  
	  int iic_fd;		//打开设备返回的控制句柄	
	  int chip_addr;	//设备地址
* 	  int val;			//H_DELAY寄存器的参数值
					//参数范围: 0x80 - 0x83  default: 0x81
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
int set_tw9903_H_range(int iic_fd,int chip_addr,int val)
{
	int ret;
	int fd_lockfile;
	fd_lockfile = open_IIC_lockfile();
	if(fd_lockfile<0)
	{
		return -1;
	}
	ret = write_iic_reg(iic_fd, chip_addr,0x0a,val); // 设置0x0a寄存器
	close_IIC_lockfile(fd_lockfile);
	return ret;
}

