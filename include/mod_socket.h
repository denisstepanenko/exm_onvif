/*	模块间采用基于udp的socket通讯的函数库
 *		wsy
 *		 2007.9
 *    本函数库提供了实现各模块间采用基于udp的socket通讯需要的最底层接口
 *    上层的应用接口及数据结构不属于本库的实现范围
 */
#ifndef MOD_SOCKET_H
#define MOD_SOCKET_H
#include <sys/types.h> 
//#include <sys/ipc.h> 
//#include <sys/msg.h> 
#include "typedefine.h"
#include "mod_cmd.h"
#include "gtthread.h"


#ifndef IN
	#define	IN
#endif

#ifndef OUT
	#define	OUT
#endif

#ifndef INOUT
	#define INOUT
#endif

#ifndef IO
	#define	IO
#endif

typedef struct {
	long	gatefd;	//网关fd，若<=0表示不需要发回给网关
	unsigned short 	dev_no;	//设备编号，用于寻找网关
	unsigned short	env;	//签名
	unsigned short	enc;	//加密
	
}gateinfo;

typedef struct {	
	gateinfo	gate;		//和网关相关的信息
	unsigned short	cmd;	//命令
	unsigned short 	len;	//参数(para)长度
	unsigned char	para[4];//参数,具体定义随命令定义不同而不同 	
}mod_socket_cmd_type;    //模块间命令的结构

#define MAX_MODULE_NAME_LEN		16//模块名称的最大长度，暂定

typedef struct {
	int		mod_id;				//模块的ID，见mod_cmd.h
	int		com_fd;				//模块的modsocket通道id
	char	module_name[MAX_MODULE_NAME_LEN];	//模块名称，如"diskman"
	int 	(*fn)(int sourceid, mod_socket_cmd_type *modsocket);//回调函数指针
}mod_socket_thread_data;//用于各模块创建侦听mod_socket命令的线程的数据


#define		MAX_MOD_SOCKET_CMD_LEN		2048			//模块间通讯的命令最大长度(缓冲区长度,应该不会有超过此值的命令,除非是程序bug)

/**********************************************************************************************
 * 函数名	:mod_socket_init()
 * 功能	:	在指定的端口建立并绑定udp socket，并返回之
 * 输入	:	send_flag,	发送数据时使用的属性flags,一旦设定,适用于整个进程
 *			recv_flag,	接受数据时使用的属性flags
 * 返回值	:负值表示失败
 *			  正整数  产生的文件描述符，以后通过它来发送或接收命令      
 **********************************************************************************************/
int mod_socket_init(IN int send_flag, IN int recv_flag);

/**********************************************************************************************
 * 函数名	:mod_socket_send()
 * 功能	:向指定的主机地址发送一个命令信息
 * 输入	:com_fd:由调用 'mod_socket_init()' 的返回值得到
 *		 target:目标模块的id，若为0则表示发送给所有模块
 *		 source:发送模块的id
 *		 cmdbuf:指向要发送的命令的缓冲区的指针(已经填充好相关信息)
 *		 cmdlen:缓冲区中有效数据的长度
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int mod_socket_send(IN int com_fd,IN int target,IN int source,IN void *cmdbuf,IN int cmdlen);

/**********************************************************************************************
 * 函数名: mod_socket_recv()
 * 功能:  从socket中接收一个命令包
 * 输入:
 *        com_fd:由调用 'mod_socket_init()' 的返回值得到
 *		 myid:本模块的id
 *		 source: 发来命令的模块id
 *		 cmdbuf:指向要发送的命令的缓冲区的指针(已经填充好相关信息)
 *		 cmd_maxlen:缓冲区的最大长度
 *		 source_addr:存放发送模块的ip，若为NULL表示不用取
 * 返回值:
 *         正值:接收到的信息的字节数
 *         负值:出错
 ************************************************************************************************/
int mod_socket_recv(IN int com_fd, IN int myid, OUT int *source, OUT void *cmdbuf,IN int cmd_maxlen, OUT char *source_addr);

int send_ack_to_main(int com_fd, int mod_id, int cmd, int result, gateinfo *gate);

/**********************************************************************************************
 * 函数名: mod_socket_req_recv()
 * 功能:  从socket中接收一个命令包
 * 输入:
 *          com_fd:由调用 'mod_socket_init()' 的返回值得到
 *		 req_id:输入时是本模块的id,输出时表示接收到的数据包的目的id
 *		 source: 发来命令的模块id
 *		 cmdbuf:指向要发送的命令的缓冲区的指针(已经填充好相关信息)
 *		 cmd_maxlen:缓冲区的最大长度
 *		 source_addr:存放发送模块的ip，若为NULL表示不用取
 * 返回值:
 *         正值:接收到的信息的字节数
 *         负值:出错
 ************************************************************************************************/
int mod_socket_req_recv(IN int com_fd, INOUT int *req_id, OUT int *source, OUT void *cmdbuf,IN int cmd_maxlen, OUT char *source_addr);


int creat_modsocket_thread(pthread_t *thread_id, int com_fd, int mod_id, char *mod_name, int (*fn)(int sourceid, mod_socket_cmd_type *modsocket));
#endif
