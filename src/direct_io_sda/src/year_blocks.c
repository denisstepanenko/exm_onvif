/******************************************************
 *  功能：通过年块寻找每天的位置
 *  接口：第一次初始化，
 *
 ******************************************************/
#include "blocks.h"

static BOOL disk_format_mark=FALSE;



/*年数据块*/
static unsigned char buf_head[YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE];
/*年块中真正的数据，天块的索引，是个循环队列,结构像下面一样
 * 	struct year_queue_buf
 * 	{
 * 		unsigned int queue_size ,queue_head,queue_tail;
 * 		struct year_block{ unsigned time; long long seek;}[MAXDAY];
 * 	}
 * 	其中在MAXDAY这么多的块中，只有从queue_head开始，从queue_tail结束的quque_size个块有效
 * */
static unsigned char *year_queue_buf;

struct year_queue_data
{
	unsigned int queue_size ,queue_head,queue_tail;
};


/**********************************************************
 * 把年块写入硬盘
 *********************************************************/
void year_save()
{
	DP("SVAE YEAR:::\n");
	long long seek=1;
	/*保存数据到硬盘*/
	int ret=hd_write(get_hd_fd(), seek, YEAR_HEAD_BLOCK_SIZE, buf_head,YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
	if(ret!=0)
	{
		DP("write YEAR BLOCK error ,plear resove the probram,and i will exit()\n");
		exit(1);
	}
}
/************************************************************
 * 出现了严重的错误
 ************************************************************/
int year_error()
{
	//free (buf_head);
	return 0;
}
/**********************************************************
 * 初始化天数据块
 * fd:硬盘文件描述符
 * block：开始的块数
 * 返回：0成功，
 * ********************************************************/
int year_init()
{
	int ret;
	/**************************************************
	 * 初始化硬盘存储结构
	 *原理：查看硬盘第一块的前两个字节是不是0x59454152a5a5a5a5
	 * ************************************************/

	/*数据块从第一块开始，第0块是boot*/
	long long seek=1;
	/*********************************************************
	 * 这个是很著名的年内存块
	 *
	 *********************************************************/
	disk_format_mark=FALSE;
    memset(buf_head, 0, YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
	//ret=sg_read(outfd, buf_head, blocks, seek,blk_sz, &iflag, &dio_tmp, &blks_read);
    /*读前二十块，年块*/
	ret=hd_read(get_hd_fd(),  seek, YEAR_HEAD_BLOCK_SIZE, buf_head,YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
	if(ret==0)
	{
		printf("read block 1 ok\n");
	}
	else
	{
		printf("read block 1 error,and i will exit()");
		myprint(buf_head,512);
		return -1;
	}
	//myprint(buf_head,512*10);


	char buf_head1[YEAR_OFFSET+3*4 +4]={0x59,0x45,0x41,0x52,0xa5,0xa5,0xa5,0xa5, \
			0,0,0,0/*当前队列的长度*/,0,0,0,0/*当前队列的队头*/,0,0,0,0/*当前队列的队尾*//*队头，队尾都是同一个位置*/};
#if 0
	/*把硬盘头破坏，从头开始写*/
	buf_head[5]=0;
#endif

	/*如果不一样，则要格式化*/
	if(memcmp(buf_head1,buf_head,YEAR_OFFSET) !=0 )
	{
		disk_format_mark=TRUE;
		DP("the year head is wrong!!!!!\n");
		/*格式化内存*/
		memset(buf_head, 0, YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
		/*把正确的头复制到buf中，并且初始化年队列，写入就算是格式化了*/
		memcpy(buf_head,buf_head1,YEAR_OFFSET+3*4);

		/*年队列占了20块；；；第一人天所点的块数为20 (因为有第0块)*/
		int times;
		times=get_my_time();
		long long blocks=HD_START+YEAR_HEAD_BLOCK_SIZE;/*最前面一个天块应该是21块开始*/
		printf("time:%d\n",time);
		/*把年队列的第一块的值写入*/
		memcpy(buf_head+YEAR_OFFSET+3*4,(unsigned char *)&times, 4);
		memcpy(buf_head+YEAR_OFFSET+3*4+4,(unsigned char *)&blocks, 8);

		myprint(buf_head,8);
		/*从第一块开始写入512个字节*/
	    //ret=sg_write(fd, buf_head, YEAR_HEAD_BLOCK_SIZE, 1, BLOCKSIZE, &oflag, &dio_tmp);
		ret=hd_write(get_hd_fd(), seek, YEAR_HEAD_BLOCK_SIZE, buf_head,YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
	    if(ret!=0)
	    {
	    	DP("write error ,plear resove the probram,and i will exit()\n");
	    	return -1;
	    }

	     /*bug!! 如果头错误，格式化，读天块时可能会出错。*/
	}
	else printf("we have formate,and we can write data\n");




	/*格式化过了,查找这次应该写“天”的位置,也就是天的块数 ,天->小时*/
	year_queue_buf=buf_head+YEAR_OFFSET ;

	struct year_queue_data *p=(struct year_queue_data *)year_queue_buf;
	unsigned int date_address=	p->queue_tail;/*最后一天*/
	struct year_block *dayblock=(struct year_block *)(year_queue_buf+sizeof(struct year_queue_data));



	/*天块肯定在年块（YEAR_HEAD_BLOCK_SIZE）后的*/
	if(dayblock[date_address].seek<YEAR_HEAD_BLOCK_SIZE)
	{
			printf("date_address:%d,seek:%lld\n",date_address,dayblock[date_address].seek);
			DP("day block in year is wrong!!!\n");
			/*如果这个出错了，就假设从开头开始*/
			memset(buf_head, 0, YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
			memcpy(buf_head,buf_head1,YEAR_OFFSET);
			struct year_queue_data *p=(struct year_queue_data *)year_queue_buf;
			p->queue_head=0;
			p->queue_size=0;
			p->queue_tail=0;
			dayblock[0].seek=HD_START+YEAR_HEAD_BLOCK_SIZE;
			dayblock[0].time=get_my_time();

			printf("seek:%lld\n\n",dayblock[0].seek);
			/*把数据再保存到硬盘中*/
			//long long blocks=;
			ret=hd_write(get_hd_fd(), seek, YEAR_HEAD_BLOCK_SIZE, buf_head,YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
		    if(ret!=0)
		    {
		    	DP("write error ,plear resove the probram,and i will exit()\n");
		    	return -1;
		    }
	}


	/*******************************************************************
	 * 下面应该有操作获取get_current_block();
	 *
	 *******************************************************************/

	return 0;
}

/***********************************************************************
 * 向年块中，插入天块；肯定插入的是最后一块的后一块
 *
 * 返回值：-1出错
 ***********************************************************************/
int year_insert(int times, long long blocks)
{
	/*此函数应该加锁*/
	unsigned int offset=YEAR_OFFSET;
	unsigned int *p=(unsigned int *)year_queue_buf;
	unsigned int queue_size=p[0];
	unsigned int queue_head=p[1];
	unsigned int queue_tail=p[2];

	//判断是不是传入参数有错。
	/*传入的块数太大*/
	if(get_hd_max_blocks()<blocks)
		return -1;
	/**/





	return 0;
}

/***********************************************************************
 * 向年块中取天块天块，可能是任意一块
 *参数：times:年中某天的时间
 ***********************************************************************/
int year_get_day_block(int times)
{
	unsigned int offset=YEAR_OFFSET;
	unsigned int *p=(unsigned int *)year_queue_buf;
	unsigned int queue_size=p[0];
	unsigned int queue_head=p[1];
	unsigned int queue_tail=p[2];


	return 0;
}
/***********************************************************************
 * 从年块中找出最后一块块数。
 ***********************************************************************/
inline unsigned int year_get_tail_block(){return *(unsigned int *)(year_queue_buf+4*2);}
 /***********************************************************************
  * 从年块中找出第一块块数。
  ***********************************************************************/
inline   unsigned int year_get_head_block(){return *(unsigned int *)(year_queue_buf+4*1);}
  /***********************************************************************
   * 返回年块队列的大小
   ***********************************************************************/
inline   unsigned int year_get_size(){return *(unsigned int *)(year_queue_buf+4*0);}
/************************************************************************
 * 返回年块最后一块天块的地址
 *************************************************************************/
inline struct year_block* year_get_tail_address(){return  \
		(struct year_block *)( (int *)year_queue_buf+4*3 + year_get_tail_block()); \
}
/************************************************************************
 * 返回年块第一块天块(队列头)的地址
 *************************************************************************/
inline struct year_block* year_get_head_address(){return  \
		(struct year_block *)( (int *)year_queue_buf+4*3 + year_get_head_block()); \
}
/************************************************************************
 * 返回年块最后一块天块在硬盘中的blocks
 *************************************************************************/
inline long long year_get_tail_blocks_of_hd(){
		// return year_get_tail_address()->seek;
		year_queue_buf=buf_head+YEAR_OFFSET ;

		struct year_queue_data *p=(struct year_queue_data *)year_queue_buf;
		unsigned int day_address=	p->queue_tail;/*最后一天*/
		struct year_block *dayblock=(struct year_block *)(year_queue_buf+sizeof(struct year_queue_data));
		return dayblock[day_address].seek;
}
/************************************************************************
 * 年块中插入新的一块
 * 把年块的第一块（头）变成最后一块（尾）；而第二块，变成第一块。插入的新的一块成了队尾
 * 参数：插入年块在硬盘中序号
 *************************************************************************/
inline int year_insert_new_day_block(long long seek)
{

	struct year_queue_data *p=(struct year_queue_data *)(buf_head+YEAR_OFFSET);
	/*硬盘满了*/
	if( hd_full_mark_get() )
	{
		DP("HD FULL\n");
		/*从头开始写*/
		if(seek==DATE_HEAD_BLOCK_SIZE)
		{
			DP("DAY HEAD BLODK\n");
			/*队头*/
			p->queue_head= 1;
			/*队尾*/
			p->queue_tail=0;
			struct year_block *yb=(struct year_block *)(buf_head+YEAR_OFFSET+sizeof(struct year_queue_data));
			yb[p->queue_tail].seek=seek;
			yb[p->queue_tail].time=time(NULL);

			/*bug!! ,当硬盘满时，应该把年块中后面没有用的表示天的块置0*/

			year_save();
			return 0;
		}
	}
	DP("NOT FULL\n");
	/*一块刚格式化的硬盘，还没满   或者已写过了*/
	{
		/*队列加一，也就是增加了一天，*/
		/*队头*/
		p->queue_head +=1;
		/*队尾*/
		p->queue_tail +=1;
		struct year_block *yb=(struct year_block *)(buf_head+YEAR_OFFSET+sizeof(struct year_queue_data));
		yb[p->queue_tail].seek=seek;
		yb[p->queue_tail].time=time(NULL);
		year_save();
		/*bug!! ,如果在年块中的下一天个天块的blocks比当前天块的blocks还要靠前，是不是可以说明下一个天块有问题呢？*/
	}
	return 0;
}















/********************************************************
 * 返回当前该读哪一块了
 * 参数：硬盘前二十块的内容
 * ******************************************************/
unsigned int get_date_address(unsigned char *buf_queue)
{
	unsigned int offset=YEAR_OFFSET;
	unsigned int *p=(unsigned int *)buf_queue;
	unsigned int queue_size=p[0];
	unsigned int queue_head=p[1];
	unsigned int queue_tail=p[2];
	/*因为队列的大小，头尾点了三个整形数据*/
	return p[queue_tail+3];
}



























/*****************************************
 * 读取年块，
 *
 *******************************************/

int year_read_init()
{
	int ret;
	/**************************************************
	 * 初始化硬盘存储结构
	 *原理：查看硬盘第一块的前两个字节是不是0x59454152a5a5a5a5
	 * ************************************************/

	/*数据块从第一块开始，第0块是boot*/
	long long seek=1;
	/*********************************************************
	 * 这个是很著名的年内存块
	 *
	 *********************************************************/

    memset(buf_head, 0, YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
	//ret=sg_read(outfd, buf_head, blocks, seek,blk_sz, &iflag, &dio_tmp, &blks_read);
    /*读前二十块，年块*/
	ret=hd_read(get_hd_fd(),  seek, YEAR_HEAD_BLOCK_SIZE, buf_head,YEAR_HEAD_BLOCK_SIZE*BLOCKSIZE);
	if(ret==0)
	{
		printf("read block 1 ok\n");
	}
	else
	{
		printf("read block 1 error,and i will exit()");
		myprint(buf_head,512);
		return -1;
	}
	//myprint(buf_head,512*10);


	char buf_head1[YEAR_OFFSET+3*4 +4]={0x59,0x45,0x41,0x52,0xa5,0xa5,0xa5,0xa5, \
			0,0,0,0/*当前队列的长度*/,0,0,0,0/*当前队列的队头*/,0,0,0,0/*当前队列的队尾*//*队头，队尾都是同一个位置*/};

	/*如果不一样，则要格式化*/
	if(memcmp(buf_head1,buf_head,YEAR_OFFSET) !=0 )
	{
		DP("the year head is wrong!!!!!\n");
		return -1;
	}
	return 0;
}

/***************************************************
 * 由传入时间，得到天的块的位置
 *
 ***************************************************/
/*ps：bug:因为时间可能出错，所以一个时间可能对应多个块，但是现在只返回第一块
 * 		  算法可以改进
 */
long long year_get_day_from_time(time_t this_time)
{
	struct year_block *yb=(struct year_block *)(buf_head+YEAR_OFFSET+sizeof(struct year_queue_data));
	//struct year_queue_data *p=(struct year_queue_data *)(buf_head+YEAR_OFFSET);

	printf("this_time:%d    %d\n",this_time,this_time/SECOFDAY);
	/*从前到后，查到第一个对应的天返回，其实可能查到多个对应的天*/
	int i;
	for(i=0; i<MAXDAY; i++)
	{
		//if(i<10)
			printf(":%d  :%d     :%lld\n",yb[i].time/SECOFDAY, yb[i].time,yb[i].seek);
		if(yb[i].time/SECOFDAY == this_time/SECOFDAY)
			return yb[i].seek;
	}
	return -1;
}

/****************************************************
 * 获取硬盘格式是否格式化标志位
 *
 ***************************************************/
inline BOOL get_disk_format_mark()
{
	return disk_format_mark;
}
