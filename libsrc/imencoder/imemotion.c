/*
imemotion.c 	by lsk 2006 -11-24
ime6410 移动侦测处理函数
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <file_def.h>	//包含了电路板型号的定义
#include "ime6410.h"
#include "ime6410api.h"
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <commonlib.h>
#include <devinfo.h>
#include <tw9903api.h>
#include "imemotion.h"


#define BYTE_NUM_D1		203
#define BYTE_NUM_HD1	102
#define BYTE_NUM_CIF	50
#define BYTE_NUM_QCIF	13
#define UNUSED_BIT_D1	4
#define UNUSED_BIT_HD1	6
#define UNUSED_BIT_CIF	4
#define UNUSED_BIT_QCIF	5
	
//#define SET_SENSE(a)		(40 - (a)*7)	//lsk 2007-6-27

struct motion_para
{
	 unsigned int    sense;
	 unsigned int    pic_size;
	 unsigned int    unused_bits;
	 unsigned char motion_area_D1[BYTE_NUM_D1];
	 unsigned char motion_area_HD1[BYTE_NUM_HD1];
	 unsigned char motion_area_CIF[BYTE_NUM_CIF];
	 unsigned char motion_area_QCIF[BYTE_NUM_QCIF];
};

struct motion_para md_para[4];
//// 灵敏度参数表  每一个灵敏度参数对应4个门限参数
const unsigned int sens_paras[4][4]=
{ 
	{5, 		5000, 	255,		5000},
  	{3,		2000,	108,		2000},
   	{1, 		1000,	50,		1000},
   	{15,		15000,	765,		15000},
};
FILE* test_fp=NULL;
unsigned char test_buf[1024];

int set_sence_level(int sens)
{
	switch(sens)
	{
		case 0:
		return 63;
		break;

		case 1:
		return 14;
		break;
		
		case 2:
		return 10;
		break;
		
		case 3:
		return 8;
		break;
		
		case 4:
		return 6;
		break;
		
		case 5:
		return 4;
		break;

		default:
		return 8;
		break;
	}
	return 14;
}

#if 0
void print_buffer_test(unsigned char* buf, int len)
{
	int i,j;
	j=0;
	for(i=0;i<len;i++)
	{
		j++;
		printf("%02x ", buf[i]);
		if(j>=10)
		{
			printf("\n");
			j=0;
		}
	}
	printf("\n\n");
}
#endif
void print_buffer_test(unsigned char* buf, int len)
{
	int i, j, k;
	j=0;
	for(i=0;i<len;i++)
	{
		for(k=7;k>=0;k--)
		{
			if((buf[i]&(0x01<<k)))
			{
				printf("%c", '*');
			}
			else
			{
				printf("%c", ' ');
			}
			j++;
		}
		
		if(j>=45)
		{
			printf("\n");
			j=0;
		}
	}
	printf("\n\n");
}

void fprint_buffer_test(FILE*fp, unsigned char* buf, int len)
{
	int i, j, k;
	j=0;
	for(i=0;i<len;i++)
	{
		j++;
		for(k=7;k>=0;k--)
		{
			if((buf[i]&(0x01<<k)))
			{
				fprintf(fp,"%c", '*');
			}
			else
			{
				fprintf(fp,"%c", ' ');
			}
		}
		if(j>=6)
		{
			fprintf(fp,"\n");
			j=0;
		}
	}
	fprintf(fp,"\n\n");
}
#if 0
int I64_restart(struct compress_struct *enc)
{
	int fd;
//	int command;
	struct I64Reg_int  *pinfo = NULL;
	if(enc==NULL)
	{
		return -1;
	}

	fd = enc->fd;
	pinfo = (struct I64Reg_int *) &(enc->i64reg);

	pinfo->com &= 0xfffffffc;
	pinfo->com |= 0x1;
	ioctl(fd, REG_COMMAND | CMD_WRITEM, pinfo->com);
	return 0;
}

int I64_stop(struct compress_struct *enc)
{
	int fd;
	int command;
	struct I64Reg_int  *pinfo = NULL;
	if(enc==NULL)
	{
		return -1;
	}
	fd = enc->fd;
	pinfo = (struct I64Reg_int *) &(enc->i64reg);

	command = ioctl(fd, REG_COMMAND, NULL);
	pinfo->com = command;
	pinfo->com &= 0xfffffffc;
	pinfo->com |= 0x2;
	ioctl(fd, REG_COMMAND | CMD_WRITEM, pinfo->com);
	return 0;
}
#endif
/*
*************************************************************
 * 函数名	:set_ime_motion_sens()
 * 功能	: 设置 ime6410 移动侦测灵敏度 
 * 输入	: 
 		struct compress_struct *enc  编码器参数
 		int ch 视频通道号
 		int sens 灵敏度  0:禁止 5表示灵敏度最高
* 返回值	:0 表示成功，负值表示失败
*************************************************************
 */
