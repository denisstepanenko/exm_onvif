
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <file_def.h>
#include <commonlib.h>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "system_para_set.h"
#include "system_test.h"
#include "gt_dev_api.h"
#include "devinfo.h"
#include "nv_pair.h"
#include "communication.h"
#include "cmd_process.h"

#include <gtlog.h>
#include <gtthread.h>
#include <signal.h>

#define DEVSET_LOCK_FILE "/lock/ipserver/devset"
// 网络通讯的参数数据结构
multicast_sock net_port0; 
multicast_sock net_port1;
static pthread_mutex_t	cmd_process_mutex=PTHREAD_MUTEX_INITIALIZER;		//多网口接收处理数据的互斥体

void printbuffer(char *buf, int len)
{
	int i=0;
	int j=0;

	for(i=0;i<len;i++)
	{
		printf("%02x",buf[i]&0xff);
		j++;
		if(j>=16)
		{
			j=0;
			printf("\n");
		}
	}
	printf("\n");
}
/*
*************************************************************************
*函数名	:netport_thread
*功能	:网络数据接收子线程
 *输入	:multicast_sock* nt 网络接口接收发送数据所需的参数
*输出	:无
*修改日志:
*************************************************************************
*/
void netport_thread(multicast_sock* nt)
{
	int ret;
	NVP_TP *dist = NULL;
	char rcv_buf[CMD_BUF_LEN];
	
	dist = nvp_create();					//创建一个存放名值对的缓冲区
	nvp_set_equal_mark(dist, eq_mark);	//设置名值对的等于符号
	nvp_set_seperator(dist, sep_mark);	//设置名值对的分隔符号

	if(dist==NULL)
	{
		printf("can not use nv_pair lib\n");
		gtlogerr("devset 模块:nv_pair lib无法使用退出\n");		
		pthread_exit(NULL);
	}
	ret = init_net_port(nt->hostname, nt->multi_port, nt);
	while(ret)
	{
		sleep(5);
		
		reset_network(ETH0);// lsk 2007 -6-13 
		if(get_eth_num()==2)// lsk 2007 -6-13 
		reset_network(ETH1);// lsk 2007 -6-13 
		perror("init net port:");
		ret = init_net_port(nt->hostname, nt->multi_port, nt);
	}
	printf("init net port ok\n");
	gtloginfo("start server thread for %s -> %d\n", nt->hostname, nt->multi_port);

	while(1)
	{
      	//读取数据
      		//// lsk 2009 -2-10 for gtvs3022
		  if(virdev_get_virdev_number()==2)
			ret =dual_id_recv_dev_pkt(nt->fd, &nt->recv_addr, nt->self_id, nt->self_id1,
							nt->target_id, nt->recv_id, rcv_buf, sizeof(rcv_buf)-1,
							&nt->enc_type, nt->flag);			
		  else
			ret = recv_dev_pkt(nt->fd, &nt->recv_addr, nt->self_id, nt->target_id,
							rcv_buf, sizeof(rcv_buf)-1, &nt->enc_type, nt->flag);
		if(ret <0)
		{
			perror("recvfrom");
			gtlogerr("recv_dev_pkt error ret=%d\n",ret);
		}
		else	
		{
		//解析接收到的命令数据包
			rcv_buf[ret]='\0';
			if(nvp_parse_string(dist, rcv_buf)<0)
			{
 				printf(" can not parse packet \n");
				gtlogerr("devset 模块:收到无法解析的命令包\n");		
			}
			else
			{
				//处理命令数据
				pthread_mutex_lock(&cmd_process_mutex);
//				printf("net port get data !!!!!!!!!!!");
				cmd_handdle(dist , nt);// 命令处理
				pthread_mutex_unlock(&cmd_process_mutex);
			}
	//		printbuffer(buf, ret);
	//	清空接收队列
			memset(rcv_buf , 0 ,sizeof(rcv_buf));
		}	
	}
	nvp_destroy(dist);	//// lsk 2009-5-14 
}

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
			gtloginfo("devset 被kill,程序退出!!\n");
			exit(0);
		break;
		case SIGINT:         ///<ctrl-c信号
			gtloginfo("devset 被用户终止(ctrl-c)\n");
			exit(0);
		break;
		case SIGSEGV:       ///<段错误信号
			gtlogerr("devset 发生段错误\n");
			printf("devset segmentation fault\n");
			exit(0);
		break;
	}
	return;
}

