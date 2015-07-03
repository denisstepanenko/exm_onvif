#include "watch_process.h"
#include <typedefine.h>
#include <file_def.h>
#include <mod_com.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <iniparser.h>
#include <commonlib.h>
#include <errno.h>
#include <sys/wait.h>

#include <devinfo.h>
#define USE_V1  					//wsy配置为支持1代系统的方式	
#define	TMP_FILE_GREP			"/var/tmp/watch_grep.tmp"		//进行grep操作用的临时文件名
#define   SAVE_DEBUG_INFO_DIR	"/log/debug"					//存储发现线程数目不足时的当事应用程序线程状况的目录


typedef struct{			//描述被监控对象的结构
	int	 min_threads;	//最小线程数//0表示不用管
	int	 ps_cnt;			//通过进程数量判断程序是否正常运行的计数器
	char  *prog_name;			//程序名(带路径)
	char  *lock_file;		//加锁文件名(带路径)
}watch_target_t;
static int	first_run_flag=1;	//初次运行标志，初次运行时启动应用程序记录日志的方式不同(不记录应用程序异常退出日志)

//描述具体的需要进行监控的应用程序的静态结构
watch_target_t 	watch_targets[]=
{//本定义中线程数都是基本线程,
	#ifdef USE_V1
		//lc to do 每个进程的线程数量需重新定义，根据稳定测试后确定
		{17,0,"/ip1004/encbox","/lock/ipserver/encbox"},
		{3,0,"/ip1004/diskman",DISKMAN_LOCK_FILE},
		{30,0,"/ip1004/rtimage",RT_LOCK_FILE},
		{4,0,"/ip1004/hdmodule",HDMOD_LOCK_FILE},
		{14,0,"/ip1004/ipmain",IPMAIN_LOCK_FILE},
		{7,0,"/ip1004/playback",HDPLAYBACK_LOCK_FILE}
	#endif
};
int	total_watch=sizeof(watch_targets)/sizeof(watch_target_t);	//总共需要监控的应用程序数量

//lc 2014-2-25 对于不同配置选项，可能执行不同的target集合，通过配置和版本区分
watch_target_t 	watch_targets2[]=
{//本定义中线程数都是基本线程,
	#ifdef USE_V1
		//lc to do 每个进程的线程数量需重新定义，根据稳定测试后确定
		{16,0,"/ip1004/encbox","/lock/ipserver/encbox"},
		{3,0,"/ip1004/diskman",DISKMAN_LOCK_FILE},
		{29,0,"/ip1004/rtimage",RT_LOCK_FILE},
		{10,0,"/ip1004/hdmodule",HDMOD_LOCK_FILE},
		{14,0,"/ip1004/ipmain",IPMAIN_LOCK_FILE},
		{7,0,"/ip1004/playback",HDPLAYBACK_LOCK_FILE}
	#endif
};
int	total_watch2=sizeof(watch_targets2)/sizeof(watch_target_t);	//总共需要监控的应用程序数量


static int g_playback_enable = 1;
static int g_multichannel_enable = 0;

//根据应用程序名找到对应的应用信息结构指针
//NULL表示没有找到
static watch_target_t *get_target_by_name(char *name)
{
	int i;
	watch_target_t *cur=NULL;
	int watchnum = 0;
	const watch_target_t *target = NULL;
	if(name==NULL)
		return NULL;
	
	if(g_multichannel_enable)
	{
		target = watch_targets2;
		watchnum = total_watch2;
	}
	else
	{	
		target = watch_targets;
		watchnum = total_watch;
	}
	
	for(i=0;i<watchnum;i++)
	{
		cur=&target[i];
		if(strcmp(cur->prog_name,name)==0)
		{
			return cur;	//找到了匹配的信息
		}
	}
	return NULL;			//没找到
}
//根据不同型号的信息来修改应用程序应有的线程数
static void fix_watch_structs(void)
{
	watch_target_t *w=NULL;
	//ipmain
	w=get_target_by_name("ipmain");
	if(w!=NULL)
	{
		if(get_quad_flag())
		{
			if(w->min_threads!=0)					//=0表示不需要监控
				w->min_threads+=1;
		}
	}

	//encbox
	w=get_target_by_name("encbox");
	if(w!=NULL)
	{
		if(w->min_threads!=0)						//=0表示不需要监控
		{
			w->min_threads+=get_videoenc_num();	//一个视频编码器需要一个线程
		}
	}
}


