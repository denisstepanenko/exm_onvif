#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <file_def.h>
#include <gtthread.h>
#include <commonlib.h>
#include <sys/file.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "gt_dev_api.h"
#include <nv_pair.h>
#include <devinfo.h>
#include "multicast_ctl.h"

//#include "communication.h"
//const char server_GUID[8]={0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F};
static const char *eq_mark="==";	// equal mark in name value pair
static const char *sep_mark="&&";	// seperator in name value pair



/*
*****************************************************
*函数名称: pass_dev_GUID
*函数功能: 传递GUID
*输入： 
*输出： unsigned char *cmd  存放guid 的缓冲区
			错误代码0 正确 负值 错误
*修改日志：
*****************************************************
*/
int pass_dev_GUID(unsigned char *cmd)
{
	return (get_devid(cmd));
}

/*
*****************************************************
*函数名称: err_rpt
*函数功能: 错误信息汇报
*输入： 	
*		ns 存放网络参数的数据结构
*		int error_code 错误代码
*输出： 	0 正确  负值错误
*修改日志：
*****************************************************
*/ 
int err_rpt(int err_code, multicast_sock* ns)
{
	int ret=0;
	int len = 0;
	const char* buf = NULL;
	NVP_TP *dist = NULL;
	if(ns->fd<=0)
	return 0;
	dist = nvp_create();
	nvp_set_equal_mark(dist, eq_mark);		// set equal mark to "=="
	nvp_set_seperator(dist, sep_mark);		// set seperator to "&&"

	nvp_set_pair_str(dist, CMD_STR, ACK_PKT);	// lsk 2007 -5 -31 cmd == ack
	nvp_set_pair_str(dist, SEQ_ACK, ns->seq_num);	//lsk 2007 -5-31

	nvp_set_pair_int(dist, RET_VAL, err_code);	//lsk 2007 -5-31

	switch(err_code)
	{
		case 0:
			nvp_set_pair_str(dist, ERR_STR,"成功");
		break;
//////boad test
		case 11:
			nvp_set_pair_str(dist, ERR_STR,"编码器芯片1没有焊上");
		break;
		case 12:
			nvp_set_pair_str(dist, ERR_STR,"编码器1无数据输入/6410虚焊/C95144 虚焊/TW2834虚焊");
		break;
		case 13:
			nvp_set_pair_str(dist, ERR_STR,"编码器1工作不稳定丢数据");
		break;
		case 14:
			nvp_set_pair_str(dist, ERR_STR,"编码器2无数据输出/6410虚焊");
		break;
		case 21:
			nvp_set_pair_str(dist, ERR_STR,"编码器芯片2没有焊上");
		break;
		case 22:
			nvp_set_pair_str(dist, ERR_STR,"编码器2无数据输入/6410虚焊/C95144 虚焊/TW2834虚焊");
		break;
		case 23:
			nvp_set_pair_str(dist, ERR_STR,"编码器2工作不稳定丢数据");
		break;
		case 24:
			nvp_set_pair_str(dist, ERR_STR,"编码器2无数据输出/6410虚焊");
		break;
		case 31:
			nvp_set_pair_str(dist, ERR_STR,"TW2834 芯片没有焊上");
		break;
		case 41:
			nvp_set_pair_str(dist, ERR_STR,"没有找到IDE设备");
		break;
		case 42:
			nvp_set_pair_str(dist, ERR_STR,"磁盘没有格式化分区");
		break;
		case 43:
			nvp_set_pair_str(dist, ERR_STR,"打开测试文件失败");
		break;
		case 44:
			nvp_set_pair_str(dist, ERR_STR,"写测试文件失败");
		break;
		case 45:
			nvp_set_pair_str(dist, ERR_STR,"读测试文件失败");
		break;
		case 51:
			nvp_set_pair_str(dist, ERR_STR,"语音芯片没有焊上");
		break;
		case 61:
			nvp_set_pair_str(dist, ERR_STR,"模拟IIC驱动没加载");
		break;
		case 62:
			nvp_set_pair_str(dist, ERR_STR,"读写9903失败");
		break;
		case 71:
			nvp_set_pair_str(dist, ERR_STR,"USB 打开错误");
		break;
		case 72:
			nvp_set_pair_str(dist, ERR_STR,"写第一个USB设备出错");
		break;
		case 73:
			nvp_set_pair_str(dist, ERR_STR,"写第二USB设备出错");
		break;
		case 74:
			nvp_set_pair_str(dist, ERR_STR,"读第一个USB设备出错");
		break;
		case 75:
			nvp_set_pair_str(dist, ERR_STR,"读第二USB设备出错");
		break;
        
			
//////// trig test 
		case 101:
			nvp_set_pair_str(dist, ERR_STR,"设备主板上的51芯片未安装");
		break;
		case 102:
			nvp_set_pair_str(dist, ERR_STR,"测试板未连结或测试板上的51芯片未安装");
		break;
		case 103:
			nvp_set_pair_str(dist, ERR_STR,"测试板与设备主板上的51芯片未安装");
		break;
		case 104:
			nvp_set_pair_str(dist, ERR_STR,"与测试板串口通信出错");
		break;
		case 105:
			nvp_set_pair_str(dist, ERR_STR,"与watch_board串口通信出错");
		break;

		case 110:
			nvp_set_pair_str(dist, ERR_STR,"输入通道0失败");
		break;
		case 111:
			nvp_set_pair_str(dist, ERR_STR,"输入通道1失败");
		break;
		case 112:
			nvp_set_pair_str(dist, ERR_STR,"输入通道2失败");
		break;
		case 113:
			nvp_set_pair_str(dist, ERR_STR,"输入通道3失败");
		break;
		case 114:
			nvp_set_pair_str(dist, ERR_STR,"输入通道4失败");
		break;
		case 115:
			nvp_set_pair_str(dist, ERR_STR,"输入通道5失败");
		break;
		case 116:
			nvp_set_pair_str(dist, ERR_STR,"输出通道0失败");
		break;
		case 117:
			nvp_set_pair_str(dist, ERR_STR,"输出通道1失败");
		break;
		case 118:
			nvp_set_pair_str(dist, ERR_STR,"输出通道2失败");
		break;
		case 119:
			nvp_set_pair_str(dist, ERR_STR,"输出通道3失败");
		break;
		
////// init disk 
		case 300:
			nvp_set_pair_str(dist, ERR_STR,"磁盘格式化成功");
		break;
		case 301:
			nvp_set_pair_str(dist, ERR_STR,"没有硬盘或cf卡");
		break;
		case 302:
			nvp_set_pair_str(dist, ERR_STR,"找不到/dev/hda1节点");
		break;
		case 303:
			nvp_set_pair_str(dist, ERR_STR,"加载磁盘失败");
		break;
		case 304:
			nvp_set_pair_str(dist, ERR_STR,"磁盘分区失败");
		break;
		case 305:
			nvp_set_pair_str(dist, ERR_STR,"格式化磁盘失败");
		break;
		default:
			nvp_set_pair_str(dist, ERR_STR,"未知错误");
		break;
	}
	buf = nvp_get_string(dist);
	len = strlen(buf);
	if((len ==0)||(buf==NULL))
	{
		printf("error get nv_pair string \n");
		return -1;
	}
	ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id, 
						(void*)buf , len, ns->enc_type, ns->flag);
	nvp_destroy(dist);
	return ret;
}
/*
*****************************************************
*函数名称: send_test_report
*函数功能: 发送测试进程参数和实时信息
*输入： 	
*		net_st 存放网络参数的数据结构
*		unsigned char* info	测试的实时信息
*		int prog 进程0-100的整数
*输出： 	0 正确  错误退出程序
*修改日志：
*****************************************************
*/ 
 int send_test_report(multicast_sock* ns, unsigned char* info,int prog)
{
	const char* buf = NULL;
	NVP_TP *dist = NULL;
	int ret = 0;
	if(ns->fd<=0)
	return 0;

	dist = nvp_create();					//创建一个存放名值对的缓冲区
	if(dist==NULL)
	{
		printf("can not use nv_pair lib \n");
		exit(1);
	}
	nvp_set_equal_mark(dist, eq_mark);	//设置名值对的等于符号
	nvp_set_seperator(dist, sep_mark);		//设置名值对的分隔符号

//// lsk 2007 -6-1
#if 0
	nvp_set_pair_str(dist, CMD_STR, M_HD_PROG_RETURN);		// cmd == M_DEV_INFO
	nvp_set_pair_str(dist, SEQ_STR, ns->seq_num);	//lsk 2007 -2-14
	nvp_set_pair_int(dist, BDTEST_PROG, prog);
	nvp_set_pair_str(dist, BDTEST_INFO, info);
#endif
	nvp_set_pair_str(dist, CMD_STR, RPT_PKT);		// cmd == process_report
	nvp_set_pair_str(dist, SEQ_ACK, ns->seq_num);	
	nvp_set_pair_int(dist, PROGRESS, prog);
	nvp_set_pair_str(dist, DETAIL_STR, info);

////end of change 
	buf = nvp_get_string(dist);
	ret =send_multicast_pkt(ns, (char*)buf);
#if 0
	len = strlen(buf);
	if((len ==0)||(buf==NULL))
	{
		printf("error get nv_pair string \n");
		return -1;
	}
	ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id, 
						(void*)buf , len, ns->enc_type, ns->flag);
