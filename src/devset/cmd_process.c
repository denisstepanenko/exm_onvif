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
#include <sys/vfs.h>	//lsk 2007 -5-31
#include "system_para_set.h"
#include "system_test.h"
#include "gt_dev_api.h"
#include "devinfo.h"
#include <nv_pair.h>
#include "communication.h"
#include "cmd_process.h"

#include <gt_errlist.h>
#include <guid.h>
#include "filelib.h"

#define NO_SERVER_IP 		501
#define SER_IP_ERR 			502
#define NO_PATH		 		503
#define NO_PASS		 		504
#define NO_USER		 		505
#define NO_SIZE		 		506
#define NO_PORT		 		507
#define DEV_IP_ERR 			508
#define DEV_NETMASK_ERR	509

#define GUID_HEX_LEN	16

static cmd_flag f = {
			.flag_update= 0,
			.flag_clear= 0,
			.flag_test= 0,
			};

static int result_updata = -1;
static const unsigned char	broadcast_addr[]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

int send_guid_setting_info(multicast_sock* ns, const char* cmd)
{
	int ret=0;
	int len = 0;
	const char* buf = NULL;
	NVP_TP *dist = NULL;
	if(cmd==NULL)
		return -1;
	dist = nvp_create();
	nvp_set_equal_mark(dist, eq_mark);		// set equal mark to "=="
	nvp_set_seperator(dist, sep_mark);		// set seperator to "&&"

	nvp_set_pair_str(dist, CMD_STR, ACK_PKT);	
	nvp_set_pair_str(dist, SEQ_ACK, ns->seq_num);	

	nvp_set_pair_int(dist, RET_VAL, 0);	
	nvp_set_pair_str(dist, ERR_STR,cmd);
	buf = nvp_get_string(dist);
	len = strlen(buf);
	if((len ==0)||(buf==NULL))
	{
		printf("error get nv_pair string \n");
		return -1;
	}
	if(virdev_get_virdev_number()==2)
	ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->recv_id, 
						(void*)buf , len, ns->enc_type, ns->flag);
	else
	ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id, 
						(void*)buf , len, ns->enc_type, ns->flag);
	nvp_destroy(dist);
	//// lsk 2009 -1-15
	if(ret>0)
	{
		printf("send ack guid setting %s\n", cmd);
		gtloginfo("send ack guid setting %s\n", cmd);
		test_log_info("send ack guid setting %s\n", cmd);
	}
	else
	{
		perror("send ack guid failed \n");
		gtloginfo("send ack guid failed \n");
		test_log_info("send ack guid failed \n");
	}
	return ret;
}
#if 1
/*
*****************************************************
*函数名称: result_rpt
*函数功能: 错误信息汇报
*输入： 	
*		ns 存放网络参数的数据结构
*		int error_code 错误代码
*		unsigned char info 错误信息
*输出： 	0 正确  负值错误
*修改日志：
*****************************************************
*/ 
int result_rpt(int err_code, unsigned char *info, multicast_sock* ns)
{
	int ret=0;
	int len = 0;
	const char* buf = NULL;
	NVP_TP *dist = NULL;
	if(ns->fd<=0)
	return 0;
	if(info==NULL)
	return 0;
	dist = nvp_create();
	nvp_set_equal_mark(dist, eq_mark);		// set equal mark to "=="
	nvp_set_seperator(dist, sep_mark);		// set seperator to "&&"

	nvp_set_pair_str(dist, CMD_STR, ACK_PKT);	// lsk 2007 -5 -31 cmd == ack
	nvp_set_pair_str(dist, SEQ_ACK, ns->seq_num);	//lsk 2007 -5-31

	nvp_set_pair_int(dist, RET_VAL, err_code);	//lsk 2007 -5-31
	nvp_set_pair_str(dist, ERR_STR,info);

	buf = nvp_get_string(dist);
	len = strlen(buf);
	if((len ==0)||(buf==NULL))
	{
		printf("error get nv_pair string \n");
		return -1;
	}
	if(virdev_get_virdev_number()==2)
	ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->recv_id, 
		(void*)buf , len, ns->enc_type, ns->flag);
	else
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
	int len=0;
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

	nvp_set_pair_str(dist, CMD_STR, RPT_PKT);		// cmd == process_report
	nvp_set_pair_str(dist, SEQ_ACK, ns->seq_num);	
	nvp_set_pair_int(dist, PROGRESS, prog);
	nvp_set_pair_str(dist, DETAIL_STR, info);

	buf = nvp_get_string(dist);
	len=strlen(buf);
	if((len ==0)||(buf==NULL))
	{
		printf("error get nv_pair string \n");
		return -1;
	}
	if(virdev_get_virdev_number()==2)
	ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->recv_id, 
		(void*)buf , len, ns->enc_type, ns->flag);
	else
	ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id, 
		(void*)buf , len, ns->enc_type, ns->flag);
	nvp_destroy(dist);
	return ret;
}
/*
*****************************************************
*函数名称: send_test_info
*函数功能: 发送测试进程参数和实时信息
*输入： 	
*		net_st 存放网络参数的数据结构
*		unsigned char* info	测试的实时信息
*输出： 	0 正确  错误返回-1
*修改日志：
*****************************************************
*/ 

int send_test_info(multicast_sock* ns, unsigned char* info)
{
	if(ns==NULL)
		return -1;
	if(info==NULL)
		return -1;
	if(virdev_get_virdev_number()==2)
	{
		if(ns->recv_id[GUID_LEN-1]==ns->self_id1[GUID_LEN-1])
		{
			send_test_report(ns, "测试结束", 100);
			result_rpt(0, info,ns);
			return 0;
		}
	}
	return -1;
}
#endif

/*
*****************************************************
*函数名称: send_ack_packet
*函数功能: 发送应答数据包
*输入： multicast_sock* ns  发送数据所需的参数
		int err_code 错误代码
*输出： 
*返回值: 错误代码0 正确 负值 错误
*修改日志：
*****************************************************
*/
int send_ack_packet(int err_code, multicast_sock *ns)
{
	int ret=0;
	int len = 0;
	const char* buf = NULL;
	NVP_TP *dist = NULL;
	dist = nvp_create();
	nvp_set_equal_mark(dist, eq_mark);		// set equal mark to "=="
	nvp_set_seperator(dist, sep_mark);		// set seperator to "&&"

	nvp_set_pair_str(dist, CMD_STR, ACK_PKT);	// lsk 2007 -5 -31 cmd == ack
	nvp_set_pair_str(dist, SEQ_ACK, ns->seq_num);	//lsk 2007 -5-31

	nvp_set_pair_int(dist, RET_VAL, err_code);	//lsk 2007 -5-31

	switch(err_code)
	{

		case 400:
			nvp_set_pair_str(dist, ERR_STR,"guid值设置失败");
		break;	

		case 401:
			nvp_set_pair_str(dist, ERR_STR,"设备类型设置失败");
		break;	

		case 402:
			nvp_set_pair_str(dist, ERR_STR,"生产批次设置失败");
		break;	

		case 403:
			nvp_set_pair_str(dist, ERR_STR,"板卡生产批次设置失败");
		break;	

		case 404:
			nvp_set_pair_str(dist, ERR_STR,"设备出厂时间设置失败");
		break;

		case 410:
			nvp_set_pair_str(dist, ERR_STR,"网口0的ip地址设置失败");
		break;

		case 411:
			nvp_set_pair_str(dist, ERR_STR,"网口0的子网掩码设置失败");
		break;

		case 412:
			nvp_set_pair_str(dist, ERR_STR,"网口0的mac地址设置失败");
		break;

		case 413:
			nvp_set_pair_str(dist, ERR_STR,"网口1的ip地址设置失败");
		break;

		case 414:
			nvp_set_pair_str(dist, ERR_STR,"网口1的子网掩码设置失败");
		break;

		case 415:
			nvp_set_pair_str(dist, ERR_STR,"网口1的mac地址设置失败");
		break;

		case 416:
			nvp_set_pair_str(dist, ERR_STR,"默认路由地址设置失败");
		break;

		case 417:
			nvp_set_pair_str(dist, ERR_STR,"设置设备的时间设置失败");
		break;
	
		case 500:
			nvp_set_pair_str(dist, ERR_STR,"清除测试文件失败");
		break;
		
		//lsk 2009 -1-6
		case 550:		
			nvp_set_pair_str(dist, ERR_STR,"格式化磁盘失败");
		break;
		//lsk 2007 -11-28
		case 600:		
			nvp_set_pair_str(dist, ERR_STR,"启动声音测试失败");
		break;

		case 601:		
			nvp_set_pair_str(dist, ERR_STR,"声音测试参数错误");
		break;
		case 602:		
			nvp_set_pair_str(dist, ERR_STR,"目前不支持语音功能");
		break;
		case 603:		
			nvp_set_pair_str(dist, ERR_STR,"无此设备");
		break;
		
		case 4112:
			nvp_set_pair_str(dist, ERR_STR,"升级文件名字格式错误");
		break;

		case 4113:
				nvp_set_pair_str(dist, ERR_STR,"无法登录ftp服务器");
		break;

		case 4114:
				nvp_set_pair_str(dist, ERR_STR,"解压文件失败");
		break;

		case 4115:
				nvp_set_pair_str(dist, ERR_STR,"设备存储空间不够，无法升级");
		break;

		case 4116:
				nvp_set_pair_str(dist, ERR_STR,"升级包与设备型号不匹配");
		break;

		case 4117:
				nvp_set_pair_str(dist, ERR_STR,"设备收到一个不支持的命令");
		break;
		case 5000:
				nvp_set_pair_str(dist, ERR_STR,"正在格式化磁盘无法重启");
		break;

		default:
			nvp_set_pair_str(dist, ERR_STR, "ok");	//lsk 2007 -5-31
			nvp_set_pair_str(dist, RET_VAL, "0");	//lsk 2007 -5-31
		break;			
	};
	buf = nvp_get_string(dist);
	len = strlen(buf);
	if((len ==0)||(buf==NULL))
	{
		printf("error get nv_pair string \n");
		return -1;
	}
/////lsk 2009-2-10 for gtvs3022
	if(virdev_get_virdev_number()==2)
	{
		if(memcmp(ns->recv_id,broadcast_addr,GUID_LEN)==0)
		{
			ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id, 
							(void*)buf , len, ns->enc_type, ns->flag);
			ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id1, 
							(void*)buf , len, ns->enc_type, ns->flag);
		}
		else
		ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->recv_id, 
			(void*)buf , len, ns->enc_type, ns->flag);
	}
	else
	ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id, 
						(void*)buf , len, ns->enc_type, ns->flag);

	nvp_destroy(dist);
	if(ret<0)		//// lsk 2009-1-15
	{
		printf("send ack packet error %d \n",ret );//// lsk 2009 -1-15
		gtlogerr("send ack packet error %d",ret);
		test_log_info("send ack packet error %d",ret);
	}
	else
	{
		printf("send ack packet %d \n",err_code);//// lsk 2007 -6-4
		gtloginfo("send ack packet %d \n",err_code);//// lsk 2009 -1-15
		test_log_info("send ack packet %d \n",err_code);//// 2009 -1-15
	}
	
	return ret;
}

