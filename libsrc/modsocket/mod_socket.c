/*	模块间采用基于udp的socket通讯的函数库
 *		wsy
 *		 2007.9
 *    本函数库提供了实现各模块间采用基于udp的socket通讯需要的最底层接口
 *    上层的应用接口及数据结构不属于本库的实现范围
 */
#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#endif
#include "mod_socket.h"					//模块间socket通讯的命令字定义
#include "gtsocket.h"
#include "gtlog.h"
#include "gtthread.h"

#define		MOD_MULTICAST_ADDR	"227.0.0.1"		//模块间多播的地址(本机)
#define		MOD_SOCKET_PORT			0x7654		//模块间socket通讯的端口
#define		MOD_SOCKET_MAGIC			0x8135		//模块间socket通讯的魔术字，用于区分正确命令


/**********************************************************************************************
 * 命令字结构,在实际应用中由于各种命令的参数个数不同,所以使用本
 * 结构定义的时候参数个数只是4,
 * 使用时需要将命令缓冲区强制转换成指向本结构的指针以便使用
 **********************************************************************************************/

typedef	struct 
{
	DWORD	magic;		//0xfedc之类，表示这是一个靠谱的命令.
	DWORD	source;		//发出命令的模块，如VMMAIN_ID等等
	DWORD	target;		//接收命令的模块，如ALL_PROCESS或VMMAIN_ID等
	DWORD	reserve;
	DWORD	reserve2;	//闲着也是闲着。。如果需要的话可以用作guid
	DWORD	len;		//data中的有效字节数
	BYTE    data[4];	//数据...
}mod_socket_type;

static int sendflag = 0;	//发送数据时使用的属性flags,在init时设定
static int recvflag = 0;	//接受数据时使用的属性flags


/**********************************************************************************************
 * 函数名	:mod_socket_init()
 * 功能	:	在指定的端口建立并绑定udp socket，并返回之
 * 输入	:	send_flag,	发送数据时使用的属性flags,一旦设定,适用于整个进程
 *			recv_flag,	接受数据时使用的属性flags
 * 返回值	:负值表示失败
 *			  正整数  产生的文件描述符，以后通过它来发送或接收命令      
 **********************************************************************************************/
int mod_socket_init( IN int send_flag, IN int recv_flag)
{
	int fd = -1;
	
	fd = udp_create(MOD_MULTICAST_ADDR,MOD_SOCKET_PORT);
	if(fd < 0)
		return -EAGAIN;

	udp_add_multicast(fd, MOD_MULTICAST_ADDR);//支持多播
	sendflag = send_flag;
	recvflag = recv_flag;

	return fd;
}

/**********************************************************************************************
 * 函数名	:mod_socket_send()
 * 功能	:向指定的主机地址发送一个命令信息
 * 输入	:com_fd:由调用 'mod_socket_init()' 的返回值得到
 *		 target:目标模块的id，若为0则表示发送给所有模块
 *		 source:发送模块的id
 *		 cmdbuf:指向要发送的命令的缓冲区的指针(已经填充好相关信息)
 *		 cmdlen:缓冲区中有效数据的长度
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int mod_socket_send(IN int com_fd,IN int target,IN int source,IN void *cmdbuf,IN int cmdlen)
{	
	struct sockaddr_in addr;
	mod_socket_type *cmd;
	char buf[MAX_MOD_SOCKET_CMD_LEN];
	int len=0;
	int rc;
	
	cmd = (mod_socket_type *)buf;
	
	memset(&addr,0,sizeof(addr));
	addr.sin_family	=	AF_INET;
	addr.sin_port	=	htons(MOD_SOCKET_PORT);
#ifdef _WIN32
	addr.sin_addr.s_addr=inet_addr(MOD_MULTICAST_ADDR);
#else
	inet_aton(MOD_MULTICAST_ADDR,&addr.sin_addr);
#endif
	//bzero(cmd,sizeof(mod_socket_type));
	if(cmdbuf!= NULL)
		memcpy(cmd->data,cmdbuf,cmdlen);
	cmd->magic	=	MOD_SOCKET_MAGIC;
	cmd->source	=	source;
	cmd->target	=	target;
	cmd->len	=	cmdlen;
	//printf("mod_socket_send cmd_len=%d\n",cmdlen);
	len	=	cmdlen+sizeof(mod_socket_type)-sizeof(cmd->data);
	errno = 0;
	rc = udp_send_data(com_fd,cmd,len,sendflag,&addr);
	if(rc!=len)
		return -errno;
	else
		return 0;
}

/**********************************************************************************************
 * 函数名: mod_socket_req_recv()
 * 功能:  从socket中接收一个命令包
 * 输入:
 *          com_fd:由调用 'mod_socket_init()' 的返回值得到
 *		 req_id:输入时是本模块的id,输出时表示接收到的数据包的目的id
 *		 source: 发来命令的模块id
 *		 cmdbuf:指向要发送的命令的缓冲区的指针(已经填充好相关信息)
 *		 cmd_maxlen:缓冲区的最大长度
 *		 source_addr:存放发送模块的ip，若为NULL表示不用取
 * 返回值:
 *         正值:接收到的信息的字节数
 *         负值:出错
 ************************************************************************************************/
