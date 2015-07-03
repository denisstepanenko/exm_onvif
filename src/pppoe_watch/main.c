#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <file_def.h>
#include <time.h>
#include <mod_cmd.h>
#include "commonlib.h"
#include "gtlog.h"
#include "send_state.h"
#include "gtthread.h"

#define VERSION "2.03"	//版本号

// 2.03 lc  移植到ip1004设备内
// 2.02	wsy	同时支持模块间通信或消息队列的两种机制
// 2.01 wsy 恢复USR_LOGINED,增加读到信息后放弃之后3行的机制
// 2.00	wsy	重写整个流程,以纠正日志文件滚动时程序不再正常报状态问题,并支持vsmain的查询状态命令
//
// 1.05 wsy 将所有的用户帐号密码相关的验证失败都总结为pppoe_pap_failed
// 1.04	wsy	增加一个线程侦听模块通讯信息，在vmmain查询时返回自己状态
// 1.03 wsy	增加PAP authentication failed关键词
// 1.02	wsy	用socket取代消息队列
// 1.01  在00236	处将函数 fgets( log_buf, sizeof(log_buf)改为fread(log_buf,1,sizeof(log_buf),f_log);
// 因为fgets()读取日志gtlog.txt时每次只读取一个字符串的信息，时间延长时fgets()的
// 读取进度会跟不上日志gtlog.txt的记录，所以会有延时。改用fread()将日志中
// 新增加的所有字符串读到缓冲log_buf中，然后再分析字符数组的内容。



#define OK					0x00
#define ERR_OPENFILE		0x01	
#define ERR_READFILE		0x02
#define ERR_WRITFILE		0x03
#define ERR_STATFILE		0x04
#define ERR_NOFILEIP		0x05
#define ERR_SIGNAL			0x06
#define ERR_OVERTIME		0x07	
#define ERR_CANNOTINITCOM	0x08
#define ERR_OTHER			0x99

#define	BYTE	unsigned char
#define	WORD	unsigned short
#define	DWORD	unsigned long

#define PPPOEWATCH_LOCKFILE 	"/lock/ipserver/pppoe_watch"

typedef  struct{
	int retcode;
	char showmsg[32];
	char chkmsg[64];
}PPPOE_SEARCH_STR;

const PPPOE_SEARCH_STR PPPOE_SEARCH_STRING[] = {
	{PPPOE_SUCCESS,		"PPPOE_SUCCESS", 	"remote IP address"},
	{PPPOE_NO_MODEM,	"PPPOE_NO_MODEM",	"LCP: timeout sending Config-Requests"},
/*wsymod,把所有的验证失败都归于一种状态.PPPOE_PAP_FAILED.
	{PPPOE_PASSWD_ERR,	"PPPOE_PASSWD_ERR",	"ai-Service-Password"},
*/	
	{PPPOE_USR_TWICE,	"PPPOE_USR_TWICE",	"login limit 1 exceeded"},
/*	{PPPOE_USR_INVALID, "PPPOE_USR_INVALID","return Illegal Account"},*/
	{PPPOE_PAP_FAILED,  "PPPOE_PAP_FAILED", "PAP authentication failed"},
//在此处添加新状态!!!		
//	{0xffff,			"",					"*********"}
};


//打开一个文件直到成功
static FILE * OpenFileTillSucc( char *filename, char *openmode )
{
	FILE * fp;
	
	while ( 1 )
	{
		fp = fopen( filename, openmode );
		if( fp != NULL )
			return fp;
		else
		{
			printf( "pppoe_watch Open the %s error!\n",filename);
			sleep( 1 );
		}
	}
	
	return NULL;
}


//计算日志文件大小，单位byte
int get_logsize(void)
{
	int getstat = 0;
	struct stat statbuf;
	
	while(1)
		{
			getstat = stat("/log/gtlog.txt",&statbuf);
			if(getstat == 0)
				break;
			else
				sleep(1);
		}
	return statbuf.st_size;
}


