/*
	负责GT1000系统到网关的连接以及各线程发送命令给网关
*/


//初始化各线程发送给网关连接线程命令的通道
#include "ipmain.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <gt_com_api.h>
#include "gate_connect.h"
#include "ipmain_para.h"
#include <gate_cmd.h>
#include <errno.h>
#include "gate_connect.h"
#include "netcmdproc.h"
#include "netinfo.h"
#include <commonlib.h>
#include "mainnetproc.h"
#include "leds_api.h"
#include "devstat.h"
#include "infodump.h"

#include <sys/syscall.h> /*此头必须带上*/

#define	TRANS_PKT_TO_GATEWAY	0x100	//需要网关连接线程将数据包转发出去


pid_t gettid()
{
     return syscall(SYS_gettid);  /*这才是内涵*/
}

static struct gate_struct gate_list[MAX_GATE_LIST]=//系统的网关列表
{       
	{PTHREAD_MUTEX_INITIALIZER,-1,0},
	{PTHREAD_MUTEX_INITIALIZER,-1,0},
	{PTHREAD_MUTEX_INITIALIZER,-1,0},
	{PTHREAD_MUTEX_INITIALIZER,-1,0},
	{PTHREAD_MUTEX_INITIALIZER,-1,0},
	{PTHREAD_MUTEX_INITIALIZER,-1,0}//alarm gate
};
/*
static struct gate_struct gate_list_dev2[MAX_GATE_LIST]=//系统的网关列表
{       
	{PTHREAD_MUTEX_INITIALIZER,-1,0},
	{PTHREAD_MUTEX_INITIALIZER,-1,0},
	{PTHREAD_MUTEX_INITIALIZER,-1,0},
	{PTHREAD_MUTEX_INITIALIZER,-1,0},
	{PTHREAD_MUTEX_INITIALIZER,-1,0},
	{PTHREAD_MUTEX_INITIALIZER,-1,0}//alarm gate
};
*/

typedef struct {
DWORD buf[1024];
}buf_t;

static buf_t gaterecv_buf[MAX_DEV_NO];
static buf_t sendrmt_buf[MAX_DEV_NO];
static int gate_recv_ch[MAX_DEV_NO]={-1};
static pthread_t recv_gate_ack_id[MAX_DEV_NO]={-1};	//处理由IP1004主动建立的连接的线程id
static pthread_t gate_connect_thread_id[MAX_DEV_NO]={-1};


void *recv_gate_ack_thread(void *devgate);
void *recv_gate_ack_thread(void *agate);
static void *gate_connect_thread(void *para);

/**********************************************************************************************
 * 函数名	:get_gate_list()
 * 功能	:获取一个描述网关结构数组的指针
 * 输入	:无
 * 返回值	:描述网关结构数组的指针，数组元素的个数为MAX_GATE_LIST
 **********************************************************************************************/
struct gate_struct *get_gate_list(int dev_no)
{
	if(dev_no == 0)
		return gate_list;
	//else
		//return gate_list_dev2;
}


static int init_gate_com_channel(void)
{
	int i = 0;

	gate_recv_ch[i]=mod_com_init(GATE_RECV_CHANNEL+i,MSG_INIT_ATTRIB);
		
	if(gate_recv_ch[i]<0)
	{
			printf("ipmain %s init_gate_com_channel error!!!\n",devlog(i));
			gtlogerr("%s初始化gate_com_channel失败!\n",devlog(i));
			return -1;
	}
#ifdef SHOW_WORK_INFO
		printf("ipmain create a com channel %x for key:%x\n",gate_recv_ch,GATE_RECV_CHANNEL);
		gtloginfo("ipmain为key值%x创建了一个com channel %x\n",GATE_RECV_CHANNEL,gate_recv_ch);
#endif

	return 0;
}


