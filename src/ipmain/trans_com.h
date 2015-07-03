//透明串口
#ifndef TRANS_COM_H
#define TRANS_COM_H
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "mod_socket.h"

struct trans_com_struct{		//描述透明串口的结构
	pthread_mutex_t mutex;		//访问结构要用到的锁
	pthread_t thread_id;
	int	ch;						//串口通道
	int flag;						//连接标志,如果已经有远程连接则置1,否则为0
	int listen_fd;					//监听连接的tcp描述符
	int net_fd;					//映射到对应的tcp描述符
	int tcp_port; 					//tcp端口
	struct sockaddr_in allow_addr;//允许访问透明串口的远程计算机地址
	int local_fd;					//串口文件描述符
	char path[20];				//串口设备路径
	DWORD	baud;				//波特率
	BYTE	databit;				//数据位，一般为8
	BYTE	parity;				//是否要奇偶校验位,一般为不需要'N'	
	BYTE	stopbit;				//停止位，一般为1
	BYTE	flow;				//流量控制，一般不需要
	DWORD	net_rec_buf[256];	//接收网络数据的缓冲区
	DWORD	com_rec_buf[256];	//接收串口数据的缓冲区
};


typedef struct{
	BYTE start;		//起位 0xfe
	BYTE crc;		//校验字节
	BYTE cmd;		//命令类型
	BYTE ichannel;	//通道数
	BYTE bitstr;	//从低至高使能
	BYTE heart;    //心跳
	BYTE reserved[2]; //保留 0
}GT_SUB_CMD_STRUCT;


/**********************************************************************************************
 * 函数名	:init_trans_com_var()
 * 功能	:初始化透明串口服务用到的变量
 * 输入	:无
 * 返回值	:无
 **********************************************************************************************/
void init_trans_com_var(void);

/**********************************************************************************************
 * 函数名	:get_trans_com_info()
 * 功能	:获取指定通道的透明串口的描述结构指针
 * 输入	:ch:串口通道号
 * 返回值	:描述指定通道号的透明串口的结构指针
 *			 NULL表示出错
 **********************************************************************************************/
struct trans_com_struct *get_trans_com_info(int ch);


/**********************************************************************************************
 * 函数名	:creat_trans_com_thread()
 * 功能	:创建透明串口服务线程
 * 输入	:attr:线程属性
 *			 arg:要创建的透明串口服务的描述结构指针
 * 返回值	:0表示成功负值表示出错
 **********************************************************************************************/
int creat_trans_com_thread(pthread_attr_t *attr,void *arg);



/*****************************************************************************************
 *函数名:keepalive_send_com_internal(int alarmchannel,int audiochannel,int heart)
 *功能  :通过调试串口向单片机发送字符串，用来表示本设备没有死机
 *输入  :alarmchannel 核警中通道号，高优先级，audiochannel 下行音频通道号，heart表示是否有心跳
 *输出  :无
 *返回  :正确返回0，错误返回负值
 * ***************************************************************************************/                                                 
int keepalive_send_com_internal(int alarmchannel,int audiochannel,int heart); 

/****************************************************************************************                                                        
 *函数名:keepalive_open_com_internal()
 *功能  :打开com_internal，获取串口fd，用来发送心跳数据,此串口为内部调试用串口                                                                
 *输入  :无                                                                                                                                      
 *输出  :无
 *返回  :正常返回0，错误返回负值                                                                                                                 
 * **************************************************************************************/                                                       
int keepalive_open_com_internal(void);

/****************************************************************************************                                                        
 *函数名:keepalive_set_com_mode(int enable,int interval)
 *功能  :设置串口工作模式                                                               
 *输入  :enable 是否使能心跳  interval 心跳检测间隔(分钟为单位)                                                                                                                                      
 *输出  :无
 *返回  :正常返回0，错误返回负值                                                                                                                 
 * **************************************************************************************/
int keepalive_set_com_mode(int enable,int interval);


int update_set_com_mode(int enable,int interval);

#endif
