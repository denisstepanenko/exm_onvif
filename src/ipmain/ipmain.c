#include "ipmain.h"
#include "commonlib.h"
#include "ipmain_para.h"
#include "maincmdproc.h"
#include "mainnetproc.h"
#include "watch_board.h"
#include "watch_process.h"
#include "trans_com.h"
#include <gt_com_api.h>
#include <devinfo.h>
#include <signal.h>
#include <dirent.h>
//#include <hdctl.h>
//#include <ftw.h>
#include "netcmdproc.h"
#include "leds_api.h"
#ifdef ARCH_3520D
#include "audioout_api.h"
#endif
#include "infodump.h"
#include "devstat.h"
//#include <gate_cmd.h>
//#include "gtvs_io_api.h"
#include "gate_connect.h"
//#include "osd_api.h"
#include "video_para_api.h"
//#include "gpsupport.h"
#ifdef TEST_FROM_TERM
	#include "testfromterm.c"
#endif
#include <ctype.h>  
#ifdef ARCH_3520A
#include "exdrv_3520Ademo/hi_wtdg/watchdog.h"
#else 
#include "hi3520D/watchdog.h"
#endif

//#include "rand48.h"
/****************在ulibc环境下********************************/
int posix_memalign(void **memptr, size_t alignment, size_t size)
{
	if (alignment % sizeof(void *) != 0)
		//|| !powerof2(alignment / sizeof(void *)) != 0
		//	|| alignment == 0
	return -EINVAL;

	*memptr = memalign(alignment, size);
	return (*memptr != NULL ? 0 : ENOMEM);
}

/*
__const unsigned short int *__ctype_b;
__const __int32_t *__ctype_tolower;
__const __int32_t *__ctype_toupper;
//为免多出一个文件，出此下策

#define ctSetup()   {  \
                         __ctype_b = *(__ctype_b_loc());   \
                         __ctype_toupper = *(__ctype_toupper_loc());   \
                         __ctype_tolower = *(__ctype_tolower_loc());  \
}
*/
/*
extern unsigned short _rand48_seed[3];
extern unsigned short _rand48_mult[3];
extern unsigned short _rand48_add;

void
lcong48(unsigned short p[7])
{
	_rand48_seed[0] = p[0];
	_rand48_seed[1] = p[1];
	_rand48_seed[2] = p[2];
	_rand48_mult[0] = p[3];
	_rand48_mult[1] = p[4];
	_rand48_mult[2] = p[5];
	_rand48_add = p[6];
}
*/
/*************************3520D 看门狗控制程序 ********************************/
int g_wtdg_fd = 0;
//hi3515  watchdog初始化，设置timeout时间
int init_wtdg_dev(int timeout)
{
	int timeOut;
	if(timeout > 0)
		timeOut = timeout/2;
	else
		timeOut = WTDG_TIMEOUT; 

	g_wtdg_fd = open("/dev/watchdog", O_RDWR);
	if (g_wtdg_fd < 0)
	{
#ifdef SHOW_WORK_INFO
		printf("open /dev/watchdog failed!");
#endif
		gtlogerr("open /dev/watchdog failed!");
		return -1;
	}
	if (ioctl(g_wtdg_fd, WDIOC_SETTIMEOUT, &timeOut) < 0)
	{
#ifdef SHOW_WORK_INFO
		printf("ioctl /dev/watchdog failed!");
#endif
		gtlogerr("ioctl /dev/watchdog failed!");
		return -1;
	}

#ifdef SHOW_WORK_INFO
	printf("init_wtdg_dev OK!\n");
#endif
	gtloginfo("初始化看门狗成功!超时时间为%d秒\n",timeOut);

	return 0;		
}

int feed_watch_dog()
{
	if (g_wtdg_fd >= 0)
	{
		if (ioctl(g_wtdg_fd, WDIOC_KEEPALIVE,0) < 0)
		{
#ifdef SHOW_WORK_INFO
			printf("feed dog failed!\n");
#endif
			gtlogerr("feed dog failed!\n");
			return -1;
		}
	}

#ifdef SHOW_WORK_INFO
	//printf("feed_watch_dog OK!\n");
#endif

	return 0;
}

/************************************************************************************/

