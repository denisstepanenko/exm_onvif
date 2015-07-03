/*
 * hdwr.cpp
 *
 *  Created on: 2013-10-28
 *      Author: yangkun
 */

#include "hdwr.h"
#include "Blocks.h"
#include "cstring"
#include <stdio.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef	MEMIN
#include "Test.h"
#endif
namespace gtsda
{
#ifdef	MEMIN
int hd_read (long long llSeek, char *cBuff, unsigned int uNumOfBlocks)
{

	if(HdSize<llSeek || HdSize < llSeek + uNumOfBlocks)
		return HD_ERR_READ;
	memcpy(cBuff, TestInMem::get_hd()+(llSeek*BLOCKSIZE), uNumOfBlocks*BLOCKSIZE);
	cout << "read llSeek: uNumOfBlocks:  "<< llSeek << "  " << uNumOfBlocks <<  endl;

	return 0;
}
int hd_write(long long llSeek, char *cBuff, unsigned int uNumOfBlocks)
{
	if(HdSize<llSeek || HdSize < llSeek + uNumOfBlocks)
	{
		ttt();
		cout <<"error!!!!!!"<< HD_ERR_WRITE << endl;
		return HD_ERR_WRITE;
	}
	memcpy(TestInMem::get_hd()+(llSeek*BLOCKSIZE),cBuff,  uNumOfBlocks*BLOCKSIZE);
	cout << "write llSeek: uNumOfBlocks:  "<< llSeek << "  " << uNumOfBlocks <<  endl;


	return 0;
}
#endif

/*
 * 功能：获取硬盘容量：
 * 返回：0出错
 * 		>0硬盘block的块数
 */
long long hd_getblocks()
{
    unsigned long long int cap;
#if 1
    int fd;
    char tmp[200]={0};
    char buf[20]={0};
    sprintf(tmp,"/sys/block/%s/size","sda");
    fd=open(tmp, O_RDONLY);
    read(fd,buf,20);
    sscanf(buf, "%lld", &cap);
#else //测试硬盘满逻辑时使用
    cap =  100000 ;
#endif
cout << "the cap is: " << cap << endl;
	return cap;
	return 0;
}

} /* namespace gtsda */
