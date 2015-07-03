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
# include <stdio.h>
namespace gtsda
{
DataWrite *RWlogic::dw;
FrameQueue RWlogic::frame_queue;
RWlogic::RWlogic(bool bMarkWR)
:bMarkWR(bMarkWR),bIsRun(false)
{
	// TODO Auto-generated constructor stub

}

RWlogic::~RWlogic()
{
	ttt();
	pthread_cancel(write_pid );
	pthread_cancel(read_pid );
	pthread_cancel(cmd_pid );
	pthread_join(write_pid, NULL);
	pthread_join(read_pid, NULL);
	pthread_join(cmd_pid, NULL);
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
	if(dr)
	{
	delete dr;
	dr = NULL;
	}
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
		cout << "Warning:: the day block will be cover " << endl;
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
int RWlogic::writedata()
{
	struct seek_block sb;//,earliest_day_block;
	int ret;
	int now,iOldTime;
	bIsRun = true;
	long long seek;
	dr = new DataRead();
	//从天块中获取最后的
	if( (ret = db->GetTail(sb) ) != 0 )
	{
		print_err(ret);
		seek = db->GetNext();
	}
	else
		seek = sb.seek;

	iOldTime = sb.time;



	cout << " current seek: " << seek << endl;

	while(bIsRun)
	{
		now = gettime();
		//cout << "now: "<< now << endl;
		//获取音视频数据
		if( (ret = dr->readpool()) <0 )
		{
			cout << "read pool error\n";
			exit(1);
		}
		if(frame_queue.size()>0)
		{
			//ttt();
			Blocks *bs = frame_queue.pop();
			write_pool((char *)bs->GetBuf(), bs->GetSize(), true);
			delete bs;
		}
		//cout << "dr size: " << dr->get_frame_buff_size() << endl;
		//FIX BUG  有时间写硬盘时卡住了怎么办？
		//判断今天是不是过去了
		if(now/SECOFDAY != iOldTime/SECOFDAY )
		{
			now = gettime();
			long long seek_tmp= db->GetBlocks();
			delete db;
			db = NULL;
			sb.time=now;
			ttt();
			//判断剩下的空间是不是能够存下这个天块
			if(HdSize - seek < seek_tmp )
			{
				ttt();
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
			ttt();
			sb.seek = seek;
			ret=yb->add(sb);
			ttt();
			seek += db->GetBlocks();
			//seek = db->GetNext();//与上一行相同的结果
			//cout << "this db getblocks: " << db->GetBlocks()<<endl;
			print_err(ret);
		}

		hb = new SecBlocks(dr->get_frame_buff_size() +  sizeof(struct hd_frame), seek);
		//ttt();
		//把视频数据加到写入缓冲区
		hb->addonetime(*dr);

		//硬盘是不是满了？
		if( HdSize - seek < hb->GetBlocks() )
		{
			cout << "Warning:: disk is full " << endl;
			gtloginfo("Warning:: disk is full ");
			//硬盘满了，年块中只有一天，说明硬盘不够存一天。
			if(1==yb->get_day_num())
			{
				cout << "Warning:: capacity of disk can't save one day " << endl;
				gtloginfo("Warning:: capacity of disk can't save one day");
				*dbbac = *db;// warning
				dbbac->write();
				seek =  first_block + Blocks::get_block_num(sizeof(struct year_block)) + \
									Blocks::get_block_num(sizeof(struct day_block)) + \
									Blocks::get_block_num(sizeof(struct day_block));
			}
			else
			{
				gtloginfo("Warning:: capacity of disk can save one day, turn to head");
				seek =  first_block + Blocks::get_block_num(sizeof(struct year_block)) + \
				Blocks::get_block_num(sizeof(struct day_block));
				hb->SetSeek(seek);
			}
		}

		//检测,如果要把天块覆盖了，把年块中最早的天块删了，备份天块。
		if ( ( ret = maybe_cover_dayblock(hb->GetBlocks(),seek) ) <0 )
		{
			gtlogerr("@%s %d maybe_cover_dayblock error:%d",__FUNCTION__ , __LINE__, ret);
			return ret;
		}

		seek += hb->GetBlocks();
		//cout << " hb block seek: "<< seek<< " blocks: " << hb->GetBlocks() << endl;
		sb.time = now;
		sb.seek = seek;

		//把索引加入天块
		if(dr->get_isi())
		{
			ret = db->add(sb);
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
		delete hb;//释放内存

	}
	cout << "debug adfadf"<< endl;
	return 0;
}
/*************************************************************************
 * 功能：	处理读取的数据，分解，并发出去
 * buf	：要处理的数据
 * buffsize: buf中数据的大小
 * seek：	处理的数据占块的大小
 *************************************************************************/
int RWlogic::readdataprocess(char *buf, unsigned int  buffsize, long long &seek)
{
	unsigned int buffcont; //计算buff的使用
	char *p = buf;
	struct hd_frame *sb;
	buffcont = 0;
	int tmp_size;
	while(isRead)
	{
		//处理完了
		blocktype bt = Blocks::judge_type(p+buffcont);
		if( sec == bt)
		{
			if(!isRead)
			{
				ttt();
				return -1;
			}
			//cout << "is sec frame!\n";
			 sb= (struct hd_frame *)(p+ buffcont);
			 //下一帧不够了，返回继续读
			 tmp_size = Blocks::get_block_num(sizeof(struct hd_frame) + sb->size)*BLOCKSIZE;
			 if(buffcont + tmp_size + 8 > buffsize )//检测不能越界 8 因为judge_type()最少要８个字节
			 {
				 seek += Blocks::get_block_num(buffcont);
				 cout << "debug 1" << endl;
			 	return 0;
			 }
			 else
			 {
#ifdef CRC
				 if(0 != SecBlocks::verify_crc((struct hd_frame *)(p+ buffcont)))
				 {
					 cout << "crc error!!seek= " << seek<< endl;
					 gtlogerr("crc error!!seek=%lld",seek);
					 exit(1);
				 }
#endif

				 frame_queue.push(p+ buffcont+sizeof(struct hd_frame),sb->size);
				 buffcont += tmp_size;
			 }
		}else if( bt == day )//处理天块的错误，只有这个还不够的。
		{
			cout << "buffcont: " << buffcont<< endl;
			cout << "this is day block!!\n" << endl;
			seek += ( Blocks::get_block_num(sizeof(struct day_block)) +\
					Blocks::get_block_num(buffcont));
			return 0;
		}
		else
		{
			cout << " not sec frame!! type: "<< bt<< endl;;
			cout << "buffcont: " << buffcont<< endl;
			myprint((const unsigned char*)(p+buffcont), 8);
			seek += Blocks::get_block_num(buffcont);
			return -1;
		}
	//usleep(40000);
	}
	ttt();
	return 0;
}
/************************************************
 * 这个时间是不是在硬盘里面
 ***********************************************/
bool RWlogic::is_in(int starttime)
{
	//1、在年块中找这个时间属于哪一天。
	readseek = yb->TimeIn(starttime);
	if( readseek <= 0)
	{
		cout << "info:your input time not in year block"<< endl;
		readseek = dbbac->TimeIn(starttime);//也不在备份天块中
		if( readseek <= 0 )
		{
			cout << "err: you input time is not in back day block,and i will exit!!"<< endl;
			gtlogerr("err: you input time is not in back day block,and i will exit!!");
			return false;
		}
	}
	else
	{
		if(dbread)
		{
			delete dbread;
			dbread =NULL;
		}
		dbread = new DayBlocks(readseek,true,false/*构造时读硬盘，析构时不写硬盘*/);
		readseek = dbread->TimeIn(starttime);//也不在备份天块中
		if( readseek <= 0 )
		{
			cout << "err: you input time is not in  day block,and i will exit!!"<< endl;
			gtlogerr("err: you input time is not in  day block,and i will exit!!");
			return false;
		}
	}
	return true;
}
void RWlogic::read_proc()
{
	ttt();
	ttt();
	ttt();
	isRead= false;

	int ret;
	long long seek;
	//dw = new DataWrite;
	Blocks *bs;
	const int buffsize= 2*1024*1024;//每次读的大小(512整数倍)
	int size = buffsize;
	char buf[512]={0};

	while(bIsRun)
	{
		ttt();
		pthread_mutex_lock(&lock_start_read_again);
		pthread_cond_wait(&start_read, &lock_start_read_again);
		isRead = true;
		pthread_mutex_unlock(&lock_start_read_again);
		cout << "read seek: " << readseek << endl;
		seek = readseek;
		while(isRead)
		{
			ttt();
			//读取buffsize大小的数据
			try
			{
			bs = new Blocks(seek, size);
			}
			catch (int ret)
			{
				if(HD_ERR_READ==ret)
				{
					ttt();
					gtlogerr("read blocks err, seek:%lld",seek);
					break;
				}
			}
			while(isRead)
			{
				if(frame_queue.size()>50)
				{
					cout << "debug111"<<endl ;
					sleep(2);
					cout << "debug1110"<<endl ;
					continue;
				}
				else if(frame_queue.size()>30)
				{
					cout << "debug112"<<endl ;
					sleep(1);
					cout << "debug1120"<<endl ;
					continue;
				}
				else
				{
					cout << "debug113"<<endl ;
					break;
				}
			}
			cout << "debug114"<<endl ;
			if( ( ret = readdataprocess((char *)bs->GetBuf(), size,  seek) ) < 0 )
			{
				cout << "error: " << ret << endl;
				break;
			}
			ttt();
			cout << "seek: " << seek<< endl;
			delete bs;
			bs = NULL;
		}
		ttt();
		//清理操作
		if(bs)
		{
			ttt();
			delete bs;
			bs = NULL;
		}




		ttt();
		pthread_cond_signal(&stop_read);
	}
}
int RWlogic::readdata(int starttime)
{
	struct seek_block sb;//,earliest_day_block;
	int ret;
	int now,iOldTime=gettime(false);
	bIsRun = true;
	long long seek;

	cout << "start time: " << starttime << endl<<endl;

	//1、在年块中找这个时间属于哪一天。
	seek = yb->TimeIn(starttime);
	if( seek <= 0)
	{
		cout << "info:your input time not in year block"<< endl;
		seek = dbbac->TimeIn(starttime);//也不在备份天块中
		if( seek <= 0 )
		{
			cout << "err: you input time is not in back day block,and i will exit!!"<< endl;
			gtlogerr("err: you input time is not in back day block,and i will exit!!");
			return -1;
		}
	}
	else
	{
		delete db;
		db =NULL;
		db = new DayBlocks(seek,true,false/*构造时读硬盘，析构时不写硬盘*/);
		seek = db->TimeIn(starttime);//也不在备份天块中
		if( seek <= 0 )
		{
			cout << "err: you input time is not in  day block,and i will exit!!"<< endl;
			gtlogerr("err: you input time is not in  day block,and i will exit!!");
			return -1;
		}
	}
	cout << "get the seek: " << seek << endl<< endl;;
	dw = new DataWrite;
	Blocks *bs;
	const int buffsize= 2*1024*1024;//每次读的大小(512整数倍)
	int size = buffsize;
	char buf[512]={0};



#if 0
	init_sigaction();
	init_time();
#endif
	while(isRead)
	{

		//读取buffsize大小的数据
		try
		{
		bs = new Blocks(seek, size);
		}
		catch (int ret)
		{
			if(HD_ERR_READ==ret)
			{
				ttt();
				cout << "read error!!!\n ";
			}
		}
		while(isRead)
		{
			if(frame_queue.size()>50)
			{
				cout << "debug111"<<endl ;
				sleep(2);
				cout << "debug1110"<<endl ;
				continue;
			}
			else if(frame_queue.size()>30)
			{
				cout << "debug112"<<endl ;
				sleep(1);
				cout << "debug1120"<<endl ;
				continue;
			}
			else
			{
				cout << "debug113"<<endl ;
				break;
			}
		}
		cout << "debug114"<<endl ;
		if( ( ret = readdataprocess((char *)bs->GetBuf(), size,  seek) ) < 0 )
		{
			cout << "error: " << ret << endl;
			return ret;
		}
		cout << "seek: " << seek<< endl;
		delete bs;
		bs = NULL;
	}

	return 0;
}
void RWlogic::start(int starttime)
{
	static struct pthread_arg pa_write, pa_read, pa_cmd;
	pa_write.wr = this;
	pa_read.wr  = this;
	pa_cmd.wr   = this;
	this->bIsRun = true;

	//初始化读相关
	  pthread_mutex_init(&lock_start_read_again,NULL);
	  pthread_cond_init(&start_read,NULL);
	  pthread_cond_init(&stop_read,NULL);
	//创建写线程
	ttt();
	pa_write.type = write_type;
	pthread_create(&write_pid, NULL, create_thread, &pa_write);
	//创建读线程
	pa_read.type = read_type;
	pthread_create(&read_pid, NULL, create_thread, &pa_read);
	//创建读写命令线程
	ttt();
	pa_cmd.type = cmd_type;
	pthread_create(&cmd_pid, NULL, create_thread, &pa_cmd);
#if 0
	if(bMarkWR/*true*/)
	{
		writedata();
	}
	else
	{
		readdata(starttime);

	}
#endif
	//cout << "we are out!!" << endl;
	//bIsRun = false;
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
		cout << "catch error!!\n";
		print_err(err);
		if(BLOCK_ERR_DATA_HEAD == err )
		{
			cout << " maybe this is new disk, and I will formate this disk!!!" << endl;
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
		//如果是空的
		if( ( BLOCK_ERR_EMPTY == ret ) && bMarkWR)
		{
			cout << "year block is empty!!\n" << endl;
			gtlogerr("year block is empty!!\n" );
			sb.seek = first_block + Blocks::get_block_num(sizeof(struct year_block)) + \
					Blocks::get_block_num(sizeof(struct day_block));
			sb.time = gettime();
			if( ( ret = yb->add(sb) ) < 0 )
			{
				print_err( ret );//异常
				return ret;
			}
		}
		else
			return ret;
	}
	cout << "current day seek: " << sb.seek << " time: " << sb.time << " " << ctime((time_t *)&sb.time)<< endl;
	//获取天块
	db = new DayBlocks(sb.seek);
	HdSize = hd_getblocks();

	init_audio_pool();

	return 0;
}
void RWlogic::format()
{
	ttt();
	//formate year block
	if(NULL != yb)
	{
		ttt();
		struct year_block *yb = (struct year_block *)this->yb->GetBuf();
		memcpy(yb->year_head, YearBlocks::year_head, sizeof(YearBlocks::year_head));
		this->yb->write();
	}
	//formate day bac block
	if(NULL != dbbac)
	{
		ttt();
		struct day_block *dbbac = (struct day_block *)this->dbbac->GetBuf();
		memcpy(dbbac->day_head, DayBlocks::day_head, sizeof(DayBlocks::day_head));
		this->dbbac->write();
	}
	//formate day block
	if(NULL !=db )
	{
		ttt();
		struct day_block *db = (struct day_block *)this->db->GetBuf();
		memcpy(db->day_head, DayBlocks::day_head, sizeof(DayBlocks::day_head));
		this->db->write();
	}
}
/***************************************************************************
 * 功能：输出今天的时间段
 * 参数：dDayData天块指针
 * 		iBeginTime开始时间
 ***************************************************************************/
void RWlogic::printOneDay(const struct day_block *dDayData, int iBeginTime ,string &timestring, bool isweb)
{
	int bTmp=0, iTmp;
	int sec;
	for( sec = iBeginTime; sec < SECOFDAY ; sec++ )
	{
		//cout << sec << "st seek: " << dDayData->seek_block_data[sec].seek << "\ttime: " <<\
				dDayData->seek_block_data[sec].time<<endl;
		if(bTmp == 0)
		{
			if( dDayData->seek_block_data[sec].seek!=0 || dDayData->seek_block_data[sec].time != 0 )
			{

				timestring += "start:\tblock: ";
				timestring +=std::to_string( dDayData->seek_block_data[sec].seek) ;
				timestring += "\ttime:  ";
				timestring += std::to_string( static_cast<long long>(dDayData->seek_block_data[sec].time ));
				timestring += " --> ";
				timestring += ctime( (const time_t *)&( dDayData->seek_block_data[sec].time ) ) ;
				if(isweb)
					timestring += "<br>";
				bTmp = 1;
			}
		}
		else
		{
			if( dDayData->seek_block_data[sec].seek==0 && dDayData->seek_block_data[sec].time == 0 )
			{
#if 1
/****************************************************************************************
 * 这段代码的作用：
 * 在day_block中有SECOFDAY个(I帧)数据，如果一天都存满数据的话，理论上这SECOFDAY中都有数据
 * 但是因为系统时间与I帧时间有误差，造成可能某一秒有I帧，但判断时与系统时间不在同一秒就没有写入day_block，
 * 以下的代码就是说如果有形如 1 0 0 1， 1 0 1 的形式就不忽略。就是两帧中有一帧或两帧是空的就忽略。
 ****************************************************************************************/
				iTmp = SECOFDAY - sec;
				if(iTmp>2)
				{
					iTmp = 2;
					if(dDayData->seek_block_data[sec+1].seek!=0 && dDayData->seek_block_data[sec+1].time != 0 )/*1 0 1的情况，中间1秒没有数据 */
					{
						sec += 1;
						continue;
					}else if(dDayData->seek_block_data[sec+1].seek==0 && dDayData->seek_block_data[sec+1].time == 0 &&\
							dDayData->seek_block_data[sec+2].seek!=0 && dDayData->seek_block_data[sec+2].time != 0)/*1 0 0 1 的情况，中间2秒没有数据*/
					{
						sec += 2;
						continue;
					}

				}
#endif
				timestring += "end:\tblock:";
				timestring += std::to_string( dDayData->seek_block_data[sec-1].seek );
				timestring += "\ttime: ";
				timestring += std::to_string( static_cast<long long>(dDayData->seek_block_data[sec-1].time));
				timestring +=  " --> ";
				timestring +=  ctime( (const time_t *)&( dDayData->seek_block_data[sec-1].time ) ) ;
				if(isweb)
					timestring +="<br><br>\n";
				bTmp = 0;
			}
			else if(sec==SECOFDAY-2)bTmp = 0;
		}
	}
}

/*************************************************************************
 * 功能：读硬盘上年块天块，输出录像时间
 *************************************************************************/
int RWlogic::read_disk_print_record_time(string &timestring, bool isweb)
{
	int tmp=0;
	int day, sec;
	int bool_tmp=0;
	if(yb==NULL){ttt();cout << "year block error" << endl;return -1;}

	//看看dbbac中是不是有数据,并输出
	//读dbbac中内容
	timestring += "daydata_bak::\n";
	if(isweb)
		timestring +="<br><br>";
	printOneDay((const struct day_block *)(dbbac->GetBuf()), 0 ,timestring, isweb );
	cout << endl;


	//遍历年块中的数据；
	const struct year_block *yearblock =  (struct year_block *)yb->GetBuf();
	timestring +=  "day of yearblock:";
	timestring += std::to_string( static_cast<long long>(yearblock->year_queue_data.queue_size) );
	timestring += "\n";
	if(isweb)
		timestring += "<br><br>\n";
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
		timestring += std::to_string( static_cast<long long>(day+1-yearblock->year_queue_data.queue_head));
		timestring +="st day's time is: " ;
		timestring +=std::to_string( static_cast<long long>( yearblock->seek_block_data[day].time ));
		timestring += "\t-->";
		timestring += ctime((const time_t*)&( yearblock->seek_block_data[day].time ) );
		if(isweb)
			timestring +="<br>";
		//cout << "today's block:\n";
		//读取天块的内容到内存中
		if(isweb)
			timestring +="<!--\n"; //html 注示
		DayBlocks dayblock(yearblock->seek_block_data[day].seek,true/*构造时读硬盘*/,false/*析构时不写入*/);
		if(isweb)
			timestring +="-->\n"; //html 注示
		//cout << "seek of begin: " << (yearblock->seek_block_data[day].time)%SECOFDAY << endl;
		//输出本天内的连续块
		//if(day==yearblock->year_queue_data.queue_head+1)
		printOneDay((const struct day_block *)(dayblock.GetBuf()),/* (yearblock->seek_block_data[day].time)%SECOFDAY*/0 ,timestring, isweb );
		if(isweb)
			timestring +="<br>\n" ;
	}
	return 0;
}
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
					if(isRead)
					{
						ttt();
						pthread_mutex_lock(&lock_start_read_again);
						isRead = false;
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
} /* namespace gtsda */
