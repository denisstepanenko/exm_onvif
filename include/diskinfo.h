#ifndef DISK_INFO_H
#define DISK_INFO_H

#define MASTER_DISK	"sda"//yk change dha ->sda
#define SLAVE1_DISK	"sdb"
#define SLAVE2_DISK	"sdc"
#define SLAVE3_DISK	"sdd"

//addby wsy
#define DEVDISK0	"/dev/sda"
#define DEVDISK1	"/dev/sdb"
#define DEVDISK2	"/dev/sdc"
#define DEVDISK3	"/dev/sdd"

//add by zw
#define SD_MASTER	("sda")
#define SD_SLAVE1	("sdb")

//add by lsk
#define SD_DISK0	("/dev/sda")
#define SD_DISK1	("/dev/sdb")

#ifndef IN
#define IN
#define IO
#define OUT
#endif

/*
*****************************************************
*函数名称: get_sys_disk_num
*函数功能: 获取磁盘数量
*输入：
*输出：
返回值: 成功返回磁盘个数
*修改日志：
*****************************************************
*/ 
int get_sys_disk_num(void);
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
long int get_sys_disk_capacity(const char* disk_name);
/*
*****************************************************
*函数名称: get_sys_partition_num
*函数功能: 获取磁盘分区数量
*输入：const char* disk_name 磁盘名称
*输出：
返回值: 成功返回磁盘数量，0表示没有分区，负值表示失败
*修改日志：
*****************************************************
*/ 
int get_sys_partition_num(const char*disk_name);




/*****************************************************
*函数名称: get_sys_disk_name
*函数功能: 获取磁盘名称
*输入：diskno:磁盘编号(从0开始)
*输出：
返回值: 成功返回磁盘名称字符串,形如"hda"，失败返回null
*****************************************************
*/ 
char *get_sys_disk_name(int diskno);


/*
*****************************************************
*函数名称: get_sys_partition_capacity
*函数功能: 获取磁盘分区容量
*输入：const char* disk_name 磁盘名称
*	    int partition_index 磁盘分区索引号
*输出：
*返回值: 成功返回磁盘容量(单位M)，0表示没有分区，负值表示失败
*修改日志：
*****************************************************
*/ 
long int get_sys_partition_capacity(const char*disk_name, int partition_index);




/*add-by-wsy 2007-11-20
*****************************************************
*函数名称: get_sys_disk_devname
*函数功能: 获取磁盘节点名称
*输入：diskno:磁盘编号(从0开始)
*输出：
返回值: 成功返回磁盘节点名称字符串，形如"/dev/hda",失败返回null
*****************************************************
*/ 
char *get_sys_disk_devname(int diskno);


/*****************************************************
*函数名称: get_sys_disk_partition_num
*函数功能: 获取磁盘分区数量
*输入：const char* disk_name 磁盘名称,形如"/dev/hda"
*输出：
返回值: 成功返回磁盘分区个数，0 表示没有分区，负值表示失败
*修改日志：
*****************************************************/
int get_sys_disk_partition_num(char *disk_devname);


/*************************************************************************
 * 	函数名:	get_sys_disk_partition_name()
 *	功能:	获取指定序号的分区节点名
 *	输入:	diskno,磁盘序号，从0-3
 *			part_no,分区序号，从1开始
 *	输出:	partitionname, 填充好分区节点名称的字符串指针
 * 	返回值:	形如 "/dev/hda1"的分区节点名称 ,错误返回NULL
 *************************************************************************/
char * get_sys_disk_partition_name(IN int diskno, IN int part_no,OUT char * partitionname);

#endif