#endif
	nvp_destroy(dist);
//	printf("ret = %d errno = %d %s\n", ret, errno, strerror(errno));

	return ret;

	return 0;
}

/*
*****************************************************
*函数名称: init_dev_net_port
*函数功能: 初始化网络连接
*输入： 	
*		net_st 存放网络参数的数据结构
*输出： 	0 正确  错误退出程序
*修改日志：
*****************************************************
*/ 
 int init_dev_net_port(multicast_sock* net_st)
{
	int ret = -1;
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
//	memset(&net_st, 0 ,sizeof(&net_st));
#if 1
	net_st->enc_type = 0 ;
	memset(net_st->self_id, 0 , sizeof(net_st->self_id));
//	memset(net_st->target_id, 0 , sizeof(net_st->target_id));
	//
	bzero(&net_st->send_addr,sizeof(net_st->send_addr));
	bzero(&net_st->recv_addr,sizeof(net_st->recv_addr));
#endif
	if((strlen(net_st->hostname))!=0)
	{
		net_st->recv_addr.sin_family = AF_INET;
		net_st->recv_addr.sin_addr.s_addr = inet_addr(net_st->hostname); //htonl(INADDR_ANY);
		net_st->recv_addr.sin_port =htons(net_st->multi_port);
	}

#if 0
	//bind socket 
	if(bind(net_st->fd, (struct sockaddr *)&net_st->recv_addr, 
		sizeof(net_st->recv_addr)) == -1)
	{
		perror("bind error");
		goto error_handdle;	// lsk 2006 -12 -28
	}
#endif
	//setsocketopt
	net_st->loop = 1;
	if(setsockopt(net_st->fd,SOL_SOCKET,SO_REUSEADDR,	
		&net_st->loop,sizeof(net_st->loop))<0)
	{
		perror("setsocketopt:SO_REUSEADDR");
		goto error_handdle;	// lsk 2006 -12 -28
	}
	// do not receive packet sent by myself
	net_st->loop = 0;	// test loop =0 no cycle , loop =1 cycle enable  
	if(setsockopt(net_st->fd,IPPROTO_IP,IP_MULTICAST_LOOP, 
		&net_st->loop,sizeof(net_st->loop))<0)
	{
		perror("setsocketopt:IP_MULTICAST_LOOP");
		goto error_handdle;	// lsk 2006 -12 -28
	}
//join one multicast group
	if((strlen(net_st->hostname))!=0)
	{
		net_st->mreq.imr_multiaddr.s_addr = inet_addr(net_st->hostname);
		net_st->mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if(net_st->mreq.imr_multiaddr.s_addr == -1)
		{
			perror("not a legal multicast address!");
			goto error_handdle;	// lsk 2006 -12 -28
		}
// lsk 2006 -12 -28
#if 1
		if(setsockopt(net_st->fd,IPPROTO_IP,IP_ADD_MEMBERSHIP, 
			&net_st->mreq , sizeof(net_st->mreq))<0)
		{
			perror("setsockopt:IP_ADD_MEMBERSHIP");
			goto error_handdle;	// lsk 2006 -12 -28
		}
#endif
	}

	// set GUID as self ID
	ret = pass_dev_GUID(net_st->self_id);	//test
	if(ret!=DEV_GUID_BYTE)
	{
		printf("can not get GUID ret = %d \n", ret);
//		printbuffer(net_st->self_id , DEV_GUID_BYTE);
		goto error_handdle;	// lsk 2006 -12 -28
//		return -1;
	}
	//// test server guid
//	memcpy(net_st->target_id, server_GUID, sizeof(server_GUID));
	memcpy((void*)&net_st->send_addr, (void*)&net_st->recv_addr,
		sizeof(net_st->recv_addr));
//	memcpy(net_st->hostname, net_st->host_name,strlen(net_st->host_name));
//	sprintf(net_st->seq_num, "%s", "0");
	
	 return 0;
error_handdle: 
	close(net_st->fd);
	net_st->fd = -1;
	return -1;
}
/*
*****************************************************
*函数名称: send_test_cmd
*函数功能: 发送组波数据
*输入： host_name host_name1 两个组播地址
*			port 组播端口号
*			net_st 存放网络参数的数据结构
*输出： 0 正确  错误退出程序
*修改日志：
*****************************************************
*/ 
int send_multicast_pkt(multicast_sock* net_st,  char * buf)
{
	int ret = -1;
	int len = 0 ;
	len = strlen(buf);

	printf("send to %s\n", inet_ntoa(net_st->send_addr.sin_addr) );
	ret =send_dev_pkt(net_st->fd, &net_st->send_addr, net_st->target_id,
	net_st->self_id, buf, len , net_st->enc_type, net_st->flag);
	if(ret<0)
	{
		printf("ret =%d %s\n", ret, strerror(errno));
	}
	return ret;
}
/*
*****************************************************
*函数名称: result_report
*函数功能: 测试结果汇报
*输入： multicast_sock *ns 网络参数数据结构
*			int index	汇报的信息索引
*输出：
*返回值: 0正确负值表示错误
*修改日志：
*****************************************************
*/ 
//lsk 2007 -6-1 
int result_report(int index, multicast_sock *ns)
{
	return (err_rpt(index, ns));
}

