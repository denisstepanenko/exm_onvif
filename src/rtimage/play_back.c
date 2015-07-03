#include <stdio.h>
#include "play_back.h"
//zsk 为录像回放添加的，认为线程锁可以优化为pthread_rwlock?多处理器时
static playback_t pb;
int default_screen=4;
extern int debug;
playback_t * get_playback_parm(void)
{

	return &pb;


}

void init_playback_parm(void)
{
	int i;
	playback_t * pb=get_playback_parm();
	memset(pb->pb_venc,0,sizeof(pb->pb_venc));//回放缓冲池连接状态
	memset(pb->pb_aenc,0,sizeof(pb->pb_aenc));
	memset(pb->pb_vct,0,sizeof(pb->pb_vct));
	memset(pb->pb_act,0,sizeof(pb->pb_act));
	
	memset(pb->pb_ct,0,sizeof(pb->pb_ct));//倒数时间
	memset(pb->frame_adjust,0,sizeof(pb->frame_adjust));//编码器回放指针偏移，>0
	memset(pb->aframe_adjust,0,sizeof(pb->aframe_adjust));//编码器回放指针偏移，>0
	
	
	
	for(i=0;i<MAX_VIDEO_ENCODER;i++)
	{
	
			pb->pb_first_fg[i]=-1;//编码器的连接状态，>0不可以连接，=0可以连接

			pb->playback_flag[i]=-1; //-2 回到实时画面但回放没结束;-1 正常状态;>0通道回放

	
	}
	for(i=0;i<TCPRTIMG_MAX_AVUSR_NO+1;i++)
		pb->current_net_ch[i]=4;

	pb->default_screen=4;
	pthread_mutex_init(&pb->mutex,NULL);
}


void set_playback_stat(int no,int stat);
void mutichannel_set_playback_en(int no)
{
	//使能录像回放
	int stat;
	int i;
	stat=get_playback_stat(no);
	playback_t * pb=get_playback_parm();
	tcprtimg_svr_t	  *p=get_rtimg_para();
	av_usr_t		  *usr=NULL; 
	stream_send_info_t	*send_info =NULL;
	//还没开始

	if (stat<0)
	{
		if(pb->pb_venc[no]==-1)//判断回放缓冲池是否已连接
		{
			//pthread_mutex_lock(&pb->mutex);
			set_playback_stat(no,no);
			//pthread_mutex_unlock(&pb->mutex);
			gtloginfo("开始录像回放，通道[%d]\n",no);
		}
		else
		{


			gtloginfo("还没有存储那么多数据，不能回放\n");
			return;

		}
		
	}
	//在回放过程中重复调用同一通道
	else if(get_playback_stat(no)!=-1)
	{
		gtloginfo("通道[%d]回放中，再次回放[%d]通道\n",stat,no);
		return;
		

	}
	

	//开始播放
	for(i=0;i<TCPRTIMG_MAX_AVUSR_NO+1;i++)
	{

		usr=&p->av_server.av_usr_list[i];
		if(usr->venc_no==no)
		{
			send_info = &usr->send_info;
			pthread_mutex_lock(&usr->u_mutex);
			send_info->first_flag=1;
			pthread_mutex_unlock(&usr->u_mutex);
			pthread_mutex_lock(&pb->mutex);
			pb->pb_ct[no]=p->frame_rate*(p->playback_pre+p->playback_dly);
			pthread_mutex_unlock(&pb->mutex);
			break;
		}

	}



	gtloginfo("playback_flag==%d\n",pb->playback_flag[no]);
	return;
}


void mutichannel_set_playback_cancel(int no)
{
	//取消录像回放
	playback_t * pb=get_playback_parm();
	pthread_mutex_lock(&pb->mutex);
	memset(pb->playback_flag,-1,sizeof(pb->playback_flag));//-2 回到实时画面但回放没结束;-1 正常状态;>0通道回放
	memset(pb->pb_ct,0,sizeof(pb->pb_ct));//倒数时间

	pthread_mutex_unlock(&pb->mutex);
	tcprtimg_svr_t	  *p=get_rtimg_para();
	av_usr_t		  *usr=NULL; 
	stream_send_info_t  *send_info =NULL;
	int i;
	int ret;
	for(i=0;i<MAX_VIDEO_ENCODER;i++)
	{
		ret=get_playback_frame_adjust(i);
		if(ret>0)
		{
			recover_playback_iframe2normal(i,ret);
			set_playback_frame_adjust(i,0);
		}

	}
	for(i=0;i<TCPRTIMG_MAX_AVUSR_NO+1;i++)
	{
		usr=&p->av_server.av_usr_list[i];
		if(usr->venc_no==no)
		{
			usr=&p->av_server.av_usr_list[i];
			send_info = &usr->send_info;
			pthread_mutex_lock(&usr->u_mutex);
			send_info->first_flag=1;
			pthread_mutex_unlock(&usr->u_mutex);
			break;
		}

	}
	return ;
}

