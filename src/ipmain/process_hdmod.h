#ifndef PROCESS_HDMOD_H
#define PROCESS_HDMOD_H

#include "mod_socket.h"

/**********************************************************************************************
 * 函数名	:process_hdmod_cmd()
 * 功能	:处理hdmodule模块发来的命令
 * 输入	:cmd:接收到的hdmodule模块的命令缓冲区
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int process_hdmod_cmd(mod_socket_cmd_type *cmd);

#endif
