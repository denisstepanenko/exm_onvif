/*	GT1000系列视频服务器组播命令(UDP)协议底层库函数
 *		 2006.9 shixin
 *    本函数库提供了实现嵌入式系统与远程计算机使用组播方式通讯的底层接口
 *    上层的应用接口及数据结构不属于本库的实现范围
 *    本函数库提供的功能是
 *    1.将明文数据报按照GT系列视频服务器的组播通讯协议格式进行打包，加密，crc32校验，之后发送给指定的地址
 *    2.从指定的socket中接收数据包，进行crc校验，将密文变换成明文返回给调用进程，并将数据包的发送者信息返回调用者
 *    
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <errno.h>

#ifdef _WIN32
	//windows
	#include <winsock2.h>
#else
	//linux
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

#include "gt_dev_api.h"
#include "tab_crc32.c"


/////////////////////////////////////////////////////////////////////////////////////////
static const unsigned char	broadcast_addr[]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

//static __inline__ void print_buffer(unsigned char *buf,int len)
static void print_buffer(unsigned char *buf,int len)
{
    int i;
    for(i=0;i<len;i++)
        printf("%02x,",buf[i]);
    return ;
}


/***************协议头、尾定义*******************************/
//函数库自己使用的定义
typedef struct{
	unsigned long	head;				//头标志
	unsigned char	version;			//协议版本号
	unsigned char	reserve;			//保留
	unsigned short	enc_type;			//数据包加密类型
	unsigned char	target_id[GUID_LEN];//目标地址(guid)
	unsigned char	source_id[GUID_LEN];//源地址(guid)
	unsigned short	reserve1;			//保留
	short			pkt_len;			//数据包长度(data中的有效字节数)
	unsigned long	xor32;				//包头的32bit异或校验码
	unsigned char	data[4];			//包中带的实际数据
}gtdev_pkt_struct;

static const unsigned long 	pkt_head=0x55aa5540;	//包头
static const unsigned char	proto_version=1;	//协议版本号

/********************************************************************/
#define	BUF_SIZE		(2048)	//底层库的缓冲区大小

/**********************************************************************************************
* 函数名	:send_dev_pkt()
* 功能	:将一段明文按照指定的加密方式形成命令数据包,发送到指定的socket地址
* 输入	:	fd:目标socket文件描述符
* 		sin:描述目标地址信息的结构指针
* 		target_id:目标id号
* 		source_id:发送者id号
* 		buf:要发送出去的数据缓冲区
* 		buflen:缓冲区中的有效字节数
* 		enc_type:要使用的加密方式()
* 		flag:发送选项，保留，暂时写0
* 返回值:	
* 		正值表示发送出去的字节数，负值表示出错
**********************************************************************************************/
int send_dev_pkt(IN int fd,IN struct sockaddr_in *sin,IN unsigned char *target_id,IN unsigned char *source_id,IN void *buf,int buflen,IN int enc_type,IN int flag)
{
	unsigned long 		send_buf[BUF_SIZE/4];
	int					pkt_len;
	unsigned long		*dw=NULL;
	unsigned long		*crc32=NULL;
	int					ret;
	int					addon;
	gtdev_pkt_struct 	*pkt=(gtdev_pkt_struct *)send_buf;

	if((fd<0)||(buflen<=0)||(buf==NULL))
		return -EINVAL;
	if(buflen>=(BUF_SIZE-sizeof(gtdev_pkt_struct)))
	{
		return -EINVAL;
	}
	flag=flag;
	pkt->head=pkt_head;
	pkt->version=proto_version;
	pkt->reserve=0;
	pkt->enc_type=enc_type;
	memcpy((void*)pkt->target_id,target_id,GUID_LEN);
	memcpy((void*)pkt->source_id,source_id,GUID_LEN);
	pkt->reserve1=0;

	if(enc_type==GTDEV_ENC_NONE)
	{
		memcpy((void*)pkt->data,buf,buflen);
		pkt_len=buflen;
	}
	else
	{
		printf("can't support enc mode %d\n",enc_type);
		return -EINVAL;
	}

	if((pkt_len%4)!=0)//按4字节对齐
	{
		addon=4-(pkt_len%4);
		memset((char*)pkt->data+pkt_len,0,addon);
	}
	else
		addon=0;

	pkt_len+=addon;
	pkt->pkt_len=pkt_len;

	dw=(unsigned long*)&pkt->version;
	pkt->xor32=dw[0]^dw[1]^dw[2]^dw[3]^dw[4]^dw[5];
	crc32=(unsigned long*)((char*)pkt->data+pkt_len);
	*crc32=tab_crc32((void*)pkt->data,pkt_len);
	ret=sendto(fd,(void*)pkt,(sizeof(gtdev_pkt_struct)-sizeof(pkt->data)+pkt_len+4),MSG_DONTROUTE,(struct sockaddr *)sin, sizeof(*sin));	
	if(ret<0)
		ret=-errno;
	return ret;
}



