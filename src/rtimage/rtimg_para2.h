/** @file	audio_pool.c
 *   @brief 	操作模块参数的函数接口声明
 *   @date 	2007.03
 */
#ifndef RTIMG_PARA2_H_20070302
#define RTIMG_PARA2_H_20070302
#include "serv_info.h"



/** 
 *   @brief     将tcprtimage2模块的所有参数和状态设置为初始值
 *   @return   0表示成功,负值表示失败
 */ 
int init_server_para(void);

int read_server_para_file(void);

tcprtimg_svr_t *get_rtimg_para(void);
#endif