/**********************************************************************************************
 * 函数名	:set_gate_ip()
 * 功能	:将一个ip地址设置到网关ip地址列表中
 * 输入	:place:要设置的网关序号
 *			 ip:服务器ip地址（ip3,ip2,ip1,ip0）
 *			 port:服务器提供服务的端口号
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/
int set_gate_ip(int dev_no,int place,DWORD *ip,WORD port)
{
	
	struct gate_struct *gate;
	DWORD newgate;
	WORD newport;
	if(place<0)
		return -1;
	if(place>(MAX_GATE_LIST-1))
		return -1;
	if(ip==NULL)
		return -1;
	if(dev_no == 0)
		gate=&gate_list[place];
	else
		return -1;
	pthread_mutex_lock(&gate->mutex);
	if(gate->fd>0)
	{
		close(gate->fd);
		gate->fd=-1;
		gate->period=0;		
	}
	newgate=htonl(*ip);
	newport=htons(port);
	//printf("going to memcpy\n");
	memcpy(&gate->gateaddr.sin_addr,&newgate,sizeof(DWORD));	
	//printf("finished memcpy\n");
	gate->gateaddr.sin_port=newport;
	//printf("going to unlock\n");
	pthread_mutex_unlock(&gate->mutex);
	//printf("finished unlock\n");
	//gtloginfo("将%s:%d增加到网关ip地址列表中\n",newgate,newport);
	return 0;
}



#ifdef FOR_PC_MUTI_TEST
#include <time.h>
extern int test_alarm_interval;		//测试报警的时间间隔
extern int test_alarm_num;		//测试报警用的报警次数
static int test_alarm_cnt=0;
extern int test_alarm_need_inc;		//报警测试是是否需要每次报警信息变化
static struct send_dev_trig_state_struct test_dev_trig={0,0,0,0,0,0,0,0};	//测试用触发
#endif



static int gatedown_timer=0;
static int req_timesync_timer = 0; //请求对时计时器
/**********************************************************************************************
 * 函数名	:gate_connect_second_proc()
 * 功能	:网关连接线程的秒处理函数
 * 输入	:无	 
 * 返回值	:无
 **********************************************************************************************/
void gate_connect_second_proc(int count,int chn)
{
#ifdef FOR_PC_MUTI_TEST
	DWORD	*alarm_stat=NULL;
	int		i;
	struct tm *ptime;
	time_t ctime;	
#endif
	if(get_regist_flag(0) == 1)
	{
		if(++req_timesync_timer > 3600)//每小时对一次时
		{
			req_timesync_timer = 0;
			send_dev_req_sync_time(-1, 0, 0,0);
		}
	}
	else
		req_timesync_timer = 0;

	//lc add 2014-10-20
	if(count>0)
	{
		if(get_regist_flag(0)==0)
		{
			if(++gatedown_timer>count)
			{
				gatedown_timer=0;
				set_relay_output(chn,1);
			}
			else
			{
				set_relay_output(chn,0);
			}
		}
		else
		{
			gatedown_timer=0;
			set_relay_output(chn,0);
		}
	}
#ifdef FOR_PC_MUTI_TEST

		if(get_reportstate_flag()!=0)
		{
			if(test_alarm_interval>0)
			{
				if(--test_alarm_cnt<=0)
				{
					
					alarm_stat=&test_dev_trig.alarmstate;
					ctime=time(NULL);
					ptime=localtime(&ctime);
					test_dev_trig.year=ptime->tm_year+1900;
					test_dev_trig.month=ptime->tm_mon+1;
					test_dev_trig.day=ptime->tm_mday;
					test_dev_trig.hour=ptime->tm_hour;
					test_dev_trig.minute=ptime->tm_min;
					test_dev_trig.second=ptime->tm_sec;
					*alarm_stat=1;
					for(i=0;i<test_alarm_num;i++)
					{		
						send_dev_trig_state(-1,&test_dev_trig,1);
						if(test_alarm_need_inc)
							*alarm_stat+=1;
					}
					test_alarm_cnt=test_alarm_interval;
					test_alarm_cnt=test_alarm_interval;
				}
			}
		}
#endif
	return ;
}
//初始化网关连接线程变量
static int init_gate_connect_var(void)
{
	struct ipmain_para_struct *para;	
//	struct gate_struct *gate;
//	struct sockaddr_in *addr;
	para=get_mainpara();
	if(para==NULL)
		return -1;
#if 0
	gate=&gate_list[0];			//动态网关地址
	pthread_mutex_lock(&gate->mutex);
	bzero(&gate->gateaddr,sizeof(struct sockaddr_in));
	gate->gateaddr.sin_family = AF_INET;
	pthread_mutex_unlock(&gate->mutex);

	gate=&gate_list[1];
	pthread_mutex_lock(&gate->mutex);
	addr=&para->rmt_gate1;
	memcpy(&gate->gateaddr,addr,sizeof(struct sockaddr_in));
	gate->gateaddr.sin_family = AF_INET;
	pthread_mutex_unlock(&gate->mutex);	
	
	gate=&gate_list[2];
	pthread_mutex_lock(&gate->mutex);
	addr=&para->rmt_gate2;
	memcpy(&gate->gateaddr,addr,sizeof(struct sockaddr_in));
	gate->gateaddr.sin_family = AF_INET;
	pthread_mutex_unlock(&gate->mutex);	

	gate=&gate_list[3];
	pthread_mutex_lock(&gate->mutex);
	addr=&para->rmt_gate3;
	memcpy(&gate->gateaddr,addr,sizeof(struct sockaddr_in));
	gate->gateaddr.sin_family = AF_INET;
	pthread_mutex_unlock(&gate->mutex);	
	
	gate=&gate_list[4];
	pthread_mutex_lock(&gate->mutex);
	addr=&para->rmt_gate4;
	memcpy(&gate->gateaddr,addr,sizeof(struct sockaddr_in));
	gate->gateaddr.sin_family = AF_INET;
	pthread_mutex_unlock(&gate->mutex);	
	
	gate=&gate_list[5];
	pthread_mutex_lock(&gate->mutex);
	addr=&para->alarm_gate;
	memcpy(&gate->gateaddr,addr,sizeof(struct sockaddr_in));
	gate->gateaddr.sin_family = AF_INET;
	pthread_mutex_unlock(&gate->mutex);	
#endif

	return 0;
}

