/*
 * MultBlocks.cpp
 *
 *  Created on: 2013-10-25
 *      Author: yangkun
 */

#include "MultBlocks.h"
namespace gtsda
{

MultBlocks::MultBlocks(char *buf,unsigned int uSizeOfBuf,unsigned int uBufSize,long long llSeek,blocktype bt)
	:Blocks(buf, uSizeOfBuf, uBufSize, llSeek,bt)
{
	// TODO Auto-generated constructor stub

}

MultBlocks::MultBlocks(unsigned int uBufSize,long long llSeek,blocktype bt)
	:Blocks(uBufSize, llSeek, bt)
{

}

MultBlocks::~MultBlocks()
{
	// TODO Auto-generated destructor stub
}

} /* namespace gtsda */


