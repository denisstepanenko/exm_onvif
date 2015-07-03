//透明串口传送模块
#include "ipmain.h"
#include "trans_com.h"
#include <commonlib.h>
#include "ipmain_para.h"
#include <termios.h> 
//初始化透明串口相关

static struct trans_com_struct trans_com[10];
static void *trans_com_thread(void *transcom);



/**********************************************************************************************
 * 函数名	:init_trans_com_var()
 * 功能	:初始化透明串口服务用到的变量
 * 输入	:无
 * 返回值	:无
 **********************************************************************************************/
void init_trans_com_var(void)
{
	int i;
	struct trans_com_struct *com;
	struct ipmain_para_struct *ipmain_para;
	
	for(i=0;i<get_total_com()+1;i++)
	{
		com=&trans_com[i];
		pthread_mutex_init(&com->mutex, NULL);//使用缺省设置
		pthread_mutex_lock(&com->mutex);
		com->thread_id=-1;
		com->listen_fd=-1;
		com->local_fd=-1;
		com->net_fd=-1;
		//com->tcp_port=-1;
		com->baud=4800;
		com->databit=8;
		com->parity='N';
		com->stopbit=1;
		com->flow=0;
		memset((char*)com->net_rec_buf,0,sizeof(com->net_rec_buf));
		memset((char*)com->com_rec_buf,0,sizeof(com->com_rec_buf));
		ipmain_para=get_mainpara();
		if(i==0)
		{
			memcpy(com->path,GTIP1004_COM1,sizeof(com->path));
			com->tcp_port=ipmain_para->com0_port;//COM1_TCP_PORT;
			com->ch=0;
		}
		else if(i==1)
		{
			memcpy(com->path,GTIP1004_COM2,sizeof(com->path));
			com->tcp_port=ipmain_para->com1_port;//COM2_TCP_PORT;
			com->ch=1;
		}
		else if(i==2)
		{
		#ifdef ARCH_3520A
			memcpy(com->path,GTIP1004_COM_INTERNAL,sizeof(com->path));
		#endif
		#ifdef ARCH_3520D
			memcpy(com->path,GTIP1004_COM0,sizeof(com->path));
		#endif
			com->tcp_port=0;//COM_INTERNAL
			com->ch=2;
		}
		else
		{
			printf("ip1004 only have %d serial ports!\n",get_total_com());			
		}
		pthread_mutex_unlock(&com->mutex);
	}	
}

/**********************************************************************************************
 * 函数名	:get_trans_com_info()
 * 功能	:获取指定通道的透明串口的描述结构指针
 * 输入	:ch:串口通道号
 * 返回值	:描述指定通道号的透明串口的结构指针
 *			 NULL表示出错
 **********************************************************************************************/
struct trans_com_struct *get_trans_com_info(int ch)
{
	if(ch<get_total_com())
		return &trans_com[ch];
	else
		return NULL;
}

int init_trans_com_thread(void)
{
	return 0;
}

static int debug_com_open = 0;