//连接到网关
int connect_to_gate(int dev_no, struct gate_struct *gate)
{
	int rc;
	int sock;
	struct sockaddr_in *addr;
	dev_gate_struct *devgate=malloc(sizeof(dev_gate_struct));
	if(gate==NULL)
		return -1;
	rc=-1;
	pthread_mutex_lock(&gate->mutex);
	do{
		if(gate->fd>0)
		{
			rc=gate->fd;
			gate->period=GATE_PERIOD_TIME;
		}
		else
		{		
#ifdef SHOW_WORK_INFO
			printf("dev %d connect_to_gate addr is %d\n",dev_no,gate->fd);
#endif
			addr=&gate->gateaddr;			
			if(addr->sin_addr.s_addr!=INADDR_ANY)	//没有设置该网关地址
			{
				addr->sin_family = AF_INET;
				sock = socket(PF_INET, SOCK_STREAM,IPPROTO_TCP);
				if(sock>0)
				{

#ifdef SHOW_WORK_INFO
					printf("ipmain dev %d  connecting to gate %s:%d...",dev_no,inet_ntoa(addr->sin_addr),ntohs(addr->sin_port));
#endif
				
					//rc=connect(sock,(struct sockaddr*)addr,sizeof(struct sockaddr_in));
					rc=connect_timeout(sock,(struct sockaddr*)addr,sizeof(struct sockaddr_in),6);
                    if(rc>=0)
					{
#ifdef SHOW_WORK_INFO
						printf(" gate connect SUCCESS!\n");
#endif	
						//gtlogwarn("连接网关%s:%d成功\n",inet_ntoa(addr->sin_addr),ntohs(addr->sin_port));
						//break;
						rc=sock;						
						gate->fd=sock;
						gate->period=GATE_PERIOD_TIME;		
						devgate->dev_no = dev_no;
						memcpy(&devgate->gate,&gate,sizeof(gate));
						create_recv_gate_ack_thread(devgate);
						
						break;
					}
					else
					{
#ifdef SHOW_WORK_INFO
						printf("gate connect FAILED!\n");
						//gtlogwarn("连接网关%s:%d失败\n",inet_ntoa(addr->sin_addr),ntohs(addr->sin_port));
#endif
						close(sock);
						break;
					}
				}
				else
					printf("dev %d connect_to_gate can't create socket!\n",dev_no);
			}
			else
			{
				//没有设置网关地址
#ifdef SHOW_WORK_INFO
				printf("didnt set the gateway address!!\n");
#endif
			}
		}	
	} while(0);
	pthread_mutex_unlock(&gate->mutex);	
	return rc;
}

static int find_active_gate(struct gate_struct *g_list,int num)
{
	int rc;
	int i;
	struct gate_struct *gate;
	rc=-1;
	gate=g_list;
	for(i=0;i<num;i++)
	{
		pthread_mutex_lock(&gate->mutex);
		if((gate->fd>0))//remed by shixin &&(gate->period>0))
			rc=gate->fd;
		pthread_mutex_unlock(&gate->mutex);
		if(rc>0)
			break;
		gate++;
	}
	return rc;
}
//在网关列表中寻找一个可用网关,如果没有则建立一个连接
//g_list:存放网关结构的列表，列表中的网关个数
//目前网关个数不能超过MAX_GATE_LIST个
//返回连接到网关的文件描述符,如果无法连接则返回负值
int get_active_gate(int dev_no, struct gate_struct *g_list,int num)
{
	int i,rc;
	struct gate_struct *gate;
	if(num>MAX_GATE_LIST)
		num=MAX_GATE_LIST;
	gate=g_list;
	rc=-1;
#ifdef DONT_CONNECT_GATE
	return -1;
#endif
	rc=find_active_gate(g_list,MAX_GATE_LIST);

	if(rc<0)
	{
		gate=g_list;
		if(num<0) //需先连接紧急报警服务器
		{
			rc=connect_to_gate(dev_no,g_list+abs(num)-1);
			if(rc>0)
				return rc;
			else
				num = abs(num)-1;
		}	
		for(i=0;i<num;i++)
		{
			rc=connect_to_gate(dev_no,gate);
			if(rc>0)
				break;
			gate++;
		}
	}
	return rc;
}