/***************在日志上记录程序退出状态*********************/
static void exit_log(int signo)
{
	switch(signo)
	{
		case SIGPIPE:
			printf("ipmain process_sig_pipe \n");	
			return ;
		break;
		case SIGTERM:
			gtloginfo("ipmain 被kill,程序退出!!\n");
			close_all_res();
			exit(0);
		break;
		case SIGKILL:
			gtloginfo("ipmain SIGKILL,程序退出!!\n");
			//lc do 反初始化mpp相关通道
			uninit_video_vadc();
			close_all_res();
			exit(0);
		break;
		case SIGINT:
			gtloginfo("ipmain 被用户终止(ctrl-c)\n");
			close_all_res();
			exit(0);
		break;
		case SIGUSR1:
			//输出系统信息到指定文件
			//lc do 
			dump_sysinfo();
		break;
		case SIGSEGV:
			gtloginfo("ipmain 发生段错误\n");
			close_all_res();
			printf("ipmain segmentation fault\n");
			exit(0);
		break;
	}
	return;
}


int set_io_delay(int valid_delay,int invalid_delay)
{
	int i;
	int ret = 0;

	ret = set_trigin_delay(valid_delay,invalid_delay);
	
	return ret;
}



/**********************************************************************************************
 * 函数名	:get_gtthread_attr()
 * 功能	:获取一个默认线程属性结构
 * 输出	:attr:返回时填充好属性结构
 * 返回值	:0表示成功负值表示出错
 * 注		:用完后应该调用pthread_attr_destroy进行释放
 **********************************************************************************************/
int get_gtthread_attr(pthread_attr_t *attr)
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
 * 函数名	:init_check_cert()
 * 功能	:初始化并检查设备的证书是否正常，并进行备份及恢复
 * 返回值	:无
 **********************************************************************************************/
static void init_check_cert(void)
{
	int	ret,ret1;
	char pbuf[200];
	struct ipmain_para_struct *ipmain_para=get_mainpara();
	if(ipmain_para->rmt_env_mode==MSG_AUTH_SSL)
	{
		//lc do 初始化证书验证
#ifdef USE_SSL		
		ret=env_init(CERT_FILE,KEY_FILE);
		printf(" 初始化证书验证!\n");  
		//ret = 0;
		if(ret!=0)
		{
			printf("env_init file %s and %s ret=%d\n",CERT_FILE,KEY_FILE,ret);
			gtlogfault("打开数字证书出错ret=%d\n",ret);	
			ipmain_para->valid_cert=0;

			sprintf(pbuf,"cmp %s %s\n",CERT_FILE,CERT_BAK_FILE);
			ret=system(pbuf);
			sprintf(pbuf,"cmp %s %s\n",KEY_FILE,KEY_BAK_FILE);
			ret1=system(pbuf);
			if((ret!=0)||(ret1!=0))
			{//备份文件和源文件不一样
				ret=env_init(CERT_BAK_FILE,KEY_BAK_FILE);
				printf("bak初始化证书验证!\n");  ret = 0;
				if(ret==0)
				{
					gtloginfo("恢复证书文件%s到%s\n",CERT_BAK_FILE,CERT_FILE);
					gtloginfo("恢复私钥文件%s到%s\n",KEY_BAK_FILE,KEY_FILE);
					sprintf(pbuf,"cp %s %s\n",CERT_BAK_FILE,CERT_FILE);
					system(pbuf);	
		
					sprintf(pbuf,"cp %s %s\n",KEY_BAK_FILE,KEY_FILE);
					system(pbuf);	
					
					ipmain_para->valid_cert=1;
				}
				else
				{
					gtloginfo("没有可用的备份证书文件\n");
				}
			}

			if(ipmain_para->valid_cert==0)
			{
				ipmain_para->rmt_env_mode=GT_CMD_NO_AUTH;
				ipmain_para->rmt_enc_mode=GT_CMD_NO_ENCRYPT;
			}
		}
		else
		{
#ifdef SHOW_WORK_INFO
			printf("env_init file %s and %s success!\n",CERT_FILE,KEY_FILE);
#endif
			ipmain_para->valid_cert=1;

			sprintf(pbuf,"cmp %s %s\n",CERT_FILE,CERT_BAK_FILE);
			ret=system(pbuf);
			sprintf(pbuf,"cmp %s %s\n",KEY_FILE,KEY_BAK_FILE);
			ret1=system(pbuf);
			if((ret!=0)||(ret1!=0))
			{
				sprintf(pbuf,"cp %s %s\n",CERT_FILE,CERT_BAK_FILE);
				system(pbuf);
				sprintf(pbuf,"cp %s %s\n",KEY_FILE,KEY_BAK_FILE);
				system(pbuf);				
				gtloginfo("更新证书备份文件%s %s\n",CERT_BAK_FILE,KEY_BAK_FILE);
				
			}


		}
#endif		
	}

}

