#include <soundapi.h>	//必须包含此文件以使用soundapi库
#include <audiofmt.h>
#include <error.h>	
#include <errno.h>
#include <typedefine.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "devinfo.h"
#include "gtlog.h"

#define		DEF_SAMRATE		8000	//缺省采样率
#define		DEF_SAMSIZE		16		//缺省采样位数
#define 	DEF_CHANNELS	2		//缺省通道数
#define 	DEF_FRAGSIZE	10		//缺省fragment大小
#define		DEF_FRAGNB		16		//缺省fragment数



#define 	VERSION			"0.09"
//0.09		检测设备是否有杂音，上海新生产的3024有时会有杂音，分析标志为音频数据经ulaw编码后，会有0值数据出现，此时重新打开音频设备即可
//0.08		启动时多打开，关闭几次音频设备：上海生产的3024，刚启动时，有时会有杂音
//0.07		恢复到和GTVS1000的deafmute相同。
//0.06		统一成先打开rec再打开open
//0.05 		加入gtopenlog	
//0.04

FILE	*rec_file=NULL;

int main(void)
{
	int devtype;
	snd_dev_t *rec_dev = NULL; //操作采集声音设备的句柄
	snd_dev_t *play_dev = NULL; //操作录制声音设备的句柄

	
	#define REC_DATA_SIZE	1024
//	unsigned short playbuf[REC_DATA_SIZE];
	unsigned short rec_buf[REC_DATA_SIZE];
	char *prec=(char*)rec_buf;				///指向采集缓冲区的指针
	
	int i,j,ret;
	
	int repeat_cnt=0;						///重试打开音频设备的次数
	int bad_cnt=0;							///读到的音频错数据次数(ulaw编码后值为0认为错误)


    gtopenlog("deafmute");   
   	init_devinfo();
   	devtype = get_devtype();
   	printf("deafmute version:%s working on %s\n",VERSION,get_devtype_str());
   	gtloginfo("deafmute version:%s ,working on %s\n",VERSION,get_devtype_str());

	while(1)
	{
		///用于存放声音片段的文件
        rec_file=fopen("/deafmuteu.raw","wb");
        if(rec_file==NULL)
                printf("can't create deafmuteu.raw!\n");


		///打开采集设备
		rec_dev = open_audio_rec_dev();
        if(rec_dev==NULL)//有错误
        {
                printf("error opening record device, errno %d:%s\n",errno,strerror(errno));
                return -1;
        }
		ret=reset_audio_inbuf(rec_dev);                           ///<清空设备缓冲区
    	ret=set_audio_params(rec_dev,8000,16,2,10,16);  ///<设置音频参数
    	ret=set_audio_in_gain(rec_dev,0);       ///<设置mic音量
    	ret=set_audio_ready(rec_dev);


        /*打开播放设备*/
        play_dev = open_audio_play_dev();
        if(play_dev==NULL)//有错误
        {
                printf("error opening play device, errno %d:%s\n",errno,strerror(errno));
                close_audio_rec_dev(&rec_dev);
                return -1;
        }

		set_playback_blocknr(play_dev,-1); ///<<阻塞模式

		//if(repeat_cnt<=0)
		{

			///采集几片数据，分析，看是否有错的(2009.07在上海生产的板子发现有的设备有时声音有较大杂音)
			///分析数据发现u-law编码后有一些0值的数据，而正常设备没有

			printf("I'm deaf-and-mute, Grrrr!\n");
			for(i=0;i<10;i++)
			{
				ret = read_audio_data(rec_dev, (char*)rec_buf, REC_DATA_SIZE);
				printf("capture %d bytes audio data...\n",ret);
				ret = conv_stereo2right((char*)rec_buf,(char*)rec_buf,ret);			///<GTVS3000使用的是右声道
				ret=conv_raw2ulaw((char*)rec_buf,(char*)rec_buf,ret);
				if((rec_file!=NULL)&&(ret>0))
				{
					fwrite(rec_buf,ret,1,rec_file);
				}
				
				prec=(char*)rec_buf;
				for(j=0;j<ret;j++)
				{
					if(prec[j]==0)
					{
						bad_cnt++;			
						printf("audio bad_cnt=%d i=%d\t j=%d!!\n",bad_cnt,i,j);
					}
				}
				if(bad_cnt>10)
				{
					break;
				}
	        }

			if(rec_file!=NULL)
			{
				fclose(rec_file);
				rec_file=NULL;
			}
			if(bad_cnt>10)
			{
				printf("检测到声音有杂音 bad_cnt=%d,重新打开音频设备,repeat=%d...\n",bad_cnt,repeat_cnt);
				gtloginfo("检测到声音有杂音bad_cnt=%d,重新打开音频设备,repeat=%d...\n",bad_cnt,repeat_cnt);			
			}
			else
			{
				//一切正常
				while(1)
	                sleep(1);
			}
		}

		//发现问题，重新打开
        /*关闭设备*/
        if( close_audio_rec_dev(&rec_dev) < 0 )
        {
                printf("close rec device failed, errno %d:%s\n",errno,strerror(errno));
        	close_audio_play_dev(&play_dev);
        	return -1;
        }

        /*关闭设备*/
        if( close_audio_play_dev(&play_dev) < 0 )
        {
                printf("close play device failed, errno %d:%s\n",errno,strerror(errno));
	        return -1;
        }
		repeat_cnt++;
		if(repeat_cnt>5)	
		{//多次不行后就不管了
			while(1)
				sleep(10);
		}
	}


	return 0;
}


