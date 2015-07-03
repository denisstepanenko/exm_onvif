#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <file_def.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <gtthread.h>
#include <commonlib.h>
#include <gt_dev_api.h>
#include <devinfo.h>
#include <nv_pair.h>
#include "diskinfo.h"
#define VERSION 		"0.03"
//ver:0.03    修改了get_sys_disk_devname 和get_sys_disk_name
//ver:0.02	添加对sd卡的支持
//

#define FILE_PATH	 "/proc/partitions"

//#define NO_DISK				3003
#define NO_DISK				0
#define DISK_NAME_ERROR 	3001
#define PART_INDEX_ERROR	3002
#define NO_PARTITIONS		3004
#define ERROR_OPEN_FILE		3005


#define TEMP_SD         ///临时为sd卡加的东西，正式版需要更改sd驱动，让其在/proc/partitions里面能显示分区
#ifdef TEMP_SD
#include <devinfo.h>
#endif



/*
*****************************************************
*函数名称: check_disk_name
*函数功能: 检查磁盘名称
*输入：const char* disk_name 磁盘名称
*输出：
返回值: 成功返回0，负值表示失败
*修改日志：
*****************************************************
*/ 
static  int check_disk_name(const char *disk_name)
{
	char buf[200];
	
	if(disk_name==NULL)
	{
		return -1;
	}
	memset(buf, 0 , sizeof(buf));
	sprintf(buf,"%s",disk_name);
	if(strncmp(buf, MASTER_DISK, strlen(buf))==0)
		return 0;
	if(strncmp(buf, SLAVE1_DISK, strlen(buf))==0)
		return 0;
	if(strncmp(buf, SLAVE2_DISK, strlen(buf))==0)
		return 0;
	if(strncmp(buf, SLAVE3_DISK, strlen(buf))==0)
		return 0;
	
	//添加检查sd名字的部分add by zw
	if(strncmp(buf, SD_MASTER, strlen(buf))==0)
		return 0;
	if(strncmp(buf, SD_SLAVE1, strlen(buf))==0)
		return 0;

	return -1;
}
/*
*****************************************************
*函数名称: open_partition_file
*函数功能: 打开/proc/partitions 文件
*输入：
*输出：
返回值: 成功返回文件指针，NULL表示失败
*修改日志：
*****************************************************
*/ 
static  FILE* open_partition_file(void)
{
	FILE* fp=NULL;
	fp=fopen(FILE_PATH, "r")	;
	if(fp==NULL)
	{
		return NULL;
	}
	return fp;
}
/*
*****************************************************
*函数名称: search_disk
*函数功能: 获取磁盘+分区的数量
*输入：const char* disk_name 磁盘名称
*输出：
返回值: 成功返回磁盘分区个数，负值表示失败
*修改日志：
*****************************************************
*/ 
static  int search_disk(const char*disk_name)
{
	FILE* fp=NULL;
	char* cp=NULL;
	int num=0;
	char buf[200];
    ////#ifdef TEMP_SD
    ////    if(get_ide_flag()==2)   //sd卡
    ////        return 1;
    ////#endif
	fp = open_partition_file();
	if(fp==NULL)
	{
		return -ERROR_OPEN_FILE;
	}
	if(disk_name==NULL)
	{
		fclose(fp);
		return -NO_DISK;
	}
		
	memset(buf, 0, sizeof(buf));
	while(fgets(buf, sizeof(buf),fp)!=NULL)
	{
		cp = strstr(buf, disk_name);
		if(cp!=NULL)
		num++;
		memset(buf, 0, sizeof(buf));
		cp = NULL;
	}
	fclose(fp);
	if(num>0)
		return num;
	return -NO_DISK;
}
/*
*****************************************************
*函数名称: find_disk_capacity
*函数功能: 获取磁盘分区容量
*输入：const char* disk_name 磁盘名称
*输出：
返回值: 成功返回磁盘分区的容量(单位K)，0 没有磁盘，负值表示失败
*修改日志：
*****************************************************
*/ 
static  long int find_disk_capacity(const char *disk_name)
{
	FILE* fp=NULL;
	char name[20];
	char buf[2][50];
	long int cap;
    ////#ifdef TEMP_SD
    ////    if(get_ide_flag()==2)   //sd卡
    ////        return 2*1000*1000; //2G的sd卡
    ////#endif    
	fp = open_partition_file();
	if(fp==NULL)
	{
		return -ERROR_OPEN_FILE;
	}
	if(disk_name==NULL)
	{
		fclose(fp);
		return -NO_DISK;
	}
		
	memset(name, 0 ,sizeof(name));
	fgets(buf[0], sizeof(buf[0]), fp);//discard first line
	memset(buf,0,sizeof(buf));
	memcpy(name,disk_name, strlen(disk_name));
	while(feof(fp)==0)
	{
		fscanf(fp, "%s", buf[0]);
		if(strncmp(buf[0], name, strlen(name))==0)
		{
			cap = atol(buf[1]);
			fclose(fp);
			return (cap/1000)*1024;
		}
		memset(buf[1],0,sizeof(buf[1]));
		memcpy(buf[1], buf[0], strlen(buf[0]));
		memset(buf[0],0,sizeof(buf[0]));
	}
	fclose(fp);
	return -NO_DISK;
}
/*
*****************************************************
*函数名称: get_sys_disk_num
*函数功能: 获取磁盘数量
*输入：
*输出：
返回值: 成功返回磁盘区个数
*修改日志：
*****************************************************
*/ 
int get_sys_disk_num(void)
{
	int num=0;
    ////#ifdef TEMP_SD
    ////    if(get_ide_flag()==2)   //sd卡
    ////       return 1;
    ////#endif    
    #if 0
	init_devinfo();
	if(get_ide_flag()==2)
	{
		//处理SD卡的情况	
		if(search_disk(SD_MASTER)>0)
			num++;
		if(search_disk(SD_SLAVE1)>0)
			num++;
	}    
	else
	{
	#endif
		//处理硬盘的情况
		if(search_disk(MASTER_DISK)>0)
			num++;
		if(search_disk(SLAVE1_DISK)>0)
			num++;
		if(search_disk(SLAVE2_DISK)>0)
			num++;
		if(search_disk(SLAVE3_DISK)>0)
			num++;
	//}

//	if(num>0)
	return num;
//	return -NO_DISK;
}



/*
*****************************************************
*函数名称: get_sys_disk_capacity
*函数功能: 获取磁盘容量
*输入：const char* disk_name 磁盘名称
*输出：
返回值: 成功返回磁盘容量(单位M)，0表示没有磁盘，负值表示失败
*修改日志：
*****************************************************
*/ 
long int get_sys_disk_capacity(const char* disk_name)
{
	int ret=0;
	if(check_disk_name(disk_name))
	return -NO_DISK;
	ret = find_disk_capacity(disk_name);
	if(ret<0)
	return ret;
	return ret/1000;
}
/*
*****************************************************
*函数名称: get_sys_partition_num
*函数功能: 获取磁盘分区数量
*输入：const char* disk_name 磁盘名称
*输出：
返回值: 成功返回磁盘分区个数，0 表示没有分区，负值表示失败
*修改日志：
*****************************************************
*/ 
int get_sys_partition_num(const char*disk_name)
{
	int num=0;

    ////#ifdef TEMP_SD
    ////    if(get_ide_flag()==2)   //sd卡
    ////        return 1;
    ////#endif    
   
	if(check_disk_name(disk_name))
		return -NO_DISK;  
	num = search_disk(disk_name);

	if(num<0)
	return num;

//	if(num ==1)
//	return -NO_PARTITIONS;

//	if(num==0)
//	return -NO_DISK;

	return num-1;
}
/*
*****************************************************
*函数名称: get_sys_partition_capacity
*函数功能: 获取磁盘分区容量
*输入：const char* disk_name 磁盘名称
*	    int partition_index 磁盘分区索引号
*输出：
返回值: 成功返回磁盘容量(单位M)，0表示没有分区，负值表示失败
*修改日志：
*****************************************************
*/ 
long int get_sys_partition_capacity(const char*disk_name, int partition_index)
{
	int ret=0;
	char buf[100];

	if(check_disk_name(disk_name))
		return -NO_DISK;

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%s%d",disk_name,partition_index);
	ret = find_disk_capacity(buf);
	if(ret<0)
	return ret;
	return ret/1000;
}



/*add-by-wsy 2007-11-21
*****************************************************
*函数名称: get_sys_disk_devname
*函数功能: 获取磁盘节点名称
*输入：diskno:磁盘编号(从0开始)
*输出：
返回值: 成功返回磁盘节点名称字符串,形如"/dev/hda"，失败返回null
*****************************************************
*/ 
char *get_sys_disk_devname(int diskno)
{
#if 0
//// lsk 2009-8-9  sd check
	int type;
	init_devinfo();
	type=get_ide_flag();
	if(type==2)
	{
		switch(diskno)
		{
			case(0): return SD_DISK0;
			case(1): return SD_DISK1;
			default: return NULL;
		}
	}
#endif
	switch(diskno)
	{
		case(0):	return DEVDISK0;
		case(1):	return DEVDISK1;
		case(2):	return DEVDISK2;
		case(3):	return DEVDISK3;
		default:	return NULL;
	}	
}

/*****************************************************
*函数名称: get_sys_disk_name
*函数功能: 获取磁盘名称
*输入：diskno:磁盘编号(从0开始)
*输出：
返回值: 成功返回磁盘名称字符串,形如"hda"，失败返回null
*****************************************************
*/ 
char *get_sys_disk_name(int diskno)
{
//// lsk 2009-8-9  sd check
#if 0
	int type;

	init_devinfo();
	type=get_ide_flag();
	if(type==2)
	{
		switch(diskno)
		{
			case(0): return SD_MASTER;
			case(1): return SD_SLAVE1;
			default: return NULL;
		}
	}
#endif	
	switch(diskno)
	{
		case(0):	return MASTER_DISK;
		case(1):	return SLAVE1_DISK;
		case(2):	return SLAVE2_DISK;
		case(3):	return SLAVE3_DISK;
		default:	return NULL;
	}	
}


/*****************************************************
*函数名称: get_sys_disk_partition_num
*函数功能: 获取磁盘分区数量
*输入：const char* disk_name 磁盘名称,形如"/dev/hda"
*输出：
返回值: 成功返回磁盘分区个数，0 表示没有分区，负值表示失败
*修改日志：
*****************************************************/
int get_sys_disk_partition_num(char *disk_devname)
{
	int num=0;
	char disk_name[20];//形如"hda",以便调用get_sys_partition_num
	
	if(disk_devname == NULL)
		return -EINVAL;
	
	strncpy(disk_name,disk_devname+5,19);
	
	return get_sys_partition_num(disk_name);
}

/*************************************************************************
 * 	函数名:	get_sys_disk_partition_name()
 *	功能:	获取指定序号的分区节点名
 *	输入:	diskno,磁盘序号，从0-3
 *			part_no,分区序号，从1开始
 *	输出:	partitionname, 填充好分区节点名称的字符串指针
 * 	返回值:	形如 "/dev/hda1"的分区节点名称 ,错误返回NULL
 *************************************************************************/
char * get_sys_disk_partition_name(IN int diskno, IN int part_no,OUT char * partitionname)
{
	FILE* fp=NULL;
	char* cp=NULL;
	int num=0;
	int result = -NO_PARTITIONS;
	char buf[200];
	
	if(partitionname == NULL)
		return NULL;
#if 0
    #ifdef TEMP_SD
    	init_devinfo();
        if(get_ide_flag()==2)   //sd卡
        {
            sprintf(partitionname,"/dev/sda1");
            return partitionname;
        };
    #endif    
#endif
	fp = open_partition_file();
	if(fp==NULL)
	{
		return NULL;
	}
	
	memset(buf, 0, sizeof(buf));
	while(fgets(buf, sizeof(buf),fp)!=NULL)
	{
		cp = strstr(buf, get_sys_disk_name(diskno));
		if(cp!=NULL)
		{
			if(++num == part_no+1)
			{
				sprintf(partitionname,"/dev/%s",cp);
				cp = index(partitionname,'\n');
				if(cp!= NULL)
					*cp = '\0';
				fclose(fp);
				return partitionname;
			}
		}	
	}
	fclose(fp);
	return NULL;
	
}


/*
*****************************************************
*函数名称: get_disk_capacity
*函数功能: 直接从/sys/block/sda/size下获取磁盘分区容量
*输入：const char* disk_name 磁盘名称
*输出：
返回值: 成功返回磁盘分区的容量(单位G)，0 没有磁盘，负值表示失败
*修改日志：yk add20130705
*****************************************************
*/ 
long long  get_disk_capacity(const char *disk_name)
{
	int fd;
	char tmp[200]={0};
	char buf[20]={0};
	unsigned long long int cap;
	sprintf(tmp,"/sys/block/%s/size",disk_name);
	fd=open(tmp, O_RDONLY);
	read(fd,buf,20);
	printf("read buf string:%s\n",buf);
	sscanf(buf, "%lld", &cap);
	printf("the cap is %d \n",cap);
	return cap*512/1000000000;	
}