int main(void)
{
	int lock_file=-1;
	char pbuf[100];
	pthread_t  eth1_thread, eth0_thread; // eth0_thread;
	int eth1_thread_node, eth0_thread_node; //, eth0_thread_node;


/**********判断本程序是否已经启动*************************/
	gtopenlog("devset");		///added by shixin 打开日志记录服务
	lock_file=create_and_lockfile(DEVSET_LOCK_FILE);
	if(lock_file<=0)
	{
		printf("devset are running!!\n");
		gtlogerr("devset 模块已运行，故启动无效退出\n");		
		exit(1);
	}
	memset(pbuf, 0 , sizeof(pbuf));
	sprintf(pbuf,"%d\n version:%s\n",getpid(),VERSION);//纪录程序版本信息
	write(lock_file,pbuf,strlen(pbuf)+1);//将进程的id号存入锁文件中

	printf("启动 devset(ver:%s)\n", VERSION);
	gtloginfo("启动 devset(ver:%s)\n", VERSION);

////注册信号处理函数
#ifndef _WIN32
	///注册信号处理函数
	signal(SIGKILL,exit_log);		 ///<kill -9信号
	signal(SIGTERM,exit_log);		 ///<普通的kill信号
	signal(SIGINT,exit_log);		        ///<ctrl-c信号
	signal(SIGSEGV,exit_log);		 ///<段错误信号
	signal(SIGPIPE,exit_log);		 ///<向已关闭的socket中写数据信号
#endif	
	
	init_sys_paras();	//初始化系统参数
	create_test_state_file();// 生成测试状态纪录文件 lsk 2007-2-8
//	printf("ok \n");
	if(get_eth_num()==2)	//如果设备有两个网口
	{

////初始化网口1 启动接收线程
		net_port1.multi_port = MULTI_CAST_PORT;
		memset(net_port1.hostname,0,sizeof(net_port1.hostname));
		memcpy(net_port1.hostname, HOSTNAME_ETH1,strlen(HOSTNAME_ETH1));
		eth1_thread_node = GT_CreateThread(&eth1_thread, (void*) &netport_thread, &net_port1);					 
		if(eth1_thread_node==0)
		{
			printf("open  net port 2 thread ok !\n");
		}
		else
		{
			gtlogerr("devset 模块:fail to open net port 2 thread !\n");		
		 	printf("fail to open net port 2 thread !\n");
//			exit(1);
		}
	}


////初始化网口0 启动接收线程
	net_port0.multi_port = MULTI_CAST_PORT;
	memset(net_port0.hostname,0,sizeof(net_port0.hostname));
	memcpy(net_port0.hostname, HOSTNAME_ETH0,strlen(HOSTNAME_ETH0));
	eth0_thread_node = GT_CreateThread(&eth0_thread, (void*) &netport_thread, &net_port0);					 
	if(eth0_thread_node==0)
	{
		printf("open  net port 1 thread ok !\n");
	}
	else
	{
		gtlogerr("devset 模块:fail to open net port 1 thread !\n");		
	 	printf("fail to open net port 1 thread !\n");
//	 	exit(1);
	}

	sleep(30);
#if 0 //for ip1004,没有硬盘
	memset(pbuf, 0 , sizeof(pbuf));
	get_dev_hd_sn("/dev/hda",pbuf,sizeof(pbuf));
#endif	
	while(1)	// do nothing here 
	{
		sleep(50);
	}
	close(lock_file);
	exit(0);
}



