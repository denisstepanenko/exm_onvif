#include "ipmain.h"
#include "mod_cmd.h"
#include "leds_api.h"
#include "mod_com.h"
#include "stdio.h"
#include "ipmain_para.h"
#include "devstat.h"
#include "netcmdproc.h"
#include "process_upnpd_cmd.h"
#include "devinfo.h"

/**********************************************************************************************
 * 函数名	:process_upnpd_cmd()
 * 功能	:处理upnpd模块发来的命令
 * 输入	:cmd:接收到的upnpd模块的命令缓冲区
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int process_upnpd_cmd(mod_socket_cmd_type *cmd)
{
	DWORD *state;
	struct ip1004_state_struct *gtstate;
	printf("vsmain recv a upnpd cmd\n");
	gtloginfo("vsmain收到一个upnpd模块命令0x%04x\n",cmd->cmd);
	state=(DWORD*)&cmd->para[sizeof(pid_t)];

	int i;
	
	switch(*state)
	{
		case UPNPD_SUCCESS:	//upnpd端口映射成功
			
			gtloginfo("upnpd发来命令告知端口正常映射\n");
			for(i=0;i<virdev_get_virdev_number();i++)
			{
				gtstate = get_ip1004_state(i);
				pthread_mutex_lock(&gtstate->mutex);
				gtstate->reg_per_state.upnp_err = 0;
				pthread_mutex_unlock(&gtstate->mutex);
				send_dev_state(-1,1,0,0,0,i);
			}
		break;
		case UPNPD_FAILURE:		//upnpd端口映射失败
			gtloginfo("upnpd发来命令告知端口映射失败\n");
			for(i=0;i<virdev_get_virdev_number();i++)
			{
				gtstate = get_ip1004_state(i);
				pthread_mutex_lock(&gtstate->mutex);
				gtstate->reg_per_state.upnp_err = 1;
				pthread_mutex_unlock(&gtstate->mutex);
				send_dev_state(-1,1,0,0,0,i);
			}
		break;
		default:
			printf("vsmain recv a unknow state:%04x from upnpd\n",(int)*state);
		break;
	}
			
	return 0;


}