/**********************************************************************************************
 * 函数名	:send_gate_pkt()
 * 功能	:将填充好的数据包委托发送线程发送给网关
 * 输入	:fd:网络连接描述符,-1表示由发送线程建立一个主动连接
 *									   -2表示如果网关服务器都连接不通则连接紧急报警服务器
 *			len:要发送的数据长度(mod_com_type结构中包括cmd及以下字段的长度)
 *			send:填充好数据的结构指针,包括env,enc应该也填充好
 * 返回值	:描述网关结构数组的指针，数组元素的个数为MAX_GATE_LIST
 * 注		:各线程要想发送数据给远程网关则调用此接口统一发送
 *			 调用者需要把要发送给远程网关服务器的数据包全部填充在 mod_com_type结构中的para字段里面
 **********************************************************************************************/
int send_gate_pkt(int fd,struct mod_com_type *send,int len,int dev_no)

{
	int ret;
	if((dev_no >= MAX_VIRDEV_NO)||(dev_no < 0))
	{
		printf("虚拟设备号%d不正确\n",dev_no);
		gtlogerr("虚拟设备号%d不正确\n",dev_no);
		return -1;
	}
	if(gate_recv_ch[dev_no]<0)
	{
		printf("vsmain:can't send_gate_pkt() gate_recv_ch=%d\n",gate_recv_ch[dev_no]);
		gtlogerr("vsmain无法发送send_gate_pkt(),gate_recv_ch为%d\n",gate_recv_ch[dev_no]);
		return -1;
	}
	if(fd<0)	//因为消息队列的type不能小于0，所以如果传进来的参数
	{
		if(fd==-2)
			fd=2;	//需要连接紧急报警服务器
		else
			fd=1;	//小于0则把它转化为1(标准输出),在接收短判断是否为1,来确定是否有有效文件描述符
	}	
	send->target=fd;					
	send->source=dev_no;
	send->cmdlen=len+2;
	send->cmd=TRANS_PKT_TO_GATEWAY;
	ret=mod_com_send(gate_recv_ch[dev_no],send,0);
	if(ret!=0)
		gtloginfo("send_gate_pkt返回%d(errno=%d\n):%s\n",ret,errno,strerror(errno));
	return ret;
}

/**********************************************************************************************
 * 函数名	:send_pkt_to_gate()
 * 功能	:将一包填充好的数据发送给网关
 * 输入	:fd:已经建立好的网络连接描述符
 *			 send:已经填充好的待发数据结构指针
 *			 len:待发数据的长度(len+cmd+para)
 *			flag:保留，调用时写0
 *			env: 
 *			enc:加密算法
 * 返回值	:描述网关结构数组的指针，数组元素的个数为MAX_GATE_LIST
 **********************************************************************************************/



			
static int send_pkt_to_gate(int fd,struct gt_pkt_struct *send,int len,int flag, int env, int enc,int dev_no)
{
	//int env,enc;
	int rc;
	struct sockaddr_in addr;
	struct gt_usr_cmd_struct* cmd;
	int addrlen=sizeof(struct sockaddr);
	struct ipmain_para_struct *para;
	DWORD remote,localip,mask;
	struct ip1004_info_struct *info;
	
	if(fd>0)
	{
		para= get_mainpara();

		cmd=(struct gt_usr_cmd_struct*)send->msg;
		switch(cmd->cmd)
		{
			case DEV_REGISTER:
				rc=getpeername(fd,(struct sockaddr *)&addr,&addrlen);//获取连接对方的ip地址
				info=(struct ip1004_info_struct *)cmd->para;
				//printf("remote rc=%d %s\n",rc,inet_ntoa(addr.sin_addr));
				if(rc==0)
				{
					mask=(DWORD)para->lan_mask;
					localip=(DWORD)para->lan_addr;
					memcpy(&remote,&addr.sin_addr,4);
					if((remote&mask)!=(localip&mask))
					{
						//不在一个子网
						info->dev_ip=ntohl(para->wan_addr);
						
					}
					else
					{//在1个子网里面
						info->dev_ip=ntohl(para->lan_addr);
					}
					
				}				
			break;
			default:
			break;
		}

		//printf("para->rmt_env_mode=%d,para->rmt_enc_mode=%d\n",para->rmt_env_mode,para->rmt_enc_mode);
		//return gt_cmd_pkt_send(fd,send,len,NULL,0,(BYTE)para->rmt_env_mode,(BYTE)para->rmt_enc_mode);
		return gt_cmd_pkt_send(fd,send,len,NULL,0,(BYTE)env,(BYTE)enc);
	
	}
	else
	{
		printf("send_pkt_to_gate:fd=%d fix it\n",fd);
		gtloginfo("send_pkt_to_gate:fd=%d fix it\n",fd);
	}
	return -3;
}

