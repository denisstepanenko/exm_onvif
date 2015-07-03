
/**
	@file:	Icaccess.c
	@brief:	test 9910
			1. test every register content to judge which register make error
			2. to confirm if this register the one makes a error
	@modify:	08.09.08 the read module have finished
			08.09.09 the write module all have finished
			08.11 modified i2c mode control,and all registers refresh
			08.12 modify all
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tw9910api.h>
#include <unistd.h>
#include <iiclib.h>
#include <gtlog.h>

#define	MAX_REG_NUM	112 ///<寄存器最大数目


static void help(void)
{
	printf("开启关闭i2c:iaccess -o  1(open) 0 (close)\n");
	printf("读取命令:iaccess -r   -a 芯片地址-d 寄存器地址-n 要读取的数目\n");
	printf("写入命令:iaccess -w  -a 芯片地址-d 寄存器地址-v 寄存器写入值\n");
	printf("寄存器组写入命令:iaccess    -t     读取的文件名		寄存器个数\n");
	//printf("**************读取文档路径应放/log/下名字为reg_val.txt**********\n");
	return;
}

static void init_iic(void)
{
	int	i2c_init;
	
	i2c_init = open_iic_dev();
	if(i2c_init < 0)
	{
         	printf("打开iic设备%s 失败!\n", i2c_init);
              exit(1);
	}

	return;
}

static void init_log(void)
{
	gtopenlog(NULL);
	return;
}

/*
	i=1 iic open;
	i-0  iic close
*/
static void iic_status(int i)
{
	int	en;

	init_iic();
	
	if(i)
	{
		
		en = enable_iic_bus();
		if(en == 0)
		{
			printf("i2c open successful!\n");
			gtloginfo("I2C open successful\n");
			return 0;
		}
		else
		{
			printf("i2c open failed!\n");
			return -1;
		}
	}
	else
	{
		en = disable_iic_bus();
		if(en == 0)
		{
			printf("i2c close successful!\n");
			gtloginfo("I2C close successful\n");
			return 0;
		}
		else
		{
			printf("i2c close failed!\n");
			return -1;
		}
	}
	return;
}

/*
	param addr: chip address
	param reg:   chip register offset
	param reg_val: register value read
	param pf_num: read register numbers
*/

static void iic_read(int addr, unsigned char reg, unsigned char reg_val, int pf_num)
{
	int	ret;
	int	i;
	
	init_iic();
	
	for( i=0; i<pf_num; i++ )	
	{
		ret=read_iic_reg(addr,reg,&reg_val);
		if(ret<0)
		{
			printf("读取芯片%02x,addr:%02x失败!\n",addr,reg_val);
			exit(1);
		}
		else
		{
			printf("chip:%02x %02x=%02x\n",addr,reg,reg_val);
			reg++;
		}
	}

	return;
}

/*
	param addr: chip address
	param reg:   chip register offset
	param reg_val: register value to write
*/

static void iic_write(int addr, unsigned char reg, unsigned char reg_val)
{
	int	ret;
	int	i;

	init_iic();

	ret=write_iic_reg(addr,reg,reg_val);
	if(ret<0)
	{
		printf("设置芯片%02x,addr:%02x=%02x失败!\n",addr,reg,reg_val);
		exit(1);
	}
	printf("设置 chip:%02x %02x=%02x\n",addr,reg,reg_val);

	return;
}

/*
	brief: 从文件读取寄存器值并一次性写入所有寄存器
	param: all_reg要读取的 文件名，
		    line要写入的寄存器个数

*/