/*
*****************************************************
*函数名称: handdle_set_fac
*函数功能: 设备产品参数设置命令的处理函数
*输入： multicast_sock* ns  发送数据所需的参数
*输出： 
*返回值: 错误代码0 正确 负值 错误
*修改日志：
*****************************************************
*/
/*
错误信息返回
错误号	出错信息
400		guid值设置失败
401		设备类型设置失败
402		生产批次设置失败
403		板卡生产批次设置失败
404		设备出厂时间设置失败
*/
int handdle_set_fac(NVP_TP *nv, multicast_sock * ns)
{
	int ret=0;
	int err_cnt=0;
	const char *p_str=NULL;
	struct tm ntime,otime;
	char* guid_buf=NULL;
	char buf[200];
	char cmd[100];

	// search for dev_tpye 
	p_str = nvp_get_pair_str(nv, DEV_TYPE, NULL);
	if(p_str!=NULL)
	{
		memset(buf,0,sizeof(buf));
		pass_dev_type(buf);
		ret = set_dev_type((unsigned char *)p_str);
		if(ret<0)
		{
			printf(" set dev type %s error ret =%d \n", p_str, ret);
			gtloginfo(" set dev type %s error ret =%d \n", p_str, ret);
			test_log_info(" set dev type %s error ret =%d \n", p_str, ret);
			send_ack_packet(401, ns);
			err_cnt++;
		}
		else	////lsk 2007 -11-1
		{
			printf(" set dev type %s\n", p_str);
			gtloginfo(" set dev type %s -> %s\n", buf, p_str);
			test_log_info(" set dev type %s -> %s\n", buf, p_str);
		}
	}
	////search for BATCH_SEQ 
	p_str = nvp_get_pair_str(nv, BATCH_SEQ, NULL);
	if(p_str!=NULL)
	{
		memset(buf,0,sizeof(buf));
		pass_dev_batch_seq(buf);
		ret = set_dev_batch_seq((unsigned char *)p_str);
		if(ret<0)
		{
			printf("set batch_seq %s error ret =%d \n", p_str, ret);
			gtloginfo("set batch_seq %s error ret =%d \n", p_str, ret);
			test_log_info("set batch_seq %s error ret =%d \n", p_str, ret);
			send_ack_packet(402, ns);
			err_cnt++;
		}
		else
		{
			printf("set batch_seq %s\n", p_str);
			gtloginfo("set batch_seq %s -> %s\n", buf, p_str);
			test_log_info("set batch_seq %s -> %s\n", buf, p_str);
		}
	}
	////search for BOARD_SEQ 
	p_str = nvp_get_pair_str(nv, BOARD_SEQ, NULL);
	if(p_str!=NULL)
	{
		memset(buf,0,sizeof(buf));
		pass_dev_board_seq(buf);
		ret = set_dev_board_seq((unsigned char *)p_str);
		if(ret<0)
		{
			printf("set board_seq %s error ret =%d \n", p_str, ret);
			gtloginfo("set board_seq %s error ret =%d \n", p_str, ret);
			test_log_info("set board_seq %s error ret =%d \n", p_str, ret);
			send_ack_packet(403, ns);
			err_cnt++;
		}
		else
		{
			printf("set board_seq %s\n", p_str);
			gtloginfo("set board_seq %s -> %s\n", buf, p_str);
			test_log_info("set board_seq %s -> %s\n", buf, p_str);
		}
	}
	////search for time of leave factory 
	p_str = nvp_get_pair_str(nv, LEAVE_FAC, NULL);
	if(p_str!=NULL)
	{
		memset(&otime,0,sizeof(otime));
		pass_produce_date(&otime);
		ret = set_produce_date((unsigned char *)p_str);
		if(ret<0)
		{
			printf("set time of leave_fac %s error ret =%d \n", p_str, ret);
			gtloginfo("set time of leave_fac %s error ret =%d \n", p_str, ret);
			test_log_info("set time of leave_fac %s error ret =%d \n", p_str, ret);
			send_ack_packet(404, ns);
			err_cnt++;
		}
		else
		{
			printf("set time of leave_fac %s\n", p_str);
			gtloginfo("set time of leave_fac %s\n", p_str);
			////lsk 2007-2-8
			memset(&ntime,0,sizeof(ntime));
			pass_produce_date(&ntime);
			test_log_info("leave_fac time change from %04d-%02d-%02d %02d:%02d:%02d  --> %04d-%02d-%02d %02d:%02d:%02d",
							otime.tm_year+1900,
							otime.tm_mon+1,
							otime.tm_mday,
							otime.tm_hour,
							otime.tm_min,
							otime.tm_sec,
							ntime.tm_year+1900,
							ntime.tm_mon+1,
							ntime.tm_mday,
							ntime.tm_hour,
							ntime.tm_min,
							ntime.tm_sec);
//			test_log_info("set time of leave_fac  to %s", p_str);////lsk 2007-2-8
		}
	}
	
	if(err_cnt==0)
		send_ack_packet(0, ns);
	////search for GUID		!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	p_str = nvp_get_pair_str(nv, GUID, NULL);
	if(p_str!=NULL)
	{
//		if(strlen(p_str) ==GUID_HEX_LEN)
		if(strlen(p_str) >=GUID_HEX_LEN)
		{
			if(virdev_get_virdev_number()==2)
			{
				if(ns->recv_id[GUID_LEN-1]==ns->self_id1[GUID_LEN-1])
				guid_buf =virdev_get_devid_str(1);
				else
				guid_buf =virdev_get_devid_str(0);
			}
			else
			guid_buf = get_devid_str();
			memset(buf,0,sizeof(buf));
			memcpy(buf,guid_buf,GUID_HEX_LEN);
			ret = set_dev_GUID((unsigned char *)p_str);
			if(ret<0)
			{
				printf("error set GUID %s ret = %d\n", p_str , ret);
				gtloginfo("error set GUID %s ret = %d\n", p_str , ret);
				test_log_info("error set GUID %s ret = %d\n", p_str , ret);
				send_ack_packet(400, ns);
				err_cnt++;
			}
			else
			{
				printf("set GUID %s\n", p_str);
				gtloginfo("set GUID %s -> %s\n", buf, p_str);
				test_log_info("set device GUID %s -> %s", buf, p_str);////lsk 2007-2-8
				if(virdev_get_virdev_number()==2)
				{
					if(ns->recv_id[GUID_LEN-1]==ns->self_id1[GUID_LEN-1])
					sprintf(cmd,"set GUID %s -> %s\n", buf, virdev_get_devid_str(1));
					else
					sprintf(cmd,"set GUID %s -> %s\n", buf, virdev_get_devid_str(0));
					send_guid_setting_info(ns,cmd);		

					if(ns->recv_id[GUID_LEN-1]==ns->self_id1[GUID_LEN-1])
					virdev_get_devid(1, ns->recv_id);
					else
					virdev_get_devid(0, ns->recv_id);
					send_guid_setting_info(ns,cmd);		
				}
				else
				{
					sprintf(cmd,"set GUID %s -> %s\n", buf, p_str);
					send_guid_setting_info(ns,cmd);		//// 2008 -10-24
					pass_dev_GUID(ns->self_id);			//// 2009-1-15
					send_guid_setting_info(ns,cmd);		//// 2009-1-15
				}
			if(err_cnt==0)
				send_ack_packet(0, ns);

//				return 0;		//lsk 2009 -1-7
			}
		
		}
		else
		{
			printf("not vailid GUID %s len =%d\n", p_str, strlen(p_str));
			gtloginfo("not vailid GUID %s \n", p_str);
			test_log_info("not vailid GUID %s \n", p_str);
			send_ack_packet(400, ns);
			err_cnt++;
		}
	}
	return 0;
}
/*
*****************************************************
*函数名称: reset_sys_para
*函数功能: 设备网络参数设置命令的处理函数
*输入： multicast_sock* ns  发送数据所需的参数
*输出： 
*返回值: 错误代码0 正确 负值 错误
*修改日志：
*****************************************************
*/
void    reset_sys_para(void)
{
        system("/ip1004/iniset /etc/conf/bak/gt_def.ini /conf/ip1004.ini\n");
        system("/ip1004/para_conv -s \n");
}

