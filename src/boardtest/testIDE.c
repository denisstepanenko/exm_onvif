#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <file_def.h>
#include <sys/vfs.h>
#include "testmod.h"
#include "pub_err.h"
#include "testIDE.h"
#include <signal.h>
#include <diskinfo.h>// added by lsk 2006 -12-14
#include <commonlib.h>
//#include "multicast_ctl.h"

typedef struct 
{
	int CF_flag;
	int Disk_flag;
	int Disk_partition;
}Disk_struct;

//static int IDE_fd=-1;
Disk_struct disk[4];
#if 0
/*
 * 将测试结果数据结构解析后存入指定的文件
 */

/*
 * 模拟测试流程
 */
//获取磁盘总容量k为单位
long get_disk_total(char *mountpath)
{
		 struct statfs	buf;
		 if(mountpath==NULL)
			return -1;
		 if(statfs(mountpath,&buf)<0)
		 {
			printf("error at check_disk\n");
			printf("获取磁盘总大小时失败，路径%s\n",mountpath);
			return -1;
		 }
		return buf.f_blocks*(buf.f_bsize>>10);
}
#endif
//如果磁盘容量大于64K 则认为CF卡可用
//否则认为CF卡不可用
#if 0
int get_cf_avail(void)
{
	
	long disktotal;
	disktotal=get_disk_total(HDMOUNT_PATH);
	printf(" 磁盘容量= %ld M\n", disktotal);
//	if (disktotal<1024*64)
	if (disktotal<200)
	{
		if(CF_flag==1)
		{
			printf("容量小于200M  !!\n");
			printf("没有检测到可用的CF卡!!\n");
		 	CF_flag=0;
		} 
		return(-1);
	}
	else if(disktotal>20000)
	{
		if(CF_flag==0) 
		{	
			CF_flag=1;
			printf("检测到可用的硬盘\n");
		}
		return(0);
	}
	else 
	{
		if(CF_flag==0) 
		{	
			CF_flag=1;
			printf("检测到可用的CF卡\n");
		}
		return(0);
	}
}
#endif
/*
*****************************************************
*函数名称: IDE_read_write_test
*函数功能: 测试磁盘文件读写
*参数	: 无
*返回值   : 0 成功 -1 失败
*lsk 2007-11-8
*****************************************************
*/
#define IDE_TEST_FILE_SIZE	1024*10
#define IDE_TEST_BUF_LEN	256
int IDE_read_write_test(void)
{
	FILE* fp=NULL;
	int i,j,ret;
	BYTE temp[IDE_TEST_BUF_LEN];
	BYTE check[IDE_TEST_BUF_LEN];
	char * testfile="/hqdata/sda1/ide_test.txt"; 	// 必须在/hqdata路径下创建文件
	BYTE cmd_buf[BUF_LEN];
	memset(cmd_buf,0,sizeof(cmd_buf));
	sprintf(cmd_buf,"rm -rf %s\n",testfile);
	fp = fopen(testfile, "wb+");
	if(fp==NULL)
	{
		printf("error open %s\n", testfile);
		gtlogerr("error open %s\n", testfile);
		return 3;
	}
	for(i=0;i<sizeof(temp);i++)
	{
		temp[i] = i;
	}
////在文件中写入测试数据
	for(j=0;j<(IDE_TEST_FILE_SIZE/IDE_TEST_BUF_LEN);j++)
	{
		for(i=0;i<sizeof(temp);i++)
		{
			ret = fwrite(&temp[i], 1,1,fp);
			if(ret!=1)
			{
				fclose(fp);
				system(cmd_buf);
				printf("write file %s error \n", testfile);
				return 4;
			}
		}
	}
	fclose(fp);
	fp = NULL;
	sleep(1);
	fp = fopen(testfile, "rb");
	if(fp==NULL)
	{
		fclose(fp);
		system(cmd_buf);
		printf("error open %s\n", testfile);
		gtlogerr("error open %s\n", testfile);
		return 3;
	}
////读取数据比较
	for(j=0;j<(IDE_TEST_FILE_SIZE/IDE_TEST_BUF_LEN);j++)
	{
		memset(check, 0, sizeof(check));
		for(i=0;i<sizeof(check);i++)
		{
			ret = fread(&check[i], 1,1,fp);
			if(ret!=1)
			{
				fclose(fp);
				system(cmd_buf);
				printf("read file %s error \n", testfile);
				gtlogerr("read file %s error \n", testfile);
				return 5;
			}
			if(check[i]!=temp[i])
			{
				fclose(fp);
				system(cmd_buf);
				printf("read data from file %s error \n", testfile);
				gtlogerr("read data from file %s error \n", testfile);
				return 5;
			}
		}
	}
	fclose(fp);
	system(cmd_buf);
	printf("IDE file test ok\n");
	gtloginfo("IDE file test ok\n");
	return 0;
}