#ifdef FOR_PC_MUTI_TEST
int test_alarm_interval=-1;		//测试报警的时间间隔
int test_alarm_num=-1;			//测试报警用的报警次数
int test_alarm_need_inc=0;		//报警测试是是否需要每次报警信息变化
#include "pc_multi_test.c"
int process_opt_h(void)
{
	printf("ip1004 vsmain模块模拟程序\n");
	printf("用法:vsmain [OPTION] [argument]\n");
	printf("OPTION 选项说明\n");
	printf("-h:显示帮助信息\n");
	printf("-i:表示连通网关后的报警间隔时间(秒) 默认为不报\n");
	printf("-n:表示达到间隔时间后的报警次数(必须小于100)\n");
	printf("-v:表示每次报警的信息需要变化\n");
	return 0;
}
int process_opt_i(char *argument)
{
	if(argument==NULL)
	{
		printf("process_opt_i argument=NULL!!!\n");
		return -1;
	}
	test_alarm_interval=atoi(argument);
	printf("自动报警间隔:%d(秒)\n",test_alarm_interval);
	return 0;
}
int process_opt_n(char *argument)
{
	if(argument==NULL)
	{
		printf("process_opt_n argument=NULL!!!\n");
		return -1;
	}
	test_alarm_num=atoi(argument);
	if(test_alarm_num>100)
	{
		printf("报警次数太大,强制设成最大值!\n");
	}
	printf("自动报警数量:%d (个/次)\n",test_alarm_num);
	return 0;
}
int process_opt_v(void)
{

	test_alarm_need_inc=1;
	printf("每个报警的报警信息自动变化\n");
	return 0;
}
int process_argument(int argc,char **argv)
{
	int oc;
	if(argc <= 1)
		return 0;
        while((oc=getopt(argc,argv,"hvi:n:"))>=0)
        {
                switch(oc)
                {
			case 'h':
				process_opt_h();
				exit(0);
			break;
			case 'i':
				process_opt_i(optarg);
			break;
			case 'n':
				process_opt_n(optarg);
			break;
			case 'v':
				process_opt_v();
			break;
			default:
			break;
                }
        }
	return 0;
}
#endif
#if 0
//以当前系统时间做随机数种子
long int set_lrandomseed(void)
{
	unsigned short int param[7];
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);

	param[0]	=	p->tm_year;
	param[1]	=	p->tm_mon;
	param[2]	=	p->tm_mday;
	param[3]	=	p->tm_wday;
	param[4]	=	p->tm_hour;
	param[5]	=	p->tm_min;
	param[6]	=	p->tm_sec;

	//lc do
	lcong48(param);
	return 0;
}
#endif