/**********************************************************************************************
* 函数名	:recv_dev_pkt()
* 功能	:	从指定接口接收一包数据，如果没有数据则阻塞
* 输入	:	fd:要接收文件的socket文件描述符
*		selfid:自己的id号
*		buflen:接收信息的缓冲区长度
*		flag:接收标志，保留，暂时写0
* 输出	:	
* 		sin:描述发送者地址信息的结构指针
* 		source_id:发送者id号
* 		msgbuf:接收到的信息
* 		enc_type:接收到的数据包的加密类型
* 返回值:	
* 		正值表示接收到msgbuf中的有效字节数，负值表示出错
*		-ENOMEM 表示缓冲区不足
**********************************************************************************************/
int recv_dev_pkt(IN int fd,OUT struct sockaddr_in *sin,IN unsigned char *selfid,OUT unsigned char *sourceid,OUT unsigned char *msgbuf,IN int buflen,OUT int *enc_type,IN int flag)
{
	unsigned long 		recv_buf[BUF_SIZE/4];//接收缓冲区
	gtdev_pkt_struct 	*pkt=(gtdev_pkt_struct *)recv_buf;
	int			ret;
	int			slen; 
	unsigned long		*dw=NULL;
	unsigned long		xor32,crc32,*pkt_crc32=NULL;
	if((fd<0)||(sin==NULL)||(selfid==NULL)||(sourceid==NULL)||(msgbuf==NULL)||(buflen<=0)||(enc_type==NULL))
		return -EINVAL;
	flag=flag;
	while(1)
	{
		slen=sizeof(*sin);
		ret=recvfrom(fd,(void *)pkt,sizeof(recv_buf),0,(struct sockaddr *)sin,&slen);
		if(ret<0)
		{
			if(errno==EINTR)
				continue;
			else
				return ret;
		}
		else if(ret==0)
		{
			continue;
		}
		//printf("recv %d bytes!!\n",ret);
		//print_buffer(recv_buf,ret);
		//printf("\n");
		if(pkt->head!=pkt_head)
			continue;	//不是协议包头
		dw=(unsigned long*)&pkt->version;
		xor32=dw[0]^dw[1]^dw[2]^dw[3]^dw[4]^dw[5];
		if(xor32!=pkt->xor32)
		{
			//printf("xor32 0x%x != 0x%x\n",xor32,pkt->xor32);
			continue;	//包头校验不对
		}
		if(memcmp(pkt->source_id,selfid,GUID_LEN)==0)
			continue;	//如果是自己发送的包则不处理
		if((memcmp(pkt->target_id,broadcast_addr,GUID_LEN)!=0)&&(memcmp(pkt->target_id,selfid,GUID_LEN)!=0))
		{
			continue;	//不是期望的地址
		}
		pkt_crc32=(unsigned long*)((char*)pkt->data+pkt->pkt_len);
		crc32=tab_crc32(pkt->data,pkt->pkt_len);
		if(crc32!=*pkt_crc32)
		{
		//	printf("crc32 0x%x != 0x %x\n",crc32,*pkt_crc32);
			continue;	//crc码错误
		}
		*enc_type=pkt->enc_type;
		memcpy(sourceid,pkt->source_id,GUID_LEN);
		if(pkt->enc_type==GTDEV_ENC_NONE)
		{
			if(buflen<pkt->pkt_len)
				return -ENOMEM;
			memcpy(msgbuf,pkt->data,pkt->pkt_len);
			return pkt->pkt_len;
		}
		else
		{
			printf("receive unknow enc_type:%d pkt",enc_type);
			continue;	//不认识的加密格式
		}
	}
	return 0;
}

