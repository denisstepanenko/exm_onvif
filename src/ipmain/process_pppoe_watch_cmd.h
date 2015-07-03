#ifndef PROCESS_PPPOE_H
#define PROCESS_PPPOE_H

#include "mod_socket.h"


/**********************************************************************************************
 * 函数名	:process_pppoe_watch_cmd()
 * 功能	:处理pppoe_watch模块发来的命令
 * 输入	:cmd:接收到的pppoe_watch模块的命令缓冲区
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int process_pppoe_watch_cmd(mod_socket_cmd_type *cmd);

#endif
