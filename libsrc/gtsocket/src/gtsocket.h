/** @file	       gtsocket.c
 *   @brief 	提供可移植的socket网络操作接口
 *   @auth      shixin
 *				错误码定义见gt_errno.h
 *   @date 	2007.06
 */
#ifndef GTSOCKET_H
#define GTSOCKET_H
#ifndef IN
#define IN 
#endif

#ifndef OUT
#define OUT
#endif

#ifndef IO
#define IO
#endif

#ifdef _WIN32
//windows
//#include <winsock2.h>
#include <windows.h>
#define     SOCK_FD                 SOCKET             ///<socket句柄类型定义
#define		socklen_t				int


#include <gt_errno.h>						///<错误码定义
#else
//linux
#include <sys/socket.h>
#include <netinet/in.h>
#define     SOCK_FD                 int             ///<socket句柄类型定义
#define     INVALID_SOCKET    -1            ///<无效的socket号

#endif

#include <errno.h>						///<错误码定义

#ifdef __cplusplus
extern "C" {
#endif
#undef EXPORT_DLL
#ifdef _WIN32
	//windows 使用

	#define EXPORT_DLL __declspec(dllexport)

#else

	//linux 使用

	#define EXPORT_DLL

#endif
/** 
 *   @brief     在指定的地址和端口上创建tcp服务socket
 *   @param  svr_addr 用于侦听的地址 INADDR_ANY表示在所有地址侦听
 *   @param  port   侦听的端口号
 *   @return   正值表示创建的socket描述符,负值表示失败
 */ 
EXPORT_DLL SOCK_FD create_tcp_listen_port(unsigned long svr_addr,int port);
/** 
 *   @brief     设置网络连接接收数据的超时时间(阻塞方式下)
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  second:接收超时时间秒数
 *   @return   0表示成功,负值表示失败
 */ 
EXPORT_DLL int net_set_recv_timeout(SOCK_FD fd,int second);
/** 
 *   @brief    设置网络连接发送数据的超时时间(阻塞方式下)
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  second:发送超时时间秒数
 *   @return   0表示成功,负值表示失败
 */ 
EXPORT_DLL int net_set_send_timeout(SOCK_FD fd,int second);
/** 
 *   @brief    将已连接的tcp设置为进行断线探测
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   0表示成功,负值表示失败
 */ 
EXPORT_DLL int	net_activate_keepalive(SOCK_FD fd);

/** 
 *   @brief    设置tcp连接的keepalive超时时间
 *   @param	   fd:已经打开的tcp连接描述符
 *	 @param	   retrys:尝试次数
 *	 @param	   timout:第一次超时的时间(秒)
 *	 @param	   interval:间隔时间(秒)
 *   @return   0表示成功,负值表示失败
 */ 
int	net_set_keepalive_time(SOCK_FD fd,int retrys,int timeout,int interval);
/** 
 *   @brief    将服务socket设置为可重用方式,防止服务退出第二次启动时不正常
 *   @param  fd:服务socket
 *   @return   0表示成功,负值表示失败
 */ 
EXPORT_DLL int net_activate_reuseaddr(SOCK_FD fd);
/** 
 *   @brief    将已连接的tcp设置为禁用nagle算法(不积累数据包)
 *   @param  d:已经打开的tcp连接描述符
 *   @return   0表示成功,负值表示失败
 */ 
EXPORT_DLL int net_set_nodelay(SOCK_FD fd);
/** 
 *   @brief    设置网络连接的linger属性,即调用close关闭socket前是否将没有发送完
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  en:0表示不使用linger
 *                     1表示使用linger  
 *   @return   0表示成功,负值表示失败
 */ 
EXPORT_DLL int net_set_linger(SOCK_FD fd,int en);
/** 
 *   @brief    设置网络连接的阻塞模式
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  en:0表示使用阻塞模式
 *                     1表示使用l非阻塞模式
 *   @return   0表示成功,负值表示失败
 */ 
EXPORT_DLL int net_set_noblock(SOCK_FD fd,int en);
/** 
 *   @brief    设置tcp发送缓冲区的长度
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  size:要设置的长度
 *   @return   0表示成功,负值表示失败
 */ 
EXPORT_DLL int net_set_tcp_sendbuf_len(SOCK_FD fd,int size);
/** 
 *   @brief    获取tcp发送缓冲区的长度
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   正值表示发送缓冲区的长度负值表示出错
 */ 
EXPORT_DLL int net_get_tcp_sendbuf_len(SOCK_FD fd);
/** 
 *   @brief    设置tcp接收缓冲区的长度
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  size:要设置的长度
 *   @return   0表示成功,负值表示失败
 */ 
EXPORT_DLL int net_set_tcp_recvbuf_len(SOCK_FD fd,int size);
/** 
 *   @brief    获取tcp接收缓冲区的长度
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   正值表示接收缓冲区的长度负值表示出错
 */ 
EXPORT_DLL int net_get_tcp_recvbuf_len(SOCK_FD fd);
/** 
 *   @brief    设置tcp发送缓冲区下限(select会用到)
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  size:要设置的长度
 *   @return   0表示成功,负值表示失败
 */ 
EXPORT_DLL int	net_set_sock_send_low(SOCK_FD fd,int size);
/** 
 *   @brief    获取tcp发送缓冲区下限
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   正值表示发送缓冲区下限,负值表示出错
 */ 
EXPORT_DLL int	net_get_sock_send_low(SOCK_FD fd);
/** 
 *   @brief    获取tcp接收缓冲区下限
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   正值表示发送缓冲区下限,负值表示出错
 */ 
EXPORT_DLL int	net_get_sock_recv_low(SOCK_FD fd);
/** 
 *   @brief    设置tcp接收缓冲区下限(select会用到)
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  size:要设置的长度
 *   @return   0表示成功,负值表示失败
 */ 
EXPORT_DLL int	net_set_sock_recv_low(SOCK_FD fd ,int size);
/** 
 *   @brief    获取接收缓冲区中的有效字节数
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   正值表示接收缓冲区中的有效字节数,负值表示出错
 */ 
#define get_fd_in_buffer_num get_sock_in_buffer_num
EXPORT_DLL int get_sock_in_buffer_num(SOCK_FD fd);
/** 
 *   @brief    获取接发送缓冲区中的有效字节数
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   正值表示发送缓冲区中的有效字节数,负值表示出错
 */ 
#define get_fd_out_buffer_num   get_sock_out_buffer_num
EXPORT_DLL int	get_sock_out_buffer_num(SOCK_FD fd);

/** 
 *   @brief    获取远程tcp连接的地址
 *   @param	   fd:已经打开的tcp连接描述符
 *	 @param		addr:准备存放ip地址的指针
 *   @return   0表示成功，负值表示失败
 */ 
EXPORT_DLL int	get_peer_ip(SOCK_FD fd,struct sockaddr_in *addr);
/** 
 *   @brief    获取tcp连接的本地地址
 *   @param	   fd:已经打开的tcp连接描述符
 *	 @param		addr:准备存放ip地址的指针
 *   @return   0表示成功，负值表示失败
 */ 
EXPORT_DLL int	get_local_ip(SOCK_FD fd,struct sockaddr_in *addr);

/** i
 *   @brief    获取远程tcp连接的地址字符串
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   远程连接地址字符串 NULL表示失败,错误码存于errno
 */ 
EXPORT_DLL char *get_peer_ip_str(SOCK_FD fd);

/** 
 *   @brief    获取一个tcp连接的本地ip地址字符串
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   本地连接地址字符串 NULL表示失败,错误码存于errno
 */ 
EXPORT_DLL char *get_local_ip_str(SOCK_FD fd);
/** 
 *   @brief    带超时参数的connect,除了timeout参数以外其它参数和connect完全一样
 *   @param  fd:已经创建好的socket
 *   @param  serv_addr:要连接的地址及端口号
 *   @param  addrlen:serv_addr结构的长度
 *   @param  timeout:超时返回时间(秒)
 *   @return   0表示成功,负值表示失败
 */										
EXPORT_DLL int  connect_timeout(SOCK_FD  fd,  const  struct sockaddr *serv_addr, socklen_t    addrlen,int timeout);
/** 
 *   @brief    连接远程tcp服务地址
 *   @param  addr_str:描述远程服务地址的字符串
 *   @param  port:远程tcp服务的端口号
 *   @param   timeout:tcp连接的超时时间(秒)
 *   @return   正值表示已连接好的描述符，负值表示出错
 */ 
EXPORT_DLL SOCK_FD tcp_connect_addr(const char *addr_str,int port,int timeout);

/** 
 *   @brief    将缓冲区的数据全部写入一个网络连接中
 *   @param  fd:目标文件描述符
 *   @param  *buf:指向要发送的缓冲区的指针
 *   @param  len:要发送的缓冲区中的有效信息字节数
 *   @return   正值表示成功写入的字节数,负值代表发送异常,错误码待定
 */ 
EXPORT_DLL SOCK_FD tcp_connect_block(const char *addr_str,int port,int timeout);

EXPORT_DLL int net_write_buf(SOCK_FD fd,void *buf,int len);
/** 
 *   @brief    从一个网络连接中读取指定字节数放入缓冲区
 *   @param  fd:目标文件描述符
 *   @param  *buf:指向存放接收数据的缓冲区指针
 *   @param  len:要接收的数据字节数
 *   @return   正值表示成功存入接收缓冲区的字节数,负值代表接收异常;
 *			  -EAGAIN 表示接收超时(SO_RCVTIMEO设定的)
 *			  -ETIMEDOUT linux下由于keepalive导致的超时
 *			  -ECONNRESET windows 下由于keepalive导致的断开或其它错误
 */ 
EXPORT_DLL int net_read_buf(SOCK_FD fd,void *buf,int len) ;
/** 
 *   @brief     关闭已经建立的网络连接
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   0表示成功,负值表示失败
 */ 
EXPORT_DLL int	net_close(SOCK_FD fd);

/*
	函数说明:
	设置tcp连接的属性,一般用于connect或accept刚成功后，包括：
	打开keepalive，时间设置为15秒发送一次探测包，失败后间隔10秒重试3次
	关闭nagle算法
	关闭linger

	recv_timeout表示接收数据没有响应超时时间
	返回0表示成功,负值表示失败
*/
EXPORT_DLL int set_tcp_socket_attr(int fd,int recv_timeout);


///////////////////////////////////////////UDP部分//////////////////////////////////////
/** 
 *   @brief    创建一个UDP描述符(发送和接收)
 *   @param  ip_str :准备侦听的地址字符串
 *   @param  port    :准备侦听的端口号
 *   @return    负值表示失败，其它值表示文件描述符       
 */ 
EXPORT_DLL int udp_create(IN char *ip_str,IN int port);
/** 
 *   @brief    打开描述符的组播功能
 *   @param  fd 之前用udp_create获取的描述符
 *   @param  multicast_addr :组播地址
 *   @return    0表示成功，负值表示失败
 */ 
EXPORT_DLL int udp_add_multicast(int fd,char *multicast_addr);


/** 
 *   @brief    发送一包数据
 *   @param  fd 之前用udp_create获取的描述符
 *   @param  data:要发送的数据缓冲区
 *   @param  len:缓冲区中的有效数据字节数
 *   @param  flags:保留，调用时写0
 *   @param  to:发送的目的地址,例
 *                  to->sin_addr.s_addr=inet_addr("224.0.0.1"); //填充地址及端口
                     to->sin_port=htons((short)6868);
 *   @return    正值表示发送出去的字节数，负值表示失败
 */ 
EXPORT_DLL int udp_send_data(IN int fd, IN void *data, IN int  len, IN int flags, IN struct sockaddr_in *to);




/** 
 *   @brief    接收一包UDP数据
 *   @param  fd 之前用udp_create获取的描述符
 *   @param  buf:准备存放数据的缓冲区
 *   @param  len:缓冲区长度
 *   @param  flags:保留，调用时写0
 *   @param  from:接收到的数据的远程地址
 *   @return   正值表示接收到buf中的有效字节数，负值表示失败
 */ 
EXPORT_DLL int udp_recv_data(IN int fd,OUT void *buf, IN int len, IN int flags, OUT struct sockaddr_in *from);




#ifdef __cplusplus
}
#endif

#endif