/*
*****************************************************
*函数名称: handdle_set_para
*函数功能: 设备网络参数设置命令的处理函数
*输入： multicast_sock* ns  发送数据所需的参数
*输出： 
*返回值: 错误代码0 正确 负值 错误
*修改日志：
*****************************************************
*/
/*
错误信息返回
错误号	出错信息
410		网口0的ip地址设置失败
411		网口0的子网掩码设置失败
412		网口0的mac地址设置失败
413		网口1的ip地址设置失败
414		网口1的子网掩码设置失败
415		网口1的mac地址设置失败
416		默认路由地址设置失败
417		设置设备的时间设置失败
*/

int handdle_set_para(NVP_TP *nv, multicast_sock * ns)
{
	int ret = -1;
	int err_cnt=0;
	const char *p_str=NULL;
	char buf[200];
		
	////search for ETH0_IP
	p_str = nvp_get_pair_str(nv, ETH0_IP, NULL);
	if(p_str!=NULL)
	{	
		memset(buf,0,sizeof(buf));
		pass_dev_IP(buf, ETH0);
		ret = set_sys_IP(ETH0 ,(unsigned char *)p_str);
		if(ret==0)
		{
			printf("set ETH0_IP %s \n", p_str);
			gtloginfo("set ETH0_IP  %s -> %s \n", buf , p_str);
			test_log_info("set ETH0_IP  %s -> %s \n", buf , p_str);
		}
		else
		{
			printf("not vailid ETH0_IP %s ret = %d\n", p_str,ret);
			gtloginfo("not vailid ETH0_IP %s ret = %d\n", p_str,ret);
			test_log_info("not vailid ETH0_IP %s ret = %d\n", p_str,ret);
			send_ack_packet(410, ns);
			err_cnt++;
		}
	}
	////search for ETH0_MASK
	p_str = nvp_get_pair_str(nv, ETH0_MASK, NULL);
	if(p_str!=NULL)
	{
		memset(buf,0,sizeof(buf));
		pass_dev_netmask(buf, ETH0);
		ret = set_sys_netmask(ETH0 ,(unsigned char *)p_str);
		if(ret==0)
		{
			printf("set ETH0_MASK %s \n", p_str);
			gtloginfo("set ETH0_MASK %s -> %s \n", buf,p_str);
			test_log_info("set ETH0_MASK %s -> %s \n",  buf, p_str);
		}
		else
		{
			printf("not vailid ETH0_MASK %s ret = %d\n", p_str,ret);
			gtloginfo("set ETH0_MASK error %s \n", p_str);
			test_log_info("set ETH0_MASK error %s \n", p_str);
			send_ack_packet(411, ns);
			err_cnt++;
		}
	}
	////search for ETH0_MAC
	p_str = nvp_get_pair_str(nv, ETH0_MAC, NULL);
	if(p_str!=NULL)
	{
		memset(buf,0,sizeof(buf));
		pass_dev_mac(buf,  ETH0);
		ret = set_sys_mac_addr(ETH0 ,(unsigned char *)p_str);
		if(ret==0)
		{
			printf("set ETH0_MAC %s \n", p_str);
			gtloginfo("set ETH0_MAC %s -> %s \n", buf , p_str);
			test_log_info("set ETH0_MAC %s -> %s \n", buf, p_str);
		}
		else
		{
			printf("not vailid ETH0_MAC %s ret = %d\n", p_str, ret);
			gtloginfo("not vailid ETH0_MAC %s ret = %d\n", p_str, ret);
			test_log_info("not vailid ETH0_MAC %s ret = %d\n", p_str, ret);
			send_ack_packet(412, ns);
			err_cnt++;
		}
	}
		////search for DEF_ROUTE
	p_str = nvp_get_pair_str(nv, DEF_ROUTE, NULL);
	if(p_str!=NULL)
	{
		memset(buf,0,sizeof(buf));
		pass_sys_gateway(buf);
		ret = set_sys_gateway((unsigned char *)p_str);
		if(ret==0)
		{
			printf("set DEF_ROUTE %s \n", p_str);
			gtloginfo("set DEF_ROUTE %s -> %s \n", buf, p_str);
			test_log_info("set DEF_ROUTE %s -> %s \n", buf, p_str);
		}
		else
		{
			printf("not vailid DEF_ROUTE %s ret = %d\n", p_str , ret);
			gtloginfo("not vailid DEF_ROUTE %s ret = %d\n", p_str , ret);
			test_log_info("not vailid DEF_ROUTE %s ret = %d\n", p_str , ret);
			send_ack_packet(416, ns);
			err_cnt++;
		}
	}
		////search for DEV_TIME
	p_str = nvp_get_pair_str(nv, DEV_TIME, NULL);
	if(p_str!=NULL)
	{
		printf("get dev time %s\n",p_str);
		memset(buf,0,sizeof(buf));
		pass_sys_date(buf);
		ret = set_sys_date((unsigned char *)p_str);
		if(ret==0)
		{
			printf("set DEV_TIME %s \n", p_str );
			gtloginfo("set DEV_TIME %s ->%s \n", buf, p_str);
			test_log_info("set DEV_TIME%s ->%s \n", buf , p_str );
		}
		else
		{
			printf("not vailid DEV_TIME %s ret = %d\n", p_str, ret);
			gtloginfo("not vailid DEV_TIME %s ret = %d\n", p_str, ret);
			test_log_info("not vailid DEV_TIME %s ret = %d\n", p_str, ret);
			send_ack_packet(417, ns);
			err_cnt++;
		}
	}
	if(2 == get_eth_num())
	{
		////search for ETH1_IP
		p_str = nvp_get_pair_str(nv, ETH1_IP, NULL);
		if(p_str!=NULL)
		{
			memset(buf,0,sizeof(buf));
			pass_dev_IP(buf,  ETH1);
			ret = set_sys_IP(ETH1 ,(unsigned char *)p_str);
			if(ret==0)
			{
				printf("set ETH1_IP %s \n", p_str);
				gtloginfo("set ETH1_IP %s -> %s \n", buf , p_str);
				test_log_info("set ETH1_IP %s -> %s\n", buf , p_str);
			}
			else
			{
				printf("not vailid ETH1_IP %s \n", p_str);
				gtloginfo("not vailid ETH1_IP %s \n", p_str);
				test_log_info("not vailid ETH1_IP %s \n", p_str);
				send_ack_packet(413, ns);
				err_cnt++;
			}
		}
		////search for ETH1_MASK
		p_str = nvp_get_pair_str(nv, ETH1_MASK, NULL);
		if(p_str!=NULL)
		{
			memset(buf,0,sizeof(buf));
			pass_dev_netmask(buf,  ETH1);
			ret = set_sys_netmask(ETH1 ,(unsigned char *)p_str);
			if(ret==0)
			{
				printf("set ETH1_MASK %s\n", p_str);
				gtloginfo("set ETH1_MASK %s -> %s\n",buf, p_str);
				test_log_info("set ETH1_MASK %s -> %s\n", buf , p_str);
			}
			else
			{
				printf("not vailid ETH1_MASK %s \n", p_str);
				gtloginfo("not vailid ETH1_MASK %s \n", p_str);
				test_log_info("not vailid ETH1_MASK %s \n", p_str);
				send_ack_packet(414, ns);
				err_cnt++;
			}
		}
		////search for ETH1_MAC
		p_str = nvp_get_pair_str(nv, ETH1_MAC, NULL);
		if(p_str!=NULL)
		{
			memset(buf,0,sizeof(buf));
			pass_dev_mac(buf,  ETH1);
			ret = set_sys_mac_addr(ETH1 ,(unsigned char *)p_str);
			if(ret==0)
			{
				printf("set ETH1_MAC %s\n", p_str);
				gtloginfo("set ETH1_MAC %s -> %s\n", buf , p_str);
				test_log_info("set ETH1_MAC %s -> %s\n", buf, p_str);
			}
			else
			{
				printf("not vailid ETH1_MAC %s \n", p_str);
				gtloginfo("not vailid ETH1_MAC %s \n", p_str);
				test_log_info("not vailid ETH1_MAC %s \n", p_str);
				send_ack_packet(415, ns);
				err_cnt++;
			}
		}
	}
//	printf("err_cnt=%d\n!!!!!!!!!!!!!!!!!!!!!!!",err_cnt); //// test !!!!
	if(err_cnt==0)
		send_ack_packet(0, ns);
	return 0;
}
/*
*****************************************************
*函数名称: stop_test
*函数功能: 设备测试停止命令处理函数
*输入： multicast_sock* ns  发送数据所需的参数
*			NVP_TP *nv 名值对存放的缓冲区指针
*输出：
*返回值: 错误代码0 正确 负值 错误
*修改日志：
*****************************************************
*/
int stop_test(NVP_TP *nv , multicast_sock *ns)
{
	unsigned int cmd;
	const char *p_str=NULL;
	BYTE cmdbuf[100];
	if((nv==NULL)||(ns==NULL))
	return -1;
//// lsk 2008-1-17 
	p_str = nvp_get_pair_str(nv, STOP_TEST, NULL);
	if(p_str!=NULL)
	{
		cmd = atoi(p_str);
		if(cmd ==1)
		{	
			memset(cmdbuf,0,sizeof(cmdbuf));
			sprintf(cmdbuf,"killall -9 %s \n", TEST_BD_PR);
			system(cmdbuf);
			memset(cmdbuf,0,sizeof(cmdbuf));
			sprintf(cmdbuf,"killall -9 %s \n", TEST_TG_PR);
			system(cmdbuf);
			memset(cmdbuf,0,sizeof(cmdbuf));
			sprintf(cmdbuf,"killall -9 %s \n", TEST_GTVM_PR);
			system(cmdbuf);
			system("/ip1004/swrbt \n");	
			gtloginfo("stop test device by %s", inet_ntoa(ns->recv_addr.sin_addr));
			test_log_info("stop test device by %s", inet_ntoa(ns->recv_addr.sin_addr));
			send_ack_packet(0, ns);	// lsk 2007 -6-27
		}
	}
	p_str = nvp_get_pair_str(nv, SYSTEM_RESET, NULL);
	if(p_str!=NULL)
	{
		cmd = atoi(p_str);
		if(cmd ==1)
		{
/////lsk 2007 -10 -31
//// lsk 2007 -4-26
			if(get_sys_flag(FORMAT_FLAG)==0)
			{
				gtloginfo("hard reset device by %s", inet_ntoa(ns->recv_addr.sin_addr));
				test_log_info("hard reset device by %s", inet_ntoa(ns->recv_addr.sin_addr));
				restart_hard();
				send_ack_packet(0, ns);	// lsk 2007 -6-27
			}
			else
			{
				send_ack_packet(5000, ns);	// lsk 2007 -6-27
			}
		}
	}
	p_str = nvp_get_pair_str(nv, PARA_RESET, NULL);
	if(p_str!=NULL)
	{
		cmd = atoi(p_str);
		if(cmd ==1)
		{
/////lsk 2007 -10 -31
			gtloginfo("reset device para by %s", inet_ntoa(ns->recv_addr.sin_addr));
			test_log_info("reset device para by %s", inet_ntoa(ns->recv_addr.sin_addr));
			reset_sys_para();
			send_ack_packet(0, ns);	//// lsk 2007 -6-27
			gtloginfo("hard reset device"); //// lsk 2007 -11-1
			test_log_info("hard reset device");
			restart_hard();
		}
	}
	return 0;
}
//static multicast_sock nt_tmp;
int test_thread_start(multicast_sock *ns, void *testprog,char *info, char *loginfo)
{
	int thread_node_t = -1;
	pthread_t thread_test;
	if(virdev_get_virdev_number()==2)
	{
		if((send_test_info(ns,info))==0)
		{
			printf("%s\n",info);
			return 0;
		}
///// lsk 2009-5-9
//		memset(&nt_tmp,0,sizeof(nt_tmp));
//		memcpy(&nt_tmp,ns,sizeof(nt_tmp));
//		thread_node_t = GT_CreateThread(&thread_test, testprog, &nt_tmp);					 
	}
//	else
	thread_node_t = GT_CreateThread(&thread_test, testprog, ns);					 
///////// lsk 2009 -2-26

	if(thread_node_t==0)
	{
		gtloginfo("%s",loginfo);
		test_log_info("%s",loginfo);
	}
	else 
	{
		gtloginfo("can not %s \n",loginfo);
		test_log_info("can not %s \n",loginfo);
	}
//	f.flag_test = 0;
//	f.flag_clear= 0;
	return 0;
}
/*
*****************************************************
*函数名称: handdle_test_dev
*函数功能: 设备测试和磁盘格式化的命令处理函数
*输入： multicast_sock* ns  发送数据所需的参数
*			NVP_TP *nv 名值对存放的缓冲区指针
*输出：
*返回值: 错误代码0 正确 负值 错误
*修改日志：
*****************************************************
*/
//// lsk 2009-5-9
static multicast_sock nt_tmp[4];

