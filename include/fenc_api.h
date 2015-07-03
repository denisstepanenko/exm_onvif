#ifndef FENC_API_H
#define FENC_API_H
#include "typedefine.h"
#include <sys/time.h>
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef IO
#define IO
#endif

struct video_enc;

#ifndef struct_HDR
#define struct_HDR
struct NCHUNK_HDR {	//avi格式的数据块头标志结构
#define IDX1_VID  		0x63643030	//AVI的视频包标记
#define IDX1_AID  		0x62773130	//AVI的音频报的标记
	unsigned long  chk_id;
	unsigned long  chk_siz;
};
#endif


typedef struct{
    ///压缩后的视频帧
    ///使用这个结构时要先分配一个大缓冲区,然后将本结构的指针指向缓冲区
    
#define MEDIA_VIDEO		0x01		//视频数据
#define MEDIA_AUDIO	0x02		//音频数据

#define FRAMETYPE_I		0x0		// frame flag - I Frame
#define FRAMETYPE_P		0x1		// frame flag - P Frame
#define FRAMETYPE_B		0x2
#define FRAMETYPE_PCM	0x5		// frame flag - Audio Frame
#define FRAMETYPE_AAC	0x7		// frame flag - Audio Frame



	struct timeval           tv;			   ///<数据产生时的时间戳
	unsigned long	           channel;	          ///<压缩通道
	unsigned short           media;		   ///<media type 音频或视频
	unsigned short           type;		          ///<frame type	I/P/声音...
	long                          len;	                 ///<frame_buf中的有效字节数
	struct NCHUNK_HDR chunk;                ///<数据块头标志，目前使用avi格式
	unsigned char            frame_buf[4];    ///<存放编码后的视频数据的起始地址
}enc_frame_t;

typedef struct motion_cfg
{
    unsigned int x_left_up;
    unsigned int y_left_up;
    unsigned int x_right_down;
    unsigned int y_right_down;	//这四个参数限定移动侦测的范围
    unsigned int threshold; /* threshold of motion vector */
    unsigned int sad_threshold;//sensitivity
    unsigned int mv_number;		//需要检测的16*16 block数
} motion_cfg;

typedef struct 
{

    ///编码类型定义
    #define     FENC_MJPEG          0
    #define     FENC_MPEG4          1
    #define     FENC_H264             2
    #define     FEBC_UNKNOW      -1
    unsigned int enc_type;           ///<编码类型

    unsigned int width;                      //length per dma buffer
    unsigned int height;
    unsigned int frame_rate;            ///<帧率     
    unsigned int frame_rate_base;  ///<应用程序不需要关心,由库函数填充
    unsigned int gop_size;                ///<I帧间隔

    ///编码模式定义
    #define FENC_CBR        1       ///<定码流
    #define FENC_VBR        2       ///<带码流上限的变码流
    unsigned int enc_mode;              ///<编码模式1:CBR 2:VBR(带码率上限)

    unsigned int bit_rate;                   ///<码流 bps  CBR时表示码流大小

    unsigned int quant;                     ///<编码质量VBR时有效
    unsigned int min_bitrate;           ///<VBR时有效
    unsigned int max_bitrate;          ///<VBR时有效
    
   //unsigned int qmax;      
   //unsigned int qmin;       
    //unsigned int intra;
    unsigned int roi_enable;            ///<编码区域使能 ,为1时 roi_x,roi_y,roi_width,roi_height有效
    unsigned int roi_x;                     ///<编码区域的起始位置
    unsigned int roi_y;	
    unsigned int roi_width;              ///<编码区域宽度
    unsigned int roi_height;	           ///<编码区域高度


    ///以下为motion detect 参数
    unsigned int motion_sen;				//移动侦测灵敏度
   	struct motion_cfg	md_config;			
   
   	//以下blind detect 参数
   	unsigned int blind_sen;					//遮挡报警灵敏度
   	unsigned int blind_alarm_time;
   	unsigned int blind_cancelalarm_time;	//取消报警时间
    //TODO

  //  unsigned int fmpeg4_qtbl;          ///<使用量化表的类型
    //enc_frame_t *coded_frame;
    //char *priv;
} fenc_param_t;