/**************************************************************************
  *函数名	:find_str_in_file
  *功能	:在文件中找指定的字字符串出现的次数
  *参数	: FileName:被操作的文件名，Str要查找的字符串
  *返回值	:正值表示指定字符串出现的次数，负值表示出错
  *************************************************************************/
int find_str_in_file(char *FileName,char *Str)
{
	int Cnt=0;
	FILE *Fp=NULL;
	char  ReadBuf[256];
	char  *PR;
	char *p;
	if(FileName==NULL)
		return -EINVAL;
	Fp=fopen(FileName,"r");
	if(Fp==NULL)
		return -errno;
	while(1)
	{
		PR=fgets(ReadBuf,sizeof(ReadBuf),Fp);
		if(PR==NULL)
		{
			//printf("errno=%d:%s\n",errno,strerror(errno));
			break;
		}
		p=strstr(PR,Str);
		if(p!=NULL)
			Cnt++;
	}
	fclose(Fp);
	//printf("%d lines!!\n",Lines);
	return Cnt;		
}

/**************************************************************************
  *函数名	:grep_process2file
  *功能	:将指定进程名的信息输出到指定文件中
  *参数	: prog_name:进程名，FileName:输出文件名
  *返回值	:0表示成功，负值表示出错
  *************************************************************************/
int grep_process2file(char *prog_name,char *FileNmae)
{
	int ret;
	char PBuf[256];
	if((prog_name==NULL)||(FileNmae==NULL))
	{
		return -EINVAL;
	}
	sprintf(PBuf,"ps | grep %s>%s",prog_name,FileNmae);
	ret=system(PBuf);
	printf("test grep_process2file PBuf=%s ret=%x errno=%d:%s!!!\n",PBuf,ret,errno,strerror(errno));
/*
	if(ret!=0)
		return -errno;
		*/
	return 0;	
}

/**************************************************************************
  *函数名	:log_ps_info2file
  *功能	:将指定的进程的信息记录到单独的日志文件中
  *			 /log/debug/prog_name ,采用追加的方式，如果文件长度
  *			大于一定值(8192 byte)以后则清0
  *参数	: prog_name:进程名,用于生成相应的日志文件，
  *			  min_threads:进程配置的最小线程数
  *		         CurThreads:进程当前的总线程数
  *返回值	:无
  *************************************************************************/
void log_ps_info2file(char *prog_name,int min_threads,int CurThreads)
{
	char LogFile[256];
	char *SaveFlag=">>";
	char SaveBuf[256];
	struct stat FStat;
	//int ret;
	sprintf(LogFile,"%s/%s",SAVE_DEBUG_INFO_DIR,prog_name);
	if(stat(LogFile,&FStat)>=0)
	{
		if(FStat.st_size>8192)
			SaveFlag=">";
	}
	sprintf(SaveBuf,"date %s %s",SaveFlag,LogFile);
	system(SaveBuf);

	sprintf(SaveBuf,"echo %s min_threads=%d CurThreads=%d >>%s",prog_name,min_threads,CurThreads,LogFile);
	system(SaveBuf);

	sprintf(SaveBuf,"ps |grep %s >>%s",prog_name,LogFile);
	system(SaveBuf);
	
	sprintf(SaveBuf,"echo ====================================================>>%s",LogFile);
	system(SaveBuf);
#if 0
	sprintf(SaveBuf,"echo \"old file info:\n\">>%s",LogFile);
	system(SaveBuf);

	sprintf(SaveBuf,"cat %s>>%s",TMP_FILE_GREP,LogFile);
	system(SaveBuf);	
#endif




	
}

/**************************************************************************
  *函数名	:run_target
  *功能	:判断是否应该启动监控目标并启动
  *参数	: target:描述被监控对象的数据结构指针
  *返回值	:0表示成功，负值表示出错
  *************************************************************************/
