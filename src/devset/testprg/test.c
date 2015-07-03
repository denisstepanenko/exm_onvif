#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <file_def.h>
#include <gtthread.h>
#include <commonlib.h>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

//#include <linux/in.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include <gt_dev_api.h>
#include <nv_pair.h>
#include "communication.h"
#include <guid.h>
#include <gtthread.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <linux/sockios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <gtlog.h>

#define CMD_HEAD		"cmd=="
#define CMD_GUID		"GUID"
#define PI_GUID			"PUID"
#define CMD_PARA_SET	"set para"
#define CMD_PRDC_SET	"set product"
#define CMD_TEST			"board"
#define CMD_UPDATA		"update"
#define CMD_CLEAN		"clear"
#define CMD_SEARCH		"search"
#define CMD_STATE		"state"
#define CMD_AUTO_TEST	"auto"
#define CMD_HELP		"help"
#define CMD_AUDIO		"audio"
#define CMD_QUIT		"q"

#define BYTE		unsigned char 
#define STDIN_FD 	0
#define VERSION	"0.02"
/*

	0.02 增加了监听功能
	0.01 devset 的测试程序
*/

#define ETH_NAME "eth0"
//#define IFNAMSIZ	  sizeof(ETH_NAME)
const char *test_dev="068700560000020f";

#define NET1
#define NET0
#define MAX_TEST_DEV_NUM		10
#define PROC1_NAME	"set_dev_ID_IP"
#define PROC2_NAME	"test_dev_board"
#define PROC3_NAME	"test_dev_multimedia"

