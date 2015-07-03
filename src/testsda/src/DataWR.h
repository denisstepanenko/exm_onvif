/*
 * DataWR.h
 *
 *  Created on: 2013-10-29
 *      Author: yangkun
 */

#ifndef DATAWR_H_
#define DATAWR_H_
//数据的获取与写入
#include "Blocks.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "media_api.h"
#ifdef __cplusplus
}
#endif
namespace gtsda
{
class DataRead;
struct bblock
{
	unsigned int size;//p中有多个xxd的值
	unsigned int xxd;
	unsigned char p[4];
};
extern int publictime;
int gettime(bool add = true);
class DataWrite:public gtsda::Blocks
{
public:
	DataWrite();
	DataWrite(DataWrite &a):Blocks(BUFSIZE,0,block){cout << "warning !!!!" << endl;};
	virtual ~DataWrite();
	int writedata();
	DataWrite  &operator=( DataRead &dr);
	int getdata(DataRead &dr);
	int getdata(const char *buf, unsigned int buffsize);
	unsigned int  get_frame_buff_size(){return buffsize;};
	//char *   GetBuf()  {return buf;};
private:
	char *buf;
	long buffsize;
	int fifo_init();
	int fifo_write(const char *buf, int size);
	int fd;
};


#ifndef MEMIN
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

#endif
class DataRead: public gtsda::Blocks
{
public:
#ifdef MEMIN
	DataRead(unsigned int uBufSize,long long llSeek=0,blocktype bt=block);
	DataRead(DataRead &a):Blocks(a.GetSize(),a.GetSeek(),a.GetType()){cout << "warning !!!!" << endl;};
#else
	DataRead(long long llSeek=0,blocktype bt=block,unsigned int channel = 0);
	//DataRead(unsigned int channel );
#endif
	virtual ~DataRead();
	int readpool();
	enc_frame_t* get_the_frame_buffer(){return the_frame_buffer;};
	unsigned int  get_frame_buff_size(){return buffsize;};
	bool get_isi(){return bIsI;};
private:
#ifndef MEMIN
	media_source_t media;
	int		seq;                 ///<媒体数据序号
	int flag;
	enc_frame_t* the_frame_buffer;
	bool bIsI; //是不是I帧
	void init_pool(unsigned int channel);
	long buffsize;
#endif

	bblock *bs;
};
} /* namespace gtsda */
#endif /* DATAWR_H_ */