int handdle_test_dev(NVP_TP *nv , multicast_sock *ns)
{
	unsigned int cmd;
	const char *p_str=NULL;

	p_str = nvp_get_pair_str(nv, BOARD_TEST, NULL);
	if(p_str!=NULL)
	{
		cmd = atoi(p_str);
		if(cmd >0)	// lsk 2009 -5-22   cmd==1  -> cmd>0
		{
//// lsk 2009-5-22  
			set_sys_flag(TEST_IDE_FLAG, cmd-1);	// 0 测硬盘 1不测硬盘
//// lsk 2009-5-9
			memset(&nt_tmp[0],0,sizeof(multicast_sock));
			memcpy(&nt_tmp[0],ns,sizeof(multicast_sock));
			test_thread_start(&nt_tmp[0], (void*) &test_bd, 
			"板卡测试成功", "start board test");
//			test_thread_start(ns, (void*) &test_bd, "板卡测试成功", "start board test");
		}
	}
	p_str = nvp_get_pair_str(nv, TRIG_TEST, NULL);
	if(p_str!=NULL)
	{
		cmd = atoi(p_str);
		if(cmd ==1)
		{
//// lsk 2009-5-9
			memset(&nt_tmp[1],0,sizeof(multicast_sock));
			memcpy(&nt_tmp[1],ns,sizeof(multicast_sock));
			test_thread_start(&nt_tmp[1], (void*) &test_trig, 
			"该设备暂无端子", "start triger test");
//			test_thread_start(ns, (void*) &test_trig, "该设备暂无端子", "start triger test");
		}
	}
	p_str = nvp_get_pair_str(nv, FORMAT_DISK, NULL);
	if(p_str!=NULL)
	{
		cmd = atoi(p_str);
		if(cmd ==1)
		{
//// lsk 2009-5-22
			if(get_ide_flag()==0)	 //// 设备不带硬盘就返回ok
			{
				send_test_report(ns, "该设备不带硬盘", 100);	//// lsk 2009-5-22
				result_rpt(0, "格式化磁盘成功", ns);
			}
			else
			{
	//// lsk 2009-5-9
				memset(&nt_tmp[2],0,sizeof(multicast_sock));
				memcpy(&nt_tmp[2],ns,sizeof(multicast_sock));
				test_thread_start(&nt_tmp[2], (void*) &format_dev_disk, 
				"格式化磁盘成功", "start to format disk");
	//		test_thread_start(ns, (void*) &format_dev_disk, "格式化磁盘成功", "start to format disk");
			}
		}
	}
//	f.flag_test = 0;
	return 0;
}
#if 0
int handdle_test_dev(NVP_TP *nv , multicast_sock *ns)
{
	unsigned int cmd;
	const char *p_str=NULL;
	int thread_node_bd = -1;
	int thread_node_tg = -1;
	int thread_node_fm = -1;
	pthread_t	thread_format;
	pthread_t	thread_testbd;
	pthread_t	thread_testtg;

	p_str = nvp_get_pair_str(nv, BOARD_TEST, NULL);
	if(p_str!=NULL)
	{
		cmd = atoi(p_str);
		if(cmd ==1)
		{
			if((send_test_info(ns,"板卡测试成功"))==0)
			{
				printf("板卡测试成功\n");
				f.flag_test = 0;
				return 0;
			}
///////// lsk 2009 -2-26
			memset(&nt_tmp,0,sizeof(nt_tmp));
			memcpy(&nt_tmp,ns,sizeof(nt_tmp));
			thread_node_bd = GT_CreateThread(&thread_testbd, (void*) &test_bd, &nt_tmp);					 

			if(thread_node_bd==0)
			{
				gtloginfo("start board test \n");
//// lsk 2007 -4-26
				test_log_info("start board test \n");
			}
			else 
			{
				gtloginfo("can not start board test \n");
//// lsk 2007 -4-26
				test_log_info("can not start board test \n");
			}
			f.flag_test = 0;
			return 0;
		}
	}
	p_str = nvp_get_pair_str(nv, TRIG_TEST, NULL);
	if(p_str!=NULL)
	{
		cmd = atoi(p_str);
		if(cmd ==1)
		{
			if((send_test_info(ns,"端子测试成功"))==0)
			{
				printf("端子测试成功\n");
				f.flag_test = 0;
				return 0;
			}
///////// lsk 2009 -2-26
			memset(&nt_tmp,0,sizeof(nt_tmp));
			memcpy(&nt_tmp,ns,sizeof(nt_tmp));
			thread_node_tg = GT_CreateThread(&thread_testtg, (void*) &test_trig, &nt_tmp);
			if(thread_node_tg==0)
			{
				gtloginfo("start triger test \n");
//// lsk 2007 -4-26
				test_log_info("start triger test \n");
			}
			else 
			{
				gtloginfo("can not start trigertest \n");
//// lsk 2007 -4-26
				test_log_info("can not start trigertest \n");
			}
			f.flag_test = 0;
			return 0;
		}
	}
	p_str = nvp_get_pair_str(nv, FORMAT_DISK, NULL);
	if(p_str!=NULL)
	{
		cmd = atoi(p_str);
		if(cmd ==1)
		{
			if((send_test_info(ns,"格式化磁盘成功"))==0)
			{
				f.flag_test = 0;
				printf("格式化磁盘成功\n");
				return 0;
			}
			memset(&nt_tmp,0,sizeof(nt_tmp));
			memcpy(&nt_tmp,ns,sizeof(nt_tmp));
			thread_node_fm = GT_CreateThread(&thread_format, (void*) &format_dev_disk, &nt_tmp);					 
			if(thread_node_fm==0)
			{
				gtloginfo("start to format disk \n");
//// lsk 2007 -4-26
				test_log_info("can not start format disk  \n");
			}
			else 
			{
				gtloginfo("can not format disk \n");
//// lsk 2007 -4-26
				test_log_info("can not format disk \n");
			}
		}
	}
//	usleep(20000);
	f.flag_test = 0;
	return 0;
}
#endif

