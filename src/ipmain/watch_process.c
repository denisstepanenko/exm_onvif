#include "ipmain.h"
#include "ipmain_para.h"
#include "maincmdproc.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mainnetproc.h"
#include "watch_board.h"
#include "gate_connect.h"
#include <sys/types.h>
#include <signal.h>
#include "leds_api.h"
#include "netcmdproc.h"
#include "maincmdproc.h"
#include "gate_connect.h"
#include "devstat.h"
#include <commonlib.h>
#include "mod_socket.h"
#include "ipmain_para.h"
#include "video_para_api.h"
#include "trans_com.h"

#ifdef HQMODULE_USE
//#include "hdmodule.h"
#endif
#define LEDS_TIME   3 //该秒数后发送一次信号灯状态给驱动

static int ledstimer=0; //发送信号灯状态计数器

static int heart_state=0;
static int net_er_cnt=0;


pthread_mutex_t g_audiodown_channel_mutex;


void restart_adsl_connect(void)
{
	pid_t pid;
	int	ret;
	pid=fork();
	if(pid==0)
	{//子进程
		setsid();
		close_all_res();
		system("/ip1004/restart_adsl &");
		exit(0);
	}
	else if((int)pid>0)
	{
		ret=waitpid(pid,NULL,WUNTRACED);
	}
}

/************************************************************************************
 *函数名:retset_dev()
 *功能  :当检测到本设备不能连接网关时则不再向外发送心跳
 *输入  : seconds 初始秒数
 *输出  : 无
 *返回  : 无
 *备注  : 2013-5-30 lc do
 * **********************************************************************************/
void reset_dev(short seconds)
{
  int ledstate=0;
  int alarmchn = -1;
  int audiochn = -1;
  //获取状态
  ledstate=get_current_netled();
  //printf("ledstate=%d\n",ledstate);
  //当前只有在连接网关成功，adsl拨号成功，注册网关成功3种情况下会继续发送心跳，其他情况不再发送心跳
  struct ipmain_para_struct* p = get_mainpara();
#if 0
  if(p->internet_mode == 0)
  {
    //在通过猫播ad的条件下，采用此判断方法
    //lc do 目前ledstate，网络只有0/1两种状态，表示是否注册成功
  	//if((ledstate==NET_GATE_CONNECTED)||(ledstate==NET_ADSL_OK)||(ledstate==NET_REGISTERED))
  	if(ledstate)
    {
    	if(heart_state==0)
    	{
      	heart_state=1;
		//lc to do
		//set_relay_output(3,0);
    	}
    	else
    	{
      	heart_state=0;
		//lc to do
	  	//set_relay_output(3,1);
  		}
    	//输出心跳
    	//set_relay_output(heart_state);
	  	//heart_state = !heart_state;
    }
 	else
    {
    }
  }
  else
  {
    //在局域网或专线条件下,只有网关注册成功，否则都要停止心跳
    if(ledstate)
    {
	    if(heart_state==0)
    	{
      		heart_state=1;
			//lc to do
      		//set_relay_output(3,0);
    	}
    	else
    	{
      		heart_state=0;
	  		//set_relay_output(3,1);
  		}
    		//输出心跳
    	//set_relay_output(heart_state);
	  	//heart_state = !heart_state;
    }
	else
	{
	}
  }
#endif
#if 1
	if(p->multi_channel_enable == 0)
	{
		if(seconds <= 30)
			alarmchn = -1;
		else
			alarmchn = p->alarm_playback_ch;
//#ifdef TEST_NO_HEART	
		if(ledstate)
    	{
			heart_state = !heart_state;
			keepalive_send_com_internal(alarmchn ,p->vadc.quad.current_net_ch,1);
    	}
 		else
    	{
    		//set_heart_beat(0);
    		keepalive_send_com_internal(alarmchn ,p->vadc.quad.current_net_ch,0);
    	}
	}
	else
	{
		//lc 2014-1-2 对于下行通道的处理，完全听从rtimage信令变化
		pthread_mutex_lock(&g_audiodown_channel_mutex);
		audiochn = p->current_audio_down_channel;
		printf("p->current_audio_down_channel is %d\n",audiochn);
		pthread_mutex_unlock(&g_audiodown_channel_mutex);
		if(ledstate)
    	{
			heart_state = !heart_state;
			keepalive_send_com(audiochn,1);
    	}
 		else
    	{
    		//set_heart_beat(0);
    		keepalive_send_com(audiochn,0);
    	}
	}
#endif

//#endif	
}

