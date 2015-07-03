/*********************************************************************************************
 * 天块的处理，包秒块的写入
 *
 ********************************************************************************************/
#include "blocks.h"
/*存入一天的帧挦址
 * 3600*24*3=86400*12=1036800   <= DATE_HEAD_BLOCK_SIZE*BLOCKSIZE=1037312
 * 天中的数据每秒都可以在此天块中找到至少一个对应关系，可以说1秒中第一个I帧对应一个节；如果此秒时没有数据或没有I帧则引节为0
 * index是天中最后一块秒（帧)在day_block中的位置（索引），size天中最后的一块秒(帧)占的块数（大小）。
 * day_buff=0x4d4f4e545aa5a55a+int index,int size+struct day_block{ int time; long long blocks;}[3600*24];
 * 注：day_block.time time函数返回的值 day_block.blocks指秒块存储在硬盘上的block的seek
 *
 *
 * */
static unsigned char day_buff[DATE_HEAD_BLOCK_SIZE*BLOCKSIZE];
/*day_block从4个整型开始，也就是16个字节*/
struct day_block *day_block=(struct day_block *)(day_buff + 4*4);

struct day_block_data{/*尾*/int index;int size;};

static unsigned char day_head[8]={0x4d,0x4f,0x4e,0x54,0x5a,0xa5,0xa5,0x5a};
/*当前秒块已写入的块序号(也是最后一个块写入的序号)，由此变量可以知道当前写到硬盘的什么位置了，\
 * 是不快写到结束位置了，要从头开始了。*/
long long current_block;


/*当前天所在的硬盘块*/
static long long current_day_block;









/*******debug1*******************/
long long seek_tmp_1;



/*******************************************************************
 * 获取当前块的序号
 *******************************************************************/
inline long long get_current_block()
{
	//if(current_blocks==0)???????????
	return current_block;
}
inline long long get_current_day_block()
{
	DP("GET_CURRENT_DAY_BLOCK:%lld\n\n\n\n",current_day_block);
	//if(current_blocks==0)???????????
	return current_day_block;
}
static inline int day_save(long long seek)
{
	DP("SVAE DAY::::seek:%lld\n",seek);
	if(seek<0){printf("seek<0\n");exit(1);}
	int tmp=get_my_time()%SECOFDAY;
	printf("111index:%d,block::%lld,%lld,%lld\n",tmp,day_block[tmp].seek,day_block[tmp-1].seek,day_block[tmp-2].seek);
	/*保存数据到硬盘*/
	int ret=hd_write(get_hd_fd(), seek, DATE_HEAD_BLOCK_SIZE, day_buff,DATE_HEAD_BLOCK_SIZE*BLOCKSIZE);
	if(ret!=0)
	{
		DP("write error ,plear resove the probram,and i will exit()\n");
		return -1;
	}


	struct day_block *day_block=(struct day_block *)(day_buff + 4*4);

	printf("222index:%d,block::%lld,%lld,%lld\n",tmp,day_block[tmp].seek,day_block[tmp-1].seek,day_block[tmp-2].seek);


	return 0;
}

inline void day_save1(){day_save(get_current_day_block());}

/*******************************************************************
 * 功能：新的一天
 *参数：blocks秒模块调用，并传入；下一步要写入的块
 *返回：要写入的秒块的位置
 ******************************************************************/
