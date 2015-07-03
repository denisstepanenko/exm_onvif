#ifndef __TESTMOD_H
#define __TESTMOD_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>




#define BUF_LEN  100


#define TESTMOD_LOCK_FILE 		("/lock/ipserver/testmod")
#define PACKAGE    				("Trigtest")
#define VERSION					("0.01")

//v0.01 从vs3024上移植，作为第一个版本




#define RESULT_FILE_NAME		("/tmp/testtrig.txt")



#if 0
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
#endif


/**********************************************************************************************
* 函数名   :s_test_rp()
* 功能  :       发送测试报告
* 输入  :      buf			发送的字符串
*				num		进程号
* 输出  :       void        
* 返回值:   void
**********************************************************************************************/
int s_test_rp(unsigned char *buf,int num);


#endif
