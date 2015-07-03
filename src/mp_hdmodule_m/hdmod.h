#ifndef HDMOD_H
#define HDMOD_H

 //// lsk 2009-9-28  test gtvs3000 real D1  
#define  REAL_D1_TEST_FOR_3000      


#include <gate_cmd.h>
#include "rtpool.h"
#include <ime6410api.h>
#include <AVIEncoder.h>
#include <iniparser.h>
#include <avilib.h>
#include "fileindex.h"
#include "hdutil.h"
#include "sqlite3.h"
#include "mod_socket.h"



#define ALARMLOG_FILE           "/log/alarmlog.txt"
#define ALARMLOG_FILE_0         "/log/alarmlog.txt.0"
#define ALARMLOGFILE_MAX_SIZE  128   //报警信息文件最大长度，k为单位


#ifndef IN
#define IN
#define IO
#define OUT
#endif

struct alarm_log_struct{
BYTE time[24]; //时间部分,如<2005-12-14 13:13:13> 
BYTE type[8];  //类型部分，ack or alarm
BYTE data[12]; //数据及回车部分，如trig:0x0001
};

struct hd_enc_struct{               //高清晰存储对象数据结构
    pthread_mutex_t mutex;              
    pthread_t       thread_id;          //录像线程id
    pthread_t       audio_thread_id;    //录音线程id
    pthread_t       keyframe_pool_thread_id; //I帧缓冲池线程id
    pthread_mutex_t audio_mutex;        //是否连接音频的互斥体
    pthread_cond_t audio_cond;          //是否需要连接音频的条件变量
    pthread_mutex_t file_mutex;         //录像文件锁
    struct takepic_struct current_takepic; //记录现在正在抓图的结构
    int hdsave_mode;                //高清晰录像工作模式 1表示需要正常工作 0表示需要停止
    int audiochannel;               //音频录像通道
    int channel;                    //记录通道号
    int rec_type;                   //预录模式,0为一直预录,1为触发和移动录像模式
    int pre_rec;                    //预录时间(秒)
    int dly_rec;                        //延时录像时间
    int max_len;                    //文件切分长度(秒)
    int pre_connect;                    //需要提前连接的秒数(临时计算)
    int del_typ;                     //删除的方式，0为一直删，1为存一个删一个
    int state;                      //当前状态0表示空闲 1表示正在预录2表示正在报警录像
    char filename[100];             //当前录像文件名
    //AVIVarHeader avi;                 //当前录像文件信息
    avi_t * aviinfo;                    //当前录像文件信息
    int recordlen;                  //当前录像事件需要持续的时间(秒),没秒钟减1，到0表示录像停止
    int filelen;                        //当前文件的长度(秒)
    int trig;                       //当前的触发状态 
    int cutflag;                        //文件切割标志
    int qid;                        //队列id
    int keynumber;                  //用于创建队列的key
    int bitrate;                    //录像比特率，kbps为单位
    int watchcnt;           //用于监视录像线程从缓冲池取数据的计数器
    int keyframe_cnt;       //用于监视keyframe线程从缓冲池取数据的计数器
    int audio_cnt;          //用于监视音频编码器状态
    int readenc_flag;       //录像线程正在从缓冲池读数据的标志
    int picerrorcode;       //在录像过程中出现的编码器错误，供抓图用
    int takingpicflag;      //本通道是否在抓图
    int pictime;             //抓图进行了的秒数
    int timemax;             //抓图最多允许进行多少秒
    pthread_t takepic_thread_id;    //抓图线程的id
    int queuenumber;             //队列中数目
    sem_t sem;
    int semflag;
    //int restart6410times;
    struct pool_head_struct streampool; //缓冲池..
    char devname[30];       //设备节点,如"/dev/IME6410_D1"
    int queryflag; //若文件被查询了则置1，关闭时记得改变其文件长度
    //int create_file_flag; //磁盘有空间创建文件标志
    int remote_trig_time; //手工录像时间,为0则不录
    int remote_trigged; //手工录像触发状态,1为触发,0为无触发
    int alarmpicflag;       //报警抓图进行中置1，否则0
    int alarmpic_required;  //需要报警抓图置1，否则置0
    char alarmpic_path[60];     //报警抓图索引路径如"/picindex/alarmpic.txt"
    char partition[16];         //当前录像文件所在分区，如"/hqdata/hda3"
    int enable;                 //该路视频是否有效，若无效则不予以录像
    int threadexit;           //线程退出命令，如果想让线程退出，就置1
};



struct msg
{
        long msg_type;
        char name[100];
};

typedef struct {
gateinfo                gate;
struct takepic_struct   takepic;
}takepic_info;

//录声音的线程
void *record_audio_thread(void *hd);
//启动录音线程
int start_audio_thread(struct hd_enc_struct *hd_new);
void get_trig_status(void);
void get_save_status(void);
void dump_clearinfo_to_log(void);
struct pool_head_struct *get_stream_pool(int channel);
int init_hdenc(void);
struct hd_enc_struct    *get_hdch(int channel);
void hd_second_proc(void);
void *record_file_thread(void *hd);
void *takepic_thread(void * takepic);
//int remote_cancel_alarm(DWORD trig);
//int trig_record_event1(struct hd_enc_struct *hd,WORD trig,int reclen);
//处理远程发来的高清晰录像指令
int remote_start_record(struct hd_enc_struct *hd,int reclen);
//处理远程发来的停止高清晰录像指令
int remote_stop_record(struct hd_enc_struct *hd);
int query_record_index(char *indexname,int ch,time_t start,time_t stop,int trig);
int start_recordfilethread(struct hd_enc_struct *hd_new);
int stop_recordfilethread(struct hd_enc_struct *hd_new);
int restart_recordfilethread(struct hd_enc_struct *hd_new);
int get_hd_minval(void);
int usr_take_pic(gateinfo *gate, struct takepic_struct *takepic);
void change_thread_id(int channel,int id);
struct compress_struct *get_encoder(int channel);
//获取整数表示的hdmod状态
DWORD get_hdmodstatint(void);
int  get_time_before(struct timeval *timenow, int diff_in_msec,struct timeval *timebefore);
int clear_hdmod_trig_flag(int channel);
//获取报警抓图标志
int get_alarmpicflag(int channel);
//获取需要报警抓图标志
int get_alarmpic_required(int channel);
//获取正在处理抓图标志
int get_takingpicflag(int channel);
int set_takingpicflag(int value, int ch);
int set_alarmpic_required(int value, int ch);
int set_alarmpicflag(int value, int ch);
/*触发一次录像事件
  *hd:描述录像及采集设备的数据结构
  *trig:触发事件(录像原因)
  *reclen:希望进行多长时间的录像(实际录像时还会加上延时录像),传0即可
  */
int trig_record_event(struct hd_enc_struct *hd,WORD trig,int reclen);
//从配置文件中读取高清晰录像相关参数
int read_hqsave_para_file(char *filename,char *section, int channel);
int init_hdenc_ch(int channel);
int set_sys_encoder_flag(int encno,int inst_flag,int errnum);

//按照hd的信息关闭一个文件
int close_record_file(struct hd_enc_struct *hd);
//管理I帧缓冲池,供抓图使用的线程
void *keyframe_pool_thread(void *hd);

avi_t * create_record_file(struct hd_enc_struct *hd);


int convert_old_ing_files(void);


void hd_playback_en(void);
void hd_playback_cancel(void);

#endif



