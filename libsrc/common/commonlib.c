/*	

 */
/*
 * 
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <gtlog.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <netdb.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>

#include <fcntl.h>      
#include <sys/file.h>
#include <termios.h>    
#include <commonlib.h>
#include "hi_rtc.h"
#ifndef gtloginfo
#include <syslog.h>

//#define gtlog  syslog		//系统日志信息记录
#define gtlog syslog
//一般性信息
#define gtloginfo(args...) syslog(LOG_INFO,##args)	//记录一般信息
//严重的错误信息
#define gtlogfault(args...) syslog(LOG_CRIT,##args)	//
//错误信息
#define gtlogerr(args...) syslog(LOG_ERR,##args)	//
//警告信息
#define gtlogwarn(args...) syslog(LOG_WARNING,##args)

#define gtlogdebug(args...) syslog(LOG_DEBUG,##args)
#endif


#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
//#include <linux/rtc.h>
//#include "hi_rtc.h"
#include "gpio_i2c.h"

/*把bcd码转成十进制*/
unsigned char inline bcdtobin(unsigned char hex)
{
	return (  (hex>>4)*10 + (hex&0xf) );
}

unsigned char inline bintobcd(unsigned char bin)
{
	return ( bin/10*16+bin%10  );
}

static int RTC_INIT = 0;

/*如果rtc没有工作就让它工作，主要是寄存器0最高位置0*/
void init()
{
	int fd = open("/dev/gpioi2c", 0);
    if(fd<0)
    {
    	printf("Open gpioi2c error!\n");
    	return ;
    }

	int value=0;
	int ret;

	/*读取值*/
	value = 0;
	value |=0xd0<<24;	//device addr
	value |=0x0<<16;	//reg addr
	ret = ioctl(fd, GPIO_I2C_READ, &value);//年
	if(ret<0)
		printf("ioctl write2\n");
	//printf("value:%#X \n",value);

	if(value&0x80)//如果最高位为1则置0写入
	{
		/*写入值*/
		value &=~0x80;  //把rtc寄存器的最高位置0，让rtc开始工作
		//value |=0x80; //把rtc寄存器的最高位置1，让rtc停止工作
		ret = ioctl(fd, GPIO_I2C_WRITE, &value);//年
		if(ret<0)
				printf("ioctl write2\n");
		//printf("value:%#X \n",value);
	}

	//释放资源
	close(fd);
}


//获取空闲磁盘空间大小，以M为单位
long get_disk_free(char *mountpath)
{
	 struct statfs  buf;
	 if(mountpath==NULL)
	 	return -1;
	 if(statfs(mountpath,&buf)<0)
	 {
	 	printf("error at check_disk, mountpath %s\n",mountpath);
	 	//gtlogwarn("获取空闲磁盘大小时失败，路径%s\n",mountpath);
	 	return -errno;
	 }
	return (buf.f_bavail*(buf.f_bsize>>10)>>10);
}

//获取磁盘总容量M为单位
long get_disk_total(char *mountpath)
{
	 struct statfs  buf;
	 if(mountpath==NULL)
	 	return -1;
	 if(statfs(mountpath,&buf)<0)
	 {
	 	printf("get_disk_total error at check_disk,path:%s\n",mountpath);
	 	gtlogwarn("获取磁盘总大小时失败，路径%s,%d:%s\n",mountpath,errno,strerror(errno));
	 	return -1;
	 }
	return (buf.f_blocks*(buf.f_bsize>>10))>>10;
}



//检查文件是否存在，如果存在则返回1否则返回0
int check_file(char *file)
{
	if(file==NULL)
		return 0;
	if(access(file,F_OK)<0)
		return 0;
	return 1;
}
/**************************************************************************
  *函数名	:get_file_lines
  *功能	:获取指定文件名的行数
  *参数	: FileName:要统计行数的文件名
  *返回值	:正值表示文件的行数，负值表示出错
  *注:应用程序名中不能有"grep"字样，否则结果会不正确
  *************************************************************************/
int get_file_lines(char *FileName)
{
	FILE *Fp=NULL;
	char  ReadBuf[256];
	char  *PR;
	char  *p;
	int Lines=0;
	if(FileName==NULL)
		return -EINVAL;
	Fp=fopen(FileName,"r");
	if(Fp==NULL)
		return -errno;
	//获取文件行数
	while(1)
	{
		PR=fgets(ReadBuf,sizeof(ReadBuf),Fp);
		if(PR==NULL)
		{
			break;
		}
		p=strstr(ReadBuf,"grep");
		if(p==NULL)
			Lines++;
	}

	fclose(Fp);
	return Lines;	
}

int get_ps_threadnum(char *psName,char *FileName)
{
	FILE *Fp=NULL;
	char  ReadBuf[256];
	char  result[10];
	char  cmdline[100];
	char buff[50]; 
	char  *PR;
	char  *p;
	char  *p1;
	char  *p2;
	char  *threadid;
	char  *other;
	FILE *fstream=NULL;       
    int  threadnum=0;
	int Lines=0;
	if(FileName==NULL || psName == NULL)
		return -EINVAL;

	Fp=fopen(FileName,"r");
	if(Fp==NULL)
		return -errno;

	memset(result,0,10);
	while(1)
	{
		PR=fgets(ReadBuf,sizeof(ReadBuf),Fp);
		if(PR==NULL)
		{
			break;
		}
		p=strstr(ReadBuf,"grep");
		if(p==NULL)
		{
			//lc do 找到指定进程
			//sscanf(ReadBuf," %s %s",threadid,other);
			p1=strstr(ReadBuf,"root");
			strncpy(result,ReadBuf,(p1-ReadBuf-1));
			printf("result is %s\n",result);
			Lines++;
		}
	}
	fclose(Fp);
	//lc do 去掉result中存在的空格
	p1 = strstr(result," ");
	if(p1 != NULL)
		p2 = p1 + 1;
	else
		p2 = result;
	
	if(Lines == 0 )
	{
		printf("err no lines was found!\n");
		return -1;
	}
	memset(cmdline,0,100);
	memset(buff,0,sizeof(buff));  
	sprintf(cmdline,"cat /proc/%s/status | grep Threads | awk '{print $2}' ",p2);

	if(NULL==(fstream=popen(cmdline,"r")))       
    {      
        printf("execute command failed: %s",strerror(errno));       
        return -1;       
    } 

	if(NULL!=fgets(buff, sizeof(buff), fstream))      
    {      
        //printf("threadnum is %s\n",buff);   
        printf("Process %s has %s threads!\n",psName,buff);
        //gtloginfo("Process %s has %s threads!\n",psName,buff);
    }      
    else     
    {     
        pclose(fstream);     
        return -1;     
    } 
	
	threadnum = atoi(buff);

	pclose(fstream);
	return threadnum;		
}


