#include "vs3_videoenc.h"
#include "videoencoder.h"
#include <gtthread.h>
#include <mshmpool.h>
#include <commonlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "device_state.h"
#include <errno.h>
#include <devinfo.h>
#include <devres.h>
#include <media_api.h>
#include "mod_cmd.h"
#include "process_modcmd.h"
#include "onvif_system.h"
#include "videoencoder.h"
//#include <fcapture_api.h>
#include <fenc_api.h>
//#include <osd_api.h>


static struct videoenc_state_struct videoenc_state={0,0,0,0,0,0,0,0,0,0};

video_enc_t	video_enc[MAX_ONVIF_DEVICE*2]; //包含主/副码流
audio_enc_t audio_enc[MAX_ONVIF_DEVICE*2];
int playback_flag=0;



struct videoenc_state_struct *get_videoenc_state(void)
{
	return &videoenc_state;
}

DWORD get_videoencstatint(void)
{
	DWORD stat;
	memcpy((void*)&stat,(void*)&videoenc_state,sizeof(DWORD));
	return stat;
}

int get_onvif_device_num(void)
{
	return 4;
}

//初始化媒体结构
static int init_media_source(void)
{
	int	i,total,ret;
	video_enc_t * v_enc=NULL;
	audio_enc_t * a_enc=NULL;
	total=get_onvif_device_num();

	if(total>MAX_ONVIF_DEVICE)
		total=MAX_ONVIF_DEVICE;
	
	// video init
	for(i=0;i<total*2;i++) //0~total 主码流,total~total*2 副码流
	{

		v_enc=&video_enc[i];
        v_enc->venc_cnt=0;
		v_enc->venc_err_cnt=0;
		v_enc->venc_first_start=1;
		if(i%2==0)//master media source
		{
			ret=init_media_rw(&v_enc->media,MEDIA_TYPE_VIDEO,0,FRAME_BUFFER_SIZE);
			if(ret!=0)
				printf("LOOOOOOK %d\n",__LINE__);
		}
			

		else  //slave media source

		{
			ret=init_media_rw(&v_enc->media,MEDIA_TYPE_VIDEO,0,FRAME_BUFFER_SIZE);
			if(ret!=0)
				printf("LOOOOOOK %d\n",__LINE__);

		}



	}	
	
	// audio enc init
	for(i=0;i<total*2;i++) //0~total 主码流,total~total*2 副码流
	{

		a_enc=&audio_enc[i];
		a_enc->enc_first_start=1;
		if(i%2==0)//master media source
		{
			ret=init_media_rw(&a_enc->media,MEDIA_TYPE_AUDIO,0,AUDIO_FRAME_SIZE);
			if(ret!=0)
				printf("LOOOOOOK %d\n",__LINE__);
		}
			

		else  //slave media source

		{
			ret=init_media_rw(&a_enc->media,MEDIA_TYPE_AUDIO,0,AUDIO_FRAME_SIZE);
			if(ret!=0)
				printf("LOOOOOOK %d\n",__LINE__);

		}



	}
	return 0;
}






static int	create_media_pool(void)
{
	int	i,total;
	int 	ret;
	char name[256];
	video_enc_t *v_enc=NULL;
	audio_enc_t *a_enc=NULL;
	total=get_onvif_device_num();

	if(total>MAX_ONVIF_DEVICE)
	{

		total=MAX_ONVIF_DEVICE;

	}

	for(i=0;i<total;i++)
	{

		//video
		v_enc=&video_enc[i*2];
		sprintf(name,"onvif video master[%d]",i);
		ret=create_media_write(&v_enc->media,get_onvif_pool_key(i,0),name,STREAM_POOL_SIZE_MASTER);

		v_enc=&video_enc[i*2+1];
		sprintf(name,"onvif video slave[%d]",i);
		ret=create_media_write(&v_enc->media,get_onvif_pool_key(i,1),name,STREAM_POOL_SIZE_SLAVE);



		//audio
		a_enc=&audio_enc[i*2];
		sprintf(name,"onvif audio master[%d]",i);
		ret=create_media_write(&a_enc->media,get_onvif_pool_key(i,0)+0x20000,name,AUDIO_POOL_SIZE_MASTER);

		a_enc=&audio_enc[i*2+1];
		sprintf(name,"onvif audio slave[%d]",i);
		ret=create_media_write(&a_enc->media,get_onvif_pool_key(i,1)+0x20000,name,AUDIO_POOL_SIZE_SLAVE);






	}

	return ret;
	
}
//初始化视频编码模块,包括变量初始化
//读取配置文件，创建缓冲池
int init_media_system(void)
{
	if(init_media_source())	
	{
		printf("%s %d err\n",__FUNCTION__,__LINE__);
		return -1;
	
	
	}
	if(create_media_pool())
	{
		printf("%s %d err\n",__FUNCTION__,__LINE__);
		return -1;
	}
	

	return 0;	
}
void video_encoder_thread_cleanup(void *v_enc)
{

}
static __inline__ int write_video_frame(video_enc_t *venc, enc_frame_t *frame)
{
	int s_len=frame->len+sizeof(enc_frame_t)-sizeof(frame->frame_buf);//-sizeof(frame->len);
	//printf("******写入的帧为[%d]帧，大小为[%d]\n",frame->type,s_len);
	return write_media_resource(&venc->media,frame,s_len,frame->type);
	
}








