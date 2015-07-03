/*
 * RWlocic.cpp
 *
 *  Created on: 2013-11-6
 *      Author: yangkun
 */

#include "RWlogic.h"
#include "wrap.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <cstdio>
# include <stdio.h>
#include "MultMediaBlocks.h"
#include "FrameQueue.h"
namespace gtsda
{
DataWrite *RWlogic::dw=NULL;


FrameQueue<Blocks *> RWlogic::frame_queue_read_video1;
FrameQueue<Blocks *> RWlogic::frame_queue_read_audio1;
unsigned int RWlogic::ReadDisk::num=0;
RWlogic::RWlogic(bool bMarkWR)
:bMarkWR(bMarkWR),bIsRun(false)
{
	// TODO Auto-generated constructor stub
	pthread_mutex_init(&lock,NULL);
	pthread_cond_init(&start_again,NULL);
	dbread = NULL;
}

RWlogic::~RWlogic()
{
	ttt();
	//pthread_cancel(write_pid );
	//pthread_cancel(read_pid );
	//pthread_cancel(cmd_pid );
	//pthread_join(write_pid, NULL);
	//pthread_join(read_pid, NULL);
	//pthread_join(cmd_pid, NULL);
	ttt();
	if(yb)
	{
		delete yb;
		yb = NULL;
	}
	if(dbread)
	{
		delete dbread;
		dbread = NULL;
	}
	if(dbbac)
	{
		ttt();
		delete dbbac;
		dbbac = NULL;
	}
	if(db)
	{
		ttt();
		delete db;
		db = NULL;
	}
/*	if(dr_video1)
	{
		delete dr_video1;
		dr_video1 = NULL;
	}
	if(dr_audio1)
	{
		delete dr_audio1;
		dr_audio1 = NULL;
	}*/
	ttt();
	free_sda();
	ttt();
}
//检测,如果要把天块覆盖了，把年块中最早的天块删了，备份天块。
/*****************************************************************
 * blocks:	此块占的块的数量
 * seek:	 此块开始的位置
 *****************************************************************/
int RWlogic::maybe_cover_dayblock(long long blocks,long long seek)
{
	int ret;
	struct seek_block sb;
	if( ( ret = yb->GetHead(sb)) < 0)//获取最早的天块
	{
		gtlogerr("@%s %d GetHead error:%d",__FUNCTION__ , __LINE__, ret);
		return ret;
	}
	if(sb.seek - seek>=0 &&\
			blocks>= sb.seek - seek)
	{
		gtloginfo("Warning:: the day block will be cover:seek:%lld,time:%d\n",sb.seek,sb.time);
		DayBlocks *db1 =new DayBlocks(sb.seek,false/*构造时不读硬盘*/,false/*不写入硬盘*/);
		*dbbac = *db1;
		delete db1;
		dbbac->write();
		cout << "debug dbbac seek: " << dbbac->GetSeek()<< endl;
		if( (ret=yb->del(sb) ) < 0)//删除最早天块
		{
			gtlogerr("@%s %d yearblock del earlist block error:%d",__FUNCTION__ , __LINE__, ret);
			return ret;
		}
	}
	return 0;
}
RWlogic* RWlogic::newRW(bool bMarkWR)
{
	return new RWlogic(bMarkWR);
}
/*把数据写到硬盘中*/
int RWlogic::writedata()
{
	gtloginfo("come to writedate");
	struct seek_block sb;//,earliest_day_block;
	int ret;
	int now,iOldTime;
	bIsRun = true;
	long long seek;
	//dr = new DataRead();
	//从天块中获取最后的
	gtlogdebug("db.seek:%lld",db->GetSeek());
	yb->GetTail(sb);
	iOldTime = sb.time;
	if( (ret = db->GetTail(sb) ) != 0 ) //sb里面是当前时间的偏移量
	{
		print_err(ret);
		gtlogerr("db getTail err:%d",ret);
		seek = db->GetNext();
		iOldTime = gettime();//添加了补丁，如果不这样可能会出现两天
	}
	else
	{
		seek = sb.seek;
		//iOldTime = sb.time;  //判断是不是同一天的时间从年块中取得了
	}

	//把多个音视频帧全部放到一块的缓冲区
	MultMediaBlocks *mb_all;
	MultMediaBlocks *mb_video;
	MultMediaBlocks *mb_audio;

	SecBlocks *hb_head;/*一个音视频块集合，只占一个BLOCKSIZE,包含其中音视频块的索引和大小*/
	SecBlocks *hb_video;/*第一路视频集合*/
	SecBlocks *hb_audio;/*第一路音频集合*/


	//打印出来刚开始要写入的seek
	gtloginfo( " current seek: %lld" , seek);

	while(bIsRun)
	{
		pthread_mutex_lock(&lock);
		//fprintf(stderr, "start again 1\n");
		pthread_cond_wait(&start_again, &lock);
		//fprintf(stderr, "start again 2\n");
		pthread_mutex_unlock(&lock);

		cout << endl << endl;
		now = gettime();
		cout << "now: "<< now << endl<< endl<< endl;

		//判断今天是不是过去了
		if(now/SECOFDAY != iOldTime/SECOFDAY )
		{
			now = gettime();
			long long seek_tmp= db->GetBlocks();
			delete db;
			db = NULL;
			sb.time=now;
			gtloginfo("the day is gone!!!");
			//判断剩下的空间是不是能够存下这个天块
			if(HdSize - seek < seek_tmp )
			{
				gtloginfo("the leave can't save one day blocks!!!");
				//不够存，就跳到硬盘开始的位置
				seek = first_block + Blocks::get_block_num(sizeof(struct year_block)) + \
						Blocks::get_block_num(sizeof(struct day_block))/*day back*/;
				//检测,如果要把天块覆盖了，把年块中最早的天块删了，备份天块。
				if ( ( ret = maybe_cover_dayblock(Blocks::get_block_num(sizeof(struct day_block)),seek) ) <0 )
				{
					gtlogerr("@%s %d maybe_cover_dayblock error:%d",__FUNCTION__ , __LINE__, ret);
					return ret;
				}
			}
			ttt();
			//cout << " this day seek: " << seek << endl;
			db = new DayBlocks(seek,false/*构造时不读硬盘*/,true/*要写入硬盘*/);//今天过去了，不从硬盘中读取天块，而直接构造
			//cout << "debug db.seek: " << db->GetSeek()<< endl;
#if 1
			db->write();
#else
			delete db;
			db = new DayBlocks(seek);
#endif
			sb.seek = seek;
			gtloginfo("year block add day block,seek:%lld,time:%d",sb.seek,sb.time);
			ret=yb->add(sb);
			ttt();
			seek += db->GetBlocks();
			//seek = db->GetNext();//与上一行相同的结果
			//cout << "this db getblocks: " << db->GetBlocks()<<endl;
			print_err(ret);
		}

		//硬盘是不是满了？
		if( HdSize - seek < LEAVE_BLOCKS )
		{
			gtloginfo("Warning:: disk is full ");
			//硬盘满了，年块中只有一天，说明硬盘不够存一天。
			if(1==yb->get_day_num())
			{
				gtloginfo("Warning:: capacity of disk can't save one day");
				*dbbac = *db;// warning
				dbbac->write();
				seek =  first_block + Blocks::get_block_num(sizeof(struct year_block)) + \
									Blocks::get_block_num(sizeof(struct day_block)) + \
									Blocks::get_block_num(sizeof(struct day_block));
			}
			else
			{
				gtloginfo("Warning:: capacity of disk can't save one day, turn to head");
				seek =  first_block + Blocks::get_block_num(sizeof(struct year_block)) + \
				Blocks::get_block_num(sizeof(struct day_block));
				//hb->SetSeek(seek);
			}
		}

		//检测,如果要把天块覆盖了，把年块中最早的天块删了，备份天块。
		if ( ( ret = maybe_cover_dayblock(LEAVE_BLOCKS,seek) ) <0 )
		{
			gtlogerr("@%s %d maybe_cover_dayblock error:%d",__FUNCTION__ , __LINE__, ret);
			return ret;
		}


		sb.time = now;
		sb.seek = seek;
		//设置seek，并在析构时写入硬盘
		long long seek_mult_media = seek;
		//构建了一个BLOCKSIZE的头集合
		mb_all = new MultMediaBlocks(BLOCKSIZE, multmedia);
		seek += 1;/* hb_head->GetBlocks() 其它它就占一块 */;


		unsigned int channel_mask=0;
		int queue_size;
		//分别从４个音视频通道，８个队列中取数据
		for(int i=0; i<MAX_CHANNEL; i++)
		{
			if( ( queue_size = mb_video_queue[i].size()) > 0 )
			{
				cout << "channel " << i << " video queue size: " << queue_size << endl;
				//处理视频块
				channel_mask |= (1<<i);
				mb_video = mb_video_queue[i].pop();
				hb_video = new SecBlocks(mb_video, mb_video->get_buff_size());
				delete mb_video;
				mb_video = NULL;
				//设置硬盘写入的seek
				hb_video->SetSeek(seek);
				//把视频集合的seek写入头块中
				mb_all->add_vidoe(seek,hb_video->GetSize()-sizeof(struct hd_frame),i);
				seek += hb_video->GetBlocks();
				//把数据存到硬盘中
				delete hb_video;
				hb_video = NULL;

				//处理音频块
				if( ( queue_size = mb_audio_queue[i].size() ) > 0 )
				{
					cout << "channel " << i <<  "audio queue size: " << queue_size << endl;
					mb_audio = mb_audio_queue[i].pop();
					hb_audio = new SecBlocks(mb_audio, mb_audio->get_buff_size());
					delete mb_audio;
					mb_audio = NULL;
					//设置硬盘写入的seek
					hb_audio->SetSeek(seek);
					//把视频集合的seek写入头块中
					mb_all->add_audio(seek,hb_audio->GetSize()-sizeof(struct hd_frame),i);
					seek += hb_audio->GetBlocks();
					//把数据存到硬盘中
					delete hb_audio;
					hb_audio = NULL;
				}
#ifdef DEBUG
				else
				{
					//！！！！！！！！！！！！！不存在有视频没有音频的情况！！！
					gtlogerr("mb_audio_queue %d had no data!!!!!!!!!!!!!!!!!");
				}
#endif
			}
#ifdef DEBUG
			else
			{
				gtlogwarn("mb_video_queue channel %d had no data");
			}
#endif
		}

		mb_all->add_next_mult_media_block_seek(seek);
		sb.time = channel_mask;
		hb_head = new SecBlocks( mb_all, sizeof(mult_media_block));
		delete mb_all;
		mb_all = NULL;
		//最后写入头块
		hb_head->SetSeek(seek_mult_media);
		delete hb_head;
		hb_head = NULL;
		cout << " head block seek: "<< seek_mult_media<< " blocks: "  << seek -seek_mult_media \
			 << " channel_mask: " << channel_mask <<endl;

		//把索引加入天块
		if(1)/*因为只有是Ｉ帧时才写入的*/
		{
			ret = db->AddBlock(sb,now);
			if(ret<0  )
			{
				gtlogerr("BLOCK_ERR_DAY_SEC_MUT:%d",ret);
				print_err(ret);
				if(BLOCK_ERR_DAY_SEC_MUT != ret)
				{
					cout<< "debug:: here will exit" << endl;
					gtlogerr("debug:: here will exit");
					bIsRun = false;
				}
			}
		}
		iOldTime = now;
		//delete hb;//释放内存
	}
	cout << "debug adfadf"<< endl;
	return 0;
}
/************************************************
 * 这个时间是不是在硬盘里面
 * 不在硬盘中返回０
 * 在硬盘中返回其seek
 ***********************************************/
long long   RWlogic::is_in(int starttime)
{
	//1、在年块中找这个时间属于哪一天。
	long long readseek = yb->TimeIn(starttime);
	if( readseek <= 0)
	{
		cout << "info:your input time not in year block"<< endl;
		readseek = dbbac->TimeIn(starttime);//也不在备份天块中
		if( readseek <= 0 )
		{
			cout << "err: you input time is not in back day block,and i will exit!!"<< endl;
			gtlogerr("err: you input time is not in back day block,and i will exit!!");
			return 0;
		}
	}
	else
	{
		if(dbread)
		{
			delete dbread;
			dbread =NULL;
		}
		ttt();
		dbread = new DayBlocks(readseek,true,false/*构造时读硬盘，析构时不写硬盘*/);
		ttt();
		readseek = dbread->TimeIn(starttime);
		ttt();
		if( readseek <= 0 )
		{
			cout << "err: you input time is not in  day block,and i will exit!!"<< endl;
			gtlogerr("err: you input time is not in  day block,and i will exit!!");
			return 0;
		}
	}
	return readseek;
}

void RWlogic::start(int starttime)
{
	static struct pthread_arg pa_write, pa_read, pa_cmd, pa_read_pool[MAX_CHANNEL];
	pa_write.wr = this;
	pa_read.wr  = this;
	pa_cmd.wr   = this;
	this->bIsRun = true;

	//初始化读相关
	pthread_mutex_init(&lock_start_read_again,NULL);
	pthread_cond_init(&start_read,NULL);
	pthread_cond_init(&stop_read,NULL);
	//分别创建从缓冲区读数据线程
	for(int i=0; i < MAX_CHANNEL; i++)
	{
		ttt();
		pa_read_pool[i].wr = this;
		pa_read_pool[i].type = read_media_data;
		pa_read_pool[i].channel = i;
		pthread_create(&write_pid, NULL, create_thread, &pa_read_pool[i]);
	}
	//创建写硬盘线程
	ttt();
	pa_write.type = write_type;
	pthread_create(&write_pid, NULL, create_thread, &pa_write);
	//创建读硬盘线程
	//pa_read.type = read_type;
	//pthread_create(&read_pid, NULL, create_thread, &pa_read);
	//创建命令线程
	//ttt();
	//pa_cmd.type = cmd_type;
	//pthread_create(&cmd_pid, NULL, create_thread, &pa_cmd);
}

int RWlogic::Init()
{
	ttt();
	int ret;
	if(init_sda()<0)
	{
		cout << "init sda error " << endl;
		exit(1);
		return 0;
	}
	else
		cout << "Init sda success!!!\n\n" ;
	//年块

	try{
		if(bMarkWR/*write*/)
			yb = new YearBlocks()/*要读也要写*/;
		else/*read*/
			yb = new YearBlocks(true, false)/*只读不写*/;
	}
	catch(int err)
	{
		gtlogwarn("catch error!!");
		print_err(err);
		if(BLOCK_ERR_DATA_HEAD == err )
		{
			gtlogwarn(" maybe this is new disk, and I will formate this disk!!!");
			if(bMarkWR)//写时可以格式化
			{
				try{
				yb = new YearBlocks(false)/*初始化时不读，析构时要写*/;
				dbbac = new DayBlocks(first_block+Blocks::get_block_num(sizeof(struct year_block)),false/*不读硬盘*/ , true/*要写入硬盘*/ );
				db = new DayBlocks(first_block + Blocks::get_block_num(sizeof(struct year_block)) + \
						Blocks::get_block_num(sizeof(struct day_block)), false/*不读硬盘*/ , true/*要写入硬盘*/);
				format();
				delete dbbac;
				dbbac = NULL;
				delete db;
				db = NULL;
				}
				catch(int err)
				{
					cout << "too much wrong !! and exit " << endl;
					exit(-1);
					cout << endl << endl;
				}
			}
			else//读的时候出错，直接返回-１;
			{
				cout << "read year block error!!" << endl;
				gtlogerr("read year block error");
				return -1;
			}
		}else
		{
			cout << "other error and I will exit" << endl;
			exit(1);
		}
	}



	//备份天块
	dbbac = new DayBlocks();
	//从年块中获取最后的天块
	struct  seek_block sb;
	if( ( ret = yb->GetTail(sb) ) < 0)
	{
		print_err( ret );//异常
		gtlogerr("year gettail err:%d\n",ret);
		//如果是空的
		if( ( BLOCK_ERR_EMPTY == ret ) && bMarkWR)
		{
			gtlogerr("year block is empty!!\n" );
			sb.seek = first_block + Blocks::get_block_num(sizeof(struct year_block)) + \
					Blocks::get_block_num(sizeof(struct day_block));
			sb.time = gettime();
			if( ( ret = yb->add(sb) ) < 0 )
			{
				print_err( ret );//异常
				gtlogerr("year add err:%d\n",ret);
				return ret;
			}
		}
		else
			return ret;
	}
	gtloginfo("current day seek:%lld,time:%d,%s",sb.seek ,sb.time, ctime((time_t *)&sb.time) );
	//获取天块
	db = new DayBlocks(sb.seek);
	HdSize = hd_getblocks();

	//init_audio_pool();

	return 0;
}
void RWlogic::format()
{
	gtloginfo("fromat");
	//formate year block
	if(NULL != yb)
	{
		gtloginfo("yearblock");
		struct year_block *yb = (struct year_block *)this->yb->GetBuf();
		memcpy(yb->year_head, YearBlocks::year_head, sizeof(YearBlocks::year_head));
		this->yb->write();
	}
	//formate day bac block
	if(NULL != dbbac)
	{
		gtloginfo("formate dbbac");
		struct day_block *dbbac = (struct day_block *)this->dbbac->GetBuf();
		memcpy(dbbac->day_head, DayBlocks::day_head, sizeof(DayBlocks::day_head));
		this->dbbac->write();
	}
	//formate day block
	if(NULL !=db )
	{
		gtloginfo("formate db");
		struct day_block *db = (struct day_block *)this->db->GetBuf();
		memcpy(db->day_head, DayBlocks::day_head, sizeof(DayBlocks::day_head));
		this->db->write();
	}
}


static int find(struct seek_block *seek_block_data ,int size, int value=0)
{
	int i;
	if(value==0)
	{
		for(i =0; i<size; i++)
		{
			if(seek_block_data[i].seek==0  && seek_block_data[i].time == 0)
				break;
		}
	}
	else
	{
		for(i =0; i<size; i++)
		{
			if(seek_block_data[i].seek!=0 || seek_block_data[i].time != 0)
				break;
		}
	}
	return i;
}
//#define NETSDK
static void write_str(string &timestring, time_t time,int is_start=1)
{
#ifndef NETSDK
	if(is_start==1)
				timestring += "start:\tblock: ";
	else
		timestring += "end:\tblock:";
				//timestring +=std::to_string( dDayData->seek_block_data[sec].seek) ;
				//timestring += "\ttime:  ";
				timestring += std::to_string( static_cast<long long>(time ));
	if(is_start==1)
				timestring += " --> ";
	else
		timestring += " --> ";
				timestring += ctime(&time  ) ;
#else
				struct tm *p=gmtime( &time  ) ;
				timestring +=std::to_string(static_cast<long long>(1900+p->tm_year)) ;
				char buf[5];
				//月
				sprintf(buf,"%02d",(1+p->tm_mon));
				timestring +=buf;
				//日
				sprintf(buf,"%02d",(p->tm_mday));
				timestring +=buf;
				//时
				sprintf(buf, "%02d", (p->tm_hour));
				timestring += buf;
				//分
				sprintf(buf, "%02d", (p->tm_min));
				timestring += buf;
				//秒
				sprintf(buf,"%02d",(p->tm_sec));
				timestring +=buf;
				if(is_start==1)
					timestring += '-';
				else
					timestring += '\n';
#endif
}
/***************************************************************************
 * 功能：输出今天的时间段
 * 参数：dDayData天块指针
 * 		iBeginTime开始时间
 ***************************************************************************/

#define VOIDBLOCK 2
void RWlogic::printOneDay(const struct day_block *dDayData,  int todaytime,int iBeginTime , int iEndTime,string &timestring)
{
	struct seek_block *seek_block_data = const_cast<struct seek_block *>(dDayData->seek_block_data);
	seek_block_data = seek_block_data + iBeginTime;
	int size = iEndTime-iBeginTime;
#define JG 3

	int num=0;
	int tmp;
	while(1)
	{
		if(num>=size)break;
		//找第一个
		tmp = find(seek_block_data+num,size-num, 1);
		num += tmp;
		//cout << num << "-->" ;
		write_str(timestring, todaytime+iBeginTime+num,1);
		//cout << "1iBeginTime: " << iBeginTime << " num: " << num << "todaytime: " << todaytime << endl;
		fflush(stdout);
		//找最后一个
		while(1)
		{
			if(num>=size)break;
			/* 1 1 1 0*/
			int tmp1=find(seek_block_data+num,size-num, 0);
			num+=tmp1;

			/*0 0 0 1*/
			tmp=find(seek_block_data+num, size-num,1);
			num+=tmp;
			if(tmp>=JG
#if 1
					||num==size  //这个条件很重要，说明已找到数组的最后了
#endif
					)
			{
				//cout << num-tmp-1 ;
				write_str(timestring, todaytime+iBeginTime+num-tmp,0);
				//cout << "2iBeginTime: " << iBeginTime << " num: " << num-tmp << "todaytime: " << todaytime << endl;
				break;
			}
		}
		cout << endl;
	}
}

/*************************************************************************
 * 功能：读硬盘上年块天块，输出录像时间
 *************************************************************************/
int RWlogic::read_disk_print_record_time(string &timestring)
{
	int tmp=0;
	int day, sec;
	int bool_tmp=0;
	if(yb==NULL){ttt();cout << "year block error" << endl;return -1;}

	//看看dbbac中是不是有数据,并输出
	//读dbbac中内容
#ifndef NETSDK
	timestring += "daydata_bak::\n";
#endif
	//printOneDay((const struct day_block *)(dbbac->GetBuf()), 0 ,timestring );
	cout << endl;


	//遍历年块中的数据；
	const struct year_block *yearblock =  (struct year_block *)yb->GetBuf();
#ifndef NETSDK
	timestring +=  "day of yearblock:";
	timestring += std::to_string( static_cast<long long>(yearblock->year_queue_data.queue_size) );
	timestring += "\n";
#endif
	bool_tmp=0;
	for(day=yearblock->year_queue_data.queue_head; \
		day < yearblock->year_queue_data.queue_head + yearblock->year_queue_data.queue_size; \
		day++)
	{
		if( yearblock->seek_block_data[day].seek==0 || yearblock->seek_block_data[day].time == 0)
		{
			cout << "worning :\tBLOCK_ERR_YEAR_PRINT\n";
			gtloginfo("worning :\tBLOCK_ERR_YEAR_PRINT\n");
			return -1;
		}
		cout << day<< " day block seek: " << yearblock->seek_block_data[day].seek << " time: " << yearblock->seek_block_data[day].time << endl;
#ifndef NETSDK
		timestring += std::to_string( static_cast<long long>(day+1-yearblock->year_queue_data.queue_head));
		timestring +="st day's time is: " ;
		timestring +=std::to_string( static_cast<long long>( yearblock->seek_block_data[day].time ));
		timestring += "\t-->";
		timestring += ctime((const time_t*)&( yearblock->seek_block_data[day].time ) );
#endif
		//cout << "today's block:\n";
		//读取天块的内容到内存中
		DayBlocks dayblock(yearblock->seek_block_data[day].seek,true/*构造时读硬盘*/,false/*析构时不写入*/);
		//cout << "seek of begin: " << (yearblock->seek_block_data[day].time)%SECOFDAY << endl;
		//输出本天内的连续块
		//if(day==yearblock->year_queue_data.queue_head+1)
#if 1
		printOneDay((const struct day_block *)(dayblock.GetBuf()),yearblock->seek_block_data[day].time/SECOFDAY*SECOFDAY, \
				/* (yearblock->seek_block_data[day].time)%SECOFDAY*/0 , SECOFDAY, timestring );
#else
		struct day_block *dbb=(struct day_block *)(dayblock.GetBuf());
		for(int i=0; i<SECOFDAY; i++)
		{
			if(dbb->seek_block_data[i].seek!=0)
				cout << "seek: " << dbb->seek_block_data[i].seek << " mask: " << dbb->seek_block_data[i].time << endl;
		}
#endif

	}

	return 0;
}
/*************************************************************************
* 功能：	从指定的时间段内查询有效数据，并返回有效的时间段
* 		只能查一天以内的时间段
* 参数：
* 返回:	0成功
* 		<0失败
**************************************************************************/
int RWlogic::get_time_index(int timestart,int timeend, string &timeindex)
{
	long long seek = yb->TimeIn(timestart);
	if(seek<0)
	{
		gtlogerr("timestart no in yearblock!!\n");
		return BLOCK_ERR_NOT_IN;
	}
	gtloginfo("debug %s:%d",__FILE__,__LINE__);

	//读取天块的内容到内存中
	DayBlocks dayblock( seek,true/*构造时读硬盘*/,false/*析构时不写入*/);
	int iTmpTime = (timestart%SECOFDAY) > (timeend%SECOFDAY)?SECOFDAY:(timeend%SECOFDAY);
	printOneDay((const struct day_block *)(dayblock.GetBuf()),timestart/SECOFDAY*SECOFDAY,\
			timestart%SECOFDAY , iTmpTime, \
			timeindex );
	gtloginfo("debug %s:%d",__FILE__,__LINE__);
	return 0;
}
/*处理控制命令*/
void RWlogic::cmd_proc()
{
	string returntring;
	msg_st msg,msg_s;
	char *msg_content=NULL;//消息内容
	int ret;

#define MAXLINE 80
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len;
	int listenfd, connfd;
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN];
	int i, n;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(CMD_PORT);

	Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	Listen(listenfd, 1);

