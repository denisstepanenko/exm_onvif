
#include "ipmain.h"
#include "hdmodapi.h"
#include "maincmdproc.h"
#include "devstat.h"
#include "netcmdproc.h"
#include "mod_socket.h"
#include "devinfo.h"



int process_hdmod_state(mod_socket_cmd_type *cmd)
{
	int i;

	static DWORD old_state=0;
	DWORD *state,change;
	struct ip1004_state_struct * gtstate;
	struct hdmod_state_struct *newstate,*change_state;
	if(cmd==NULL)
		return -1;
	state=(DWORD*)&cmd->para[sizeof(pid_t)];
	gtloginfo("recv a hdmodule HDMOD_STATE_RETURN cmd state=0x%08x\n",(int)*state);

	if(*state!=old_state)
	{		
		change=*state^old_state;
		newstate=(struct hdmod_state_struct *)state;
		change_state=(struct hdmod_state_struct *)change;
		for(i=0;i<virdev_get_virdev_number();i++)
		{
			gtstate=get_ip1004_state(i);
			pthread_mutex_lock(&gtstate->mutex);
			gtstate->reg_dev_state.cf_err=newstate->cf_err;
			send_dev_state(-1,1,0,0,0,i);
		}

		/*gtstate->reg_dev_state.video_enc1_err=newstate->video_enc1_err;
		gtstate->reg_dev_state.video_enc2_err=newstate->video_enc2_err;
		gtstate->reg_dev_state.video_enc3_err=newstate->video_enc3_err;
		gtstate->reg_dev_state.video_enc4_err=newstate->video_enc4_err;
		*///gtstate->reg_dev_state.cf_err=newstate->cf_err;
		//gtstate->reg_per_state.disk_full=newstate->disk_full;
		pthread_mutex_unlock(&gtstate->mutex);
		//gtloginfo("视频丢失，芯片故障，发送状态\n");

		
		old_state=*state;
	}
	return 0;
}


/**********************************************************************************************
 * 函数名	:process_hdmod_cmd()
 * 功能	:处理hdmodule模块发来的命令
 * 输入	:cmd:接收到的hdmodule模块的命令缓冲区
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int process_hdmod_cmd(mod_socket_cmd_type *cmd)
{
	if(cmd==NULL)
		return -1;
	switch(cmd->cmd)
	{

		case MOD_BYPASSTO_GATE_CMD:
			printf("recv hdmodule MOD_BYPASSTO_GATE_CMD\n");
			gtloginfo("recv hdmodule MOD_BYPASSTO_GATE_CMD\n");
			bypass2gate(cmd);
		break;
		case HDMOD_STATE_RETURN:
			process_hdmod_state(cmd);
		break;
		case MOD_BYPASSTO_GATE_ACK:
			printf("recv hdmodule MOD_BYPASSTO_GATE_ACK\n");
			gtloginfo("recv hdmodule MOD_BYPASSTO_GATE_ACK\n");
			process_gate_cmd_ack(cmd);
		break;
		default:
			printf("recv a unknow hdmodule cmd:%04x \n",cmd->cmd);
			gtloginfo("recv a unknow hdmodule cmd:%04x \n",cmd->cmd);
			
		break;
	}	
	return 0;
}


