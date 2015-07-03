/*
**************************************************************
文件名称：CMD_PROCESS.h
编写者： lsk
编写日期：2006-9-12
简要描述：声明通讯协议的数据结构和变量
修改记录：
修改日志：
***************************************************************
*/
#ifndef _CMD_PROCESS_H_
#define _CMD_PROCESS_H_
#include <nv_pair.h>
typedef struct 
{
	unsigned int grp[4];
}ip_grp;

typedef struct 
{
	int flag_update;
	int flag_clear;
	int flag_test;
}cmd_flag;

void devide_ip_addr(char*ip_addr, ip_grp* ip_info);
int cmd_handdle(NVP_TP *nv , multicast_sock *ns);
int send_ack_packet(int err_code, multicast_sock *ns);
int send_test_report(multicast_sock* ns, unsigned char* info,int prog);
int result_rpt(int err_code, unsigned char *info, multicast_sock* ns);
#endif
