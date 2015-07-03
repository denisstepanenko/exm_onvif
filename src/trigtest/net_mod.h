#ifndef __NET_MOD_H
#define __NET_MOD_H

#include"testmod.h"


#define ACK_PKT	"ack"
#define SEQ_ACK	"seq_ack"
#define RET_VAL	"ret"
#define ERR_STR	"err"

#define RPT_PKT		"process_report"
#define PROGRESS		"progress"
#define DETAIL_STR	"detail"

#ifdef  TEST224
#define HOSTNAME_ETH0 		"224.0.0.1"
#define DEF_MULT_ADDR0		"224.0.0.0"
#else
#define DEF_MULT_ADDR		"224.0.0.0"
#define HOSTNAME_ETH0 		"225.0.0.1"
#define DEF_MULT_ADDR0		"225.0.0.0"
#endif
#define HOSTNAME_ETH1 		"226.0.0.1"
#define DEF_MULT_ADDR1		"226.0.0.0"
#define DEF_MULT_NETMASK	"255.0.0.0"

#define MULTI_CAST_PORT 3310

#define CMD_STR 	"cmd"
#define SEQ_STR 	"seq"

#define	DEV_GUID_BYTE		8	//GUID占用的字节数





/******************************************************
*函数名称: init_dev_net_port
*函数功能: 初始化网络连接
*输入： 	
*		net_st 存放网络参数的数据结构
*输出： 	0 正确  错误退出程序
*修改日志：
******************************************************/ 
 int init_dev_net_port(multicast_sock* net_st);

/******************************************************
*函数名称: send_test_report
*函数功能: 发送测试进程参数和实时信息
*输入： 	
*		net_st 存放网络参数的数据结构
*		unsigned char* info	测试的实时信息
*		int prog 进程0-100的整数
*输出： 	0 正确  错误退出程序
*修改日志：
******************************************************/ 
 int send_test_report(multicast_sock* ns, unsigned char* info,int prog);

/******************************************************
*函数名称: send_test_cmd
*函数功能: 发送组波数据
*输入： host_name host_name1 两个组播地址
*			port 组播端口号
*			net_st 存放网络参数的数据结构
*输出： 0 正确  错误退出程序
*修改日志：
******************************************************/ 
int send_multicast_pkt(multicast_sock *net_st,  char *buf);


/*
*****************************************************
*函数名称: pass_dev_GUID
*函数功能: 传递GUID
*输入： 
*输出： unsigned char *cmd  存放guid 的缓冲区
			错误代码0 正确 负值 错误
*修改日志：
*****************************************************
*/
int pass_dev_GUID(unsigned char *cmd);








#endif
