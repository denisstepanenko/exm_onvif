#ifndef SEND_STATE_H
#define SEND_STATE_H






/*
 *初始化与主进程通讯的命令通道
 * 返回值:0 表示发送成功
 * 负值:    表示出错
*/
int init_com_channel(void);
/*
 * 发送状态给主进程 
 * 参数:stat:要发送的状态
 * 返回值:0 表示发送成功
 * 负值:    表示出错
*/
int send_pppoe_stat2main(int stat);

//监听来自vm/smain的查询并发送相关状态

void *recv_modsocket_thread (void *data);
void *recv_modcom_thread (void *data);

int creat_recv_modsocket_thread(void);
#endif