void reset_dev_dog(int serialmode)
{
  int ledstate=0;
    //获取状态
  ledstate=get_current_netled();
  //printf("ledstate=%d\n",ledstate);
  //当前只有在连接网关成功，adsl拨号成功，注册网关成功3种情况下会继续发送心跳，其他情况不再发送心跳
  struct ipmain_para_struct* p = get_mainpara();
#if 0 
  if(ledstate)
  {
#ifdef USE_WTDG
	  feed_watch_dog();
#endif
  }
  else
  {
	  if(!serialmode)
		feed_watch_dog();
  }
#endif 
  feed_watch_dog();
}

int choose_audio(int alarmchannel,int audiochannel)
{
	int final_chn=-1;
  if(alarmchannel < 0)
  	{
  	  //printf("audiochannel is %d\n",audiochannel);
	  final_chn=audiochannel;
  	}
  else
  	{
  	  //printf("alarmchannel is %d\n",alarmchannel);
	  final_chn=alarmchannel;
  	}

  choose_enable(final_chn);

  return 0;
}


void choose_audio_enable(int seconds)
{
	int alarmchn = -1;
	int audiochn = -1;

	struct ipmain_para_struct* p = get_mainpara();
	
	if(p->multi_channel_enable == 0)
	{
		if(seconds <= 30)
			alarmchn = -1;
		else
			alarmchn = p->alarm_playback_ch;
			
		choose_audio(alarmchn,p->vadc.quad.current_net_ch);
	}
	else
	{
		//lc 2014-1-2 对于下行通道的处理，完全听从rtimage信令变化
		pthread_mutex_lock(&g_audiodown_channel_mutex);
		audiochn = p->current_audio_down_channel;
		//printf("p->current_audio_down_channel is %d\n",audiochn);
		pthread_mutex_unlock(&g_audiodown_channel_mutex);
			
		choose_audio(audiochn,-1);
	}

}

//重启猫
#if 0
void reset_modem_second_proc(void)
{
	struct ip1004_state_struct 	*gtstate;
	struct dev_state_struct 	*devstate;
	struct ipmain_para_struct 	*para;
	static  int reset_cnt=0;					//复位modem电源侧次数
	static int link_err_counter=0; 			//断线时间计数器	
	static int reset_modem_counter=0;		 //重启猫计数器	
	

	para=get_mainpara();
	if((para->reset_modem==0)||(para->internet_mode!=0)) //不重启猫或不是adsl上网
		return;
	gtstate=get_ip1004_state(0);
	devstate=&gtstate->reg_dev_state;
	
	if (devstate->link_err)//断线状态
	{
		
		if(++link_err_counter>para->reset_modem_time)
		{
			//set_relay_output(3,1);					
			gtloginfo("adsl断线%d秒,重启猫,端子3输出1\n",para->reset_modem_time);
			link_err_counter=0;
			reset_modem_counter++;
			reset_cnt++;
		}
		if(reset_modem_counter>0)//已经给猫断电了
		{
			if(++reset_modem_counter>8)
			{
				//set_relay_output(3,0);
				gtloginfo("给猫断电8秒后上电\n");
				if((reset_cnt%5)==4)
				{
					sleep(2);
					restart_adsl_connect();	//5 次重起一次服务线程
				}				
				reset_modem_counter=link_err_counter=0;
			}
		}

	}
	else
	{
		link_err_counter=reset_modem_counter=0;
		//set_relay_output(3,0);	
	}
}

#endif

#if 0
static int test_counter=0;
static int test_flag=0;
//test
void test_second_proc(void)
{
	if(++test_counter>2)
		{
			if(test_flag==0)
				{
					set_alarm_state_bit(1,3);
					gtloginfo("端口3输出1\n");
					test_flag=1;
				}
			else
				{
					set_alarm_state_bit(0,3);
					gtloginfo("端口3输出0\n");
					test_flag=0;
				}
			test_counter=0;
		}
	

}
#endif

//示意灯
void leds_second_proc(void)
{
	//int i;
	//DWORD errbuffer=0;
		if(ledstimer++>=LEDS_TIME)//每LEDS_TIME秒更新一次
		{
			ledstimer=0;
		
			//ERR灯
			set_error_led_state();

		}


}

