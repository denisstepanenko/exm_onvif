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
using std::queue;
using  std::string;
namespace gtsda
{
class RWlogic;
enum rw_type
{
	read_type  = 1,
	write_type = 2,
	cmd_type   = 3
};
struct pthread_arg
{
	rw_type type;
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
	//读
	void read_proc();
	int readdata(int starttime);
	int readdataprocess(char *buf, unsigned buffsize, long long &seek);
	int read_disk_print_record_time(string &timestring, bool isweb=false);
	static void printOneDay(const struct day_block *dDayData, int iBeginTime ,string &timestring, bool isweb);
	bool is_in(int starttime);
	//处理交互命令
	void cmd_proc();

	static void write_for_rtsp(int )
	{
		static int times;
		cout << "size: " << frame_queue.size()<< endl;
		if(frame_queue.size()>0)
		{
			if( times++ > 3){cout << "write error!!\n";exit(1);}
			//cout << "size: " << frame_queue.size()<< endl;
			Blocks *bs = frame_queue.pop();
			//ttt();
			 dw->getdata( (const char *)bs->GetBuf(),bs->GetSize() );
			// ttt();
			 dw->writedata();
			 times=0;
			 delete bs;
			 //ttt();
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
	static void *create_thread(void *args)
	{
		struct pthread_arg pa;
		struct pthread_arg *pa1 = static_cast< pthread_arg * >(args);
		ttt();
		memcpy(&pa, pa1, sizeof(struct pthread_arg));
		ttt();

		if(pa.type==write_type)
			cout <<"this is write"<<endl;
		else
			cout << "this is read" << endl;
		cout << "isRun: " << pa.wr->IsRun() << endl;

		if(pa.type==write_type)
		{
			cout <<"this is write"<<endl;
			pa.wr->printids("write pthread\n");
			pa.wr->writedata();
			cout <<"write process  exit"<<endl;
		}
		else if(pa.type==read_type)
		{
			cout <<"this is read"<<endl;
			pa.wr->printids("read pthread\n");
			pa.wr->read_proc();
			cout <<"read process  exit"<<endl;
		}
		else if(pa.type==cmd_type)
		{
			cout << "this is cmd" << endl;
			pa.wr->printids("cmd pthread\n");
			pa.wr->cmd_proc();
			cout <<"cmd process  exit"<<endl;
		}

		ttt();
		gtlogwarn("proc %d exit!\n",pa.type);
		cout<< "proc " << pa.type << " exit!\n";

		pthread_exit(NULL);
		//f->thread();
		return NULL;
	}
	void printids(const char *s)
	{
		pid_t      pid;
		pthread_t  tid;

		pid = getpid();
		tid = pthread_self();
		printf("%s pid %u tid %u (0x%x)\n", s, (unsigned int)pid,
		       (unsigned int)tid, (unsigned int)tid);
	}
private:
	RWlogic(bool bMarkWR);
	YearBlocks *yb;
	DayBlocks *db;
	DayBlocks *dbread;
	DayBlocks *dbbac;//day_bac_block
	SecBlocks *hb;
	DataRead *dr;
	static DataWrite *dw;
	static char   *hd;
	bool bMarkWR;/*true write;  false read*/
	bool bIsRun;
	long long HdSize;//num of blocks
	static FrameQueue frame_queue;

	//读硬盘
	bool isRead;
	int starttime;
	long long readseek;
	//线程相关
	pthread_t read_pid,write_pid ,cmd_pid;
	//读
	pthread_mutex_t lock_start_read_again;
	pthread_cond_t start_read, stop_read;
	//消息队列
	int msgid;

	//缓冲区
	media_source_t	media;	//与池子有关
#define VIDEO_POOL_SIZE_SAVE 512*1024			//缓冲区总大小
#define BUFFER_SIZE  40*1024					//帧缓冲大小
	void init_audio_pool();
	void write_pool(char *buf, unsigned int buffsize,bool is_key);
	enc_frame_t *the_frame_buffer;
};

} /* namespace gtsda */
#endif /* RWLOCIC_H_ */
