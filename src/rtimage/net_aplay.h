/** @file	       net_aplay.h
 *   @brief 	接收并处理客户端发来的音视频下行命令及数据流
 *   @date 	2007.03
 */
#ifndef NET_AVSTREAM_H_20070330
#define NET_AVSTREAM_H_20070330


#define RTIMAGE_AUDIO_DEV   2  //音频设备编号
#define RTIMAGE_AUDIO_DEC   0  //音频解码器编号
#define RTIMAGE_AUDIO_DEV_CHN 0 //音频设备通道号

int create_rtnet_aplay_servers(void);

void check_and_play(int no);

#endif