	printf("Accepting connections ...\n");
	while (bIsRun)
	{
		cliaddr_len = sizeof(cliaddr);
		connfd = Accept(listenfd, (struct sockaddr *) &cliaddr, &cliaddr_len);
		while (bIsRun)
		{
			memset(&msg, 0, sizeof(msg));
			n = Read(connfd, &msg, sizeof(msg));
			if (n == 0)
			{
				printf("the other side has been closed.\n");
				break;
			}
			if(msg.msg_head != MSG_HEAD)
			{
				cout << "msg head error!\n";
				continue;
			}
			memset(&msg_s, 0 ,sizeof(msg_s));
			msg_s.msg_head = MSG_HEAD;
			msg_s.msgtype  = cmd_ret_ok;
			if(msg.msgtype == request_time)
			{
				cout << "request_time" << endl;
				if( ( ret = read_disk_print_record_time(returntring)) < 0 )
				{
					cout << "read print error!! " << endl;

				}
				msg_s.msg_len = returntring.length();
			}
			else if(msg.msgtype == cmd_play)
			{
				cout << "cmd_play" << endl;
				ttt();
				starttime = msg.msg_len;
				cout << "start time: " <<  starttime  << endl;
				if(!is_in(starttime))
				{
					//这个没有在里面
					ttt();
					msg_s.msgtype  = cmd_ret_err;
					msg_s.msg_len = 0;
				}
				else
				{
					//1、如果有播放线程，关闭之
					// del yangkun if(isRead)
					{
						ttt();
						pthread_mutex_lock(&lock_start_read_again);
						// isRead = false;
						pthread_cond_wait(&stop_read, &lock_start_read_again); //等待停止信号
						ttt();
						pthread_mutex_unlock(&lock_start_read_again);
					}



					//2、启动播放线程
					ttt();
					pthread_cond_signal(&start_read); //发送播放

					ttt();


					returntring= "ok";
					msg_s.msg_len = 0;
				}
			}
			else
			{
				msg_s.msgtype  = cmd_ret_err;
				returntring = "your msg type is wrong";
				msg_s.msg_len = returntring.length();
				ttt();
			}
			Write(connfd, &msg_s, sizeof(msg_s));
			ttt();
			if(msg.msgtype != cmd_play)
			{
				ttt();
				cout << " len size: " << returntring.length() << " " << returntring.size() << endl;
				Write(connfd, returntring.c_str() ,returntring.length());
				//清理内存
				returntring.clear();
			}
		}
		Close(connfd);
	}


