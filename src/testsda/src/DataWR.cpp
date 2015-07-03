/*
 * DataWR.cpp
 *
 *  Created on: 2013-10-29
 *      Author: yangkun
 */

#include "DataWR.h"
#include "fcntl.h"
#include <stdio.h>
namespace gtsda
{

DataWrite::DataWrite()
:Blocks(BUFSIZE,0,block)
{
	//buf = GetBuf();
	fifo_init();

}

DataWrite::~DataWrite()
{
	/*释放管道*/
		close(fd);
}
/*初始化管道*/
int DataWrite::fifo_init()
{
	mkfifo("test.264", 0777);
	int ret;
	fd = open("test.264", O_WRONLY);
	if (fd < 0)
	{
		perror("openfile error");
		return -1;
	}
	return 0;
}
/*写管道*/
int DataWrite::fifo_write(const char *buf, int size)
{
	int ret = ::write(fd, buf, size);
	if (ret < 0)
	{
		cout << " fifo write error!\n";
		return -1;
	}
	return ret;
}
int DataWrite::writedata()
{
	int ret;
	if( ( ret=fifo_write( ( char *)GetBuf(), buffsize) <0) )
	{
		cout << "fifo_write error!!"<< endl;
		return ret;
	}
	return 0;
}
DataWrite  &DataWrite::operator=(DataRead &dr)
{

	memcpy( (void *)(GetBuf()) , dr.get_the_frame_buffer()->frame_buf , dr.get_frame_buff_size());
	buffsize = dr.get_frame_buff_size();
	return *this;
}
int DataWrite::getdata(DataRead &dr)
{
	memcpy( (void *)(GetBuf()) , dr.get_the_frame_buffer()->frame_buf , dr.get_frame_buff_size());
	buffsize = dr.get_frame_buff_size();
	return 0;
}
int DataWrite::getdata(const char *buf, unsigned int buffsize)
{
	memcpy( (void *)(GetBuf()) , buf, buffsize);
	this->buffsize = buffsize;
	return 0;
}



#ifdef MEMIN
DataRead::DataRead(unsigned int uBufSize,long long llSeek, blocktype bt)
	:Blocks(uBufSize,llSeek,bt)
{
	//把内容格式化成bblock的格式
	unsigned int i;
	bs = (struct bblock *)GetBuf();
	bs->size = GetSize()/4 -2;
	bs->xxd = rand();
	unsigned int *p = (unsigned int *)(bs->p);
	for(i =0; i< bs->size; i++)
	{
		*(p+i) = bs->xxd;
	}
}
#else
DataRead::DataRead(long long llSeek,blocktype bt,unsigned int channel)
	:Blocks(BUFSIZE,llSeek,bt),seq(-1),flag(-1),the_frame_buffer(NULL)
{
	init_pool(channel);
}
/*
DataRead::DataRead(unsigned int channel)
	:Blocks(BUFSIZE,0,block),seq(-1),flag(-1),the_frame_buffer(NULL)
{
	init_pool(channel);
}
*/
#endif
DataRead::~DataRead()
{

}
void DataRead::init_pool(unsigned int channel)
{
	int err;
	char name[]="record";
	//初始化mediaapi
	//初始化数据
	memset(&media ,0, sizeof(media_source_t));
	media.dev_stat= -1; //表示没有连接
	err=connect_media_read(&media ,0x30000+channel, name, /*MSHMPOOL_LOCAL_USR*/1);
	if(err!=0)
	{
		cout<< "1error in connect media read and exit: "<< err << endl;;
		gtlogerr("error in connect media read and exit\n");
		exit(1);
		//throw ret;
	}
	else
	{
		cout << "connect success\n";
		gtloginfo("connect success\n");
	}
}
int DataRead::readpool()
{
	memset((void *)GetBuf(), 0, BUFSIZE);
	seq=-1;flag=-1;
	int ret=read_media_resource(&media,(void *)GetBuf(), BUFSIZE, &seq, &flag);
	if(ret<0)
	{
		cout << "error in read media resource\n" ;
		return ret;
	}
	the_frame_buffer=(enc_frame_t *)GetBuf();
	buffsize = the_frame_buffer->len;
	if(flag==1)
		bIsI=false;
	else
		bIsI=true;
	return 0;
}



#ifdef MEMIN
int publictime;
#else
#include <time.h>
#endif
int gettime(bool add)
{
#ifdef MEMIN
	if(add)
		return publictime++;
	else
		return publictime;
#else
		return time(NULL);
#endif
}
} /* namespace gtsda */
