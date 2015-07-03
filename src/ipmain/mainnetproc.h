#ifndef MAIN_NETCMD_PROC_H
#define MAIN_NETCMD_PROC_H
#include <gt_com_api.h>
#include "mod_socket.h"


typedef struct{
int thread_no;	//线程编号
int listen_fd;	//监听fd
int	dev_no;		//设备编号
}mainnet_info;

/**********************************************************************************************
 * 函数名	:print_gate_pkt()
 * 功能	:将设备与网关通讯的数据包显示在屏幕上(调试用)
 * 输入	:recv:要打印的通讯数据包
 * 返回值	:无
 **********************************************************************************************/
#ifdef	SHOW_GATE_CMD_REC
void print_gate_pkt(struct gt_pkt_struct* recv);
#endif

/**********************************************************************************************
 * 函数名	:init_mainnetcmd_threads()
 * 功能	:创建接受远程命令连接的线程池
 * 输入	:attr:线程属性
 *			 para:提供命令服务的端口指针
 * 返回值	:无
 **********************************************************************************************/
int init_mainnetcmd_threads(pthread_attr_t *attr,int port, int dev_no);
#endif