//网关连接线程接收其他线程发来的命令
static int recv_gate_cmd(struct mod_com_type *recv,int dev_no)
{
	int rc;
	rc = mod_com_recv(gate_recv_ch[dev_no],0,recv,MAX_MOD_CMD_LEN,0);
	return rc;

}

static int log_connect_ok_flag[MAX_VIRDEV_NO]={0,0};	//记录过连接网关成功标志
static int log_connect_fail_flag[MAX_VIRDEV_NO]={0,0};  //记录过连接网关失败标志


//处理接收到的其他线程发来的网关相关命令
int gate_connect_cmd_proc(struct mod_com_type *cmd)
{
	struct gt_pkt_struct *send;
	struct gt_usr_cmd_struct *send_cmd;
	struct sockaddr_in gate_addr;
	struct ipmain_para_struct *mainpara;
	int addrlen;
	int rmt_fd;
	int sendlen;
	int trynum;
	int ret;
	int env,enc;
	int dev_no;
	
	dev_no = cmd->source;
	switch(cmd->cmd)
	{
		case TRANS_PKT_TO_GATEWAY:
			send=(struct gt_pkt_struct *)(sendrmt_buf[dev_no].buf);
			sendlen=cmd->cmdlen-2;	//去掉cmd->cmd的长度
			memcpy(send->msg,cmd->para,sendlen);	

			printf("ipmain get TRANS_PKT_TO_GATEWAY cmd from source %d,target is %d\n",cmd->source,cmd->target);
			if(cmd->target<=2)	//如果小于等于2表示没有确定的网络连接
			{	
				if(cmd->target==2)
					trynum=-MAX_GATE_LIST;
				else
					trynum=MAX_GATE_LIST-1;//最后一个地址是紧急报警服务器
				rmt_fd=get_active_gate(dev_no,get_gate_list(dev_no),trynum);
				
				 
				
				if(rmt_fd>0)
				{
					set_gate_connect_flag(dev_no,1);
					addrlen=sizeof(struct sockaddr_in);
					ret=getpeername(rmt_fd,(struct sockaddr *)&gate_addr,&addrlen);//获取连接对方的ip地址
					if(ret>=0)
					{
						if(!log_connect_ok_flag[dev_no])
						{
							gtloginfo("%s连接国通网关%s:%d成功,fd=%d\n",devlog(dev_no),inet_ntoa(gate_addr.sin_addr),ntohs(gate_addr.sin_port),rmt_fd);
							log_connect_ok_flag[dev_no]=1;
							log_connect_fail_flag[dev_no]=0;  
						}
					}			
					//将检查是否有需要重新发送的报警的工作放到 recv_gate_ack_thread 线程中

				}
				else
				{
					set_gate_connect_flag(dev_no,0);
				//printf("#########rmt_fd<=0\n");
					if(!log_connect_fail_flag[dev_no])
					{
						log_connect_fail_flag[dev_no]=1;
						log_connect_ok_flag[dev_no]=0;
						gtlogerr("%s不能连接到任何国通网关服务器\n",devlog(dev_no));
					}
				}
				mainpara = get_mainpara();
				env = mainpara->rmt_env_mode;
				enc = mainpara->rmt_enc_mode;
			
			}
			else
			{
				rmt_fd=cmd->target;
				env = cmd->env;
				enc = cmd->enc;
			}
			if(rmt_fd<0)
				ret=-1;
			else
			{
				addrlen=sizeof(struct sockaddr_in);
				ret=getpeername(rmt_fd,(struct sockaddr *)&gate_addr,&addrlen);//获取连接对方的ip地址
				ret=send_pkt_to_gate(rmt_fd,send,sendlen,0,env,enc,dev_no);
				send_cmd=(struct gt_usr_cmd_struct*)send->msg;

#ifdef SHOW_WORK_INFO
				printf("%s发送命令:0x%04x(%s) 到%s(fd=%d) 返回码%d\n",devlog(dev_no),send_cmd->cmd,get_gtcmd_name(send_cmd->cmd),inet_ntoa(gate_addr.sin_addr),rmt_fd,ret);
#endif
				//// lsk 2010-8-11
/*
				if(send_cmd->cmd==DEV_POSITION_RETURN)
				{
					;
				}
				else
				gtloginfo("%s发送命令:0x%04x (%s)到%s(fd=%d) 返回码%d\n",devlog(dev_no),send_cmd->cmd,get_gtcmd_name(send_cmd->cmd),inet_ntoa(gate_addr.sin_addr),rmt_fd,ret);
//				printf("%s发送命令:0x%04x (%s)到%s(fd=%d) 返回码%d\n",devlog(dev_no),send_cmd->cmd,get_gtcmd_name(send_cmd->cmd),inet_ntoa(gate_addr.sin_addr),rmt_fd,ret);
*/
			}
			
		break;
		default:
			printf("ipmain: dev %d gate_connect_trhead recv a unknow cmd:%x\n",dev_no,cmd->cmd);
			ret=-1;
		break;
	}
	return ret;
}