#define audio_file_path	"/hqdata/audiotest.wav"
static int audioflag=0;		/// lsk 2008 -10-17
//lc change audiodelay to 10 seconds
static int audiodelay = 10;//5*60
//static int audioduration = 0;

void start_audio_test(multicast_sock *ns )
{

    multicast_sock net_st;
    unsigned char buf[400];
    unsigned char info[400];
    int          audiochannel = 0;
    int          during = audiodelay;/*每个通道的播放时间,当前播放四通道*/
    int          sec_count = 0;


    if(audiodelay == 0)
    {
        return;
    }

    audioflag = 1;
    system("killall -9 watch_proc\n");
    system("killall -15 rtimage \n");
    system("killall -15 hdmodule \n");
    system("killall -9 ipmain \n");
    system("killall -15 diskman \n"); 					
    system("killall -15 encbox \n");					
    
    system("clear\n");
    sleep(1);

    
    memset(buf, 0, sizeof (buf));
    memcpy(&net_st,ns,sizeof(net_st));

    gtloginfo("start audio play test thread\n");
    test_log_info("start audio test thread\n");

    sprintf(info, "开始测试音频通道");
    send_test_report(ns, info, 0);

    printf("开始测试,时长%d,flag:%d\n",during*4,audioflag);
    
    while((sec_count < 4* during) && (audioflag ==1))
    {
        if(during*audiochannel == sec_count)
        {
            sprintf(info, "测试第%d个音频通道，周期%d秒", audiochannel, during);
            send_test_report(ns, info, audiochannel);
            system("killall -9 recplay");
            
            sprintf(buf, "/ip1004/recplay %d &", audiochannel);
            system(buf);
            audiochannel++;
        }   
        sleep(1);
        sec_count++;
    }


    system("killall -9 recplay\n");
    sprintf(info, "成功");
    send_test_report(ns, info, audiochannel);
    gtloginfo("audio test thread finished\n");
    test_log_info("audio test thread finished\n");
    send_ack_packet(0, ns);

    printf("测试正确\n");

    system("/ip1004/swrbt &");

    audioflag = 0;
    
    pthread_exit(NULL);
    
}
/*
*****************************************************
*函数名称: handdle_test_dev_audio
*函数功能: 设备声音系统测试的命令处理函数
*输入： multicast_sock* ns  发送数据所需的参数
*			NVP_TP *nv 名值对存放的缓冲区指针
*输出：
*返回值: 错误代码0 正确 负值 错误
*修改日志：
*****************************************************
*/

void stop_audio_test (void)
{


	//system("killall -9 recplay\n");
	gtloginfo("audio test stopped\n");
	test_log_info("audio test stopped\n");
	audioflag=0;
       sleep(1);
}

int handdle_test_dev_audio(NVP_TP *nv , multicast_sock *ns)
{
	int ret, delay, duration;
	char buf[200];

	if(virdev_get_virdev_number()==2)
	{
		if(ns->recv_id[GUID_LEN-1]==ns->self_id1[GUID_LEN-1])
		{
			send_ack_packet(0, ns);
			result_rpt(0,"该设备暂无语音" ,ns);
			return 0;
		}
	}
	////// lsk 2008-9-27    stop audio play test 
	ret = nvp_get_pair_int(nv, AUDIO_TEST_STOP, 0);
	if(ret==1)	//// stop 
	{
		stop_audio_test();
		send_ack_packet(0, ns);
		return 0;
	}
	///// end of change 
	delay = nvp_get_pair_int(nv, AUDIO_TEST_DELAY, 2);
	if(delay <0)
	{
		gtlogerr("audio test illegal para %s = %d \n", AUDIO_TEST_DELAY, delay);
		test_log_info("audio test illegal para %s = %d \n", AUDIO_TEST_DELAY, delay);
		send_ack_packet(601,ns);
		return -1;
	}
	duration= nvp_get_pair_int(nv, AUDIO_TEST_DURATION, 3600);	// lsk 2008-9-27 20->3600
	if(duration <0)
	{
		gtlogerr("audio test illegal para %s = %d \n", AUDIO_TEST_DURATION, duration);
		test_log_info("audio test illegal para %s = %d \n", AUDIO_TEST_DURATION, duration);
		send_ack_packet(601,ns);
		return -1;
	}
	if(audioflag==1)		////如果开始测试了，先结束再重新开始
		stop_audio_test();

       //audiodelay = delay;
       //audioduration = duration;
       printf("测试音频,delay:%d,duration:%d\n",delay,duration);
	memset(&nt_tmp[1],0,sizeof(multicast_sock));
	memcpy(&nt_tmp[1],ns,sizeof(multicast_sock));
	test_thread_start(&nt_tmp[1], (void*) &start_audio_test, 
	"测试音频", "启动音频测试");

       send_ack_packet(0, ns);
       return 0;

#if 0
	memset(buf,0,sizeof(buf));
	sprintf(buf, "/ip1004/recplay -d %d &\n", delay);
	ret = system(buf);
	ret = 0;
	if(ret==0)
	{
		gtloginfo("audio test start\n");
		test_log_info("audio test start\n");
		audioflag=1;
		send_ack_packet(0, ns);
		return 0;
	}
	gtloginfo("can not start audio test \n");
	test_log_info("can not start audio test \n");
	send_ack_packet(600, ns);
	return -1;
#endif    
}
/*
*****************************************************
*函数名称: handdle_test_state
*函数功能: 测试状态处理函数
*输入： 		multicast_sock* ns  发送数据所需的参数
*			NVP_TP *nv 名值对存放的缓冲区指针
*输出：
*返回值: 		错误代码0 正确 负值 错误
*修改日志：
*****************************************************
*/
int handdle_test_state(NVP_TP *nv , multicast_sock *ns)
{
	int i,ret;
	int len;
	unsigned int cmd;
	const char *p_str=NULL;
	test_info* tp=NULL;
	test_info t_st;
	unsigned char name_buf[BUF_LEN*2];
// lsk 2007 -6-12
	const char* buf = NULL;
	NVP_TP *dist = NULL;

	dist = nvp_create();
	nvp_set_equal_mark(dist, eq_mark);		// set equal mark to "=="
	nvp_set_seperator(dist, sep_mark);		// set seperator to "&&"

	nvp_set_pair_str(dist, CMD_STR, ACK_PKT);	// lsk 2007 -5 -31 cmd == ack
	nvp_set_pair_str(dist, SEQ_ACK, ns->seq_num);	//lsk 2007 -5-31

	nvp_set_pair_str(dist, ERR_STR, "ok");
	nvp_set_pair_str(dist, RET_VAL, "0");
////end of change

	p_str = nvp_get_pair_str(nv, SET_STATE, NULL);
	if(p_str!=NULL)		//setting test state
	{
		cmd = atoi(p_str);
		if(cmd==1)
		{
			for(i=0;i<TEST_STAGE;i++)
			{
				memset(&t_st, 0, sizeof(t_st));
				memset(name_buf, 0, sizeof(name_buf));
				sprintf(name_buf, "%s%d", TEST_STATE_VALUE, i+1);
				t_st.value = nvp_get_pair_int(nv, name_buf, 0);
				memset(name_buf, 0, sizeof(name_buf));
				sprintf(name_buf, "%s%d", TEST_STATE_NAME, i+1);
				p_str = nvp_get_pair_str(nv, name_buf, NULL);
				if(p_str!=NULL)
				{
					sprintf(t_st.name, "%s", p_str);
//					printf("%s %d %d\n", p_str, t_st.value, i);
////lsk 2007 -4-27
					gtloginfo("test state change : %s = %d",t_st.name, t_st.value);
					test_log_info("test state change : %s = %d",t_st.name, t_st.value);
					set_test_state_record(t_st.name, t_st.value, i);
				}
			}

		}
	}
	p_str = nvp_get_pair_str(nv, GET_STATE, NULL);
	if(p_str!=NULL)	// request test state 
	{
		cmd = atoi(p_str);
		if(cmd==1)
		{
			for(i=0;i<TEST_STAGE;i++)
			{
				tp = get_test_state_record(i);
				if(tp!=NULL)
				{
					memset(name_buf, 0, sizeof(name_buf));
					sprintf(name_buf, "%s%d", TEST_STATE_NAME, i+1);
		////lsk 2007 -6-12
//					nvp_set_pair_str(nv, name_buf, tp->name);
					nvp_set_pair_str(dist, name_buf, tp->name);
					memset(name_buf, 0, sizeof(name_buf));
					sprintf(name_buf, "%s%d", TEST_STATE_VALUE, i+1);
		////lsk 2007 -6-12
//					nvp_set_pair_int(nv, name_buf,  tp->value);
					nvp_set_pair_int(dist, name_buf,  tp->value);
				}
			}
		////lsk 2007 -4-27
			gtloginfo("%s get test state", inet_ntoa(ns->recv_addr.sin_addr));
			test_log_info("%s get test state get",  inet_ntoa(ns->recv_addr.sin_addr));

		}
	}
	p_str = nvp_get_pair_str(nv, CLR_STATE, NULL);
	if(p_str!=NULL)
	{
		cmd = atoi(p_str);
		if(cmd==1)
		{
			for(i=0;i<TEST_STAGE;i++)
			{
				tp = get_test_state_record(i);
				printf("backup test state %s=%d\n",tp->name,tp->value);
////lsk 2007 -4 -27
				gtloginfo("backup test state %s=%d", tp->name, tp->value);
				test_log_info("backup test state %s=%d", tp->name, tp->value);
			}
////lsk 2007 -4 -27
			gtloginfo("get clear test state command, reset test state information");
			test_log_info("get clear test state command, reset test state information");
			clear_test_state_record();
		}
	}
#if 0
	p_str = nvp_get_string(nv);
	len = strlen(p_str);
	if((len ==0)||(p_str==NULL))
	{
		printf("error get nv_pair string \n");
		return -1;
	}
	ret = send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id, 
						(void*)p_str , len, ns->enc_type, ns->flag);
	return 0;
