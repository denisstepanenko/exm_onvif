/**
	@file:	record_i2creg.c
	@brief:	test assigned register of one chip
			1. test  registers to see which get a error
			2. can test for any register of any chip
	@param:	record_i2creg -a<chip addr>  -s<test interval (seconds)>  -n<test register number>  -r<register addr1 addr2 addr...>  ; 	
	@modify:	08.09.19  have finished
			08.09.26  modified once
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tw9910api.h>
#include <unistd.h>
#include <time.h>
#include <iiclib.h>

#define REG_MAX_NUM	20   ///<要读取的寄存器最大数目

static int i2c_init = -1;			///<iic设备的文件描述符



static void help(void)
{
	printf("record_i2creg -a 芯片地址-s 读取时间间隔(   秒)  -n 要读取的寄存器数目-r 寄存器地址1 地址2 地址...  \n");
	printf("************寄存器组赋值存取文档路径放在/log/collectreg.txt**********\n");
	return;
}

static void init_iic(void)
{
	
	i2c_init = open_iic_dev();
	if(i2c_init < 0)
	{
         	printf("打开iic设备%s 失败!\n", i2c_init);
              exit(1);
	}

	return;
}

int main(int argc,char *argv[]) {

	int 		ret;
	int 		addr;		///<芯片地址
	unsigned char  reg[REG_MAX_NUM]={0};  		 ///<寄存器地址
	unsigned char reg_val[REG_MAX_NUM] = {0};	///<寄存器值
	unsigned char	*reg_val_pf; 				///< 所有要打印的寄存的值
	int ch;		///< 命令行返回值
	int tag = 0;	///<	命令行输入判断
	
	unsigned int reg_num;  ///< 记录寄存器个数
	unsigned int second; ///< 记录间隔
	unsigned int i = 0, j = 6;
	time_t timep; ///< 时钟参数

	FILE *fp = NULL;     
	unsigned char reg_buf[100]; 
	unsigned char reg_temp[REG_MAX_NUM] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}; 


	/**
		检查命令行参数
	*/
	opterr =0 ;
	while( ( ch = getopt( argc, argv, "a:s:n:r:" ) ) != -1 )
	{
		switch( ch )
		{
			case 'a' :
				//printf( "\t opt_%s   ch_%c\n", optarg, ch);
				addr = strtol(optarg, NULL, 16);
				break;
				
			case 'r' :
				//printf( "\t opt_%s   ch_%c\n", optarg, ch);
				reg[0] = strtol(optarg, NULL, 16);
				break;
				
			case 's' :
				//printf( "\t opt_%s   ch_%c\n", optarg, ch);
				second = strtol(optarg, NULL, 10);
				//printf("%d\n",pf_num);
				break;
				
			case 'n' :
				//printf( "\t opt_%s   ch_%c\n", optarg, ch);
				reg_num = strtol(optarg, NULL, 10);
				break;
				
			default:
				break;
		}
	}

		
	///检查命令行个数
	if (argc < ( 8 + reg_num))
	{
		help();
		return -1;
	}

	///寄存器最大数目不要超过REG_MAX_NUM
	if(reg_num > REG_MAX_NUM)
	{
		printf("The registers you want read have beyond %d\n", REG_MAX_NUM);
		return;
	}

	///寄存值赋值
	for(i = 1; i < reg_num; i++)
	{
		reg[i] = strtol(argv[(i+8)], NULL, 16);
	}

	init_iic();

	fp = fopen("/log/collectreg.txt", "a+");
	if(fp == NULL)
	{
		printf("open collectreg failed \n");
		return -1;
	}

	///初始寄存器临时存储值，以供对比差异
	for(i = 0; i < reg_num; i++)
	{
		ret=read_iic_reg(addr, reg[i], &reg_temp[i]);
		if(ret<0)
		{
			printf("读取芯片%02x,addr:%02x失败!\n",addr,reg_temp[i]);
			exit(1);
		}
		time(&timep);
		sprintf(reg_buf,"chip:%02x %02x=%02x  current time=%s\n", addr, reg[i], reg_temp[i], ctime(&timep));
		printf("%s", reg_buf);
		fputs(reg_buf, fp);
		fflush(fp);
	}

	///未输入-n参数
		while(1)
		{
			for(i = 0; i < reg_num; i++)
			{
				reg_val[i] = reg_temp[i];
				ret=read_iic_reg(addr, reg[i], &reg_temp[i]);
				if(ret<0)
				{
					printf("读取芯片%02x,addr:%02x失败!\n",addr,reg_temp[i]);
					exit(1);
				}	
				if(reg_val[i] != reg_temp[i])
				{
					time(&timep);
					sprintf(reg_buf,"chip:%02x %02x=%02x  current time=%s\n", addr, reg[i], reg_temp[i], ctime(&timep));
					printf("%s", reg_buf);
					fputs(reg_buf, fp);
					fflush(fp);
					sleep(second);
				}
				else
				{
					sleep(second);
				}
				//printf("argc=7 reg_temp_%02x\n", reg_temp);
			}
		}
	

	fclose(fp);
	return 0;	
}