//测试IDE设备
/*
*****************************************************
*函数名称: test_IDE
*函数功能: 测试IDE控制模块函数
*参数	: 
*		multicast_sock*ns 存放网络参数的数据结构
*		*int prog 进度	
*返回值   : 错误代码
*lsk 2006-12-14 
*****************************************************
*/
int test_IDE(multicast_sock* ns, int* prog)
{
	int i;
	int part=0;
	int part_fg=0;
	int disk_fg=0;
	long int cap[4];
	char disk_name[50];
	unsigned char buf[200];
	i= get_sys_disk_num();
	if (i<=0)
	{
		printf("找不到 IDE 设备\n");
		*prog+=20;
		send_test_report(ns, "找不到IDE设备", *prog);
		return 1;
	}
	printf("找到  %d 个IDE设备\n", 1);
	for(i=0;i<1;i++)
	{
		disk[i].CF_flag = 0;
		disk[i].Disk_flag = 0;
		disk[i].Disk_partition = 0;
		part = 0;
		switch(i)
		{
			case 0:
				memset(disk_name, 0 ,sizeof(disk_name));
				memcpy(disk_name, MASTER_DISK, strlen(MASTER_DISK));
			break;
			case 1:
				memset(disk_name, 0 ,sizeof(disk_name));
				memcpy(disk_name, SLAVE1_DISK, strlen(SLAVE1_DISK));
			break;
			case 2:
				memset(disk_name, 0 ,sizeof(disk_name));
				memcpy(disk_name, SLAVE2_DISK, strlen(SLAVE2_DISK));
			break;
			case 3:
				memset(disk_name, 0 ,sizeof(disk_name));
				memcpy(disk_name, SLAVE3_DISK, strlen(SLAVE3_DISK));
			break;
		}
		
		cap[i] = get_sys_disk_capacity(disk_name);
		
		if(cap[i]>0)
		{
			disk_fg = 1;
			memset(buf, 0, sizeof(buf));
			if(cap[i]<200)
			{
				sprintf(buf, "%s %s disk capacity = %ldM", "没有检测到可用的CF卡!", 
						disk_name, cap[i]);
			}
			else if((cap[i]>=200)&&(cap[i]<=2000))
			{
				disk[i].CF_flag = 1;
				sprintf(buf, "%s %s disk capacity = %ldM", "检测到可用的CF卡!", 
						disk_name, cap[i]);
			}
			else		// if(cap[i]>2000)
			{
				disk[i].Disk_flag = 1;
				sprintf(buf, "%s %s disk capacity = %ldG", "检测到可用的硬盘!", 
						disk_name, cap[i]/1000);
			}
			printf("%s\n", buf);
			*prog=20;
			send_test_report(ns, buf, *prog);

			if(disk[i].Disk_flag||disk[i].CF_flag)
			{
				memset(buf, 0, sizeof(buf));
				part = get_sys_partition_num(disk_name);
				if(part==0)
				{
					sprintf(buf, "%s","磁盘没有格式化分区");
					disk[i].Disk_partition=0;
					part_fg=1;	//not formated disk
				}
				else
				{
					sprintf(buf, "%s 有%d个分区", disk_name, part);
					disk[i].Disk_partition=part;
				}
				printf("%s\n", buf);
				*prog=25;
				send_test_report(ns, buf, *prog);
			}
		}
	}

	if(disk_fg==1)
	{
		if(part_fg==1)
			return 2;
		else
			return IDE_read_write_test();	/// lsk 2007 -11-8 增加文件读写测试
	}
	return 1;
}

#if 0
int test_IDE(void)
{
	if (access(IDE_DEV,F_OK)==0)
	{
		printf("找到 IDE 设备\n");
		if(IDE_fd<0)
		{
			IDE_fd = open(IDE_DEV, O_RDWR);
			printf("open IDE...... \n");
		}
		if(IDE_fd<0)
		{
			printf("打不开IDE 设备!\n");
			return 2;
		}
		if (get_cf_avail())
			return 3;
	}
	else
	{
		printf("找不到 IDE 设备\n");
		return 1;
	}
	return 0; 
}
#endif

