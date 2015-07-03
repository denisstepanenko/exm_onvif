/*	模块间通讯的函数库
 *		shixin
 *		 2005.1
 *    本函数库提供了实现各模块间通讯需要的最底层接口，实质上是对linux提供的消息队列进行了包装
 *    上层的应用接口及数据结构不属于本库的实现范围
 */
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include "mod_com.h"
#include <errno.h>
#include <syslog.h>
/**********************************************************************************************
 * 函数名	:mod_com_init()
 * 功能	:建立一个命令通道，如果该通道已存在，则挂在这个通道上，返回建立通道的id 
 * 输入	:key:  希望创建的命令通道的关键字,多个关联到相同命令通道的模块需要使用相同的key
 *			  flag: 创建命令通道的一些控制属性,写MSG_INIT_ATTRIB即可
 * 返回值	:负值表示失败
 *			  正整数  创建或挂接的命令通道id号，以后通过这个号来向命令通道发送或接收命令      
 **********************************************************************************************/
int mod_com_init(key_t key,int flag)
{
    int queue_id;
    queue_id = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
    if (queue_id <0 ) {//表示已经创建了该键值的消息队列
        queue_id=msgget(key,0);
        if(queue_id <0 )
        {				//表示打开消息队列失败
            printf("mod_com_init can't create message:0x%x!\n",key);
            return -1;
        }
    }
    return queue_id;    
}

/**********************************************************************************************
 * 函数名	:mod_com_send()
 * 功能	:向指定命令通道发送一个命令信息
 * 输入	:com_id:命令通道的id,由调用 'mod_com_init()' 的返回值得到
 *	  		 send:指向要发送的命令的缓冲区的指针(已经填充好target，source，len，msg字段)
 *			  flag:需要以什么属性来执行本函数,如果是IPC_NOWAIT则不管命令发出与否,
 * 	      			  函数马上返回,如果是0表示如果没有发出命令则进程阻塞
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int  mod_com_send(int com_id,struct mod_com_type *send,int flag)
{
	int rc;
	if(com_id<0)
	{
		printf("get a error comid:%d\n",com_id);
		return -1;
	}
	//rc = msgsnd(com_id,send, send->cmdlen+8, MSG_NOERROR|IPC_NOWAIT|flag);//消息长度如果大于msgsz字节则被截掉
	rc = msgsnd(com_id,send, send->cmdlen+sizeof(struct mod_com_type)-sizeof(send->para)-4, MSG_NOERROR|IPC_NOWAIT|flag);//消息长度如果大于msgsz字节则被截掉

	return rc;
}

/**********************************************************************************************
 * 函数名: mod_com_recv()
 * 功能:  从指定的命令通道和命令地址中接收一个数据包

 * 输入:
 *         com_id:命令通道的id,由调用 'mod_com_init()' 的返回值得到
 *         addr:要从命令通道中获取的数据包的地址,如果为0会接收到发送给全部目标地址的消息,本信息在
 *                调用时一般是事先约定好的功能块的地址
 *         recv:指向要存放接收数据的缓冲区,缓冲区应该是一个能容纳最长命令的buffer
 *         buf_len:接收的消息的最大长度,一般写一个比较大的值如缓冲区大小
 *         flag:MSG_NOERROR 表示如果接收到的数据长度大于buf_len则截断消息,并且不通知发送者,一般
 *                          应用时应加上这个标记,测试时可以不用这个标记
 *              IPC_NOWAIT  表示如果没有接收到消息也可以立即返回
 *              这两个标记可以一起使用(MSG_NOERROR|IPC_NOWAIT)
 * 返回值:
 *         正值:接收到的信息的字节数(去掉target,source两个字段后)
 *         负值:出错
 ************************************************************************************************/
int  mod_com_recv(int com_id,long int addr,struct mod_com_type *recv,int buf_len,int flag)
{
	char msg[50] = {0};
	int rc;
	if(com_id<0)
	{
		printf("mod_com_recv get a error com_id:%d\n",com_id);
		return -EINVAL;
	}
	recv->target=addr;
	rc = msgrcv(com_id, recv, buf_len, addr, flag|MSG_NOERROR);
	if(rc>0)
		return (rc-8);
	else
		return -errno;
}


//例:UPDATE进程把接收到的命令发送回去
/*
char rec_cmd_buf[MAX_MOD_CMD_LEN];
char send_cmd_buf[MAX_MOD_CMD_LEN];
void main(void)
{
	int main_send_ch,main_recv_ch;
	int rec_len,ret;
	struct mod_com_type *recv,*send;
	main_send_ch=mod_com_init(MAIN_SEND_CMD_CHANNEL,MSG_INIT_ATTRIB);
	if(main_send_ch<0)
	{
		//出错处理
	}
	main_recv_ch=mod_com_init(MAIN_RECV_CMD_CHANNEL,MSG_INIT_ATTRIB);
	if(main_recv_ch<0)
	{
		//出错处理
	}
	while(1)
	{
		rec_len=mod_com_recv(main_send_ch,UPDATE_PROCESS_ID,(struct mod_com_type *)(rec_cmd_buf),MAX_MOD_CMD_LEN,MSG_NOERROR);//如果没有收到命令,程序应该是阻塞在这里的
		if(rec_len >0)//表示收到了一个完整命令
		{
			recv=(struct mod_com_type *)(rec_cmd_buf)；
			memcpy(send_cmd_buf,rec_cmd_buf,rec_len);
			send=(struct mod_com_type *)(send_cmd_buf)；
			send->target=recv->source;
			send->source=UPDATE_PROCESS_ID;
			ret=mod_com_send(main_recv_ch,send,rec_len,IPC_NOWAIT);//直接返回，不阻塞
			if(ret<0)
			{
				//发送出错
			}			
		}
		else
		{
			//可能是程序有bug
		}
	}
	
}

*/


