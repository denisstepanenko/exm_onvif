/******************************************************
 * 功能：管理硬盘上的块
 *
 *****************************************************/
#ifndef __BLOCKS_H__
#define __BLOCKS_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>


/*测试程序逻辑是不是正常*/
//#define DEBUG_LOG
/*测试*/
#define DEBUG

/*硬盘从第一块开始写*/
#define HD_START 1
/*----------------------------------------------*
 * const defination                             *
 *----------------------------------------------*/
typedef enum {
    FALSE    = 0,
    TRUE     = 1,
} BOOL;
#include "media_api.h"
#define MEDIA_VIDEO		0x01		//视频数据
#define MEDIA_AUDIO	0x02		//音频数据

struct NCHUNK_HDR {	//avi格式的数据块头标志结构
#define IDX1_VID  		0x63643030	//AVI的视频包标记
#define IDX1_AID  		0x62773130	//AVI的音频报的标记
	unsigned long  chk_id;
	unsigned long  chk_siz;
};
typedef struct{
    ///压缩后的视频帧
    ///使用这个结构时要先分配一个大缓冲区,然后将本结构的指针指向缓冲区

#define MEDIA_VIDEO		0x01		//视频数据
#define MEDIA_AUDIO	0x02		//音频数据

#define FRAMETYPE_I		0x0		// frame flag - I Frame
#define FRAMETYPE_P		0x1		// frame flag - P Frame
#define FRAMETYPE_B		0x2
#define FRAMETYPE_PCM	0x5		// frame flag - Audio Frame

	struct timeval           tv;			   ///<数据产生时的时间戳
	unsigned long	           channel;	          ///<压缩通道
	unsigned short           media;		   ///<media type 音频或视频
	unsigned short           type;		          ///<frame type	I/P/声音...
	long                          len;	                 ///<frame_buf中的有效字节数
	struct NCHUNK_HDR chunk;                ///<数据块头标志，目前使用avi格式
	unsigned char            frame_buf[4];    ///<存放编码后的视频数据的起始地址
}enc_frame_t;


/*硬盘默认块大小512字节  这个值可以说是不能改的*/
#define BLOCKSIZE 512

/*本程序最多能管理800天*/
#define MAXDAY 800
/*年循环队列中头的偏移*/
#define YEAR_OFFSET 8
/*天循环队列中头的偏移*/
#define DATE_OFFSET 8

#ifdef DEBUG_LOG
	/*60*12/512=2*/
	#define YEAR_HEAD_BLOCK_SIZE 20
	#define DATE_HEAD_BLOCK_SIZE 2
/*测试程序逻辑时，就让硬盘有100000*512，也就50M左右那么大，4Mbit码率可以写100S，也就是一天多一点了*/
#define MAXBLOCKS 100000
#else
/*
 * 表示年的数据占20块，表示天的数据点2026块
 * 最大800天 20->800*12/512
 * 2026->24*3600*12/512
 */
	#define YEAR_HEAD_BLOCK_SIZE 20
	#define DATE_HEAD_BLOCK_SIZE 2026
/*160G的硬盘有这么多个块*/
#define MAXBLOCKS 312581808
#endif






extern media_source_t media;
#define BUFFER_SIZE 400*1024 //最大4M，但个帧应该不会超过100K





/******************************************************************
 * 年块
 * *****************************************************************/
struct year_block{ int time; long long seek;}__attribute__ ((packed));



unsigned int get_date_address(unsigned char *buf_queue);
inline int myprint(unsigned  char *p, long size);

/****************************************************************
 * 天块
 *
 ******************************************************************/
struct day_block{ int time; long long seek;}__attribute__ ((packed));
long long get_current_block();

/***********************************************
 * 时间
 **********************************************/
#ifdef DEBUG_LOG
	/*如果测试程序逻辑，可以让天变短点，就一天只有60秒*/
	#define SECOFDAY (1*60)
#else
	#define SECOFDAY (24*3600)
#endif   /*DEBUG_LOG*/



/***************debug***************************************************/
#define DP(fmt...)	\
	do{\
		printf(fmt);\
		printf("function:[%s],line:%d: \n",__FUNCTION__,__LINE__);\
	}while(0)


/***************debug***************************************************/

extern inline void set_seek(long long seek);
#endif