/**********************************************************************************************
 * * 函数名        :dual_id_recv_dev_pkt()
 * * 功能  :       从指定接口接收一包数据，如果没有数据则阻塞
 * * 输入  :       fd:要接收文件的socket文件描述符
 * *               selfid:自己的id号，长度为 GUID_LEN
 * *		   selfid1:另外一个id号 长度为GUID_LEN
 * *               buflen:接收信息的缓冲区长度
 * *               flag:接收标志，保留，暂时写0
 * * 输出  :       
 * *               sin:描述发送者地址信息的结构指针
 * *               source_id:发送者id号 长度为GUID_LEN
 * *               msgbuf:接收到的信息
 * *               enc_type:接收到的数据包的加密类型
 * *	           recv_id:接收到的id号 
 * * 返回值:       
 * *               正值表示接收到msgbuf中的有效字节数，负值表示出错
 * *               -ENOMEM 表示缓冲区不足
 * *	lsk 2009-2-9 增加了一个输入的id，两个id任何一个正确都能够接收数据包
 * **********************************************************************************************/
EXPORT_DLL int dual_id_recv_dev_pkt(IN int fd,OUT struct sockaddr_in *sin,IN unsigned char *selfid, IN unsigned char *selfid1,OUT unsigned char *sourceid,OUT unsigned char *recv_id, OUT unsigned char *msgbuf,IN int buflen,OUT int *enc_type,IN int flag)
{
	unsigned long 		recv_buf[BUF_SIZE/4];//接收缓冲区
	gtdev_pkt_struct 	*pkt=(gtdev_pkt_struct *)recv_buf;
	int			ret;
	int			slen; 
	unsigned long		*dw=NULL;
	unsigned long		xor32,crc32,*pkt_crc32=NULL;
	//// lsk 2009 -2-9
	if((fd<0)||(sin==NULL)||(selfid==NULL)||(recv_id==NULL)||(selfid1==NULL)||(sourceid==NULL)||(msgbuf==NULL)||(buflen<=0)||(enc_type==NULL))
		return -EINVAL;
	flag=flag;
	while(1)
	{
		slen=sizeof(*sin);
		ret=recvfrom(fd,(void *)pkt,sizeof(recv_buf),0,(struct sockaddr *)sin,&slen);
		if(ret<0)
		{
			if(errno==EINTR)
				continue;
			else
				return ret;
		}
		else if(ret==0)
		{
			continue;
		}
		//printf("recv %d bytes!!\n",ret);
		//print_buffer(recv_buf,ret);
		//printf("\n");
		if(pkt->head!=pkt_head)
			continue;	//不是协议包头
		dw=(unsigned long*)&pkt->version;
		xor32=dw[0]^dw[1]^dw[2]^dw[3]^dw[4]^dw[5];
		if(xor32!=pkt->xor32)
		{
			//printf("xor32 0x%x != 0x%x\n",xor32,pkt->xor32);
			continue;	//包头校验不对
		}
		//// lsk 2009-2-9
		if((memcmp(pkt->source_id,selfid,GUID_LEN)==0)||(memcmp(pkt->source_id,selfid1,GUID_LEN)==0))
			continue;	//如果是自己发送的包则不处理2009 -2-9 lsk 
		if((memcmp(pkt->target_id,broadcast_addr,GUID_LEN)!=0)&&(memcmp(pkt->target_id,selfid,GUID_LEN)!=0)&&(memcmp(pkt->target_id,selfid1,GUID_LEN)!=0))
		{
			continue;	//不是期望的地址
		}
		else	//// lsk 2009 -2-9 
		{
			memcpy(recv_id,pkt->target_id,GUID_LEN);
		}
		pkt_crc32=(unsigned long*)((char*)pkt->data+pkt->pkt_len);
		crc32=tab_crc32(pkt->data,pkt->pkt_len);
		if(crc32!=*pkt_crc32)
		{
		//	printf("crc32 0x%x != 0x %x\n",crc32,*pkt_crc32);
			continue;	//crc码错误
		}
		*enc_type=pkt->enc_type;
		memcpy(sourceid,pkt->source_id,GUID_LEN);
		if(pkt->enc_type==GTDEV_ENC_NONE)
		{
			if(buflen<pkt->pkt_len)
				return -ENOMEM;
			memcpy(msgbuf,pkt->data,pkt->pkt_len);
			return pkt->pkt_len;
		}
		else
		{
			printf("receive unknow enc_type:%d pkt",enc_type);
			continue;	//不认识的加密格式
		}
	}
	return 0;
}

