#ifndef FIX_DISK_H
#define FIX_DISK_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iniparser.h>

#define FIXDISK_INTERVAL   4*60*60  //单位为秒

#ifndef IN
#define IN
#define OUT
#define IO
#endif

/*
 * 函数名	:fix_disk()
 * 功能	:如果errno应该修理磁盘，则检查相关分区的修理记录并用相关选项进行修理
 * 输入	:path:形如"/hqdata/hda2/xxx"的字符串，只要前缀是"/hqdata/hda2"就可以
 *	 	 errno:错误码，正值
 * 返回值:无
*/
void fix_disk( char *path,int diskerrno);

/*
 * 函数名	:is_disk_error()
 * 功能	:判断传进来的errno是否是应该修理磁盘的问题
 * 输入	:error:错误码
 * 返回值	:1表示应修理磁盘，0表示不用修理磁盘
*/
int is_disk_error(int error);

/*wsyadd, 用于处理调用ftw_sort出错时，需要修理磁盘的情况*/
int  fix_disk_ftw(const char *dirname, int err);

#endif
