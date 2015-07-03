/*
**************************************************************
文件名称：communication.h
编写者： lsk
编写日期：2006-9-12
简要描述：声明通讯协议的数据结构和变量
修改记录：
修改日志：
***************************************************************
*/
#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_
#define BUF_LEN  100
//#define GUID_HEX_LEN
#define DATA_BUF_LEN 	4*1024
#define CMD_BUF_LEN 	4*1024

#include <devinfo_virdev.h>
//#define TEST224
#ifdef  TEST224
#define HOSTNAME_ETH0 		"224.0.0.1"
#define DEF_MULT_ADDR0		"224.0.0.0"
#else
#define DEF_MULT_ADDR		"224.0.0.0"	//lsk 224->225 2007 -6-13
#define HOSTNAME_ETH0 		"225.0.0.1"
#define DEF_MULT_ADDR0		"225.0.0.0"
#endif
#define HOSTNAME_ETH1 		"226.0.0.1"
#define DEF_MULT_ADDR1		"226.0.0.0"
#define DEF_MULT_NETMASK	"255.0.0.0"

#define MULTI_CAST_PORT 3310

#define CMD_STR 	"cmd"
#define SEQ_STR 	"seq"

#define ACK_PKT	"ack"
#define SEQ_ACK	"seq_ack"
#define RET_VAL	"ret"
#define ERR_STR	"err"

#define GUID					"guid"
#define DEV_TYPE  			"dev_type_str"
#define BATCH_SEQ			"batch_seq"
#define BOARD_SEQ 			"board_seq"
#define LEAVE_FAC 			"leave_fac"

#define ETH0_IP				"eth0_ip"
#define ETH0_MASK	 		"eth0_mask"
#define ETH0_MAC	 		"eth0_mac"
#define ETH1_IP				 "eth1_ip"
#define ETH1_MASK			"eth1_mask"
#define ETH1_MAC			"eth1_mac"
#define DEF_ROUTE			"def_route"
#define DEV_TIME				"dev_time"

#define BOARD_TEST  			"board_test"
#define TRIG_TEST  			"trig_test"
#define FORMAT_DISK  		"format_disk"
#define SYSTEM_RESET		"system_reset"
#define PARA_RESET			"para_reset"
#define STOP_TEST			"stop_test"	//lsk 2008-1-17

#define UPDATE_MODE			"mode"
#define UPDATE_SERVER 		"server"
#define UPDATE_PORT			"port"
#define UPDATE_PATH 			"path"
#define UPDATE_USER			"user"
#define UPDATE_PASS			"pass"
#define UPDATE_SIZE			"size"

#define INST_NAME	 		"inst_name"
#define ETH_NUM 				"eth_num"
#define FIRMWARE	 		"firmware"
#define CMD_PORT 			"cmd_port"
#define IMAGE_PORT	 		"image_port"
#define AUDIO_PORT	 		"audio_port"
#define FTP_PORT	 			"ftp_port"
#define TELNET_PORT	 		"telnet_port"
#define WEB_PORT	 		"web_port"
#define COM0_PORT	 		"com0_port"
#define COM1_PORT	 		"com1_port"
#define UPDATING	 			"updating"
#define FORMATTING	 		"formatting"
#define BOARD_TESTING 		"board_testing"
#define TRIG_TESTING 		"trig_testing"

// command definition
// lsk 2006-12-14 change code to string 
// search device   
//#define M_SEARCH_DEV 			"0010"
#define M_SEARCH_DEV 			"search_dev"
// set factory infomation
//#define M_SET_FACTORY 			"0020"
#define M_SET_FACTORY 			"set_factory"

////产品信息相关数据
typedef struct 
{
	BYTE guid[BUF_LEN];			//设备GUID
	BYTE guid1[BUF_LEN];			//设备GUID1  for gtvs3022
	BYTE dev_type[BUF_LEN];		//设备类型
	BYTE batch_seq[BUF_LEN];		//设备批次编号
	BYTE board_seq[BUF_LEN];		//主板编号
	BYTE leave_fac[BUF_LEN];		//出厂时间
} set_factory;

typedef struct 
{
	BYTE year[5];
	BYTE mon[3];
	BYTE day[3];
	BYTE hour[3];
	BYTE min[3];
	BYTE sec[3];
} sys_time_set;

// set system parameters
#define M_SET_PARA 				"set_para"
////支持两个网络接口的设备
//// 网卡的物理地址，网关，IP地址，子网掩码
typedef struct 
{
	BYTE eth0_ip[BUF_LEN];		
	BYTE eth0_mask[BUF_LEN];	
	BYTE eth0_mac[BUF_LEN];		
	BYTE eth1_ip[BUF_LEN];		
	BYTE eth1_mask[BUF_LEN];	
	BYTE eth1_mac[BUF_LEN];		
	BYTE def_route[BUF_LEN];		
	BYTE dev_time[BUF_LEN];		
}set_para;