#endif
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
*函数名称: update_report
*函数功能: 汇报升级结果
*输入： multicast_sock* ns  发送数据所需的参数
*			result_updata 结果数据
*输出： 
*修改日志：
*****************************************************
*/
void update_report(multicast_sock *ns)
{
	int len = 0;
	unsigned char buf[400];
	multicast_sock nt;
	memset (&nt, 0 ,sizeof(nt));
	memcpy(&nt, ns, sizeof(nt));
	// report updata results
	gtloginfo("update reporting...... \n");
	test_log_info("update reporting...... \n");
	while(is_bussy())	// waitting for other test finished
	{
		sleep(1);
	}

	while(f.flag_update);//等待升级结束获取结果

//// lsk 2007 -5 -31
//	sprintf(buf, "%s%s%s%s%s%s%s%s%s%s%s%d:%s,", sep_mark, CMD_STR, eq_mark, M_FTP_UPDATE_DEV_RETURN,
//	sep_mark, SEQ_STR, eq_mark,seq,sep_mark, RESULT_STR ,eq_mark, -result_updata, get_gt_errname(-result_updata));
	sprintf(buf, "%s%s%s%s%s%s%s%s%s%s%s%d%s%s%s%s",
	sep_mark, CMD_STR, eq_mark, ACK_PKT,
	sep_mark, SEQ_ACK, eq_mark,nt.seq_num,
	sep_mark, RET_VAL,eq_mark, -result_updata, 
	sep_mark, ERR_STR,eq_mark,get_gt_errname(-result_updata));
////end of change 

	len = strlen(buf);
	if(result_updata==0)
	{
		gtloginfo("update succeed \n");
//// lsk 2007 -4-26
		test_log_info("update succeed \n");
	}
	else
	{
		gtlogerr("update failed %s", get_gt_errname(-result_updata));
//// lsk 2007 -4-26
		test_log_info("update failed %s", get_gt_errname(-result_updata));
	}
//	send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id, 
//				(void*)buf , len, ns->enc_type, ns->flag);
////lsk 2007 -6-1
	send_dev_pkt(nt.fd, &nt.send_addr, nt.target_id, nt.self_id, 
				(void*)buf , len, nt.enc_type, nt.flag);

	result_updata = -1;
//	f.flag_update = 0;
	gtloginfo("update report  end\n");
	test_log_info("update report end\n");
	pthread_exit(NULL);
}
void print_update_para(update_dev *up_para)
{
	printf("path = %s\n", up_para->path);
	printf("user = %s\n", up_para->user);
	printf("password = %s\n", up_para->password);
	printf("server ip = %s\n", up_para->server);
	printf("port = %d\n", up_para->port);
	printf("size = %ld\n", up_para->size);
}
/*
*****************************************************
*函数名称: system_update
*函数功能: 系统升级任务
*输入： update_dev *up_para升级所需参数数据结构指针
*输出： 升级结果存放在result_updata 中
*修改日志：
*****************************************************
*/
void system_update(update_dev *up_para)
{
//// lsk 2007 -10-31
	gtloginfo("start updating ......\n");
	test_log_info("start updating ......\n");
	while(is_bussy())	// waitting for other test finished
	{
		sleep(1);
	}
	set_sys_flag(UPDATA_FLAG, 1);
//	print_update_para(up_para);		// test
	result_updata = update_system_ftp(up_para->user, up_para->password, 
		up_para->server, up_para->port, up_para->size, up_para->path);

	//restart killed threads
	system("tcpsvd -vE 0.0.0.0 21 ftpd /hqdata/ & \n");
	system("/ip1004/watch_proc &\n");	
	// clear flag
	set_sys_flag(UPDATA_FLAG, 0);
	f.flag_update = 0;
//// lsk 2007 -10-31
	gtloginfo("updating finished......\n");
	test_log_info("updating finished......\n");
	pthread_exit(NULL);
}
/*
*****************************************************
*函数名称: devide_ip_addr
*函数功能: 将IP 地址分解为整数
*输入： char*ip_addr* 需要分解的IP地址
*			存放解析后的ip地址的缓冲区指针
*输出： 
*返回值:
*修改日志：
*****************************************************
*/
void devide_ip_addr(char*ip_addr, ip_grp* ip_info)
{
	int i;
	unsigned char *p_str = NULL;
	unsigned char buf[100];

	memset(buf, 0 ,sizeof(buf));
	memcpy(buf, ip_addr,strlen(ip_addr));

	for(i=0;i<3;i++)
	{
		p_str=strrchr(buf,'.');
		if(p_str!=NULL)
		{
			p_str++;
			ip_info->grp[3-i]= atoi(p_str);
			printf("%d\n",ip_info->grp[3-i]);
			p_str--;
			memset(p_str, 0 , 4);
		}
	}
	ip_info->grp[0]= atoi(buf);
//	printf("%d\n",ip_info->grp[0]);
//	printf("%d.%d.%d.%d\n", ip_info->grp[0],ip_info->grp[1],ip_info->grp[2],ip_info->grp[3]);
}
/*
*****************************************************
*函数名称: handdle_update_dev
*函数功能: 启动软件升级的线程
*输入： multicast_sock* ns  发送数据所需的参数
*			NVP_TP *nv 名值对存放的缓冲区指针
*输出： 
*返回值: 错误代码0 正确，负值 错误
*修改日志：
*****************************************************
*/
int handdle_update_dev(NVP_TP *nv, multicast_sock *ns)
{
	int len = 0;
	int i;
	int thread_node_up = -1;
	int thread_node_rpt = -1;
	pthread_t	thread_up;
	pthread_t	thread_rpt;
	const char *p_str=NULL;
	 ip_grp	mask;
	 ip_grp	server;
	 ip_grp	dev_ip;
	
	update_dev    up_para;
	unsigned char ip_addr[50];
	unsigned char buf[100];
	unsigned char send_buf[200];

	memset (&mask, 0, sizeof(mask));
	memset (&server, 0, sizeof(server));
	memset (&dev_ip, 0, sizeof(dev_ip));
	memset (&up_para, 0, sizeof(up_para));

	memset (ip_addr, 0, sizeof(ip_addr));
	memset (buf, 0, sizeof(buf));
	memset (send_buf, 0, sizeof(send_buf));

	
//// get dev eth0 ip address
	pass_dev_IP(ip_addr, ETH0);
//// device ip address test
	p_str = strrchr(ip_addr, '.');
	if(p_str==NULL)
	{
		printf("dev IP is invalid %s \n", ip_addr);
		sprintf(buf,"%d:%s", DEV_IP_ERR, "设备IP错误,");
		goto errorret;
	}
	devide_ip_addr(ip_addr, &dev_ip);
	memset (ip_addr, 0 ,sizeof(ip_addr));
//// get dev eth0 net mask address
	pass_dev_netmask(ip_addr, ETH0);
//// device netmask address test  
	p_str=strrchr(ip_addr, '.');
	if(p_str==NULL)
	{
		printf("dev net mask is invalid %s \n", ip_addr);
		sprintf(buf,"%d:%s", DEV_NETMASK_ERR, "设备子网掩码错误,");
		goto errorret;
	}
	devide_ip_addr(ip_addr, &mask);
	memset (ip_addr, 0 ,sizeof(ip_addr));

//// find server ip address 
	p_str = nvp_get_pair_str(nv, UPDATE_SERVER, NULL);
	if(p_str!=NULL)
	{
		memcpy(ip_addr, p_str , strlen(p_str));
		p_str = strrchr(ip_addr, '.');
		if(p_str==NULL)
		{
			printf("server IP is invalid %s \n", ip_addr);
			sprintf(buf,"%d:%s", SER_IP_ERR, "服务器IP错误,");
			goto errorret;
		}
	}
	else
	{
		printf("缺少服务器IP \n");
		sprintf(buf,"%d:%s", NO_SERVER_IP,"缺少服务器IP,");
		goto errorret;
	}
	devide_ip_addr(ip_addr, &server);
	memset (ip_addr, 0 ,sizeof(ip_addr));

//// 找出子网掩码中的有效字段个数
////填充到服务器地址中
//// change server ip to a vailid ip to login FTP server
////server ip = 10.10.1.160
///例子: dev ip = 192.168.2.15  net mask = 255.255.255.0 server ip = 192.168.2.160
//// dev ip = 192.168.2.15  net mask = 255.255.0.0 server ip = 192.168.1.160
	for(i=0;i<4;i++)
	{
		if(mask.grp[i]!=0)
		{
			server.grp[i] = dev_ip.grp[i];
		}
	}
////将设置好的FTP服务器地址填充到数据结构中
	sprintf(up_para.server, "%d.%d.%d.%d",server.grp[0],server.grp[1],server.grp[2],server.grp[3]);
	////find update port
	p_str = nvp_get_pair_str(nv, UPDATE_PORT, NULL);
	if(p_str!=NULL)
	{
		up_para.port = atoi(p_str);
	}
	else
	{
		sprintf(buf,"%d:%s", NO_PORT,"缺少端口号,");
		goto errorret;
	}
	////find update file size
	p_str = nvp_get_pair_str(nv, UPDATE_SIZE, NULL);
	if(p_str!=NULL)
	{
		up_para.size= atoi(p_str);
	}
	else
	{
		sprintf(buf,"%d:%s", NO_SIZE, "缺少文件大小,");
		goto errorret;
	}

	////find update username
	p_str = nvp_get_pair_str(nv, UPDATE_USER, NULL);
	if(p_str!=NULL)
	{
		memcpy(up_para.user, p_str,strlen(p_str));
	}
	else
	{
		sprintf(buf,"%d:%s", NO_USER,"缺少用户名,");
		goto errorret;
	}
	////find update password
	p_str = nvp_get_pair_str(nv, UPDATE_PASS, NULL);
	if(p_str!=NULL)
	{
		memcpy(up_para.password, p_str,strlen(p_str));
	}
	else
	{
		sprintf(buf,"%d:%s", NO_PASS, "缺少密码,");
		goto errorret;
	}
	////find update path
	p_str = nvp_get_pair_str(nv, UPDATE_PATH, NULL);
	if(p_str!=NULL)
	{
		memcpy(up_para.path, p_str,strlen(p_str));
	}
	else
	{
		sprintf(buf,"%d:%s", NO_PATH,"缺少路径,");
		goto errorret;
	}
	//启动升级任务
	thread_node_up = GT_CreateThread(&thread_up, (void*) &system_update, &up_para);					 
	//启动升级结果汇报任务
	thread_node_rpt = GT_CreateThread(&thread_rpt, (void*) &update_report, ns);					 

	// set flag and start to update device
	return 0;
errorret:

//lsk 2007 -6-1
//	sprintf(send_buf, "%s%s%s%s%s%s%s%s", sep_mark, CMD_STR, eq_mark, M_FTP_UPDATE_DEV_RETURN,
//	sep_mark, RESULT_STR ,eq_mark, buf);

	sprintf(send_buf, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
	sep_mark, CMD_STR, eq_mark, ACK_PKT,
	sep_mark, SEQ_ACK, eq_mark, ns->seq_num,
	sep_mark, RET_VAL,eq_mark, "4111",
	sep_mark, ERR_STR,eq_mark, buf);
////end of change 

//// lsk 2007 -4-26
	gtlogerr("%s",buf);
	test_log_info("%s",buf);
	len = strlen(send_buf);
	printf("update failed: %s\n", send_buf);
	send_dev_pkt(ns->fd, &ns->send_addr, ns->target_id, ns->self_id, 
				(void*)send_buf , len, ns->enc_type, ns->flag);
	f.flag_update = 0;
	return -1;
}
#if 0
/*
*****************************************************
*函数名称: rm_file_select
*函数功能: 清除文件夹下除了file_path 下的文件的所有文件。
*输入：	const char* dir 文件的路径
		const char* file_path 保留文件的路径
		file_path 是 dir的一个子文件或者是子目录
*输出：
*返回值: 0 正确 负值错误
*修改日志：
*****************************************************
*/
int rm_file_select(const char* dir, const char* file_path)
{
	/*to be finished later*/
	return 0;
}
#endif
#if 0
int statfs(const char *path, struct statfs *buf);
       int fstatfs(int fd, struct statfs *buf);