long long day_new(long long blocks)
{

/*头在前尾在后，尾是最新的
 *
 *
 */

	/**********************************
	 * bug!!!
	 * 是否要判断，你占的块中是否有天块？比如你判断了这一块是秒块，你会不会占了秒块后的天块呢？
	 * 解决：在把天块插入年块时，应该检查下一个天块的位置是不是正常的。如果不正常可以删去，什么
	 * 是不正常：后一块的天块的blocks比前一块的还要靠硬盘前
	 *
	 * ********************************/





	/*记录当前天块的位置 */
	current_day_block = blocks;

	int year_tail_block;

	/*不管怎么样先把头给换成新的*/
	memset(day_buff, 0,DATE_HEAD_BLOCK_SIZE*BLOCKSIZE);
	memcpy(day_buff, day_head, sizeof(day_head));
	/*把索引和最后一块置0*/

	struct day_block_data *day_head_queue=(struct day_block_data *)(day_buff+sizeof(day_head));
	day_head_queue->index=0;
	day_head_queue->size=0;
	day_block=(struct day_block *)(day_buff + 4*4);
	day_block[day_head_queue->index].time=get_my_time();
	day_block[day_head_queue->index].seek= blocks+DATE_HEAD_BLOCK_SIZE;

	/*判断最后一天是不是在硬盘的最后,快要结束的位置；如果是硬盘的最后位置，那这块从开始写入*/
	/*怎么判断是不是最后位置呢？
	 * 0x4d4f4e545aa5a55a天块24*3600*12/512=2025块(总2026块)
	 * 如果剩下的没有2026块。那就从头开始写
	 * */
	if(get_hd_max_blocks()-year_get_tail_blocks_of_hd()< DATE_HEAD_BLOCK_SIZE )
	{
		DP("hd is full???\n");
		//满足这些条件，就说明硬盘满了
		if(!hd_full_mark_get())
			hd_full_mark_set(TRUE);
		/*把第一块（头）块删了（第二块就成第一（头）块了），队列的大小没有改变*/
		/*如果头大于尾,说明已转了一圈了。*/
		/*都要把当前的队头删了*/
		year_insert_new_day_block(DATE_HEAD_BLOCK_SIZE);

		/*更改最后一块的位置*/
		current_block =YEAR_HEAD_BLOCK_SIZE+DATE_HEAD_BLOCK_SIZE;
		day_save(current_block);
		/*后来添加*/
		return current_block;
	}

		DP("hd is not full\n");
//	else if( year_get_tail_block()<year_get_head_block() )
//	{
		//year_insert_new_day_block(year_get_head_address()->blocks);
		year_insert_new_day_block(blocks);
//	}
//	else
//		/*硬盘录像时间不长，硬盘空间还没有用完*/
//	{

//	}
		/*格式化内存*/
		day_save(blocks);
		current_block = blocks+DATE_HEAD_BLOCK_SIZE;

		return current_block;


	return 0;
}
/****************************************************************************
 * 功能：获取当前时间应该写入秒块的位置
 ****************************************************************************/
inline long long day_get_sec_blocks()
{
	/*1、从年中找出今天；2、在今天中找出最后块写入的地址*/
	/*20130805 yk maybe bug  一天的写入的最后一秒，不一定是天块是记录的最后一秒的位置：：：因为一秒中有很多帧*/

	struct day_block_data *day_head_queue=(struct day_block_data *)(day_buff+sizeof(day_head));
	int index=day_head_queue->index ;

	struct day_block *day_block=(struct day_block *)(day_buff + sizeof(day_head) +sizeof(struct day_block_data));
	long long last_blocks = day_block[index].seek;

	/*将要写入的块的位置＝已写入最后一块开始的位置 + 已写入最后一块的大小*/
	long long current_blocks=last_blocks + day_head_queue->size;
	return current_blocks;
}

/******************************************************************************
 * 功能：当一个秒（帧）块写入硬盘时，同时更新天表中对应的此秒位和块位置
 * 参数：blocks:此帧在硬盘上位置（块数），size：此帧在硬盘上所定的块数
 ******************************************************************************/
inline int day_set_sec_block(long long seek,int size,time_t current_time)
{/*注意：这个函数参数名字有点不太好*/
	struct day_block_data *day_head_queue=(struct day_block_data *)(day_buff+sizeof(day_head));
	/*找到上帧在天表中的位置*/
	int index= day_head_queue->index;
	struct day_block *day_block=(struct day_block *)(day_buff + 4*4);
#if 0//yk del 20130805
	/*下一个就是*/
	day_block[index+1].seek=seek;
	day_block[index+1].time=get_my_time();
	/*用完了要更新天表 int seek,int size*/
#else
	int tmp=current_time%SECOFDAY;
	day_block[tmp].seek=seek;
	day_block[tmp].time=current_time;
	DP("index:%d,last_seek:%lld,%lld\n\n\n\n\n\n\n\n",tmp,day_block[tmp-1].seek,day_block[tmp].seek);
#endif
	day_head_queue->index=tmp;/*index+1; yk change 20130805*/
	day_head_queue->size=size;
	return 0;
}
/******************************************************************************
 * 功能：读出上最近一帧（秒）数据写入的时间
 * 使用情况：有可能一秒对应多帧，则最前面一帧有效（就是写入了硬盘保存的意思）
 * 返回：时间
 ******************************************************************************/
