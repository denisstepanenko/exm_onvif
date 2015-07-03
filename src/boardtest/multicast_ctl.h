#ifndef 	_MULTICAST_CTL_H_
#define	_MULTICAST_CTL_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define	DEV_GUID_BYTE		8	//GUID占用的字节数


#define BUF_LEN  100
//#define TEST224
#ifdef  TEST224
#define HOSTNAME_ETH0 		"224.0.0.1"
#define DEF_MULT_ADDR0		"224.0.0.0"
#else
#define DEF_MULT_ADDR		"224.0.0.0"
#define HOSTNAME_ETH0 		"225.0.0.1"
#define DEF_MULT_ADDR0		"225.0.0.0"
#endif
#define HOSTNAME_ETH1 		"226.0.0.1"
#define DEF_MULT_ADDR1		"226.0.0.0"
#define DEF_MULT_NETMASK	"255.0.0.0"

#define MULTI_CAST_PORT 3310

#define CMD_STR 	"cmd"
#define SEQ_STR 	"seq"

////lsk 2007 -6-1
#define ACK_PKT	"ack"
#define SEQ_ACK	"seq_ack"
#define RET_VAL	"ret"
#define ERR_STR	"err"

#define RPT_PKT		"process_report"
#define PROGRESS		"progress"
#define DETAIL_STR	"detail"

//// end of change 

////硬件测试状态信息反馈数据包
#define M_TEST_DEV_RETURN 			"test_dev_return"
#define RESULT_STR					"result_str"
#define TEST_OPT						"test_opt"

#define BOARD_TEST  			"board_test"
#define TRIG_TEST  			"trig_test"
#define FORMAT_DISK  		"format_disk"

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
	unsigned char target_id[8];			// 目标设备的GUID
	unsigned char ip_addr[BUF_LEN];		//设备自身的IP地址
	unsigned char hostname[50];			//主机IP地址
	unsigned char seq_num[50];
}multicast_sock;
////系统内部运行的线程标志
#define UPDATA_FLAG		1
#define TEST_BD_FLAG	2
#define TEST_TG_FLAG		3
#define FORMAT_FLAG		4
#define CLEAN_FLAG		5

#define TRIG_NODE 		"trigtest"
#define BOARD_NODE 		"boardtest"
#define FMAT_NODE		"format"
#define CLR_NODE		"clear"
#define REPORT			"report"

//path of temp file that record infotmation about clearing and formating 
#define TEST_BD_RESULT	"/tmp/testbd.txt"
#define TEST_TG_RESULT	"/tmp/testtrig.txt"
#define FMAT_INFO 		"/tmp/init_info.txt"	
#define CLR_INFO 		"/tmp/clr_info.txt"	

 int send_test_report(multicast_sock* ns, unsigned char* info,int prog);
int init_dev_net_port(multicast_sock* net_st);
int send_multicast_pkt(multicast_sock* net_st,  char * buf);
int result_report(int index, multicast_sock *ns);


#endif
