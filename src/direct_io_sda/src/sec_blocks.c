/***********************************************************************
 * 功能：处理秒块操作
 *
 **********************************************************************/
#include "blocks.h"
//debug1
time_t time_tmp_1;
static	long long thisseek;

struct hd_frame
{
	char data_head[8];			/*帧数据头的标志位 0x5345434fa55a5aa5*/
	short frontIframe;			/*前一个I帧相对于本帧的偏移地址   如果前一帧在硬盘的最后面，这个值可能是负值*/
	short is_I;		/*本帧是不是I帧*/
	unsigned int size;			/*本帧视频数据块的大小*/
};

/*存储秒块的的缓存， */
static char frame_buffer[BUFFER_SIZE];	//帧缓冲区大小400k

/*秒头*/
static char framehead[8]={0x53,0x45,0x43,0x4f,0xa5,0x5a,0x5a,0xa5};
/*存硬盘数据指针头*/
char *hd_frame_buff=NULL;
/*最近的前一个I帧的偏移地址*/



//其它数据
static int		seq=-1;                 ///<媒体数据序号
static int 	flag;
/*当有帧I帧时才开始存*/
int bool_start=0;
/***********************************************************************
 * 功能：获取秒块的大小
 * 原理：从seek处读取一块
 **********************************************************************/
int sec_get_block_size(long long seek)
{
	int ret;
	long long blocks=1;
	char *p=frame_buffer;
	ret=hd_read( get_hd_fd(),  seek, blocks, p,blocks*BLOCKSIZE);
	if(ret!=0)
	{
		printf("read block 1 error,and i will exit() \n");
		return -1;
	}
	//myprint(date_buf,DATE_HEAD_BLOCK_SIZE);
	if(memcmp(p,framehead,8)!=0)
	{
		/*出错啦，怎么办？*/
		/*输出这是第几个错误了*/
		printf("2seek:%lld\n",seek);
		printf("wrong data\n");
		myprint((unsigned char *)p,BLOCKSIZE);

		long long  tmp1=0;
		int tmp2=1;
		/*开始错误处理*/
		while(1)
		{
			p=frame_buffer;
			/*先读700块*/
			blocks=700;
			memset(p,0,BUFFER_SIZE);
			ret=hd_read(get_hd_fd(),  seek, blocks, p,blocks*BLOCKSIZE);
			if(ret!=0)
			{
				printf("read block 1 error,and i will exit() \n");
				return -1;
			}
			/*如果这读这一块，前面几个数据对了，那就接上了*/
			for(tmp1=0; tmp1<=blocks; tmp1++)
			{
				/*找到一块好的了。*/
				if(memcmp(p+BLOCKSIZE*tmp1,framehead,8)==0)
				{
					printf("\n\n\naaaa:tmp1:%lld\n",tmp1);
					/*下次从这开始*/
					seek = seek + tmp1;
					printf("1seek:%lld\n",seek);
					tmp2=0;
					//break;
					return -1;
				}
			}
			seek += blocks;
			printf("2seek:%lld\n",seek);
		}
	}
	/*
	else
		printf("the frame size:%d\n",*(int *)(p+12));
	*/
	//myprint((unsigned char *)p,BLOCKSIZE);
	int tmp_size= *(int *)(p+12);
	return tmp_size;

}

/**********************************************************
 * 功能：如果硬盘最后空间不够写了；则要从硬盘开头写
 *      1、如果此时硬盘最开始块是天块
 *      2、如果此时硬盘最开始块是秒（帧）块
 *返回：返回要写硬盘的块的序号
 *
 **********************************************************/
long long  turn_to_hdhead()
{
	struct year_block *tmp=(struct year_block *)year_get_head_address();
	/*如果硬盘开始第一块是天块*/
	if( tmp->seek==YEAR_HEAD_BLOCK_SIZE )
	{
		DP("it is day block\n");
		/*在年块中插入新的一块，同时把把队列的头和尾分别”移动“*/
		year_insert_new_day_block(YEAR_HEAD_BLOCK_SIZE+1);//yk add +1 20130802
		/*那就返回天块，在这写了*/
		return YEAR_HEAD_BLOCK_SIZE+1;
	}
	/*是秒块*/
	DP("it is sec block\n");
	return YEAR_HEAD_BLOCK_SIZE+1;
}
/***********************************************************
 * 功能：把帧写入硬盘
 * 注意：这运行此函数前要初始化内存池
 * 参数:seeks写入的位置，buff,写入帧缓冲区，buffsize,写入数据大小
 * 返回：
 **********************************************************/
int sec_write_frame(long long seeks,char *buff,unsigned int buffsize)
{

	return 0;
}

/**********************************************************
 * 上层接口
 * 功能：写文件
 *
 *
 **********************************************************/
