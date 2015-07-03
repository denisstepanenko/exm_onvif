#include "ipmain.h"
#include "mod_cmd.h"
#include "leds_api.h"
#include "mod_com.h"
#include "stdio.h"
#include "ipmain_para.h"
#include "process_pppoe_watch_cmd.h"

/**********************************************************************************************
 * 函数名	:process_pppoe_watch_cmd()
 * 功能	:处理pppoe_watch模块发来的命令
 * 输入	:cmd:接收到的pppoe_watch模块的命令缓冲区
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int process_pppoe_watch_cmd(mod_socket_cmd_type *cmd)
{
	DWORD *state;
	in_addr_t addr;
	printf("ipmain recv a pppoe_watch cmd\n");
	gtloginfo("ipmain收到一个pppoe模块命令0x%04x\n",cmd->cmd);
	state=(DWORD*)&cmd->para[sizeof(pid_t)];
	switch(*state)
	{
		case PPPOE_SUCCESS:	//adsl正常连接
			gtloginfo("pppoe发来命令告知adsl现正常连接\n");

			refresh_netinfo();			
			if(get_current_netled()< NET_ADSL_OK)
			{
				//gtloginfo("pppoe,之前netled为%d,现在为adslok\n",get_current_netled());
				//set_net_led_state(NET_ADSL_OK);
				#ifdef USE_LED
					set_net_led_state(0);
				#endif
			}
			break;
		case PPPOE_NO_MODEM:		//找不到adsl modem
			gtloginfo("pppoe发来命令告知找不到adsl modem\n");
			if(get_current_netled()< NET_GATE_CONNECTED)
			{
				addr=get_net_dev_ip("ppp0");
				if((int)addr==-1)//
				{
					#ifdef USE_LED
						set_net_led_state(0);
					#endif
					//set_net_led_state(NET_NO_MODEM);
				}
			}
			break;
		case PPPOE_PASSWD_ERR:		//adsl帐号密码错误
			gtloginfo("pppoe发来命令告知adsl账号密码有错\n");
			if(get_current_netled()< NET_GATE_CONNECTED)
			{
				addr=get_net_dev_ip("ppp0");
				if((int)addr==-1)//
				{
								#ifdef USE_LED
					set_net_led_state(0);
				#endif
					//set_net_led_state(NET_INVAL_PASSWD);
				}
			}
			break;
		case PPPOE_USR_TWICE:		//帐号重复登入
			gtloginfo("pppoe发来命令告知adsl账号重复登入\n");
			if(get_current_netled()< NET_GATE_CONNECTED)
			{
				addr=get_net_dev_ip("ppp0");
				if((int)addr==-1)//
				{
								#ifdef USE_LED
					set_net_led_state(0);
				#endif
					//set_net_led_state(NET_LOGINED);
				}
			}
			break;
		case PPPOE_USR_INVALID:		//帐号无效
			gtloginfo("pppoe发来命令告知	adsl账号无效\n");	
			if(get_current_netled()< NET_GATE_CONNECTED)
			{
				addr=get_net_dev_ip("ppp0");
				if((int)addr==-1)//
				{
								#ifdef USE_LED
					set_net_led_state(0);
				#endif
				//	set_net_led_state(NET_INVAL_USR);	
				}
			}
			break;
		case PPPOE_PAP_FAILED:		//用户名密码验证失败
			gtloginfo("pppoe发来命令告知	adsl用户名密码验证失败\n");	
			if(get_current_netled()< NET_GATE_CONNECTED)
			{
				addr=get_net_dev_ip("ppp0");
				if((int)addr==-1)//
				{
									#ifdef USE_LED
					set_net_led_state(0);
				#endif
					//set_net_led_state(NET_PAP_FAILED);	
				}
			}
			break;
		default:
			printf("ipmain recv a unknow state:%04x from pppoe\n",(int)*state);
		break;
	}
	return 0;


}
