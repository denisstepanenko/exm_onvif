#ifndef UPNP_SEND_STATE_H
#define UPNP_SEND_STATE_H

#include "upnpd.h"

#define	MOD_COM_CHANNEL		0
#define	MOD_SOCKET_CHANNEL	1


/*
 *初始化与主进程通讯的命令通道
 * 返回值:0 表示发送成功
 * 负值:    表示出错
*/
int init_com_channel(void);
/*
 * 发送状态给主进程 
 * 参数:channel: 可为MOD_SOCKET_CHANNEL或MOD_COM_CHANNEL

 * 返回值:0 表示发送成功
 * 负值:    表示出错
*/
int send_upnpd_stat2main(int channel);


//监听来自vmmain的查询并发送相关状态
void *recv_modsocket_thread (void *data);

//监听来自vsmain的查询并发送相关状态
void *recv_modcom_thread(void *data);


/*将当前状态发送给modcom和modsocket两个通道*/
void send_state();

int create_recv_modsocket_thread(void);
#endif