int sec_write_data()
{
	int ret;


	/*把前8个字符置成规定的值*/
	struct hd_frame framehead={0x53,0x45,0x43,0x4f,0xa5,0x5a,0x5a,0xa5};

	/*最近的前一个I帧的偏移地址*/
	long long  frontIseek=day_get_sec_blocks();;
	/*获取当前应该写入的位置*/
    int64_t seek =day_get_sec_blocks();

    printf("\n\n\nget seek:%lld\n",seek);


    /*!!!!!!!!bug   当年块后面不是天块时就有问题了*/
    if(seek < YEAR_HEAD_BLOCK_SIZE+DATE_HEAD_BLOCK_SIZE)
    {
    	printf("error seek and exit\n");
    	return -1;
    }

	//初始化数据
	memset(&media ,0, sizeof(media_source_t));
	media.dev_stat= -1; //表示没有连接


	ret=connect_media_read(&media ,0x30000, "video", /*MSHMPOOL_LOCAL_USR*/1);
	if(ret<0)
	{
		printf("error in connect media read and exit\n");
		return -1;
	}


	static int i_times=0;

	/*现在是一天开始的那一刻*/
	time_t in_new_day;

#ifdef DEBUG_LOG
	/*过了几天时的*/
	static int allday_times=0;
	static long long all_frames=1;
	int tmp_time=get_my_time();
#endif /*DEBUG_LOG*/
	//debug
	printf("now we begin read media pool\n");
	while(1)
	{
		//debug1
		//time_tmp_1=get_my_time();
		//day_cmp_date("11",time_tmp_1);

#ifdef DEBUG_LOG
	//printf("this frame :%lld\n",all_frames++);
#endif /*DEBUG_LOG*/

	    /*检查一下当前时间，如果一天结束了，新的一天开始了怎么办呢？*/
		if((get_my_time()%SECOFDAY==0)&&(in_new_day!=get_my_time()) )
		{
			/*在1s内可能有很多帧*/
			in_new_day=get_my_time();
			printf("1day:seek:%lld\n",seek);
			seek=day_new(seek);// seek被传入，肯定要变了。怎么让它变
			printf("2day:seek:%lld\n\n\n",seek);

#ifdef DEBUG_LOG
			allday_times++;
			printf("this is day:\t\t%d\n\n\n\n\n\n",allday_times);
			//if(allday_times>2)return 0;
#endif /*DEBUG_LOG*/
		}

		//读取帧，返回值为帧大小 但是kframe是什么呢？　难道flag 就是是不是I帧的那项？
		memset(frame_buffer, 0, BUFFER_SIZE);
		seq=-1;flag=-1;
		ret=read_media_resource(&media,frame_buffer, BUFFER_SIZE, &seq, &flag);
		if(ret<0)
		{
			printf("error in read media resource\n");
			exit(1);
		}
		enc_frame_t* the_frame_buffer=(enc_frame_t *)frame_buffer;

		int is_i;
		if(flag==1)
			is_i=0;
		else
			is_i=1;
		/*当有一个I帧时才开始执行下面的代码，存储视频数据*/
		if(is_i){bool_start=1;}
		if(!bool_start)continue;

		/*！！！把视频帧的地址向前偏移16位，存放存入硬盘里的帧头！！！！这种操作危险*/
		hd_frame_buff=the_frame_buffer->frame_buf-sizeof(struct hd_frame);

		/*本块的长要+块头*/
		//int blocks=	(( (the_frame_buffer->len +sizeof(struct hd_frame)) + BLOCKSIZE -1)/BLOCKSIZE);
		int blocks=	(( (the_frame_buffer->len +sizeof(struct hd_frame)) + BLOCKSIZE -1)/BLOCKSIZE);//yk change 20130731

		/*检查硬盘剩下的空间是不是够用*/
		if(blocks > get_hd_max_blocks() - seek)
		{
			//printf("11seek=%lld\n",seek);
			//printf("22the ---:%lld\n",(get_hd_max_blocks() - seek));
			//printf("33the hd don't have enough blocks---blocks:%d,seek:%lld\n",blocks,seek);
			/***********************************************
			 怎么处理??????????????????把seek值改了。返回就ok了
			***********************************************/
			seek =turn_to_hdhead();
			printf("44seek=%lld\n",seek);
		}

		/*最近的前一I帧偏移地址为*/
		framehead.frontIframe=seek-frontIseek;				//这会不会出错呢？？？？
		framehead.size=the_frame_buffer->len;


		/*当本帧是I帧时，把本帧的块seek存入*/
		if(is_i)
		{

			frontIseek =seek;
			framehead.is_I=1;
		    /*如果一秒中没有两个I帧，更新此秒（帧）块在天块中索引信息*/
			time_t current_time=get_my_time();
		    if(day_get_lastsec_block()!=current_time)
		    {
		    	DP("set sec block\n");
		    	day_set_sec_block(seek,blocks,current_time);/*注意：这个函数参数名字有点不太好*/
		    }
		    /*有100次I帧时把天块写入硬盘*/
		    i_times++;
		    //printf("i_time:%d\n",i_times);
		    if(i_times%20==0)
			{
		    	DP("write day\n");
		    	day_save1();
		    	//return 0;
			}
			 printf("I\n\n\n");
		}

		//day_cmp_date("22",time_tmp_1);
		/*把帧描述头写入hd_frame_buff中*/
		memcpy(hd_frame_buff,&framehead,sizeof(struct hd_frame));
		/*存硬盘*/
		ret=hd_write(get_hd_fd(), seek, blocks, hd_frame_buff,blocks*BLOCKSIZE);
	    if(ret!=0)
	    {
	    	printf("write date buf error ,plear resove the probram,and i will exit()\n");
	    	exit(1);
	    }
		//day_cmp_date("33",time_tmp_1);



#ifdef DEBUG_LOG
	    printf("currnet second:%d,second:%d,:sec:%d:seek:%lld\n",get_my_time(),get_my_time()-tmp_time,get_my_time()%60,seek);
	    //printf("len:%d,blocks:%d\n",(unsigned int)(framehead.size +sizeof(struct hd_frame)),blocks);
#else
	    printf("seek:%lld\n",seek);
#endif

	    /*下一块的位置*/
	    seek += blocks;


	}
	return 0;
}




