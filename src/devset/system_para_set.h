/*
**************************************************************
文件名称：system_para_set.h
编写者： lsk
编写日期：2006-9-12
简要描述：声明文件名称：system_para_set.c中系统参数设置的函数
修改记录：
修改日志：
***************************************************************
*/
#ifndef _SYSTEM_PARA_SET_H_
#define _SYSTEM_PARA_SET_H_
#include "communication.h"
//#include <nv_pair.h>
int init_sys_paras(void);
void prinf_sys_info(void);

int set_sys_IP(int net_adp_index, unsigned char *cmd);
int set_sys_netmask(int net_adp_index,  unsigned char *cmd);
int set_sys_mac_addr(int net_adp_index,  unsigned char *cmd);
int set_sys_gateway(unsigned char *cmd);

int set_dev_type(unsigned char *cmd);
int set_dev_GUID(unsigned char *cmd);
int set_produce_date(unsigned char *cmd);
int set_dev_board_seq( unsigned char *cmd);
int set_dev_batch_seq( unsigned char *cmd);
//void atoi_time(BYTE* time_byte, struct tm *time_int);

int set_sys_date(unsigned char *cmd);
int set_config_info(BYTE* info , BYTE* val);
int pass_sys_date(unsigned char *cmd);
int pass_dev_GUID(unsigned char *cmd);
int pass_dev_IP(unsigned char *cmd, int index);
int pass_dev_netmask(unsigned char *cmd, int index);
int pass_dev_mac(unsigned char *cmd, int index);
int pass_dev_type(unsigned char *cmd);
int pass_sys_gateway( unsigned char *cmd);
int pass_dev_batch_seq( unsigned char *cmd);
int pass_dev_board_seq( unsigned char *cmd);
int pass_produce_date(struct tm *ptime);
int get_dev_hd_sn(const char* szDevName, char* szSN, size_t nLimit);

int handdle_search_dev(multicast_sock * ns,int channel);
//int handdle_search_dev(NVP_TP *dist, multicast_sock * ns);
void reset_network(int index);
void set_sys_flag(int index , int flag);
int get_sys_flag(int index);
int is_bussy(void);
 int init_net_port(char* host_name, int port, multicast_sock* net_st);


#endif
