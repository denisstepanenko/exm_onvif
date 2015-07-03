#ifndef PROCESS_RTIMG_H
#define PROCESS_RTIMG_H
#include <mod_cmd.h>
#include "mod_socket.h"


/**********************************************************************************************
 * 函数名	:process_rtimg_cmd()
 * 功能	:处理tcprtimg模块发来的命令
 * 输入	:cmd:接收到的tcprtimg模块的命令缓冲区
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int process_rtimg_cmd(mod_socket_cmd_type *cmd);

#endif