static void iic_write_all(char *all_reg, int *line)
{

	unsigned char	reg_buf[112][15];  ///<register value from all_reg's txt
	unsigned char	reg_temp[2]; 	 ///register value save template 
	unsigned char * p_buf = NULL;    ///< point for reg_buf
	unsigned char	* reg_val_pf;  	///< 所有要打印的寄存的值
	
	unsigned char	reg,reg_end;
	unsigned char	reg_val;
	int	addr;
	
	FILE	*fp = NULL;
	int	ret;
	int 	regs_num;
	int	j;

	init_iic();
	
	fp = fopen( all_reg, "r" );
	if(fp == NULL)
	{
		printf("Cannot open %s\n",all_reg);
		//printf("/log/下可能无reg_val.txt\n");
		return 0;
	}

	///读取文件内容到缓存
	fread( reg_buf, 15, 112, fp );
	fclose(fp);
	//printf("reg_buf is %s\n",*reg_buf);
				
	///将文档中的数值赋到变量中
	reg_val_pf = *reg_buf;

	///读取芯片地址和偏移量
	reg_val_pf += 5;
	addr = strtol(reg_val_pf, NULL, 16);
	reg_val_pf += 2;
	reg = strtol(reg_val_pf, NULL, 16);
				
	reg_val_pf += 4;
	///循环写入寄存器
	regs_num = *line;
	for( j=0; j<regs_num; j++ )
	{	
		reg_temp[0] = *reg_val_pf;
		reg_temp[1] = *(reg_val_pf+1);
		reg_val = strtol( reg_temp, NULL, 16);;
		//printf("\tnew reg_val is %x\n",reg_val);
		reg_end = reg;
					
		ret=write_iic_reg(addr,reg,reg_val);
		if(ret<0)
		{
			printf("设置芯片%02x,addr:%02x=%02x失败!\n",addr,reg,reg_val);
			exit(1);
		}
		printf("设置chip:%02x %02x=%02x\n",addr,reg,reg_val);
		///读取每次要写入的偏移量
		reg_val_pf += 10;
		reg = strtol(reg_val_pf, NULL, 16);
		reg_val_pf += 4;
	}

	return;
}


int  main(int argc,char *argv[]) 
{

	int 	addr;				///<芯片地址
	unsigned char reg;			///<寄存器地址
	unsigned char	reg_val;		///寄存器读取内容
	

	int	ch;					///< for commend line
	int	pf_num; 			///< read numbers 
	int	type;				///< function choosing sign
	int	i2c_tag;				///< for iic_status()
	int	reg_wr_num = 0;		///< for iic_write_all ,to input how many register u wanner write

	
	
	if(argc>8)
	{
		reg_val= strtol(argv[8],NULL,16);
	}


	opterr =0 ;
	while( ( ch = getopt( argc, argv, "o:rwa:d:v:n:t" ) ) != -1 )
	{
		switch( ch )
		{
			case 'o':
				type = 0;
				i2c_tag = strtol(optarg, NULL, 10);
				//printf("i2c status %d, i2c tag %d\n", i2c_statu, i2c_tag);
				break;
				
			case 'a' :
				addr = strtol(optarg, NULL, 16);
				break;
	
			case 'd' :
				//printf( "\t opt_%s   ch_%c\n", optarg, ch);
				reg = strtol(optarg, NULL, 16);
				break;
				

			case 'r':
				type = 1;
				break;
				
			case 'n' :
				pf_num = strtol(optarg, NULL, 10);
				break;

			case 'w':
				type = 2;
				break;
				
			case 'v' :
				reg_val = strtol(optarg, NULL, 16);
				break;
				
			case 't' :
				type = 3;
				break;
				
			default:
				break;
		}
	}


	switch(type)
	{
		case 0:
			if(argc != 3)
			{
				help();
				return -1;
			}
			else
			{
				iic_status(i2c_tag);
			}
			break;

		case 1:
			if(argc != 8)
			{
				help();
				return -1;
			}
			else
			{
				iic_read(addr, reg, reg_val, pf_num);
			}
			break;

		case 2:
			if(argc != 8)
			{
				help();
				return -1;
			}
			else
			{
				iic_write(addr, reg, reg_val);
			}
			break;

		case 3:
			if(argc != 4)
			{
				help();
				return -1;
			}
			else
			{	
				reg_wr_num = strtol(argv[3], NULL, 10);
				iic_write_all(argv[2], &reg_wr_num);
			}
			break;
			
	}



	return 0;	
}
