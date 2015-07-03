/** @file	       gtsocket.c
 *   @brief 	提供可移植的socket网络操作接口
 *   @auth      shixin
 *   @date 	2007.06
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
//windows
#include <stdio.h>
#include <stdlib.h>
struct _RPC_ASYNC_STATE;	///<防止出编译警告
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#define close(a)		closesocket(a)			///<关闭socket
#define write(a, b, c)	send(a, b, c, 0)		///<写操作
#define read(a, b, c)	recv(a, b, c, 0)		///<读操作

#else
//linux
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
//#include <errno.h>
#endif

#include "gtsocket.h"
#include "gtlog.h"
#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

#ifndef TIOCOUTQ
#define TIOCOUTQ      0x5411
#endif
#ifndef INADDR_NONE
#define INADDR_NONE     0xffffffff
#endif

#if 0 //def _WIN32
#undef	errno
#define errno			WSAGetLastError()		///<错误码
#endif

#ifdef _WIN32
//platform SDK #include <mstcpip.h>
  #pragma comment (lib,"ws2_32")

  //   New   WSAIoctl   Options     
  #define   SIO_RCVALL                         _WSAIOW(IOC_VENDOR,1)  
  #define   SIO_RCVALL_MCAST             _WSAIOW(IOC_VENDOR,2)  
  #define   SIO_RCVALL_IGMPMCAST     _WSAIOW(IOC_VENDOR,3)  
  #define   SIO_KEEPALIVE_VALS         _WSAIOW(IOC_VENDOR,4)  
  #define   SIO_ABSORB_RTRALERT       _WSAIOW(IOC_VENDOR,5)  
  #define   SIO_UCAST_IF                     _WSAIOW(IOC_VENDOR,6)  
  #define   SIO_LIMIT_BROADCASTS     _WSAIOW(IOC_VENDOR,7)  
  #define   SIO_INDEX_BIND                 _WSAIOW(IOC_VENDOR,8)  
  #define   SIO_INDEX_MCASTIF           _WSAIOW(IOC_VENDOR,9)  
  #define   SIO_INDEX_ADD_MCAST       _WSAIOW(IOC_VENDOR,10)  
  #define   SIO_INDEX_DEL_MCAST       _WSAIOW(IOC_VENDOR,11)   
#endif
/** 
 *   @brief     在指定的地址和端口上创建tcp服务socket
 *   @param  svr_addr 用于侦听的地址 INADDR_ANY表示在所有地址侦听
 *   @param  port   侦听的端口号
 *   @return   正值表示创建的socket描述符,负值表示失败
 */ 
SOCK_FD create_tcp_listen_port(unsigned long svr_addr,int port)
{
    SOCK_FD                      fd;
    struct sockaddr_in           svr;
#ifdef _WIN32
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;
 
        wVersionRequested = MAKEWORD( 2, 0 );
 
        err = WSAStartup( wVersionRequested, &wsaData );
        if ( err != 0 ) {
            /* Tell the user that we couldn't find a usable */
            /* WinSock DLL.                               */
            //printf("can't initialize socket library\n");
			errno=WSAGetLastError();
			return -1;
        }
    }
#endif

    fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd==INVALID_SOCKET)
    {
        //printf("can't create socket:%s!\n",strerror(errno));
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
        return INVALID_SOCKET;
    }    
///如果服务器终止后,服务器可以第二次快速启动而不用等待一段时间
 	net_activate_reuseaddr(fd);	
	memset(& svr, 0 ,sizeof(struct sockaddr_in));
 	svr.sin_family=AF_INET; 
 	svr.sin_port=htons((unsigned short)port); 
 	svr.sin_addr.s_addr=htonl(svr_addr); 
 	if(bind(fd,(struct sockaddr *)&svr,sizeof(svr))<0) 
  	{   	    
	      //  printf("Bind Error:%s\n\a",strerror(errno)); 	
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
            close(fd);
	        return INVALID_SOCKET;
  	} 
	net_set_noblock(fd,0);
    return fd;
}
/** 
 *   @brief     关闭已经建立的网络连接
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   0表示成功,负值表示失败
 */ 
int	net_close(SOCK_FD fd)
{
	int ret;
	ret=close(fd);
	if(ret!=0)
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
	}
	return ret;
}

/** 
 *   @brief     设置网络连接接收数据的超时时间(阻塞方式下)
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  second:接收超时时间秒数
 *   @return   0表示成功,负值表示失败
 */ 
