#ifndef MP_HDMODULE_H
#define MP_HDMODULE_H

#include "gtlog.h"

#define  VERSION		"0.09"
/*
0.09   1. 直接使用硬盘上的数据库，只在进程启动时创建数据库
         2. 增加音频
0.08 从硬盘往内存搬数据库文件出错就重启，在发现四个盘的空间都不足时停止录像线程
0.07 录像文件单独使用/conf/diskinfo.ini
0.06 启动是录像盘加日志
0.05 在空间小于250M时调盘
0.04 增加了读写数据缓冲区的接口，用在本地录像，这些接口使用以前的获取音视频数据的方法。
0.03 多分区的情况下,配置成触发录像后不启动录像线程，3024上也有这问题，修改；
        增加了保存文件出错后的检查分区的功能。
0.02 减少内存
0.01 2013-07-08 创建

*/

//测试定义

#define SHOW_WORK_INFO

#if EMBEDED==0
	#define FOR_PC_MUTI_TEST		//支持在同一台pc机上启动多个程序
#endif

//#define     USE_FFMPEG_LIB                  ///使用ffmpeg库做音频编码FIXME:临时演示用 ,如果要正式使用，这部分内容需要重新设计





//#define	D1_MAX_FRAME_SIZE	0x20000 //I桢数据最大size
#ifndef FOR_PC_MUTI_TEST
	#define  HDENC0_KEY_NUMBER 	123 //用于创建消息队列的KEY值
#else
	extern int HDENC0_KEY_NUMBER; 	 //用于创建消息队列的KEY值
#endif
#define HQMODULE_USE
#if EMBEDED
#define PATH_TYPE 0 //开关，为1则在查询结果和.txt文件中显示/hqdata/....     
#else
#define PATH_TYPE 1 //开关，为1则在查询结果和.txt文件中显示/hqdata/....     
#endif

//#define RECORD_PS_FILE	//开关，如果选中，则同时录ps流的mpg文件

#define GET_PIC_SPACE     5 //抓图所需最小磁盘空间，以M为单位


#ifndef FOR_PC_MUTI_TEST
#define POOLSIZE		  15   //抓图缓冲池大小changed by shixin from 60
#else
#define POOLSIZE		  5
#endif
//zw-del 2011-06-15 #define TAKE_PIC_MAX      20    //一次最多允许抓图的张数

#define TAKE_PIC_MAX        (70)

#define MOTION_PRE_REC	10	//移动和触发录像预录秒数
#define MOTION_DLY_REC	10	//移动和触发录像延迟秒数


#include <file_def.h>
#include <mod_com.h>

int get_gtthread_attr(pthread_attr_t *attr);
#endif



