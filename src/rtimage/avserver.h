/** @file	       avserver.c
 *   @brief 	提供音视频服务的相关函数声明
 *   @date 	2007.03
 */
 #ifndef AVSERVER_H_20070309
 #define AVSERVER_H_20070309

#define	HDDBUFF_MAX_FRAME		(25*90)	//按照1秒30帧，共90帧计算
 
 /** 
 *   @brief     创建音视频服务线程
 *   @return   0表示成功,负值表示失败
 */ 
int create_av_server(void);
 
/** 
 *   @brief     音视频上行服务的秒处理程序
 */ 
void avserver_second_proc(void);

/**
*	@brief		创建硬盘缓冲区线程
*	@return		0表示成功，负值表示失败
*/
int  create_diskbuff_thread(void);

void  set_playback_en(int no);
void set_playback_cancel(int no);
void set_hddbuf_offset(int offset);
int get_playback_stat(int no);

#endif