//关闭所有已经打开的资源(除了指定的描述符以外)
void CloseAllResExc(int Fd)
{
	int MaxOpen,i;
	MaxOpen=sysconf(_SC_OPEN_MAX);
	if(MaxOpen<0)
	{
		printf("sysconf error !\n");
	}
	//stdin stdout stderr不关闭
	for (i=3;i<MaxOpen;i++)
	{
		if(i!=Fd)
			close(i);
	}	
}

//关闭所有已经打开的资源
void close_all_res(void)
{
	int maxopen,i;
	maxopen=sysconf(_SC_OPEN_MAX);
	if(maxopen<0)
	{
		printf("sysconf error !\n");
	}
	//stdin stdout stderr不关闭
	for (i=3;i<maxopen;i++)
	{
		close(i);
	}	
}
int deamon_init(void)
{
	pid_t pid;
	int i;
	int maxopen;
	if((pid=fork())<0)
	{
		printf("deamon fork error !\n");
		return -1;
	}
	if(pid!=0)
		exit(0);
	setsid();
	chdir("/");
	umask(0);

	maxopen=sysconf(_SC_OPEN_MAX);
	if(maxopen<0)
	{
		printf("sysconf error !\n");
		return -1;
	}
	for (i=3;i<maxopen;i++)
	{
		close(i);
	}
	close(0);	//保留标准输出和错误输出
	
	
	return 0;
}

//#define DISKMAN_LOCK_FILE "/lock/vserver/diskman"
//#define IPMAIN_LOCK_FILE "/lock/vserver/vsmain"
//创建并加锁文件
//返回打开的文件描述符
int create_and_lockfile(char *lockfile)
{
	int lf;
	char dir[100];
	char sbuf[120];
	char *rch;
	if(lockfile==NULL)
		return -1;
	lf=open(lockfile,O_RDWR|O_CREAT,0640);//打开锁文件,若不存在则建立
	if(lf<0)
	{
		strncpy(dir,lockfile,strlen(lockfile));
		rch=strrchr(dir,'/');
		if(rch!=NULL)
		{
			*rch='\0';
			sprintf(sbuf,"mkdir %s -p\n",dir);
			system(sbuf);
			
		}
		//mkdir("/lock",0770);
		//mkdir("/lock/vserver",0770);
		lf=open(lockfile,O_RDWR|O_CREAT,0640);
		if(lf<0)
		{
			printf("create lock file:%s error!\n",lockfile);
			gtloginfo("创建锁文件%s失败\n",lockfile);
			return -2;
		}
	}
	if(flock(lf,LOCK_EX|LOCK_NB)!=0)//将进程标志文件锁定，以防止多次运行同一程序的多个副本
	{
		close(lf);//added
		return -1;
	}		
	return lf;
}
/**********************************************************************************************
 * 函数名	:create_lockfile_save_version()
 * 功能	:创建锁文件并将其加锁,返回打开的文件描述符,按照格式存放版本号信息
 * 参数	:lockfile:要加锁的文件名,
 *			 version:进程版本号字符串
 * 返回值	:正值表示打开的文件描述符,负值表示出错
 * 本函数一般用于判断是否有某进程的副本正在运行
 * 文件格式:
 *第一行:进程号
 *第二行:version:进程版本号
 **********************************************************************************************/
int create_lockfile_save_version(char *lockfile,char *version)
{
	char pbuf[100];
	int fd=-1;
	if(lockfile==NULL)
		return -EINVAL;
	fd=create_and_lockfile(lockfile);
	if(fd<0)
		return fd;
	
	sprintf(pbuf,"%d\nversion:%s\n",getpid(),version);
	write(fd,pbuf,strlen(pbuf)+1);					//将进程的id号存入锁文件中
	return fd;
}
#if 0
////added by lsk 2006 -11-7
//强制加锁文件  
//输入 fd 文件控制字 ,cmd 命令, wait 等待标志
//返回0 成功 -1 失败
// cmd = F_RDLCK 读禁止 ； F_WRLCK 写禁止; F_UNLCK 解除锁定
// wait = 0 无法锁定则立即返回， =1 等待锁定
int force_lockfile(int fd, short int cmd, int wait)
{
	int ret;
	struct flock tp;
	if(fd<0)
		return -1;
//	printf("cmd = %d , F_UNLCK = %d, wrlck = %d, rdlck = %d\n", cmd, F_UNLCK, F_WRLCK,F_RDLCK);
	tp.l_type = cmd;
	tp.l_whence = SEEK_SET;
	tp.l_len = 0;
	tp.l_pid = getpid();
	if(wait ==0)
	return fcntl(fd , F_SETLK, &tp);
	if(wait ==1)
	return fcntl(fd , F_SETLKW, &tp);
	return -1;
}
#endif
/**********************************************************************************************
 * 函数名	:write_process_info()
 * 功能	:将进程的版本号信息以系统统一的格式写入到一个打开的文件中
 * 参数	:lfd:被写入的文件描述符
 *			 version:字符串表示的版本信息
 * 返回值	:0表示成功负值表示出错
 **********************************************************************************************/
int write_process_info(int fd,char *version)
{
	char pbuf[100];
	if((fd<0)||(version==NULL))
		return -EINVAL;
	sprintf(pbuf,"%d\nversion:%s\n",getpid(),version);
	write(fd,pbuf,strlen(pbuf)+1);//将进程的id号存入锁文件中
	return 0;
}

