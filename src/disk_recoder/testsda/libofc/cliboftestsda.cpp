/*
 * cliboftestsda.cpp
 *
 *  Created on: 2014-2-8
 *      Author: yangkun
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include <string>

#include "gtlog.h"

#include "RWlogic.h"

#include "cliboftestsda.h"

using namespace gtsda;
gtsda::RWlogic *rdl;
gtsda::RWlogic::ReadDisk *rd[SDA_READ_NUM];
void *thr_fn(void *arg)
{
	static int i;
	memcpy(&i, arg, 4);
	std::cout << "sda pthread  " << i << " create success!!!" <<  std::endl;
	rd[i]->read_proc();
	rd[i]->thenum = i;
	return NULL;
}
//判断用户号是否正常
bool inline is_userno(int userno)
{
	if(userno>=0 && userno <SDA_READ_NUM)
		return true;
	else
		return false;
}

//初始化读硬盘库
int sda_init()
{
	rdl= gtsda::RWlogic::newRW(false/*read*/);
	rdl->Init();
	//首先创建READ_NUM个读线程
	int i,*j;
	int err;
	for(i=0; i<SDA_READ_NUM; i++)
	{
		j = new (int);
		*j = i;
		rd[i] = new gtsda::RWlogic::ReadDisk(0);
		err = pthread_create(&rd[i]->ntid, NULL, thr_fn, j);
		if (err != 0) {
			fprintf(stderr, "can't create thread: %s\n", strerror(err));
			exit(1);
		}
	}
	return 0;
}

//获取录像时间段
int query_record_section(int channel,int time_start,int time_end,\
		/*IN OUT*/char * time_sec_buf,int time_sec_buf_size)
{
	string str;
	int ret;
	ret=rdl->get_time_index(time_start,time_end,str);
	cout << "query record section: " << str << endl;
	int size=str.size();
	size=size<time_sec_buf_size?size:time_sec_buf_size;
	memcpy(time_sec_buf, str.c_str(), size);
	return ret;
}
//获取空闲通道
int sda_get_idle_user()
{
	int i;
	for(i=0; i<SDA_READ_NUM; i++)
	{
		if(!rd[i]->isRead)
			//break;
			return i;
	}
	return -1;
}

//开始播放
int sda_startplay(int usrno,int channel,int starttime, int playtime)
{
	fprintf(stderr, "come in startplay!!\n");
	if(!is_userno(usrno))
		return ERR_WRONG_CHANNEL;
	long long seek;
	fprintf(stderr, "come in startplay!!1\n");
	if( ( seek=rdl->is_in(starttime) ) == 0)
	{
		fprintf(stderr, "get seek err\n");
		cout << "starttime is wrong\n\n\n" << endl;
		//时间不在块中，或者在块中找不到此时间
		return BLOCK_ERR_NOT_IN;
	}
	fprintf(stderr, "starttime:%d,get seek:%lld!!\n\n\n\n\n\n",starttime,seek);
	//bug fix 以下代码可能不安全
	rd[usrno]->setread(channel,seek );
	//rd[usrno]->isRead = true;
	rd[usrno]->start_play();
	fprintf(stderr, "over\n");
	return 0;
}

//停止播放
int sda_stopplay(int usrno)
{
	if(!is_userno(usrno))
		return ERR_WRONG_CHANNEL;
	rd[usrno]->clear();
	rd[usrno]->isRead = false;
	return 0;
}

//获取音视频数据
int sda_get_media(int usrno,int channel,int media_type,/*IN OUT*/char *buf,unsigned int buf_size, \
		/*OUT*/unsigned int *media_size, /*OUT*/stream_format_t *sft)
{
	if(!is_userno(usrno))
		return ERR_WRONG_CHANNEL;

	gtsda::FrameQueue<Blocks *> *video_queue=NULL;
	gtsda::FrameQueue<Blocks *> *audio_queue=NULL;
	gtsda::Blocks *bs=NULL; 	//返回结构
	unsigned int size;			//返回的数据大小

	if(TYPE_VIDEO == media_type)
	{
		//获取视频队列
		video_queue = rd[usrno]->get_video();
		//cout << "before video queue size: " << video_queue->size() << endl;
		//检查队列中的数据大小
		while(video_queue->size()<10)
		{
			//cout << "no enough video data!,size: " << video_queue->size()<< endl;
			rd[usrno]->start_play();
			rd[usrno]->set_notful();
		}
		//cout << "after video queue size: " << video_queue->size() << endl;
		//获取最前面的数据
		bs = video_queue->pop();
		size = bs->GetSize();
		//if(bs->GetType() == noblock)
		//	*video_type = TYPE_VIDEO_I;
		//else
		//	*video_type = TYPE_VIDEO_P;
	}
	else if(TYPE_AUDIO == media_type)
	{
		//获取音频队列
		audio_queue = rd[usrno]->get_audio();
		if(audio_queue->size()<=0)
		{
			//cout << "no audio data!!!" << endl;
			//rd[fParams.channel]->set_notful();
			return ERR_NO_DATA;
		}
		//cout << "audio queue size: " << audio_queue->size() << endl;
		bs 	=audio_queue->pop();
		size= bs->GetSize();
	}
	else return ERR_TYPE;

	//复制数据
	//cout << "size: " << size << "buf_size: " << buf_size << endl;
	size = buf_size<=size?buf_size:size;
	//cout << "size: " << size  << endl;
	memcpy(buf, bs->GetBuf(), size);//哪个小选哪个
	*media_size=size;
	delete bs;
	bs=NULL;
	return 0;
}

//关闭sda
void sda_free()
{
	free(rdl);
}
