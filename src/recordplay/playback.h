#ifndef PLAYBACK_H
#define PLAYBACK_H
#include "avilib.h"
#include <gate_cmd.h>


#define VERSION         "0.06"
/*********************************************版本描述****************************************/
//ver:0.01  创建文件
//ver:0.02  被动模式增加网络监测,不然程序会崩溃
//ver:0.03  现象:使用VLC反复连接某台设备时，出现跳帧的现象;
//              原因:在监测到连接关闭的消息后，关闭socket，但是发送线程关闭要有一定的延时
//              ，如果这时有新的连接，并且连接的fd和上一次的一样，就会造成上次的发送线程
//              没有关闭。
//ver:0.04  对gtsf头中，音频格式的描述存在问题，且无法兼容ip对讲模式，在此修复
//ver:0.05  解决对关闭连接时，select的处理问题
//ver:0.06  解决二次播放时，音频格式不对问题
/*****************************************版本描述结束**************************************/


#define         PLAYBACK_TRUE                    1
#define         PLAYBACK_FALSE                  0

#define PLAYBACK_NUM                                5
#define PLAYBACK_FRAMERATE_25              25
#define PRE_CONNECT_SEC_MAX                 30  /*预连接的秒数*/

#define PLAYBACK_STAT_IDLE                     0     /* 没有使用*/
#define PLAYBACK_STAT_USED                    1     /* 占用资源，但还没有准备好*/
#define PLAYBACK_STAT_OK                    2     /* 准备就绪OK*/

#define PLAYBACK_CTRL_IDLE                0     /* 无操作*/
#define PLAYBACK_CTRL_START             1     /* 打开网络*/
#define PLAYBACK_CTRL_CLOSE             2     /* 关闭网络*/
#define PLAYBACK_CTRL_PAUSE             3     /* 暂停播放*/
#define PLAYBACK_CTRL_RESUME             4     /* 重新播放*/
#define PLAYBACK_CTRL_SPEED             5     /* 设置速率*/
#define PLAYBACK_CTRL_SEEK             6     /* 查找功能*/


#define PLAYBACK_SOURCE_FILE                 0     /* 数据源是本地文件*/
#define PLAYBACK_SOURCE_POOL                1     /* 数据源是本地缓冲池*/


#define MAX_FILE_NAME_SIZE       256
#define PLAYBACK_BUFF_LEN         400*1024


#define PLAYBACK_SUCCESS                    0     /* OK*/
#define PLAYBACK_ERR_BUF_SIZE           -1     /*buffer错误，可能太小*/
#define PLAYBACK_ERR_PARAM               -2    /* 参数错误*/
#define PLAYBACK_ERR_USER_FULL        -3     /* 用户已满 */
#define PLAYBACK_ERR_NO_FILE            -4     /* 没有查到文件*/
#define PLAYBACK_ERR_FILE_ACCESS     -5     /* 没有查到文件*/
#define PLAYBACK_ERR_NO_OPEN           -6     /* 没有查到文件*/
#define PLAYBACK_ERR_CONNECT            -7     /* 现场连接错误*/
#define PLAYBACK_ERR_POLL_NODATA    -8     /* 现场没有数据*/
#define PLAYBACK_ERR_FILE_NOINDEX    -9     /* 回放没有索引文件*/



enum transpeed
{
    PLAYBACK_QSPEED = 0,
    PLAYBACK_HSPEED,
    PLAYBACK_NSPEED,
    PLAYBACK_2SPEED,
    PLAYBACK_4SPEED,
    PLAYBACK_ISPEED
    
};
/*
typedef enum
{
	PLAYBACK_CTRL_PAUSE,
	PLAYBACK_CTRL_RESUME,
	PLAYBACK_CTRL_SPEED,
	PLAYBACK_CTRL_SEEK
}recordctl_t;
*/

typedef struct hd_playback_struct{ 
    int           channel;
    int           sourcefrom;
    int           frames;                               //读取的总帧数
    char        Indexfilename[256];           //查询出来的索引文件名
    int           fileindex;                            //当前回放的录像文件的索引
    int           recordfiletotal;                   //本次回放的录像文件总数
    int           lastframe;                           // /*结束帧计数，只有播放到最后一个文件时使用*/
    FILE 	    *index_fp;                           //打开的索引文件名
    avi_t *     aviinfo;                               //当前录像文件信息
    struct gt_time_struct    starttime;  //起始订阅时间（有效时间段内）
    struct gt_time_struct    endtime;   //终止订阅时间（有效时间段内）    
    int	    start;                                   //回放的开始时间
    int	    stop;                                    //回放的结束时间
    int           framrate;                            //回放帧率
    int           state;                                  //当前的表状态0表示没有准备好，1占用资源，2准备好了
    int           oper;                                  //当前操作0表示关闭 1正在回放
    media_source_t media;                       //连接现场的时候使用
    int         pre_connect;                        //连接现场时预获取的时间
    char      peeraddr[32];                             //连接的对端地址
    short     peerport;                              //连接的对端端口
    int         speed;                                
    int         socket;                                 //本地发送的socket
    int         packetsum;                         //传输包计数
    int         playbackindex;                    //回放索引
	int         audio_source;                     //音频来源，暂时放此，理想方式是每个文件各自解析
    
}playback_struct;




typedef struct playback_open_struct{ 
unsigned int peer_ip;			//请求端ip地址
unsigned short peer_port;		//请求端监听port端口
unsigned short channel;		//0-31   录像通道号
unsigned int speed;			//播放速率Enum transpeed类型

struct gt_time_struct    starttime;  //起始订阅时间（有效时间段内）
struct gt_time_struct    endtime;   //终止订阅时间（有效时间段内）
    
}playback_open_struct;



int playbackInit();
int playbackClose(int index);
int playbackOpen(viewer_subscribe_record_struct *pplaybackopen);
//int playbackReadFrame(int playbackId, int start,  int stop);
DWORD get_playbackmodstat(void);
playback_struct * getplayback(int index);
int playbackfileOpen(viewer_subscribe_record_struct *pplaybackopen);
int playbackConnectOK(int playbackId, int fd);
#endif



