/*
 * RWlocic.h
 * 硬盘总体的读写逻辑控制
 *  Created on: 2013-11-6
 *      Author: yangkun
 */

#ifndef RWLOCIC_H_
#define RWLOCIC_H_

#include <string>
#include "Blocks.h"
#include "YearBlocks.h"
#include "DayBlocks.h"
#include "SecBlocks.h"
#include "FrameQueue.h"
#include <queue>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <fstream>
#include "MultMediaBlocks.h"
using std::queue;
using  std::string;
namespace gtsda
{
class RWlogic;



enum rw_type
{
	read_type  = 1, /*从硬盘读出数据*/
	write_type = 2, /*把数据写入硬盘*/
	cmd_type   = 3, /*处理命令*/
	read_media_data  =4   /*从缓冲区读出需要的音视频数据*/
};
struct pthread_arg
{
	rw_type type;		//读写类型
	int channel;		//通道，只在read_media_data时有用
	RWlogic *wr;
};
//命令端口
#define CMD_PORT 60000
#define MSG_HEAD  0x5a5aa5a5
enum msg_type
{
	request_time = 0xa5a5a501,/*请求录像时间段*/
	cmd_play	 = 0xa5a5a502,/*发送播放命令*/
	cmd_ret_ok   = 0xa5a5a503,/*返回成功*/
	cmd_ret_err  = 0xa5a5a504  /*返回失败*/
};
struct msg_st
{
	unsigned msg_head;
	msg_type msgtype;
	unsigned int msg_len; 		/*消息长度*/
}__attribute__ ((packed));
class RWlogic
{
public:
	static RWlogic* newRW(bool bMarkWR=true/*write*/);
	virtual ~RWlogic();
	void start(int starttime=0/*读取的时候，读的开始时间，此参数只在读的时候有效*/);
	int Init();
	void Stop(){bIsRun = false;};
	bool IsRun(){return bIsRun;}
	//写
	int writedata();
	int maybe_cover_dayblock(long long blocks, long long seek);
	void format();
	//读硬盘
	//读出所有数据，返回有效的时间段
	int read_disk_print_record_time(string &timestring);
	//从指定的时间段内查询有效数据，并返回有效的时间段
	int get_time_index(int timestart,int timeend, string &timeindex);
	//返回一天的数据
	static void printOneDay(const struct day_block *dDayData, int todaytime,int iStartTime , int iEndTime, string &timestring);
	long long  is_in(int starttime);
	//处理交互命令
	void cmd_proc();

	//从音视频缓冲区读出数据
	void read_pool_proc(int channel);
	static void write_for_rtsp(int )
	{

		static int times;
		cout << "size: " << frame_queue_read_video1.size()<< endl;
		if(frame_queue_read_video1.size()>0)
		{
			if( times++ > 3){cout << "write error!!\n";exit(1);}
			cout << "size: " << frame_queue_read_video1.size()<< endl;
			Blocks *bs = frame_queue_read_video1.pop();
			//ttt();
			cout << "buff size: "<< bs->GetSize() << endl;
			 dw->getdata( (const char *)bs->GetBuf(),bs->GetSize() );
			// ttt();
			dw->writedata();
			 times=0;
			 delete bs;
			 //ttt();
			 usleep(40000);
		}
		else
			ttt();
	}
	#define REALTIME
	void init_sigaction(void)
	{
		struct sigaction act;

		act.sa_handler = write_for_rtsp;
		act.sa_flags   = 0;
		sigemptyset(&act.sa_mask);
	#ifdef REALTIME
		sigaction(SIGALRM, &act, NULL);
	#else
		sigaction(SIGPROF, &act, NULL);
	#endif
	}
	/* init */
	void init_time(void)
	{
		struct itimerval val;

		val.it_value.tv_sec = 0;
		val.it_value.tv_usec = 40000;
		val.it_interval = val.it_value;
	#ifdef REALTIME
		setitimer(ITIMER_REAL, &val, NULL);
	#else
		setitimer(ITIMER_PROF, &val, NULL);
	#endif
	}
	//线程相关
	static void * create_thread(void *args);
	void printids(const char *s);

private:
	RWlogic(bool bMarkWR);
	YearBlocks *yb;
	DayBlocks *db;
	DayBlocks *dbread;
	DayBlocks *dbbac;//day_bac_block
	//SecBlocks *hb;
	//DataRead *dr;
	static DataWrite *dw;
	static char   *hd;
	bool bMarkWR;/*true write;  false read*/
	bool bIsRun;
	long long HdSize;//num of blocks
	static FrameQueue<Blocks *> frame_queue_read_video1;
	static FrameQueue<Blocks *> frame_queue_read_audio1;
	//读硬盘
	long long readseek;
	int starttime;
	void setreadseek(long long seek){readseek=seek;};
	//线程相关
	pthread_t read_pid,write_pid ,cmd_pid;
	//读
	pthread_mutex_t lock_start_read_again;
	pthread_cond_t start_read, stop_read;
	//消息队列
	int msgid;

