#pragma once
#ifndef _RTSP_H_
#define _RTSP_H_

#ifdef  __cplusplus
extern "C" {
#endif

#define MAX_RTSP_VALUE_SIZE            (128)        ///定义的普通数组的最大长度

enum RtspContentsType
{
  PLAY,                 ///<播放、快放、慢放等
  PLAY_RESPONSE,        ///<播放、快放、慢放等的响应
  PAUSE,                ///<暂停
  PAUSE_RESPONSE,       ///<暂停响应
  TEARDOWN,             ///<停止
  TEARDOWN_RESPONSE,    ///<停止响应
  UNKNOWN_CONTENTS_TYPE ///<未定义
};

//当前仅仅支持两种回应:成功或者不接受
enum RtspResponseCode
{
  RESPONSE_CODE_SUCCESS,                          ///<回应对方执行成功
  RESPONSE_CODE_NOT_ACCEPTABLE,                   ///<回应对方该请求错误

  RESPONSE_CODE_UNKNOWN
};

enum RangeTimeType
{
  SMPTE,              ///<相对时间
  NPT,                ///<正常时间
  CLOCK,              ///<绝对时间
  UNKNOWN_TIME_TYPE   ///<未定义
};

///请根据RtspContentsType类型来查找相应的值，例如，当其为PLAY时，cseq、time_type、begin、end、scale可能有效，
///其他的不用关心
typedef struct RTSP
{
  enum RtspContentsType rtsp_type;
  enum RtspResponseCode response_code;
  char cseq[MAX_RTSP_VALUE_SIZE];                  ///在收到用户请求时，需要将此参数保存以在做出回应时设置到回应消息里面
  enum RangeTimeType time_type;                    ///参看上面的定义,用于PLAY消息头中
  char begin[MAX_RTSP_VALUE_SIZE];                 ///开始时间
  char end[MAX_RTSP_VALUE_SIZE];                   ///结束时间，与begin对应
  int scale;                                       ///默认为1

  char seq[MAX_RTSP_VALUE_SIZE];                   ///用于PLAY_RESPONSE的头字段的RTP_INFO中
  char rtp_time[MAX_RTSP_VALUE_SIZE];
  char url[MAX_RTSP_VALUE_SIZE];
} rtsp_t;


///应答对方的播放、快放、慢放、快倒、慢倒、逐帧进/退命令等命令
///此处需要传入回调回来时的CSeq值，url指定了seq与rtp_time所回应的那个流的URL，seq指定该流的第一个包的序号
///rtp_time 是针对收到的play命令中的开始时间与结束时间所生成的RTP时间戳，Play命令的发起者根据该时间戳计算
///RTP时间与NPT时间的映射。
///scale为自己实际的播放速度。Scale 为1，正常播放；不等于1，为正常播放速率的倍数；负数为倒放。
rtsp_t * gtaua_make_answer_play(char * cseq, enum RtspResponseCode code, char * seq, char * rtp_time, char * url, int scale);

///应答暂停请求
rtsp_t * gtaua_make_answer_pause(char * cseq, enum RtspResponseCode code);

///应答停止请求
rtsp_t * gtaua_make_answer_stop(char * cseq, enum RtspResponseCode code);

///释放通过上面的make方法创建的rtsp
void gtaua_free_rtsp(rtsp_t * contents);

//enum RtspContentsType gtaua_get_rtsp_contents_type(rtsp_t * contents);
//char * gtaua_get_cseq(rtsp_t * contents);
//enum RangeTimeType gtaua_get_time_type(rtsp_t * contents);               ///参看上面的定义,用于PLAY消息头中
//char * gtaua_get_begin_time(rtsp_t * contents);                          ///开始时间
//char * gtaua_get_end_time(rtsp_t * contents);                            ///结束时间，与begin对应
//int gtaua_get_scale(rtsp_t * contents);                                  ///默认为1
//char * gtaua_get_seq(rtsp_t * contents);                                 ///用于PLAY_RESPONSE的头字段的RTP_INFO中
//char * gtaua_get_rtp_time(rtsp_t * contents);
//char * gtaua_get_url(rtsp_t * contents);

#ifdef  __cplusplus
}
#endif

#endif

