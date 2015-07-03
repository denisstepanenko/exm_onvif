/*
 * Blocks.h
 *
 *  Created on: 2013-10-24
 *      Author: yangkun
 */

#ifndef BLOCKS_H_
#define BLOCKS_H_
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>

#include "gtlog.h"
#include <error.h>

#include "hdwr.h"
#include "err.h"

/****************************************************************
 * 无版本0.01:实现比较稳定的单路的视频读写
 * 0.02:
 * 0.03:实现1路音视频的写入
 * 0.04:实现稳定的音视频的读写
 * 0.05:多路音视频写入与读出
 ****************************************************************/
#define Version "0.05"
#define DEBUG
#define CRC

//没写入逻辑时，暂时不考虑读出逻辑
//#define NOREAD

using std::cout;
using std::endl;
using std::ostream;
using std::istream;
namespace gtsda
{
#define MAX_CHANNEL 4
/*硬盘默认块大小512字节  这个值可以说是不能改的*/
#define BLOCKSIZE 512
#define BUFSIZE  400*1024
/*本程序最多能管理800天*/
#define MAXDAY 800
/*年循环队列中头的偏移*/
#define YEAR_OFFSET 8
#define SECOFDAY (24*3600)
#define first_block 1
//每秒写入的数据最大的块；判断硬盘满时或者天块被覆盖时，逻辑比较简单。
//最大的8Mbit/s 4个通道,加上音频和其它误差；估计最多可能产生4.5Mbyte的数据
#define LEAVE_BLOCKS ((4*1024*1024 + 512*1024 + BLOCKSIZE -1)/BLOCKSIZE )

//#define get_block_num(buffsize) (( (buffsize) + BLOCKSIZE -1)/BLOCKSIZE )
typedef enum
{
	noblock		= 0, /*什么块也没有*/
	block		= 1,
	collection	= 2,/*包含多个块的包*/
	year		= 3,
	day			= 4,
	day_bac		= 5,
	sec			= 6,
	audio		= 7,
	video		= 8,
	multmedia  = 9,/*包含多个音视频块*/
	mult_video  = 10,
	mult_audio  = 11,

}blocktype;
typedef enum
{
	get_start = 1,
	get_tail   = 2
}get_type;
struct  seek_block{ int time; long long seek;}__attribute__ ((packed));
struct year_queue{ unsigned int queue_size ,queue_head,queue_tail;};
struct year_block
{
	unsigned char year_head[8];
	struct year_queue year_queue_data;
	struct seek_block seek_block_data[MAXDAY];
}__attribute__ ((packed));
//struct day_queue { int index, size;};
struct day_block
{
	unsigned char day_head[8];
//	struct day_queue day_queue_data;
	struct seek_block seek_block_data[SECOFDAY];
}__attribute__ ((packed));

struct hd_frame
{
	char data_head[8];			/*帧数据头的标志位 0x5345434fa55a5aa5*/
	short frontIframe;			/*前一个I帧相对于本帧的偏移地址   如果前一帧在硬盘的最后面，这个值可能是负值*/
	short is_I;		/*本帧是不是I帧*/
	unsigned int size;			/*本帧视频数据块的大小*/
	unsigned int crc;			//冗余位
}__attribute__ ((packed));

//视频块,不块对齐
//只有四种媒体块：video,audio块；vidoe,audio集合块；
struct media_block
{
	char data_head[8];
	unsigned int size;	//数据长度
	int parm;
	//无视频属性，以后要添加，现在只支持h264,d1,25
	//......后面是视频块
}__attribute__ ((packed));


typedef struct{	//视频格式信息结构
	unsigned char format;		//编码格式format
	unsigned char  type;		//frame type	I/P/B...
	unsigned char ratio;  //分辨率
}video_format_t;
typedef struct{	//音频格式信息结构
	unsigned char  a_wformat;	//声音格式
	unsigned char  a_sampling;	//声音采样率
	unsigned char  a_channel;	//声音通道
	unsigned char  a_nr_frame;	//一包声音里面有几块数据
	unsigned char  a_bitrate;		//声音码流
	unsigned char  a_bits;		//音频采样位数
	unsigned char  a_frate;		//音频数据的帧率(没秒钟有几包音频数据)
}audio_format_t;
struct one_channel_media
{
	long long    video_seek;  //视频1在硬盘上偏移
	unsigned int video_buf_size;
	video_format_t vft;
	long long    audio_seek;  //音频1在硬盘上偏移
	unsigned int audio_buf_size;
	audio_format_t aft;
}__attribute__ ((packed));

//音视频块集合
struct mult_media_block
{
	char data_head[8];
	struct one_channel_media all_media_head[MAX_CHANNEL];
	long long next_mult_media_block;//下一个mult_media_block的地址
}__attribute__ ((packed));




class Blocks
{
public:
	Blocks(unsigned int uBufSize,long long llSeek=0,blocktype bt=block);
	Blocks(char *buf,unsigned int uSizeOfBuf,unsigned int uBufSize,long long llSeek=0,blocktype bt=block);
	Blocks(long long llSeek,unsigned int uBufSize);//read from disk，and get type
//浅copy
	Blocks(Blocks *bsBlocks,unsigned int uBufSize);
	unsigned int GetSize(){return uSize;};
	long long GetSeek(){return llSeek;};
	void SetSeek(long long llSeek)
	{
		if(llSeek<0)
		{
			cout << "set seek error" << endl;
			throw -1;
		}
		else
			this->llSeek=llSeek;

	};
	long long GetBlocks(){return uNumOfBlocks;};
	long long GetNext(){return llSeek + uNumOfBlocks;};//下一个写入的位置，就是本块的下一块
	virtual ~Blocks();
	blocktype GetType()const{return bType;};
	const void * const GetBuf()const  {return cBuff;};
	void SetBuf(char *buf,unsigned int uSizeOfBuf)
	{
		memcpy(this->cBuff, buf, (uSize<uSizeOfBuf?uSize:uSizeOfBuf));
	};
	static unsigned int get_block_num(unsigned int  buffsize){ \
		return (( (buffsize) + BLOCKSIZE -1)/BLOCKSIZE ); }
	static blocktype judge_type(const char *buf);
	friend ostream & operator <<(ostream &os,const Blocks &bk);

	int read();
	int write();
	Blocks & operator=(const Blocks &bs);
private:
	char *cBuff;
	blocktype bType;
	unsigned int uSize;			//缓冲区总大小
	unsigned int uNumOfBlocks;	//buf所占的块。
	long long llSeek;			//第多少块
	static pthread_mutex_t mutex_wr;
};

} /* namespace gtsda */
#endif /* BLOCKS_H_ */
