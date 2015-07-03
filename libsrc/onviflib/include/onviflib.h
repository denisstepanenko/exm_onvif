/**  @file	onviflib.h
 *   @brief onvif wrapper lib 
 *   @date 	2015.04
 */

/*	ChangeLog
 *	1.0.0.1 onviflib 初始版本，只有probe和请求音视频功能
*/

#ifndef ONVIF_LIB_H
#define	ONVIF_LIB_H
#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/time.h>
#define BYTE    unsigned char
#define WORD    unsigned short
#define DWORD   unsigned long
#endif

#define IN
#define OUT
#define IO

#ifndef BYTE
#define BYTE	unsigned char
#endif
#ifndef LONG
#define LONG	long
#endif
#define CBL(x,y)	{x,#x,y}
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
  //windows
#define EXPORT_DLL __declspec(dllexport)
#else
  //linux
#define EXPORT_DLL
#define CALLBACK
#endif
///////////////////////////--macro definition--/////////////////////////////////////////

//ipc设备发出的错误码

//SDK 相关错误码
#define	ERR_ONVIF_SDK_ERRNO_BASE		0x3000	///错误码基数
#define ERR_ONVIF_SDK_INVALID_ARGS    (ERR_ONVIF_SDK_ERRNO_BASE+1)  //参数错误
#define ERR_ONVIF_SDK_NO_SUCH_DEV     (ERR_ONVIF_SDK_ERRNO_BASE+2)  //不存在目标设备
#define ERR_ONVIF_SDK_NO_AUTH         (ERR_ONVIF_SDK_ERRNO_BASE+3)  //没有权限 
#define ERR_ONVIF_SDK_SESSION_EXIST   (ERR_ONVIF_SDK_ERRNO_BASE+4)  //实时session已经存在



//rtsp event notify
#define RTSP_STOPPED    0
#define RTSP_CONNECTING 1
#define RTSP_CONNFAIL   2
#define RTSP_CONNSUCC   3
#define RTSP_NOSIGNAL   4
#define RTSP_RESUME     5
#define RTSP_AUTHFAILED 6
#define RTSP_NODATA   	7



#define MT_VIDEO		  0x01	//视频数据
#define MT_AUDIO	      0x02	//音频数据

#define RATIO_D1_PAL       0x0      //720*576
#define RATIO_D1_NTSC      0x1      //704*576
#define RATIO_CIF_PAL      0x2      // 352*288
#define RATIO_CIF_NTSC     0x3      // 320*240
#define RATIO_720P         0x4      // 1280*720
#define RATIO_1080P        0x5      // 1920*1080
#define RATIO_ERR          0x10

//视频编码类型
#define         VT_MPEG4                     1
#define         VT_H264                      2


#define         AT_G711                      7
#define         AT_AAC                       255

//////////////////////--end-macro--///////////////////////////////////////////

//////////////////////--typedef--////////////////////////////////////////////

typedef struct video_mediainfo
{
    unsigned short  format;		//编码格式format
	unsigned short  ratio;        //分辨率
    int             framerate;
	int             en_interval;
	int             bitrate;
}video_mediainfo_t;

typedef struct audio_mediainfo
{
    unsigned short a_sampling;	//声音采样率
	unsigned char  a_wformat;	//声音格式
	unsigned char  a_channel;	//声音通道
	unsigned char  a_nr_frame;	//一包声音里面有几块数据
	unsigned char  a_bitrate;		//声音码流
	unsigned char  a_bits;		//音频采样位数
	unsigned char  a_frate;		//音频数据的帧率(没秒钟有几包音频数据)
}audio_mediainfo_t;

typedef struct timepoint
{
	WORD year;
	BYTE month;
	BYTE day;
	BYTE hour;
	BYTE minute;
	BYTE second;
	BYTE reserve;
}timepoint_t;

typedef unsigned int dev_handler_t;               //ipc device index
  
typedef enum{
    FRAMETYPE_V = 1,          ///<  video
    FRAMETYPE_A,              ///<  audio
    FRAMETYPE_H,
}frametype_t;
  
typedef struct
{
    unsigned int	ip;             // msg recv from
    int		type;           // device type
    char	EndpointReference[100];
	
    int     port;           // onvif port
    char    host[24];       // ip of xaddrs
    char    url[128];       // /onvif/device_service
}device_info_t;
//typedef struct deviceinfo
//{
//    unsigned int ip;
//    char reference[100];
//}device_info_t;