int net_set_recv_timeout(SOCK_FD fd,int second)
{
	int ret;
#ifdef _WIN32	
	int	timeout;
#else
	struct timeval timeout;
#endif

#ifdef _WIN32
	timeout=second*1000;
#else	
	timeout.tv_sec=second;
	timeout.tv_usec=0;
#endif
	ret = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	if(ret!=0)
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
	}
	return	ret;
}

/** 
 *   @brief    设置网络连接发送数据的超时时间(阻塞方式下)
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  second:发送超时时间秒数
 *   @return   0表示成功,负值表示失败
 */ 
int net_set_send_timeout(SOCK_FD fd,int second)
{
	int	ret;
#ifdef _WIN32	
	int	timeout;
#else
	struct timeval timeout;
#endif

#ifdef _WIN32
	timeout=second*1000;
#else	
	//timeout.tv_sec=second;
	//timeout.tv_usec=0;
	timeout.tv_sec=0;
	timeout.tv_usec= second;
#endif
	ret = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
	if(ret!=0)
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
	}
	return ret;
}

/** 
 *   @brief    将已连接的tcp设置为进行断线探测
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   0表示成功,负值表示失败
 */ 
int	net_activate_keepalive(SOCK_FD fd)
{
	int ret;
    int keepalive = 1;
    ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepalive, sizeof(keepalive));
	if(ret!=0)
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
	}
	return ret;
}
typedef struct      
{  
	u_long     onoff;  
	u_long     keepalivetime;  
	u_long     keepaliveinterval;  
}TCP_KEEPALIVE;  
/** 
 *   @brief    设置tcp连接的keepalive超时时间
 *   @param	   fd:已经打开的tcp连接描述符
 *	 @param	   retrys:尝试次数
 *	 @param	   timout:第一次超时的时间(秒)
 *	 @param	   interval:间隔时间(秒)
 *   @return   0表示成功,负值表示失败
 */ 
int	net_set_keepalive_time(SOCK_FD fd,int retrys,int timeout,int interval)
{
#ifdef _WIN32
	TCP_KEEPALIVE   inKeepAlive   =   {0};   //输入参数  
	unsigned   long   ulInLen   =   sizeof(TCP_KEEPALIVE);    

	TCP_KEEPALIVE   outKeepAlive   =   {0};   //输出参数  
	unsigned   long   ulOutLen   =   sizeof(TCP_KEEPALIVE);    

	unsigned   long   ulBytesReturn   =   0;    

	//设置socket的keep   alive为5秒，并且发送次数为3次(默认)      
	inKeepAlive.onoff=1;    
	inKeepAlive.keepaliveinterval=timeout*1000;   //开始首次KeepAlive探测前的TCP空闭时间
	inKeepAlive.keepalivetime=interval*1000;   // 两次KeepAlive探测间的时间间隔   
	if(WSAIoctl(
		(unsigned   int)fd,
		SIO_KEEPALIVE_VALS,      
		(LPVOID)&inKeepAlive,ulInLen,      
		(LPVOID)&outKeepAlive,ulOutLen,      
		&ulBytesReturn,   
		NULL,   NULL)   
		==   SOCKET_ERROR)    
	{                    
		printf("WSAIoctl   failed.   error   code(%d)!\n",WSAGetLastError());  
		return -1;
	}   
	return 0;
#else
	//linux
//	#include   <netinet/tcp.h>  
//	……  
	////KeepAlive实现  
	//下面代码要求有ACE,如果没有包含ACE,则请把用到的ACE函数改成linux相应的接口  
	int   keepAlive   =   1;//设定KeepAlive  
	int   keepIdle   =   5;//开始首次KeepAlive探测前的TCP空闭时间  
	int   keepInterval   =   5;//两次KeepAlive探测间的时间间隔  
	int   keepCount   =   3;//判定断开前的KeepAlive探测次数  

	if(setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(void*)&keepAlive,sizeof(keepAlive))   ==   -1)  
	{  
	  printf("setsockopt   SO_KEEPALIVE   error!\n");  
	  return -1;
	}  

	if(setsockopt(fd,SOL_TCP,TCP_KEEPIDLE,(void   *)&keepIdle,sizeof(keepIdle))   ==   -1)  
	{  
	  printf("setsockopt   TCP_KEEPIDLE   error!\n");
	  return -1;
	}  

	if(setsockopt(fd,SOL_TCP,TCP_KEEPINTVL,(void   *)&keepInterval,sizeof(keepInterval))   ==   -1)  
	{  
	  printf("setsockopt   TCP_KEEPINTVL   error!\n");
	  return -1;
	}  

	if(setsockopt(fd,SOL_TCP,TCP_KEEPCNT,(void   *)&keepCount,sizeof(keepCount))   ==   -1)  
	{  
	  printf("setsockopt   TCP_KEEPCNT   error!\n");
	  return -1;
	}   
	return 0;
