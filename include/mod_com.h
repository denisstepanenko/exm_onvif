/*	模块间通讯的函数库
 *		shixin
 *		 2005.1
 *    本函数库提供了实现各模块间通讯需要的最底层接口，实质上是对linux提供的消息队列进行了包装
 *    上层的应用接口及数据结构不属于本库的实现范围
 */
#ifndef MOD_COM_H
#define MOD_COM_H
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include "mod_cmd.h"					//模块间通讯的命令字定义
#include "mod_socket.h"
#define		RECV_MOD_CMD_ALL	0	//接收所有目标地址的消息


#ifndef FOR_PC_MUTI_TEST
#define 		MAIN_SEND_CMD_CHANNEL	0x6060		//主进程发送命令的通道，其他进程应该在这个通道上侦听发给自己的命令
#define 		MAIN_RECV_CMD_CHANNEL	0x6061		//主进程接收命令的通道，其他进程应该将发送给主进程的信息发送到这个通道上

#define		GATE_RECV_CHANNEL		0x6066  		//send msg to remote gateway,used only by vsmain 
#define 	GATE_RECV_CHANNEL_DEV1	0x6067			//供虚拟设备使用的，其他同上								
#else
extern int		MAIN_SEND_CMD_CHANNEL;				//主进程发送命令的通道，其他进程应该在这个通道上侦听发给自己的命令
extern int 	MAIN_RECV_CMD_CHANNEL;				//主进程接收命令的通道，其他进程应该将发送给主进程的信息发送到这个通道上

extern int 	GATE_RECV_CHANNEL;			 		//send msg to remote gateway,used only by vsmain 

#endif



#define		MAX_MOD_CMD_LEN		1024			//模块间通讯的命令最大长度(缓冲区长度,应该不会有超过此值的命令,除非是程序bug)


#define 	MSG_INIT_ATTRIB			0				//初始化命令通道的时候，flag用这个定义


/**********************************************************************************************
 * 命令字结构,在实际应用中由于各种命令的参数个数不同,所以使用本
 * 结构定义的时候参数个数只是2,
 * 使用时需要将命令缓冲区强制转换成指向本结构的指针以便使用
 **********************************************************************************************/
struct mod_com_type
{
    long int target;                     	//命令的目标模块地址
    long int source;			 	//发送命令的模块的地址
    unsigned short  env;			//加密算法，在和网关通讯相关时有意义
    unsigned short  enc;			//同上
    unsigned short cmdlen;		//命令的长度，以字节为单位，包括cmdlen,cmd和参数
    unsigned short cmd;			//命令字
    unsigned char  para[2];   	//命令的参数
};


/**********************************************************************************************
 * 函数名	:mod_com_init()
 * 功能	:建立一个命令通道，如果该通道已存在，则挂在这个通道上，返回建立通道的id 
 * 输入	:key:  希望创建的命令通道的关键字,多个关联到相同命令通道的模块需要使用相同的key
 *			  flag: 创建命令通道的一些控制属性,写MSG_INIT_ATTRIB即可
 * 返回值	:负值表示失败
 *			  正整数  创建或挂接的命令通道id号，以后通过这个号来向命令通道发送或接收命令      
 **********************************************************************************************/
int mod_com_init(key_t key,int flag);

/**********************************************************************************************
 * 函数名	:mod_com_send()
 * 功能	:向指定命令通道发送一个命令信息
 * 输入	:com_id:命令通道的id,由调用 'mod_com_init()' 的返回值得到
 *	  		 send:指向要发送的命令的缓冲区的指针(已经填充好target，source，len，msg字段)
 *			  flag:需要以什么属性来执行本函数,如果是IPC_NOWAIT则不管命令发出与否,
 * 	      			  函数马上返回,如果是0表示如果没有发出命令则进程阻塞
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int  mod_com_send(int com_id,struct mod_com_type *send,int flag);

/**********************************************************************************************
 * 函数名: mod_com_recv()
 * 功能:  从指定的命令通道和命令地址中接收一个命令包
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
int  mod_com_recv(int com_id,long int addr,struct mod_com_type *recv,int buf_len,int flag);



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
		rec_len=mod_com_recv(main_send_ch,UPDATE_PROCESS_ID,(struct mod_com_type *)(rec_cmd_buf),MSG_NOERROR);//如果没有收到命令,程序应该是阻塞在这里的
		if(rec_len >0)//表示收到了一个完整命令
		{
			recv=(struct mod_com_type *)(rec_cmd_buf)；
			memcpy(send_cmd_buf,rec_cmd_buf,rec_len);
			send=(struct mod_com_type *)(send_cmd_buf)；
			send->target=recv->source;
			send->source=UPDATE_PROCESS_ID;
			send->cmdlen=recv->cmdlen;//发送数据时，应用程序需要填充命令长度字段			
			ret=mod_com_send(main_recv_ch,send,IPC_NOWAIT);//直接返回，不阻塞
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

#endif