typedef enum streamtype
{
    STREAM_MAIN,
    STREAM_SEC
}stream_type_t;

/**
   * @brief     probe回调
   * @param     dev_index,代表设备的序号
   * @param     pinfo 是关于ipc设备信息的一块连续内存区域 
   * @param     count 表示内存区域中，device_info_t类型数据的个数 
   * @return
   */
  typedef void (CALLBACK *probe_callback)(dev_handler_t dev_index,device_info_t* pinfo);

  /**
   * @brief     视频数据回调
   * @param     data 视频帧数据
   * @param     len 视频帧长度
   * @param     ts 帧时间戳
   * @param     seq,帧序号
   * @param     usr_data 上层传递自定义数据
   */
  typedef int (CALLBACK *video_Callback)(unsigned char* data,int len,unsigned int ts,unsigned short seq,void* usr_data);

  /**
   * @brief     音频数据回调
   * @param     data 音频帧数据
   * @param     len 音频帧长度
   * @param     usr_data 上层传递自定义数据
   */
  typedef int (CALLBACK *audio_Callback)(unsigned char* data,int len,void* usr_data);

  /**
   * @brief     连接中事件回调
   * @param     event 事件号
   * @param     usr_data 上层传递自定义数据
   */
  typedef int (CALLBACK *notify_Callback)(int event, void * usr_data);
////////////////////////////-end typedef -///////////////////////////////////////////////////////////


//////////////////////////--interface--/////////////////////////////////////
  
  /**
   * @brief     初始化sdk，一个运行实例中调用1次
   * @param     none
   * @return	0:初始化成功，<0出错
   */
  EXPORT_DLL int onvif_lib_init();

  /**
   * @brief    去初始化sdk
   * @param    none
   * @return   0:去初始化成功，<0出错
   */
  EXPORT_DLL int onvif_lib_uninit(void);
  
  /**
   *   @brief     对ipc进行探测,异步进行
   *   @param     probe_cb 探测结果回调函数
   *   @param     usr_data 用户自定义数据
   *   @return    0表示成功，负值表示失败
   */ 
  EXPORT_DLL int onvif_lib_start_probe(IN probe_callback probe_cb,IN void* usr_data);
  
  /**
   *   @brief     设置任一设备的用户名，密码,内部会获取设备信息，同步执行
   *   @param     dev_index 设备序列号
   *   @param     usr_name/password 该设备的用户名密码
   *   @return    0表示成功，负值表示失败
   */ 
  EXPORT_DLL int onvif_lib_set_auth(IN dev_handler_t dev_index,IN const char* usr_name,IN const char* password);

  /**
   *   @brief     获取设备音视频码流格式信息同步执行
   *   @param     dev_index 设备序列号
   *   @param     type 码流类型 主/副
   *   @param     v_info , a_info ,音视频码流格式信息输出
   *   @return    0表示成功，负值表示失败
   */ 
  EXPORT_DLL int onvif_lib_get_av_mediainfo(IN dev_handler_t dev_index,IN stream_type_t type,IO video_mediainfo_t* v_info,IO audio_mediainfo_t* a_info);

  /**
   *   @brief     请求设备实时音视频数据,rtsp过程异步，rtp过程异步
   *   @param     dev_index,设备序列号
   *   @param     type 设备码流类型 主/副
   *   @param     vcb,acb 音视频数据，回调函数
   *   @param     usr_data 用户自定义数据
   *   @return    0表示成功，负值表示失败
   */ 
  EXPORT_DLL int onvif_lib_rtsp_start(IN dev_handler_t dev_index,IN stream_type_t type,IN video_Callback vcb,IN audio_Callback acb,IN notify_Callback ncb,IN void* usr_data);
	
  /**
   * @brief     停止实时音视频,rtsp过程同步
   * @param     dev_index,设备序列号
   * @param     type 设备码流类型 主/副
   * @return    是否成功停止请求，0正常 <0 异常
   */
  EXPORT_DLL int onvif_lib_rtsp_stop(IN dev_handler_t dev_index,IN stream_type_t type);
 
#ifdef __cplusplus
} // extern "C"
#endif


#endif
