/*
 * Blocks.cpp
 *
 *  Created on: 2013-10-24
 *      Author: yangkun
 */

#include "Blocks.h"
#include "SecBlocks.h"
#include "DayBlocks.h"
#include "YearBlocks.h"

namespace gtsda
{
pthread_mutex_t Blocks::mutex_wr = PTHREAD_MUTEX_INITIALIZER;
Blocks::Blocks(unsigned int uBufSize, long long llSeek,blocktype bt)
		:bType(bt),uSize(uBufSize),llSeek(llSeek)
{
	uNumOfBlocks = get_block_num(uBufSize);
	cBuff = new char[uNumOfBlocks*BLOCKSIZE];
	memset(cBuff, 0, uNumOfBlocks*BLOCKSIZE);
}
Blocks::Blocks(char *buf,unsigned int uSizeOfBuf,unsigned int uBufSize,long long llSeek,blocktype bt)
		:bType(bt),uSize(uBufSize),llSeek(llSeek)
{
	uNumOfBlocks = get_block_num(uBufSize);
	cBuff = new char[uNumOfBlocks*BLOCKSIZE];
	memset(cBuff, 0, uNumOfBlocks*BLOCKSIZE);
	memcpy(this->cBuff, buf, (uSize<uSizeOfBuf?uSize:uSizeOfBuf));
	//cout << cBuff << endl;
}
Blocks::Blocks(long long llSeek,unsigned int uBufSize)
:uSize(uBufSize),uNumOfBlocks(get_block_num(uBufSize)),llSeek(llSeek)
{
	uNumOfBlocks = get_block_num(uBufSize);
	cBuff = new char[uNumOfBlocks*BLOCKSIZE];
	memset(cBuff, 0, uNumOfBlocks*BLOCKSIZE);
	if( read() <0 )
	{
		cout << "read error!!!"<< endl;
		throw HD_ERR_READ;
	}

}
Blocks::~Blocks()
{
	if(cBuff!=NULL)
		delete[] cBuff;
}

Blocks & Blocks::operator=(const Blocks &bs)
{
	if(this == &bs)
		return *this;
	delete []cBuff;
	uSize  = bs.uSize;			//缓冲区总大小
	uNumOfBlocks = bs.uNumOfBlocks;	//buf所占的块。
	llSeek = bs.llSeek;
	cBuff = new char [uSize];
	memcpy(cBuff, bs.cBuff, uSize);
	return *this;
}

ostream & operator <<(ostream &os,const Blocks &bk)
{
	os <<  \
	" seek:" << bk.llSeek<< \
	" block size:" << bk.uSize \
	<< " num of Blocks:" << bk.uNumOfBlocks \
	<< " type:" << bk.bType;
	return os;
};

blocktype Blocks::judge_type(const char *buf)
{
	if(NULL == buf)
	{
		ttt();
		cout << "buf is null" << endl;
		return noblock;
	}
	if(0== memcmp(buf, SecBlocks::framehead, 8))//秒块
	{
		return sec;
	}else if(0== memcmp(buf, DayBlocks::day_head, 8))//天块
	{
		return day;
	}else if(0== memcmp(buf, YearBlocks::year_head, 8))//年块
	{
		return year;
	}

	return noblock;
}
int Blocks::read()
{
	int ret;
	//cout << "read  seek:" << llSeek <<"\tblocks:"<<uNumOfBlocks<<endl;
	pthread_mutex_lock(&mutex_wr);
	ret =  hd_read(llSeek, cBuff, uNumOfBlocks);
	pthread_mutex_unlock(&mutex_wr);
	return ret;
}
int Blocks::write()
{
	int ret;
	//cout << "write seek:" << llSeek <<"\tblocks:"<<uNumOfBlocks<<endl;;
	pthread_mutex_lock(&mutex_wr);
	ret =   hd_write(llSeek, cBuff, uNumOfBlocks);
	pthread_mutex_unlock(&mutex_wr);
	return ret;
}

} /* namespace gtsda */