#if 0
int result_report(int index, multicast_sock *ns)
{
	int len = 0;
	int ret = -1;
	dictionary	*	ini = NULL;
	char inibuf[100];
	char *s=NULL;
	unsigned char buf[1024];

	memset(buf, 0 , sizeof(buf));
	memset(inibuf, 0 , sizeof(inibuf));

	if(ns==NULL)
	{
		return -1;
	}
	if(ns->fd <=0)
		return 0;
	switch(index)
	{			
		case TEST_TG_FLAG:
		ini = iniparser_load(TEST_TG_RESULT);
		if (ini==NULL) {
			fprintf(stderr, "cannot parse file [%s]", TEST_TG_RESULT);
			return -1;
		}
		sprintf(inibuf, "%s:%s", TRIG_NODE, REPORT);
		s = iniparser_getstr(ini, inibuf);
		if(s!=NULL)
		{
			sprintf(buf, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
				sep_mark, CMD_STR, eq_mark,M_TEST_DEV_RETURN,
				sep_mark, SEQ_STR, eq_mark,ns->seq_num,
				sep_mark, TEST_OPT, eq_mark, TRIG_TEST,
				sep_mark, RESULT_STR ,eq_mark, s);
			iniparser_freedict(ini);
		}
		else
		{
			printf("can not find [%s]:%s in %s\n", TRIG_NODE, REPORT,TEST_TG_RESULT);
			iniparser_freedict(ini);
			return -1;
		}
			break;
			
		case TEST_BD_FLAG:
		ini = iniparser_load(TEST_BD_RESULT);
		if (ini==NULL) {
			fprintf(stderr, "cannot parse file [%s]", TEST_BD_RESULT);
			return -1;
		}
		sprintf(inibuf, "%s:%s", BOARD_NODE, REPORT);
		s = iniparser_getstr(ini, inibuf);
		if(s!=NULL)
		{
			sprintf(buf, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
				sep_mark, CMD_STR, eq_mark,M_TEST_DEV_RETURN,
				sep_mark, SEQ_STR, eq_mark,ns->seq_num,
				sep_mark, TEST_OPT, eq_mark, BOARD_TEST,
				sep_mark, RESULT_STR ,eq_mark, s);
			iniparser_freedict(ini);
		}
		else
		{
			printf("can not find [%s]:%s in %s\n", BOARD_NODE, REPORT,TEST_BD_RESULT);
			iniparser_freedict(ini);
			return -1;
		}
			break;
			
		case FORMAT_FLAG:
		ini = iniparser_load(FMAT_INFO);
		if (ini==NULL) {
			fprintf(stderr, "cannot parse file [%s]", FMAT_INFO);
			return -1;
		}
		sprintf(inibuf, "%s:%s", FMAT_NODE, REPORT);
		s = iniparser_getstr(ini, inibuf);
		if(s!=NULL)
		{
			sprintf(buf, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
				sep_mark, CMD_STR, eq_mark,M_TEST_DEV_RETURN,
				sep_mark, SEQ_STR, eq_mark,ns->seq_num,
				sep_mark, TEST_OPT, eq_mark, FORMAT_DISK,
				sep_mark, RESULT_STR ,eq_mark, s);
			iniparser_freedict(ini);
		}
		else
		{
			printf("can not find [%s]:%s in %s\n", FMAT_NODE, REPORT,FMAT_INFO);
			iniparser_freedict(ini);
			return -1;
		}

			break;
#if 0
		case CLEAN_FLAG:
			sprintf(buf, "%s%s%s%s%s%s%s%d:%s", sep_mark, CMD_STR, eq_mark, M_CLEAR_DEV_RETURN,
			sep_mark, RESULT_STR ,eq_mark, CLR_OK, clear_OK);
			break;
#endif		
		default:
			printf("can not report result : get error index \n");
			return -1;
			break;
	}

	len = strlen(buf);
	ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id, 
						(void*)buf , len, ns->enc_type, ns->flag);
	return ret;
}
#endif

