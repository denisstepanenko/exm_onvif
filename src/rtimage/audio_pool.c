/** @file	audio_pool.c
 *   @brief 	操作音频缓冲池的函数定义
 *   @date 	2007.03
 */
#include "rtimage2.h"
#include "debug.h"
static  media_source_t  audio_enc[MAX_AUDIO_ENCODER];       ///<音频编码器结构


/** 
 *   @brief     设置音频编码器属性
 *   @param  afmt     准备存放音频编码器属性的指针
 *   @return   0表示成功,负值表示失败
 *   
 */ 
static int set_audio_enc_attrib(OUT audio_format_t *afmt)
{

	if(afmt==NULL)
		return -EINVAL;
	afmt->a_wformat=7;		//WFORMAT_ULAW			0x0007//声音格式
	afmt->a_sampling=8000;	//声音采样率
	afmt->a_channel=1;		//声音通道
	afmt->a_nr_frame=1;		//一包声音里面有几块数据
	afmt->a_bitrate=8000;		//声音码流
	afmt->a_bits=8;			//音频采样位数
	afmt->a_frate=8;			//(8000/1024)音频帧率 FIXME
	return sizeof(audio_format_t);
}

/** 
 *   @brief     初始化音频编码缓冲池相关变量
 *   @return   0表示成功,负值表示失败
 */ 
int init_audio_enc_pool(void)
{
    int i;
    int ret;
    media_source_t *aenc=&audio_enc[0];
    init_media_rw(aenc,MEDIA_TYPE_AUDIO,i,32*1024);
    ret=create_media_write(aenc,get_audio_enc_key(0),"tcprtimg",128*1024);
    if(ret>=0)
    {
        set_audio_enc_attrib(&aenc->attrib->fmt.a_fmt);
    }
    
    return ret;

}


/** 
 *   @brief     获取音频编码缓冲池结构指针
 *   @param  no 音频编码器序号
 *   @return   音频编码缓冲池结构指针
 */ 
media_source_t *get_audio_enc(IN int no)
{
    return &audio_enc[no];
}