#endif
}
/** 
 *   @brief    将服务socket设置为可重用方式,防止服务退出第二次启动时不正常
 *   @param  fd:服务socket
 *   @return   0表示成功,负值表示失败
 */ 
int net_activate_reuseaddr(SOCK_FD fd)
{
  int	ret;
  int reuseaddr = 1;
  ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseaddr,sizeof(reuseaddr));
  if(ret!=0)
  {
#ifdef _WIN32
	errno=WSAGetLastError();
#endif
  }
  return ret;
}

/** 
 *   @brief    将已连接的tcp设置为禁用nagle算法(不积累数据包)
 *   @param  d:已经打开的tcp连接描述符
 *   @return   0表示成功,负值表示失败
 */ 
int net_set_nodelay(SOCK_FD fd)
{
  int ret;
  int nodelay = 1;
  ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
  if(ret!=0)
  {
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
  }
  return ret;
}

#if 0
int net_activate_oobinline(SOCK_FD fd)
{
  int oob_inline = 1;
 return setsockopt(fd, SOL_SOCKET, SO_OOBINLINE, &oob_inline, sizeof(oob_inline));
}


int net_set_iptos_throughput(SOCK_FD fd)
{
  int tos = IPTOS_THROUGHPUT;
  /* Ignore failure to set (maybe this IP stack demands privilege for this) */
  return setsockopt(fd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));
}
#endif
/** 
 *   @brief    设置网络连接的linger属性,即调用close关闭socket前是否将没有发送完
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  en:0表示不使用linger
 *                     1表示使用linger  
 *   @return   0表示成功,负值表示失败
 */ 
int net_set_linger(SOCK_FD fd,int en)
{
//  	int retval;
	int	ret;
  	struct linger the_linger;
	if(en)
	{
  		the_linger.l_onoff = 1;
  		the_linger.l_linger = (unsigned short)INT_MAX;
	}
	else
	{
		the_linger.l_onoff = 0;
  		the_linger.l_linger = 0;
	}
  	ret = setsockopt(fd, SOL_SOCKET, SO_LINGER, (char*)&the_linger,sizeof(the_linger));
	if(ret!=0)
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
	}
	return ret;
}

/** 
 *   @brief    设置tcp发送缓冲区的长度
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  size:要设置的长度
 *   @return   0表示成功,负值表示失败
 */ 
int net_set_tcp_sendbuf_len(SOCK_FD fd,int size)
{
	int bufsize;
	int ret;
	if((fd<0)||(size<0))
		return -1;
	bufsize=size/2;
	//lensize=sizeof(bufsize);
	ret= setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&bufsize, sizeof(bufsize));	
	if(ret!=0)
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
	}
	return ret;
}
/** 
 *   @brief    获取tcp发送缓冲区的长度
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   正值表示发送缓冲区的长度负值表示出错
 */ 
int net_get_tcp_sendbuf_len(SOCK_FD fd)
{
	int ret;
	int bufsize,lensize;
	lensize=sizeof(bufsize);
	ret=getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&bufsize, &lensize);
	if(ret==0)
	{
		return bufsize;
	}
	else
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
		return -1;
	}
}
/** 
 *   @brief    设置tcp接收缓冲区的长度
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  size:要设置的长度
 *   @return   0表示成功,负值表示失败
 */ 
int net_set_tcp_recvbuf_len(SOCK_FD fd,int size)
{
	int ret;
	int bufsize;
	if((fd<0)||(size<0))
	{
		errno=EINVAL;
		return -1;
	}
	bufsize=size/2;
	ret = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&bufsize, sizeof(bufsize));	
	if(ret!=0)
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
	}
	return ret;
}

/** 
 *   @brief    获取tcp接收缓冲区的长度
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   正值表示接收缓冲区的长度负值表示出错
 */ 