//分析日志并发送pppoe状态的线程
void *  send_pppoe_status_thread( void * data )
{
	BYTE log_buf[1024];
	FILE * f_log; //日志的文件指针
	int oldlogsize = 0;//前一次的日志文件大小,单位byte
	int newlogsize=0; //当前的日志文件大小,单位byte
	int state;
	int i;
	int ret;
	int state_no; //需要对照的状态数目，自己计算
	int dropline = 0;	//从现在开始要drop的行数
	
	printf("send_pppoe_status_thread start running!\n");
	gtloginfo("send_pppoe_status_thread start running!\n");

	//Open the log file
	f_log = OpenFileTillSucc( "/log/gtlog.txt", "r" );
	fseek( f_log, 0, SEEK_END );
	oldlogsize = get_logsize();

	while( 1 )
	{
		
		if(fgets(log_buf,1000,f_log)== NULL) //没有读到
		{
			//检查是否滚动了文件
			newlogsize = get_logsize();
			if(newlogsize < oldlogsize) //滚动了
			{
				fclose(f_log);
				f_log = OpenFileTillSucc( "/log/gtlog.txt", "r" );
				oldlogsize = get_logsize();
			}
			else	//只是没有新纪录而已
			{
				oldlogsize = newlogsize;
				sleep(1);
			}
		
		}
		else //读到了
		{
			if(dropline!=0)
			{
				dropline--;
				continue;
			}
			state_no = sizeof(PPPOE_SEARCH_STRING)/sizeof(PPPOE_SEARCH_STR);
			//处理读到的这一行
			for(i=0; i< state_no; i++)
			{
				if(strstr(log_buf, PPPOE_SEARCH_STRING[i].chkmsg)!=NULL) //发现一条信息,发送
				{
					state = PPPOE_SEARCH_STRING[i].retcode;
					ret = send_pppoe_stat2main(state);
					printf("pppoe_watch->>Sending the pppoe message %s![%s]\n",ret<0?"FAILED":"OK",PPPOE_SEARCH_STRING[i].showmsg);
					gtloginfo("->> Sending the pppoe message %s![%s]\n",ret<0?"FAILED":"OK",PPPOE_SEARCH_STRING[i].showmsg);
					dropline = 3;
				}
			}
			
		}
			
	}
	
	return NULL;	
}


/* 
 *  通过实时分析 /log/gtlog.txt文件达到检测adsl连接进程状态的功能
 *	状态分为:
 *		Adsl 正常接通；
 *		找不到adsl modem；
 *		Adsl 帐号密码错误；
 *		等等..
*/
int main(int argc, char *argv[])
{ 	
	pthread_t send_pppoe_status_thread_id;
	pthread_t recv_modcom_thread_id;
	int ret;

		
	gtopenlog("pppoe_watch");
    if(create_lockfile_save_version(PPPOEWATCH_LOCKFILE,VERSION)<0)
    {
        printf("trying to start pppoe_watch but it's already running, exit!..\n");
        gtloginfo("trying to start pppoe_watch but it's already running,exit!..\n");
        exit(1);
    }
    printf("pppoe_watch [v%s] start running..\n",VERSION);
    gtloginfo("pppoe_watch [v%s] start running..\n",VERSION);

	//Init com channel
	ret = init_com_channel();
	if( ret < 0 )
	{
		printf("pppoe_watch init_com_channel error ret = %d, exit!!!\n",ret);
		gtloginfo("初始化进程间通讯失败ret = %d，退出\n",ret);
		exit(1);
	}	
	
	//创建线程send_pppoe_status_thread
	gt_create_thread(&send_pppoe_status_thread_id, send_pppoe_status_thread,NULL);

	//创建监听网络命令线程
	creat_recv_modsocket_thread();
	gt_create_thread(&recv_modcom_thread_id, recv_modcom_thread,NULL);
	
	while(1)
		sleep(1);
		
	//should never reach here
	return 0;
}