inline void set_seek(long long seek)
{
	thisseek=seek;
}

/**********************************************************
 * 上层接口
 * 功能：读帧
 *
 *返回-1读出错 -2读出来后帧头出错误 -3新的一天中没有数据 -4未知的错误
 **********************************************************/
int sec_read_data()
{
	/*
	//上一帧的时间 比较有用
	time_t last_frame_time;
	*/
	long long *read_seek=&thisseek;
	long long *seek=read_seek;
	int ret;
	int blocks;
	char *p=frame_buffer;
	//while(1)
	{


		//usleep(40000);		//25帧
begin:
		blocks=1;
		memset(p,0,BUFFER_SIZE);
DP("1\n");
		ret=hd_read(get_hd_fd(),  *read_seek, blocks, p,blocks*BLOCKSIZE);
		if(ret!=0)
		{
			printf("read block 1 error,and i will exit() \n");
			//*seek=read_seek;/*如果读出错，通过指针把当前的seek传回去*/
			return -1;
		}
DP("2\n");
		//myprint(date_buf,DATE_HEAD_BLOCK_SIZE);
		if(memcmp(p,framehead,8)!=0 )
		{
			DP("21\n");
			long long ret1;
			/*出错啦，怎么办？*/
			/*输出这是第几个错误了*/
			printf("2seek:%lld\n",*read_seek);
			printf("wrong data\n");
			myprint((unsigned char *)p,/*BLOCKSIZE*/8);
			/*是天块的头*/
			if( day_head_memcmp(p)==0)
			{
				ret1=day_read_block(*read_seek);
				if(ret1>0)
				{
					printf("new day begin!!!\n");
					/*新的一天，从新的一天的第一个秒帧开始读*/
					*read_seek =ret1;
					DP("211\n");
					goto begin;
				}

				printf("ERR:not new day!!!may be it's bad block ret=%lld\n",ret1);
				/*错的一塌糊涂*/
				//*seek=read_seek;/*如果读出错，通过指针把当前的seek传回去*/

				return (int )ret1;
			}
			/*****************************************************************************************
			 * 错误处理：下一帧数据既不是秒帧，也不是天帧！A、可能是硬盘用到最后没空间了，又从头开始写了，所以要从头开始读
			 * B、这就是一个错误帧！！！（这种情况应该不会出现，除非sgio有问题）
			 * ****************************************************************************************/
			else
			{
				DP("22\n");
				long long ret1=-1;
				ret1=day_getnextbkad_formcur(*read_seek);

				if(ret1<YEAR_HEAD_BLOCK_SIZE+1)
				{
					if(1==ret1)
					{
						printf("WARRING::day is over,and sec frame is over!!!\n");
						*read_seek=YEAR_HEAD_BLOCK_SIZE+1;
						goto begin;
					}
					else
					{
						printf("ERR:UNKNOW ERROR!!!   \
								day_getnextbkad_formcur:::ret1=%ld",ret1);
						return ret1;
					}
				}

				printf("ret1:%lld\n",ret1);
				*read_seek=ret1;
				goto begin;
			}/*end of if( day_head_memcmp*/

		}



		//struct hd_frame
		int tmp_size= *(int *)(p+12);
DP("3\n");
		/*再次读取这一帧剩余部分*/
		blocks=(tmp_size+16 +512 -1)/512 -1;
		*read_seek = *read_seek+1;
		ret=hd_read(get_hd_fd(),  *read_seek, blocks, p+BLOCKSIZE,blocks*BLOCKSIZE);
		if(ret!=0)
		{
			printf("read block 1 error,and i will exit() \n");
			*seek=*read_seek-1;
			return -1;
		}
DP("4\n");
#if 1
		ret= fifo_write(p+16, tmp_size);
		if(ret<0)
		{
			perror("write error\n");
			exit(1);
		}
#endif
		printf("ret=%d\n",ret);
		*read_seek += blocks;
		printf("read seek:%lld\n",*read_seek);
	}
	return 0;
}