int net_get_tcp_recvbuf_len(SOCK_FD fd)
{
	int bufsize,lensize;
	int ret;
	lensize=sizeof(bufsize);
	ret=getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&bufsize, &lensize);
	if(ret==0)
		return bufsize;
	else
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
		return ret;
	}
}
/** 
 *   @brief    设置tcp发送缓冲区下限(select会用到)
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  size:要设置的长度
 *   @return   0表示成功,负值表示失败
 */ 
int	net_set_sock_send_low(SOCK_FD fd,int size)
{
	int ret;
	int bufsize;
	if((fd<0)||(size<0))
	{
		errno=EINVAL;
		return -1;
	}
	bufsize=size;
	ret = setsockopt(fd, SOL_SOCKET, SO_SNDLOWAT, (char*)&bufsize, sizeof(bufsize));	
	if(ret!=0)
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
	}
	return ret;
}
/** 
 *   @brief    获取tcp发送缓冲区下限
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   正值表示发送缓冲区下限,负值表示出错
 */ 
int	net_get_sock_send_low(SOCK_FD fd)
{
	int bufsize,lensize;
	int ret;
	lensize=sizeof(bufsize);
	ret=getsockopt(fd, SOL_SOCKET, SO_SNDLOWAT, (char*)&bufsize, &lensize);
	if(ret==0)
		return bufsize;
	else
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
		return ret;	
	}
}
/** 
 *   @brief    获取tcp接收缓冲区下限
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   正值表示发送缓冲区下限,负值表示出错
 */ 
int	net_get_sock_recv_low(SOCK_FD fd)
{
	int bufsize,lensize;
	int ret;
	lensize=sizeof(bufsize);
	ret=getsockopt(fd, SOL_SOCKET, SO_RCVLOWAT, (char*)&bufsize, &lensize);
	if(ret==0)
		return bufsize;
	else
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
		return ret;	
	}
}
/** 
 *   @brief    设置tcp接收缓冲区下限(select会用到)
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  size:要设置的长度
 *   @return   0表示成功,负值表示失败
 */ 
int	net_set_sock_recv_low(SOCK_FD fd ,int size)
{
	int	ret;
	int bufsize;
	if((fd<0)||(size<0))
	{
		errno=EINVAL;
		return -1;
	}
	bufsize=size;
	ret = setsockopt(fd, SOL_SOCKET, SO_RCVLOWAT, (char*)&bufsize, sizeof(bufsize));	
	if(ret!=0)
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
	}
	return ret;
}

/** 
 *   @brief    获取接收缓冲区中的有效字节数
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   正值表示接收缓冲区中的有效字节数,负值表示出错
 */ 
#define get_fd_in_buffer_num get_sock_in_buffer_num
int get_sock_in_buffer_num(SOCK_FD fd)
{//获取接收缓冲区的有效数据
	int ret;
	unsigned long cnt=100;
#ifdef _WIN32
	ret=ioctlsocket(fd,FIONREAD,&cnt);
#else
	ret=ioctl(fd,FIONREAD,&cnt);
#endif
	if(ret<0)
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
		if(errno>0)
			return -errno;
		else
			return -1;
	}
	else
	{
		return (int)cnt;
	}
}

/** 
 *   @brief    获取接发送缓冲区中的有效字节数
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   正值表示发送缓冲区中的有效字节数,负值表示出错
 */ 
int	get_sock_out_buffer_num(SOCK_FD fd)
{//获取发送缓冲区中未发送出的数据数

#ifndef _WIN32
	int ret;
	int cnt=0;
	ret=ioctl(fd,TIOCOUTQ  ,&cnt);
	if(ret<0)
		return -1;
	else
		return cnt;
#else
///#warn "can't support this function in windows"
	///TODO 寻找windows下的替代品...
	return 0;
#endif
}

/** 
 *   @brief    获取远程tcp连接的地址字符串
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   远程连接地址字符串 NULL表示失败,错误码存于errno
 */ 
char *get_peer_ip_str(SOCK_FD fd)
{
	int Ret;
	struct sockaddr_in RmtAddr;
	int AddrLen=sizeof(struct sockaddr);
	
	Ret=getpeername(fd,(struct sockaddr *)&RmtAddr,&AddrLen);
	if(Ret==0)
		return inet_ntoa(RmtAddr.sin_addr);
	else
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
		return NULL;
	}
}

