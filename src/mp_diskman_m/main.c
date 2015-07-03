#include "mp_diskman.h"
#include "diskmanager.h"
#include <commonlib.h>
#include "process_modcmd.h"
#include <devinfo.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "process_modcmd.h"
#include "mpdisk.h"
#include "diskinfo.h"
#include "hdutil.h"
#include "sqlite3.h"
#include "fileindex.h"
#include <errno.h>

#if 0
//收集子进程
void collect_child(void)
{
	pid_t Pid;
	int Status;
	int i;
	for(i=0;i<5;i++)
	{
		Pid=waitpid(-1,&Status,WNOHANG);
		if((int)Pid>0)
          	{
				  gtloginfo("收集了一个子进程\n");		
                  continue;
          	}
		else
			break;
	}
}
#endif



struct partition_info_struct part_info[MAX_DISKNO][PART_PER_DISK];

struct partition_info_struct * get_part_info(int diskno, int partno)
{
	if((diskno >= get_disk_no())||(partno >= PART_PER_DISK))
		return NULL;
	else	
		return &part_info[diskno][partno];
}

//在日志上记录退出过程
static void exit_log(int signo)
{
	switch(signo)
	{
		case SIGPIPE:
			printf("diskman process_sig_pipe \n");	
			return ;
		break;
		case SIGTERM:
			gtloginfo("diskman 被kill,程序退出!!\n");
			close_all_res();
			exit(0);
		break;
		case SIGKILL:
			gtloginfo("diskman SIGKILL,程序退出!!\n");
			close_all_res();
			exit(0);
		break;
		case SIGINT:
			gtloginfo("diskman 被用户终止(ctrl-c)\n");
			close_all_res();
			exit(0);
		break;
		case SIGUSR1://输出系统信息到指定文件
			dump_sysinfo();
		break;
		//case SIGCHLD:
			// collect_child();
		//break;
		case SIGSEGV:
			gtloginfo("diskman 发生段错误\n");
			close_all_res();
			printf("diskman segmentation fault\n");
			exit(0);
		break;
	}
	return;
}

void second_proc(void) 
{
	while(1)
	{	
		//remove_oldest_file();
		sleep(10);
	}
}

//获取一个默认属性结构
//用完了应该释放
static int get_gtthread_attr(pthread_attr_t *attr)
{
	int rc;
	if(attr==NULL)
		return -1;
	memset((void*)attr,0,sizeof(pthread_attr_t));
	rc=pthread_attr_init(attr);
	if(rc<0)
		return -1;
	rc=pthread_attr_setdetachstate(attr,PTHREAD_CREATE_DETACHED);//分离状态
	rc=pthread_attr_setschedpolicy(attr,SCHED_OTHER);
	return 0;
}


/**********************************************************************************************
 * 函数名	:set_hd_capacity_to_parafile()
 * 功能	:设置设备的磁盘容量值
 * 输入	:value:程序检测到的磁盘容量值MB为单位
 * 返回值	:0表示成功，负值表示出错
 **********************************************************************************************/
int set_hd_capacity_to_parafile(int value)
{
	dictionary *ini=NULL;
	FILE *fp=NULL;
	
	ini=iniparser_load_lockfile(IPMAIN_PARA_FILE,1,&fp);
	if(ini==NULL)
		return -EINVAL;
		
	if(value>=0)
	{
		iniparser_setint(ini,"resource:disk_capacity",value);
	}
	save_inidict_file(IPMAIN_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);
	
	if(virdev_get_virdev_number()==2)//虚拟设备
	{
		system("/ip1004/ini_conv -s");
	}
	return 0;
}

int main(void)
{
	pthread_attr_t  thread_attr,*attr;
	int lock_file;
	char pbuf[100];
	int i;
	int parttotal=0;//所有硬盘的分区数量之和
	long disktotal = 0; //所有硬盘的总容量
#ifdef FOR_PC_MUTI_TEST
	gen_multi_devices();
#endif
	signal(SIGKILL,exit_log);
	signal(SIGTERM,exit_log);
	signal(SIGINT, exit_log);
	signal(SIGSEGV,exit_log);
	signal(SIGPIPE,exit_log);
	signal(SIGUSR1,exit_log);
	signal(SIGCHLD,exit_log);
	setsid();
	gtopenlog("diskman");

	lock_file=create_and_lockfile(DISKMAN_LOCK_FILE);
	if(lock_file<=0)
	{
		printf("diskman module are running!!\n");
		gtlogerr("diskman模块已运行，故启动无效退出\n");		
		exit(0);
	}	
	sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
	write(lock_file,pbuf,strlen(pbuf)+1);//将进程的id号存入锁文件中
	//以上完成判断模块是否已经执行的功能
#ifdef FOR_PC_MUTI_TEST
	SetOutputTty(lock_file);// 将输出影射到另一个描述符
#endif
	printf("启动diskman(ver:%s).......\n",VERSION);
	gtloginfo("启动diskman(ver:%s).......\n",VERSION);

	////zw-test		
	//fileindex_init_filelock();
	////zw-test
	InitAllDabase();



	init_devinfo();//初始化设备信息
	init_com_channel();//初始化通讯命令通道
	init_diskman();// 处理目录和变量
	read_diskman_para_file(IPMAIN_PARA_FILE);//从配置文件读参数

	if(get_gtthread_attr(&thread_attr)==0)
		attr=&thread_attr;
	else
		attr=NULL;
	creat_diskman_modsocket_thread();
	if(get_ide_flag()!=0) //有存储盘
	{
	//以下为检查和修理磁盘,对硬盘应有的每个分区都作检查和挂载
#if 0//wsy,移到init_all_ide_drv中执行。EMBEDED==1 	
		mpdisk_check_disknode();
		mpdisk_creat_partitions(); //在rc.conf中应该已经建了，这是保险起见
	
		if(access(REBOOT_FILE,F_OK)!=0) //刚刚重起
		{
			
			hdutil_e2fsck_and_mount_all_partitions();
			close(creat(REBOOT_FILE,F_OK|W_OK|R_OK));
		}
#endif
		//hdutil_init_all_partitions();
		//disktotal=mpdisk_get_sys_disktotal();
		for(i=0;i<get_sys_disk_num();i++)
		{
	               //获取磁盘容量，分区数然后保存到配置文件中去	
			disktotal+= get_sys_disk_capacity(get_sys_disk_name(i));  //MB
			parttotal+= get_sys_partition_num(get_sys_disk_name(i));
		}
		set_hd_capacity_to_parafile(disktotal); //setting the capacity to the file /conf/gt1000.ini
		set_hd_capacity(disktotal);             //setting the capacity to the file  /conf/devinfo.ini
		
		if(disktotal>200)//
		{
			if(disktotal<20*1024)//小于20G的认为是CF卡 
				gtloginfo("检测到可用的CF卡,容量:%dMB,分区数%d\n",disktotal,parttotal);
			else
				gtloginfo("检测到可用的硬盘,容量:%dMB,分区数%d\n",disktotal,parttotal);
		}	
		else
		{//没有mount上		
			gtlogerr("没有检测到可用的硬盘/CF卡\n");//add string by shixin
		}
	}
	
	
	creat_diskman_thread(attr,NULL);
	if(attr!=NULL)
	{
		pthread_attr_destroy(attr);
	}
	second_proc();
	exit(0);
}


