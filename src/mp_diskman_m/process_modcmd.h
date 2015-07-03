#ifndef PROCESS_GATECMD_H
#define PROCESS_GATECMD_H
#include <gate_cmd.h>
#include "mod_socket.h"


//初始化与主进程通讯的命令通道
int init_com_channel(void);
int creat_diskman_modsocket_thread(void);
int send_state2main(void);

#endif