/** 
 *   @brief    获取一个tcp连接的本地ip地址字符串
 *   @param  fd:已经打开的tcp连接描述符
 *   @return   本地连接地址字符串 NULL表示失败,错误码存于errno
 */ 
char *get_local_ip_str(SOCK_FD fd)
{
    int ret;
    struct sockaddr_in local_addr;
    int     addrlen=sizeof(struct sockaddr_in);
    ret=getsockname(fd,(struct sockaddr *)&local_addr,&addrlen);
    if(ret==0)
        return inet_ntoa(local_addr.sin_addr);
    else
    {
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
		return NULL;    
    }
}
/** 
 *   @brief    获取远程tcp连接的地址
 *   @param	   fd:已经打开的tcp连接描述符
 *	 @param		addr:准备存放ip地址的指针
 *   @return   0表示成功，负值表示失败
 */ 
int	get_peer_ip(SOCK_FD fd,struct sockaddr_in *addr)
{
	int	ret;
	int addrlen=sizeof(struct sockaddr);
	if(addr==NULL)
	{
		//errno=EINVAL;
		return -1;
	}
	ret=getpeername(fd,(struct sockaddr *)addr,&addrlen);
	if(ret!=0)
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
	}
	return ret;
}
/** 
 *   @brief    获取tcp连接的本地地址
 *   @param	   fd:已经打开的tcp连接描述符
 *	 @param		addr:准备存放ip地址的指针
 *   @return   0表示成功，负值表示失败
 */ 
int	get_local_ip(SOCK_FD fd,struct sockaddr_in *addr)
{
	int	ret;
	int addrlen=sizeof(struct sockaddr);
	if(addr==NULL)
	{
		//errno=EINVAL;
		return -1;
	}
	ret=getsockname(fd,(struct sockaddr *)addr,&addrlen);
	if(ret!=0)
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
	}
	return ret;
}


/** 
 *   @brief    设置网络连接的阻塞模式
 *   @param  fd:已经打开的tcp连接描述符
 *   @param  en:0表示使用阻塞模式
 *                     1表示使用l非阻塞模式
 *   @return   0表示成功,负值表示失败
 */ 

int net_set_noblock(SOCK_FD fd,int en)
#ifdef _WIN32
{//windows
	int ret;
	ret = ioctlsocket(fd,FIONBIO,(unsigned long*)&en);  
	if(ret!=0)
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
	}
	return ret;
}
#else
{//linux
  	int curr_flags = fcntl(fd, F_GETFL);
  	if (curr_flags<0)
  	{
  	  	return -1;
  	}
	if(en)
	{
  		curr_flags |= O_NONBLOCK;
	}
	else
	{
		curr_flags &= ~O_NONBLOCK;
	}
  	return fcntl(fd, F_SETFL, curr_flags);
  
}
#endif

/** 
 *   @brief    带超时参数的connect,除了timeout参数以外其它参数和connect完全一样
 *   @param  fd:已经创建好的socket
 *   @param  serv_addr:要连接的地址及端口号
 *   @param  addrlen:serv_addr结构的长度
 *   @param  timeout:超时返回时间(秒)
 *   @return   0表示成功,负值表示失败
 */ 
int  connect_timeout(SOCK_FD  fd,  const  struct sockaddr *serv_addr, socklen_t    addrlen,int timeout)
{
	int rc,ret;
	int sockerr;
	socklen_t sklen;
	fd_set writefds;
	struct timeval	timeval;
	if((fd<0)||(serv_addr==NULL))
	{
		errno=EINVAL;
		return -1;
	}
	rc=net_set_noblock(fd,1);
	if(rc<0)
	{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
		return -1;
	}//
	FD_ZERO(&writefds);
	rc=-1;
	do{	

		errno=0;
 		rc=connect(fd,serv_addr,addrlen);
        if(rc<0)
		{
#ifdef _WIN32
		errno=WSAGetLastError();
#endif

#ifndef _WIN32
			if(errno==EINPROGRESS)	//linux
#endif
			{
				while(1)
				{
				//printf("还在进行连接,timeout=%d\n",timeout);
				timeval.tv_sec=timeout;
				timeval.tv_usec=0;
				FD_SET(fd,&writefds);
				ret=select(fd+1,NULL,&writefds,NULL,&timeval);
				if(ret==0)
				{
					rc=-1;
					//printf("连接超时\n");
#ifdef _WIN32
					errno=WSAGetLastError();
#endif
					break;
				}

				if(FD_ISSET(fd,&writefds))
				{
					sockerr=-1;
					sklen=sizeof(int);
                //下面的一句一定要，主要针对防火墙 
					ret=getsockopt(fd, SOL_SOCKET, SO_ERROR,(void*)&sockerr, &sklen);
					//printf("收到 fd连接信号sockerr=%d %s\n",sockerr,strerror(sockerr));
					if(sockerr==0)
					{
						rc=0;
						//printf("连接成功!!!!!!!!!!!!!!!\n");
						break;
					}
					else if(sockerr!=EINPROGRESS)
					{
						rc=-1;
						errno=sockerr;
						break;
					}
					continue;
				}		
				else
				{
					rc=-1;
					break;
				}
				}
			}
			//else
			//	printf("连接错误%d\n",errno);
		}
	}while(0);
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
	net_set_noblock(fd,0);
	return rc;
}

