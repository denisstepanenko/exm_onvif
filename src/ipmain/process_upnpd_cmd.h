#ifndef PROCESS_UPNPD_H
#define PROCESS_UPNPD_H

#include "mod_socket.h"


/**********************************************************************************************
 * 函数名	:process_upnpd_cmd()
 * 功能	:处理upnpd模块发来的命令
 * 输入	:cmd:接收到的upnpd模块的命令缓冲区
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int process_upnpd_cmd(mod_socket_cmd_type *cmd);
#endif
