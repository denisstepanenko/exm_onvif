#include "vs3_videoenc.h"
#include <commonlib.h>
#include <signal.h>

#include "gtthread.h"
#include "videoencoder.h"
#include "watch_process.h"
#include "device_state.h"
#include <devinfo.h>
#include <devres.h>
#include "process_modcmd.h"
#include "onviflib.h"
#include "onvif_system.h"
//#include "onvifcb.h"


//在日志上记录退出过程
static void exit_log(int signo)
{
	switch(signo)
	{
		case SIGPIPE:
			printf("process_sig_pipe \n");	
			return ;
		break;
		case SIGTERM:
		case SIGKILL:
			gtloginfo("onvifbox 被kill,程序退出!!\n");
			close_all_res();
			exit(0);
		break;
		case SIGINT:
			gtloginfo("onvifbox 被用户终止(ctrl-c)\n");
			close_all_res();
			exit(0);
		break;
		case SIGUSR1:
			//输出系统信息到指定文件
			//dump_sysinfo();
		break;
		case SIGSEGV:
			gtloginfo("onvifbox 发生段错误\n");
			printf("onvifbox segmentation fault\n");
			close_all_res();
			exit(0);
		break;
	}
	return;
}

void regist_signals(void)
{
	signal(SIGTERM,exit_log);	//kill信号
	signal(SIGKILL,exit_log);	//kill -9 信号
	signal(SIGSEGV,exit_log);	//段错误信号
	signal(SIGPIPE, exit_log);	//向已经关闭的连接中写数据
	signal(SIGUSR1,SIG_IGN);	
}



int main(int argc,char *argv[])
{
	int lock_fd;
    
	close_all_res();
	setsid();
	gtopenlog("onvifbox");							//打开日志记录
	lock_fd=create_lockfile_save_version(ENC_LOCK_FILE,VERSION);	//打开锁文件
	if(lock_fd<0)
	{
		printf("encbox module are running!!\n");
		exit(1);
	}



	printf("启动onvifbox(ver:%s).......\n",VERSION);
	gtloginfo("启动onvifbox(ver:%s).......\n",VERSION);
	



       
	regist_signals();						 //注册信号处理函数
	if(GT_SUCCESS!=init_devinfo())	                                    //初始化设备信息
	{
		gtloginfo("init_devinfo err!\n");
		exit(1);
	}
	init_com_channel();                               //初始化通讯命令通道
	if(GT_SUCCESS!=creat_modcmdproc_thread())          //创建通讯线程*/
	{   
		gtloginfo("create_modecmdporc_thread err!\n");
		exit(1);
	}


	if(GT_SUCCESS!=onvif_lib_init())						//初始化onviflib
	{

		gtloginfo("onvif_lib_init err!\n");
		exit(1);
	
	}
	if(GT_SUCCESS!=init_media_system())					//初始化媒体信息
	{

		gtloginfo("init_media_system err!\n");
		exit(1);

	}
	
	if(GT_SUCCESS!=init_onvif_system())		//根据配置文件初始化相关内容
	{

		gtloginfo("init_onvif_system failed!\n");
		exit(1);
	
	}
	create_onvif_device_thread();
	create_ip_device_thread();


	second_proc();
	exit(0);
}