//打开透明串口，绑定到指定网络描述符
static int  trans_com_open(struct trans_com_struct *com,int net_fd)
{
	int ret;
	if(com==NULL)
		return -1;
	if(net_fd<0)
		return -1;

	debug_com_open = 0;
	pthread_mutex_lock(&com->mutex);
	if(com->flag!=0)
	{
		//说明已经有一个连接在访问了
	 #ifdef SHOW_WORK_INFO		
		//printf("there have already a connect on com netport is %d \n",net_fd);
	 #endif
	 #ifdef ARCH_3520A
		if(com->ch == 2)
		{
			//lc do 检查其波特率，如果不为9600，意味被占掉，重新关闭打开!
		  struct termios   Opt;
		  int speed;
		  tcgetattr(com->local_fd, &Opt);
          speed = cfgetospeed(&Opt); 
		  #ifdef SHOW_WORK_INFO
			//printf("获取串口(fd=%d)波特率为%d\n",com->local_fd,speed);
		  #endif
			//gtloginfo("获取串口(fd=%d)波特率%d\n",com->local_fd,speed);
			if(speed != B9600)
			{
              close(com->local_fd);
			  com->local_fd=-1;
			  gtloginfo("重新打开串口(ch=%d)\n",com->ch);
			  com->local_fd=open(com->path,O_RDWR|O_NOCTTY);
			  //lc do 2013-9-13 加锁，防止其他进程占用
			  struct flock f1;
			  f1.l_type=F_WRLCK;
			  f1.l_start=0;
			  f1.l_whence=SEEK_SET;
			  f1.l_len=0;
			  if(fcntl(com->local_fd,F_SETLK,&f1)<0)
				gtlogerr("com %d set F_WRLCK err!\n",com->ch);
			  else
				printf("com %d set F_WRLCK suceed!\n",com->ch);
				if(com->local_fd < 0)
				{
				#ifdef SHOW_WORK_INFO	
					printf("trans_com_thread can't open COM!!!\n");
				#endif
			       ret= -2;
					goto com_open_exit;
				}    
				#ifdef SHOW_WORK_INFO
					printf("open com with baud=%d databit=%d stopbit=%d parity=%c\n",(int)com->baud, (int)com->databit,(int)com->stopbit,(int)com->parity);
				#endif
			
				set_com_speed(com->local_fd,com->baud);
				//if (set_com_mode(com->local_fd,8,1,'N') <0)  		
		    	if (set_com_mode(com->local_fd,com->databit,com->stopbit,com->parity) <0)  
				{
				#ifdef SHOW_WORK_INFO	
					printf("trans_com_thread set COM error!\n");
				#endif
					com->local_fd=-1;
					ret= -2;
					goto com_open_exit;
				}		
				debug_com_open = 1;
				com->flag=1;
				com->net_fd=net_fd;//
				ret=0;

				pthread_mutex_unlock(&com->mutex);
	
				return ret;
			}
			else
			{
				pthread_mutex_unlock(&com->mutex);
				return 0;
			}
		}
		#endif
		ret= -2;
		goto com_open_exit;
	}
	else
	{
		//之前没有其他连接			
		printf("enable a connect %d\n",net_fd);					
		if(com->local_fd>0)
		{
			//已经被打开过???
			printf("trans_com_thread COM local_fd > 0,already opened!\n");
		}	
    	else
    	{
		  com->local_fd=open(com->path,O_RDWR|O_NOCTTY);
		  //lc do 2013-9-13 加锁，防止其他进程占用
		  struct flock f1;
		  f1.l_type=F_WRLCK;
		  f1.l_start=0;
		  f1.l_whence=SEEK_SET;
		  f1.l_len=0;
		  if(fcntl(com->local_fd,F_SETLK,&f1)<0)
			gtlogerr("com %d set F_WRLCK err!\n",com->ch);
		  else
			printf("com %d set F_WRLCK suceed!\n",com->ch);
   		}
		
		if(com->local_fd < 0)
		{
		#ifdef SHOW_WORK_INFO	
			printf("trans_com_thread can't open COM!!!\n");
		#endif
		       ret= -2;
			goto com_open_exit;
		}    
#ifdef SHOW_WORK_INFO
		printf("open com with baud=%d databit=%d stopbit=%d parity=%c\n",(int)com->baud, (int)com->databit,(int)com->stopbit,(int)com->parity);
#endif
		
		set_com_speed(com->local_fd,com->baud);
		//if (set_com_mode(com->local_fd,8,1,'N') <0)  		
	    if (set_com_mode(com->local_fd,com->databit,com->stopbit,com->parity) <0)  
		{
		#ifdef SHOW_WORK_INFO	
			printf("trans_com_thread set COM error!\n");
		#endif
			com->local_fd=-1;
			ret= -2;
			goto com_open_exit;
		}
		debug_com_open = 1;
		com->flag=1;
		com->net_fd=net_fd;//
		ret=0;
	}	
com_open_exit:
	pthread_mutex_unlock(&com->mutex);
	
	return ret;
}
//关闭透明串口
static int  trans_com_close(struct trans_com_struct *com,fd_set *readfds,fd_set *writefds)
{
	//FILE *comstream=NULL;
	if(com==NULL)
		return 0;
	if(!com->flag)
		return 0;
	pthread_mutex_lock(&com->mutex);
	if(com->local_fd>0)
	{
		if(readfds!=NULL)
			FD_CLR(com->local_fd,readfds);
		if(writefds != NULL)
			FD_CLR(com->local_fd,writefds);

		#if 0
		comstream=fdopen(com->local_fd,"rw");
		comstream=NULL;//added by test FIXME
		if(comstream!=NULL)
		{
			fflush(comstream);
		}
		else
		#endif
		usleep(500000);//
		
		close(com->local_fd);
		com->local_fd=-1;
	}
	if(com->net_fd>0)
	{
		if(readfds!=NULL)
			FD_CLR(com->net_fd,readfds);
		if(writefds != NULL)
			FD_CLR(com->net_fd,writefds);

		close(com->net_fd);
		com->net_fd=-1;
	}	
	com->flag=0;
	memset(&com->allow_addr,0,sizeof(com->allow_addr));
	pthread_mutex_unlock(&com->mutex);
#ifdef SHOW_WORK_INFO
	printf("透明串口%d成功关闭\n",com->ch);
#endif
	gtloginfo("透明串口%d成功关闭\n",com->ch);
	return 0;
}
//设置select使用的fdsets
static int trans_com_setfds(struct trans_com_struct *com,fd_set *readfds,fd_set *writefds,int *max_fd)
{
	if(com==NULL)
		return 0;
	
	if(writefds!=NULL)
	{
		if(com->flag)
		{
			if(com->net_fd>0)
				FD_SET(com->net_fd,writefds);
			if(com->local_fd>0)
				FD_SET(com->local_fd,writefds);			
		}
	}
	if(readfds!=NULL)
	{
		FD_ZERO(readfds);
		if(com->listen_fd>0)
		{
			FD_SET(com->listen_fd,readfds);
			if(*max_fd<com->listen_fd)
				*max_fd=com->listen_fd;
		}
		if(com->flag)
		{
			if(com->net_fd>0)
			{
				FD_SET(com->net_fd,readfds);
				if(*max_fd<com->net_fd)
					*max_fd=com->net_fd;
			}
			if(com->local_fd>0)
			{
				FD_SET(com->local_fd,readfds);	
				if(*max_fd<com->local_fd)
					*max_fd=com->local_fd;	
			}
		}
	}

	return 0;
}
//从网络读取数据发送到串口
static int read_net2com(struct trans_com_struct *com,BYTE *tempbuf,int bufsize)
{
	int ret;
	if((com->local_fd<0)||(com->net_fd<0)||(tempbuf==NULL))
		return -1;
	ret=read(com->net_fd,tempbuf,bufsize);
	if(ret>0)
	{
		#ifdef SHOW_WORK_INFO
			printf("recv net data to transcom :");
			print_buffer(tempbuf,ret);
			printf("\n");
		#endif	
		write(com->local_fd,tempbuf,ret);
		
			
	}
	else
		return -2;//可能网络连接断开
	return 0;
}
//从串口读取数据发送到网络
static int read_com2net(struct trans_com_struct *com,BYTE *tempbuf,int bufsize)
{
	int ret;
	if((com->local_fd<0)||(com->net_fd<0)||(tempbuf==NULL))
		return -1;
	ret=read(com->local_fd,tempbuf,bufsize);
	if(ret>0)
	{
		#ifdef SHOW_WORK_INFO
			printf("recv transcom data to net :");
			print_buffer(tempbuf,ret);
			printf("\n");
		#endif	
		if(write(com->net_fd,tempbuf,ret)<=0)
			return -2;//可能网络连接已经断开
	}
	else
		return -2;//可能串口异常
	return 0;
}