	while(bIsRun)
	{


#if 0
		if(p_msg_r->msgtype == request_time)
		{
			ttt();
			if( ( ret = read_disk_print_record_time(returntring)) < 0 )
			{
				cout << "read print error!! " << endl;

			}
		}
		else if(p_msg_r->msgtype == cmd_play)
		{
			ttt();
			//1、如果有播放线程，关闭之
			pthread_mutex_lock(&lock_start_read_again);
			isRead = false;
			//pthread_cond_signal(&stop_read); //发送停止
			pthread_mutex_unlock(&lock_start_read_again);
ttt();

			pthread_cond_wait(&stop_read, &lock_start_read_again);


			//2、启动播放线程
			ttt();
			pthread_cond_signal(&start_read); //发送播放

			ttt();

	        returntring = "i am hear!\n";
		}
		else
		{
			returntring = "your msg type is wrong";
			ttt();
		}
		cout << "buf size: " << returntring.size() << endl;
#endif
		//发送
		//memcpy(p_msg_s->msg_content, returntring.c_str() ,returntring.size());







	}//


}

#if 0
/*******************************************************************************************
 *初始化 内存池
 *******************************************************************************************/
void RWlogic::init_audio_pool()
{
	int ret;
	char name[256]="send pool";	//池子的名字
	ret=init_media_rw(&media,MEDIA_TYPE_VIDEO,0,BUFFER_SIZE);
	if(ret<0)
	{
		printf("error in init_media_rw,and exit\n");
		exit(1);
	}
	//初始化内存池
		ret=create_media_write(&media, 0x30008,name,VIDEO_POOL_SIZE_SAVE);

	if(ret<0)
	{
		printf("error in create_media_write\n");
		exit(1);
	}
	media.attrib->stat=ENC_STAT_OK;
	media.buflen=BUFFER_SIZE;

	media.attrib->fmt.v_fmt.format=VIDEO_H264;
	media.attrib->fmt.v_fmt.ispal=0;
	media.attrib->fmt.v_fmt.v_buffsize=BUFFER_SIZE;
	media.attrib->fmt.v_fmt.v_frate=25; //帧率
	media.attrib->fmt.v_fmt.v_height= 576;
	media.attrib->fmt.v_fmt.v_width= 704;


	if(media.temp_buf ==NULL)
	{
		printf("1111error in media tmemp_buf\n");
	}
	the_frame_buffer =(enc_frame_t *)media.temp_buf;		//构造那个avi结构头　方法特殊
	memset(the_frame_buffer, 0, sizeof(enc_frame_t));
	the_frame_buffer->channel=0;					//通道；
	the_frame_buffer->chunk.chk_id=IDX1_VID;		//视频
	the_frame_buffer->media=MEDIA_VIDEO;
}
void RWlogic::write_pool(char *buf, unsigned int buffsize,bool is_key)
{

	if (is_key)
		the_frame_buffer->type = FRAMETYPE_I;
	else
		the_frame_buffer->type = FRAMETYPE_P;
	memcpy((char *) the_frame_buffer->frame_buf, buf, buffsize);
	the_frame_buffer->len = buffsize;
	the_frame_buffer->chunk.chk_siz = buffsize;
	int ret=write_media_resource(&media, the_frame_buffer, buffsize+sizeof(enc_frame_t), is_key/*MEDIA_TYPE_VIDEO*/);
	if(ret<0)
	{
		gtlogerr("cat't write media resource and exit");
	}
}
#endif
void RWlogic::printids(const char *s)
{
	pid_t      pid;
	pthread_t  tid;

	pid = getpid();
	tid = pthread_self();
	printf("%s pid %u tid %u (0x%x)\n", s, (unsigned int)pid,
	       (unsigned int)tid, (unsigned int)tid);
}
//线程相关
void * RWlogic::create_thread(void *args)
{
	struct pthread_arg pa;
	struct pthread_arg *pa1 = static_cast< pthread_arg * >(args);
	ttt();
	memcpy(&pa, pa1, sizeof(struct pthread_arg));
	ttt();

	//cout << "isRun: " << pa.wr->IsRun() << endl;

	/************把数据写入硬盘*************************/
	if(pa.type==write_type)
	{
		cout <<"this is write"<<endl;
		pa.wr->printids("write pthread\n");
		pa.wr->writedata();
		cout <<"write process  exit"<<endl;
	}
	/************从硬盘读出想要的数据*************************/
	else if(pa.type==read_type)
	{
		cout <<"this is read"<<endl;
		pa.wr->printids("read pthread\n");
		//pa.wr->read_proc();
		cout <<"read process  exit"<<endl;
	}
	/************处理命令*************************/
	else if(pa.type==cmd_type)
	{
		cout << "this is cmd" << endl;
		pa.wr->printids("cmd pthread\n");
		pa.wr->cmd_proc();
		cout <<"cmd process  exit"<<endl;
	}
	/************从音视频缓冲区读出数据*************************/
	else if(pa.type==read_media_data)
	{
		cout << "this is read media data" << endl;
		gtloginfo("read media pool channel: %d",pa.channel);
		pa.wr->printids("cmd pthread\n");
		pa.wr->read_pool_proc(pa.channel);
		cout <<"cmd process  exit"<<endl;
	}
	ttt();
	gtlogwarn("proc %d exit!\n",pa.type);
	cout<< "proc " << pa.type << " exit!\n";

	pthread_exit(NULL);
	//f->thread();
	return NULL;
}
//从音视频缓冲区读出数据
void RWlogic::read_pool_proc(int channel)
{
	bool read_pool_start=false;
	//初始化数据
	DataRead *dr_video = new DataRead(0, video, 0+channel);
	DataRead *dr_audio = new DataRead(0, audio, 0x20000+channel);
	MultMediaBlocks *mb_video=NULL;
	MultMediaBlocks *mb_audio=NULL;
	bool is_i;
	while(!read_pool_start)
	{
		//读到第一个Ｉ帧时才开始
		read_pool_start  = dr_video->readpool();
	}

	while(bIsRun)
	{
		//视频分２Ｍ存１s的数据应该够了，音频分128Ｋ存１s的数据
		mb_video = new MultMediaBlocks(1224*1024 ,mult_video);
		mb_audio = new MultMediaBlocks(128*1024  ,mult_audio);
		//获取上一次，最后一个Ｉ帧，加入块
		mb_video->add(dr_video,video);
		//获取音视频数据
		while(bIsRun)
		{
			is_i  = dr_video->readpool();
			//读到Ｉ帧退出
			if(is_i)break;
			mb_video->add(dr_video,video);
			//一个视频对应两个音频
			while(dr_audio->get_remain_frame_num()>0)
			{
				//读音频缓冲区
				dr_audio->readpool();
				//把音频缓冲区，加入"大包"
				mb_audio->add(dr_audio,audio);
			}
		}
		//把单独的音频集合，视频集合组成一个更大的音视频集合
		//debug
		//cout <<"channel:\t"<< channel <<"\n video buff size: " << mb_video->get_buff_size() <<\
				"audio buff size : " << mb_audio->get_buff_size()<< endl;
		//myprint((unsigned char*)(mb_video->GetBuf()+sizeof(struct hd_frame)), 8);
		//myprint((unsigned char*)(mb_video->GetBuf()+sizeof(struct hd_frame)\
				+sizeof(struct media_block)), 8);

		//把组合好的音视频数据的集合放放队列里面
		mb_video_queue[channel].push(mb_video);
		mb_audio_queue[channel].push(mb_audio);
		mb_video=NULL;
		mb_audio=NULL;
		if(0==channel)
			pthread_cond_signal(&start_again);
	}
}