DESCRIPTION
       statfs  returns  information  about a mounted file system.  path is the
       path name of any file within the mounted filesystem.  buf is a  pointer
       to a statfs structure defined as follows:

              struct statfs {
                 long    f_type;     /* type of filesystem (see below) */
                 long    f_bsize;    /* optimal transfer block size */
                 long    f_blocks;   /* total data blocks in file system */
                 long    f_bfree;    /* free blocks in fs */
                 long    f_bavail;   /* free blocks avail to non-superuser */
                 long    f_files;    /* total file nodes in file system */
                 long    f_ffree;    /* free file nodes in fs */
                 fsid_t  f_fsid;     /* file system id */
                 long    f_namelen;  /* maximum length of filenames */
                 long    f_spare[6]; /* spare for later */
              };
#endif
/*
*****************************************************
*函数名称: clear_testting_doc
*函数功能: 检查磁盘空间
*输入： 
*输出：
*返回值: 
*修改日志：
*****************************************************
*/
int check_disk_space(void)
{
	int ret=0;
	struct statfs buf;
	ret = statfs("/hqdata/", &buf);
//	ret = statfs(TEST_LOG_PATH, &buf);
	if(ret)
	{
		printf("error get hqdata info \n");
		gtloginfo("error get hqdata info ");
		test_log_info("error get hqdata info ");
	}
	else
	{
		printf("free blocks = %ld\n",buf.f_bfree);
		printf(" block size = %ld\n", (long)buf.f_bsize);
		printf("tatal  blocks = %ld\n",buf.f_blocks);
	}
	return ret;
}