int run_target(watch_target_t *target)
{
	int l_fd=-1;
	int ret;
	pid_t pid;
	char *prog_name=NULL;
	char k_buf[100];
	int thread_num;
	l_fd=create_and_lockfile(target->lock_file);		//试图锁住文件
	do
	{
		if(l_fd<0)
		{	//目标程序已经运行
			if(target->min_threads<=0)			//不需要判断进程数量
				return 0;						//程序正常运行
			else
			{//判断目标进程的线程数是不是满足最小值以及是否有僵尸进程
				if(++target->ps_cnt>2)
				{//	20秒(如果监控时间间隔设的是10秒)判断一次线程数
					target->ps_cnt=0;
					//获取程序名
					prog_name=rindex(target->prog_name,'/');
					if(prog_name!=NULL)
						prog_name++;
					else
						prog_name=target->prog_name;	

					
					ret=grep_process2file(prog_name,TMP_FILE_GREP);
					//如果读取线程数失败则直接返回
					if(ret!=0)	
						return 0;
					//lc do 新的方式获取线程数
					thread_num=get_ps_threadnum(prog_name,TMP_FILE_GREP);
										
					if(thread_num>=target->min_threads)
					{
						ret=find_str_in_file(TMP_FILE_GREP," Z ");//判断是否有僵尸进程
						if(ret>0)
						{	//发现有僵尸进程
							gtloginfo("find %s have %d Z threads!!!\n",prog_name,ret);
							log_ps_info2file(prog_name,target->min_threads,thread_num);
							break;
						}
						else
						{
							return 0;
						}
					}
					else
					{
						sleep(1);
						thread_num=get_ps_threadnum(prog_name,TMP_FILE_GREP);
						if((thread_num>0)&&(thread_num<target->min_threads))
						{
							//进程的实际运行线程数小于预设的最小值
							if(!first_run_flag)
							{
								gtloginfo("%s min_threads=%d Current=%d!!\n",prog_name,target->min_threads,thread_num);
								log_ps_info2file(prog_name,target->min_threads,thread_num);
							}
							break;
						}
						else
						{
							//读取文件出错则不用重新启动进程
							return 0;
						}
						
					}
					
				}
				else
					return 0;
			}
		}
		else
		{
			// 进程还没有启动或异常退出
			if(!first_run_flag)
				gtloginfo("%s maybe exited ,start it!\n",target->prog_name);
			flock(l_fd,LOCK_UN);
			fsync(l_fd);
			close(l_fd);	//关闭已经打开的文件
			break;
		}
	}while(0);

	pid=fork();
	if(pid==0)
	{
		gtlogerr("watch_process fork child pid is %d,last errno is %d\n",pid,errno);
		setsid();
		//结束进程的其它副本	
		prog_name=rindex(target->prog_name,'/');
		if(prog_name!=NULL)
			prog_name++;
		else
			prog_name=target->prog_name;	
		
		chdir("/");
		umask(0);

		//结束所有已经启动的副本
		printf("watch_process2 kill %s!!\n",prog_name);
		sprintf(k_buf,"killall -15 %s",prog_name);
		ret=system(k_buf);		

		//关闭已经打开的所有资源
		close_all_res();
		sleep(1);
		printf("watch_process2 start %s !!\n",target->prog_name);
		gtloginfo("watch_process2 start %s !!\n",target->prog_name);
		sprintf(k_buf,"%s &",target->prog_name);
		
		//在后台启动目标进程
		ret=system(k_buf);
		if(ret==0)
			exit(0);
		else
			exit(1);	

	}
	else
	{
		printf("%s fork=%d !!\n",target->prog_name,(int)pid);
	}
	
	return 0;
}
#if 0
/**************************************************************************
  *函数名	:CheckFileSize
  *功能	:检查指定的文件是否超过规定的大小
  *			 如果超过则将其更名为.0 .0改为.1...
  *			 最多纪录MaxNum个文件
  *参数	: target:描述被监控对象的数据结构指针
  *返回值	:0表示成功，负值表示出错
  *************************************************************************/
void CheckFileSize(char *FileName,int Size,int MaxNum)
{
	int i;
	int ret;	

	struct stat Stat;
	if((FileName==NULL)||(Size<0)||(MaxNum<0))
	{
		return;
	}
	ret=stat(FileName,&Stat);
	if(ret<0)
		return ;
	if(Stat.st_size<Size)
		return;
	char TmpFile[100];
	char OldFile[100];
	char NewFile[100];
	char SBuf[256];
	for(i=(MaxNum-1);i>0;i--)
	{
		 sprintf(OldFile, "%s.%d", FileName, i-1);
               sprintf(NewFile, "%s.%d", FileName, i);
               rename(OldFile, NewFile);
	}
	sprintf(NewFile,"%s.0",FileName);
	sprintf(TmpFile,"%s.tmp",FileName);

	rename(FileName,TmpFile);
	
	sprintf(SBuf,"cp %s %s -f",TmpFile,NewFile);
	system(SBuf);


	remove(TmpFile);
	
	return;
	
}
#endif