int fd_write_buf(int fd,void *buf,int len)
{
	int left,writted; 
	char *wp; 
	int err;
	wp=buf; 
	left=len; 
	while(left>0) 
	{ 
		/* 开始写*/ 
		writted=write(fd,wp,left); 
		//printf("KKK fd_write_buf ret =%d,left=%d\n",writted,left);
		if(writted<=0) /* 出错了*/ 
		{ 
			err=errno;
			if(err==EINTR) /* 中断错误 我们继续写*/ 
				writted=0; 
			else if(err==EPIPE)	//网络连接出问题
			{
				return -EPIPE;
			}
			else
				return(0-err);/* 其他错误 没有办法*/ 
	 	} 
		left-=writted; 
		wp+=writted; /* 从剩下的地方继续写 */ 
	} 
	return(writted);
}

/*   
	从一个文件描述符里面读取指定长度的数据
*/
int fd_read_buf(int fd,void *buf,int len) 
{ 
	int left; 
	int ret; 
	char *rp; 

	left=len; 
	rp=buf;
	while(left>0) 
	{ 
		ret=read(fd,rp,left); 
		if(ret<0) 
		{ 
			if(errno==EINTR) 
				ret=0; 
			else if(errno==ETIMEDOUT)
				return (0-ETIMEDOUT);
			else if(errno==EHOSTUNREACH)
				return (0-EHOSTUNREACH);
			else
			{
				#if 0
				if(errno==EAGAIN)
				{
					printf("common lib fd_read_buf EAGAIN !!!!!\n");
					printf("buf have %d    \n",(len-left));
					rp=(char*)buf;
					for(ret=0;ret<(len-left);ret++)
						printf("0x%02x ",rp[ret]);
					printf("\n");
				}
				#endif
				return(0-errno); 
			}
		} 
		else if(ret==0) 
			//break;fixbug 如果在读了n个字节后对方close则无法从while出来
			return 0;
		left-=ret; 
		rp+=ret; 
	} 
	return(len-left); 
}
#if 0
/** 
 *   @brief     在指定的地址和端口上创建tcp服务socket
 *   @param  svr_addr 用于侦听的地址 INADDR_ANY表示在所有地址侦听
 *   @param  port   侦听的端口号
 *   @return   正值表示创建的socket描述符,负值表示失败
 */ 
#include <netinet/in.h>
int create_tcp_listen_port(unsigned long svr_addr,int port)
{
    int                         fd;
    struct sockaddr_in svr;
    fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd<0)
    {
        printf("can't create socket:%s!\n",strerror(errno));
        return -errno;
    }
    
///如果服务器终止后,服务器可以第二次快速启动而不用等待一段时间
 	net_activate_reuseaddr(fd);
	
	bzero(& svr, sizeof(struct sockaddr_in));
 	svr.sin_family=AF_INET; 
 	svr.sin_port=htons(port); 
 	svr.sin_addr.s_addr=htonl(svr_addr); 

 	if(bind(fd,(struct sockaddr *)&svr,sizeof(svr))<0) 
  	{   	    
	        printf("Bind Error:%s\n\a",strerror(errno)); 	
               close(fd);
	        return -errno;
  	} 
       return fd;
}

int net_set_recv_timeout(int fd,int second)
{
	struct timeval timeout;
	timeout.tv_sec=second;
	timeout.tv_usec=0;
	return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

}
int net_set_snd_timeout(int fd,int second)
{
	struct timeval timeout;
	timeout.tv_sec=second;
	timeout.tv_usec=0;
	return setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

}
int	net_activate_keepalive(int fd)
{
  int keepalive = 1;
  return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
}

int net_activate_reuseaddr(int fd)
{
  int reuseaddr = 1;
  return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,sizeof(reuseaddr));
}

int net_set_nodelay(int fd)
{
  int nodelay = 1;
  return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
}
#if EMBEDED==0
int net_set_quickack(int fd)
{
	int ack=1;
	return setsockopt(fd,IPPROTO_TCP,TCP_QUICKACK,&ack,sizeof(ack));
}
#endif


int net_activate_oobinline(int fd)
{
  int oob_inline = 1;
 return setsockopt(fd, SOL_SOCKET, SO_OOBINLINE, &oob_inline, sizeof(oob_inline));
}

int net_set_iptos_throughput(int fd)
{
  int tos = IPTOS_THROUGHPUT;
  /* Ignore failure to set (maybe this IP stack demands privilege for this) */
  return setsockopt(fd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));
}

int net_set_linger(int fd,int en)
{
  	int retval;
  	struct linger the_linger;
	if(en)
	{
  		the_linger.l_onoff = 1;
  		the_linger.l_linger = INT_MAX;
	}
	else
	{
		the_linger.l_onoff = 0;
  		the_linger.l_linger = 0;
	}
  	return setsockopt(fd, SOL_SOCKET, SO_LINGER, &the_linger,sizeof(the_linger));
}

