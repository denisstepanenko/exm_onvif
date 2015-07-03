/**  @file	gt_netsdk_ip.h
 *   @brief 	IP1004报警服务集成件网络SDK接口库函数
 *   		本函数库提供了通过tcp命令端口登陆设备并模拟海康等SDK库请求实时视频数据
 *   @date 	2013.12
 */

/*	ChangeLog
	1.0.10  �޸ķ����ļ�ͷtype��frametype_ƥ��
		    1.0.0.9 : 增加获取视频通道名称功能，通过下载设备ip1004.ini并解析实现
        1.0.0.8 : 根据媒体服务器的需求进行接口调整
        1.0.0.7 : 下行音频加入通道选项
        1.0.0.6 : 加入录像的信令控制与接收数据部分
        1.0.0.5 : 对实时部分完善，进行了20小时左右的压力测试，反复开关视频，现象正常
        1.0.0.4 : 在注销设备时将相关连接关闭，对回调函数数组加锁防止崩溃(已并发测试)
        1.0.0.3 : 加入下行音频请求支持，每台设备只支持一路
 	1.0.0.2 : 加入设备信息获取接口，将实时订阅/退订变为多通道支持，同时加锁保证线程安全
        1.0.0.1	: 首先实现库初始化/反初始化，用户登陆/退出，订阅/停止订阅
*/

#ifndef GT_NETSDK_IP_H
#define	GT_NETSDK_IP_H
#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/time.h>
#include <typedefine.h>
#endif
#include <gt_netsdk_errno.h>
#include <gt_netsdk_exdef.h>
#include <gtsf.h>

#undef IN
#undef OUT
#undef IO

#define	IN
#define	OUT
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
  typedef struct gt_av_handle_ gt_dev_av_handle_t;

#ifdef _WIN32
  //windows 使用
#define EXPORT_DLL __declspec(dllexport)
#else
  //linux 使用
#define EXPORT_DLL
#define CALLBACK
  typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
  } SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
