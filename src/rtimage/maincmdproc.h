/** @file	       maincmdproc.c
 *   @brief 	接收并处理从主进程发来的命令的函数声明
 *   @date 	2007.03
 */
#ifndef MAIN_CMD_PROC_H_20070305
#define MAIN_CMD_PROC_H_20070305



/** 
 *   @brief     创建及初始化与主进程通讯的命令通道
 *   @return   0表示成功,负值表示失败
 */ 
int init_com_channel(void);

/** 
 *   @brief     创建接收并处理主进程发来的命令的线程
 *   @param  无
 *   @return   0表示成功,负值表示失败
 */ 
int creat_mod_cmdproc_thread(void);

/** 
 *   @brief     设置网络音视频服务忙状态
 *   @param  busy 0表示没有用户连接 1表示至少有一个用户连接音视频
 */ 
void set_net_enc_busy(int busy);


/**
*	@brief 	创建监测网络发送的数据包个数
*	@param	无
*	@return	0
*/
int creat_snd_pkts_thread(void);


int send_rtimg_stop_playback(void);

#endif

