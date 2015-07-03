/*
 * SecBlocks.h
 *
 *  Created on: 2013-10-29
 *      Author: yangkun
 */

#ifndef SECBLOCKS_H_
#define SECBLOCKS_H_

#include "Blocks.h"
#include "DataWR.h"
namespace gtsda
{

class SecBlocks: public gtsda::Blocks
{
public:
	static const unsigned  char framehead[8];//={0x53,0x45,0x43,0x4f,0xa5,0x5a,0x5a,0xa5};
	SecBlocks(unsigned int uBufSize,long long llSeek );
	SecBlocks(long long llSeek);
	virtual ~SecBlocks();
	//添加一次
	int addonetime(DataRead &bs);
#ifdef CRC
	static unsigned  crc32_tab[];
	unsigned int static crc32( const void *buf, size_t size)
	{
		unsigned int crc=0;
		const unsigned int *p;
		p = (unsigned int *)buf;
		crc = crc ^ ~0U;
		while (size--)
			crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
		return crc ^ ~0U;
	}
	//0成功，不为0失败
	static int verify_crc(struct hd_frame *b)
	{
		if(NULL == b)return 0;
		unsigned int crc = b->crc ,crc1;
		b->crc = 0 ;
		crc1 = crc32((char *)b,(b->size +sizeof(struct hd_frame))/4);
		//cout << "this crc: " << std::hex << crc1 << std::dec<< endl;
		b->crc=crc;
		if(crc != crc1)
			return crc1;
		else
			return 0;
	}
	//返回crc校验的结果
	unsigned int GetCrc()
	{
		if(sb == NULL)throw 0;
		unsigned int crc = sb->crc,ret;
		sb->crc = 0;
		ret=crc32((char *)sb,(sb->size +sizeof(struct hd_frame))/4);
		sb->crc=crc;
		return ret;
	};
	//写入数据时使用
	void make_crc()
	{
		if( sb == NULL)throw 0;
		sb->crc = 0;
		unsigned int crc=crc32((char *)sb,(sb->size +sizeof(struct hd_frame))/4);
		//cout << "this crc: "<< std::hex << crc << std::dec<< endl;
		sb->crc =crc;
	}
	//读数据时把数据做crc校检，同时与数据结构中的crc做对比；如果一致，返回0，如果不同，返回不同的crc;
	int verify_crc()
	{
		if( sb == NULL)throw 0;
		unsigned int crc = sb->crc ,crc1;
		sb->crc = 0 ;
		crc1 = crc32((char *)sb,(sb->size +sizeof(struct hd_frame))/4);
		if(crc != crc1)
			return crc1;
		else
			return 0;
	}
#endif
private:
	struct hd_frame *sb;
	bool bOneTime;
	bool bIsWrite;
};

} /* namespace gtsda */
#endif /* SECBLOCKS_H_ */
