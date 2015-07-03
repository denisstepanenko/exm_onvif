//GT1000系统内部通讯(udp-socket)的命令字
#ifndef MOD_SOCKET_CMD_H
#define MOD_SOCKET_CMD_H
#include <typedefine.h>
#include "mod_socket.h"
#include "gt_com_api.h"

/************************************************************************************
	主进程发送给其它进程的命令
************************************************************************************/
//主进程查询其它进程的状态,这个命令什么参数也没有
#define	S_MAIN_QUERY_STATE			0x1001


typedef struct {
	unsigned short	env;	//签名
	unsigned short	enc;	//加密
	unsigned short	cmd;	//命令
	unsigned short 	len;	//参数长度
	unsigned char	para[4];//参数,具体定义随命令定义不同而不同 	
}mod_socket_cmd_type;    //模块间命令的结构

/**********************************************************************************
	其他进程发送给主进程的命令
***********************************************************************************/

/*实时音视频传送模块向主进程返回自己的状态,状态标志位的意义由实时图像模块确定,0表示正常，1表示异常
 *当实时图像模块自己发现状态变化时发送此命令，或者当接收到主进程发来的MAIN_QUERY_STATE命令时发送此命令 
 *格式:S_RTSTREAM_STATE_RETURN(2)+len(2)+state(4)
 *state:实时图像模块的状态，如果全0表示一切正常，具体位的意义由实时图像模块确定
 */
#define S_RTSTREAM_STATE_RETURN		0x0301
struct s_rtimage_state_struct{
	unsigned		mod_state		:1;	//模块状态0表示正常 1表示异常,此位由主模块设置
	unsigned		net_enc_err		:1;	//网络的压缩芯片故障
	unsigned 		net_enc_busy	:1;  //网络编码芯片忙标志
	unsigned 		test_d1_flag		:1; //测试d1通道标志
	unsigned		reserve			:28;
};

/**********************************************************************************
 *	网络监控进程相关命令
***********************************************************************************/

/*
 *	PPPOE监控进程发送给主进程状态
 *    格式:S_PPPOE_STATE_RETURN(2)+len(2)+state(4)
 *    state:PPPOE监控模块的状态，

*/
#define S_PPPOE_SUCCESS		0	//adsl正常连接
#define S_PPPOE_NO_MODEM	1	//找不到adsl modem
#define S_PPPOE_PASSWD_ERR	2	//adsl帐号密码错误
#define S_PPPOE_USR_TWICE	3	//帐号重复登入
#define S_PPPOE_USR_INVALID	4	//帐号无效
#define S_PPPOE_PAP_FAILED	5	//另种帐号无效

#define S_PPPOE_STATE_RETURN		0x0501


/**********************************************************************************
 *	upnp端口映射进程相关命令
***********************************************************************************/

/*
 *	upnp端口映射进程发送给主进程状态
 *    格式:S_UPNPD_STATE_RETURN(2)+len(2)+state(4)
 *    state:UPNPD监控模块的状态，

*/
#define S_UPNPD_SUCCESS		0	//upnp端口正常映射
#define S_UPNPD_FAILURE		1	//upnp映射失败

#define S_UPNPD_STATE_RETURN		0x0c01





/*
	转发命令，此类命令用于主模块将远程网关发来的相关命令直接转发到相应模块
	以及将相应模块需要返回给网关的命令发送出去
*/
//实际转发时，第一个字段应该是4字节的文件描述符，表示收到命令的连接
//主模块发送的命令
/*主模块将网关发来的信息原样包含在模块内部通讯的消息体里发送给相应模块*/
/*格式
	S_GATE_BYPASSTO_MOD_CMD(2)+len(2)+gate_fd(4)+cmd_pkt(len-2-4)
	gate_fd:主进程用于标识网关的文件描述符，其它进程只要把这个描述符原样发回就可以了
*/
#define  S_GATE_BYPASSTO_MOD_CMD		0x1000//网关发来的命令如果主模块自己处理不了则转发给相应的模块处理

//模块发送的命令
/*模块将需要发送给网关的返回消息包含在消息体内，发给主进程，由主进程转发给网关*/
/*格式
	S_MOD_BYPASSTO_GATE_CMD(2)+len(2)+gate_fd(4)+cmd_pkt(len-2-4)
	gate_fd:主进程发送GATE_BYPASSTO_MOD_CMD命令时带的gate_fd参数，其它模块需要在返回信息的时候原样填充这个字段
*/
#define  S_MOD_BYPASSTO_GATE_CMD		0x2000//模块反馈一些信息给网关	



#endif



