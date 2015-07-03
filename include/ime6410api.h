#ifndef IME6410API_H
#define IME6410API_H

#ifndef _WIN32

#include <sys/ipc.h>
#include <ime6410.h>
#include <sys/time.h>
#else
#include <windows.h>
#endif //_WIN32

#include <pthread.h>

#define MEDIA_VIDEO		0x01		//视频数据
#define MEDIA_AUDIO		0x02		//音频数据

#define IDX1_VID  		0x63643030	//AVI的视频包标记
#define IDX1_AID  		0x62773130	//AVI的音频报的标记


#ifndef struct_HDR

#define struct_HDR
struct NCHUNK_HDR {	//avi格式的数据块头标志结构
	unsigned long  chk_id;
	unsigned long  chk_siz;
};
#endif



struct compress_struct
{

	int		fd;					//压缩设备的设备描述符
#ifndef _WIN32
	int		init_tw9903flag;		//已经初始化过tw9903标志
	int		NTSC_PAL;			//pal制输入标志
	int		frame_rate;			//帧率
	int		AudioFrameType;	//音频类型 0表示不需要音频 其他值表示音频类型 定义见ime6410.h
	pthread_mutex_t mutex;		//需要独占访问时用到的互斥体
	int		full_ch;				//9903使用的视频输入通道号
	int     	saturation; 			//9903 饱和度,128为缺省,范围为0-255，下同
	int     	hue;        			//9903 色度
	int     	contrast;   			//9903 对比度
	int     	brightness; 			//9903 亮度
	int		sharp1;
	int		sharp2;
	int 		md_sense;		    // 移动侦测的灵敏度0-5  0表示禁止运动检测,5,4,3,2,1表示灵敏度高,较高,中,较低,低   lsk 2007-1-5
	int 		md_enable;		    //移动侦测使能 0 : 禁止  1 : 使能   lsk 2007-1-5
  
	int 		md_var[4];			//移动侦测变化量		lsk 2007-1-5
	struct I64Reg i64reg;			//编码器的参数结构
	key_t EncKey;				//视频编码器用到的缓冲池key
#endif
	char ini_sec[60];				//配置文件中的节名
	char dev_node[60];			//设备节点名
	char dev_name[60];			//设备名
	char driver_name[60]; 		//驱动程序名	

};

struct stream_fmt_struct
{							//数据帧的结构
	struct timeval tv;			//数据产生时的时间戳
	unsigned long	channel;	//压缩通道，如果是6410则仅有一个通道，6400有4个
	unsigned short media;		//media type 音频或视频
	unsigned short  type;		//frame type	I/P/声音...
	 long len;				//frame len 后面的视频或音频祯数据的长度
	struct NCHUNK_HDR chunk;//数据块头标志，目前使用avi格式
	char data[4];				//frame data changed by shixin 060327
};


/**********************************************************************************************
 * 函数名	:read_enc_para_file()
 * 功能	:从指定配置文件里的指定节中读取编码参数
 * 输入	:filename:配置文件名
 *			 section:配置文件中的节,如"netencoder","hqenc0"...节名不能超过20byte
 * 输出	 enc:指向要存放参数的指针	,函数返回时会被填充好
 * 返回值	:0表示成功负值表示出错
 **********************************************************************************************/
int read_enc_para_file(char *filename,char *section, struct compress_struct   *enc);
#define ReadEncParaFile read_enc_para_file


/**********************************************************************************************
 * 函数名	:init_enc_default_val()
 * 功能	:将编码器参数设置为默认值,应该在读取配置文件之前调用一次
 * 输入	:无
 * 输出:encoder 指向需要被填充的编码器参数结构的指针
 *					函数返回时被填充好 
 * 返回值	:无
 **********************************************************************************************/
void init_enc_default_val(struct compress_struct *encoder);

/**********************************************************************************************
 * 函数名	:open_ime6410()
 * 功能	:打开6410，
 * 输入	:devname:设备节点名
 *			 encoder:描述编码器相关数据的结构指针
 * 返回值	:0 正常打开，文件描述符填充到encoder->fd
 *			  负值表示 错误
 *	如果设备已经打开则直接返回0
 **********************************************************************************************/
int open_ime6410(char *devname,struct compress_struct *encoder);

#if 1 //wsy sep 2006
//百分比的值
int set_imevideo_brightness(struct compress_struct *encoder,int val);
int set_imevideo_saturation(struct compress_struct *encoder,int val);
int set_imevideo_contrast(struct compress_struct *encoder,int val);
int set_imevideo_hue(struct compress_struct *encoder,int val);
//将ime6410前端的9903切换成指定通道

/*
	初始化6410
	encoder:已经填充好的参数
	返回值 0:成功
			 <0:错误
*/
int switch_imevideo_channel(struct compress_struct *encoder,int ch);
#endif

/**********************************************************************************************
 * 函数名	:init_ime6410()
 * 功能	:按照输入的参数结构初始化ime6410硬件
 * 输入	:encoder:已经填充好参数并且设备已经打开的结构体
 * 返回值	:0表示成功负值表示出错
 **********************************************************************************************/