RWlogic::ReadDisk::ReadDisk(long long seek):isRead(false),
framenum(0),framecount(0),channel(0)
{
	//只允许有5个客户端
	if(num>5)
		throw(0);
	num++;
	pthread_mutex_init(&lock,NULL);
	pthread_cond_init(&notfull,NULL);
	pthread_cond_init(&startplay,NULL);
}
RWlogic::ReadDisk::~ReadDisk()
{
	num--;
	/*
	if(audio_pool)
	{
		delete audio_pool;
		audio_pool = NULL;
	}
	if(video_pool)
	{
		delete video_pool;
		video_pool = NULL;
	}
	*/
}

/*************************************************************************
 * 功能：	处理读取的数据，分解，并发出去
 * 参数：
 * video: 视频１块
 * auido: 音频１块
 *************************************************************************/
int RWlogic::ReadDisk::readdataprocess(SecBlocks *video, SecBlocks *audio)
{
	if(!video||!audio)
	{
		gtlogerr("readdataprocess video or audio si null");
		throw ;
	}
	//static unsigned int audio_count,video_count;
	//audio_count=0,video_count=0;
	bool isfirst=true;
	//第一个头
	struct media_block *mb_video = (struct media_block *)((char *)video->GetBuf() +sizeof(struct hd_frame));
	struct media_block *mb_audio = (struct media_block *)((char *)audio->GetBuf() +sizeof(struct hd_frame));

	unsigned int video_size = mb_video->size;
	unsigned int audio_size = mb_audio->size;

	//视频头
	mb_video = (struct media_block *)((char*)mb_video + sizeof(struct media_block));
	video_size -= sizeof(struct media_block);
	//音频头
	mb_audio = (struct media_block *)((char*)mb_audio + sizeof(struct media_block));
	audio_size -= sizeof(struct media_block);

	//把视频数据放到内存池里面0x30008
	while(isRead&&video_size>0)
	{
		//video_count++;
			//cout << "video leave : " << video_size << "frame : " << mb_video->size << endl;
			//数据
			video_size -= mb_video->size;
			//cout << "debug1111" << endl;
			//把数据传走
			//video_pool->write_pool((char*)mb_video+sizeof(struct media_block), mb_video->size);
			if(isfirst)
			{
				isfirst=false;
				//type=0是Ｉ帧, type=1是Ｐ帧
				video_queue.push((char*)mb_video+sizeof(struct media_block), mb_video->size,0);
			}
			else
				video_queue.push((char*)mb_video+sizeof(struct media_block), mb_video->size);
			mb_video = (struct media_block *)((char*)mb_video + \
								sizeof(struct media_block) +mb_video->size);
			video_size -= sizeof(struct media_block);
	}

	//如果音频没放完，继续

	//把音频数据放到内存池里面0x50008
	//ubuntu播放录音文件:aplay  -t raw -c 1 -f MU_LAW -r 8000 1.pcm
	while(isRead&&audio_size>0)
	{
		//audio_count++;
			//cout << "audio leave : " << audio_size << "frame : " << mb_audio->size << endl;
			//数据
			audio_size -= mb_audio->size;
			//cout << "debug1112" << endl;
			//把数据传走
			//audio_pool->write_pool((char*)mb_audio+sizeof(struct media_block), mb_audio->size);
			audio_queue.push(        (char*)mb_audio+sizeof(struct media_block), mb_audio->size);
			mb_audio = (struct media_block *)((char*)mb_audio + \
								sizeof(struct media_block) +mb_audio->size);
			audio_size -= sizeof(struct media_block);
	}
	//cout << "video cout: " << video_count << "\taudio count: " << video_count*2 << " " << audio_count << endl;
	return 0;
}
/*从硬盘读出想要的数据*/
void RWlogic::ReadDisk::read_proc()
{
#ifndef NOREAD
	sleep(1);
	int ret;
	long long seek;
	//dw = new DataWrite;
	SecBlocks *sb;
	SecBlocks *sb_video;
	SecBlocks *sb_audio;
	timecount = 0;
	//video_pool = new MediaPoolWrite(1);
	//audio_pool = new MediaPoolWrite(0);
	while(1)
	{
start:
	fprintf(stderr, "startplay0\n");
	while(!isRead)
	{
		pthread_mutex_lock(&lock);
		fprintf(stderr, "startplay1\n");
		pthread_cond_wait(&startplay, &lock);
		fprintf(stderr, "startplay2\n");
		pthread_mutex_unlock(&lock);
	}
		if(isRead)
			cout << "read channel:" << channel << " seek: " << readseek << endl;
		seek = readseek;
		while(isRead)
		{
			cout << "debug!!!!!!!!" << endl;
#ifndef READTEST
			pthread_mutex_lock(&lock);
			cout << "video_queue.size(): " << video_queue.size() ;
			cout << "audio_queue.size(): " << audio_queue.size() ;
			while(video_queue.size()>24)
			{
				//cout << "  11video_queue.size(): " << video_queue.size() << endl;
				//cout << "continue" << endl;
				//usleep(10000);
				pthread_cond_wait(&notfull, &lock);
			}
			pthread_mutex_unlock(&lock);
			//防止阻塞
			if(!isRead)
			{
				fprintf(stderr, "goto\n\n\n");
				goto start;
			}
#endif
			//读取头数据
			try
			{
				//先把512的头读了
				sb = new SecBlocks( sizeof(struct mult_media_block) ,seek );
			}
			catch (int ret)
			{
				if(HD_ERR_READ==ret)
				{
					ttt();
					gtlogerr("read blocks err, seek:%lld",seek);
					//如果出现读取错误，就退出这个循环，停住
					isRead=false;
					break;
				}
				else if(BLOCK_ERR_DATA_HEAD == ret)
				{
					ttt();
					gtlogerr("read blocks head err, seek:%lld,and exit",seek);
					//如果出现读取错误，就退出这个循环，停住
					isRead=false;
					break;
				}
			}
			struct mult_media_block *mmb = (struct mult_media_block *)((char *)sb->GetBuf() +sizeof(struct hd_frame));
			//cout << "video seek: " << mmb->video1_seek << " block num: " << mmb->video1_buf_size <<\
					"\taudio seek: " << mmb->audio1_seek << " block num: " << mmb->audio1_buf_size <<endl;
			seek = mmb->next_mult_media_block;

			sb_video = new SecBlocks(mmb->all_media_head[channel].video_buf_size, \
					mmb->all_media_head[channel].video_seek );
			sb_audio = new SecBlocks(mmb->all_media_head[channel].audio_buf_size,\
					mmb->all_media_head[channel].audio_seek );

			if( ( ret = readdataprocess(sb_video, sb_audio) ) < 0 )
			{
				cout << "error: " << ret << endl<<endl<<endl;
				//如果出现读取错误，就退出这个循环，停住
				isRead=false;
				break;
			}
			cout << "seek: " << seek<< endl;
			//清理操作
			if(sb)
			{
				delete sb;
				sb = NULL;
			}
			if(sb_audio)
			{
				delete sb_audio;
				sb_audio = NULL;
			}
			if(sb_video)
			{
				delete sb_video;
				sb_video = NULL;
			}

		}
		//pthread_cond_signal(&stop_read);
	}
#endif
}