void mutichannel_set_playback_to_live(int enc_no)
{
	playback_t * pb=get_playback_parm();

	pthread_mutex_lock(&pb->mutex);
	pb->playback_flag[enc_no]=-2;
	pthread_mutex_unlock(&pb->mutex);
	tcprtimg_svr_t	  *p=get_rtimg_para();
	av_usr_t		  *usr=NULL; 
	stream_send_info_t  *send_info =NULL;
	int i;
	//找到那个用户在链接这个enc
	for(i=0;i<TCPRTIMG_MAX_AVUSR_NO+1;i++)
	{

		usr=&p->av_server.av_usr_list[i];
		if(usr->venc_no==enc_no)
		{
			send_info = &usr->send_info;
			pthread_mutex_lock(&usr->u_mutex);
			send_info->first_flag=1;
			pthread_mutex_unlock(&usr->u_mutex);
			break;
		}
	}

}

/****************************************************
 *函数名:get_playback_stat()
 *功能  : 返回当前录像回放的标志位
 *输入  : 无
 *返回值:正直表示正在录像回放的通道，-1停止录像回放
 * *************************************************/
int get_playback_stat(int no)
{
	int ret;
	playback_t * pb=get_playback_parm();
	if(no<0||no>=MAX_VIDEO_ENCODER)
	  no=0;
		
	ret = pb->playback_flag[no];
	return ret;
}
/****************************************************
 *函数名:set_playback_stat()
 *功能  : 设置当前录像回放的标志位
 *输入  : 无
 *返回值:正直表示正在录像回放的通道，-1停止录像回放
 * *************************************************/
void set_playback_stat(int no,int stat)
{
	playback_t * pb=get_playback_parm();
	pthread_mutex_lock(&pb->mutex);
	pb->playback_flag[no]=stat;
	pthread_mutex_unlock(&pb->mutex);

}
/****************************************************
 *函数名:get_playback_frame_adjust()
 *功能  : 返回当前录像回放的标志位
 *输入  : 无
 *返回值:正直表示正在录像回放的通道，-1停止录像回放
 * *************************************************/
int get_playback_frame_adjust(int no)
{
	int ret;
	playback_t * pb=get_playback_parm();
	ret = pb->frame_adjust[no];
	//printf("get_playback_frame_adjust[%d][%d]\n",no,ret);
	return ret;
}
/****************************************************
 *函数名:set_playback_frame_adjust()
 *功能  : 设置当前录像回放的标志位
 *输入  : 无
 *返回值:正直表示正在录像回放的通道，-1停止录像回放
 * *************************************************/
void set_playback_frame_adjust(int no,int frame_ct)
{
	playback_t * pb=get_playback_parm();
	pthread_mutex_lock(&pb->mutex);
	pb->frame_adjust[no]=frame_ct;
	pthread_mutex_unlock(&pb->mutex);

	gtloginfo("set_playback_frame_adjust[%d][%d]\n",no,pb->frame_adjust[no]);
	printf("set_playback_frame_adjust[%d][%d]\n",no,pb->frame_adjust[no]);

}
/****************************************************
 *函数名:get_playback_aframe_adjust()
 *功能  : 返回当前录像回放的标志位
 *输入  : 无
 *返回值:正直表示正在录像回放的通道，-1停止录像回放
 * *************************************************/
int get_playback_aframe_adjust(int no)
{
	int ret;
	playback_t * pb=get_playback_parm();
	ret = pb->aframe_adjust[no];


	return ret;
}
/****************************************************
 *函数名:set_playback_aframe_adjust()
 *功能  : 设置当前录像回放的标志位
 *输入  : 无
 *返回值:正直表示正在录像回放的通道，-1停止录像回放
 * *************************************************/
void set_playback_aframe_adjust(int no,int frame_ct)
{
	playback_t * pb=get_playback_parm();
	pthread_mutex_lock(&pb->mutex);
	pb->aframe_adjust[no]=frame_ct;
	pthread_mutex_unlock(&pb->mutex);
	printf("set_playback_frame_adjust[%d][%d]\n",no,pb->aframe_adjust[no]);

}


