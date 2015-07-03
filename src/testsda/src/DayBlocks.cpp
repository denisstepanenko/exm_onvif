/*
 * DayBlocks.cpp
 *
 *  Created on: 2013-10-29
 *      Author: yangkun
 */

#include "DayBlocks.h"

namespace gtsda
{
const unsigned char DayBlocks::day_head[8]={0x4d,0x4f,0x4e,0x54,0x5a,0xa5,0xa5,0x5a};
DayBlocks::DayBlocks(long long llSeek , bool bIsRead, bool bIsWrite)
	:MultBlocks(get_block_num(sizeof( struct day_block ) ) *BLOCKSIZE ,llSeek ,day) , \
	 bIsWrite(bIsWrite),iOperateTimes(0)
{
	daydata = (struct day_block *)GetBuf();
	//以下两行１２号新添加
	memset(daydata, 0 ,GetSize());
	memcpy(daydata->day_head, day_head, sizeof(day_head));	//复制文件头
	if(bIsRead)
		ReadBlock();
}


DayBlocks::~DayBlocks()
{
	if(bIsWrite)
		write();
}

int DayBlocks::read()
{
	int ret;
	if( ( ret = Blocks::read() ) < 0 )
	{
		print_err( ret);
		return ret;
	}
	const char *p =(const char *)GetBuf();
	if( memcmp(day_head,p,  sizeof(day_head)) != 0 )
	{
		print_err(BLOCK_ERR_DATA_HEAD);
		return BLOCK_ERR_DATA_HEAD;
	}
	return 0;
}
int DayBlocks::ReadBlock()
{
	int ret;
	//从硬盘中读取年块
	if( ( ret=read() ) < 0)
	{
		if( BLOCK_ERR_DATA_HEAD == ret )
		{
			cout << "maybe new disk " << endl;
		}
		else
			cout << "other err: " << ret << " !!!!" << endl;
		/*
		 * else 异常
		 */
		memset(daydata, 0 ,GetSize());
		memcpy(daydata->day_head, day_head, sizeof(day_head));	//复制文件头
		//写入数据
		write();
	}
	ttt();
	return 0;
}

//插入，读取块数据
int DayBlocks::add(const struct  seek_block &sb)
{
	int index;//今天的第多少秒
	int ret;
	//block中数据是否合法
	if(&sb == NULL )
			return BLOCK_ERR_NULL;
	if(sb.time==0 ||sb.seek==0)
			return BLOCK_ERR_ZERO;

	//复制数据
	index = sb.time%SECOFDAY;

	if(daydata->seek_block_data[index].seek !=0 )
		return BLOCK_ERR_DAY_SEC_MUT;//当前的秒块位置有值了。
	else
		memcpy(&(daydata->seek_block_data[index]), &sb, sizeof(struct  seek_block));

//cout << "sb.time: "<< sb.time << " index: "<< sb.time%SECOFDAY << " seek: " << sb.seek <<  endl;

	//每tmp == x次写入一次天块。  hd_current_day_seek
	iOperateTimes++;
	if(iOperateTimes >= 10)
	{
		gtloginfo("I frame->seek:%lld\ttime:%d\tsequence:%d\n",sb.seek,sb.time,sb.time%SECOFDAY);
		//ttt();
		iOperateTimes = 0;
		if( (ret = write()) < 0)
		{
			gtloginfo("day block write:time:%d\tsequence:%d\n",sb.time,sb.time%SECOFDAY);
			cout <<  "write err" << endl;
			return ret;
		}
	}
	return 0;
}
int DayBlocks::del(const struct  seek_block &sb)
{
	return 0;
}
int DayBlocks::Get   (struct  seek_block &sb,get_type get)
{
	if(get_start == get )
	{
		return GetHead(sb);
	}
	else
		return GetTail(sb);
}
int DayBlocks::GetHead(struct  seek_block &sb)
{
	int i;
	for(i=0; i < SECOFDAY; i++)
	{
		if(daydata->seek_block_data[i].seek!=0 || \
				daydata->seek_block_data[i].time != 0)
		{
			memcpy(&sb, &daydata->seek_block_data[i], sizeof(struct seek_block) );
			return 0;
		}
	}
	ttt();
	return BLOCK_ERR_EMPTY;
}
int DayBlocks::GetTail(struct  seek_block &sb)
{
	int i;
	for(i=SECOFDAY; i > 0; i--)
	{
		if(daydata->seek_block_data[i].seek!=0 || \
				daydata->seek_block_data[i].time != 0)
		{
			memcpy(&sb, &daydata->seek_block_data[i], sizeof(struct seek_block) );
			return 0;
		}
	}
	ttt();
	return BLOCK_ERR_EMPTY;
}
//判断时间或seek是不是在这个块中
int DayBlocks::In(struct  seek_block &sb, InType iType)
{
	if(InTime == iType)
		return TimeIn(sb.time);
	else
		return SeekIn(sb.seek);
		return 0;
}
inline long long  DayBlocks::TimeIn(int iTime)
{

	unsigned int daysec,sec;
	daysec = iTime%SECOFDAY;
	if( ( daydata->seek_block_data[daysec].time!=0 ) && ( daydata->seek_block_data[daysec].seek!=0 ) )
	{
		if(daydata->seek_block_data[daysec].time/SECOFDAY == iTime/SECOFDAY)//是同一天的
			return daydata->seek_block_data[daysec].seek;
		else//不是同一天
			return BLOCK_ERR_NOT_IN;
	}
	for(sec = ((daysec<=10)?0:(daysec-10)) ;
			sec < ( (SECOFDAY-daysec>10)?(daysec+10):SECOFDAY);
			sec++)
	{
		if(daydata->seek_block_data[daysec].time/SECOFDAY == daysec)//是同一天的
			return daydata->seek_block_data[daysec].seek;
		else//不是同一天
			return BLOCK_ERR_NOT_IN;
	}
	return BLOCK_ERR_NOT_IN;
}
int  DayBlocks::SeekIn(long long llSeek)
{
	int sec;
	for(sec = 0 ; sec < SECOFDAY; sec++)
	{
		if( ( daydata->seek_block_data[sec].seek == llSeek ) && ( daydata->seek_block_data[sec].time !=0 )  )
			return daydata->seek_block_data[sec].time;
		else//不是同一天
			return BLOCK_ERR_NOT_IN;
	}
	return BLOCK_ERR_NOT_IN;
}

DayBlocks & DayBlocks::operator=(const DayBlocks &db)
{
	if(this == &db)
		return *this;
	ttt();
	struct day_block * d = (struct day_block *)db.GetBuf();
	memcpy(this->daydata, db.GetBuf(), sizeof(struct day_block));
	return *this;
}

} /* namespace gtsda */