int init_ime6410(struct compress_struct *encoder);
#define InitVEncoder init_ime6410

/**********************************************************************************************
 * 函数名	:is_keyframe()
 * 功能	:判断一帧数据是否是关键帧,本函数在文件切割时会用到
 * 输入	:frame 需要被判断的数据帧
 * 返回值	:0 表示非关键侦
 *			 1表示是关键侦
 **********************************************************************************************/
int is_keyframe(struct stream_fmt_struct *frame);

/**********************************************************************************************
 * 函数名	:i64_read_frame()
 * 功能	:从6410中读取1帧视频或者1帧音频数据
 * 输入	:encoder:已经填充好参数并且设备已经打开的结构体
 *			 buf_len:输出缓冲区的大小，如果一包数据大于这个值则截断	
 *			 key_f:读取关键侦标志，如果为1则丢弃其他侦，只读取iframe
 *				当存文件的第一侦数据时需要设置此参数，其他时候为0
 * 输出	 buffer:目标缓冲区,本函数将把读取到的数据放入此缓冲区
 * 返回值	:0表示成功负值表示出错
 * 			-6 表示encoder中的设备fd为负值
 * 			-5 表示输入的参数encoder为空
 * 			-4 表示输入的buffer为空
 * 			-3表示参数错误
 * 			-2 表示文件结束
 * 			-1表示其它错误
 **********************************************************************************************/
int i64_read_frame(struct compress_struct *encoder,struct stream_fmt_struct *buffer,int buf_len,int req_keyf);
#define ReadEncFrame i64_read_frame

/**********************************************************************************************
 * 函数名	:close_ime6410()
 * 功能	:关闭已经打开的6410
 * 输入	:encoder描述编码器的数据结构
 * 返回值	:0 表示成功 ，负值表示失败
 **********************************************************************************************/
int close_ime6410(struct compress_struct *encoder);
#define CloseVEnc	close_ime6410


/**********************************************************************************************
 * 函数名	:I64_Start()
 * 功能	:启动6410，开始压缩数据
 * 输入	:enc描述编码器的数据结构
 * 返回值	:0 表示成功，负值表示失败
 **********************************************************************************************/
int I64_Start(struct compress_struct *enc);
#define StartVEnc I64_Start


/**********************************************************************************************
 * 函数名	:I64_CheckRegister()
 * 功能	:打印6410的寄存器到终端，一般用不到
 * 输入	:encoder描述编码器的数据结构
 * 返回值	:0 表示成功，负值表示失败
 **********************************************************************************************/
int I64_CheckRegister(struct compress_struct *encoder);

/**********************************************************************************************
 * 函数名	:reinstall_encdrv()
 * 功能	:重新安装6410驱动程序
 * 输入	:enc描述编码器的数据结构
 * 返回值	:0 表示成功，负值表示失败
 **********************************************************************************************/
int reinstall_encdrv(struct compress_struct *enc);
#define ReinstallEncDrv reinstall_encdrv


/**********************************************************************************************
 * 函数名	:OpenEnc()
 * 功能	:打开编码器 等同于open_ime6410，只是节点名已经填充在结构中了
 * 输入	:encoder描述编码器的数据结构
 * 返回值	:0 表示成功，负值表示失败
 **********************************************************************************************/
int OpenEnc(struct compress_struct *encoder);


/**********************************************************************************************
 * 函数名	:fill_driver_name()
 * 功能	:根据名称填充编码器结构中的字段
 * 输入	:name:编码器名称(节点名)可以为 RT_6410_DEV，HQDEV0
 * 输出	:enc:描述编码器参数的结构指针，
 *			 返回时与输入节点名对应的节点名，驱动路径，驱动名称将被填充
 * 返回值	:无
 **********************************************************************************************/
 #include <file_def.h>

static __inline__ int fill_driver_name(struct compress_struct   *enc,char *name)
{
	if(enc==NULL)
		return -1;
/*
	初始化节点及驱动名称
*/
	sprintf(enc->dev_node,"%s",RT_6410_DEV);
	sprintf(enc->dev_name,"%s",NET6410_NAME);
	sprintf(enc->driver_name,"%s",NET6410_DRV);
	if(name==NULL)
		return 0;
	if(memcmp(name,HQDEV0,strlen(HQDEV0))==0)
	{
		sprintf(enc->dev_node,"%s",HQDEV0);
		sprintf(enc->dev_name,"%s",LOCAL6410_NAME);
		sprintf(enc->driver_name,"%s",LOCAL6410_DRV);			
	}
	return 0;
}
int get_pic_width(struct compress_struct *enc);
int get_pic_height(struct compress_struct *enc);

//读出9903的视频丢失
int read_9903_video_loss(struct compress_struct *encoder);

int get_frame_rate(int ispal,int rate);
int get_picture_size(int ispal,int size);
//int SetVEncBitRate(struct compress_struct *Enc,int bitrate);
//int RestartVCompress(struct compress_struct *encoder);
#endif

