#include "ipmain.h"
#include "ipmain_para.h"
#include "leds_api.h"
#include "devstat.h"
#include "netcmdproc.h"
#include "process_kbd.h"

static DWORD old_state=0;//键盘状态

int set_keyboard_alarm(int mode,int ch,int flag)
{
	struct alarm_motion_struct *alarm_motion;
	struct ipmain_para_struct *mainpara;
	struct alarm_trigin_struct *trigin;

	if(ch>=get_video_num()+get_trigin_num())
		return -1;
	
	mainpara=get_mainpara();
	alarm_motion=&mainpara->alarm_motion;
	if(mode==0)//外部
		trigin=&alarm_motion->trigin[ch];
	else 
		{
			if(mode==1)
		   		trigin=&alarm_motion->motion[ch];
				
		 	else 
		 		return -1;
		}
	trigin->setalarm=flag;

	//根据具体情况日志
	if(flag==1)
		{
			if(mode==0)
				gtloginfo("第%d路外部侦测布防\n",ch);
			else
				gtloginfo("第%d路移动侦测布防\n",ch);
		}
	else
		{
			if(mode==0)
				gtloginfo("第%d路外部侦测撤防\n",ch);
			else
				gtloginfo("第%d路移动侦测撤防\n",ch);
		}
	return 0;
}

static int process_keyboard_state(mod_socket_cmd_type *cmd)
{
	DWORD *state,change;
	struct keyboard_state_struct *newstate,*change_state;
	struct ip1004_state_struct *ip1004;
	struct per_state_struct *per_state;
	
	if(cmd==NULL)
		return -1;
	if(cmd->cmd!=KEYBOARD_STAT_RETURN)
		return -1;

	
	state=(DWORD*)&cmd->para[sizeof(pid_t)];
	printf("recv a keyboard KEYBOARD_STAT_RETURN cmd state=0x%08x\n",(int)(*state));
	if(*state!=old_state)
	{		
		gtloginfo("recv a keyboard KEYBOARD_STAT_RETURN cmd state=0x%08x\n",(int)(*state));
		change=*state^old_state;
		newstate=(struct keyboard_state_struct *)state;
		change_state=(struct keyboard_state_struct *)(&change);
		
		if(change_state->connect_fail)
		{
			//wsyadd,点灯并报状态
			ip1004=get_ip1004_state(0);
			per_state=&ip1004->reg_per_state;
			per_state->keyboard_err=newstate->connect_fail;
			//gtloginfo("键盘状态改变，发送给网关并点error灯\n");
			send_dev_state(-1,1,1,0,0,0);
			set_error_led_state();
			
			if(newstate->connect_fail)
				{
					gtloginfo("keyboard 连接异常 \n");
				}
			else
				{
					gtloginfo("keyboard连接正常 \n");
				}
		}
		old_state=*state;
	}
	return 0;
}
//
static int process_alarm_set(mod_socket_cmd_type *cmd)
{
	struct keyboard_alarm_set_struct *alarm;
	int ch;
	if(cmd==NULL)
		return -1;
	if(cmd->cmd!=KEYBOARD_ALARM_SET)
		return -1;
	alarm=(struct keyboard_alarm_set_struct *)cmd->para;
	gtloginfo("recv a keyboard KEYBOARD_ALARM_SET cmd mode=%d channel=%d flag=%d\n",alarm->mode,alarm->channel,alarm->flag);
	//wsy add,根据命令内容进行设置
	if(alarm->channel==-1) //所有
		{
			if(alarm->mode==0)
				{
					for(ch=0;ch<get_trigin_num();ch++)
						set_keyboard_alarm(alarm->mode,ch,alarm->flag);
				}
			else
				{
					for(ch=0;ch<get_video_num();ch++)
						set_keyboard_alarm(alarm->mode,ch,alarm->flag);
				}
			return 0;
		}
	else
		{		
			return set_keyboard_alarm(alarm->mode,alarm->channel,alarm->flag);
		}
}

/**********************************************************************************************
 * 函数名	:process_keyboard_cmd()
 * 功能	:处理键盘模块发来的命令
 * 输入	:cmd:接收到的键盘模块的命令缓冲区
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int process_keyboard_cmd(mod_socket_cmd_type *cmd)
{
	if(cmd==NULL)
		return -1;
	switch(cmd->cmd)
	{

		case KEYBOARD_STAT_RETURN:
			process_keyboard_state(cmd);
		break;
		case KEYBOARD_ALARM_SET:
			process_alarm_set(cmd);
		break;

		default:
			printf("recv a unknow keyboard cmd:%04x \n",cmd->cmd);
			gtloginfo("recv a unknow keyboard cmd:%04x \n",cmd->cmd);
			
		break;
	}	
	return 0;
}


