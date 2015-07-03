
#include "ipmain.h"
#include "hdmodapi.h"
#include "maincmdproc.h"
#include "devstat.h"
#include "netcmdproc.h"
#include "video_para_api.h"
#include "devinfo.h"

extern int video_motion_detected(unsigned long motion);
extern int video_blind_detected(unsigned long blind);
extern int video_loss_proc(unsigned long loss);

int process_videoenc_state(mod_socket_cmd_type *cmd)
{
	int i;
	static DWORD old_state=0;
	DWORD *state,change;
	struct ip1004_state_struct * gtstate;
	struct videoenc_state_struct *newstate,*change_state;
	if(cmd==NULL)
		return -1;
	state=(DWORD*)&cmd->para[sizeof(pid_t)];
#ifdef SHOW_WORK_INFO
	printf("recv a encbox VIDEOENC_STATE_RETURN cmd state=0x%08x\n",(int )*state);
#endif
	gtloginfo("recv a encbox VIDEOENC_STATE_RETURN cmd state=0x%08x\n",(int )*state);

	change=*state^old_state;
	newstate=(struct videoenc_state_struct *)state;
	change_state=(struct videoenc_state_struct *)change;
	/*
	if(virdev_get_virdev_number()==2)
	{
		gtstate=get_ip1004_state(0);
		pthread_mutex_lock(&gtstate->mutex);
		gtstate->reg_dev_state.video_enc0_err=newstate->video_enc0_err;
		pthread_mutex_unlock(&gtstate->mutex);
		gtstate=get_ip1004_state(1);
		pthread_mutex_lock(&gtstate->mutex);
		gtstate->reg_dev_state.video_enc0_err=newstate->video_enc1_err;
		pthread_mutex_unlock(&gtstate->mutex);
		for(i=0;i<virdev_get_virdev_number();i++)
		{
			send_dev_state(-1,1,0,0,0,i);
		}
	}
	else
	*/
	{	gtstate=get_ip1004_state(0);
		pthread_mutex_lock(&gtstate->mutex);
		gtstate->reg_dev_state.video_enc0_err=newstate->video_enc0_err;
		gtstate->reg_dev_state.video_enc1_err=newstate->video_enc1_err;
		gtstate->reg_dev_state.video_enc2_err=newstate->video_enc2_err;
		gtstate->reg_dev_state.video_enc3_err=newstate->video_enc3_err;
		gtstate->reg_dev_state.video_enc4_err=newstate->video_enc4_err;
		pthread_mutex_unlock(&gtstate->mutex);
		send_dev_state(-1,1,0,0,0,0);
	}
	/*
	if((get_quad_flag()==0)&&(virdev_get_virdev_number()==1))//3021
	{
		video_motion_detected(newstate->video_motion0);//目前的型号只会用到第0路移动侦测
		video_blind_detected(newstate->video_blind0);
	}
	*/

	return 0;
}

int process_videoenc_vda_state(mod_socket_cmd_type *cmd)
{
	int i;
	static DWORD old_state=0;
	DWORD *state,change;
	struct ip1004_state_struct * gtstate;
	struct videoenc_vda_state_struct *newstate,*change_state;
	//unsigned char loss[4] = {0};
	//unsigned char motion[4] = {0};
	//unsigned char blind[4] = {0};
	unsigned long loss,motion,blind;
	
	if(cmd==NULL)
		return -1;
	state=(DWORD*)&cmd->para[0];
#ifdef SHOW_WORK_INFO
	//printf("recv a encbox VIDEOENC_VDA_STATE cmd state=0x%08x\n",(int )*state);
#endif	
	//gtloginfo("recv a encbox VIDEOENC_VDA_STATE cmd state=0x%08x\n",(int)*state);
	loss = cmd->para[0];
	motion = cmd->para[1];
	blind =cmd->para[2];
	
	//printf("loss is 0x%08x\n",loss);

	video_loss_proc(loss);
	video_motion_detected(motion);
	video_blind_detected(blind);

	return 0;

}

int encbox_coaxial_err_count = 0;
pthread_mutex_t encbox_mutex;

int process_videoenc_err(mod_socket_cmd_type *cmd)
{
	int i;
	char errstate;
	if(cmd==NULL)
		return -1;
	errstate=cmd->para[0];

	gtloginfo("recv encbox VIDEOENC_COAXIAL_ERR 0x%02x!\n",errstate);

	//lc to do 发送设备状态给网关，待定义
	pthread_mutex_lock(&encbox_mutex);
	encbox_coaxial_err_count++;
	pthread_mutex_unlock(&encbox_mutex);
	
	return 0;
	
}

/**********************************************************************************************
 * 函数名	:process_videoenc_cmd()
 * 功能	:处理videoenc模块发来的命令
 * 输入	:cmd:接收到的videoenc模块的命令缓冲区
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int process_videoenc_cmd(mod_socket_cmd_type *cmd)
{
	if(cmd==NULL)
		return -1;
	switch(cmd->cmd)
	{
		case MOD_BYPASSTO_GATE_ACK:
			printf("recv encbox MOD_BYPASSTO_GATE_ACK\n");
			gtloginfo("recv encbox MOD_BYPASSTO_GATE_ACK\n");
			process_gate_cmd_ack(cmd);
		break;
		case VIDEOENC_STATE_RETURN:
			process_videoenc_state(cmd);
		break;
		//lc do 接受vda相关状态，并作处理
		case VIDEOENC_VDA_STATE:
			process_videoenc_vda_state(cmd);
		break;
		case VIDEOENC_COAXIAL_ERR:
			process_videoenc_err(cmd);
			break;
		default:
			printf("recv a unknow encbox cmd:%04x \n",cmd->cmd);
			gtloginfo("recv a unknow encbox cmd:%04x \n",cmd->cmd);
			
		break;
	}	
	return 0;
}