/** 
 *   @brief    连接远程tcp服务地址
 *   @param  addr_str:描述远程服务地址的字符串
 *   @param  port:远程tcp服务的端口号
 *   @param   timeout:tcp连接的超时时间(秒)
 *   @return   正值表示已连接好的描述符，负值表示出错
 */ 
SOCK_FD tcp_connect_addr(const char *addr_str,int port,int timeout)
{
	SOCK_FD	rmt_sock=-1;
	int	ret=-1;
	//char *remoteHost = NULL;
	struct hostent *hostPtr = NULL;
	 struct sockaddr_in serverName = { 0 };
	if((addr_str==NULL)||(port<0)||(timeout<0))
	{
		errno=EINVAL;
		return -1;
	}
#ifdef _WIN32
	   {
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;
 
        wVersionRequested = MAKEWORD( 2, 0 );
 
        err = WSAStartup( wVersionRequested, &wsaData );
        if ( err != 0 ) {
            /* Tell the user that we couldn't find a usable */
            /* WinSock DLL.                               */
            //printf("can't initialize socket library\n");
			errno=WSAGetLastError();
			return -1;
        }
    }

#endif


	hostPtr = gethostbyname(addr_str); /* struct hostent *hostPtr. */ 
    if (NULL == hostPtr)
    {
		hostPtr = gethostbyaddr(addr_str, strlen(addr_str), AF_INET);
        if (NULL == hostPtr) 
        {
#ifdef _WIN32
			errno=WSAGetLastError();
#endif
//        	printf("Error resolving server address:%s\n",addr_str);
			return -1;
        }			
    }
	rmt_sock=socket(PF_INET, SOCK_STREAM,IPPROTO_TCP);
	if(rmt_sock<0)
	{
		//printf("create socket err! rmt_sock=%d errno=%d:%s\n",rmt_sock,errno,strerror(errno));
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
			return -1;
	}
	serverName.sin_family = AF_INET;
    serverName.sin_port = htons((unsigned short)port);
	//serverName.sin_addr.s_addr=htonl(addr_str);
    (void) memcpy(&serverName.sin_addr,hostPtr->h_addr,hostPtr->h_length);
    ret = connect_timeout(rmt_sock,(struct sockaddr*) &serverName,sizeof(serverName),timeout);
	
    if (ret<0)
    {
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
    	close(rmt_sock);	
		return -1;
   	}
	return rmt_sock;	
		
}


/** 
 *   @brief    连接远程tcp服务地址
 *   @param  addr_str:描述远程服务地址的字符串
 *   @param  port:远程tcp服务的端口号
 *   @param   timeout:tcp连接的超时时间(秒)
 *   @return   正值表示已连接好的描述符，负值表示出错
 */ 