/**********************************************************************************************
 * 函数名	:creat_connect_thread()
 * 功能	:创建一个连接并发送命令 给网关的线程
 * 输入	:attr:创建线程的属性
 *		 
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int creat_connect_thread(pthread_attr_t *attr)
{
	int i = 0;
	init_gate_com_channel();	//初始化各线程与网关连接线程通讯命令的通道
	pthread_create(&gate_connect_thread_id[i],attr, gate_connect_thread, (void*)i);//创建连接网关以及将其他线程转发过来的命令发送给网关的线程
	return 0;
}
//负责连接网关以及将其他线程转发过来的命令发送给网关的线程
static void *gate_connect_thread(void *p)
{	
	int rc;
	struct mod_com_type *recv;
	int log_errflag=0;
	int dev_no;

	
	dev_no =(int)p;


#ifdef DISPLAY_THREAD_INFO
	printf("ipmain gate_connect_thread thread running...\n");
#endif
	gtloginfo("start gate_connect_thread %d...\n",dev_no);
	init_gate_connect_var();
	while(1)
	{		
		if(gate_recv_ch[dev_no]<0)
		{
			sleep(1);
			continue;
		}
		recv=(struct mod_com_type *)(gaterecv_buf[dev_no].buf);
		rc=recv_gate_cmd(recv,dev_no);
		if(rc>=0)
		{
			log_errflag=0;
			gate_connect_cmd_proc(recv);
		}
		else
		{
			if(!log_errflag)
			{
				log_errflag=1;
				gtlogerr("网关连接线程读取命令出错:%d:%s",rc,strerror(errno));
			}
			printf("ipmain recv_gate_cmd error rc=%d %s!!\n",rc,strerror(errno));
			usleep(500000);
		}
	}
	
	return NULL;
}


/*
	接收并处理由ip1004主动向网关建立的连接的描述符
*/
int create_recv_gate_ack_thread(dev_gate_struct *devgate)
{
	int *id;
	pthread_attr_t  thread_attr,*attr;
	int dev_no;
	dev_no = devgate->dev_no;
	
	id=(int*)&recv_gate_ack_id[dev_no];
	if(*id<0)
	 {
		if(get_gtthread_attr(&thread_attr)==0)
			attr=&thread_attr;
		else
			attr=NULL;
		pthread_create(&recv_gate_ack_id[dev_no], attr, recv_gate_ack_thread, devgate);
		

		if(attr!=NULL)
		{
			pthread_attr_destroy(attr);
		}
#ifdef SHOW_WORK_INFO
		printf("dev %d create recv_gate_ack_thread thread %d\n",dev_no,*id);
#endif
	}
	else
	{
#ifdef SHOW_WORK_INFO
		printf("dev %d recv_gate_ack_thread thread already started!\n",dev_no);
#endif
		gtloginfo("dev %d recv_gate_ack_thread thread already started!\n",dev_no);
		return -1;
	}
	return 0;
}
/*
	处理刚连通网关时需要做的一些事
*/
int ProcessGateConnect(int fd, int dev_no)
{//added by shixin
	struct ipmain_para_struct *para;
	//lc do 报警和端子记录，发送部分
#ifdef DUMP_ALARM	
	if((virdev_get_virdev_number()!=2)||(dev_no == 0))
	{
		 //审查报警纪录，若有未收到ACK的纪录，重发
			check_alarmlog();
			//若有未收到ACK的端子输入状态变化记录,重发
			check_triginlog();
	}
#endif	
	//发送更改了的配置文件到服务器
	para= get_mainpara();

#ifndef  FOR_PC_MUTI_TEST
	if((CheckParaFileChange(dev_no,2)==1))
	{
		AddParaFileVersion(dev_no,2);
		send_para_to_rmt(fd,2,para->rmt_env_mode,para->rmt_enc_mode,1,dev_no);
		CopyPara2Bak(dev_no,2);		
		send_dev_state(fd,1,1,para->rmt_env_mode,para->rmt_enc_mode,dev_no);
	}
	else if(para->devpara[dev_no].sendgateini_flag == 1)
	{
		send_para_to_rmt(fd,2,para->rmt_env_mode,para->rmt_enc_mode,1,dev_no);
	}
	
	para->devpara[dev_no].sendgateini_flag = 0;
#endif

	
//	send_para2gate_flag3=0;
	return 0;
}


