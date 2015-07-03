#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
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

//#define FORMAT
#define SD_TEST
int main(void)
{
	long int i=0;
	init_devinfo();
	i=get_sys_disk_num();
	if(i>0)
		printf("disk num = %ld \n", i);
#ifdef SD_TEST
	i= get_sys_disk_capacity(SD_MASTER);
#else
	i= get_sys_disk_capacity(MASTER_DISK);
#endif
	if((i>0)&&(i<2000))
		printf("disk capacity = %ldM \n", i);
	if(i>=2000)
		printf("disk capacity = %ldG \n", i/1000);
#ifdef SD_TEST
	i= get_sys_partition_num(SD_MASTER);
#else
	i= get_sys_partition_num(MASTER_DISK);
#endif
	if(i>0)
		printf("partition num = %ld \n", i);
#ifdef SD_TEST
	i= get_sys_partition_capacity(SD_MASTER, 1);
#else
	i= get_sys_partition_capacity(MASTER_DISK, 1);
#endif
	if(i>0)
		printf("partition 1 capacity  = %ldM \n", i);
	exit(0);

}