void RWlogic::ReadDisk::write_proc()
{
	sleep(1);
	ttt();
	cout << "cout : " << count<< endl;
#ifdef	WRITEHD
	using namespace std;
	char videoname[200];
	char audioname[200];
	sprintf(videoname,"/mnt/yk/video%d.264",count);
	sprintf(audioname,"/mnt/yk/audio%d.pcm",count);
	cout << "videoname :" << videoname << " audioname: " << audioname << endl;
	ofstream videoof(videoname);
	ofstream audioof(audioname);
#endif
	while(1)
	{
		if(!isRead)
		{
			if(count==0)ttt();
			sleep(1);
		}
		if(isRead)
		{

			if(video_queue.size()>0)
			{
#ifdef TESTFRAMERATE
				//帧率控制
				if(framenum==0)
				{
					gettimeofday(&oldtime, NULL);
					framenum ++;
				}
				else
				{
					framenum ++;
					if(framenum==100)
					{
						gettimeofday(&nowtime, NULL);
						int tmp = (nowtime.tv_sec*1000000+nowtime.tv_usec)-(oldtime.tv_sec*1000000+oldtime.tv_usec);
						cout << "channel " << count << " frame rate: " << (float)(framenum*1000000)/tmp << endl;;
						framenum=0;
					}
				}
#endif

				//播放时间控制
				framecount--;
				if(framecount<=0)
				{

					isRead=false;
					sleep(1);
					video_queue.clear();
					audio_queue.clear();
					continue;
				}


				//if(count==0)ttt();
				//cout << "video size: " << video_queue.size() << endl;
				Blocks *bs=video_queue.pop();
#ifdef	WRITEHD
				videoof.write((char *)bs->GetBuf(), bs->GetSize());
				videoof.flush();
#endif
#ifdef NETSEND

#endif
				delete bs;
				int i;
				for(i=0; i<2; i++)
				{
					//cout << "audio_queue size: " << audio_queue.size() << endl;
					if(audio_queue.size()<=0)break;
					Blocks *abs=audio_queue.pop();
#ifdef	WRITEHD
					audioof.write((char *)abs->GetBuf(), abs->GetSize());
					audioof.flush();
#endif
					delete abs;
				}


				usleep(10000);
			}
			else//没有数据，睡会
			{
				usleep(40000);
			}

			while(audio_queue.size()>0)
			{
				//cout << "audio_queue size: " << audio_queue.size() << endl;
				//if(count==0)ttt();
				Blocks *abs=audio_queue.pop();
#ifdef	WRITEHD
				audioof.write((char *)abs->GetBuf(), abs->GetSize());
				audioof.flush();
#endif
#ifdef NETSEND

#endif
				delete abs;
			}
		}
	}




}
//type=1 音频，type=2视频
//isi 1是Ｉ帧   0是Ｐ帧 音频无意义
//返回０正确，１缓冲区没有数据，３错误
int RWlogic::ReadDisk::get_date(char *data, int *size,int type,int *isi)
{
	if(type==1)
	{
		if(audio_queue.size()<=0)
			return 1;
		Blocks *abs=audio_queue.pop();
		memcpy(data,(char *)abs->GetBuf(), abs->GetSize());
		*size = abs->GetSize();
		return 0;
	}
	if(type==2)
	{
		if(video_queue.size()<=0)
			return 1;
		Blocks *abs = video_queue.pop();
		memcpy(data,(char *)abs->GetBuf(), abs->GetSize());
		*size = abs->GetSize();
		if(0 ==  abs->GetType())
			*isi = 1;
		else
			*isi = 0;
		return 0;
	}
	return 3;
}
} /* namespace gtsda */
