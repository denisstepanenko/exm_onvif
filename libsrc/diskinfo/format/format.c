#include <file_def.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

/*
*****************************************************
*函数名称: format_disk
*函数功能: 格式化磁盘 按照磁盘的容量选择不同的格式化参数
*输入：char* disk_name 磁盘名称
*输出：
返回值:0 成功，1表示没有磁盘，2表示格式化失败
*修改日志：
*****************************************************
*/ 
int format_disk(char* disk_name)
{
	int ret;
	long int cap;
	cap = get_sys_disk_capacity(disk_name);
	if (cap<=0)
	{
		printf("找不到%s IDE 设备\n", disk_name);
		return 1;
	}
	printf("找到  %s IDE设备\n", disk_name);
		
	if(cap<200)
	{
		printf("%s capacity = %ld less than 200M \n", disk_name, cap);
		printf("没有检测到可用的CF卡!!\n");
		return 1;
	}
	else if((cap>=200)&&(cap<=2000))
	{
		printf("检测到可用的CF卡\n");
		printf("%s disk capacity = %ldM\n", disk_name, cap);
		ret = system("mke2fs /dev/hda1 -b 4096 -j -L hqdata -m 1 -i 65536\n");	//再格式化
		printf("ret = %d \n",ret);
		if(ret!=0)
		{
			return 2;
		}
	}
	else		// if(cap[i]>2000)
	{
		printf("检测到可用的硬盘\n");
		printf("%s disk capacity = %ldG\n", disk_name, cap/1000);
		ret = system("mke2fs /dev/hda1 -b 4096 -j -L hqdata -m 1 -i 524288\n");	//再格式化
		if(ret!=0)
		{
			return 2;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int ret=0;
	char disk_name[50];
	if(argc<2)
	{
		printf("lack parameter\n");
		printf("format <disk name>\n");
		exit(1);
	}
#if 0
	system("killall -9 watch_proc \n");
	system("killall -9 hdmodule\n");
	system("killall -9 diskman\n");
	system("killall -9 vsftpd\n");
	system("umount /hqdata\n");
#endif
	memset(disk_name,0,sizeof(disk_name));
	memcpy(disk_name, argv[1], strlen(argv[1]));
	ret = format_disk(disk_name);
	exit(ret);
}

