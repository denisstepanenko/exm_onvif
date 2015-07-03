/*
 * MultMediaBlocks.cpp
 *
 *  Created on: 2013-12-23
 *      Author: yangkun
 */

#include "MultMediaBlocks.h"

namespace gtsda
{
const unsigned char MultMediaBlocks::audio_head[8] =      {0x61, 0x75, 0x64, 0x69, 0x6f, 0xa5, 0xaa, 0x5a};/*audio 0xa5,0xaa,0x5a*/
const unsigned char MultMediaBlocks::video_head[8] =      {0x76, 0x69, 0x64, 0x65, 0x6f, 0xa5, 0xaa, 0x5a};/*video 0xa5, 0xaa,0x5a*/
const unsigned char MultMediaBlocks::mult_audio_head[8] = {0x61, 0x75, 0x64, 0x69, 0x6f, 0x6d, 0x75, 0xaa};/*audio mu 0xaa*/
const unsigned char MultMediaBlocks::mult_video_head[8] = {0x76, 0x69, 0x64, 0x65, 0x6f, 0x6d, 0x75, 0xaa};/*video mu 0xaa*/
const unsigned char MultMediaBlocks::mult_media_head[8] = {0x6d, 0x75, 0x6c, 0x74, 0xaa, 0x5a, 0xa5, 0x55};/*mult 0xaa 0x5a 0xa5 0x55*/


MultMediaBlocks::MultMediaBlocks(unsigned int uBufSize,blocktype bt)
:Blocks(uBufSize, 0, bt)
{
	/**************************************************
	 * 解释最近一行为什么这样写，是不是感觉到很怪异？
	 * 写的时候是考虑到最大可能<<使用使用SecBlocks类的代码>>，还是其它crc的算法
	 * 先留出SecBlocks的struct hd_frame头所占的位置
	 ***************************************************/
	if(bt == multmedia)//mult_media不同的处理
	{
		mmb = (struct mult_media_block *)( (char *)GetBuf() + sizeof(struct hd_frame) );
		//根据不同类型复制不同的头
		memcpy(mmb->data_head, MultMediaBlocks::mult_media_head, 8);
	}
	else
	{
		//cout <<"debug bt: " << bt<< endl;
		mb = (struct media_block *)( (char *)GetBuf() + sizeof(struct hd_frame) );
		mb->size = sizeof(struct media_block);
		//根据不同类型复制不同的头
		add_head(mb,bt);
		//ttt();
		//myprint((const unsigned char  *)mb->data_head,8);
	}

	/***************************************************/


}
MultMediaBlocks::~MultMediaBlocks()
{
	// TODO Auto-generated destructor stub
}
//根据不同类型复制不同的头
void MultMediaBlocks::add_head(struct media_block *m_b)
{
	//if(!m_b)throw(0);
	//视频
	if(GetType() == video)
		memcpy(m_b->data_head, MultMediaBlocks::video_head, 8);
	//视频集合
	else if(GetType() == mult_video)
		memcpy(m_b->data_head, MultMediaBlocks::mult_video_head, 8);
	//音频
	else if(GetType() == audio)
		memcpy(m_b->data_head, MultMediaBlocks::audio_head, 8);
	//音频集合
	else if(GetType() == mult_audio)
		memcpy(m_b->data_head, MultMediaBlocks::mult_audio_head, 8);
	//不在范围内
	else
		throw(0); //其它类型本类不支持

}
void MultMediaBlocks::add_head(struct media_block *m_b,blocktype bt)
{
	//if(!m_b)throw(0);
	//视频
	if(bt == video)
		memcpy(m_b->data_head, MultMediaBlocks::video_head, 8);
	//视频集合
	else if(bt == mult_video)
		memcpy(m_b->data_head, MultMediaBlocks::mult_video_head, 8);
	//音频
	else if(bt == audio)
		memcpy(m_b->data_head, MultMediaBlocks::audio_head, 8);
	//音频集合
	else if(bt == mult_audio)
		memcpy(m_b->data_head, MultMediaBlocks::mult_audio_head, 8);
	//不在范围内
	else
	{
		cout << "ddddddddddddddddddddd bt: " << bt  << endl;
		throw(0); //其它类型本类不支持
	}

}
/************************************************
 * 功能：把DataRead中的数据复制到本类中，可能的块：audio,video,mult_video,mult_audio
 * 参数：DataRead *dr 读到的数据
 * 返回：大于等于0 复制成功
 * 		小于0    失败，可能是将要溢出
 ************************************************/
int MultMediaBlocks::add(DataRead *dr,blocktype bt)
{
	if(!dr)throw(0);
	//检查剩下的空间还够不够存入本帧数据
	if(dr->get_frame_buff_size() + sizeof(struct media_block) > this->GetSize() - sizeof(struct hd_frame) - mb->size)
	{
		ttt();
		cout << "the buff is small"<<endl;
		gtlogerr("the buff is small");
		return -1;
	}
	//构建头
	struct media_block *mb_local = ( struct media_block * )( (char *)mb + mb->size );
	add_head(mb_local,bt);
	mb_local->size = dr->get_frame_buff_size();


	mb->size = mb->size + sizeof(struct media_block);
	//存入数据
	memcpy( (char *)mb + mb->size, dr->get_the_frame_buffer()->frame_buf, dr->get_frame_buff_size());
	//cout << "size : "<< dr->get_frame_buff_size()<<endl;
	mb->size  = mb->size +  dr->get_frame_buff_size() ;
	//myprint((const unsigned char  *)mb_local->data_head,8);
	return 0;
}

} /* namespace gtsda */