void *recv_gate_ack_thread(void *dev_gate)
{
	int netfd;
	int ret;//,quitflag;
	fd_set readfds;
	int sel;
	int env=0,enc=0;
	in_addr_t addr;
	

	struct ipmain_para_struct *para;
	struct gt_pkt_struct *recv;
	struct gate_struct *gate;
	struct timeval	timeout;
	struct sockaddr_in gate_addr;
	int addrlen,temp;
 	DWORD recv_gate_ack_buffer[1024];
	DWORD	recv_gate_ack_tmp_buf[1024];
	int		connect_time=0;		//线程创建的时间(秒为单位)

	int dev_no;
	dev_gate_struct *devgate;
	if(dev_gate==NULL)
	{
		printf("recv_gate_ack_thread's para=NULL exit thread !\n");
		return NULL;
	}
	devgate =(dev_gate_struct *)dev_gate;
	dev_no	= devgate->dev_no;	
	gate=(struct gate_struct *)devgate->gate;
#ifdef SHOW_WORK_INFO
	printf("dev %d enter recv_gate_ack_thread! fd %d\n",dev_no,gate->fd);
#endif
	gtloginfo("dev %d start recv_gate_ack_thread...\n",dev_no);
	
	if(dev_no == 0)
	{
	//lc to do 设置led灯
	/*
		if((get_current_netled()<NET_GATE_CONNECTED)||(get_current_netled()==NET_PAP_FAILED))
			set_net_led_state(NET_GATE_CONNECTED);//连通了网关
	*/
	#ifdef USE_LED
				set_net_led_state(7);
	#endif
	}

	memset(recv_gate_ack_buffer,0,1024*sizeof(DWORD));
	recv=(struct gt_pkt_struct*)recv_gate_ack_buffer;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);		//设置线程为可取消的
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	FD_ZERO(&readfds);
	timeout.tv_sec=1;
	timeout.tv_usec=0;
	pthread_mutex_lock(&gate->mutex);
	netfd=gate->fd;	//其他地方不会修改gate->fd
	pthread_mutex_unlock(&gate->mutex);
	//quitflag=0;
	addrlen=sizeof(struct sockaddr); //modified by wsy
	getpeername(netfd,(struct sockaddr *)&gate_addr,&addrlen);
	connect_time=time(NULL);
	para= get_mainpara();
	ret=net_activate_keepalive(netfd);
	ret=net_set_recv_timeout(netfd,3);//FIXME
	ret=net_set_nodelay(netfd);

	ProcessGateConnect(netfd,dev_no);
	
	while(1)
	{

//	printf("gate->period=%d!!!!!!!!!\n",gate->period);
		
		if(netfd>0)
		{
			FD_SET(netfd,&readfds);
			sel=select(netfd+1,&readfds,NULL,NULL,&timeout);	
			
		}
		else
			break;
		

		if(sel==0)
		{
			FD_ZERO(&readfds);
			timeout.tv_sec=1;
			timeout.tv_usec=0;
#if 0
                    //added by shixin
                    if((((int)time(NULL)-connect_time)%3600)==0)
                    {//连接1小时对一次时
						send_dev_req_sync_time(netfd,para->rmt_env_mode,para->rmt_enc_mode,dev_no);
                    }
#endif
			if(gate->fd<0)
			{
				printf("dev %d recv_gate_ack_thread gate->fd=%d break!\n",dev_no,gate->fd);
				gtloginfo("dev %d recv_gate_ack_thread gate->fd=%d break!\n",dev_no,gate->fd);
				log_connect_ok_flag[dev_no]=0;
				log_connect_fail_flag[dev_no]=0;

				break;
			}
			else
				continue;
		}
		else if(sel<0)
		{
			if(errno==EINTR)
				continue;
			else
			{
				gtloginfo("dev %d recv_gate_ack_thread sel=%d errno=%d 关闭主动连接\n",dev_no,sel,errno);
				pthread_mutex_lock(&gate->mutex);
				//if(gate->period<=0)
				{
					gate->period=0;
					gate->fd=-1;
					log_connect_ok_flag[dev_no]=0;
					log_connect_fail_flag[dev_no]=0;
				}
				pthread_mutex_unlock(&gate->mutex);
				close(netfd);		
				break;
			}
		}
		if(FD_ISSET(netfd,&readfds))
		{			
			env=para->rmt_env_mode;
			enc=para->rmt_enc_mode;

#ifdef SHOW_WORK_INFO
			printf("gate master connect ...!!!!!!!!!!!!!!!!\n");
#endif
			ret=gt_cmd_pkt_recv_env(netfd,recv,sizeof(recv_gate_ack_buffer),recv_gate_ack_tmp_buf,0,&env,&enc);
#ifdef SHOW_WORK_INFO
			printf("gate master connect recv ret=%d ###########\n",ret);
#endif

			if(ret>=0)
			{		
#ifdef	SHOW_GATE_CMD_REC
				printf("dev %d recv gate pkt->",dev_no);
				print_gate_pkt(recv);	
#endif
				if((para->inst_ack))
				{
					if(env!=para->rmt_env_mode)
					{
#ifdef SHOW_WORK_INFO
							printf("%s收到的数字信封格式不匹配 set:env=%d recv:env=%d\n",devlog(dev_no),para->rmt_env_mode,env);
#endif
							gtlogwarn("%s收到的数字信封格式不匹配 set:env=%d recv:env=%d\n",devlog(dev_no),para->rmt_env_mode,env);

							continue;
					}
				}
				
				
				pthread_mutex_lock(&gate->mutex);
				gate->period=GATE_PERIOD_TIME;		//收到一个命令后恢复超时时间
				pthread_mutex_unlock(&gate->mutex);
				process_netcmd(netfd,(struct gt_usr_cmd_struct*)( recv->msg),env,enc,dev_no);
			}
			else
			{
				
				if(ret==-EAGAIN)
					continue;
				set_gate_connect_flag(dev_no,0);
				//rc=
				addrlen=sizeof(struct sockaddr); //modified by wsy
				if(dev_no == 0)
				{
					//lc to do 设置led灯
					/*
					if(get_current_netled()>=NET_GATE_CONNECTED)
					
					{
						//gtloginfo("断线，netled原为%d,现改为ADSL_OK\n",get_current_netled());
						addr=get_net_dev_ip("ppp0");
						if((int)addr==-1)//
							set_net_led_state(NET_NO_ADSL);
						else
							set_net_led_state(NET_ADSL_OK);	
					}
					*/
					#ifdef USE_LED
						set_net_led_state(0);
					#endif
				}
				if(ret==-ETIMEDOUT)
				{
	#ifdef		SHOW_WORK_INFO
					 printf("ETIMEDOUT 错误:%s到网关%s(fd=%d) 的主动连接超时,应重新注册\n",devlog(dev_no),inet_ntoa(gate_addr.sin_addr),netfd);
	#endif
					 gtlogerr("错误ETIMEDOUT:%s到网关 %s (fd=%d)的主动连接超时,应重新注册\n",devlog(dev_no),inet_ntoa(gate_addr.sin_addr),netfd);
				}
				else if(ret==-EHOSTUNREACH)
				{
	#ifdef		SHOW_WORK_INFO
					printf("%s到网关 %s(fd=%d) 的主动连接 EHOSTUNREACH 错误:host unreachable,应重新注册\n",devlog(dev_no),inet_ntoa(gate_addr.sin_addr),netfd);
	#endif
					gtlogerr("%s到网关 %s(fd=%d) 的主动连接host unreachable,应重新注册\n",devlog(dev_no),inet_ntoa(gate_addr.sin_addr),netfd);
				}
				else
				{
	#ifdef		SHOW_WORK_INFO
					printf("remote gate close device %d master connection,ret =%d\n",dev_no,ret);
	#endif
					temp=0-ret;
					gtloginfo("远程网关服务器:%s(fd=%d) 关闭%s的主动连接,ret=%d(%s)\n",inet_ntoa(gate_addr.sin_addr),netfd,devlog(dev_no),ret,strerror(temp));
				}

				break;
			}
		}
		else
		{
#ifdef		SHOW_WORK_INFO
			printf("dev %d recv_gate_ack_thread no fd is set\n",dev_no);
#endif
			gtlogerr("dev %d recv_gate_ack_thread no fd is set\n",dev_no);
			sleep(1);
		}
		//FD_SET(netfd,&readfds);
		
	}
					
#ifdef SHOW_WORK_INFO
	printf("dev %d exit recv_gate_ack_thread connect :%d s...\n",dev_no,(int)(time(NULL)-connect_time));
#endif	
	gtloginfo("dev %d exit recv_gate_ack_thread connect :%d s...\n",dev_no,(int)(time(NULL)-connect_time));
				
	sleep(20);			//网关需要隔一段时间才重新连接
	
	pthread_mutex_lock(&gate->mutex);
	//if(gate->period<=0)
	{
		gate->period=0;
		gate->fd=-1;		
		close(netfd);	
		log_connect_ok_flag[dev_no]=0;
		log_connect_fail_flag[dev_no]=0;
	}
	pthread_mutex_unlock(&gate->mutex);
	set_regist_flag(dev_no,0);
	recv_gate_ack_id[dev_no]=-1;
	return NULL;
}