#if 0
void watch_process_second_proc(void) //
{
	int i,status,fd,pid;
	struct module_struct *mod;
	if(watchprocesstimer++>=20)
	{
		watchprocesstimer=0;
		//扫描 一旦换了班子就取消注释
		for(i=0;i<SYS_MAX_MODULE;i++)
		{
			mod=&sysmods[i];
			//printf("mod->pid is %d\n",mod->pid);
			fd=open(mod->lockfile,O_RDWR|O_CREAT,0640);
			if(fd<0)
				continue;
			if(flock(fd,LOCK_EX|LOCK_NB)==0) //进程已被中止
			{
				printf("process %s stopped!..\n",mod->path);
				pid=mod->pid;
				mod->pid=-1;
				flock(fd,LOCK_UN);
				close(fd);
				if(pid>0)
					waitpid(pid,&status,0);//收集以前的子进程信息

				start_process(mod);
				
				
			}
			else
			{
				close(fd);
#ifdef SHOW_WORK_INFO
				//printf("process ok!\n");
#endif
			}
			

		}
		
	}
	
	return ;
}
#endif
///发送查询所有模块状态的命令
static int send_query_stat_cmd(void)
{
	//lc do 目前只发送给rtimg videoenc pppoe upnp
    send_query_state_cmd(RTIMAGE_PROCESS_ID);
    send_query_state_cmd(HQSAVE_PROCESS_ID);
    send_query_state_cmd(DISKMAN_ID);
    send_query_state_cmd(VIDEOENC_MOD_ID);                
	send_query_state_cmd(UPNPD_MOD_ID);
	send_query_state_cmd(PPPOE_WATCH_ID);
    //sleep(2);
    //send_query_state_cmd(HW_DIAG_MOD_ID);       
    return 0;
}

static short first_seconds_count = 0;

static void *internal_com_read_thread(void *transcom)
{
	int max_fd=-1;//com1_fd=-1; 
	struct trans_com_struct *com;
	BYTE  *com_rec_buf;
 	int ret;
	fd_set readfds;
	char	tmpbuf[256];
	GT_SUB_CMD_STRUCT* cmd;
	struct ip1004_state_struct *ip1004state;
	BYTE crc_in = 0;
	
	if(transcom==NULL)
		return NULL;
	else
		com=(struct trans_com_struct *)transcom;

#ifdef DISPLAY_THREAD_INFO
	printf("internal_com_read_thread %d running...\n",com->ch);
#endif
	gtloginfo("start internal_com_read_thread %d...\n",com->ch);
	
	com_rec_buf=(BYTE*)com->com_rec_buf;
	
	FD_ZERO(&readfds);
	if(com->local_fd>0)
	{
		FD_SET(com->local_fd,&readfds);	
		if(max_fd<com->local_fd)
			max_fd=com->local_fd;	
	}
  	while(1) 		
  	{
  		select(max_fd+1,&readfds,NULL,NULL,NULL);
		if(FD_ISSET(com->local_fd,&readfds))
		{
			ret=read(com->local_fd,tmpbuf,sizeof(GT_SUB_CMD_STRUCT));
			if(ret == sizeof(GT_SUB_CMD_STRUCT))
			{
				//lc do 解析信息
				cmd = (GT_SUB_CMD_STRUCT*)tmpbuf;
				crc_in = cmd->crc ; //获取命令包校验值
				cmd->crc = 0 ;

				unsigned char crc_reg = 0;
				int i = 0 ;
				for (i = 0; i < sizeof(GT_SUB_CMD_STRUCT); i++)
				{
					crc_reg ^=*(tmpbuf+i);
				}

				if(crc_in==crc_reg)//判断命令包校验是否正确
				{
					if(cmd->cmd == 0x24)
					{
						//lc do process the cmd
						ip1004state = get_ip1004_state(0);
						pthread_mutex_lock(&ip1004state->mutex);
						printf("cmd in bitstr is %02x\n",cmd->bitstr);
						//memcpy((((BYTE*)&ip1004state->reg_per_state)+2),(BYTE*)&cmd->bitstr,1);
						pthread_mutex_unlock(&ip1004state->mutex);
					}
				}
				else 
				{
					gtlogerr("接收内部串口信息crc校验不正确，抛弃!\n");
					continue;
				}
			}
			else if(ret < 0)
			{
				gtlogerr("读取内部串口数据出错，关闭内部串口!\n");
				pthread_mutex_lock(&com->mutex);
				if(com->local_fd>0)
				{
					if(&readfds!=NULL)
					FD_CLR(com->local_fd,&readfds);
					usleep(500000);//
		
					close(com->local_fd);
					com->local_fd=-1;
					com->flag=0;
				}
				pthread_mutex_unlock(&com->mutex);
				break;
			}
			else
			{
				continue;
			}
		}
		else
		{
			printf("no fd is set\n");
		}
	}

	return NULL;	

}


void create_com_internal_read_thread()
{
	pthread_attr_t  thread_attr;
	pthread_attr_t  *attr = &thread_attr;

	struct trans_com_struct *com=get_trans_com_info(2);
	memset((void*)attr,0,sizeof(pthread_attr_t));

	pthread_attr_init(attr);
	pthread_attr_setdetachstate(attr,PTHREAD_CREATE_DETACHED);//分离状态
	pthread_attr_setschedpolicy(attr,SCHED_OTHER);
	
	pthread_create(&com->thread_id,attr, internal_com_read_thread, (void*)com);

	pthread_attr_destroy(attr);

	return;
}