SOCK_FD tcp_connect_block(const char *addr_str,int port,int timeout)
{
	SOCK_FD	rmt_sock=-1;
	int	ret=-1;
	//char *remoteHost = NULL;
	struct hostent *hostPtr = NULL;
	 struct sockaddr_in serverName = { 0 };
	if((addr_str==NULL)||(port<0)||(timeout<0))
	{
		errno=EINVAL;
		return -1;
	}
#ifdef _WIN32
	   {
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;
 
        wVersionRequested = MAKEWORD( 2, 0 );
 
        err = WSAStartup( wVersionRequested, &wsaData );
        if ( err != 0 ) {
            /* Tell the user that we couldn't find a usable */
            /* WinSock DLL.                               */
            //printf("can't initialize socket library\n");
			errno=WSAGetLastError();
			return -1;
        }
    }

#endif


	hostPtr = gethostbyname(addr_str); /* struct hostent *hostPtr. */ 
    if (NULL == hostPtr)
    {
		hostPtr = gethostbyaddr(addr_str, strlen(addr_str), AF_INET);
        if (NULL == hostPtr) 
        {
#ifdef _WIN32
			errno=WSAGetLastError();
#endif
//        	printf("Error resolving server address:%s\n",addr_str);
			return -1;
        }			
    }
	rmt_sock=socket(PF_INET, SOCK_STREAM,IPPROTO_TCP);
	if(rmt_sock<0)
	{
		//printf("create socket err! rmt_sock=%d errno=%d:%s\n",rmt_sock,errno,strerror(errno));
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
			return -1;
	}
	serverName.sin_family = AF_INET;
    serverName.sin_port = htons((unsigned short)port);
    (void) memcpy(&serverName.sin_addr,hostPtr->h_addr,hostPtr->h_length);
    ret = connect(rmt_sock,(struct sockaddr*) &serverName,sizeof(serverName));
    if (ret<0)
    {
#ifdef _WIN32
		errno=WSAGetLastError();
#endif
    	close(rmt_sock);	
		return -1;
   	}
	return rmt_sock;	
		
}


/** 
 *   @brief    将缓冲区的数据全部写入一个网络连接中
 *   @param  fd:目标文件描述符
 *   @param  *buf:指向要发送的缓冲区的指针
 *   @param  len:要发送的缓冲区中的有效信息字节数
 *   @return   正值表示成功写入的字节数,负值代表发送异常,错误码待定
 */ 
int net_write_buf(SOCK_FD fd,void *buf,int len)
{
	int left,writted=0; 
	char *wp; 
	int err;
	wp=buf; 
	left=len; 
	while(left>0) 
	{ 
		/* 开始写*/ 
		writted=write(fd,wp,left); 
		if(writted<=0) /* 出错了*/ 
		{ 
#ifdef _WIN32
			errno=WSAGetLastError();
#endif
			err=errno;
			if(err==EINTR) /* 中断错误 我们继续写*/ 
				writted=0; 
#ifndef _WIN32
			else if(err==EPIPE)	//网络连接出问题
			{
				return -EPIPE;
			}
#endif
			else
			{
				if(err>0)	/* 其他错误 没有办法*/ 
					return -err;
				else
					return -1;

			}
		}
		left-=writted; 
		wp+=writted; /* 从剩下的地方继续写 */ 
	} 
	return(writted);
}


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
int net_read_buf(SOCK_FD fd,void *buf,int len) 
{ 
	int left; 
	int ret; 
	int	err;
	char *rp; 

	left=len; 
	rp=buf;
	while(left>0) 
	{ 
		ret=read(fd,rp,left); 
		if(ret<=0) 
		{ 
#ifdef _WIN32
			errno=WSAGetLastError();
			err=WSAGetLastError();
#else
			err=errno;
#endif
			if(ret==0)
                        return -140;    ///远程断开连接
			if(err==EINTR)
			{
				ret=0;
				continue;
			}			
			else if(err==ETIMEDOUT)
			{
#ifdef _WIN32
				//接收超时
				if(left<len)
					return (len-left);
				else
					return -EAGAIN;
#else
				//keepalive超时
				return (0-ETIMEDOUT);
#endif
			}
			else if(err==EHOSTUNREACH)
				return (0-EHOSTUNREACH);
			else
			{
				//接收超时
				if(err==EAGAIN)
				{				
					if(left<len)
						return (len-left);
					else
						return -EAGAIN;
				}
				if(err>0)
					return -err;
				else
					return -1;
			}
		} 
		left-=ret; 
		rp+=ret; 
	} 
	return(len-left); 
}