int set_ime_motion_sens(struct compress_struct *enc, int ch, int sens)
{
	int fd;
	//unsigned int addr=0;
	struct I64Reg_int  *pinfo=NULL;

	if(enc==NULL)
	{
		return -1;
	}
	pinfo=(struct I64Reg_int  *)&enc->i64reg;
	fd = enc->fd;
	if(sens>5)
	{
		sens = 5;
	}
	if(sens <0)
	{
		sens = 0;
	}
	if(ch>3)
	{
		ch = 3;
	}
	if(ch <0)
	{
		ch = 0;
	}
	//lsk test sens 
//	printf("set channal %d sens to %d \n",ch, SET_SENSE(sens));

	switch(ch)
	{
		case 0:
		if(sens==0)
		{
			enc->i64reg.md.Ch1MdType = 0;
//			enc->i64reg.md.Ch1MdEnable =0;
		}
		else
		{
			enc->i64reg.md.Ch1MdType = 1;
//			enc->i64reg.md.Ch1MdEnable =0;
		}
		enc->i64reg.md.Ch1MdLevel=set_sence_level(sens);
		break;

		case 1:
		if(sens==0)
		{
			enc->i64reg.md.Ch2MdType = 0;
//			enc->i64reg.md.Ch2MdEnable =0;
		}
		else
		{
			enc->i64reg.md.Ch2MdType = 1;
//			enc->i64reg.md.Ch2MdEnable =0;
		}
		enc->i64reg.md.Ch2MdLevel=set_sence_level(sens);
		break;

		case 2:
		if(sens==0)
		{
			enc->i64reg.md.Ch3MdType = 0;
//			enc->i64reg.md.Ch3MdEnable =0;
		}
		else
		{
			enc->i64reg.md.Ch3MdType = 1;
//			enc->i64reg.md.Ch3MdEnable =0;
		}
		enc->i64reg.md.Ch3MdLevel=set_sence_level(sens);
		break;

		case 3:
		if(sens==0)
		{
			enc->i64reg.md.Ch4MdType = 0;
//			enc->i64reg.md.Ch4MdEnable =0;
		}
		else
		{
			enc->i64reg.md.Ch4MdType = 1;
//			enc->i64reg.md.Ch4MdEnable =0;
		}
		enc->i64reg.md.Ch4MdLevel=set_sence_level(sens);
		break;
	}
	if(fd<0)
	{
//		printf("cna not start ime6410 \n");
		return -1;
	}
	ioctl(fd, REG_MOTIONDETECT | CMD_WRITEM, pinfo->md);

#if 0
	if(I64_stop( enc))
	{
		printf("cna not stop ime6410 \n");
		return -1;
	}
	if(sens>SCR_MODE_QCIF)
	{
		sens = SCR_MODE_QCIF;
	}
	if(sens <SCR_MODE_D1)
	{
		sens = SCR_MODE_D1;
	}
	
	md_para[ch].sense = sens;
	pinfo->md_thred[ch][0] = ((sens_paras[sens][0]<<16)|sens_paras[sens][1]);
	pinfo->md_thred[ch][1] = ((sens_paras[sens][2]<<16)|sens_paras[sens][3]);

	addr = REG_MDTHRED0+ ch*8;
	ioctl(fd, addr | CMD_WRITEM, pinfo->md_thred[ch][0]);
	ioctl(fd, (addr+4) | CMD_WRITEM, pinfo->md_thred[ch][1]);
	printf("reg %08x = %08x \n", addr, pinfo->md_thred[ch][0]);
	printf("reg %08x = %08x \n", addr+4, pinfo->md_thred[ch][1]);
	printf("reg %08x = %08x \n", REG_COMMAND,  ioctl(fd, REG_COMMAND, NULL));

	if(I64_restart(enc))
	{
		printf("cna not start ime6410 \n");
		return -1;
	}
	printf("reg %08x = %08x \n", REG_COMMAND,  ioctl(fd, REG_COMMAND, NULL));
#endif
	return 0;
}
/*
*************************************************************
 * 函数名	:set_ime_motion_area()
 * 功能	: 设置 ime6410 移动侦测区域
 * 输入	: 
 		unsigned char ch 视频通道号 
 		unsigned short *area  区域参数缓冲区
 		1个bit对应一个宏模块
 		0 为不进行移动侦测
 		1 需要进行移动侦测
 		字节数取决于图像的大小
 		 int pic_size  图像的大小 (D1,  HD1, CIF)
720×576（D1模式）: 1620个移动侦测宏模块，203个字节来表示移动侦测的状态。
720×288（HD1模式）: 810个移动侦测宏模块，102个字节来表示移动侦测的状态。
352×288（CIF模式）: 396个移动侦测宏模块，50个字节来表示移动侦测的状态。
 * 返回值	:0 表示成功，负值表示失败
*************************************************************
 */
