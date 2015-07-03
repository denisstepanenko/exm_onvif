/****************************************************************************
 * 功能：year_blocks.c date_blocks.c不相关的功能函数。比如输出，时间等
 *
 ****************************************************************************/

#include "blocks.h"
/*全局变量*/
media_source_t media;


static long long max_blocks=MAXBLOCKS;
/*硬盘满标志位；硬盘满后，就要循环写入；所以是否满处理方式不同*/
BOOL hd_full_mark = FALSE;

inline long long get_hd_max_blocks()
{
	return max_blocks;
}

/*设置硬盘满标志位为真*/
inline void hd_full_mark_set(BOOL mark)
{
	hd_full_mark = mark;
}
/*读取硬盘满标志位*/
inline BOOL hd_full_mark_get()
{
	return hd_full_mark;
}

/*******************************时间函数**********************/
/*获取时间，时间就是从1970年到现在的秒数*/
int get_my_time()
{
	return (int) time(NULL);
}
#if 0
struct tm *gmtime(const time_t *calptr)
struct tm
{
	int tm_sec; /* Seconds.	[0-60] (1 leap second) */
	int tm_min; /* Minutes.	[0-59] */
	int tm_hour; /* Hours.	[0-23] */
	int tm_mday; /* Day.		[1-31] */
	int tm_mon; /* Month.	[0-11] */
	int tm_year; /* Year	- 1900.  */
	int tm_wday; /* Day of week.	[0-6] */
	int tm_yday; /* Days in year.[0-365]	*/
	int tm_isdst; /* DST.		[-1/0/1]*/
}

time_t mktime (struct tm *__tp)
#endif

/*******************************时间函数**********************/

/***************debug***************************************************/
/************************************************************
 * 格式化输出大量的数据
 ************************************************************/
int myprint(unsigned char *p, long size)
{
	unsigned int i;
	for (i = 0; i < size; i++)
	{
		if (i % 10 == 0)
			printf("\n%-8d", i);
		printf("    %-2X", *(p + i));
	}
	printf("\n");
	return 0;
}

/********fifo***************************************************************/
static int fd;
/*初始化管道*/
int fifo_init()
{
	mkfifo("test.264", 0777);
	int ret;
	fd = open("test.264", O_WRONLY);
	if (fd < 0)
	{
		perror("openfile error");
		return -1;
	}
	return 0;
}
/*写管道*/
int fifo_write(char *buf, int size)
{
	int ret = write(fd, buf, size);
	if (ret < 0)
	{
		//perror("write error\n");
		return -1;
	}
	return ret;
}
/*释放管道*/
void fifo_free()
{
	close(fd);
}
/********fifo***************************************************************/
