/*
 * MultMediaBlocks.h
 * 音视频块的数据结构
 *  Created on: 2013-12-23
 *      Author: yangkun
 */

#ifndef MULTMEDIABLOCKS_H_
#define MULTMEDIABLOCKS_H_
#include "Blocks.h"
#include "FrameQueue.h"
#include "DataWR.h"
#include <list>
namespace gtsda
{

class MultMediaBlocks:public Blocks
{
public:
	static const unsigned char audio_head[8];
	static const unsigned char video_head[8];
	static const unsigned char mult_audio_head[8];
	static const unsigned char mult_video_head[8];
	static const unsigned char mult_media_head[8];

	MultMediaBlocks(unsigned int uBufSize,blocktype bt);
	virtual ~MultMediaBlocks();
	int add(DataRead *dr,blocktype bt);
	void add_head(struct media_block *m_b);
	void add_head(struct media_block *m_b,blocktype bt);

	void add_vidoe(long long seek,unsigned int buf_size,int channel)
	{
		if(!mmb)throw(0);
		mmb->all_media_head[channel].video_seek = seek;
		mmb->all_media_head[channel].video_buf_size= buf_size;
	};
	void add_audio(long long seek,unsigned int buf_size,int channel)
	{
		if(!mmb)throw(0);
		mmb->all_media_head[channel].audio_seek = seek;
		mmb->all_media_head[channel].audio_buf_size= buf_size;
	};
	void add_next_mult_media_block_seek(long long seek){mmb->next_mult_media_block=seek;};
	unsigned int get_buff_size(){return mb->size;};

	//处理音频块，视频块的类
	//从缓冲中读取数据放到big_block中
protected:
	struct media_block *mb;
	struct mult_media_block *mmb;

};










} /* namespace gtsda */
#endif /* MULTMEDIABLOCKS_H_ */