int main(int argc,char **argv)
{
	//int ret,ret1;
	pthread_attr_t  thread_attr,*attr;				//线程属性结构
#ifdef TEST_FROM_TERM
	pthread_t testfromterm_thread_id;
#endif
	struct ipmain_para_struct *ipmain_para;		//参数指针
	int lock_file;									//锁文件的描述符
	char pbuf[100];								//临时缓冲区
	char devid[32];								//设备guid临时缓冲区
	unsigned short serv_port;						//命令服务端口
	close_all_res();								//关闭所有已经打开的资源
#ifdef FOR_PC_MUTI_TEST
	process_argument(argc,argv);
	gen_multi_devices();
#endif

	gtopenlog("ipmain");							//打开日志
	setsid();										//设置进程为领头进程
	show_time(NULL);								//显示系统当前时间 秒.毫秒
#ifdef DEAMON_MODE
	if(deamon_init()!=0)
	{
		printf("deamon_init error!\n");
	}
#endif
	/***************判断模块是否已经运行***************************/
	lock_file=create_and_lockfile(IPMAIN_LOCK_FILE);
	if(lock_file<=0)
	{
		printf("ipmain module are running!!\n");
		gtlogerr("ipmain模块已运行，故启动无效退出\n");		
		exit(0);
	}	
	sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
	write(lock_file,pbuf,strlen(pbuf)+1);				//将进程的id号存入锁文件中

#ifdef FOR_PC_MUTI_TEST
	SetOutputTty(lock_file);						// 将输出影射到另一个描述符
#endif

	//ctSetup();
	gtloginfo("启动ipmain (ver:%s).......\n",VERSION);

	/***************注册信号处理函数**********************************/
	signal(SIGKILL,exit_log);		//kill -9信号
	signal(SIGTERM,exit_log);		//普通的kill信号
	signal(SIGINT,exit_log);		//ctrl-c信号
	signal(SIGSEGV,exit_log);		//段错误信号
	signal(SIGPIPE,exit_log);		//向已关闭的socket中写数据信号


	init_devinfo();					//用配置文件初始化设备信息(guid等)
	init_devstat();				//初始化设备状态
	
	get_devid(devid);
	printf("devid=%02x%02x%02x%02x%02x%02x%02x%02x\n",devid[7],devid[6],devid[5],devid[4],devid[3],devid[2],devid[1],devid[0]);
	gtloginfo("devid=%02x%02x%02x%02x%02x%02x%02x%02x\n",devid[7],devid[6],devid[5],devid[4],devid[3],devid[2],devid[1],devid[0]);
	
	init_para();					//将参数表设置为初始值
	readmain_para_file(IPMAIN_PARA_FILE,get_mainpara());	//从配置文件读取参数表
	read_config_ini_file(CONFIG_FILE, get_mainpara());			//从配置文件中读取部分设备参数值
	printf("本设备上网方式为: %s\n",get_internet_mode_str());
	gtloginfo("本设备上网方式为: %s\n",get_internet_mode_str()); 
	ipmain_para=get_mainpara();
	ipmain_para->total_com=get_total_com();
	if(ipmain_para->total_com>get_total_com())
		ipmain_para->total_com=1;
	printf("[ipmain(ver:%s)] module run pid=%d waiting for remote gateway at port:%d\n",VERSION,getpid(),serv_port);

   // set_lrandomseed();//设随机数种子 
	init_check_cert();					//初始化并检查证书
#ifdef USE_VDA
	init_video_vadc(&ipmain_para->vadc);
#endif
#ifdef ARCH_3520D
	if(open_rtc_dev() < 0)
	{
		printf("init_rtc_dev failed!\n");
	}
	if(init_audioout() < 0)
	{
		printf("init_audioout failed!\n");
	}
#endif

	//lc do 初始化gpio,设置valid/invalid时间，默认为0/3秒
#ifdef USE_IO
	init_gpiodrv();	//初始化gpio管脚驱动们
	//lc do 
	set_io_delay(GTIP_IO_VALID_DELAY,GTIP_IO_INVALID_DELAY); 
#endif	
	//lc do 初始化led
#ifdef USE_LED
	init_leds();								//初始化指示灯状态
#endif	
	
	init_netcmd_var();						//初始化和网关通讯的状态
	//lc do 串口服务初始化
	init_trans_com_var();					//初始化透明串口需要用到的数据结构
		
	init_com_channel();						//初始化主进程与其他进程通讯的命令通道
#ifdef DISPLAY_THREAD_INFO
	printf("prepare to start threads...\n");			
#endif
	if(get_gtthread_attr(&thread_attr)==0)
		attr=&thread_attr;
	else
		attr=NULL;
		
	creat_connect_thread(attr);				//创建网关连接线程

	//lc do 对视频丢失、遮挡、移动进行检测,此部分工作移至videoenc中
	creat_vda_proc_thread(attr,NULL);

#if EMBEDED==1
	//lc do 监控端子状态变化并进行相应动作
	init_ipalarm_fd();
	init_ipalarm_fdset();
	creat_watch_board_thread(attr);		//创建监视输入端子的线程
#endif
	creat_cmdproc_thread(attr);				//创建接受主进程命令的线程
	
	init_mainnetcmd_threads(attr,ipmain_para->devpara[0].cmd_port,0);

	creat_trans_com_thread(attr,get_trans_com_info(0));		//创建透明串口服务线程
	if(get_total_com()>1)
		creat_trans_com_thread(attr,get_trans_com_info(1));	//创建透明串口服务线程		
#ifdef TEST_FROM_TERM
	pthread_create(&testfromterm_thread_id,attr,test_from_term_thread,NULL);//创建透明串口处理线程
#endif

	
	if(attr!=NULL)
	{
		pthread_attr_destroy(attr);
	}

	sleep(1);
	show_time(NULL);							//显示系统当前时间 秒.毫秒
	
	signal(SIGUSR1,exit_log);
	watch_process_thread();					//转化为监控线程
	//应该永远不会执行到这里
	closelog();
	exit(-1);
}