void OnSIGSEGV(int n,struct siginfo *siginfo,void *myact)  
{  
        int i, num;  
        char **calls;  
        gtlogerr("Fault address:%X\n",siginfo->si_addr);     
        //num = backtrace(buffer, SIZE);  
        //calls = backtrace_symbols(buffer, num);  
        //for (i = 0; i < num; i++)  
        //        printf("%s\n", calls[i]);  

		gtlogerr("pid is %d\n sigment fault!\n");
		
        exit(1);  
}    

/**************************************************************************
  *函数名	:main
  *功能	:watch_proc的主函数
  *参数	: 无
  *返回值	:应该用不返回
  *************************************************************************/
int main(void)
{
	int timeout;
	dictionary      *ini;	
	int lock_file;
	char pbuf[100];
	int pid;
	int Status;
	int i;
	int watchnum = 0;
	const watch_target_t *target = NULL;
#ifdef USE_V1
	#warning "this program will compile for v1 system!"
#else
	#warning "this program will compile for v2 system!"
#endif
	close_all_res();		//关闭已经打开的所有资源
	gtopenlog("watch_proc");

	//判断模块是否已经执行
	lock_file=create_and_lockfile(WATCH_LOCK_FILE);
	if(lock_file<=0)
	{
		printf("watch_process module are running!!\n");
		gtlogerr("watch_process 模块已运行，故启动无效退出\n");		
		exit(0);
	}	
	sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
	write(lock_file,pbuf,strlen(pbuf)+1);//将进程的id号存入锁文件中
	
	printf("watch_proc(%s) running...\n",VERSION);
	gtloginfo("watch_proc(%s) running...\n",VERSION);

	 struct sigaction act;
     int sig = SIGSEGV;
     sigemptyset(&act.sa_mask);
     act.sa_sigaction = OnSIGSEGV;
     act.sa_flags = SA_SIGINFO;
     if(sigaction(sig, &act, NULL)<0)
     {
        printf("sigaction error!\n");
		exit(1);
     }
	

	init_devinfo();
	fix_watch_structs();
	//从配置文件中读取监控时间间隔参数，默认值10秒
	ini=iniparser_load(WATCH_PARA_FILE);
	if (ini==NULL) 
	{
            	printf("watch_proc  cannot parse ini file file [%s]", WATCH_PARA_FILE);
		gtlogerr("watch_proc 无法解析ini文件[%s]",WATCH_PARA_FILE);
            	timeout=10;
        }
	else
	{
		timeout=iniparser_getint(ini,"product:watch_interval",10);
		if(timeout>1800)//不能超过半个小时
			timeout=10;
		if(timeout<1)
			timeout=1;

		//lc 2014-2-25
		g_playback_enable = iniparser_getint(ini,"netencoder:playback_enable",1);  //default 1
		g_multichannel_enable = iniparser_getint(ini,"multichannel:enable",0);     //default 0
		
		iniparser_freedict(ini);
	}
	//创建用于记录调试日志的目录
	if(access(SAVE_DEBUG_INFO_DIR,F_OK)!=0)
	{
		if(mkdir(SAVE_DEBUG_INFO_DIR,0755)<0)
		{//不能创建目录
			printf("can't create dir:%s!!\n",SAVE_DEBUG_INFO_DIR);
			gtlogerr("can't create dir:%s!!\n",SAVE_DEBUG_INFO_DIR);
		}
	}

	if(g_multichannel_enable)
	{
		target = watch_targets2;
		watchnum = total_watch2;
	}
	else
	{	
		target = watch_targets;
		watchnum = total_watch;
	}

	while(1)
	{
		//对所有的对象进行探测并执行
		for(i=0;i<watchnum;i++)
		{
			run_target(&target[i]);
		}
		
		for(i=0;i<10;i++)
		{
			//收集已经退出的子进程资源
			pid=waitpid(-1,&Status,WNOHANG);
			if((int)pid>0)
                	{
                        continue;
                	}
			else
				break;
			
		}

		sleep(timeout);	//
		first_run_flag=0;	//程序已经启动，清除首次运行标志
	}

	return 0;
}