// start system test 
#define M_TEST_DEV 				"test_dev"
#if 0
typedef struct {
	WORD	board_test;
	WORD	trig_test;
	WORD 	format_disk;
}test_dev;
#endif
//////// lsk 2009-2-18
#define RPT_PKT		"process_report"
#define PROGRESS		"progress"
#define DETAIL_STR	"detail"


#define M_TEST_DEV_AUDIO		"test_dev_audio"
#define AUDIO_TEST_DELAY		"delay"
#define AUDIO_TEST_DURATION	"duration"
#define AUDIO_TEST_STOP		"stop"

// start system updating
#define M_FTP_UPDATE_DEV 		"ftp_update_dev"
////系统升级的相关参数 
typedef struct 
{
	int  	  mode;					//升级模式
	char   server[BUF_LEN];		//服务器的IP地址
	char   user[BUF_LEN];			//登陆用户名
	char   password[BUF_LEN];		//登陆密码
	char   path[BUF_LEN];			// 升级文件路径
	int     	port;					// 服务器端口号
	unsigned long int  	  size;					// 升级文件的大小
} update_dev;

// clear unused docs
#define M_CLEAR_DEV 			"clear_dev"
// set test state
#define M_TEST_STATE			"test_state"
#define SET_STATE				"set_state"
#define GET_STATE				"get_state"
#define CLR_STATE				"clear_state" 
#define TEST_STATE_NAME		"test_state_name"
#define TEST_STATE_VALUE		"test_state_value"
//#define TSET_STATE_NODE			"test_state"
// return device information  to server
#define M_DEV_INFO 				"dev_info"

//// lsk 2009-2 10 for gtvs 3022 
typedef struct 
{
	WORD cmd_port;
	WORD image_port;
	WORD audio_port;
	WORD ftp_port;
	WORD telnet_port;
	WORD web_port;
	WORD com0_port;
	WORD com1_port;
	BYTE   inst_name[BUF_LEN];
}DEV_DIFF_PARA;

typedef struct 
{
	set_factory fac;
	WORD eth_num;
	set_para para;
	BYTE   firmware[BUF_LEN];
	BYTE   HD_serial_NO[BUF_LEN];
	DEV_DIFF_PARA dev;
	DEV_DIFF_PARA dev1;
	WORD updating;
	WORD formatting;
	WORD board_testing;
	WORD trig_testing;
	WORD clearing;
	WORD test_ide;
}dev_info;


// return teset results to server
#define M_TEST_DEV_RETURN 			"test_dev_return"
#define RESULT_STR					"result_str"
#define TEST_OPT						"test_opt"
// return updata results to server
#define M_FTP_UPDATE_DEV_RETURN 	"ftp_update_dev_return"

//return clear command results to server
#define M_CLEAR_DEV_RETURN 		"clear_dev_return"

//return test progress to server 
#define M_TEST_STATE_RETURN 		"test_state_return"
#define M_HD_PROG_RETURN			"hdtest_prog_return"

#define BDTEST_PROG		"bdtest_prog"
#define TRIGTEST_PROG	"trigtest_prog"
#define FDISK_PROG		"fdisk_prog"
#define BDTEST_INFO		"bdtest_info"
#define TRIGTEST_INFO	"trigtest_info"
#define FDISK_INFO		"fdisk_info"

//// 网络通讯相关的数据
typedef struct
{
	int loop;							//环路收发控制字
	int fd;							//socket控制字
	int enc_type;						//加密类型
	int flag;							// 发送标志
	int multi_port;					//组播端口
	struct sockaddr_in recv_addr;		//接收地址
	struct sockaddr_in send_addr;		//发送地址
	struct ip_mreq mreq;				//组播组
	unsigned char self_id[8];				// 设备自身的GUID
	unsigned char self_id1[8];				// 虚拟设备的另一个GUID  lsk 2009-2-10
	unsigned char target_id[8];			// 目标设备的GUID
	unsigned char recv_id[8];			// 接收到的虚拟设备的GUID
	unsigned char ip_addr[BUF_LEN];		//设备自身的IP地址
	unsigned char hostname[50];			//主机IP地址
	unsigned char seq_num[50];			//报文序列号	
}multicast_sock;

#define ETH0 0
#define ETH1 1
////系统内部运行的线程标志
#define UPDATA_FLAG		1
#define TEST_BD_FLAG		2
#define TEST_TG_FLAG		3
#define FORMAT_FLAG		4
#define CLEAN_FLAG			5
#define TEST_IDE_FLAG		6

static const char *eq_mark="==";	// equal mark in name value pair
static const char *sep_mark="&&";	// seperator in name value pair



#endif
