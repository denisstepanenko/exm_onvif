/*
		处理录像文件索引的相关函数  --wsy Dec 2007
*/

#ifndef FILEINDEX_H
#define FILEINDEX_H


#include "sys/stat.h"
#include "unistd.h"
#include <sys/types.h>
#include <fcntl.h>
#include "stdio.h"
#ifndef IN
#define IN
#define	OUT
#define IO
#endif

#define 	FILEINDEX_VERSION		("0.01")
//ver:0.01 2010-06-17	zw		修改open_db时在使用cp前先判断目标文件


struct query_index_process_struct{
FILE 	*index_fp;
time_t	start;
time_t	stop;
int 	ch;
int 	trig_flag;

};




int fileindex_init_filelock();


//初始化并创建表，返回值有意义
int fileindex_init_db(IN char * partition);


/*************************************************************************
 * 	函数名:	fileindex_add_to_partition()
 *	功能:	把一条文件名加入其所在的分区的索引文件的末尾
 *	输入:	mountpath,所在分区，形如/hqdata/hda1
 *			filename,录像文件名
 *	输出:	
 * 	返回值:	成功返回0,否则返回负值
 *************************************************************************/
int fileindex_add_to_partition(IN char* mountpath, IN char *filename);

/*************************************************************************
 * 	函数名:	fileindex_add_to_partition_upidx()
 *	功能:	把一条文件名加入其所在的分区的索引文件的末尾
 *	输入:	mountpath,所在分区，形如/hqdata/hda1
 *			filename,录像文件名
 *	输出:	
 * 	返回值:	成功返回0,否则返回负值
 *************************************************************************/
int fileindex_add_to_partition_upidx(IN char* mountpath, IN char *filename);



/*************************************************************************
 * 	函数名:	fileindex_del_oldest()
 *	功能:	从分区索引中删去最老的未加锁记录并删除相应文件
 *	输入:	mountpath,分区名称，形如/hqdata/hda1
 *			no,需要删除的文件数目
 *	输出:	
 * 	返回值:	成功返回0,否则返回负值
 *************************************************************************/
int fileindex_del_oldest(IN char* mountpath, int no);


/*************************************************************************
 * 	函数名:	fileindex_rename_in_partition()
 *	功能:	将分区索引文件的指定文件名改成给定的文件名
 *	输入:	mountpath,分区名称
 *			oldname,旧名称
 			newname,新名称
 *	输出:	
 * 	返回值:	成功返回0,否则返回负值
 *************************************************************************/
int fileindex_rename_in_partition(IN char*mountpath, IN char *oldname, IN char* newname);

int fileindex_lock_by_time(IN char* mountpath, IN int flag,IN int  starttime, IN int stoptime, IN int  trig, IN int ch);



/*************************************************************************
 * 	函数名:	fileindex_create_index()
 *	功能:	为整个分区下的录像文件创建索引
 *	输入:	path，分区挂载名称,如"/hqdata/hda2"
 *			forced: 0表示当前没有索引时才创建,1表示无论如何都重新创建，1
 *	输出:	
 * 	返回值:	成功返回0,否则返回负值
 *************************************************************************/
int fileindex_create_index(IN  char *path, IN int forced);




/*************************************************************************
 * 	函数名:	fileindex_get_oldest_file_time()
 *	功能:	获取分区中最老的可删除文件的创建时间
 *	输入:	mountpath，分区挂载名称,如"/hqdata/hda2"
 *	输出:	
 * 	返回值:	成功返回创建时间,否则返回负值
 *************************************************************************/
int fileindex_get_oldest_file_time(char *mountpath);




int fileindex_query_index(char *mountpath, struct query_index_process_struct *qindex);


int fileindex_convert_ing(char *mountpath);

int InitAllDabase();
int CloseAllDabase();
#endif
