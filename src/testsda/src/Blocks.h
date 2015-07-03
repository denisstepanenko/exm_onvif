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

#define CRC

using std::cout;
using std::endl;
using std::ostream;
using std::istream;
namespace gtsda
{



/*硬盘默认块大小512字节  这个值可以说是不能改的*/
#define BLOCKSIZE 512
#define BUFSIZE  400*1024
/*本程序最多能管理800天*/
#define MAXDAY 800
/*年循环队列中头的偏移*/
#define YEAR_OFFSET 8
#define SECOFDAY (24*3600)
#define first_block 1
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
class Blocks
{
public:
	Blocks(unsigned int uBufSize,long long llSeek=0,blocktype bt=block);
	Blocks(char *buf,unsigned int uSizeOfBuf,unsigned int uBufSize,long long llSeek=0,blocktype bt=block);
	Blocks(long long llSeek,unsigned int uBufSize);//read from disk，and get type
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
	Blocks(Blocks &bk){};//暂时禁止复制对象！！！！
	char *cBuff;
	blocktype bType;
	unsigned int uSize;			//缓冲区总大小
	unsigned int uNumOfBlocks;	//buf所占的块。
	long long llSeek;			//第多少块
	static pthread_mutex_t mutex_wr;
};

} /* namespace gtsda */
#endif /* BLOCKS_H_ */