/*
	函数说明:
	设置tcp连接的属性,一般用于connect或accept刚成功后，包括：
	打开keepalive，时间设置为15秒发送一次探测包，失败后间隔10秒重试3次
	关闭nagle算法
	关闭linger

	recv_timeout表示接收数据没有响应超时时间
	返回0表示成功,负值表示失败
*/
int set_tcp_socket_attr(int fd,int recv_timeout)
{
	int ret=0;
        ret+=net_activate_keepalive(fd);
	ret+=net_set_keepalive_time(fd,3,15,10);
       ret+=net_set_recv_timeout(fd,recv_timeout);
    ret+=net_set_nodelay(fd);        
    ret+=net_set_linger(fd,0);
	if(ret==0)
		return 0;
	else
		return -1;
}


/** 
 *   @brief    创建一个UDP描述符(发送和接收)
 *   @param  ip_str :准备侦听的地址字符串
 *   @param  port    :准备侦听的端口号
 *   @return    负值表示失败，其它值表示文件描述符       
 */ 
int udp_create(IN char *ip_str,IN int port)
{
    int fd;
     struct sockaddr_in addr;
#ifdef _WIN32
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;
 
        wVersionRequested = MAKEWORD( 2, 0 );
 
        err = WSAStartup( wVersionRequested, &wsaData );
        if ( err != 0 ) {
            /* Tell the user that we couldn't find a usable */
            /* WinSock DLL.                               */
            //printf("can't initialize socket library\n");
			errno=WSAGetLastError();
			return -1;
        }
    }
#endif


     
    fd =socket(AF_INET,SOCK_DGRAM,0); //创建socket
    if(fd == -1)
    {
            perror("Opening socket");
            return -1;
    }    
    memset(&addr,0,sizeof(addr));
    addr.sin_family= AF_INET;
    addr.sin_addr.s_addr=inet_addr(ip_str); //填充地址及端口
    addr.sin_port=htons((short)port);

   if(net_activate_reuseaddr(fd)<0)
    {
            perror("setsocketopt:SO_REUSEADDR");
            return -1;
    }
    if(bind(fd,(struct sockaddr *)&addr,sizeof(addr))<0)
    {
        perror("bind");
        return -1;
    }

    return fd;
    
}

/** 
 *   @brief    打开描述符的组播功能
 *   @param  fd 之前用udp_create获取的描述符
 *   @param  multicast_addr :组播地址
 *   @return    0表示成功，负值表示失败
 */ 
int udp_add_multicast(int fd,char *multicast_addr)
{
    int loop;
    struct ip_mreq command;
	int ret;
#if 0
#ifdef _WIN32
	//设置该套接字为广播类型，
	bool opt=true;
	if(setsockopt(mfd,SOL_SOCKET,SO_BROADCAST,(char FAR *)&opt,sizeof(opt)) == SOCKET_ERROR )
	{
		printf
	}
#endif
#endif               
    loop=1;
    if(setsockopt(fd,IPPROTO_IP,IP_MULTICAST_LOOP,(char*)&loop,sizeof(loop))<0)
    {
            perror("setsocketopt:IP_MULTICAST_LOOP");
            return -1;
    }
    //join multicast group
    command.imr_multiaddr.s_addr = inet_addr(multicast_addr);
    command.imr_interface.s_addr = htonl(INADDR_ANY);

    if(command.imr_multiaddr.s_addr == -1)
    {
        perror("not a legal multicast address!");
        //exit(1);
        return -1;
    }

    if(setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char*)&command,sizeof(command))<0)
    {
        perror("setsockopt:IP_ADD_MEMBERSHIP");
        return -1;
    }
    else
	{
        //printf("IP_ADD_MEMBERSHIP success \n");    
	}
    return 0;
}
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
int udp_send_data(IN int fd, IN void *data, IN int  len, IN int flags, IN struct sockaddr_in *to)
{
    socklen_t tolen=sizeof(struct sockaddr);
    return sendto(fd,data,len,flags,(struct sockaddr *)to, tolen);
}


/** 
 *   @brief    接收一包UDP数据
 *   @param  fd 之前用udp_create获取的描述符
 *   @param  buf:准备存放数据的缓冲区
 *   @param  len:缓冲区长度
 *   @param  flags:保留，调用时写0
 *   @param  from:接收到的数据的远程地址
 *   @return   正值表示接收到buf中的有效字节数，负值表示失败
 */ 
int udp_recv_data(IN int fd,OUT void *buf, IN int len, IN int flags, OUT struct sockaddr_in *from)
{
    socklen_t recvlen=sizeof(struct sockaddr);     
    return recvfrom(fd,buf,len,flags,(struct sockaddr *)from,&recvlen);
}