int set_ime_motion_area(unsigned char ch, unsigned short *area, int pic_size)
{
	if(area==NULL)
	{
		return -1;
	}
	if(ch>3)
	{
		ch = 3;
	}

	switch(pic_size)
	{
		case SCR_MODE_QCIF:
			memcpy(md_para[ch].motion_area_QCIF, (unsigned char*)area, BYTE_NUM_QCIF);
			md_para[ch].unused_bits = UNUSED_BIT_QCIF;
			break;
		case SCR_MODE_HD1:
			memcpy(md_para[ch].motion_area_HD1, (unsigned char*)area, BYTE_NUM_HD1);
			md_para[ch].unused_bits = UNUSED_BIT_HD1;
			break;
		case SCR_MODE_CIF:
			memcpy(md_para[ch].motion_area_CIF, (unsigned char*)area, BYTE_NUM_CIF);
			md_para[ch].unused_bits = UNUSED_BIT_CIF;
			break;
		case SCR_MODE_D1:
			memcpy(md_para[ch].motion_area_D1, (unsigned char*)area, BYTE_NUM_D1);
			md_para[ch].unused_bits = UNUSED_BIT_D1;
			break;
		default:
			return -1;
			break;
	}
	md_para[ch].pic_size = pic_size;				
	return 0;
}

/*
*************************************************************
 * 函数名	:set_ime_motion_para()
 * 功能	: 设置 ime6410 移动侦测参数
 * 输入	: 
 		struct compress_struct *enc  编码器参数
 		unsigned char ch 视频通道号
 		int sen	灵敏度参数
 		unsigned short *area  区域参数缓冲区
 		 int pic_size  图像的大小 (D1,  HD1, CIF)
 * 返回值	:0 表示成功，负值表示失败
*************************************************************
 */
int set_ime_motion_para(struct compress_struct *enc, unsigned char ch, int sen,unsigned short *area, int pic_size)
{
	if(set_ime_motion_area(ch, area, pic_size))
	{
		printf("error set motion area \n");
		return -1;
	}
	if(set_ime_motion_sens(enc, ch, sen))
	{
		printf("error set motion sense \n");
		return -1;
	}
	return 0;
}
/*
*************************************************************
 * 函数名	:get_ime_motion_stat()
 * 功能	: 设置 ime6410 移动侦测灵敏度 
 * 输入	: 
 		struct compress_struct *enc  编码器参数
 		int ch 视频通道号
 * 返回值	:0 表示没有移动, 1表示有移动，负值表示失败
*************************************************************
 */
