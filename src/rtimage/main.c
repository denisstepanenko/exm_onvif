/** @file	main.c
 *   @brief 	rtimage2模块的主函数定义
 *   @date 	2007.03
 */
#include "rtimage2.h"
#include <signal.h>


#include <commonlib.h>

#include "maincmdproc.h"
#include "net_avstream.h"
#include "avserver.h"
#include "net_aplay.h"
#include  "debug.h"
//zsk add
//#include <mcheck.h>
#include <stdlib.h>
#ifdef FOR_PC_MUTI_TEST
#include "pc_multi_test.c"
#endif
/** 
 *   @brief     在日志上记录退出过程
 *   @param  signo 信号量编号
 */
static void exit_log(int signo)
{
	switch(signo)
	{
		case SIGPIPE:       ///<向已关闭的socket中写数据信号
			printf("process_sig_pipe \n");	
			return ;
			break;
		case SIGTERM:      ///<普通的kill信号
		case SIGKILL:       ///<kill -9信号
			gtloginfo("tcprtimg 被kill,程序退出!!\n");
			close_all_res();
			exit(0);
			break;
		case SIGINT:         ///<ctrl-c信号
			gtloginfo("tcprtimg 被用户终止(ctrl-c)\n");
			close_all_res();
			exit(0);
			break;
		case SIGSEGV:       ///<段错误信号
			gtloginfo("tcprtimg 发生段错误\n");
			printf("tcprtimg segmentation fault\n");
			close_all_res();
			exit(0);
			break;
	}
	return;
}

/** 
 *   @brief     tcprtimage2模块的秒处理线程,这里调用的函数不应有阻塞的
 *   @param  无
 */
static void second_proc_thread(void)
{
	printf(" start second_proc_thread...\n"); 	
	gtloginfo(" start second_proc_thread...\n"); 	
	while(1)
	{
		sleep(1);
		avserver_second_proc();
	}
	return ;
}

int process_opt_h(void)
{
	printf("实时音视频服务程序tcprtimg version:%s\n",version);
	printf("用法:tcprtimg [OPTION] [argument]\n");
	printf("OPTION 选项说明\n");
	printf("-h:显示帮助信息\n");
	printf("-v:显示版本信息并退出程序\n");
	return 0;
}

/** 
 *   @brief     处理tcprtimage的输入参数
 *   @param  argc 参数数目
 *   @param  argv 参数值数组
 *   @return   0表示成功 ,负值表示除错
 */
int process_argument(int argc,char **argv)
{
	int oc;
	if(argc<2)
	{
		return 0;
	}
	printf("*************************************************\n");
	while((oc=getopt(argc,argv,"hv"))>=0)
	{
		switch(oc)
		{
			case 'h':
				process_opt_h();
				exit(0);
				break;
			case 'v':
				printf("tcprtimage version:%s\n",version);
				create_lockfile_save_version(RT_LOCK_FILE,(char*)version);
				printf("*************************************************\n");
				exit(0);
				break;
			default:
				break;
		}
	}

	printf("*************************************************\n\n\n");
	return 0;
}

/** 
 *   @brief  tcprtimage2模块的入口函数
 */
int main(int argc,char **argv)
{
	//setenv("MALLOC_TRACE", "/mnt/zsk/mtrace.log", 1);
	//mtrace();

	int ret;
	close_all_res();                            ///<关闭所有继承下来的已打开资源
	setsid();                                      ///<将进程设置为首进程
	gtopenlog("rtimage");                  ///<打开日志

	process_argument(argc,argv);
	///判断模块是否已经启动
	if(create_lockfile_save_version(RT_LOCK_FILE,(char*)version)<0)
	{
		printf("rtimage module are running!!\n");
		gtloginfo("rtimage module are running!!\n");
		exit(1);
	}

	///<显示启动信息
	printf     ("[rtimage(ver:%s)] process run pid=%d \n",version,getpid());
	gtloginfo("[rtimage(ver:%s)] process run pid=%d \n",version,getpid());

	///注册信号处理函数
	signal(SIGKILL,exit_log);		 ///<kill -9信号
	signal(SIGTERM,exit_log);		 ///<普通的kill信号
	signal(SIGINT,exit_log);		        ///<ctrl-c信号
	signal(SIGSEGV,exit_log);		 ///<段错误信号
	signal(SIGPIPE,exit_log);		 ///<向已关闭的socket中写数据信号
	signal(SIGUSR1,begin_debug);               //开始debug信息的信号
	signal(SIGUSR2,end_debug);						//结束debug信息的信号
	init_devinfo();                                ///<初始化设备信息

	ret=init_server_para();               ///<系统参数及状态初始化

	if(ret<0)
	{
		printf    ("不能分配参数内存,程序退出!\n");
		gtlogerr("不能分配参数内存,程序退出!\n");
		exit(1);
	}

	read_server_para_file();           ///<从配置文件中获取参数
	init_com_channel();			///<初始化与主进程通讯的命令通道


	creat_mod_cmdproc_thread(); ///<创建接收并处理主进程发来的命令的线程
	create_av_server();                  ///<创建音视频上行服务线程
	
    create_rtnet_av_servers();      ///<创建网络音视频服务socket及线程池
    create_rtnet_aplay_servers(); ///<创建音频下行服务socket及线程池



    second_proc_thread();              ///<转化为秒处理线程   

    exit(0);
		       
}


