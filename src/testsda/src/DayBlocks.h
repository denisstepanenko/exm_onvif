/*
 * DayBlocks.h
 *
 *  Created on: 2013-10-29
 *      Author: yangkun
 */

#ifndef DAYBLOCKS_H_
#define DAYBLOCKS_H_

#include "MultBlocks.h"

namespace gtsda
{

class DayBlocks: public gtsda::MultBlocks
{
public:
	static const unsigned char day_head[8];
	DayBlocks(long long llSeek=first_block+get_block_num(sizeof(struct year_block)) ,\
			bool bIsRead=true/*构造时是否硬盘读取内容，默认读*/,bool bIsWrite=true/*析构时是否写入*/); //day_bac_block
	int read();
	//插入，读取块数据
	virtual int add(const struct  seek_block &sb);
	virtual int del(const struct  seek_block &sb);
	virtual int Get   (struct  seek_block &sb,get_type get) ;
	virtual int GetHead(struct  seek_block &sb);
	virtual int GetTail(struct  seek_block &sb);
	//判断时间或seek是不是在这个块中
	virtual int In(struct  seek_block &sb, InType iType=InTime);
	virtual inline long long TimeIn(int iTime);
	virtual inline int       SeekIn(long long llSeek);
	virtual ~DayBlocks();

	DayBlocks & operator=(const DayBlocks &db);
private:
	bool bIsWrite;
	int iOperateTimes;
	struct day_block *daydata;
	int ReadBlock();
};

} /* namespace gtsda */
#endif /* DAYBLOCKS_H_ */