int get_cmdline_heartbeat()
{
	FILE *fp;
	int heart = 1;
	char* pheartbeat = NULL;
    if ((fp = popen("cat /proc/cmdline", "r")) == NULL) {
        perror("popen failed");
        return -1;
    }
    char buf[256];  //获取第一行参数
    if( fgets(buf,255,fp) != NULL )
    {
		//find heartbeat
		pheartbeat = strstr(buf,"heartbeat");
		if(pheartbeat == NULL)
			return 1;
		pheartbeat += 10;
		heart = atoi(pheartbeat);
		printf("heart is %d\n",heart);
    }
		
    if (pclose(fp) == -1) {
        perror("pclose failed");
        return -1;
    }

	return heart;
}

static int encbox_timer = 0;
extern int encbox_coaxial_err_count;
extern pthread_mutex_t encbox_mutex;

void encbox_second_proc(void)
{
	if(++encbox_timer > 3600)//每小时检查一次
	{
		encbox_timer = 0;
		//2013-12-27 lc 在1小时内，如果接到encbox发来VIDEOENC_COAXIAL_ERR超过350,重启设备(防止encbox segment fault)
		pthread_mutex_lock(&encbox_mutex);
		if(encbox_coaxial_err_count >= 350)
		{
			gtlogerr("encbox 连续出现VIDEOENC_COAXIAL_ERR，重启encbox");
			if(access("/tmp/hwrbt",R_OK|W_OK) == 0)
				system("/tmp/hwrbt 5");
			else
				system("/ip1004/hwrbt 5");
		}
		encbox_coaxial_err_count = 0;
		pthread_mutex_unlock(&encbox_mutex);
	}
}


void *watch_process_thread(void)
{
	//static int regist_cnt=0;
	static int net_refresh_cnt=0;
	struct ipmain_para_struct *ipmain_para;
	int serial_mode = 1;
	int serial_interval = 1;
	int kernel_heartbeat = 0;
	int relay_count=0;
	int relay_chn=0;
	//static in_addr_t old_wan=0;
#ifdef DISPLAY_THREAD_INFO
	printf("watch_process_thread thread running...\n");
#endif
	gtloginfo("start watch_process_thread...\n");

	sleep(10);

    send_query_stat_cmd();

	ipmain_para=get_mainpara();
 	
	serial_mode = get_serial_mode();
	serial_interval = get_serial_interval()*60;
	//lc do 根据内核启动参数，判断是否发心跳
	//kernel_heartbeat = get_cmdline_heartbeat();
	
	relay_count=ipmain_para->gatedown_relay_count;
	relay_chn=ipmain_para->gatedown_relay_chn;

#ifdef USE_WTDG
	if(serial_mode)
	{
		if(init_wtdg_dev(serial_interval) < 0)
		{
			printf("init_wtdg_dev failed!\n");
		}
	}
	else
	{
		if(init_wtdg_dev(-1) < 0)
		{
			printf("init_wtdg_dev failed!\n");
		}
	}
#endif

	while(1)
	{		
		netcmd_second_proc();
		//lc do AD模块和切换通道等操作,移至videoenc
		//vadc_second_proc();

		gate_connect_second_proc(relay_count,relay_chn);
		//lc do 对端子的处理
		//watch_board_second_proc();
		//lc do led灯处理
#ifdef USE_LED
		leds_second_proc();
#endif		
		encbox_second_proc();
		
//#ifdef   GPS_SUPPORT
//		GPS_second_proc();
//#endif		
//#ifdef HQMODULE_USE
//		hd_second_proc();
//#endif		
		if(++net_refresh_cnt>10)
		{
			net_refresh_cnt=0;
			refresh_netinfo();
#if 0			
			ipmain_para=get_mainpara();
			if(old_wan!=ipmain_para->wan_addr)
			{
				old_wan=ipmain_para->wan_addr;
				if(old_wan!=0)
				{
					set_regist_flag(0);
					system_regist(-1,1);//ip地址变化后发送注册信息
				}
			}
#endif
			
		}
		if(first_seconds_count >= 31)
			first_seconds_count = 60;
#ifdef ARCH_3520A			
		//lc do 需要根据led灯状态，判断是否需要发送心跳
		if(kernel_heartbeat)
		{
			//lc do 2013-5-30
			if(keepalive_open_com_internal()<0)
  			{
    			printf("keepalive_open_com_internal error \n");  
				gtlogerr("打开心跳用串口失败! \n");
				continue;
 			}
	
			//使能心跳功能
			keepalive_set_com_mode(serial_mode,serial_interval);
    		//侦听串口信息
			reset_dev(first_seconds_count++);
		}
#endif		

#ifdef ARCH_3520D
		//lc do 需要根据led灯状态，判断是否需要发送心跳
		reset_dev_dog(serial_mode);
		choose_audio_enable(first_seconds_count++);
#endif
		sleep(1);
	}
	
	return NULL;
}

