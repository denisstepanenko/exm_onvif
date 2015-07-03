/*
		为支持多硬盘多分区的库,调用diskinfo和devinfo库
							--wsy Nov-Dec 2007
*/


#include "mpdisk.h"
#include "stdio.h"
#include "unistd.h"
#include "errno.h"
#include "file_def.h"
#include "hdutil.h"
#include "dirent.h"


//用于传递get_disk_free_fn所需要的参数
struct diskfree_struct 
{
	char * freest_partition;//当前最空的分区名，形如"/hqdata/hda2"
	int  freespace_max; //当前最空的分区的空间数,M为单位
};


//获取指定分区节点的挂载点名称,
//输入:devname,形如"/dev/sda2"; 输出, mountpath,形如"/hqdata/sda2"
//返回值:	挂载点名称的字符串指针("/hqdata/sda2"),失败返回null
char * partitionname2mountpath(IN char * devname, OUT char* mountpath)
{
    char *lp;
    
    if((devname == NULL) ||(mountpath == NULL))
        return NULL;

    lp=strstr(devname,"/sd");
    if(lp!=NULL)
    {
        sprintf(mountpath,"%s%s",HDMOUNT_PATH,lp);
        return mountpath;
    }
    else
        return NULL;
}



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
int  mpdisk_process_all_partitions(IN int (*fn)(IN char *devname, IN char* mountpath, IO void *fn_arg),IO void *arg )
{
    int i;
    int j;
    char partitionname[100];
    char mount_path[100];
    int result = 0;

    for(i=0;i<get_sys_disk_num();i++)
    {   
        for(j=1;j<=get_sys_partition_num(get_sys_disk_name(i));j++)
        {
            memset(partitionname,0,sizeof(partitionname));
            memset(mount_path,0,sizeof(mount_path));
            get_sys_disk_partition_name(i,j, partitionname);
            partitionname2mountpath(partitionname,mount_path);
            result += (*fn)(partitionname, mount_path,arg);
        }
    }
    return result;
}

//处理单个分区。被mpdisk_get_emptiest_partition调用
//如果这分区的空闲磁盘比disk_free更大(单位:MB)
//就把磁盘的相关信息通过disk_free和freest_partition输出
//返回值为0表示成功，为负表示失败
int get_disk_free_fn(IN const char *devname, IN const char* mountpath, IO void * arg)
{
	long partition_free = 0;
	struct diskfree_struct *diskfree;
	
	if((arg== NULL)||(mountpath == NULL))
		return -EINVAL;
	diskfree = (struct diskfree_struct *)arg;
	partition_free = get_disk_free(mountpath);
	if((partition_free > diskfree->freespace_max)&&(get_disk_total(mountpath)>200)) //当前处理的分区更空闲
	{
		diskfree->freespace_max = partition_free;
		sprintf(diskfree->freest_partition, mountpath);
	}
	return 0;
}

/*************************************************************************************
*	函数名称: mpdisk_get_emptiest_partition
*	函数功能: 获取最空闲的磁盘分区挂载点及其容量
*	输入:
*	输出： partition name: 最空闲的分区挂载点
*	返回值: 返回目前最空闲的分区的剩余空间数(MB),负值表示错误
*	修改日志：
************************************************************************************/
long mpdisk_get_emptiest_partition(OUT char* partition_name)
{
	int max_diskfree = 0; //最空闲的分区剩余空间数
	int i,j;
	struct diskfree_struct diskfree;
	
	if(partition_name == NULL)
		return -EINVAL;
	
		
	diskfree.freest_partition 	= partition_name;
	diskfree.freespace_max 		= max_diskfree;
	mpdisk_process_all_partitions(&get_disk_free_fn, &diskfree);
	return diskfree.freespace_max;
}



/****************************************************************************************
*函数名称: mpdisk_check_disknode
*函数功能: 根据devinfo库，检查系统应有的硬盘节点是否都有，如果没有，则记录日志
*输入:无
*输出：无 
*返回值:  无
*修改日志：
***************************************************************************************/
void mpdisk_check_disknode(void)
{
	int i;
	for(i=0;i<get_disk_no();i++)
	{
		if(check_file(get_sys_disk_devname(i))==0)//没有硬盘节点
		{
			gtlogerr("无%s硬盘设备节点!!!!!!!!!!\n",get_sys_disk_devname(i));
		}	
	}
}

//处理单个分区. 供mpdisk_get_sys_disktotal调用
//功能:将目前分区的容量加到disktotal上，单位为M
//返回值，0成功，负值错误码
int get_disk_total_fn(IN const char * devname, IN const char * mountpath, IO void * arg)
{
	int *disktotal;
	if((mountpath == NULL)||(arg == NULL))
		return -EINVAL;
	disktotal = (int *)arg;	
	*disktotal += get_disk_total(mountpath);
	return 0;
}



/*************************************************************************
 * 	函数名:	mpdisk_get_sys_disktotal()
 *	功能:	计算当前系统挂载了的所有分区的总容量，单位M
 *	输入:	无
 *	输出:	无
 * 	返回值:	总容量，单位M
 *************************************************************************/
int mpdisk_get_sys_disktotal(void)
{
	int i,j;
	int disktotal=0;
	
	mpdisk_process_all_partitions(&get_disk_total_fn,&disktotal);
	return disktotal;
}


/*************************************************************************
 * 	函数名:	mpdisk_check_disk_status()
 *	功能:	检查每个当前有的硬盘分区节点是否有ls无内容但又占用空间的情况
 *	输入:	fn,需要执行的回调函数
 *	输出:	无
 * 	返回值:	0表示成功，负值表示失败
 *************************************************************************/
int mpdisk_check_disk_status(IN int (*fn)(IN char *devname, IN char* mountpath, IO void *fn_arg))
{
	if(fn == NULL)
		return -EINVAL;	
	return mpdisk_process_all_partitions(fn,NULL);
}



//被mpdisk_creat_partitions调用，用于为每个当前有的硬盘分区节点创建挂载点
//返回0表示成功，负值表示失败
int creat_partitions_fn(IN  char * devname, IN  char * mountpath, IO void *arg)
{
	if((devname ==NULL)||(mountpath == NULL))
		return -EINVAL;
	mkdir(mountpath,0755);
	return 0;
}

/*************************************************************************
 * 	函数名:	mpdisk_creat_partitionse()
 *	功能:	为每个当前有的硬盘分区节点创建挂载点
 *	输入:	无
 *	输出:	无
 * 	返回值:	0表示成功，负值表示失败
 *************************************************************************/
int mpdisk_creat_partitions(void)
{
	return mpdisk_process_all_partitions(&creat_partitions_fn,NULL);
}


