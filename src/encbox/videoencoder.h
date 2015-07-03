#ifndef VIDEO_ENCODER
#define VIDEO_ENCODER
#include <typedefine.h>
#include <fenc_api.h>
#include <mshmpool.h>
#include <media_api.h>
typedef struct{
//视频编码器结构
	int						venc_cnt;		//用于监视编码器的计数器
	int						venc_err_cnt;	//视频编码器出错的次数
	int						venc_first_start;	//程序刚启动标志
	media_source_t			media;			//媒体源结构
}video_enc_t;

typedef struct{
//视频编码器结构
	int						enc_first_start;	//程序刚启动标志
	media_source_t			media;			//媒体源结构
}audio_enc_t;

int get_onvif_device_num(void);

DWORD get_videoencstatint(void);





#endif