	//数据写入硬盘
private:
	//多路音视频读后组成预定的数据包通过队列保存
	FrameQueue<MultMediaBlocks *> mb_video_queue[MAX_CHANNEL];
	FrameQueue<MultMediaBlocks *> mb_audio_queue[MAX_CHANNEL];
	pthread_mutex_t lock;
	pthread_cond_t start_again;



/*************************************************************************
 *	多路音视频读出：
 *	每路读出要创建两个线程，一个从硬盘读数据放到队列中；一个从队列中读数据发出
*************************************************************************/
public:
	class ReadDisk
	{
#define TESTFRAMERATE	//计算帧率
//#define WRITEHD			//把从硬盘读出来的音视频数据保存成文件
//#define NETSEND			//把从硬盘读出来的音视频数据通过网络发送走

	private:
		long long readseek;
		unsigned int timecount;
		//第音视频队列
		FrameQueue<Blocks *> video_queue;
		FrameQueue<Blocks *> audio_queue;
		//MediaPoolWrite *audio_pool;
		//MediaPoolWrite *video_pool;
		//记录有几个实例
		static unsigned int num;
#ifdef TESTFRAMERATE
		//帧率控制
		int framenum;
		struct timeval oldtime,nowtime;
#endif

		//线程锁与同步
		pthread_mutex_t lock;
		pthread_cond_t notfull;
		pthread_cond_t startplay;
		//播放控制
		int channel;
	public:
		void set_notful()
		{
			//pthread_mutex_lock(&lock);
			pthread_cond_signal(&notfull);
			//pthread_mutex_unlock(&lock);
		};
		void start_play()
		{
			//pthread_mutex_lock(&lock);
			isRead = true;
			pthread_cond_signal(&startplay);
			//pthread_mutex_unlock(&lock);
		};
	public:
		//记录这是第几个
		unsigned int thenum;

		int get_date(char *data, int *size,int type,int *isi);
		//播放时间控制
		int framecount;
		int count;
		bool isRead;
#ifdef NETSEND

		int state; //当前的表状态0表示没有准备好，1占用资源，2准备好了
		int oper;
		int peeraddr[32]; //连接的对端地址
		short peerport; //连接的对端端口
		int speed;
		int socket; //本地发送的socket
#endif





		//线程相关
		pthread_t ntid;
		ReadDisk(long long seek);
		~ReadDisk();
		//读
		void read_proc();
		int readdataprocess(SecBlocks *video, SecBlocks *audio);
		void setread(unsigned int channel,long long seek,int framenum=0)
		{
			this->channel=channel;
			readseek=seek;
			framecount=framenum;
		};
		FrameQueue<Blocks *> *get_audio(){return &audio_queue;};
		FrameQueue<Blocks *> *get_video(){return &video_queue;};
		//把读到内存的数据通过网络发送出去，或直接写硬盘
		void write_proc();

		//清空音视频缓冲区
		void clear(){video_queue.clear();audio_queue.clear();set_notful();};

	};

};

} /* namespace gtsda */
#endif /* RWLOCIC_H_ */
