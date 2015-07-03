/*
 * testlib.c
 *
 *  Created on: 2014-2-8
 *      Author: yangkun
 */
#include <stdio.h>
#include "cliboftestsda.h"
#define BUFFSIZE 400*1024
int main(int argc, char *argv[])
{
	unsigned int channel=0;
	unsigned int starttime;
	if(argc!=3)
	{
		printf("usage: %s channelnum starttime\n",argv[0]);
		exit(1);
	}
	channel= atoi(argv[1]);
	if(channel<0 || channel>3)
	{
		printf("the channel is error!\n");
		exit(1);
	}
	starttime = atoi(argv[2]);
	if(starttime==0)
	{
		printf("starttime is error!!\n");
		exit(1);
	}
	printf("channel: %d,starttime:%d\n",channel,starttime);

	int ret;
	char buf[BUFFSIZE];
	//初始化sdk
	sda_init();

	/*测试查询录像时间段*/
#if 0
	//查询时间段
	memset(buf, 0, BUFFSIZE);
	query_record_section(0,1390953994,starttime, buf, BUFFSIZE);
	printf("queue time:%s\n",buf);
	goto error;
#else
	//获取空闲通道
	int userno=sda_get_idle_user();
	printf("the idle userno: %d\n", userno);


	//开始 1390986994 九点多,  1390976994 1392372454  时间自己查询得到
	ret=sda_startplay(userno,channel,starttime, 1000);
	if(ret<0)
	{
		printf("start play error!\n");
		goto error;
	}
	//获取数据保存数据
	FILE *video_fp = fopen("video00.264","w");
	FILE *audio_fp = fopen("audio00.pcm","w");
	int media_size;
	stream_format_t sft;
	int aaa;
	//先读100帧数据
	for(aaa=100;aaa>0;aaa--)
	{
		printf("aaa=%d\n",aaa);
		//usleep(10000);
		memset(buf, 0, BUFFSIZE);
		//视频
		ret=sda_get_media(userno,channel,TYPE_VIDEO,buf, BUFFSIZE, \
				&media_size, &sft);
		if(ret<0)
		{
			printf("error in get media: %d\n",ret);
			goto error;
		}
		printf("video->media_size:%d, \n",media_size);
		fwrite(buf,1, media_size,video_fp);
		fflush(video_fp);
		//音频
		while(1)
		{
			memset(buf, 0, BUFFSIZE);
			ret=sda_get_media(userno,channel,TYPE_AUDIO,buf, BUFFSIZE, \
					&media_size, &sft);
			if(ret<0)
			{
				if(ret==ERR_NO_DATA)
				{
					printf("no audio data!!\n");
					break;
				}
				else
				{
					printf("error in get media: %d\n",ret);
					goto error;
				}
			}
			//printf("audio->media_size:%d \n",media_size);
			fwrite(buf, 1, media_size,audio_fp);
			fflush(audio_fp);
		}

	}
	fclose(video_fp);
	fclose(audio_fp);
	//调用停止接口
	sda_stopplay(userno);











#if 0
	channel=2;
printf("begin start again\n");
	//再次调用开始播放接口
	//开始 1390986994 九点多,  1390976994   时间自己查询得到
	ret=sda_startplay(userno,channel,1392392407, 1000);
	if(ret<0)
	{
		printf("start play error!\n");
		goto error;
	}
	//获取数据保存数据
	video_fp = fopen("video11.264","w");
	audio_fp = fopen("audio11.pcm","w");
	//先读100帧数据
	for(aaa=100;aaa>0;aaa--)
	{
		printf("aaa=%d\n",aaa);
		usleep(10000);
		memset(buf, 0, BUFFSIZE);
		//视频
		ret=sda_get_media(userno,channel,TYPE_VIDEO,buf, BUFFSIZE, \
				&media_size, &sft);
		if(ret<0)
		{
			printf("error in get media: %d\n",ret);
			goto error;
		}
		printf("video->media_size:%d, \n",media_size);
		fwrite(buf,1, media_size,video_fp);
		fflush(video_fp);
		//音频
		while(1)
		{
			memset(buf, 0, BUFFSIZE);
			ret=sda_get_media(userno,channel,TYPE_AUDIO,buf, BUFFSIZE, \
					&media_size, &sft);
			if(ret<0)
			{
				if(ret==ERR_NO_DATA)
				{
					printf("no audio data!!\n");
					break;
				}
				else
				{
					printf("error in get media: %d\n",ret);
					goto error;
				}
			}
			printf("audio->media_size:%d \n",media_size);
			fwrite(buf, 1, media_size,audio_fp);
			fflush(audio_fp);
		}

	}
	fclose(video_fp);
	fclose(audio_fp);
	sda_stopplay(userno);
#endif






#endif
error:
	sda_free();
}