#endif

  //接口函数与符号定义
  typedef void * gt_dev_handle_t;
  typedef void * gt_session_handle_t;

  typedef enum{
    FRAMETYPE_V = 1,          ///< 视频帧
    FRAMETYPE_A,              ///< 音频帧
    FRAMETYPE_H,
  }frametype_t;

  /**
   * @brief     实时回调
   * @param     real_handle 调用gt_require_rt_av_service获得的实时播放句柄
   * @param     frame_buf   帧数据
   * @param     len         帧数据长度
   * @param     type        帧数据类型
   * @return
   */
  typedef void (CALLBACK *Rtframe_Callback)(gt_session_handle_t real_handle, void *frame_buf, int len, frametype_t type);

  /**
   * @brief     录像回调
   * @param     playback_handle  调用gt_require_pb_av_service得到的录像回放句柄
   * @param     frame_buf        帧数据
   * @param     len              帧数据长度
   * @param     type             帧数据类型
   * @param     format           帧数据的格式信息，具体查看gtsf.h
   * @return
   */
  typedef void (CALLBACK *Pbframe_Callback)(gt_session_handle_t playback_handle, void *frame_buf, int len,
                                            frametype_t type, stream_format_t *format);

  /**
   * @brief     录像回放结束回调
   * @param     playback_handle  调用gt_require_pb_av_service得到的录像回放句柄
   */
  typedef void (CALLBACK *Pbfinish_Callback)(gt_session_handle_t playback_handle);

  /**
   * @brief     初始化netsdk，一个运行实例中调用1次
   * @param     none
   * @return	0:初始化成功，<0出错
   */
  EXPORT_DLL int gt_netsdk_init(int pb_listen_port);

  /**
   * @brief    去初始化netsdk
   * @param    none
   * @return   0:去初始化成功，<0出错
   */
  EXPORT_DLL int gt_netsdk_uninit(void);

  /**
   * @brief    登录到远程设备,创建设备描述结构
   * @param    dev_ip     设备ip字符串
   * @param    dev_port   设备命令端口号
   * @param    env        是否使用数字证书 1:使用 0:不使用
   * @param    usrname:
   *               当env=0时表示用户名字符串 NULL表示不需要用户名
   *               当env=1时表示数字证书公钥文件名
   * @param    passwd:
   *               当env=0时表示密码字符串 NULL表示不需要密码
   *               当env=1时表示数字证书私钥文件名
   * @return   指向远程设备的句柄指针,NULL表示失败，进行对设备操作时直接使用这个句柄；
   * 注意:env=1时 usrname和passwd不能为空
   */
  EXPORT_DLL gt_dev_handle_t gt_register_dev(IN const char *dev_ip, IN int dev_port, IN int env,
                                             IN const char *usrname, IN const char *passwd);

  /**
   * @brief  将 login_handle 销毁相关结构, 必须在所有相关链接断开后调用
   */
  EXPORT_DLL int gt_unregister_dev(IN gt_dev_handle_t login_handle);

  /**
   * @brief      异步请求实时视频，将对应handle加入传输管理链表
   * @parm       login_handle 调用gt_register_dev获得的设备句柄
   * @parm       channel      请求通道号,
   * @parm       audioenable  是否需要音频
   * @parm       head_buf     返回的avi头,
   * @parm       head_len     返回的avi头长度
   * @parm       callback     每一帧的回调函数,Rtframe_Callback类型
   * @return     返回实时播放的real_handle值，为NULL表示失败
   */
  EXPORT_DLL gt_session_handle_t gt_require_rt_av_service(IN gt_dev_handle_t login_handle, IN int channel, IN int audioenable,
                                                          IN Rtframe_Callback callback);

  /**
   * @brief     停止实时视频，从传输管理数组删除
   * @parm      real_handle  调用gt_require_rt_av_service获得的实时播放句柄
   * @return    是否成功停止请求，0正常 <0 异常
   */
  EXPORT_DLL int gt_stop_rt_av_service(gt_session_handle_t real_handle);

  /*
    typedef enum
    {
    QSPEED = 0,
    HSPEED,
    NSPEED,
    DSPEED,
    ESPEED,
    ISPEED
    }transpeed_t;
  */
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

  /**
   * @brief       请求录像视频，将对应handle加入传输管理链表
   * @param       login_handle   调用gt_register_dev得到的连接句柄
   * @param       channel        请求通道号,speed播放速率,start/end播放时间段
   * @param       speed          回放速率，但是取值范围呢？
   * @param       start          回放起始时间
   * @param       end            回放结束时间
   * @param       callback       帧数据回调函数,Pbframe_Callback类型
   * @param       fini_callback  回放结束回调, Pbfinish_Callback类型
   * @return      该设备录像请求的playback_handle值，等于NULL为失败
   */
  EXPORT_DLL gt_session_handle_t gt_require_pb_av_service(IN gt_dev_handle_t login_handle, IN int channel, IN int speed,IN unsigned char* local_ip,
                                                          IN timepoint_t* start, IN timepoint_t* end,
                                                          IN Pbframe_Callback callback, IN Pbfinish_Callback fini_callback);

  /**
   * @brief     停止事实视频，从传输管理链表中删除
   * @parm      playback_handle  调用gt_require_pb_av_service获取的录像回放句柄
   * @return    是否成功停止请求，0正常 <0 异常
   */
  EXPORT_DLL int gt_stop_pb_av_service(IN gt_session_handle_t playback_handle);

  typedef enum record_ctrlcmd
    {
      CPAUSE = 0,
      CRESUME,
      CSPEED,
      CSEEK,
    } record_ctrlcmd_t;

  typedef struct
  {
    record_ctrlcmd_t ctrl;
    int              speed;
    timepoint_t      start;
    timepoint_t      end;
  } record_ctl_t;

  /**
   * @brief    录像播放控制
   * @parm     playback_handle   调用gt_require_pb_av_service得到的录像回放句柄
   * @parm     cmd               record_ctrlcmd类型，停止，继续，速度，定位等
   * @return   是否成功停止请求，0正常 <0 异常
   */
  EXPORT_DLL int gt_ctrl_pb_av_service(IN gt_session_handle_t playback_handle, IN record_ctl_t* cmd);

  /**
   * @brief    查询录像索引，返回录像索引列表
   * @parm     login_handle   调用gt_register_dev得到的连接句柄
   * @parm     start          起始时间
   * @parm     end            结束时间
   * @parm     channel        通道号
   * @parm     index_path     存放传出的录像索引路径
   * @parm     name_buf_len   传入数组长度
   * @return   是否有录像，0正常 <0 没有
   */
  EXPORT_DLL int gt_query_ftp_record(IN gt_dev_handle_t login_handle, IN timepoint_t *start, IN timepoint_t *end,
                                     IN int channel, OUT char *index_path, IN int name_buf_len);

  /**
   *   @brief     获取最后一次发生错误的错误码
   *   @param     无
   *   @return    最后一次发生错误的错误码
   */
  EXPORT_DLL int gt_get_last_error(void);

  /**
   *   @brief     设置tcp连接超时时间(全局)
   *   @param     timeout  判断超时的时间(秒)，默认值10s
   *   @return    无
   *              应用程序如果需要改变默认值则启动时调用一次即可
   */
  EXPORT_DLL void gt_set_connect_timeout(IN int timeout);

  /**
   *   @brief     获取tcp连接超时时间(全局)
   *   @param     无
   *   @return    timeout 判断超时的时间(秒)
   */
  EXPORT_DLL int gt_get_connect_timeout(void);

  /**
   *   @brief    获取设备的注册信息
   *   @param    login_handle  调用gt_register_dev得到的连接句柄;
   *   @param    info          存放设备相关信息
   *   @return   0表示成功,负值表示失败
   */
  EXPORT_DLL int gt_query_regist_info(IN gt_dev_handle_t login_handle, OUT dev_regist_info_t *info);

  /**
   *   @brief     开始订阅音频下行服务
   *   @param	  login_handle  调用gt_register_dev得到的连接句柄
   *   @param     speak_port    设备端的音频连接端口 8097
   *   @param     channel       下行音频通道号
   *   @param     encoder       编码方式，现取1
   *   @param     sample_rate   音频数据采样率
   *   @return    返回下行语音的speak handle值, 为NULL表示失败
   */
  EXPORT_DLL gt_session_handle_t gt_require_speak_service(IN gt_dev_handle_t login_handle, IN int channel, IN int speak_port,
                                                          IN int encoder, IN int sample_rate);

  /**
   *   @brief   向设备发送下行音频数据
   *   @param   speak_handle  调用gt_register_dev得到的连接句柄
   *   @param   frame_buf     音频数据缓冲区buf_len音频数据长度
   *   @return	负值表示出错,正值表示发送数据长度
   */
  EXPORT_DLL int gt_write_speak_data(IO gt_session_handle_t speak_handle, IN BYTE *frame_buf, IN int buf_len);

  /**
   *   @brief     停止订阅音频下行服务
   *   @param     speak_handle  调用gt_require_speak_service得到的连接句柄
   *   @return    0表示成功，负值表示失败
   */
  EXPORT_DLL int gt_stop_speak_service(IO gt_session_handle_t speak_handle);

#ifdef __cplusplus
} // extern "C"
#endif


#endif
