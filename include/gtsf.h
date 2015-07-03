#ifndef GTSF_H
#define GTSF_H
#include <sys/time.h>
#ifdef __cplusplus
extern "C" {
#endif


#define GTSF_MARK            0XBFCF0267
#define GTSF_HEAD_SIZE            28



#define MEDIA_VIDEO		  0x01	//视频数据
#define MEDIA_AUDIO	    0x02	//音频数据
#define MEDIA_SPEECH		0x03	//对讲数据
#define MEDIA_COMMAND		0x04	//信令，在发送命令用
#define MEDIA_STREAMID	0x05	//流票,由TCP监听方产生，主动连接方在连接后发送给监听方


#define FRAMETYPE_I		0x0		// IME6410 Header - I Frame
#define FRAMETYPE_P		0x1		// IME6410 Header - P Frame
#define FRAMETYPE_B		0x2
#define FRAMETYPE_PCM	0x5		// IME6410 Header - Audio Frame
#define FRAMETYPE_ADPCM	0x6		// IME6410 Header - Audio Frame
#define FRAMETYPE_AAC	0x7		// frame flag - Audio Frame
#define FRAMETYPE_MD	0x8

#define RATIO_D1_PAL       0x0      //720*576
#define RATIO_D1_NTSC      0x1      //704*576
#define RATIO_CIF_PAL      0x2      // 352*288
#define RATIO_CIF_NTSC     0x3      // 320*240


//视频编码类型
#define         VIDEO_MPEG4                     0
#define         VIDEO_H264                      1
#define         VIDEO_MJPEG                     2

typedef struct{	//视频格式信息结构
	struct timeval tv;			//数据产生时的时间戳
	unsigned long	Sequence;  //序列号
	unsigned char format;		//编码格式format
	unsigned char  type;		//frame type	I/P/B...
	unsigned char ratio;  //分辨率
	unsigned char recv[4];			//是否是pal制视频
	
}stream_video_format_t;

typedef struct{	//音频格式信息结构
	struct timeval tv;			//数据产生时的时间戳
	unsigned short a_sampling;	//声音采样率
	unsigned char  a_channel;	//声音通道
	unsigned char  a_wformat;	//声音格式
	unsigned char  a_nr_frame;	//一包声音里面有几块数据
	unsigned char  a_bitrate;		//声音码流
	unsigned char  a_bits;		//音频采样位数
	unsigned char  a_frate;		//音频数据的帧率(没秒钟有几包音频数据)
	
}stream_audio_format_t;

typedef union{ //媒体格式定义联合体
	stream_video_format_t v_fmt;
	stream_audio_format_t a_fmt;
}stream_format_t;




typedef struct gtsf_stream_fmt_struct
{
	unsigned long mark;               /*命令标示*/
	unsigned char type;		          //media type 音频或视频
	unsigned char	encrypt_type;	      //加密类型，0是不加密
	unsigned char	channel;		        //音视频通道，注意音频和视频的通道可能不一样
	unsigned char	version;			      //通讯协议的版本号，目前没有，可以保留
	long len;		//不包括帧头，后面的数据长度，如果加密，加密后的数据长；从data开始
	stream_format_t media_format;      //数据块格式
	char data[0];				              //frame data
}gtsf_stream_fmt;

#ifdef __cplusplus
};
#endif

#endif



