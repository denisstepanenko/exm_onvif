#include "ipmain.h"
#include "maincmdproc.h"
#include "devstat.h"
#include "netcmdproc.h"
#include "devinfo.h"
//#include "process_diskman.h"




int process_diskman_state(mod_socket_cmd_type *cmd)
{
	int i;
	static DWORD old_state=0;
	DWORD *state,change;
	struct ip1004_state_struct * gtstate;
	struct diskman_state_struct *newstate,*change_state;
	if(cmd==NULL)
		return -1;
	state=(DWORD*)&cmd->para[sizeof(pid_t)];
	gtloginfo("recv a diskman DISKMAN_STATE_RETURN cmd state=0x%08x\n",(int)*state);

	if(*state!=old_state)
	{		
		change=*state^old_state;
		newstate=(struct diskman_state_struct *)state;
		change_state=(struct diskman_state_struct *)change;
		for(i=0;i<virdev_get_virdev_number();i++)
		{	
			gtstate=get_ip1004_state(i);
			pthread_mutex_lock(&gtstate->mutex);		
			gtstate->reg_dev_state.cf_err=newstate->cf_err;
			gtstate->reg_per_state.disk_full=newstate->disk_full;
			pthread_mutex_unlock(&gtstate->mutex);
			send_dev_state(-1,1,0,0,0,i);
		}
		old_state=*state;
	}
	return 0;
}

/**********************************************************************************************
 * 函数名	:process_diskman_cmd()
 * 功能	:处理diskman模块发来的命令
 * 输入	:cmd:接收到的diskman模块的命令缓冲区
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int process_diskman_cmd(mod_socket_cmd_type *cmd)
{
	if(cmd==NULL)
		return -1;
	switch(cmd->cmd)
	{
	
		case MOD_BYPASSTO_GATE_ACK:
			printf("recv diskman MOD_BYPASSTO_GATE_ACK\n");
			gtloginfo("recv diskman MOD_BYPASSTO_GATE_ACK\n");
			process_gate_cmd_ack(cmd);
		break;
		case DISKMAN_STATE_RETURN:
			process_diskman_state(cmd);
		break;
	
		default:
			printf("recv a unknow diskman cmd:%04x \n",cmd->cmd);
			gtloginfo("recv a unknow diskman cmd:%04x \n",cmd->cmd);
			
		break;
	}	
	return 0;
}


