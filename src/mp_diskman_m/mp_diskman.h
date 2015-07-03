

#ifndef MP_DISKMAN_H
#define MP_DISKMAN_H


#define  VERSION "0.04"

/*
0.04   直接使用硬盘上的数据库，只在进程启动时创建数据库
0.03   在搬数据库文件出错时重启
0.02   2013-09-06 查找最早的文件时，只返回最早的一个；
         删除的流程修改了，另改了几个小bug
0.01   2013-07-19 创建
*/

#if EMBEDED==0
	#define FOR_PC_MUTI_TEST		//支持在同一台pc机上启动多个程序
#endif
#include <file_def.h>

#endif