typedef struct fenc{
   
    fenc_param_t      param;            ///<视频编码参数
    
    void                   *encoder;        ///<video_enc_t  结构,描述视频编码器的结构
    
    int                       priv_size;
    void                    *priv_data;

    int (*mdtect_callback)(struct fenc *handle,int motion_stat,int blind_stat);///TODO 可以根据需要定义motion_stat结构
	
}fenc_t;



/**********************************************************************************
 *      函数名: fenc_open()
 *      功能:   以指定的编码格式打开编码器
 *      输入:   enc_type:编码格式
 *      输出:   无
 *      返回值: 指向编码器结构的指针,NULL表示失败，错误码见errno
 *********************************************************************************/
fenc_t *fenc_open(IN unsigned int enc_type);

/**********************************************************************************
 *      函数名: fenc_set_param()
 *      功能:   设定编码器的工作参数
 *      输入:   enc: 已经用fenc_open 返回的句柄
 *                     param:待设置的参数
 *      输出:   enc:填充好新参数值的结构指针
 *      返回值: 0表示成功,负值表示失败，错误码见errno
 *********************************************************************************/
int fenc_set_param(IO fenc_t *enc,IN fenc_param_t *param);

/**********************************************************************************
 *      函数名: fenc_encode_frame()
 *      功能      :   设定编码器的工作参数
 *      输入      :enc: 已经用fenc_open 返回的句柄
 *                        buf_size:输出缓冲区的大小
 *                        data:存放原始视频数据的缓冲区(和压缩参数大小匹配)
 *      输出      :frame: 填充好的压缩后的视频帧(tv字段没有填充)
 *      返回值: 正值表示有效的压缩数据字节数，,负值表示失败，错误码见errno
 *********************************************************************************/
int fenc_encode_frame(IN fenc_t *enc,OUT enc_frame_t *frame, IN int buf_size, IN void * data);


/**********************************************************************************
 *      函数名: fenc_close()
 *      功能:   关闭编码器
 *      输入:   enc: 已经用fenc_open 返回的句柄
 *      输出:   enc:释放了内存，无效的指针
 *      返回值: 0表示成功,负值表示失败，错误码见errno
 *********************************************************************************/
int fenc_close(IO fenc_t *enc);

/**********************************************************************************
 *      函数名: fmjpeg_encoder_sj()
 *      功能:   将一帧原始视频数据编码成jpeg格式的图片文件
 *      输入:   width:图片宽度
 *                     height:图片高度
 *                     data:原始视频缓冲区
 *                     outfile:输出文件名
 *      输出:   outfile文件:存放压缩好的jpg文件
 *      返回值: 正值表示文件大小,负值表示失败，错误码见errno
 *********************************************************************************/
int fmjpeg_encoder_sj(IN int width,IN int height,IN void *data,IN char *outfile);


int fenc_set_motion_detect_callback(fenc_t *enc,int (*mdtect_callback)(fenc_t *handle,int motion_stat,int blind_stat));


/**********************************************************************************
 *      函数名: fenc_read_para_file()
 *      功能:   从指定的配置文件中读取编码参数
 *      输入:   filename: 配置文件名
 *                     section:描述编码参数信息的ini节名
 *      输出:   enc_param:填充好新参数值的结构指针
 *      返回值: 0表示成功,负值表示失败，错误码见errno
 *********************************************************************************/
int fenc_read_para_file(IN char *filename,IN char *section, OUT fenc_param_t  *enc_param);

/**********************************************************************************
 *      函数名: motion_blind_read_para_file()
 *      功能:   从指定的配置文件中读取移动侦测和视频遮挡参数
 *      输入:   enc_no: 编码器通道号
 *				 filename: 配置文件名
 *      输出:   enc_param:填充好新参数值的结构指针
 *      返回值: 0表示成功,负值表示失败，错误码见errno
 *********************************************************************************/
int motion_blind_read_para_file(IN int enc_no, IN char *filename, OUT fenc_param_t  *enc_param);



int fenc_reset_timeout(int no,int num);

#endif

