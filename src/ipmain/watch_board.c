/*
        主进程与51模块通讯，交互
*/

#include "ipmain.h"

#include <errno.h>   
#include "watch_board.h"
#include "commonlib.h"
#include <netinet/in.h>	//ntohs
#include <gt_com_api.h>
#include "netcmdproc.h"
#include "watch_process.h"
#include "alarm_process.h"
#include "ipmain_para.h"
#include "devstat.h"
#include "infodump.h"
#include "gate_cmd.h"
//#include "gtvs_io_api.h"

#include "ip_device.h"

static pthread_t watchbd_thread_id=-1;
static int ipalarm_fd[4] = {-1};


//取当前时间和触发状态，填充dev_trig结构
int get_dev_trig(int dev_no,struct send_dev_trig_state_struct *dev_trig)
{
	time_t timep;
	struct tm *p;
	
	DWORD year;
	BYTE month,day,hour,min,sec;
	struct ip1004_state_struct *gtstate;
	if(dev_trig==NULL)
		return -1;
	gtstate=get_ip1004_state(dev_no);

	pthread_mutex_lock(&gtstate->mutex);
	memcpy(&dev_trig->alarmstate,(char *)&gtstate->reg_trig_state,4);
	pthread_mutex_unlock(&gtstate->mutex);

  	
	time(&timep);
	p=localtime(&timep);
	year=1900+p->tm_year;
	memcpy(&(dev_trig->year),&year,2);

	month=1+p->tm_mon;
	memcpy(&(dev_trig->month),&month,1);
	day=p->tm_mday;
	hour=p->tm_hour;
	min=p->tm_min;
	sec=p->tm_sec;
	memcpy(&(dev_trig->day),&day,1);
	memcpy(&(dev_trig->hour),&hour,1);
	memcpy(&(dev_trig->minute),&min,1);
	memcpy(&(dev_trig->second),&sec,1);
	return 0;
}


static DWORD oldalarmin = 0;
/**********************************************************************************************
 * 函数名	:watch_board_second_proc()
 * 功能	:定期喂狗的秒处理函数
 * 输入	:无
 * 返回值	:无
 **********************************************************************************************/
void watch_board_second_proc(void)
{
	time_t timenow;
	timenow=time((time_t *)NULL);	
	process_trigin_event(get_ip1004_state(0)->alarmin_state,oldalarmin,timenow);
	oldalarmin = get_ip1004_state(0)->alarmin_state;
}

static const int alarm_port[4] = {5655,5656,5657,5658};
static fd_set ipalarm_read_fds;
static int ipalarm_maxfds = 0;

int init_ipalarm_fdset()
{
	int i;
	FD_ZERO(&ipalarm_read_fds);
	for (i=0; i<get_video_num(); ++i)
	{
		if(ipalarm_fd[i]>0)
		  FD_SET(ipalarm_fd[i],&ipalarm_read_fds);

		if(ipalarm_maxfds <= ipalarm_fd[i])    
		  ipalarm_maxfds = ipalarm_fd[i];
	}

	return 0;
}

int  read_ipalarm_status(OUT DWORD *status)
{
	int ret=0, i;
	DWORD readstatus = 0;
	char *p=NULL;
	char frame_buf[4096];
    
	struct timeval TimeoutVal;
	struct sockaddr_in client;
	socklen_t addrlen=sizeof(client);
     
	TimeoutVal.tv_sec = 1;
	TimeoutVal.tv_usec = 0;

	do
	{
		
		init_ipalarm_fdset();
		printf("ipalarm maxfd is %d\n",ipalarm_maxfds);
		ret = select(ipalarm_maxfds+1, &ipalarm_read_fds, NULL, NULL, &TimeoutVal);
		if (ret < 0) {
			gtloginfo("ipalarm select <0 exit\n");
			break;
		}
		else if (0 == ret) {
			printf("get aenc stream select time out\n");
			break;
		}

		for (i=0; i<get_video_num(); ++i)
		{
			if (FD_ISSET(ipalarm_fd[i], &ipalarm_read_fds))
			{
				ret=recvfrom(ipalarm_fd[i],(void *)frame_buf,4096,0,(struct sockaddr*)&client,&addrlen);
				printf("Got data form ip:%s,port %d\n",inet_ntoa(client.sin_addr),htons(client.sin_port));

				//parse the msg
				if(ret<6)
				{
					gtloginfo("recvfrom ipalarm chn %d ERR len[%d]\n",i,ret);
					continue;
				}
				ipcall_pkt_t *p=(ipcall_pkt_t *)frame_buf;
				if(p->pkt_head!=head)
				{
					printf("head error:%#x\n",p->pkt_head);
					gtloginfo("recvfrom ipalarm chn %d HEAD ERR[%d]\n",i,p->pkt_head);
					continue;	
				}
				
				//judgement packet 
				readstatus |= (0x01<<i);
			}
		}

		//mix readstatus with status!
		*status = ((readstatus << 8)|*status);
		printf("alarm status is %x\n",*status);

	}while(0);

	return ret;

}