int net_set_noblock(int fd,int en)
{
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

#if 0
int net_set_tcp_keepalive_time(int fd,int second)
{//实际上到了 time+interval*probes 才会检测出
	int file;
	char buf[16];
	int time,probes,interval;
	probes=3;
	interval=10;
	time=second;
	file=open("/proc/sys/net/ipv4/tcp_keepalive_time",O_WRONLY|O_TRUNC|O_CREAT);
	if(file<0)
		return -1;
	sprintf(buf,"%d\n",time);
	write(file,buf,strlen(buf));
	close(file);
	file=open("/proc/sys/net/ipv4/tcp_keepalive_probes",O_WRONLY|O_TRUNC|O_CREAT);
	if(file<0)
		return -1;
	sprintf(buf,"%d\n",probes);
	write(file,buf,strlen(buf));
	close(file);

	file=open("/proc/sys/net/ipv4/tcp_keepalive_intvl",O_WRONLY|O_TRUNC|O_CREAT);
	if(file<0)
		return -1;
	sprintf(buf,"%d\n",interval);
	write(file,buf,strlen(buf));
	close(file);
	return 0;
	
}
#endif
int net_set_tcp_sendbuf_len(int fd,int size)
{
	int bufsize;
	int ret;
	if((fd<0)||(size<0))
		return -EINVAL;
	bufsize=size/2;
	//lensize=sizeof(bufsize);
	ret= setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));	
	if(ret!=0)
		return -errno;
	return 0;
}
int net_get_tcp_sendbuf_len(int fd)
{
	int bufsize,lensize;
	int rc;
	if(fd<0)
		return -EINVAL;
	lensize=sizeof(bufsize);
	rc=getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &bufsize, &lensize);
	if(rc==0)
		return bufsize;
	else
		return rc;
}
int net_set_tcp_recvbuf_len(int fd,int size)
{
	int bufsize;
	if((fd<0)||(size<0))
		return -1;
	bufsize=size/2;
	return setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));	
}
int net_get_tcp_recvbuf_len(int fd)
{
	int bufsize,lensize;
	int rc;
	if(fd<0)
		return -1;
	lensize=sizeof(bufsize);
	rc=getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufsize, &lensize);
	if(rc==0)
		return bufsize;
	else
		return rc;
}
//设置发送缓冲区下限
int	net_set_sock_send_low(int fd,int size)
{
	int bufsize;
	if((fd<0)||(size<0))
		return -1;
	bufsize=size;
	return setsockopt(fd, SOL_SOCKET, SO_SNDLOWAT, &bufsize, sizeof(bufsize));	
}
int	net_get_sock_send_low(int fd)
{
	int bufsize,lensize;
	int rc;
	if(fd<0)
		return -1;
	lensize=sizeof(bufsize);
	rc=getsockopt(fd, SOL_SOCKET, SO_SNDLOWAT, &bufsize, &lensize);
	if(rc==0)
		return bufsize;
	else
		return rc;	
}
int	net_get_sock_recv_low(int fd)
{
	int bufsize,lensize;
	int rc;
	if(fd<0)
		return -1;
	lensize=sizeof(bufsize);
	rc=getsockopt(fd, SOL_SOCKET, SO_RCVLOWAT, &bufsize, &lensize);
	if(rc==0)
		return bufsize;
	else
		return rc;	
}
//设置接收缓冲区下限
int	net_set_sock_recv_low(int fd,int size)
{
	int bufsize;
	if((fd<0)||(size<0))
		return -1;
	bufsize=size;
	return setsockopt(fd, SOL_SOCKET, SO_RCVLOWAT, &bufsize, sizeof(bufsize));	
}

#include <unistd.h>
#include <sys/ioctl.h>
int get_fd_in_buffer_num(int fd)
{//获取接收缓冲区的有效数据
	int ret;
	int cnt=100;
	ret=ioctl(fd,FIONREAD,&cnt);
	if(ret<0)
		return -errno;
	else
	{
		return cnt;
	}
}
int	get_fd_out_buffer_num(int fd)
{//获取发送缓冲区中未发送出的数据数
	int ret;
	int cnt=0;
	ret=ioctl(fd,TIOCOUTQ  ,&cnt);
	if(ret<0)
		return -1;
	else
		return cnt;
}




//返回文件描述符的ip地址字符串
//如果获取失败则返回NULL
char *GetPeerIpStr(int Fd)
{
	int Ret;
	struct sockaddr_in RmtAddr;
	int AddrLen=sizeof(struct sockaddr);
	
	Ret=getpeername(Fd,(struct sockaddr *)&RmtAddr,&AddrLen);
	if(Ret==0)
		return inet_ntoa(RmtAddr.sin_addr);
	else
		return NULL;
}


