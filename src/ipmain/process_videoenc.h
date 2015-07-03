#ifndef PROCESS_VIDEOENC_H
#define PROCESS_VIDEOENC_H

#include "mod_socket.h"


/**********************************************************************************************
 * 函数名	:process_videoenc_cmd()
 * 功能	:处理videoenc模块发来的命令
 * 输入	:cmd:接收到的videoenc模块的命令缓冲区
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int process_videoenc_cmd(mod_socket_cmd_type *cmd);
int process_videoenc_vda_state(mod_socket_cmd_type *cmd);
int process_videoenc_err(mod_socket_cmd_type *cmd);

#endif
