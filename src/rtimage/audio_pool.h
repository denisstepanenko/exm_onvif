/** @file	audio_pool.c
 *   @brief 	操作音频缓冲池的函数声明
 *   @date 	2007.03
 */
#ifndef AUDIO_POOL_20070305
#define AUDIO_POOL_20070305
#include <media_api.h>

/** 
 *   @brief     初始化音频编码缓冲池相关变量
 *   @return   0表示成功,负值表示失败
 */ 
int init_audio_enc_pool(void);

/** 
 *   @brief     获取音频编码缓冲池结构指针
 *   @param  no 音频编码器序号
 *   @return   音频编码缓冲池结构指针
 */ 
media_source_t *get_audio_enc(IN int no);
int get_audio_enc_remain(int no);

#endif