int mod_socket_req_recv(IN int com_fd, IO int *req_id, OUT int *source, OUT void *cmdbuf,IN int cmd_maxlen, OUT char *source_addr)
{
	struct sockaddr_in addr;
	DWORD buf[MAX_MOD_SOCKET_CMD_LEN];
	mod_socket_type *cmd;
	int len;
       int  myid=*req_id;
	if(cmdbuf ==NULL )
		return -EINVAL;

	cmd = (mod_socket_type *)buf;
	memset(&addr,0,sizeof(addr));
	while(1)
	{
		len = udp_recv_data(com_fd, cmd,MAX_MOD_SOCKET_CMD_LEN,recvflag, &addr);
		if(len<=0)	//socket error
			return len;

		if((int)cmd->len > cmd_maxlen) //命令长度超过提供的缓冲区
		{
		    errno=ENOBUFS;
		    return -ENOBUFS;
              }
		
		if(cmd->magic != MOD_SOCKET_MAGIC) //魔术数不对
			continue;

		if(((int)cmd->target != myid)&&(cmd->target != ALL_PROCESS)&&(myid!=ALL_PROCESS))//不是发给我的
			continue;

              *source=cmd->source;
              *req_id=cmd->target;
		memcpy(cmdbuf,cmd->data,cmd->len);//拷贝命令
		//printf("mod_socket_req_recv cmd_len=%d\n",cmd->len);
		if(source_addr != NULL) //填充source ip
		{
			memcpy((void*)source_addr,(void*)inet_ntoa(addr.sin_addr),strlen(inet_ntoa(addr.sin_addr))+1);
		}
		return (len-(sizeof(mod_socket_type)-sizeof(cmd->data)));
	}
	
}

/**********************************************************************************************
 * 函数名: mod_socket_recv()
 * 功能:  从socket中接收一个命令包
 * 输入:
 *        com_fd:由调用 'mod_socket_init()' 的返回值得到
 *		 myid:本模块的id
 *		 source: 发来命令的模块id
 *		 cmdbuf:指向要发送的命令的缓冲区的指针(已经填充好相关信息)
 *		 cmd_maxlen:缓冲区的最大长度
 *		 source_addr:存放发送模块的ip，若为NULL表示不用取
 * 返回值:
 *         正值:接收到的信息的字节数
 *         负值:出错
 ************************************************************************************************/
int mod_socket_recv(IN int com_fd, IN int myid, OUT int *source, OUT void *cmdbuf,IN int cmd_maxlen, OUT char *source_addr)
{
    return mod_socket_req_recv(com_fd,&myid,source,cmdbuf,cmd_maxlen,source_addr);	
}



int send_ack_to_main(int com_fd, int mod_id, int cmd, int result, gateinfo *gate)
{
	DWORD send_buf[200];
	mod_socket_cmd_type *socketcmd;
	DWORD state;	
	struct usr_cmd_ack_struct *ack;
	
	socketcmd = (mod_socket_cmd_type *)send_buf;
	memcpy(&socketcmd->gate,gate,sizeof(gateinfo));//将gateinfo原样抄送
	socketcmd->cmd	=	MOD_BYPASSTO_GATE_ACK;
	ack = (struct usr_cmd_ack_struct *)socketcmd->para;
	ack->rec_cmd 	= cmd;
	ack->result 	= result;
	socketcmd->len	=	sizeof(struct usr_cmd_ack_struct);
	
	return mod_socket_send(com_fd,MAIN_PROCESS_ID,mod_id,socketcmd,sizeof(mod_socket_cmd_type)-sizeof(socketcmd->para)+socketcmd->len);
}


//监听来自主模块的查询并发送相关状态
void *recv_modsocket_thread (void *data)
{
	
	int len;
	int sourceid;
	DWORD buf[MAX_MOD_SOCKET_CMD_LEN/sizeof(DWORD)];      //shixin changed to DWORD
	char sourceaddr[100];
	mod_socket_cmd_type *modsocket;
	mod_socket_thread_data *threaddata;
	
	if(data == NULL)
		return NULL;

	threaddata = (mod_socket_thread_data *)data;
	
	printf("%s start recv_modsocket_thread!\n",threaddata->module_name);
	gtloginfo ("start recv_modsocket_thread!\n");
       
	
	while(1)
	{
		len = mod_socket_recv(threaddata->com_fd, threaddata->mod_id,&sourceid, &buf, MAX_MOD_SOCKET_CMD_LEN, sourceaddr);
		if(len > 0)
		{
			if(sourceid!=VIDEOENC_MOD_ID)
				printf("%s recved a module-cmd from id %d, ip %s\n",threaddata->module_name,sourceid,sourceaddr);
			modsocket = (mod_socket_cmd_type *)buf;
			threaddata->fn(sourceid,modsocket);
		}
              else
                {
                    sleep(1);
                }
	}
	return NULL;
}

int creat_modsocket_thread(pthread_t *thread_id, int com_fd, int mod_id, char *mod_name, int (*fn)(int sourceid, mod_socket_cmd_type *modsocket))
{
		
	mod_socket_thread_data *threaddata;
	
	threaddata = (mod_socket_thread_data *)malloc(sizeof (mod_socket_thread_data));
	threaddata->mod_id = mod_id;
	strncpy(threaddata->module_name,mod_name,MAX_MODULE_NAME_LEN); 
       threaddata->module_name[MAX_MODULE_NAME_LEN-1]='\0';
	threaddata->fn = fn;
	threaddata->com_fd = com_fd;
	return gt_create_thread(thread_id, recv_modsocket_thread, (void *)threaddata);

}