/*
*****************************************************
*函数名称: clear_testting_doc
*函数功能: 清除设备上的测试文件并汇报清除结果
*输入： multicast_sock* ns  发送数据所需的参数
*输出：
*返回值: 
*修改日志：
*****************************************************
*/
void clear_testting_doc(multicast_sock *ns)
{
	char buf[40];
	int ret=0;
	multicast_sock nt;
//	struct statfs buf;
	memset (&nt,0,sizeof(nt));
	memcpy(&nt,ns,sizeof(nt));
///2007 -5-31
//	statfs(const char *path, struct statfs *buf);
//	check_disk_space();
///end of change 
//// lsk 2007 -4-26
	gtloginfo("get clear command to remove all testing files");
	test_log_info("get clear command to remove all testing files");
	while(is_bussy())	// waitting for other test finished
	{
		sleep(1);
	}
	set_sys_flag(CLEAN_FLAG, 1);
//	ret = system("mkdir /hqdata/index/old_log \n");	//lsk 2007 -6-12
//	ret = system("cp -f /log/gtlog.*  /hqdata/index/old_log \n");	//lsk 2007 -6-12
	
	rm_file_select("/log", TEST_LOG_PATH);
//	system("rm -rf /log/*\n");
////lsk 2007 -5-31
//	ret = system("rm -rf /hqdata/*\n");	////mod lsk 2007 -10-31

//// lsk 2009-5-22 有硬盘就格式化
	if(get_ide_flag())
	{
		if(access("/dev/hda1",F_OK)!=0)	//如果没安装硬盘
		{
			gtloginfo("ide disk did not installed");
			send_ack_packet(0, &nt);
		}
		else
	 	{
			sprintf(buf, "%s -B 1 \n",PROG_FMAT);	//// lsk 2009-2-27 不重启设备
			ret= system(buf);				//// lsk 2009-1-6	
			if(ret<0)						//// lsk 2009-1-6
			{
				gtlogerr("init disk failed ret =%d",ret);
				test_log_info("init disk failed ret=%d",ret);
				send_ack_packet(550, &nt);
			}
			else
			{
				gtloginfo(" init disk ok");
				test_log_info("init disk ok");
				send_ack_packet(0, &nt);
			}
		}
	}
	else		////  没有也无所谓
	send_ack_packet(0, &nt);

#if 0
	ret = system("/ip1004/unlockall \n");	//lsk 2007 -6-12
	result_report(CLEAN_FLAG, ns);
	printf("clear doc ret =%d\n",ret);
	check_disk_space();
////lsk 2007-10-31	
	if(ret ==0)
	send_ack_packet(0, &nt);
	else
	send_ack_packet(500, &nt);
#endif
////end of change		
	set_sys_flag(CLEAN_FLAG, 0);
	f.flag_clear= 0;
	pthread_exit(NULL);
}
/*
*****************************************************
*函数名称: handdle_clear_dev
*函数功能: 启动清除文件的线程
*输入： multicast_sock* ns  发送数据所需的参数
*输出： 
*返回值: 错误代码0 正确，负值 错误
*修改日志：
*****************************************************
*/
int handdle_clear_dev(multicast_sock *ns)
{

//// lsk 2009-5-9
	memset(&nt_tmp[3],0,sizeof(multicast_sock));
	memcpy(&nt_tmp[3],ns,sizeof(multicast_sock));
	test_thread_start(&nt_tmp[3], (void*)&clear_testting_doc, 
	"ok", "start clear testting doc");
//	test_thread_start(ns, (void*)&clear_testting_doc, "ok", "start clear testting doc");
	return 0;
}

#if 0
int handdle_clear_dev(multicast_sock *ns)
{
	int thread_node_clr = -1;
	pthread_t	thread_clear;
	thread_node_clr = GT_CreateThread(&thread_clear, (void*) &clear_testting_doc, ns);					 
	if(thread_node_clr==0)
	{
//		gtloginfo("start clear testting doc \n");
		test_log_info("start clear testting doc \n");
	}
	else 
	{
		gtloginfo("can not start clear testting doc  \n");
		test_log_info("can not start clear testting doc  \n");
	}
	return 0;
	
}
#endif
/*
*****************************************************
*函数名称: cmd_handdle
*函数功能: 对命令进行处理
*输入： multicast_sock* ns  发送数据所需的参数
			NVP_TP *nv 名值对存放的缓冲区指针
*输出：
*返回值: 错误代码0 正确，负值 错误
*修改日志：
*****************************************************
*/
int cmd_handdle(NVP_TP *nv ,  multicast_sock *ns)
{
	int ret = -1;
	const char *p_str=NULL;
	const char *seq_str=NULL;
	int cnt=0;
	if(nv == NULL)
	{
		return -1;
	}

	p_str = nvp_get_pair_str(nv, CMD_STR, NULL);
	if(p_str==NULL)
	{
		printf("can not find <cmd> string\n");
		return -1;
	}
	//// added by lsk 2007 -2-14
	seq_str = nvp_get_pair_str(nv, SEQ_STR,NULL);
	if(seq_str==NULL)
	{
		printf("can not find <seq> string\n");
		memset(ns->seq_num,0,sizeof(ns->seq_num));
		sprintf(ns->seq_num, "%s",  "0");
	}
	else
	{
		memset(ns->seq_num,0,sizeof(ns->seq_num));
		memcpy(ns->seq_num, seq_str,strlen(seq_str));
	}
	printf("get command %s seq_num = %s\n", p_str, ns->seq_num);
	if(strncasecmp(p_str, M_SEARCH_DEV ,strlen(M_SEARCH_DEV))==0)
	{
		printf("get command: search dev\n");
		ret = handdle_search_dev(ns,0);
	if(virdev_get_virdev_number()==2)
		ret = handdle_search_dev(ns,1);
#if 0	
		////// testing!!!!!
		ns->self_id[7]+=0x11;
//		ns->self_id[0]+=2;
//		ns->self_id[1]+=2;
		ret = handdle_search_dev(ns,1);
		ns->self_id[7]-=0x11;
//		ns->self_id[0]-=2;
//		ns->self_id[1]-=2;
		printf("get command: search dev\n");
		//////end of test 
#endif 
	}

	if(strncasecmp(p_str, M_SET_FACTORY ,strlen(M_SET_FACTORY))==0)
	{
		printf("get command: set dev factory info\n");
/// lsk 2007 -4 -26 
		gtloginfo("get command: set dev factory info %s", inet_ntoa(ns->recv_addr.sin_addr));
		test_log_info("get command: set dev factory info %s", inet_ntoa(ns->recv_addr.sin_addr));
		ret = handdle_set_fac(nv,ns);
		pass_dev_GUID(ns->self_id);	////lsk 2007 -11- 1 及时更新guid
		system("/ip1004/para_conv -m\n");
	}

	if(strncasecmp(p_str, M_SET_PARA ,strlen(M_SET_PARA))==0)
	{
		printf("get command: set dev para\n");
/// lsk 2007 -4 -26 
		gtloginfo("get command: set dev  para %s", inet_ntoa(ns->recv_addr.sin_addr));
		test_log_info("get command: set dev para %s", inet_ntoa(ns->recv_addr.sin_addr));
		ret = handdle_set_para(nv,ns);	
		system("/ip1004/para_conv -m\n");
	}
			
	if(strncasecmp(p_str, M_TEST_DEV ,strlen(M_TEST_DEV))==0)
	{
		printf("get command: test dev\n");
/// lsk 2007 -4 -26 
		gtloginfo("get command: test dev %s", inet_ntoa(ns->recv_addr.sin_addr));
		test_log_info("get command: test dev %s", inet_ntoa(ns->recv_addr.sin_addr));
		// 查询任务是否正在执行
		cnt=0;
#if 0
		while(is_bussy())		/// lsk 2009 -7-13
		{
			sleep(1);
			cnt++;
			if(cnt>=8)
				break;
		}
#endif		
		if((f.flag_test==0)&&(get_sys_flag(TEST_TG_FLAG)==0)&&
			(get_sys_flag(TEST_BD_FLAG)==0)&&(get_sys_flag(FORMAT_FLAG)==0))
		{
			f.flag_test = 1;
			ret = handdle_test_dev(nv ,ns);
			f.flag_test = 0;
		}

		stop_test(nv,ns);
	}
//// LSK 2007 -11-28
	if(strncasecmp(p_str, M_TEST_DEV_AUDIO ,strlen(M_TEST_DEV_AUDIO))==0)
	{
		printf("get command: test dev AUDIO \n");
		gtloginfo("get command: test dev AUDIO %s", inet_ntoa(ns->recv_addr.sin_addr));
		test_log_info("get command: test dev AUDIO %s", inet_ntoa(ns->recv_addr.sin_addr));
		ret = handdle_test_dev_audio(nv ,ns);
	}
	if(strncasecmp(p_str, M_FTP_UPDATE_DEV ,strlen(M_FTP_UPDATE_DEV))==0)
	{
		printf("get command:FTP update dev \n");
/// lsk 2007 -4 -26 
		gtloginfo("get command: FTP update dev %s", inet_ntoa(ns->recv_addr.sin_addr));
		test_log_info("get command: FTP update dev %s", inet_ntoa(ns->recv_addr.sin_addr));
		// 查询任务是否正在执行
		if(f.flag_update==0)
		{
			f.flag_update = 1;
			ret = handdle_update_dev(nv, ns);
		}
	}
			
	if(strncasecmp(p_str, M_CLEAR_DEV ,strlen(M_CLEAR_DEV))==0)
	{
		printf("get command: clear dev disk\n");
/// lsk 2007 -4 -26 
		gtloginfo("get command: clear dev disk %s", inet_ntoa(ns->recv_addr.sin_addr));
		test_log_info("get command: clear dev disk %s", inet_ntoa(ns->recv_addr.sin_addr));
		// 查询任务是否正在执行
		if(f.flag_clear==0)
		{
			f.flag_clear=1;
			ret = handdle_clear_dev(ns);
			f.flag_clear=0;
		}
	}
	if(strncasecmp(p_str, M_TEST_STATE,strlen(M_TEST_STATE))==0)
	{
		printf("get command: test state\n");
/// lsk 2007 -4 -26 
		gtloginfo("get command: test state %s", inet_ntoa(ns->recv_addr.sin_addr));
		test_log_info("get command: test state %s", inet_ntoa(ns->recv_addr.sin_addr));
		ret = handdle_test_state(nv, ns);
	}
	return ret;
}


