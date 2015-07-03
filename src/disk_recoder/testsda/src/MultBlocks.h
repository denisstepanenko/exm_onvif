/*
 * MultBlocks.h
 *
 *  Created on: 2013-10-25
 *      Author: yangkun
 */

#ifndef MULTBLOCKS_H_
#define MULTBLOCKS_H_

#include "Blocks.h"

namespace gtsda
{
typedef enum
{
	InTime=1,
	InSeek=2
}InType;
/*
 * 年块天块中都有很多的块。这些块处理方法类似
 */
class MultBlocks: public gtsda::Blocks
{
public:
	MultBlocks(char *buf,unsigned int uSizeOfBuf,unsigned int iBufSize,long long llSeek,blocktype bt);
	MultBlocks(unsigned int uBufSize,long long llSeek,blocktype bt);
	//插入，读取块数据
	virtual int add(const struct  seek_block &sb)=0;
	virtual int del(const struct  seek_block &sb)=0;
	virtual int Get   (struct  seek_block &sb,get_type get)=0 ;
	virtual int GetHead(struct  seek_block &sb)=0;
	virtual int GetTail(struct  seek_block &sb)=0;
	//判断时间或seek是不是在这个块中
	virtual int In(struct  seek_block &sb, InType iType=InTime)=0;
	virtual inline long long TimeIn(int iTime)=0;
	virtual inline int       SeekIn(long long llSeek)=0;
	virtual ~MultBlocks();
private:
	unsigned int uHeadSize;	//数据头占的大小
	char *data;				//数据
	unsigned int uDataSize;	//数据的大小

};

} /* namespace gtsda */
#endif /* MULTBLOCKS_H_ */