/**********************************************************************************************
 * 函数名	:creat_trans_com_thread()
 * 功能	:创建透明串口服务线程
 * 输入	:attr:线程属性
 *			 arg:要创建的透明串口服务的描述结构指针
 * 返回值	:0表示成功负值表示出错
 **********************************************************************************************/
int creat_trans_com_thread(pthread_attr_t *attr,void *arg)
{
	struct trans_com_struct *com;
	if(arg==NULL)
		return -1;
	com=(struct trans_com_struct *)arg;
	return pthread_create(&com->thread_id,attr, trans_com_thread, arg);//创建与51模块通讯的线程
}


static void *trans_com_thread(void *transcom)
{

	int accept_fd=-1,max_fd=-1;//com1_fd=-1; 
 	struct sockaddr_in  server_addr;//,newguest; 
	struct trans_com_struct *com;
	BYTE  *net_rec_buf;
	BYTE  *com_rec_buf;
	unsigned short port;
 	int addrlen,ret;
	fd_set readfds;
	char	tmpbuf[256];
	//BYTE tempbuf[6]={0x02,0x01,0x0e,0xff,0xff,0x0f}; //wsy add,速度最快的命令
	struct sockaddr_in guest_addr;
//	int i;

	if(transcom==NULL)
		return NULL;
	else
		com=(struct trans_com_struct *)transcom;

#ifdef DISPLAY_THREAD_INFO
	printf("trans_com_thread thread %d running...\n",com->ch);
#endif
	gtloginfo("start trans_com_thread %d...\n",com->ch);
	
	init_trans_com_thread();
	net_rec_buf=(BYTE*)com->net_rec_buf;
	com_rec_buf=(BYTE*)com->com_rec_buf;
	
 	if((com->listen_fd=socket(AF_INET,SOCK_STREAM,0))<0) 
  	{ 
        printf("ipmain create trans_com_thread Socket Error:%s\n\a",strerror(errno)); 
		gtlogfault("error:%s,不能为透明串口%d连接创建socket ,程序退出!\n",strerror(errno),com->ch);
		exit(1); 
  	}   
	bzero(& server_addr, sizeof(struct sockaddr_in));
 	server_addr.sin_family=AF_INET; 
	port=(WORD)com->tcp_port;
 	server_addr.sin_port=htons(port); 
 	server_addr.sin_addr.s_addr=htonl(INADDR_ANY); 
 	addrlen=1; 
 /* 如果服务器终止后,服务器可以第二次快速启动而不用等待一段时间  */ 
 	setsockopt(com->listen_fd,SOL_SOCKET,SO_REUSEADDR,&addrlen,sizeof(int));
 	if(bind(com->listen_fd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0) 
  	{ 
	    printf("trans_com_thread ipmain Bind Error:%s\n\a",strerror(errno)); 
		gtlogfault("error:%s,不能为透明串口%d连接绑定socket ,程序退出!\n",strerror(errno),com->ch);
	    exit(1); 
  	} 
  	listen(com->listen_fd,1);
	max_fd=com->listen_fd;//最大文件描述符号
	FD_ZERO(&readfds);
  	while(1) 		
  	{
		trans_com_setfds(com,&readfds,NULL,&max_fd);
  		select(max_fd+1,&readfds,NULL,NULL,NULL);
		if(FD_ISSET(com->listen_fd,&readfds))
		{
			
			addrlen=sizeof(guest_addr);
	   		accept_fd=accept(com->listen_fd,(void*)&guest_addr,&addrlen); 
	   		if((accept_fd<0)&&(errno==EINTR)) 
	          		continue; 
	   		else if(accept_fd<0) 
	    		{ 
	        		printf("ipmain trans_com_thread Accept Error:%s\n\a",strerror(errno)); 
	        		continue; 
	    		} 
		#ifdef SHOW_WORK_INFO	
			printf("ipmain trans_com_thread come a new guest:%s \n",inet_ntoa(guest_addr.sin_addr));
			printf("the trans com allow addr is:%s \n",inet_ntoa(com->allow_addr.sin_addr));
		#endif
			//检查ip地址是否合法，不合法则断开连接(这部分代码还没有)
		sprintf(tmpbuf,"%s",inet_ntoa(guest_addr.sin_addr));
		
		#ifndef TRANS_COM_NOT_CHECK_REMOTEIP
			if(memcmp(&com->allow_addr.sin_addr,&guest_addr.sin_addr,4)!=0)
			{
				#ifdef SHOW_WORK_INFO	
					printf("非法地址:%s (allow:%s)尝试连接透明串口%d,关闭连接 !\n",tmpbuf,inet_ntoa(com->allow_addr.sin_addr),com->ch);
				#endif
				gtloginfo("非法地址:%s (allow:%s)尝试连接透明串口%d,关闭连接 !\n",tmpbuf,inet_ntoa(com->allow_addr.sin_addr),com->ch);
				close(accept_fd);
				continue;
			}
		#else
			if(memcmp(&com->allow_addr.sin_addr,&guest_addr.sin_addr,4)!=0)
			{
				#ifdef SHOW_WORK_INFO	
				printf("测试状态(警告):非法地址:%s (allow:%s)尝试连接透明串口端口!\n",tmpbuf,inet_ntoa(com->allow_addr.sin_addr));
				#endif
				gtloginfo("测试状态(警告):非法地址:%s (allow:%s)尝试连接透明串口%d端口!\n",tmpbuf,inet_ntoa(com->allow_addr.sin_addr),com->ch);

			}
		#endif
			if(com->flag!=0)
			{
				//说明已经有一个连接在访问了
				printf("目标com 口已经存在一个连接\n");
			}
			if(trans_com_open(com,accept_fd)==0)
			{		
			#ifdef SHOW_WORK_INFO	
				printf("trans_com%d opened success!\n",com->ch);
			#endif
				gtloginfo("%s 成功打开透明串口%d\n",inet_ntoa(guest_addr.sin_addr),com->ch);
				if(max_fd<com->net_fd)
				{
					max_fd=com->net_fd;
				}
				//FD_SET(com->net_fd,&readfds);	//将新建立的连接描述符加入到等待队列
				net_set_nodelay(com->net_fd);
				net_activate_keepalive(com->net_fd);
				net_set_recv_timeout(com->net_fd,3);//
				net_set_noblock(com->net_fd,1);
				ret=1;
				setsockopt(com->net_fd,SOL_SOCKET ,SO_RCVLOWAT ,&ret,sizeof(ret));
			}
			else
			{
			#ifdef SHOW_WORK_INFO	
				printf("trans_com_thread can't open com1 !\n");
			#endif
				gtloginfo("%s 打开透明串口%d失败,关闭连接\n",inet_ntoa(guest_addr.sin_addr),com->ch);
				close(accept_fd);
				continue;
			}

		}
		else if(FD_ISSET(com->net_fd,&readfds))
		{
			//usleep(50000);//wsy,睡50毫秒
			if(read_net2com(com,net_rec_buf,1)!=0)//FIXME 1024
			{//出现错误
			#ifdef SHOW_WORK_INFO	
				printf("read_net2com remote close connection:%d\n",com->net_fd);
			#endif

				trans_com_close(com,&readfds,NULL);
			}		
			
			//write(com->net_fd,tempbuf,6);
			
		}
		else if(FD_ISSET(com->local_fd,&readfds))
		{

			if(read_com2net(com,net_rec_buf,1)!=0)// FIXME 1024
			{//出现错误
			#ifdef SHOW_WORK_INFO
				printf("read_com2net remote close connection:%d\n",com->net_fd);
			#endif
				gtlogerr("fd %d读取透明串口%d数据出错,关闭透明串口\n",com->net_fd,com->ch);
				trans_com_close(com,&readfds,NULL);
			}

		}
		else
		{
			printf("no fd is set\n");
		}
	}

	return NULL;	
}

/****************************************************************************************                                                        
 *函数名:keepalive_open_com_internal()
 *功能  :打开com_internal，获取串口fd，用来发送心跳数据,此串口为内部调试用串口                                                                
 *输入  :无                                                                                                                                      
 *输出  :无
 *返回  :正常返回0，错误返回负值                                                                                                                 
 ***************************************************************************************/ 
int keepalive_open_com_internal(void)
{
  struct trans_com_struct *com=NULL;
  int ret=0;
 
  com=&trans_com[2]; 
  com->baud=9600;
	com->databit=8;
	com->parity='N';
	com->stopbit=1;
	com->flow=0;

	ret=trans_com_open(com,0);

  return ret;
}

/*****************************************************************************************
 *函数名:keepalive_send_com_internal(int alarmchannel,int audiochannel,int heart)
 *功能  :通过调试串口向单片机发送字符串，用来表示本设备没有死机
 *输入  :alarmchannel,核警中的通道号高优先级，audiochannel 下行音频通道号，只要此值大于等于0，即表示心跳存在
 *输出  :无
 *返回  :正确返回0，错误返回负值
 * ***************************************************************************************/


int keepalive_send_com_internal(int alarmchannel,int audiochannel,int heart)
{
  struct trans_com_struct *com=NULL;
 // char cmd_buff[6]={0};
  GT_SUB_CMD_STRUCT cmd_buff;
  int ret=0;
  int j=0; 
  BYTE bitsrc = 1;
  BYTE temp = 0;
  
  memset((void*)&cmd_buff,0,sizeof(GT_SUB_CMD_STRUCT));
  cmd_buff.start = 0xfe;
  cmd_buff.cmd   = 0x22;
  cmd_buff.ichannel = 0x04;
  if(alarmchannel < 0)
  	{
  	  printf("audiochannel is %d\n",audiochannel);
	  cmd_buff.bitstr = bitsrc << audiochannel;
  	}
  else
  	{
  	  printf("alarmchannel is %d\n",alarmchannel);
  	  cmd_buff.bitstr = bitsrc << alarmchannel;
  	}
  cmd_buff.heart = heart?1:0;

  printf("bitstr is %02x\n",cmd_buff.bitstr);
  //printf("keepalive_send_com_internal audiochannel is %d\n",audiochannel);

  for(j=0;j<sizeof(GT_SUB_CMD_STRUCT);j++)
  {
	temp ^= *((BYTE*)&cmd_buff+j);
  }
  cmd_buff.crc = temp;

  //printf("cmd_buff.crc is %02x\n",cmd_buff.crc);
  
  com=&trans_com[2];
  if(com->local_fd>0)
  {
    ret=write(com->local_fd,(BYTE*)&cmd_buff,sizeof(GT_SUB_CMD_STRUCT));
    if(ret!=sizeof(GT_SUB_CMD_STRUCT))
    {
      printf("通过com0发送心跳字符串错误,发送字节数=%d\n",ret);
    }
    //printf("fd=%d,write ret=%d\n",com->local_fd,ret);
  }
  else
  {
    printf("com0的local_fd无效,local_fd=%d\n",com->local_fd);
  }

  return ret;
}

int keepalive_send_com(int audiochannel,int heart)
{
	struct trans_com_struct *com=NULL;
 // char cmd_buff[6]={0};
  GT_SUB_CMD_STRUCT cmd_buff;
  int ret=0;
  int j=0; 
  BYTE bitsrc = 1;
  BYTE temp = 0;
  
  memset((void*)&cmd_buff,0,sizeof(GT_SUB_CMD_STRUCT));
  cmd_buff.start = 0xfe;
  cmd_buff.cmd   = 0x22;
  cmd_buff.ichannel = 0x04;

  if(audiochannel >= 0)
  	cmd_buff.bitstr = bitsrc << audiochannel;
  else
  	cmd_buff.bitstr = bitsrc << 4;
  cmd_buff.heart = heart?1:0;

  printf("bitstr is %02x\n",cmd_buff.bitstr);
  //printf("keepalive_send_com_internal audiochannel is %d\n",audiochannel);

  for(j=0;j<sizeof(GT_SUB_CMD_STRUCT);j++)
  {
	temp ^= *((BYTE*)&cmd_buff+j);
  }
  cmd_buff.crc = temp;

  //printf("cmd_buff.crc is %02x\n",cmd_buff.crc);
  
  com=&trans_com[2];
  if(com->local_fd>0)
  {
    ret=write(com->local_fd,(BYTE*)&cmd_buff,sizeof(GT_SUB_CMD_STRUCT));
    if(ret!=sizeof(GT_SUB_CMD_STRUCT))
    {
      printf("通过com0发送心跳字符串错误,发送字节数=%d\n",ret);
    }
  }
  else
  {
    printf("com0的local_fd无效,local_fd=%d\n",com->local_fd);
  }

  return ret;
}


static int is_updating = 0;
static int debug_for_test;
//2013-6-7 lc do
int keepalive_set_com_mode(int enable,int interval)
{
  struct trans_com_struct *com=NULL;
 // char cmd_buff[6]={0};
  GT_SUB_CMD_STRUCT cmd_buff;
  int ret=0;
  int j=0; 
  BYTE bitsrc = 1;
  BYTE temp = 0;

if(!is_updating)
{
  if(debug_com_open == 0)
  	return 0;
  
  memset((void*)&cmd_buff,0,sizeof(GT_SUB_CMD_STRUCT));
  cmd_buff.start = 0xfe;
  cmd_buff.cmd   = 0x20;
  cmd_buff.ichannel = 0x04;
  cmd_buff.bitstr = 0;
  //cmd_buff.heart = 0;
  if((debug_for_test++) %2 == 0)
  	cmd_buff.heart = 1;
  else
  	cmd_buff.heart = 0;
  cmd_buff.reserved[0] = enable?1:0;  
  cmd_buff.reserved[1] = interval;

  printf("serial heart is %02x\n",cmd_buff.heart);
  
  gtloginfo("serial enable is %02x,interval is %02x\n",cmd_buff.reserved[0],cmd_buff.reserved[1]);
  
  for(j=0;j<sizeof(GT_SUB_CMD_STRUCT);j++)
  {
	temp ^= *((BYTE*)&cmd_buff+j);
  }
  cmd_buff.crc = temp;

  //gtloginfo("cmd_buff.crc is %02x\n",cmd_buff.crc);
  
  com=&trans_com[2];
  if(com->local_fd>0)
  {
    ret=write(com->local_fd,(BYTE*)&cmd_buff,sizeof(GT_SUB_CMD_STRUCT));
    if(ret!=sizeof(GT_SUB_CMD_STRUCT))
    {
      printf("通过com0发送心跳字符串错误,发送字节数=%d\n",ret);
    }
    //printf("fd=%d,write ret=%d\n",com->local_fd,ret);
  }
  else
  {
    printf("com0的local_fd无效,local_fd=%d\n",com->local_fd);
  }
}
  return ret;
}

int update_set_com_mode(int enable,int interval)
{
	
struct trans_com_struct *com=NULL;
 // char cmd_buff[6]={0};
  GT_SUB_CMD_STRUCT cmd_buff;
  int ret=0;
  int j=0; 
  BYTE bitsrc = 1;
  BYTE temp = 0;

  memset((void*)&cmd_buff,0,sizeof(GT_SUB_CMD_STRUCT));
  cmd_buff.start = 0xfe;
  cmd_buff.cmd   = 0x20;
  cmd_buff.ichannel = 0x04;
  cmd_buff.bitstr = 0;
  cmd_buff.reserved[0] = enable?1:0;  
  cmd_buff.reserved[1] = interval;

  printf("serial heart is %02x\n",cmd_buff.heart);
  
  gtloginfo("serial enable during update is %02x,interval is %02x\n",cmd_buff.reserved[0],cmd_buff.reserved[1]);
  
  for(j=0;j<sizeof(GT_SUB_CMD_STRUCT);j++)
  {
	temp ^= *((BYTE*)&cmd_buff+j);
  }
  cmd_buff.crc = temp;

  //gtloginfo("cmd_buff.crc is %02x\n",cmd_buff.crc);
  
  com=&trans_com[2];
  if(com->local_fd>0)
  {
    ret=write(com->local_fd,(BYTE*)&cmd_buff,sizeof(GT_SUB_CMD_STRUCT));
    if(ret!=sizeof(GT_SUB_CMD_STRUCT))
    {
      printf("通过com0发送心跳字符串错误,发送字节数=%d\n",ret);
    }
    //printf("fd=%d,write ret=%d\n",com->local_fd,ret);
  }
  else
  {
    printf("com0的local_fd无效,local_fd=%d\n",com->local_fd);
  }

  is_updating = 1;

  return ret;
}