//读端子数据的线程
static void *watch_board_thread(void *para)
{
	DWORD oldstate = 0;//记录旧的端子输入状态，与报警无关
	DWORD oldpower = 0;
	DWORD newstate = 0;
	DWORD trig_diff = 0; //记录新旧输入状态的差异
	DWORD powerstate = 0;
	struct ipmain_para_struct *mainpara; 
	int ret,i;
	time_t timep;
	struct ip1004_state_struct *ip1004state;
#ifdef DISPLAY_THREAD_INFO
	printf("watch_board_thread running...\n");
#endif
	gtloginfo("start watch_board_thread...\n");

	mainpara = get_mainpara();
	//lc do 
#ifdef USE_IO
	set_trigin_attrib_perbit(mainpara->trig_in);
	printf("set_trigin_attrib_perbit %02x!!\n",mainpara->trig_in);
#endif
	//////set_trigout_attrib_perbit(mainpara->alarm_out);
	ip1004state = get_ip1004_state(0);

	//init_ipalarm_fdset();

	while(1)
	{
		//lc do 
#ifdef USE_IO		
		ret = read_trigin_status(&newstate);
		if(ret!=4)
		{
			gtloginfo("read_trigin_status return ret = %d\n",ret);
			usleep(1000000);
			continue;
		}

		//normal case : wait 1 sec
		ret = read_ipalarm_status(&newstate);
		if(ret<0)
		{
			gtlogerr("read ipalarm status read err!exit thread!\n");
			break;
		}

		if(mainpara->power_monitor_enable)
		{
			ret = read_power_status(&powerstate);
			if(ret!=1)
			{
				gtlogerr("read power status return ret = %d\n",ret);
				usleep(1000000);
				continue;
			}
			//printf("read pwr status is %d\n",powerstate);	
		}

		//process the state
		//printf("newstate is %x tin mask is %x\n",newstate,get_mainpara()->tin_mask);
		newstate&=get_mainpara()->tin_mask;
		if(newstate != oldstate || powerstate!=ip1004state->reg_per_state.pwr_loss)
		{
			
			time(&timep);
			//记日志
			printf("trigger from %02x to %02x\n",(int)oldstate,(int)newstate);
			gtloginfo("端子触发状态改变 %02x->%02x\n",(int)oldstate,(int)newstate);
			trig_diff = newstate ^ oldstate; 	
		
			printf("ip1004state->alarmin_state=[0x%lx]...newstate=0x%lx\n",ip1004state->alarmin_state,newstate);
			gtloginfo("ip1004state->alarmin_state=[0x%lx]...newstate=0x%lx\n",ip1004state->alarmin_state,newstate);
			printf("ip1004state->old_alarmin_state=[0x%lx]...oldstate=0x%lx\n",ip1004state->old_alarmin_state,oldstate);
			gtloginfo("ip1004state->old_alarmin_state=[0x%lx]...oldstate=0x%lx\n",ip1004state->old_alarmin_state,oldstate);

			//更新到ip1004state里去
			ip1004state->alarmin_state = newstate;
			ip1004state->old_alarmin_state = oldstate;
			ip1004state->alarmin_change_time = timep;
			
			if(mainpara->power_monitor_enable)
			{
				if(powerstate!=ip1004state->reg_per_state.pwr_loss)
				{
					if(powerstate==1)
					  gtlogwarn("外部电源丢失！！上报平台\n");
					else
					  gtlogwarn("外部电压恢复！！上报平台\n");
					ip1004state->reg_per_state.pwr_loss=powerstate;
				}

				//lc do add power monitor
				if((((newstate>>6)&1)==1)&&(((trig_diff>>6)&1)==1)) //低电压触发
				{
					gtlogwarn("电池电压过低！上报平台\n");
					ip1004state->reg_dev_state.pow_underflow_err=1;
				}
				if((((newstate>>7)&1)==1)&&(((trig_diff>>7)&1)==1)) //高电压触发
				{
					gtlogwarn("电池电压过高！上报平台\n");
					ip1004state->reg_dev_state.pow_overflow_err=1;
				}

				printf("pwr_loss is %d,pwr_under is %d,pwr_over is %d\n",powerstate,ip1004state->reg_dev_state.pow_underflow_err,ip1004state->reg_dev_state.pow_overflow_err);
			}
			//报告状态给设备
			send_dev_state(-1,1,0,-1,-1,0);
			send_alarmin_state_change(oldstate,newstate,timep,0);
			//写到日志里去
			//lc do
#ifdef DUMP_ALARM			
			dump_trigininfo_to_log(oldstate,newstate,timep,"--");	
#endif			
		}
		process_trigin_event(newstate,oldstate,timep);//处理报警相关
		oldstate=newstate;
#endif
		//lc do
		//usleep(1000000);
	}
	
	return 0; //should never reach here
}


int init_ipalarm_fd()
{
	int i,recvfd;
	struct sockaddr_in server;
	
	for(i=0;i<4;++i)
	{
		if((recvfd=socket(AF_INET,SOCK_DGRAM,0))==-1)
		{
			perror("Creating recvfd failed.");
			exit(1);
		}
		bzero(&server,sizeof(server));
		server.sin_family = AF_INET;
		server.sin_port = htons(alarm_port[i]);
		server.sin_addr.s_addr = htonl(INADDR_ANY);
		if(bind(recvfd,(struct sockaddr *)&server,sizeof(struct sockaddr))==-1)
		{
			perror("Bind()error.");
			exit(1);

		}
		ipalarm_fd[i] = recvfd;
		printf("recv fd at %d chn is %d\n",i,recvfd);
	}

	return 0;
}





/**********************************************************************************************
 * 函数名	:creat_watch_board_thread()
 * 功能	:创建接收并处理51报警模块发来的命令的线程
 * 输入	:attr:线程属性
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int creat_watch_board_thread(pthread_attr_t *attr)
{
	return pthread_create(&watchbd_thread_id,attr, watch_board_thread, NULL);//创建监视端子输入的线程
}