inline int day_get_lastsec_block()
{
	int index= *(int *)(day_buff + 4*2 + 4*0);
	return day_block[index].time;
}
inline int day_get_lastsec_block_index()
{
	int index= *(int *)(day_buff + 4*2 + 4*0);
	return index;
}


/*******************************************************************************
 * 初始化天，把天内容读到内存中
 *
 *返回-1错误
 *******************************************************************************/
int day_init()
{
	int ret;
	/*返回硬盘中最后一天*/
	long long last_day_block=year_get_tail_blocks_of_hd();
	DP("DAY,last_day_block:%lld\n",last_day_block);

	/*初始写这两个位置*/
	current_day_block=last_day_block;


	/*取这个天块的内容*/
	memset(day_buff, 0, DATE_HEAD_BLOCK_SIZE*BLOCKSIZE);
	ret=hd_read( get_hd_fd(),  last_day_block, DATE_HEAD_BLOCK_SIZE, day_buff,DATE_HEAD_BLOCK_SIZE*BLOCKSIZE);
	if(ret!=0)
	{
		printf("read day error\n");
		return -1;
	}
	/*判断这块数据正常吗*/
	if(memcmp(day_head,day_buff,sizeof(day_head)) !=0 )
	{
		/**/
		printf("day head is wrong!!\n");
		day_new(last_day_block);

		return -1;
	}

	current_block=day_get_sec_blocks();
	printf("current_day_block:%lld,current_block:%lld\n",current_day_block,current_block);

	/*debug*/
	//if(current_block==0||current_block<current_day_block)
	//{
	//	printf("current_block error\n");
	//	exit(1);
	//}
	//debug

	if(get_disk_format_mark()==TRUE)
	{
		day_new(YEAR_HEAD_BLOCK_SIZE+1);
		return 0;
	}
	struct day_block_data *day_head_queue=(struct day_block_data *)(day_buff+sizeof(day_head));
	struct day_block *day_block=(struct day_block *)(day_buff + 4*4);
	printf("save time:%d,current time:%d\n",day_block[day_head_queue->index].time,get_my_time());
	/*判断最后一天是今天吗？           还是昨天，前天*/
	if(day_block[day_head_queue->index].time/SECOFDAY == get_my_time()/SECOFDAY)
	{
		printf("it is today!\n\n\n");
		//是今天
		return 0;
	}
	else
	{
		printf ("not today\n");
		int index=day_get_lastsec_block_index();
		//在最后一秒后面的位置写入新的一天
		day_new(day_get_sec_blocks());
	}
	return 0;
}












/*******************************************************************************
 * 指定seek读出天块的内容
 *
 *返回:正值 返回第一个秒块  -1读出错 -2天块头出错 -3这一天中没能保存视频数据
 *******************************************************************************/
long long  day_read_block(long long day_seek)
{
	int ret;
	/*取这个天块的内容*/
	memset(day_buff, 0, DATE_HEAD_BLOCK_SIZE*BLOCKSIZE);
	ret=hd_read( get_hd_fd(),  day_seek, DATE_HEAD_BLOCK_SIZE, day_buff,DATE_HEAD_BLOCK_SIZE*BLOCKSIZE);
	if(ret!=0)
	{
		printf("read day error\n");
		return -1;
	}
	/*判断这块数据正常吗*/
	if(memcmp(day_head,day_buff,sizeof(day_head)) !=0 )
	{
		/**/
		printf("day head is wrong!!\n");

		return -2;
	}
	struct day_block *sec_queue=(struct day_block *)(day_buff+sizeof(day_head)+sizeof(struct day_block_data));
	int loop;
	for(loop=1/* 0 yk change debug*/; loop<SECOFDAY; loop++)
	{
		printf("1day_read_block:index:%d time:%d    blocks:%lld\n",loop,sec_queue[loop].time,sec_queue[loop].seek);
		/*如果找到第一个帧，返回*/
		if(sec_queue[loop].seek!=0/*&&*/||sec_queue[loop].time!=0)
			return sec_queue[loop].seek;

	}
	/*在这一天中没有写秒帧*/
	return -3;
}