//带超时的connect
int  connect_timeout(int  fd,  const  struct sockaddr *serv_addr, socklen_t    addrlen,int timeout)
{
	int rc,ret;
	int sockerr;
	socklen_t sklen;
	fd_set writefds;
	int errtimes=0;
	struct timeval	timeval;
	if((fd<0)||(serv_addr==NULL))
		return -1;
	rc=net_set_noblock(fd,1);
	if(rc<0)
		{}//
	FD_ZERO(&writefds);
	rc=-1;
	do{		
		rc=connect(fd,serv_addr,addrlen);
		if(rc<0)
		{
			if(errno==EINPROGRESS)
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
					break;
				}
				if(FD_ISSET(fd,&writefds))
				{
					sockerr=-1;
					sklen=sizeof(int);
					getsockopt(fd, SOL_SOCKET, SO_ERROR,(void*)&sockerr, &sklen);
					//printf("收到 fd连接信号sockerr=%d %s\n",sockerr,strerror(sockerr));
					if(sockerr==0)
					{
						rc=0;
						//printf("连接成功!!!!!!!!!!!!!!!\n");
						break;
					}
					else
					{
						if(++errtimes>5)
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
	net_set_noblock(fd,0);
	return rc;
}

int tcp_connect_addr(const char *addr_str,int port,int timeout)
{
	int	rmt_sock=-1;
	int	ret=-1;
	char *remoteHost = NULL;
	struct hostent *hostPtr = NULL;
	 struct sockaddr_in serverName = { 0 };
	if((addr_str==NULL)||(port<0)||(timeout<0))
		return -EINVAL;

	hostPtr = gethostbyname(addr_str); /* struct hostent *hostPtr. */ 
    	if (NULL == hostPtr)
    	{
       	 hostPtr = gethostbyaddr(addr_str, strlen(remoteHost), AF_INET);
        	if (NULL == hostPtr) 
        	{
        		printf("Error resolving server address:%s\n",addr_str);
			if(errno>0)
        			return -errno;
			else
				return -errno;
        	}			
    	}
	rmt_sock=socket(PF_INET, SOCK_STREAM,IPPROTO_TCP);
	if(rmt_sock<0)
	{
		printf("create socket err! rmt_sock=%d errno=%d:%s\n",rmt_sock,errno,strerror(errno));
		return -errno;
	}
	serverName.sin_family = AF_INET;
    	serverName.sin_port = htons(port);
    	(void) memcpy(&serverName.sin_addr,hostPtr->h_addr,hostPtr->h_length);
    	ret = connect_timeout(rmt_sock,(struct sockaddr*) &serverName,sizeof(serverName),timeout);
    	if (ret<0)
    	{
    		close(rmt_sock);
        	return -errno;
   	}
	return rmt_sock;	
		
}
#endif


#include <sys/types.h>
#include <sys/stat.h>
//获取可执行文件的名称及路径存放于ExecName中
//ExecName缓冲区的长度应该足够长,如100字节
int GetExecName(char *ExecName,int BufLen)
{
	int Ret;
	pid_t Pid;
	char filename[100];
	struct stat fbuf;
	if(ExecName==NULL)
		return -EINVAL;
	Pid=getpid();
	sprintf(filename,"/proc/%d/exe",(int)Pid);
	Ret=access(filename,F_OK);
	if(Ret<0)
		return -ENOENT;
	Ret=lstat(filename,&fbuf);
	if(Ret<0)
		return -1;
	if(!S_ISLNK(fbuf.st_mode))
		return -EINVAL;
	if((fbuf.st_size+1)>BufLen)
		return -EINVAL;
	Ret=readlink(filename,ExecName,BufLen);
	if(Ret<=0)//!=fbuf.st_size)
	{
		//printf("readlink Ret=%d fbuf.st_size=%d!!\n",Ret,(int)fbuf.st_size);
		return -1;
	}
	ExecName[Ret]='\0';
	return 0;
	
}


int clr_option_ctrl_char(struct termios *options)
{
	options->c_cc[VINTR]    = 0;     /* Ctrl-c */ 
	options->c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	options->c_cc[VERASE]   = 0;     /* del */
	options->c_cc[VKILL]    = 0;     /* @ */
	options->c_cc[VEOF]     = 4;     /* Ctrl-d */
	options->c_cc[VTIME]    = 0;     /* inter-character timer unused */
	                               /* 不使用字符间的计时器 */
	options->c_cc[VSWTC]    = 0;     /* '\0' */
	options->c_cc[VSTART]   = 0;     /* Ctrl-q */ 
	options->c_cc[VSTOP]    = 0;     /* Ctrl-s */
	options->c_cc[VSUSP]    = 0;     /* Ctrl-z */
	options->c_cc[VEOL]     = 0;     /* '\0' */
	options->c_cc[VREPRINT] = 0;     /* Ctrl-r */
	options->c_cc[VDISCARD] = 0;     /* Ctrl-u */
	options->c_cc[VWERASE]  = 0;     /* Ctrl-w */
	options->c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	options->c_cc[VEOL2]    = 0;     /* '\0' */
	memset(&options->c_cc,0,sizeof(options->c_cc));
	return 0;
}
//设置串口
int set_com_mode(int fd,int databits,int stopbits,int parity)
{ 
	struct termios options; 
	int ret;
#ifdef SHOW_WORK_INFO
	printf("设置串口，文件描述符%d,设置参数%d,%d,%c\n",fd,databits,stopbits,parity);
#endif
	gtloginfo("设置串口，文件描述符%d,设置参数%d,%d,%c\n",fd,databits,stopbits,parity);
	ret=tcgetattr( fd,&options);
	if  ( ret  !=  0) { 
		printf("SetupSerial 1 error!!\n"); 
		gtlogerr("启动串口连接错误，文件描述符%d,返回值%d\n",fd,ret);
		return(-1);  
	}
	//memset((void*)&options,0,sizeof(options));//added by shixin
	options.c_lflag=0;	//added by shixin
	options.c_iflag=0;	//added by shixin 
	options.c_cflag &= ~CSIZE; 
	switch (databits) /*......*/
	{   
	case 7:		
		options.c_cflag |= CS7; 
		break;
	case 8:     
		options.c_cflag |= CS8;
		break;   
	default:    
		fprintf(stderr,"Unsupported data size:%d set to default 8\n",databits); 
		gtlogwarn("设置串口中不支持的数据大小%d，设为缺省值8\n",databits);
		options.c_cflag |= CS8;
	break;
	}
    switch (parity) 
    {   
	case 'n':
	case 'N':    
		options.c_cflag &= ~PARENB;   /* Clear parity enable */
		options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
		parity='n';
		break;  
	case 'o':   
	case 'O':     
		options.c_cflag |= (PARODD | PARENB); /* ......*/  
		options.c_iflag |= INPCK;             /* Disnable parity checking */ 
		break;  
	case 'e':  
	case 'E':   
		options.c_cflag |= PARENB;     /* Enable parity */    
		options.c_cflag &= ~PARODD;   /* ......*/     
		options.c_iflag |= INPCK;       /* Disnable parity checking */
		break;
	case 'S': 
	case 's':  /*as no parity*/   
	    	options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
	break;  
	default:   
		fprintf(stderr,"Unsupported parity set to default\n");  
		gtlogwarn("设置串口中不支持的校验位%d，设为缺省值'N'\n",parity);
		options.c_cflag &= ~PARENB;   /* Clear parity enable */
		options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
	break;
    }  
/* .....*/  
    switch (stopbits)
    {   
	case 1:    
		options.c_cflag &= ~CSTOPB;  
		break;  
	case 2:    
		options.c_cflag |= CSTOPB;  
	   break;
	default:    
		 fprintf(stderr,"Unsupported stop bits set to default 1\n"); 
		 gtlogwarn("设置串口中不支持的停止位%d，设为缺省值1\n",stopbits);
		 options.c_cflag &= ~CSTOPB;  
	break; 
    } 
/* Set input parity option */ 
    if (parity != 'n')   
	options.c_iflag |= INPCK; 

	
    tcflush(fd,TCIFLUSH);

    clr_option_ctrl_char(&options);
    options.c_cc[VTIME] = 150; /* ....15 seconds*/   
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
	
    options.c_lflag  &= ~(ICANON|ECHO |ECHO| ISIG);  /*Input*///Enot term
    options.c_oflag  &= ~OPOST;   /*Output*/




    if (tcsetattr(fd,TCSANOW,&options) != 0)   
    { 
	perror("SetupSerial port");
	gtlogerr("设置串口时tcsetattr失败,文件描述符%d\n",fd);
	return (-1);  
    } 
    return (0);  
}

static int speed_arr[] = { B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300};
static int name_arr[] =  { 115200,  57600,  38400,  19200,  9600,  4800,  2400,  1200,  300 };
//设置串口的波特率
int set_com_speed(int fd, int speed)
{
	int   i; 
	int   status; 
	struct termios   Opt;
#ifdef SHOW_WORK_INFO
	printf("设置串口(fd=%d)波特率%d\n",fd,speed);
#endif
	gtloginfo("设置串口(fd=%d)波特率%d\n",fd,speed);
	tcgetattr(fd, &Opt); 
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) 
	{ 
		if  (speed == name_arr[i]) 
		{     
			tcflush(fd, TCIOFLUSH);     
			cfsetispeed(&Opt, speed_arr[i]);  
			cfsetospeed(&Opt, speed_arr[i]);   
			status = tcsetattr(fd, TCSANOW, &Opt);  
			if  (status != 0) 
			{        
				printf("tcsetattr fd1");  
				return -errno;     
			}    
			tcflush(fd,TCIOFLUSH);   
			return 0;
		}  
	}
	return -EINVAL;
	
}

////读取进程的版本号

int get_prog_version(char *buf,char *lockfile)
{
	char buffer[200];
	int i;
	FILE *fp;
	char *pt;
	char *ps;
	int rc;
	if(buf==NULL)
		return -1;
	if(lockfile==NULL)
		return -1;
	memcpy(buf,"0",2);
	fp=fopen(lockfile,"r");
	if(fp==NULL)
		return -1;
	rc=-1;
	for(i=0;i<10;i++)
	{
		pt=fgets(buffer,sizeof(buffer),fp);
		if(pt==NULL)
		{
			rc=-1;
			break;
		}
		ps=strstr(pt,(char*)"version:");
		if(ps==NULL)
			continue;
		pt=strchr(ps,(int)':');
		if(pt==NULL)
		{
			rc=-1;
			break;
		}		pt++;
		if(strlen(pt)>20)
		{
			rc=-1;
			break;
		}		
		memcpy(buf,pt,strlen(pt)+1);
		if((buf[strlen(pt)-1]=='\r')||(buf[strlen(pt)-1]=='\n'))
			buf[strlen(pt)-1]='\0';

//		fclose(fp);    //add by wsy
		rc=0;
		break;
	}
NOTFOUND:
	fclose(fp);
	return rc;
	
}

#define KVERSION_FILE	"/proc/version"

//获取内核版本号字符串
char *get_kernel_version(void)
{
	static char kverbuf[20];
	static char *kver=NULL;
	static char tempbuf[256];
	FILE *kf=NULL;

	if(kver==NULL)
	{
		sprintf(kverbuf,"2.4");
		kf=fopen(KVERSION_FILE,"r");
		if(kf!=NULL)
		{
			//tbuf[0]='\0';
			fgets(tempbuf,sizeof(tempbuf),kf);
			if(strstr(tempbuf,"2.6.")!=NULL)
				sprintf(kverbuf,"2.6");
			fclose(kf);
			kver=kverbuf;
		}
	}
	return kver;
}

int lock_file(int fd,int wait)
{//锁定指定描述符的文件,wait=0表示如果无法锁定则返回错误
  //wait=1表示如果无法锁定则等待
	int ret;
  	if(fd<0)
		return -EINVAL;
	if(wait)
		ret=flock(fd,LOCK_EX);
	else
		ret=flock(fd,LOCK_EX|LOCK_NB);
	if(ret<0)
		return -errno;
	else
	{
		fsync(fd);
		return ret;
	}
}
int unlock_file(int fd)
{//解除文件的锁定

	if(fd<0)
		return -EINVAL;
	flock(fd,LOCK_UN);
	fsync(fd);
	return 0;
}





//将字符串转换为16进制数字
DWORD atohex(char *buffer)
{
	int i,len;
	DWORD hex=0;
	char *p;
	DWORD ret;
	char ch;
	char buf[12];
	if(buffer==NULL)
		return 0;
	memcpy(buf,buffer,sizeof(buf));
	p=strrchr(buf,'x');//找到最后一个x的位置
	if(p==NULL)
	{
		p=strrchr(buf,'X');
	}	
	if(p==NULL)
		p=buf;
	else
		p++;
	len=strlen(p);

	if(len>8)
	{
		i=len-8;
		p+=i;
		len=strlen(p);	
	}
	p+=len;
	p--;
	for(i=0;i<len;i++)
	{
		ch=(char)toupper((int)*p);
		*p=ch;
		if(isdigit(*p))
		{
			ret=*p-'0';
		}
		else //字母
		{
			//if(!isupper(*p))
			*p=(char)toupper((int)*p);
			ret=*p-'A'+10;
		}
		hex|=ret<<(i*4);
		p--;
	}
	return hex;	
	
}


//add by wsy@June 2006
/**********************************************************************************************
 * 函数名	:gttime2tm()
 * 功能	:将gt_time_struct转为tm结构的函数
 * 参数	:gttime:指向gt_time_struct结构的指针
 *			:time:要输出的指向tm结构的指针
 * 返回值	:0表示成功，负值表示出错
 **********************************************************************************************/
int gttime2tm (struct gt_time_struct *gttime, struct tm *time)
{	
	time->tm_year =	gttime->year-1900;
	time->tm_mon  =	gttime->month-1;
	time->tm_mday =	gttime->day;
	time->tm_hour =	gttime->hour;
	time->tm_min  =	gttime->minute;
	time->tm_sec  =	gttime->second;
	return 0;
}

/**********************************************************************************************
 * 函数名	:get_dev_time()
 * 功能	:获取时钟芯片的时间
 * 参数	:
 * 返回值	:0表示成功，负值表示出错
 **********************************************************************************************/

int get_dev_time(struct tm *ntime) 
{
	int fd = open("/dev/gpioi2c", 0);
    if(fd<0)
    {
    	printf("Open gpioi2c error!\n");
    	return -1;
    }

	int value=0;
	int ret;
	struct tm nowtime;
	time_t sysTime;
	
	value = 0;
	value |=0xd0<<24;
	value |=0x6<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//年
	if(ret<0)
		printf("ioctl read year err\n");
	value &=0xff;
	printf("year:%#X \n",value);
	nowtime.tm_year=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x5<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//月
	if(ret<0)
		printf("ioctl read month err\n");
	value &=0x1f;//月只是低5位有效
	printf("month:%#X \n",value);
	nowtime.tm_mon=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x4<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//日
	if(ret<0)
		printf("ioctl read day err\n");
	value &=0x3f;//日只是低6位有效
	printf("day:%#X	\n",value);
	nowtime.tm_mday=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x2<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//时
	if(ret<0)
			printf("ioctl read hour err\n");
	value &=0x3f;//时只是低7位有效
	printf("hour:%#X \n",value);
	nowtime.tm_hour=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x1<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//分
	if(ret<0)
		printf("ioctl read minute err\n");
	value &=0x7f;//分只是低7位有效
	printf("min:%#X \n",value);
	nowtime.tm_min=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x0<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//秒
	if(ret<0)
		printf("ioctl read second err\n");
	value &=0x7f;//秒只是低7位有效
	printf("sec:%#X \n",value);
	nowtime.tm_sec=bcdtobin((unsigned char)value);

	//释放资源
	close(fd);

	printf("get_dev_time %d-%d-%d-%d-%d-%d!\n",nowtime.tm_year,nowtime.tm_mon,nowtime.tm_mday,nowtime.tm_hour,nowtime.tm_min,nowtime.tm_sec);
	ntime->tm_year = nowtime.tm_year + 100;
	ntime->tm_mon  = nowtime.tm_mon - 1;
	ntime->tm_mday = nowtime.tm_mday;
	ntime->tm_hour = nowtime.tm_hour;
	ntime->tm_min  = nowtime.tm_min;
	ntime->tm_sec  = nowtime.tm_sec;

	sysTime = mktime(ntime);
	stime(&sysTime);
	
	return 0;
	
}


/**********************************************************************************************
 * 函数名	:set_dev_time()
 * 功能	:设置设备的时间
 * 参数	:ntime:指向要设置的时间结构
 * 返回值	:0表示成功，负值表示出错
 **********************************************************************************************/
int set_dev_time(struct tm *ntime)
{
#if EMBEDED
	if(RTC_INIT == 0)
	{
		init();
		RTC_INIT = 1;
	}

	int fd = open("/dev/gpioi2c", 0);
    if(fd<0)
    {
    	printf("Open gpioi2c error!\n");
    	return -1;
    }

	unsigned int value=0;
	int ret;
	unsigned char tm;
	struct tm nowtime;
	
	tm=bintobcd(ntime->tm_year-100);
	value = tm;
	value |=0xd0<<24;
	value |=0x6<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//年
	if(ret<0)
		printf("ioctl write year err\n");
	value &=0xff;
	printf("year:%#X \n",value);
	ntime->tm_year=bcdtobin((unsigned char)value);

	tm=bintobcd(ntime->tm_mon+1);
	value = tm;
	value &=0x1f;//月只是低5位有效
	value |=0xd0<<24;
	value |=0x5<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//月
	if(ret<0)
		printf("ioctl write month err\n");
	printf("month:%#X \n",value);

	tm=bintobcd(ntime->tm_mday);
	value = tm;
	value &=0x3f;//日只是低6位有效
	value |=0xd0<<24;
	value |=0x4<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//日
	if(ret<0)
		printf("ioctl write day err\n");
	printf("day:%#X	\n",value);

	tm=bintobcd(ntime->tm_hour);
	value = tm;
	value &=0x3f;//时只是低7位有效
	value &= ~(1L<<6);//第6位置0是24小时制
	value |=0xd0<<24;
	value |=0x2<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//时
	if(ret<0)
		printf("ioctl write hour err\n");
	printf("hour:%#X \n",value);

	tm=bintobcd(ntime->tm_min);
	value = tm;
	value &=0x7f;//分只是低7位有效
	value |=0xd0<<24;
	value |=0x1<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//分
	if(ret<0)
		printf("ioctl write minute err\n");
	printf("min:%#X \n",value);

	tm=bintobcd(ntime->tm_sec);
	value = tm;
	value &=0x7f;//秒只是低7位有效
	value |=0xd0<<24;
	value |=0x0<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//秒
	if(ret<0)
		printf("ioctl write second err\n");
	printf("sec:%#X \n",value);

	//释放资源
	close(fd);
	
	/*update system time*/
	if (get_dev_time(&nowtime) != 0)
	{
		printf("%s %d %s\n", __FILE__, __LINE__, strerror(errno));
		gtlogerr("update system time failed!\n");
		return (-1);
	}
	
	return ret;
#endif
	
}

int g_rtc_fd = 0;

int open_rtc_dev()
{
	g_rtc_fd = open("/dev/hi_rtc", O_RDWR);
    if(g_rtc_fd<0)
    {
    	printf("Open hi_rtc error!\n");
    	return -1;
    }

	return 0;
}

int close_rtc_dev()
{
	if(g_rtc_fd)
		close(g_rtc_fd);

	return 0;
}

int set_dev_time_d(struct tm *ntime)
{
	int ret;
	rtc_time_t tm;
	struct tm nowtime;
	time_t sysTime;

	tm.year = ntime->tm_year+1900;
	tm.month = ntime->tm_mon+1;
	tm.date = ntime->tm_mday;
	tm.hour = ntime->tm_hour;
	tm.minute = ntime->tm_min;
	tm.second = ntime->tm_sec;
	tm.weekday = 0;

	gtloginfo("set_dev_time_d %d-%d-%d-%d-%d-%d!\n",tm.year,tm.month,tm.date,tm.hour,tm.minute,tm.second);

	ret = ioctl(g_rtc_fd, HI_RTC_SET_TIME, &tm);
    if (ret < 0) {
        printf("ioctl: HI_RTC_SET_TIME failed\n");
        close(g_rtc_fd);
		return ret;
    }

	sysTime = mktime(ntime);
	stime(&sysTime);
#if 0
	if (get_dev_time_d(&nowtime) != 0)
	{
		printf("%s %d %s\n", __FILE__, __LINE__, strerror(errno));
		gtlogerr("update system time failed!\n");
		return (-1);
	}
#endif	
	return 0;
	
}

int get_dev_time_d(struct tm *ntime)
{
	int ret;
	rtc_time_t tm;
	time_t sysTime;
	
	ret = ioctl(g_rtc_fd, HI_RTC_RD_TIME, &tm);
	if (ret < 0)
	{
        printf("ioctl: HI_RTC_RD_TIME failed\n");
        close(g_rtc_fd);
		return ret;
    }

	gtloginfo("get_dev_time_d %d-%d-%d-%d-%d-%d!\n",tm.year,tm.month,tm.date,tm.hour,tm.minute,tm.second);

	ntime->tm_year = tm.year-1900;
    ntime->tm_mon  = tm.month-1;
    ntime->tm_mday = tm.date;
	ntime->tm_hour = tm.hour;
	ntime->tm_min  = tm.minute;
	ntime->tm_sec  = tm.second;

	sysTime = mktime(ntime);
	stime(&sysTime);
	
	return 0;	
}


/**********************************************************************************************
 * 函数名	:get_percent()
 * 功能	:根据变量的值和范围计算出变量对应的百分比值
 * 参数	:min:变量允许的最小值
 *			 max:变量允许的最大值
 *			 val:变量值
 * 返回值	:该变量对应的百分比值
 **********************************************************************************************/
int get_percent(int min,int max,int val)
{
	int total;
	int cur;
	int percent;
	total=max-min+1;
	if(total<=0)
		return 50;
	cur=val-min;
	percent=(cur*100)/total;
	return percent;
}

/**********************************************************************************************
 * 函数名	:get_percent()
 * 功能	:根据变量的范围和百分比计算出变量的值
 * 参数	:min:变量允许的最小值
 *			 max:变量允许的最大值
 *			 percent:变量的百分比值
 * 返回值	:该变量的实际值
 **********************************************************************************************/
int get_value(int min,int max,int percent)
{
	int total;
	int val;
	//total=max-min+1;
	total = max-min; //wsyfixed
	if(total<=0)
		return 50;//FIXME 要是范围不对则返回一个默认值
	if(percent<0)
		percent=0;
	if(percent>100)
		percent=50;//如果超过100% 则设置成50
	val=(percent*total)/100;
	val+=min;
	return val;
}
#if 0
////added by lsk 2006 -11-7
/**********************************************************************************************
 * 函数名	:force_lockfile()
 * 功能	:强制加锁文件
 * 参数 ：fd 文件控制字 
 *	  cmd 命令
 *	  wait 等待标志
 *	 cmd = F_RDLCK 读禁止 ； F_WRLCK 写禁止; F_UNLCK 解除锁定
 *	 wait = 0 无法锁定则立即返回， =1 等待锁定
 * 返回值	:0 成功 -1 失败
 **********************************************************************************************/
//强制加锁文件  
//输入 fd 文件控制字 ,cmd 命令, wait 等待标志
//返回0 成功 -1 失败
// cmd = F_RDLCK 读禁止 ； F_WRLCK 写禁止; F_UNLCK 解除锁定
// wait = 0 无法锁定则立即返回， =1 等待锁定
int force_lockfile(int fd, short int cmd, int wait)
{	
	struct flock tp;
	tp.l_len = 0;
	tp.l_type = cmd;
	tp.l_whence = SEEK_SET; 
	tp.l_start = 0;
	tp.l_pid = getpid();
	if(wait == 0)
	return fcntl(fd, F_SETLK,&tp);

	if(wait ==1)
	return fcntl(fd, F_SETLKW,&tp);

	return fcntl(fd, F_SETLKW,&tp);
}
#endif

/**********************************************************************************************
 * 函数名	:delay_on_vstate() //wsy
 * 说明 :主要供视频状态使用
 * 功能	:对输入的原始状态，根据之前的值，相应的保持参数等进行处理
 *		 达到的效果是:
 *			当某位状态从0变成1时，维持valid_delay秒数才让输出也变成1
 *		 	当某位状态从1变成0时，维持invalid_delay秒数才让输出也变成0
 * 参数	:	newstate:最新的状态,按位排列
 *			 valid_delay:见以上说明
 *			 invalid_delay: 见上
 *			 stateparam: 用于保存之前各种状态和计数器的结构
 * 返回值	:处理之后输出的状态，也按位排列
 **********************************************************************************************/


unsigned long delay_on_vstate(unsigned long newstate, int valid_delay, int invalid_delay, vstate_t *stateparam)
{
	int i;
	int newbit,oldbit;
	unsigned long result=0;

	result=stateparam->old_result;
	for(i=0;i<get_video_num();i++)
	{
		newbit=(newstate>>i)&1;
		oldbit=((stateparam->old_value)>>i)&1;
		if(newbit==oldbit)
		{
			if(newbit==1)//1->1
			{
				if(stateparam->valid_stable_time[i] < valid_delay)
				{
					if(++stateparam->valid_stable_time[i] == valid_delay) //到了跳转时间
						result |= 1<<i;

				}	
			}	
			else//0->0
			{
				if(stateparam->invalid_stable_time[i] < invalid_delay)
				{
					if(++stateparam->invalid_stable_time[i] == invalid_delay) //到了跳转时间
						result &= ~(1<<i);	
				}
			}
		}
		else
		{
			stateparam->valid_stable_time[i]=0;
			stateparam->invalid_stable_time[i]=0;
			if(newbit==1)//0->1
			{
				
				if(valid_delay==0)
					result |=  1<<i;
				
			}
			else//1->0
			{	
				if(invalid_delay==0)
					result &= ~(1<<i);	
				
			}

		}
	}
	stateparam->old_value=newstate;
	stateparam->old_result=result;
	return result;

}

/**********************************************************************************************
 * 函数名	:show_time
 * 功能	:	打印当前时间
 * 参数	:	infostr:要显示在时间之后的调试信息字符串,可以为空
 * 返回值	:0表示成功,负值表示错误
 **********************************************************************************************/

int show_time(char *infostr)
{
	struct timeval tv;
	struct tm *ptime;
	char pbuf[60];
	time_t ctime;
	if(gettimeofday(&tv,NULL)<0)
	{
		return -1;
	}
	ctime=tv.tv_sec;
	ptime=localtime(&ctime);
	if(ptime!=NULL)
	{
		if(infostr == NULL)
			sprintf(pbuf,"%d-%d-%d %d:%d:%d.%03d\n",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min,ptime->tm_sec,(int)tv.tv_usec/1000);	
		else
			sprintf(pbuf,"%d-%d-%d %d:%d:%d.%03d --%s\n",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min,ptime->tm_sec,(int)tv.tv_usec/1000,infostr);	
		printf("%s",pbuf);
	}
	return 0;
	
}