int get_ime_motion_stat(struct compress_struct *enc, int ch)
{
	int fd=-1;
	int var=0;
	if((enc==NULL)||(enc->fd<0))
	{
		return -1;
	}
	if(ch>3)
	{
		ch = 3;
	}
	if(ch <0)
	{
		ch = 0;
	}
	fd = enc->fd;
	
//	var = ioctl(fd, REG_MOTIONDETECT, NULL);	//test lsk 2007 -6 -27
//	printf("get motion para 0x%08x\n", var);	//test lsk 

	var = ioctl(fd, REG_MDFRAMECNT, NULL);
	//remed by shixin	printf("get motion para 0x%08x\n", var);
	var = (var>>(ch*8))&0xff;
	if(var!=(enc->md_var[ch]))
	{
		enc->md_var[ch] = var;
		return 1;
	}
	return 0;
}
/*
*************************************************************
 * 函数名	:process_motion_pkt()
 * 功能	: 处理ime6410 移动侦测数据包 
 * 输入	: 
  		int ch 视频通道号
		unsigned char *buf  参数缓冲区
 		 int pic_size  图像的大小 (D1,  HD1, CIF)
 * 返回值	:0 表示没有移动侦测，1表示有移动侦测， 负值表示失败
*************************************************************
 */
int process_motion_pkt(unsigned char ch, unsigned char *buf, int pic_size)
{
	int len;
	int i;
	int ret=0;
	unsigned int unused_bit;
	unsigned char *area=NULL;

	if(buf==NULL)
	{
		return -1;
	}
	if(ch>3)
	{
		ch = 3;
	}
	test_fp = fopen("/tmp/mdtest.txt", "a+");
	if(test_fp == NULL)
	{
		printf("error open record file\n");
		return -1;
	}
	printf("get motion data\n");
//	fprintf(test_fp, "get motion data\n");
//	fflush(test_fp);
//	printf("get motion data pic_size=%d channel = %d\n", pic_size, ch);
	switch(pic_size)
	{
		case SCR_MODE_QCIF:
			area = md_para[ch].motion_area_QCIF;
			md_para[ch].unused_bits = UNUSED_BIT_QCIF;
			len = BYTE_NUM_QCIF;
			break;
			
		case SCR_MODE_HD1:
			area = md_para[ch].motion_area_HD1;
			md_para[ch].unused_bits = UNUSED_BIT_HD1;
			len = BYTE_NUM_HD1;
			break;
			
		case SCR_MODE_CIF:
			area = md_para[ch].motion_area_CIF;
			md_para[ch].unused_bits = UNUSED_BIT_CIF;
			len = BYTE_NUM_CIF;
			break;
			
		case SCR_MODE_D1:
			area = md_para[ch].motion_area_D1;
			md_para[ch].unused_bits = UNUSED_BIT_D1;
			len = BYTE_NUM_D1;
			break;
			
		default:
			return -1;
			break;
	}
//	print_buffer_test(buf, len+4);
//	memset(test_buf, 0 , sizeof(test_buf));
	print_buffer_test(buf, len);
//	fprint_buffer_test(test_fp, buf, len);
//	fflush(test_fp);
	fclose(test_fp);
#if 1
	for(i=0;i<(len-1);i++)
	{
//		printf("%02x ", area[i]&buf[i]);
		if(test_buf[i]!=buf[i])
		{
			test_buf[i] = buf[i];
			if(area[i]&test_buf[i])
			{
				ret = 1;
			}
		}
		
	}
	unused_bit = md_para[ch].unused_bits;
	if((test_buf[i]>>unused_bit)!=(buf[i]>>unused_bit))
	{
		test_buf[i] = buf[i];
		if((area[i]>>unused_bit)&(test_buf[i]>>unused_bit))
		{
	//		printf("%02x \n", area[i]&buf[i]);
			ret = 1;
		}
	}
	if(ret ==1)
	{
		printf("channal %d motion detacted \n", ch);
	}
#endif
	return ret;
}


