#ifndef PROCESS_KEYBOARD_H
#define PROCESS_KEYBOARD_H
#include "mod_socket.h"

/**********************************************************************************************
 * 函数名	:process_keyboard_cmd()
 * 功能	:处理键盘模块发来的命令
 * 输入	:cmd:接收到的键盘模块的命令缓冲区
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int process_keyboard_cmd(mod_socket_cmd_type *cmd);
#endif
