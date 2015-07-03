/*处理与实时图像传送模块通讯的命令

*/
#include "ipmain.h"
#include "gate_cmd.h"
#include "maincmdproc.h"
#include "netcmdproc.h"
#include "process_rtimg.h"
#include "ipmain_para.h"
//#include "hdmodule.h"
#include "devstat.h"
#include "hdmodapi.h"
#ifdef ARCH_3520D
#include "audioout_api.h"
#endif

static DWORD	old_rtimg_state=0;

#if 0
//获取实时图像传送进程的状态
//正常返回0 有故障返回1
int get_netenc_state(void)
{
	struct ip1004_state_struct * gtstate;
	int state;
	gtstate=get_ip1004_state();
	pthread_mutex_lock(&gtstate->mutex);
	state=gtstate->reg_dev_state.video_enc0_err;
	pthread_mutex_unlock(&gtstate->mutex);	
	return state;
}
#endif

int process_rtimg_state(mod_socket_cmd_type *cmd)
{//
	pid_t *pid;
	DWORD *state;
	struct rtimage_state_struct *newstate,*change_state;
	struct ip1004_state_struct * gtstate;
	struct ipmain_para_struct *main_para;
	//struct hd_enc_struct	*hd_enc=NULL;
	DWORD	change;
	if(cmd->cmd!=RTSTREAM_STATE_RETURN)
		return -1;
	pid=(pid_t *)cmd->para;
	state=(DWORD*)&cmd->para[sizeof(pid_t)];
	printf("recv rtimg state:pid=%d state=0x%08x\n",(int)*pid,(int)*state);
	gtloginfo("收到rtimage状态:pid=%d state=0x%08x\n",(int)*pid,(int)*state);
	*state&=(DWORD)(~1);
	//printf("process_rtimg_state state is %x\n",*state);
	if(old_rtimg_state!=*state)
	{
		change=old_rtimg_state^*state;
		change&=0x3f;			//
		//printf("process_rtimg_state change is %x\n",change);
		if(change!=0)
		{
			change_state=(struct rtimage_state_struct*)&change;			
			newstate=(struct rtimage_state_struct *)state;	
			if(change_state->net_enc_err)
			{
				gtstate=get_ip1004_state(0);
				pthread_mutex_lock(&gtstate->mutex);
				gtstate->reg_dev_state.video_enc0_err=newstate->net_enc_err;
				pthread_mutex_unlock(&gtstate->mutex);
				//gtloginfo("视频丢失，芯片故障，发送状态\n");
#ifdef SHOW_WORK_INFO
				printf("视频丢失，芯片故障，发送状态\n");
#endif
				send_dev_state(-1,1,0,0,0,0);
			}
			if(change_state->net_enc_busy)
			{
				if(!newstate->net_enc_busy)
				{//空闲
#if 0				
					if(get_quad_flag()==1)
						{
							//lc do 当编码器空闲(30 sec)时，切换回默认通道
							
							main_para=get_mainpara();
							if(main_para->net_ch<4)
								set_net_scr_full(main_para->net_ch);
							else
								set_net_scr_quad();
							
#ifdef SHOW_WORK_INFO
							printf("图像连接空闲 切换到默认通道:%d\n",main_para->net_ch);
#endif							
							gtloginfo("图像连接空闲 切换到默认通道:%d\n",main_para->net_ch);
						}
#endif
				}

			}
		}		
		old_rtimg_state=*state;
	}
	return 0;
}


void process_playback_stop_cmd()
{
	gtloginfo("向videoenc发送停止录像回放命令RTIMG_PLAYBACK_STOP_CMD\n");
	alarm_cancel_playback();
}

extern pthread_mutex_t g_audiodown_channel_mutex;

void process_audiodown_cmd(mod_socket_cmd_type *cmd)
{
	pid_t *pid;
	int   *p_audio_down_chan;
	struct ipmain_para_struct * para;
	if(cmd->cmd!=RTIMG_AUDIODOWN_CMD)
		return -1;

	pid=(pid_t *)cmd->para;
	p_audio_down_chan=(int*)&cmd->para[sizeof(pid_t)];
	printf("recv rtimg audio down channel:pid=%d chn=%d\n",(int)*pid,(int)*p_audio_down_chan);
	gtloginfo("收到rtimage下行通道号:chn=%d\n",(int)*p_audio_down_chan);

	para=get_mainpara();

	pthread_mutex_lock(&g_audiodown_channel_mutex);
	para->current_audio_down_channel = (int)*p_audio_down_chan;
	pthread_mutex_unlock(&g_audiodown_channel_mutex);

	return;
	
}


/**********************************************************************************************
 * 函数名	:process_rtimg_cmd()
 * 功能	:处理tcprtimg模块发来的命令
 * 输入	:cmd:接收到的tcprtimg模块的命令缓冲区
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int process_rtimg_cmd(mod_socket_cmd_type *cmd)
{
	switch(cmd->cmd)
	{
		case MOD_BYPASSTO_GATE_ACK:
			printf("recv tcprtimg MOD_BYPASSTO_GATE_ACK\n");
			gtloginfo("recv tcprtimg MOD_BYPASSTO_GATE_ACK\n");
			process_gate_cmd_ack(cmd);
			break;

		case RTSTREAM_STATE_RETURN:
			process_rtimg_state(cmd);
			break;

		case RTIMG_PLAYBACK_STOP_CMD:
			printf("接收到RTIMG_PLAYBACK_STOP_CMD\n");
			gtloginfo("接收到RTIMG_PLAYBACK_STOP_CMD\n");
			process_playback_stop_cmd();
			break;
		case RTIMG_AUDIODOWN_CMD:
			printf("接收到RTIMG_AUDIODOWN_CMD\n");
			gtloginfo("接收到RTIMG_AUDIODOWN_CMD\n");
			process_audiodown_cmd(cmd);
			break;
					
		default:
			printf("vsmain recv a unknow cmd:%04x from rtimg\n",cmd->cmd);
			gtloginfo("recv a unknow cmd:%04x from rtimg\n",cmd->cmd);
			break;
	}
	return 0;
}




