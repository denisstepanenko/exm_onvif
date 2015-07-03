
//wsy注:采用编译开关的方式选择是走以前的消息队列还是走udp-socket实现
//函数调用和接口保持不变。

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <mod_com.h>
#include <mod_cmd.h>
#include <mod_socket.h>
#include <string.h>
#include "send_state.h"
#include "gtlog.h"




#ifndef DWORD
	#define DWORD unsigned long
#endif

static int send_cmd_ch=-1,recv_cmd_ch=-1;//发送命令和接收命令的通道号(从本模块的角度看)
static int	com_fd= -1; //发送和接收命令的udp socket

static pthread_t recv_modsocket_thread_id = -1;
	

static int current_state = PPPOE_SUCCESS;  //保存状态

/*
 *初始化与主进程通讯的命令通道
 * 返回值:0 表示发送成功
 * 负值:    表示出错
*/

int init_com_channel(void)
{
	send_cmd_ch=mod_com_init(MAIN_RECV_CMD_CHANNEL,MSG_INIT_ATTRIB);
	recv_cmd_ch=mod_com_init(MAIN_SEND_CMD_CHANNEL,MSG_INIT_ATTRIB);
	if((send_cmd_ch<0)||(recv_cmd_ch<0))
	{
		return -1;
	}
	
	com_fd	=	mod_socket_init(0,0);	
	return 0; 
}

//发送命令到主进程
//len:mod_com_type结构中 para字段的有效信息长度
static int send_main_cmd(struct mod_com_type *send,int len)
{
	send->target=MAIN_PROCESS_ID;
	send->source=PPPOE_WATCH_ID;
	send->cmdlen=len+2;
	return mod_com_send(send_cmd_ch,send,0);
}

/*
 * 发送状态给主进程 
 * 参数:stat:要发送的状态
 * 返回值:0 表示发送成功
 * 负值:    表示出错
*/
int send_pppoe_stat2main(int stat)
{
	
	DWORD socketbuf[20];
	mod_socket_cmd_type *cmd;
	
	int sendstat;
	DWORD *state;
	pid_t *pid;
	DWORD buffer[10];
	struct mod_com_type *send;
	
	current_state = stat;
	sendstat=stat;
	send=(struct mod_com_type *)buffer;
	pid=(pid_t*)send->para;
	state=(DWORD*)&send->para[sizeof(pid_t)];
	send->cmd=PPPOE_STATE_RETURN;
	*state=sendstat;
	*pid=getpid();
	send_main_cmd(send,sizeof(pid_t)+sizeof(DWORD));
	
	cmd=(mod_socket_cmd_type *)socketbuf;
	cmd->cmd	=	PPPOE_STATE_RETURN;
	cmd->len	=	4+sizeof(pid_t);
	pid=(pid_t*)cmd->para;
	*pid=getpid();
	state=(DWORD*)&cmd->para[sizeof(pid_t)];
	*state=sendstat;
	mod_socket_send(com_fd,MAIN_PROCESS_ID,PPPOE_WATCH_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);

	return 0;
	
}

//监听来自主模块的查询并发送相关状态
static int process_modsocket_cmd(int sourceid , mod_socket_cmd_type *modsocket)
{	
	printf("pppoe_watch recved a module-cmd from id %d\n",sourceid);
	switch (sourceid)
	{
		case MAIN_PROCESS_ID:	
			if(modsocket->cmd == MAIN_QUERY_STATE) //查询状态
				send_pppoe_stat2main(current_state);
		break;
		default:
		break;
	}
	return 0;
}


void *recv_modcom_thread (void *data)
{
	
	int ret;
	char buf[MAX_MOD_CMD_LEN];
	struct mod_com_type *recv;

	printf("pppoe_watch start recv_modcom_thread!\n");
	gtloginfo("pppoe_watch start recv_modcom_thread!\n");
	
	recv=(struct mod_com_type*)(buf);	
	while(1)
	{
		
		ret=mod_com_recv(recv_cmd_ch,PPPOE_WATCH_ID,recv,MAX_MOD_CMD_LEN,0);
		if(ret>0)
		{
			if(recv->cmd == MAIN_QUERY_STATE) //查询状态
			{
				gtloginfo("recv MAIN_QUERY_STATE cmd!\n");
				send_pppoe_stat2main(current_state);	
			}
		}
	}
	return NULL;
}

int creat_recv_modsocket_thread(void)
{
	return creat_modsocket_thread(&recv_modsocket_thread_id,com_fd,PPPOE_WATCH_ID,"pppoe_watch", process_modsocket_cmd);
}	


