/*
		为支持多硬盘多分区的库,包括索引文件操作等,包装diskinfo/devinfo库
							--wsy Nov-Dec 2007
*/

#ifndef MPDISK_H
#define MPDISK_H

#include "sys/stat.h"
#include "unistd.h"
#include <sys/types.h>
#include <fcntl.h>
#include "sqlite3.h"

#ifndef IN
#define IN
#define	OUT
#define IO
#endif


#define MAX_DISKNO		4
#define PART_PER_DISK	4

struct partition_info_struct{
char	dev_node[20]; //形如/dev/hda4
char	part_name[20]; //形如/hqdata/hda4
sqlite3 * db;			//相应的数据库指针

};

/****************************************************************************
*	函数名称: mpdisk_get_emptiest_partition
*	函数功能: 获取最空闲的磁盘分区挂载点及其容量
*	输入:
*	输出： partition name: 最空闲的分区挂载点
*	返回值: 返回目前最空闲的分区的剩余空间数(MB),负值表示错误
*	修改日志：
****************************************************************************/
long mpdisk_get_emptiest_partition(OUT char* partition_name);

/****************************************************************************
*	函数名称: mpdisk_get_emptiest_partition
*	函数功能: 获取最空闲的磁盘分区挂载点及其容量
*	输入:
*	输出： partition name: 最空闲的分区挂载点
*	返回值: 返回目前最空闲的分区的剩余空间数(MB),负值表示错误
*	修改日志：
****************************************************************************/
long mpdisk_get_emptiest_partition_str(OUT char* partition_name,int len);




/****************************************************************************************
*函数名称: mpdisk_check_disknode
*函数功能: 根据devinfo库，检查系统应有的硬盘节点是否都有，如果没有，则记录日志
*输入:无
*输出：无 
*返回值:  无
*修改日志：
***************************************************************************************/
void mpdisk_check_disknode(void);

/**************************************************************************
*	函数名称: mpdisk_process_all_partitions()
*	函数功能: 对于所有已挂载的磁盘分区，用fn进行处理
*	输入:		fn,用于处理所有分区的函数指针
				(
					fn的参数包括:	
					输入:	devname,形如"/dev/hda3"
						 	mountpath,形如"/hqdata/hda3"	 
					输入输出: void类型指针fn_arg,用于传递自定义的信息，不用的话也可以为空
				)
*	输入输出： 	void类型指针arg,会被直接传给fn_arg，不用的话可以为空	
*	返回值: 	所有的fn返回值之和
*	修改日志：
*	附注:		如果还觉得不好理解怎么使用，可以参考mpdisk.c里的相关函数,
				例如mpdisk_creat_partitions()等的实现
*************************************************************************/
int  mpdisk_process_all_partitions(IN int (*fn)(IN char *devname, IN char* mountpath, IO void *fn_arg),IO void *arg );


/*************************************************************************
 * 	函数名:	mpdisk_get_sys_disktotal()
 *	功能:	计算当前系统挂载了的所有分区的总容量，单位M
 *	输入:	无
 *	输出:	无
 * 	返回值:	总容量，单位M
 *************************************************************************/
int mpdisk_get_sys_disktotal(void);



/*************************************************************************
 * 	函数名:	mpdisk_creat_partitionse()
 *	功能:	为每个当前有的硬盘分区节点创建挂载点
 *	输入:	无
 *	输出:	无
 * 	返回值:	0表示成功，负值表示失败
 *************************************************************************/
int mpdisk_creat_partitions(void);


/*************************************************************************
 * 	函数名:	mpdisk_check_disk_status()
 *	功能:	检查每个当前有的硬盘分区节点是否有ls无内容但又占用空间的情况
 *	输入:	fn,需要执行的回调函数
 *	输出:	无
 * 	返回值:	0表示成功，负值表示失败
 *************************************************************************/
int mpdisk_check_disk_status(IN int (*fn)(IN char *devname, IN char* mountpath, IO void *fn_arg));

#endif