//#define _MULTI_TEST_
////测试线程的状态数据结构
typedef struct
{
	int node_th;
	int test_stat;
	pthread_t	p_th;
	unsigned char dev_ip[20];
	unsigned char dev_guid[20];
}multi_test_thread;
multi_test_thread th[MAX_TEST_DEV_NUM];// 最多支持 MAX_TEST_DEV_NUM个设备同时测试
unsigned char target_GUID[16];	////被测试设备的guid
//广播地址guid
const char broadcast_GUID[8]={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//本机的guid
static char server_GUID[8]={0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F};
static char server_GUID_str[16];
struct GT_GUID svr_guid;		//存放server的guid
struct GT_GUID dev_guid;		//存放设备的guid
//存放网络参数
multicast_sock net_port0;
multicast_sock net_port1;
/*
*************************************************************************
*函数名	:printbuffer
*功能	:打印数据
 *输入	:char *buf 要打印的信息
 		:int len 信息长度参数
*输出	:无
*修改日志:
*************************************************************************
*/
void printbuffer(char *buf, int len)
{
	int i = 0;
	int j = 0;

	for(i=0;i<len;i++)
	{
		printf("%02x ",buf[i]&0xff);
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
*函数名	:set_port_id
*功能	:设置self id 和 target id
 *输入	:multicast_sock* net_st 网络接口接收发送数据所需的参数
 		:char *self_id 测设备的guid
 		:char * target_id 被测试设备的GUID 
*输出	:无
*修改日志:
*************************************************************************
*/
void set_port_id(multicast_sock* net_st, char *self_id, char * target_id)
{
	net_st->enc_type = 0;
//	memset(net_st->self_id, 2 , sizeof(net_st->self_id));
	memcpy(net_st->self_id, self_id, sizeof(net_st->self_id));
	memcpy(net_st->target_id, target_id, sizeof(net_st->target_id));
//	printbuffer(net_st->target_id, sizeof(net_st->target_id));
//	printbuffer(net_st->self_id, sizeof(net_st->self_id));
//	return 0;
}
/*
*************************************************************************
*函数名	:set_server_id
*功能	:设置测试设备的guid
 *输入	:unsigned char * target_id  测试设备的GUID 
*输出	:无
*修改日志:
*************************************************************************
*/
void set_server_id(unsigned char * server_id)
{
	memcpy(server_GUID,server_id,sizeof(server_GUID));
	memcpy(net_port0.self_id, server_GUID, sizeof(server_GUID));
	memcpy(net_port1.self_id, server_GUID, sizeof(server_GUID));
}
/*
*************************************************************************
*函数名	:set_device_id
*功能	:设置被测设备的guid
 *输入	:unsigned char * target_id  被测试设备的GUID 
*输出	:无
*修改日志:
*************************************************************************
*/
void set_device_id(unsigned char * target_id)
{
	set_port_id(&net_port0, (unsigned char*)server_GUID, target_id);
	set_port_id(&net_port1, (unsigned char*)server_GUID, target_id);
}
/*
*************************************************************************
*函数名	:print_menu
*功能	:打印菜单
 *输入	:无
*输出	:无
*修改日志:
*************************************************************************
*/
void print_menu(void)
{
	printf("生产测试软件测试程序[VERSION  %s]\n", VERSION);
	printf("查找设备命令测试	<search> \n");
	printf("输入设备GUID 		<GUID>:<GUID string>\n");
	printf("生产参数设置命令测试	<set product>:<command string>\n");
	printf("系统参数设置命令测试	<set para>:<command string>\n");
	printf("测试声音系统		<audio> \n");
	printf("系统升级命令测试	<update>:<command string>\n");
	printf("清除命令测试		<clear> \n");
	printf("板卡测试命令测试	<board>:<command string> \n");
	printf("测试状态		<state>:<command string> \n");
	printf("自动测试		<auto>:<device GUID>\n");
	printf("监听测试主机		<puid>:<主机GUID>\n");
	printf("打印菜单		<help> \n");
	printf("退出测试程序		<q>\n");
}
/*
*************************************************************************
*函数名	:send_test_cmd
*功能	:发送测试命令
*输入	:multicast_sock* nt 网络接口接收发送数据所需的参数
		:char * buf 命令信息
		:unsigned char* t_id 被测设备GUID
*输出	:无
*修改日志:
*************************************************************************
*/
int send_test_cmd(char * buf, unsigned char* t_id)
{
	int ret = -1;
	int len = 0 ;
	len = strlen(buf);
	memcpy(net_port0.target_id, t_id,sizeof(net_port0.target_id));
	memcpy(net_port1.target_id, t_id,sizeof(net_port1.target_id));
	
//	printbuffer(net_port0.target_id, sizeof(net_port0.target_id));
	#ifdef NET0
	printf("send to %s ", inet_ntoa(net_port0.send_addr.sin_addr) );
	printbuffer(net_port0.target_id,8);
	ret =send_dev_pkt(net_port0.fd, &net_port0.send_addr, net_port0.target_id,
	net_port0.self_id, buf, len , net_port0.enc_type, 0);
	if(ret<0)
	{
		printf("ret =%d %s\n", ret, strerror(errno));
	}
	#endif
	#ifdef NET1
	printf("send to %s\n", inet_ntoa(net_port1.send_addr.sin_addr));
	ret =send_dev_pkt(net_port1.fd, &net_port1.send_addr, net_port1.target_id,
	net_port1.self_id, buf, len , net_port1.enc_type, 0);
	if(ret<0)
	{
		printf("ret =%d %s\n", ret, strerror(errno));
	}
	#endif
	return 0;
}
/*
*************************************************************************
*函数名	:get_test_dev_ip
*功能	:获得测试设备的IP地址
*输入	:struct GT_GUID id
*输出	:unsigned char *ip_addr 设备的IP地址
*修改日志:
*************************************************************************
*/
int get_test_dev_ip(struct GT_GUID id, unsigned char *ip_addr)
{
	int i;
	unsigned char guid_str[20];
	guid2hex(id, guid_str);
	for(i=0;i<MAX_TEST_DEV_NUM;i++)
	{
		if(th[i].node_th!=-1)
		{
			if(strncasecmp(guid_str, th[i].dev_guid, strlen(guid_str))==0)
			{
				sprintf(ip_addr,"%s",th[i].dev_ip);
				break;
			}
		}
	}
	return 0;
}
/*
*************************************************************************
*函数名	:test_fac_para_set
*功能	:测试功能函数:设备出厂参数设置
 *输入	:multicast_sock* ns 网络接口接收发送数据所需的参数
*输出	:0 正确 负值错误
*修改日志:
*************************************************************************
*/
int test_fac_para_set(multicast_sock*ns)
{
	const char* buf=NULL;
	NVP_TP *dist = NULL;
	struct GT_GUID guid;
	unsigned char dev_guid_str[20];
	
	dist = nvp_create();
	nvp_set_equal_mark(dist, eq_mark);	//设置名值对的等于符号
	nvp_set_seperator(dist, sep_mark);	//设置名值对的分隔符号

	memset(dev_guid_str, 0 ,sizeof(dev_guid_str));
	memcpy(&guid, ns->target_id, sizeof(guid));
	guid2hex(guid, dev_guid_str);

	nvp_set_pair_str(dist, CMD_STR, M_SET_FACTORY);		// cmd == M_SET_FACTORY
	nvp_set_pair_str(dist, SEQ_STR, "210");				// seqence number =210
	nvp_set_pair_str(dist, GUID, dev_guid_str);				// GUID
	nvp_set_pair_str(dist, BOARD_SEQ,"gt3.61");			// board seqence number
	nvp_set_pair_str(dist, BATCH_SEQ, "2007_001");		// batch seqence number
	nvp_set_pair_str(dist, LEAVE_FAC, "20070103120000");	//time of leave factory

	buf = nvp_get_string(dist);
	send_test_cmd((char*)buf, ns->target_id);
	nvp_destroy(dist);
	return 0;
}
/*
*************************************************************************
*函数名	:test_para_set
*功能	:测试功能函数:设备参数设置
 *输入	:multicast_sock* ns 网络接口接收发送数据所需的参数
*输出	:0 正确 负值错误
*修改日志:
*************************************************************************
*/
int test_para_set(multicast_sock*ns)
{
	const char* buf=NULL;
	NVP_TP *dist = NULL;
	struct GT_GUID id;
	unsigned char ip_addr[20];
	dist = nvp_create();
	nvp_set_equal_mark(dist, eq_mark);	//设置名值对的等于符号
	nvp_set_seperator(dist, sep_mark);	//设置名值对的分隔符号

	nvp_set_pair_str(dist, CMD_STR, M_SET_PARA);			// cmd == M_SET_PARA
	nvp_set_pair_str(dist, SEQ_STR, "211");				// seqence number =211

	memset(ip_addr,0,sizeof(ip_addr));
	memcpy(&id, ns->target_id, sizeof(id));
	get_test_dev_ip(id, ip_addr);

	nvp_set_pair_str(dist, ETH0_IP, ip_addr);				// eth0 IP
	nvp_set_pair_str(dist, ETH1_IP, "192.168.0.10");		// eth1 IP  
	nvp_set_pair_str(dist, ETH0_MASK, "255.0.0.0");			// eth0 net mask  
	nvp_set_pair_str(dist, ETH1_MASK, "255.255.255.0");		//eth1 net mask   
	nvp_set_pair_str(dist, DEV_TIME, "20070226111240");		//time of device

	buf = nvp_get_string(dist);
	send_test_cmd((char *)buf, ns->target_id);
	nvp_destroy(dist);
	return 0;
}
/*
*************************************************************************
*函数名	:set_test_dev_stat
*功能	:设置测试设备的状态标志
*输入	:unsigned char* id 设备的guid或者ip地址
		:int val 状态标志
*输出	:无
*修改日志:
*************************************************************************
*/
void set_test_dev_stat(unsigned char* id, int val)
{
	int i;
	for(i=0;i<MAX_TEST_DEV_NUM;i++)
	{
		if(th[i].node_th!=-1)
		{
			if(strncasecmp(th[i].dev_guid, id, strlen(id))==0)
			{
				th[i].test_stat=val;
				return;
			}
			if(strncasecmp(th[i].dev_ip, id, strlen(id))==0)
			{
				th[i].test_stat=val;
				return;
			}
		}
	}
}
/*
*************************************************************************
*函数名	:get_test_dev_stat
*功能	:读取测试设备的状态标志
*输入	:unsigned char* id 设备的guid或者ip地址
*输出	:无
*返回值	:测试设备的状态标志
*修改日志:
*************************************************************************
*/
int get_test_dev_stat(unsigned char* id)
{
	int i;
	for(i=0;i<MAX_TEST_DEV_NUM;i++)
	{
		if(th[i].node_th!=-1)
		{
			if(strncasecmp(th[i].dev_guid, id, strlen(id))==0)
			{
				return th[i].test_stat;
			}
			if(strncasecmp(th[i].dev_ip, id, strlen(id))==0)
			{
				return th[i].test_stat;
			}
		}
	}
	return 0;
}
/*
*************************************************************************
*函数名	:record_test_stat
*功能	:测试功能函数:设备测试状态纪录功能测试
*输入	:multicast_sock* ns 网络接口接收发送数据所需的参数
		:unsigned char *prc_name 测试工序名称
		:int prc_num 测试工序的序号
*输出	:无
*修改日志:
*************************************************************************
*/
int record_test_stat(multicast_sock*ns, unsigned char *prc_name, int prc_num)
{
	const char* buf=NULL;
	NVP_TP *dist = NULL;
	unsigned char temp[100];

	dist = nvp_create();
	nvp_set_equal_mark(dist, eq_mark);	//设置名值对的等于符号
	nvp_set_seperator(dist, sep_mark);	//设置名值对的分隔符号

	nvp_set_pair_str(dist, CMD_STR, M_TEST_STATE);	// cmd == M_SET_PARA
	nvp_set_pair_str(dist, SEQ_STR, "212");			// seqence number =212


	nvp_set_pair_int(dist, SET_STATE, 1);
	memset(temp,0,sizeof(temp));
	sprintf(temp, "%s%d", TEST_STATE_VALUE, prc_num);
	nvp_set_pair_int(dist, temp, 1);
	
	memset(temp,0,sizeof(temp));
	sprintf(temp, "%s%d", TEST_STATE_NAME, prc_num);
	nvp_set_pair_str(dist, temp, prc_name);		   

	buf = nvp_get_string(dist);
	printf("%s",buf);
	send_test_cmd((char *)buf, ns->target_id);
	nvp_destroy(dist);
	return 0;
}
/*
*************************************************************************
*函数名	:dev_test_process
*功能	:测试功能函数:设备硬件功能测试
 *输入	:multicast_sock* ns 网络接口接收发送数据所需的参数
*输出	:无
*修改日志:
*************************************************************************
*/
void dev_test_process(multicast_sock*ns)
{
	struct GT_GUID id;
	unsigned char hex[20];
	unsigned char buf[1024];

	memset(hex,0,sizeof(hex));
	memset(buf,0,sizeof(buf));
	memcpy(&id, ns->target_id, sizeof(id));
	guid2hex(id, hex);

	sprintf(buf, "%s%s%s%s%s%s%s", CMD_HEAD, M_TEST_DEV,
		sep_mark, SEQ_STR, eq_mark,"213" ,
		"&&board_test==1");
	set_test_dev_stat(hex, 1);
	send_test_cmd(buf, ns->target_id);
	memset(buf,0,sizeof(buf));
	while(get_test_dev_stat(hex))
	{
		sleep(1);
	}
	sleep(2);

	sprintf(buf, "%s%s%s%s%s%s%s", CMD_HEAD, M_TEST_DEV,
		sep_mark, SEQ_STR, eq_mark,"214" ,
		"&&trig_test==1");
	set_test_dev_stat(hex, 1);
	send_test_cmd(buf, ns->target_id);
	memset(buf,0,sizeof(buf));
	while(get_test_dev_stat(hex))
	{
		sleep(1);
	}
	sleep(2);
	sprintf(buf, "%s%s%s%s%s%s%s", CMD_HEAD, M_TEST_DEV,
		sep_mark, SEQ_STR, eq_mark,"215" ,
		"&&format_disk==1");
	set_test_dev_stat(hex, 1);
	send_test_cmd(buf, ns->target_id);
	memset(buf,0,sizeof(buf));
	while(get_test_dev_stat(hex))
	{
		sleep(1);
	}

	sleep(2);
	sprintf(buf, "%s%s%s%s%s%s", CMD_HEAD, M_CLEAR_DEV,
	sep_mark, SEQ_STR, eq_mark,"216");
//	set_test_dev_stat(hex, 1);
//	send_test_cmd(buf, ns->target_id);
	while(get_test_dev_stat(hex))
	{
		sleep(1);
	}
}
/*
*************************************************************************
*函数名	:auto_test_thread
*功能	:自动测试线程
 *输入	:multicast_sock* nt 网络接口接收发送数据所需的参数
*输出	:无
*修改日志:
*************************************************************************
*/
void auto_test_thread(multicast_sock*ns)
{
	multicast_sock nt;
	memcpy(&nt, ns, sizeof(nt));
	sleep(5);	//wait for get dev ip address 
	while(1)
	{
		sleep(2);
		test_fac_para_set(&nt);	//出厂信息设置
		sleep(2);
		test_para_set(&nt);		//设备参数设置
		record_test_stat(&nt, PROC1_NAME,1);	//测试流程信息纪录
		sleep(2);
		dev_test_process(&nt);	// 设备硬件测试
		record_test_stat(&nt, PROC2_NAME,2);	//测试流程信息纪录
		sleep(2);
		record_test_stat(&nt, PROC3_NAME,3);	//测试流程信息纪录
		sleep(2);
	}
}
/*
*************************************************************************
*函数名	:search_dev
*功能	:查找设备
 *输入	:无
*输出	:无
*修改日志:
*************************************************************************
*/
void search_dev(void)
{
	unsigned char buf[200];
	sprintf(buf, "%s%s%s%s%s%s", CMD_HEAD, M_SEARCH_DEV,
			sep_mark, SEQ_STR, eq_mark,"106");
//	set_device_id((unsigned char*) broadcast_GUID);
	send_test_cmd(buf,(char*)broadcast_GUID);
	set_device_id((unsigned char*) &dev_guid);
}
/*
*************************************************************************
*函数名	:cmd_process
*功能	:命令处理
*输入	:char* cmd 命令信息
		:multicast_sock* ns 网络接口接收发送数据所需的参数
*输出	:无
*修改日志:
*************************************************************************
*/
int cmd_process(char * cmd, multicast_sock*ns)
{
//	int ret = -1;
	int i;
	unsigned char*p_str = NULL;
	unsigned char cmd_buf[1024];
	unsigned char buf[2048];
	struct tm *ptime;
	time_t ctime;

	memset(cmd_buf, 0 ,sizeof(cmd_buf));
	memset(buf, 0 ,sizeof(buf));
	memcpy(cmd_buf, cmd, strlen(cmd));

//	memcpy(buf, cmd, strlen(cmd));

	if(strncasecmp(cmd_buf, CMD_AUTO_TEST,strlen(CMD_AUTO_TEST))==0)
	{
		printf("start auto test\n");
		p_str=strchr(cmd_buf, ':');
		p_str+=1;
		memset(target_GUID, 0 , sizeof(target_GUID));
		memcpy(target_GUID, p_str, 16);
		dev_guid = hex2guid(target_GUID);
		search_dev();
		for(i=0;i<MAX_TEST_DEV_NUM;i++)
		{
			if(th[i].node_th==-1)
			 {
				memcpy(th[i].dev_guid,target_GUID,sizeof(target_GUID));
			 	th[i].node_th= GT_CreateThread(&th[i].p_th, (void*) &auto_test_thread, ns);					 
			 	if(th[i].node_th==0)
				{
					gtloginfo("start auto test DEV GUID = %s\n", target_GUID);
					printf("start auto test DEV GUID = %s\n", target_GUID);
				}
				else 
				{
					gtloginfo("can not start board test \n");
				}
				break;
			}
		}
	}
	else if(strncasecmp(cmd_buf, CMD_PARA_SET,strlen(CMD_PARA_SET))==0)
	{
//		sprintf(buf, "%s%s%s", CMD_HEAD, M_SET_PARA,"&&eth0_mask==255.255.255.0");
//		sprintf(buf, "%s%s%s", CMD_HEAD, M_SET_PARA,"&&eth0_ip==192.168.2.15");
//		sprintf(buf, "%s%s%s", CMD_HEAD, M_SET_PARA,"&&eth0_mac==00:74:00:00:02:0F");
//		sprintf(buf, "%s%s%s", CMD_HEAD, M_SET_PARA,"&&dev_time==20060801120500");
//		sprintf(buf, "%s%s%s", CMD_HEAD, M_SET_PARA,"&&def_route==192.168.2.250");
		p_str=strchr(cmd_buf, ':');
		if(p_str!=NULL)
		{
			p_str+=1;
			sprintf(buf, "%s%s%s%s%s%s%s", CMD_HEAD, M_SET_PARA,
				sep_mark, SEQ_STR, eq_mark,"c9f80f9e-para-f5676053920c" ,
				p_str);
//			set_device_id((unsigned char*)&dev_guid);
//			set_port_id(&net_port0, (unsigned char*)server_GUID, (unsigned char*)&dev_guid);
//			set_port_id(&net_port1, (unsigned char*)server_GUID, (unsigned char*)&dev_guid);
			send_test_cmd(buf, (char*)&dev_guid);
			ctime=time(NULL);
			ptime=localtime(&ctime);	
			memset(buf,0,sizeof(buf));
			sprintf(buf,"%s%s%s%s%s%s&&dev_time==%04d%02d%02d%02d%02d%02d",
				CMD_HEAD, M_SET_PARA,
				sep_mark, SEQ_STR, eq_mark,
				"c9f80f9e-para-f5676053920c" ,
				(1900+ptime->tm_year),
				(ptime->tm_mon+1),
				ptime->tm_mday,
				ptime->tm_hour,
				ptime->tm_min,
				ptime->tm_sec);
			send_test_cmd(buf, (char*)&dev_guid);

		}
		else
		{
			printf("参数格式错误\n");
			print_menu();
		}
	}
	else if(strncasecmp(cmd_buf, CMD_PRDC_SET,strlen(CMD_PRDC_SET))==0)
	{
//		sprintf(buf, "%s%s%s", CMD_HEAD, M_SET_FACTORY,"&&dev_type_str==GT1000");
//		sprintf(buf, "%s%s%s", CMD_HEAD, M_SET_FACTORY,"&&batch_seq==1080");
//		sprintf(buf, "%s%s%s", CMD_HEAD, M_SET_FACTORY,"&&board_seq==102");
//		sprintf(buf, "%s%s%s", CMD_HEAD, M_SET_FACTORY,"&&leave_fac==20010103120000");
		p_str=strchr(cmd_buf, ':');
		if(p_str!=NULL)
		{
			p_str+=1;
			sprintf(buf, "%s%s%s%s%s%s%s%s", CMD_HEAD, M_SET_FACTORY, "&&batch_seq==01",
				sep_mark, SEQ_STR, eq_mark,"c9f80f9e-fact-f5676053920c",p_str);
//			set_port_id(&net_port0, (unsigned char*)server_GUID, (unsigned char*)&dev_guid);
//			set_port_id(&net_port1, (unsigned char*)server_GUID, (unsigned char*)&dev_guid);
			send_test_cmd(buf, (char*)&dev_guid);

		}
		else
		{
			printf("参数格式错误\n");
			print_menu();
		}
	}
	else if(strncasecmp(cmd_buf, CMD_TEST,strlen(CMD_TEST))==0)
	{
//		sprintf(buf, "%s%s%s", CMD_HEAD, M_TEST_DEV,"&&board_test==1&&trig_test==1&&format_disk==1");
//		sprintf(buf, "%s", );
		p_str=strchr(cmd_buf, ':');
		if(p_str!=NULL)
		{
			p_str+=1;
			sprintf(buf, "%s%s%s%s%s%s%s", CMD_HEAD, M_TEST_DEV,
				sep_mark, SEQ_STR, eq_mark,"103" ,
				p_str);
//			set_port_id(&net_port0, (unsigned char*)server_GUID, (unsigned char*)&dev_guid);
//			set_port_id(&net_port1, (unsigned char*)server_GUID, (unsigned char*)&dev_guid);
			send_test_cmd(buf, (char*)&dev_guid);
//			dev_guid.year+=1;
//			send_test_cmd(buf, (char*)&dev_guid);  		/// lsk test
//			dev_guid.year-=1;

		}
		else
		{
			printf("参数格式错误\n");
			print_menu();
		}
	}
	
	else if(strncasecmp(cmd_buf, CMD_AUDIO,strlen(CMD_AUDIO))==0)
	{
		p_str=strchr(cmd_buf, ':');
		if(p_str!=NULL)
		{
			p_str+=1;
			sprintf(buf, "%s%s%s%s%s%s%s", CMD_HEAD, M_TEST_DEV_AUDIO,
				sep_mark, SEQ_STR, eq_mark,"108" ,
				p_str);
//			set_port_id(&net_port0, (unsigned char*)server_GUID, (unsigned char*)&dev_guid);
//			set_port_id(&net_port1, (unsigned char*)server_GUID, (unsigned char*)&dev_guid);
			send_test_cmd(buf, (char*)&dev_guid);
		}
		else
		{
			printf("参数格式错误\n");
			print_menu();
		}
	}
	else if(strncasecmp(cmd_buf, CMD_UPDATA,strlen(CMD_UPDATA))==0)
	{
//		sprintf(buf, "%s%s%s", CMD_HEAD, M_FTP_UPDATE_DEV,"&&port==23&&server==160&&path==/up/update.tar.gz&&user==lsk&&pass==1234&&size==5767932");
//		send_test_cmd(buf);
#if 1
		p_str=strchr(cmd_buf, ':');
		if(p_str!=NULL)
		{
			p_str+=1;
			sprintf(buf, "%s%s%s%s%s%s%s", CMD_HEAD, M_FTP_UPDATE_DEV,
				sep_mark, SEQ_STR, eq_mark,"104" ,
				p_str);
			send_test_cmd(buf, (char*)&dev_guid);
		}
		else
		{
			printf("参数格式错误\n");
			print_menu();
		}
#endif
	}
	else if(strncasecmp(cmd_buf, CMD_CLEAN,strlen(CMD_CLEAN))==0)
	{
		sprintf(buf, "%s%s%s%s%s%s", CMD_HEAD, M_CLEAR_DEV,
				sep_mark, SEQ_STR, eq_mark,"105");
		send_test_cmd(buf, (char*)&dev_guid);
	}
	else if(strncasecmp(cmd_buf, CMD_SEARCH,strlen(CMD_SEARCH))==0)
	{
		search_dev();
	}
	else if(strncasecmp(cmd_buf, CMD_HELP,strlen(CMD_HELP))==0)
	{
		print_menu();
	}
	else if(strncasecmp(cmd_buf, PI_GUID,strlen(PI_GUID))==0)	////// test for other PC
	{
		////puid:bbb2d6870d006370		
		p_str=strchr(cmd_buf, ':');
		p_str+=1;
		memset(server_GUID_str, 0 , sizeof(server_GUID_str));
		memcpy(server_GUID_str, p_str, sizeof(server_GUID_str));
		svr_guid= hex2guid(server_GUID_str);
		set_server_id((char*)&svr_guid);
	}
	
	else if(strncasecmp(cmd_buf, CMD_GUID,strlen(CMD_GUID))==0)
	{
		p_str=strchr(cmd_buf, ':');
		p_str+=1;
		memset(target_GUID, 0 , sizeof(target_GUID));
		memcpy(target_GUID, p_str, 16);
		dev_guid = hex2guid(target_GUID);
		set_device_id((char*)&dev_guid);
	}
	else if(strncasecmp(cmd_buf, CMD_STATE, strlen(CMD_STATE))==0)
	{
		p_str=strchr(cmd_buf, ':');
		if(p_str!=NULL)
		{
			p_str+=1;
			sprintf(buf, "%s%s%s%s%s%s%s", CMD_HEAD, M_TEST_STATE,
				sep_mark, SEQ_STR, eq_mark,"107",p_str);
			send_test_cmd(buf, (char*)&dev_guid);
		}
		else
		{
			print_menu();
		}

	}
	else if(strncasecmp(cmd_buf, CMD_QUIT, strlen(CMD_QUIT))==0)
	{
		printf("bye bye !!!!!\n");
		exit(0);
	}
	else 
	{
		printf("unknow command \n");
		print_menu();
		return -1;
	}
	printf("%s\n", buf);
	return 0;
}
/*
*****************************************************
*函数名称: init_net_port
*函数功能: 初始化网络连接
*输入： 		host_name host_name1 两个组播地址
*			port 组播端口号
*			net_st 存放网络参数的数据结构
*输出： 0 正确  错误退出程序
*修改日志：
*****************************************************
*/
int init_net_port(char* host_name, int port, multicast_sock* net_st)
{
//	int ret = -1;
	if(net_st==NULL)
	{
		return -1;
	}

	//construct socket
	net_st->fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(net_st->fd == -1)
	{
		perror("socket");
		return -1;
	}
//		 printf("socket fd = %d\n ", net_port0.fd);

#if 0
	net_st->enc_type = 0 ;
	memset(net_st->self_id, 0 , sizeof(net_st->self_id));
	memset(net_st->target_id, 0 , sizeof(net_st->target_id));
	memset(net_st->seq_num, 0 , sizeof(net_st->seq_num));
	//
	bzero(&net_st->send_addr,sizeof(net_st->send_addr));
	bzero(&net_st->recv_addr,sizeof(net_st->recv_addr));
#endif
	if(host_name!=NULL)
	{
		net_st->recv_addr.sin_family = AF_INET;
		net_st->recv_addr.sin_addr.s_addr = inet_addr(host_name); //htonl(INADDR_ANY);
		net_st->recv_addr.sin_port =htons(port);
	}
#if 1
	//bind socket 
	if(bind(net_st->fd, (struct sockaddr *)&net_st->recv_addr, 
		sizeof(net_st->recv_addr)) == -1)
	{
		perror("bind error");
//		gtlogerror("bind error");	//lsk 2007 -12 -4
		goto error_handdle;	// lsk 2006 -12 -28
//		return -1;
	}
#endif
	//setsocketopt
	net_st->loop = 1;
	if(setsockopt(net_st->fd,SOL_SOCKET,SO_REUSEADDR,	
		&net_st->loop,sizeof(net_st->loop))<0)
	{
		perror("setsocketopt:SO_REUSEADDR");
//		gtlogerror("setsocketopt:SO_REUSEADDR");	//lsk 2007 -12 -4
		goto error_handdle;	// lsk 2006 -12 -28
//		return -1;
	}
	// do not receive packet sent by myself
	net_st->loop = 0;	// test loop =0 no cycle , loop =1 cycle enable  
	if(setsockopt(net_st->fd,IPPROTO_IP,IP_MULTICAST_LOOP, 
		&net_st->loop,sizeof(net_st->loop))<0)
	{
		perror("setsocketopt:IP_MULTICAST_LOOP");
//		gtlogerror("setsocketopt:IP_MULTICAST_LOOP");	//lsk 2007 -12 -4
		goto error_handdle;	// lsk 2006 -12 -28
//		return -1;
	}

//join one multicast group
	if(host_name!=NULL)
	{
		net_st->mreq.imr_multiaddr.s_addr = inet_addr(host_name);
		net_st->mreq.imr_interface.s_addr = htonl(INADDR_ANY);
//		net_st->mreq.imr_interface.s_addr = htonl(port);  //lsk 2007-11-2 
		if(net_st->mreq.imr_multiaddr.s_addr == -1)
		{
			perror("not a legal multicast address!");
//			gtlogerror("not a legal multicast address!");	//lsk 2007 -12 -4
			goto error_handdle;	// lsk 2006 -12 -28
//			return -1;
		}
// lsk 2006 -12 -28
	//	reset_network(ETH1);
	//	reset_network(ETH0);
#if 1
		if(setsockopt(net_st->fd,IPPROTO_IP,IP_ADD_MEMBERSHIP, 
			&net_st->mreq , sizeof(net_st->mreq))<0)
		{
	//		reset_network(ETH1);
	//		reset_network(ETH0);
			if(setsockopt(net_st->fd,IPPROTO_IP,IP_ADD_MEMBERSHIP, 
				&net_st->mreq , sizeof(net_st->mreq))<0)
			{
				perror("setsockopt:IP_ADD_MEMBERSHIP");
//				gtlogerror("setsockopt:IP_ADD_MEMBERSHIP");//lsk 2007 -12 -4
				goto error_handdle;	// lsk 2006 -12 -28
//				return -1;
			}
		}
#endif
	}
	// set GUID as self ID
#if 0
	ret = pass_dev_GUID(net_st->self_id);	//test
	if(ret!=GUID_HEX_LEN/2)
	{
		printf("can not get GUID ret = %d \n", ret);
//		printbuffer(net_port0.self_id , DEV_GUID_BYTE);
		goto error_handdle;	// lsk 2006 -12 -28
//		return -1;
	}
#endif
	set_device_id((unsigned char*)&dev_guid);
	memcpy(net_st->hostname,host_name,strlen(host_name));
	net_st->multi_port=port;
	memcpy((void*)&net_st->send_addr, (void*)&net_st->recv_addr,sizeof(net_st->recv_addr));
	 return 0;
error_handdle: 
	close(net_st->fd);
	net_st->fd = -1;
	return -1;
}
 /*
*****************************************************
*函数名称: get_local_IP_address
*函数功能: 获取本机IP地址
*输入： 无
*输出： ip地址
*返回值: 0 正确 负值错误
*修改日志：
*****************************************************
*/ 
int get_local_IP_address(char* IP_addr)
{
	int sock; 
	struct sockaddr_in sin; 
	struct ifreq ifr; 
//	unsigned char mac[6]; 


	sock = socket(AF_INET, SOCK_DGRAM, 0); 

	if (sock == -1) 
	{ 
		perror("Error: get local IP socket fail!"); 
		return -1; 
	} 

	strncpy(ifr.ifr_name, ETH_NAME, IFNAMSIZ); 
	ifr.ifr_name[IFNAMSIZ - 1] = 0; 

	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) 
	{ 
		perror("Error: get local IP ioctl fail!"); 
		close(sock);
		return -1; 
	} 

	memcpy(&sin, &ifr.ifr_addr, sizeof(sin)); 
	sprintf(IP_addr, "%s",inet_ntoa(sin.sin_addr)); 
	close(sock);
	return 0;
}
 /*
*****************************************************
*函数名称: check_test_dev_info
*函数功能: 检查测试设备的状态
*输入	:multicast_sock* ns 网络接口接收发送数据所需的参数
		:unsigned char *buf 接收到的数据信息 
*输出： 无
*返回值: 0 正确 负值错误
*修改日志：
*****************************************************
*/ 
int check_test_dev_info(multicast_sock*ns, unsigned char *buf)
{
	int i;
	NVP_TP *dist = NULL;
	const char* p_str=NULL;
	unsigned char ip_addr[50];
	
	memset(ip_addr,0,sizeof(ip_addr));
	dist = nvp_create();
	nvp_set_equal_mark(dist, eq_mark);	//设置名值对的等于符号
	nvp_set_seperator(dist, sep_mark);	//设置名值对的分隔符号

	if(nvp_parse_string(dist, buf)<0)
	{
		printf(" can not parse packet \n");
		nvp_destroy(dist);
		return -1;
	}
	p_str = nvp_get_pair_str(dist, CMD_STR, NULL);
	if(p_str==NULL)
	{
		printf("can not find <cmd> string\n");
		return -1;
	}
	if(strncasecmp(p_str, M_TEST_DEV_RETURN ,strlen(M_TEST_DEV_RETURN))==0)
	{
		sprintf(ip_addr, "%s", inet_ntoa(ns->recv_addr.sin_addr));
		set_test_dev_stat(ip_addr, 0);
	}
	if(strncasecmp(p_str, M_CLEAR_DEV_RETURN ,strlen(M_CLEAR_DEV_RETURN))==0)
	{
		sprintf(ip_addr, "%s", inet_ntoa(ns->recv_addr.sin_addr));
		set_test_dev_stat(ip_addr, 0);
	}
	if(strncasecmp(p_str,M_FTP_UPDATE_DEV_RETURN,strlen(M_FTP_UPDATE_DEV_RETURN))==0)
	{
		sprintf(ip_addr, "%s", inet_ntoa(ns->recv_addr.sin_addr));
		set_test_dev_stat(ip_addr, 0);
	}
	if(strncasecmp(p_str, M_DEV_INFO,strlen(M_DEV_INFO))==0)
	{
		p_str = nvp_get_pair_str(dist, GUID, NULL);
		if(p_str==NULL)
		{
	//		printf("can not find <cmd> string\n");
			return -1;
		}
		for(i=0;i<MAX_TEST_DEV_NUM;i++)
		{
			if(th[i].node_th!=-1)
			{
				if(strncasecmp(p_str, th[i].dev_guid, strlen(p_str))==0)
				{
					printf("dev GUID = %s\n", th[i].dev_guid);
					memset(th[i].dev_ip, 0, sizeof(th[i].dev_ip));
					sprintf(th[i].dev_ip,"%s", inet_ntoa(ns->recv_addr.sin_addr));
					printf("ip address = %s\n", th[i].dev_ip);
					break;
				}
			}
		}
	}
	
	nvp_destroy(dist);
	return 0;
}
int get_net_dev_info(char *dev,int cmd,struct ifreq *req)
{//info_fd打开后不会关闭
        #define MAX_NET_IF      32//最多32个网络设备接口
        struct ifreq ifr[MAX_TEST_DEV_NUM];
        struct ifconf ifc;
        int  i,rc,ifs;
        int fd;
	int info_fd=-1;
        if((dev==NULL)||(req==NULL))
        {
                return -1;
        }
#if 1
        printf("size of struct ifreq.ifr_flags = %d \n", sizeof(ifr[1].ifr_flags));
        printf("size of struct ifreq.ifr_metric = %d \n", sizeof(ifr[1].ifr_metric));
        printf("size of struct ifreq.ifr_mtu = %d \n", sizeof(ifr[1].ifr_mtu));
        printf("size of struct ifreq.ifr_newname = %d \n", sizeof(ifr[1].ifr_newname));
        printf("size of struct ifreq.ifr_data = %d \n", sizeof(ifr[1].ifr_data));
       	printf("size of struct ifreq = %d \n", sizeof(struct ifreq));

        printf("size of struct  ifc.ifc_req = %d \n", sizeof( ifc.ifc_req));
        printf("size of struct  ifc.ifc_buf = %d \n", sizeof( ifc.ifc_buf));
        printf("size of struct  ifc.ifc_len = %d \n", sizeof( ifc.ifc_len));
        printf("size of struct ifconf = %d \n", sizeof(struct ifconf));
        return 0;   
#endif
        if(info_fd<0)
                info_fd = socket( AF_INET, SOCK_DGRAM, 0 );
        fd=info_fd;
        if(fd<0)
                return -2;
        ifc.ifc_len = sizeof (ifr);
        ifc.ifc_req = ifr;
        rc = ioctl( fd, SIOCGIFCONF, &ifc );

        if(rc<0)
        {
                perror("get_net_dev_info");
                //close(fd);
                return -3;
        }
        ifs = ifc.ifc_len / sizeof (struct ifreq);
        for(i=0;i<ifs;i++)
        {
                if(memcmp(ifr[i].ifr_name,dev,strlen(ifr[i].ifr_name))==0)
                {
                        rc = ioctl( fd, cmd, &ifr[i] );
                        memcpy((void*)req,(void*)&ifr[i],sizeof(struct ifreq));
                        //close(fd);
                        return 0;
                }
        }
        //close(fd);
        return -4;//没有找到    
}
    
void getkey_thread(void )
{
	char s[256];
	while(1)
	{
		fgets(s,sizeof(s),stdin);
		printf("%s\n", s);
		if(cmd_process(s, &net_port0)==0)
		{
			printf("send command %s\n", s);
		}
		memset(s, 0, sizeof(s));
	}
}
int guid_exchange_check(char* buf,multicast_sock *ns)
{
	NVP_TP *dist = NULL;
	const char *p_str=NULL;

	dist = nvp_create();					//创建一个存放名值对的缓冲区

	if(nvp_parse_string(dist, buf)<0)
	{
		printf(" can not parse packet \n");
	}
	
	p_str = nvp_get_pair_str(dist, CMD_STR, NULL);
	if(p_str==NULL)
	{
		printf("can not find <cmd> string\n");
		goto error_parse;
	}
	
	if(strncasecmp(p_str, M_SET_FACTORY ,strlen(M_SET_FACTORY)))
		goto error_parse;
	p_str = nvp_get_pair_str(dist, GUID, NULL);
	if(p_str==NULL)
		goto error_parse;
	if(strlen(p_str) <GUID_HEX_LEN)
	{
		printf(" get error guid %s len=%d \n", p_str, strlen(p_str) );
		goto error_parse;
	}
	
	memset(server_GUID_str, 0 , sizeof(server_GUID_str));
	memcpy(server_GUID_str, p_str, sizeof(server_GUID_str));
	svr_guid= hex2guid(server_GUID_str);
	set_server_id((char*)&svr_guid);
	printf("PUID change to %s\n",server_GUID_str);
	
error_parse:	
	nvp_destroy(dist);
	return -1;
}

int main(void)
{
	int len = 0;
	int i;
	int ret= -1;
	int max_fd=0;
	int thread_node=-1;
	fd_set read_fds;
	pthread_t  thread_key;
	struct GT_GUID guid;
	time_t time_cur;
	struct tm *tp;
	char buf[1024];
	char inp_buf[256];
 //       struct ifreq ifr;
//	get_net_dev_info("eth0", 1, &ifr);
//	exit(0);
	
	for(i=0;i<MAX_TEST_DEV_NUM;i++)
	{
		th[i].node_th = -1;
		th[i].test_stat = 0;
		memset(th[i].dev_guid,0,sizeof(th[i].dev_guid));
		memset(th[i].dev_ip,0,sizeof(th[i].dev_ip));
	}

	print_menu();
	memset(&net_port0, 0 ,sizeof(net_port0));
	memset(&net_port1, 0 ,sizeof(net_port1));

// creat two sockets to get data from server
#ifdef NET0
	init_net_port(HOSTNAME_ETH0, MULTI_CAST_PORT, &net_port0);
#endif
#ifdef NET1
	init_net_port(HOSTNAME_ETH1, MULTI_CAST_PORT, &net_port1);
#endif
//	 printf("socket fd = %d\n ", net_port0.fd);
	dev_guid = hex2guid((char*)test_dev);
	set_device_id((unsigned char*)&dev_guid);
//	set_port_id(&net_port0, (unsigned char*)server_GUID, (unsigned char*)&dev_guid);
//	set_port_id(&net_port1, (unsigned char*)server_GUID, (unsigned char*)&dev_guid);

	 len = sizeof(net_port0.send_addr);
	 memset(buf, 0, sizeof(buf));
	 memset(inp_buf, 0, sizeof(inp_buf));
	get_local_IP_address(buf);
	printf("\nserver IP address is %s \n",buf);	
 	memset(buf, 0, sizeof(buf));


	thread_node = GT_CreateThread(&thread_key, (void*) &getkey_thread, NULL);
	if(thread_node!=0)
	{
		perror("thread create");
	}
	
      	FD_ZERO(&read_fds);

//   	FD_SET(STDIN_FD,&read_fds);

	if(net_port0.fd>0)
    	{
        	FD_SET(net_port0.fd,&read_fds);
    	}
		if(net_port1.fd>0)
    	{
        	FD_SET(net_port1.fd,&read_fds);
    	}
      while(1)
        {
		if(max_fd<net_port0.fd)
		max_fd = net_port0.fd;
		if(max_fd<net_port1.fd)
      		max_fd = net_port1.fd;

//		printf("%d %d %d \n", net_port0.fd,net_port1.fd,STDIN_FD);
		select(max_fd+1,&read_fds,NULL,NULL,NULL);

#ifdef NET0
		if(FD_ISSET(net_port0.fd, &read_fds))	//接收网络端口1
		{
			ret = recv_dev_pkt(net_port0.fd, &net_port0.recv_addr, net_port0.self_id, 
				net_port0.target_id, buf, sizeof(buf), &net_port0.enc_type, 0);
			if(ret <0)
	      		{
				perror("recvfrom");
			}
			else	
			{
				printf("received %d bytes from %s\n", ret, inet_ntoa(net_port0.recv_addr.sin_addr));
				memcpy((char*)&guid,net_port0.target_id,sizeof(guid));
				memset(inp_buf,0,sizeof(inp_buf));
				guid2hex(guid, inp_buf);
				printf("source guid = %s\n",inp_buf);

				time_cur = time(NULL);
				tp = localtime(&time_cur);
				sprintf(inp_buf, "<%04d-%02d-%02d %02d:%02d:%02d>", 
					(1900+tp->tm_year),(1+tp->tm_mon), tp->tm_mday, 
					tp->tm_hour, tp->tm_min, tp->tm_sec);
				printf("%s\n" , inp_buf);
				printf("%s\n" , buf);
				/// lsk 2009 -5-14
//				guid_exchange_check(buf, &net_port0);
#ifdef _MULTI_TEST_
				check_test_dev_info(&net_port0, buf);
#endif
	//			printbuffer(buf, ret);
				memset(buf , 0 ,sizeof(buf));
			}	
		}
#endif
#ifdef NET1
  		if(FD_ISSET(net_port1.fd, &read_fds))	//接收网络端口2
		{
			ret = recv_dev_pkt(net_port1.fd, &net_port1.recv_addr, net_port1.self_id, 
				net_port1.target_id, buf, sizeof(buf), &net_port1.enc_type, 0);
			if(ret <0)
	      		{
				perror("recvfrom");
			}
			else	
			{
				printf("received %d bytes from %s\n", ret, inet_ntoa(net_port1.recv_addr.sin_addr));
				printf("%s\n" , buf);
//				check_test_dev_info(&net_port1, buf);
	//			printbuffer(buf, ret);
				memset(buf , 0 ,sizeof(buf));
			}	
		}
#endif

#if 0
		if(FD_ISSET(STDIN_FD,&read_fds))
		{
			ret = read(STDIN_FD, inp_buf, sizeof(inp_buf)-1);
			if(ret >0)
			{
				 printf("get command %s \n", inp_buf);
				 if(cmd_process(inp_buf, &net_port0)==0)
				 {
					printf("send command %s\n", inp_buf);
				 }
				 memset(inp_buf, 0, sizeof(inp_buf));
			}
			 else
			 {
				perror("read inputs:");
			 }
		}
        	FD_SET(STDIN_FD,&read_fds);
#endif
		if(net_port0.fd>0)
        	{
	        	FD_SET(net_port0.fd,&read_fds);
        	}
 		if(net_port1.fd>0)
        	{
	        	FD_SET(net_port1.fd,&read_fds);
        	}


       }
	exit(0);
}


