#ifndef MEDIA_API_H
#define MEDIA_API_H
#include <mshmpool.h>
#ifndef IN
	#define IN
	#define OUT
	#define IO
#endif
//媒体类型定义
#define		MEDIA_TYPE_VIDEO		0
#define		MEDIA_TYPE_AUDIO		1


///视频编码类型
#define         VIDEO_MPEG4                     0
#define         VIDEO_H264                        1
#define         VIDEO_MJPEG                      2



//媒体状态定义
#define 	ENC_NO_INIT		0		//编码器没有初始化
#define	ENC_STAT_OK		1		//编码器工作正常
#define	ENC_STAT_ERR		2		//编码器故障	

typedef struct{	//视频格式信息结构
	int format;		//视频编码格式      VIDEO_MPEG4等
	int ispal;			//是否是pal制视频
	int v_width;		//图像宽度
	int v_height;		//图像高度
	int v_frate;		//图像珍率
	int v_buffsize;		//建议缓冲区大小
}video_format_t;

typedef struct{	//音频格式信息结构
	int a_wformat;	//声音格式
	int a_sampling;	//声音采样率
	int a_channel;	//声音通道
	int a_nr_frame;	//一包声音里面有几块数据
	int a_bitrate;		//声音码流
	int a_bits;		//音频采样位数
	int a_frate;		//音频数据的帧率(没秒钟有几包音频数据)
}audio_format_t;

typedef union{ //媒体格式定义联合体
	video_format_t v_fmt;
	audio_format_t a_fmt;
}media_format_t;

typedef struct{ //媒体属性结构
	int				media_type;		//媒体类型
	int				stat;			//状态
	media_format_t	fmt;				//媒体格式
}media_attrib_t;


typedef struct{ //媒体源结构
	pthread_mutex_t	mutex;
	int				media_type;				//媒体类型(MEDIA_TYPE_VIDEO或MEDIA_TYPE_AUDIO)
	int				no;						//同类资源编号
	int				dev_stat;				//-1表示还没有连接到具体的编码设备缓冲池 ,0表示已经连接上
	pthread_t			thread_id;				//线程id
	int				max_data_len;			//该设备的数据块最大长度(动态刷新)
	MSHM_POOL		mpool;					//媒体用到的资源共享缓冲池
	void *			*temp_buf;				//读取数据用的缓冲区(按DWORD对齐)
	int				buflen;					//缓冲区长度,负值表示内存分配失败
	media_attrib_t		*attrib;					//设备属性		
}media_source_t;









/****************************************************************************/
/*******************************读写都用到的操作接口**************************/
/****************************************************************************/

/**********************************************************************************************
 * 函数名	:init_media_rw()
 * 功能	:初始化读写媒体信息用的结构
 * 输入	:type:媒体格式,音频还是视频 MEDIA_TYPE_VIDEO,MEDIA_TYPE_AUDIO
 *			 no:资源编号
 *			 buflen:要分配用于临时读写的缓冲区(media->temp_buf)的大小
 * 输出	 media:返回时填充好相应的信息
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/
int init_media_rw(OUT media_source_t *media,IN int type,IN int no,IN int buflen);

/**********************************************************************************************
 * 函数名	:get_media_attrib()
 * 功能	:获取描述媒体属性的结构指针
 * 输入	:media:描述媒体信息的结构指针
 * 返回值	:描述媒体属性结构的指针,NULL表示参数为NULL,或者还未连接
 **********************************************************************************************/
void *get_media_attrib(IN media_source_t *media);