/*******************************************************************************
 * 由时间返回当前块的位置
 *
 *
 *******************************************************************************/
long long  day_read_init(time_t this_time)
{
	int ret;

	/*从年表中返回当前天的seek块，就是位置 */
	long long day_seek=year_get_day_from_time(this_time);
DP("day_seek:%lld\n",day_seek);

	//更新天块的内容,把天块内容读到内存中
	ret=day_read_block(day_seek);
	if(ret<0)
	{
		printf("read error:%d\n",ret);
		return  ret;
	}

DP("1\n");
	/*从天块中找到一个 时间上最接近此块的帧块*/
	struct day_block *day_block=(struct day_block *)(day_buff + sizeof(day_head) +sizeof(struct day_block_data));
	printf("day_read_init:1 %d   %lld\n",day_block[this_time%SECOFDAY].time,day_block[this_time%SECOFDAY].seek);
	if(day_block[this_time%SECOFDAY].time==0||day_block[this_time%SECOFDAY].seek==0)
	{
		/*如果当前时间是空的，找个最近的。*/
		int loop;
		for(loop=-10; loop <10; loop ++)
		{
			int tmp=this_time+loop;
			printf("day_read_init:%d   %lld\n",day_block[tmp%SECOFDAY].time,day_block[tmp%SECOFDAY].seek);
			if(day_block[tmp%SECOFDAY].time!=0&&day_block[tmp%SECOFDAY].seek!=0)
				return day_block[tmp%SECOFDAY].seek;
		}
DP("2\n");
		/*如果时间点前后10个都没有找到符合的，那么就返回错误了。*/
		return -1;
	}
	else
		return day_block[this_time%SECOFDAY].seek;


}

/**************************************************************************
 * 判断数据头是不是天头
 * 返回-1不是天头，0是天头
 **************************************************************************/
inline int day_head_memcmp(char *buf)
{
	if(memcmp(buf,day_head,sizeof(day_head)) !=0 )
		return -1;
	else
		return 0;
}

/***************************************************************************
 * 传入当前帧地址，获取下帧地址
 *
 *返回：大于等于(YEAR_HEAD_BLOCK_SIZE+1)是正常值    0失败 1这一天读完了
 **************************************************************************/
long long  day_getnextbkad_formcur(long long seek)
{
	struct day_block *day_block=(struct day_block *)(day_buff + sizeof(day_head) +sizeof(struct day_block_data));
	int loop;
	BOOL mark=FALSE;
	for(loop=0; loop <SECOFDAY; loop++)
	{
		/*找到当前块的位置了*/
		if(day_block[loop].seek==seek)
			mark=TRUE;
		if(mark)
		{
			/*找到当前位置后面的最近一个位置了*/
			if(day_block[loop].seek!=0&&day_block[loop].time!=0)
				return day_block[loop].seek;
		}
	}
	/*查完了没有找到这一天中最近的一帧*/
	if(loop==SECOFDAY)
	{
		/*一天找完了！！！*/
		return 1;
	}


	return 0;

}




//debug1
int day_cmp_date(char *p,time_t  this_time)
{
	if(p!=NULL)
		printf(p);
	int tmp=this_time%SECOFDAY;
	struct day_block *sec_queue=(struct day_block *)(day_buff+sizeof(day_head)+sizeof(struct day_block_data));
	int loop;
	if(tmp>10)
		loop=tmp-10;
	else
		loop=tmp;

//	printf(":::%d    ",sec_queue);
	for(; loop<tmp+10;loop++)
		printf(" %d, %lld ",sec_queue[loop].time,sec_queue[loop].seek);

	printf("\n");
	return 0;
}