/****************************************************************************/
/*******************************读操作的接口**************************/
/****************************************************************************/
/**********************************************************************************************
 * 函数名	:connect_media_read()
 * 功能	:连接到指定key的媒体缓冲池(读取数据的程序使用)
 * 输入	:key:要连接的媒体缓冲池的key
 *			 name:连接者的名字字符串
 *			 usr_type:连接者的用户类型 MSHMPOOL_LOCAL_USR和MSHMPOOL_NET_USR
 * 输出	 media:返回时填充好相应的信息
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/
int connect_media_read(OUT media_source_t *media,IN int key,IN char *name,IN int usr_type);

/**********************************************************************************************
 * 函数名	:disconnect_media_read()
 * 功能	:从已连接的媒体缓冲池断开(读取数据的程序使用)
 * 输入	:meida:描述已连接的媒体缓冲池结构指针
 * 输出	 media:返回时填充好相应的信息
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/
int disconnect_media_read(IO media_source_t *media);

/**********************************************************************************************
 * 函数名	:move_media_place()
 * 功能	:将当前用户的读取位置移动place个位置
 *			
 * 输入	:media:秒数媒体的指针
 *			 place:要移动的数量,
 *				负值表示向前移动,正值表示向后移动
 * 输出	 media:返回时填充好相应的信息
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/
int move_media_place(IO media_source_t *media,IN int place);

/**********************************************************************************************
 * 函数名	:reactive_media_usr()
 * 功能	:重新激活媒体服务(读取数据的程序使用)
 * 输入	:meida:描述已连接的媒体缓冲池结构指针
 * 输出	 media:返回时填充好相应的信息
 * 返回值	:0表示正常返回, 1表示已经重新激活激活 
 **********************************************************************************************/
int reactive_media_usr(IO media_source_t *media);

/**********************************************************************************************
 * 函数名	:read_media_resource()
 * 功能	:从媒体缓冲池中读取一块数据
 * 输入	:media:描述媒体信息的结构指针
 *			 buf_len:buf缓冲区的长度
 * 输出	 buf:返回时填充读取到的数据包
 *			 eleseq:数据包的序号
 *			 flag:数据包的标记
 * 返回值	:正值表示存入buf中的有效字节数,负值表示出错
 **********************************************************************************************/
static __inline__ int read_media_resource(IN media_source_t *media,OUT void *buf,IN int buf_len,OUT int *eleseq,OUT int *flag)
{
	return MShmPoolGetResource(&media->mpool,buf,buf_len,eleseq,flag);
}

/**********************************************************************************************
 * 函数名	:read_media_remain()
 * 功能	:返回缓冲区内的可用元素数
 * 输入	:meida:描述已连接的媒体缓冲池结构指针
 * 返回值	:缓冲区内的元素数
 **********************************************************************************************/
static __inline__ int read_media_remain(IN media_source_t *media)
{
    return MShmEleRemain(&media->mpool);
}

/****************************************************************************/
/*******************************写操作的接口**************************/
/****************************************************************************/

/**********************************************************************************************
 * 函数名	:create_media_write()
 * 功能	:创建共享缓冲池(对缓冲池进行写操作的程序调用)
 * 输入	:key:共享内存缓冲池的key
 *			 name:创建者的名字
 *			 size:准备创建的缓冲池大小(byte)
 * 输出	 media:返回时填充好相应的信息
 * 返回值	:0表示成功,负值表示失败
 **********************************************************************************************/
int create_media_write(OUT media_source_t *media,IN int key,char *name,int size);

/**********************************************************************************************
 * 函数名	:set_media_attrib()
 * 功能	:设置媒体资源属性
 * 输入	:media:描述媒体信息的结构指针
 *			 attrib:要设置的属性信息缓冲区
 *			 att_len:attrib中的有效字节数
 * 输出	 media:返回时填充好相应的信息
 * 返回值	:att_len表示成功,负值表示失败
 **********************************************************************************************/
int set_media_attrib(IO media_source_t *media,IN void *attrib,IN int att_len);

/**********************************************************************************************
 * 函数名	:set_media_timeout_num()
 * 功能	:设置媒体缓冲池中判断用户超时的元素数
 * 输入	:media:描述媒体信息的结构指针
 *                   num:判断超时的最大元素数
 * 返回值	:0表示成功,负值表示失败
 **********************************************************************************************/
static __inline__ int set_media_timeout_num(IO media_source_t *media,IN int num)
{
    return MShmPoolSetMaxNum(&media->mpool,num);
}

static __inline__ void set_media_timeout_num_t(IN int num)
{
	MSHmPoolSetMaxNum_t(num);
}

/**********************************************************************************************
 * 函数名	:write_media_resource()
 * 功能	:向媒体缓冲池中写入一块
 * 输入	:media:描述媒体信息的结构指针
 *			 buf:要写入的数据缓冲区指针
 *			 buf_len:buf中有效的字节数
 *			 flag:数据包的标记
 * 输出	 media:返回时将buf中的内容填充到相应位置
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/
static __inline__  int write_media_resource(IO media_source_t *media,IN void *buf, IN int buf_len,IN int flag)
{
	return MShmPoolAddResource(&media->mpool,buf,buf_len,flag);
}
#endif
