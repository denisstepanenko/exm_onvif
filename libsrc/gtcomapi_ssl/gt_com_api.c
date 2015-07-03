/*	嵌入式视频服务器与计算机间通讯的底层函数库
 *		
 *		 2005.1
 *    本函数库提供了实现嵌入式系统与远程计算机通讯的底层接口，
 *    上层的应用接口及数据结构不属于本库的实现范围
 *    本函数库提供的功能是
 *    1.将明文数据报按照GT1000的通讯协议格式进行打包，加密，签名，crc32校验，之后发送给指定的tcp端口
 *    2.从指定的tcp端口接收数据包，进行crc校验，验证签名，将密文变换成明文返回给调用进程                        
 *    接收的时候本库可以通过协议头中的定义判断数据包中使用的签名方式，加密方式等
 *    发送的时候根据参数选择签名方式，加密方式等	
 */
/*
 * 基本数据结构定义
 */
#define  _CRT_SECURE_NO_WARNINGS
#define  _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <errno.h>
#include <gtsocket.h>
#include <gtlog.h>
#ifdef _WIN32
//	#pragma comment(lib,"gtsocket.lib")
#define get_unaligned(ptr)        (*(ptr))
#define put_unaligned(val, ptr) ((void)( *(ptr) = (val) ))
#else
//#include <asm/unaligned.h> 	//处理字节对齐
//get_unaligned(ptr); 
//put_unaligned(val, ptr); 
#endif
//#define put_unaligned(val,ptr)	*ptr=val
//#define get_unaligned(ptr)	*ptr


static inline unsigned int __get_unaligned_le32(const unsigned char *p)
{
        return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}
static inline void __put_unaligned_le16(unsigned short val, unsigned char *p)
{
        *p++ = val;
        *p++ = val >> 8;
}
static inline void __put_unaligned_le32(unsigned int val, unsigned char *p)
{
        __put_unaligned_le16(val >> 16, p + 2);
        __put_unaligned_le16(val, p);
}

static inline unsigned int __get_unaligned_be32(const unsigned char *p)
{
        return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
}
static inline void __put_unaligned_be16(unsigned short val, unsigned char *p)
{
        *p++ = val >> 8;
        *p++ = val;
}
static inline void __put_unaligned_be32(unsigned int val, unsigned char *p)
{
        __put_unaligned_be16(val >> 16, p);
        __put_unaligned_be16(val, p + 2);
}

#ifndef __ARMEB__
#define put_unaligned(val, ptr) ({                                 \
        void *__gu_p = (ptr);                                           \
        __put_unaligned_le32((unsigned int)(val), __gu_p);            \
         (void)0; })

#define get_unaligned(ptr) ({                                 \
        __get_unaligned_le32(ptr);            })

#else
#define put_unaligned(val, ptr) ({                                 \
        void *__gu_p = (ptr);                                           \
        __put_unaligned_be32((unsigned int)(val), __gu_p);        })
#define get_unaligned(ptr) ({                                 \
        __get_unaligned_be32(ptr);            })

#endif


#include "gt_com_api.h"

#ifdef USE_SSL
#include "envelop/gt_env.h"
#endif
/***************协议头、尾定义*******************************/
//函数库自己使用的定义
#define GT_SYNC_HEAD0	'@'		//协议头标志0
#define GT_SYNC_HEAD1	0x55	//协议头标志1	
#define GT_SYNC_HEAD2	0xaa	//协议头标志2
#define GT_SYNC_HEAD3	0x55	//协议头标志3

#define GT_PKT_TAIL	0x66886623	//协议尾标志
/********************************************************************/

#ifdef USE_SSL
static int get_addon_len(BYTE auth_type,BYTE encrypt_type);
#endif

#define EBUF_ADDON			512	//加密用缓冲区应当比解密缓冲区大的长度

static  int print_buffer(unsigned char *buf,int len)
{
    int i;
    for(i=0;i<len;i++)
	printf("%02x,",buf[i]);
    return 0;
}
int  gt_cmd_pkt_recv_env(int fd,struct gt_pkt_struct *recv,int buf_len,void *tempbuf,int flag,int *msg_type,int *encrypt_type);
extern unsigned long tab_crc32 (const unsigned char *buf, unsigned int len);
//函数接口定义
/**********************************************************************************************
 * 函数名	:gt_cmd_pkt_send()
 * 功能	:将一段明文按照指定的加密,签名方式形成命令数据包,发送到指定的socket
 * 输入	:fd:目标socket文件描述符
 *			 send:指向要发送的命令的缓冲区的指针
 *			 msg_len:要发送的缓冲区中的有效信息包的字节数(msg字段,不包括session,crc,tail等的长度)
 *			 tempbuf:(保留)，调用时传NULL就可以了
 *			 flag:(保留)，调用时传0就可以了
 *			 msg_type:发送数据包的认证类型,0表示不需要认证,MSG_AUTH_SSL表示使用数字信封，其他值待定
 *			 encrypt_type:发送数据包的加密类型,具体定义见"数据加密类型选项"
 * 返回值	:正值表示发送出去的字节数，负值表示出错
 * 注		: 调用者应当分配足够大的缓冲区以容纳要发送的数据,本函数将直接使用该缓冲区进行各种填充,变换等操作
 *	  		  调用着只需填充send->msg字段即可，其它字段由库函数自己填充
 *	 		 send以缓冲区的起始地址必须是4字节对齐的，否则上层应用处理可能会出问题
 **********************************************************************************************/
int  gt_cmd_pkt_send(int fd,struct gt_pkt_struct *send,int msg_len,void *tempbuf,int flag,BYTE msg_type,BYTE encrypt_type)
{
	struct gt_pkt_struct *s=NULL;
	struct gt_pkt_head *head;
	DWORD	*ph,xor32,crcval;
	int	ret,crclen;
	WORD addon;
	void *ebuf;
#ifdef USE_SSL
	int plen, elen;
	DWORD tbuf[256];//sizeof buf;
	void *pbuf;
#endif
	int send_msglen;

	BYTE *buf;
	int mallocflag=0;//分配内存标志
	if(((long)send%4)!=0)
		printf("unaligned !\n");
	
	if(fd<0)
	{
		printerr("\ngt_cmd_pkt_send get fd=%d!error\n",fd);
		return -EINVAL;
	}
	if(send==NULL)
	{
		printerr("\ngt_cmd_pkt_send get NULL sendbuf!error\n");
		return -EINVAL;
	}
	ebuf=NULL;

	s=send;
	send_msglen=msg_len;
	buf=(char*)s->msg;
	if(((send_msglen)%4)!=0)   //	4字节对齐
	{
		addon=4-(send_msglen)%4;
		memset((char*)(buf+send_msglen),0,addon);
	}
	else
	{
		addon=0;
	}
	send_msglen+=addon;
	addon=0;
	
	if(msg_type==GT_CMD_NO_AUTH)
	{//不需要使用数字信封，不加密


	}
	else
	{
#ifdef USE_SSL
	    if(msg_type==MSG_AUTH_SSL)
	    {//采用数字信封

		
	    		//s=(struct gt_pkt_struct*)calloc(4,(EBUF_ADDON+msg_len)/4);//,4);
			/*tbuf=malloc(EBUF_ADDON+msg_len);//FIXME 考虑一下是否需要动态分配内存
			if(tbuf==NULL)
			{
				printerr("gt_com_api gt_cmd_pkt_send not enough memory\n");
				return -1;
			}
			*/	
			if((EBUF_ADDON+msg_len)<sizeof(tbuf))
			{
				s=(struct gt_pkt_struct *)(tbuf);
				mallocflag=0;
			}
			else
			{
				ret=posix_memalign(&s,sizeof(unsigned long),EBUF_ADDON+msg_len);
				if(ret!=0)
				{
					printf("gt_cmd_pkt_send posix_memalign failed ret=%d :%s!!\n",ret,strerror(ret));
					return -ret;
				}
				else
				{
					mallocflag=1;
					printf("gt_cmd_pkt_send posix_memalign s=%x ret=%d\n",s,ret);
				}

			}
			
			ebuf=(void *)s->msg;
			elen=EBUF_ADDON+send_msglen-sizeof(s->head);
			memcpy((void*)s,(void*)&send->head,sizeof(send->head));
			
			pbuf=(void*)send->msg;
			plen=send_msglen;
			ret=env_pack(encrypt_type,pbuf,plen,ebuf,&elen);
			if(ret!=0)
			{
				printerr("gt_com_api gt_cmd_pkt_send env_pack error %d\n",ret);
                            gtlogerr("gt_cmd_pkt_send  加密错误 !env_pack ret=%d etype=%d elen=%d\n",ret,encrypt_type,elen);
			
				return -EIO;
			}			
			send_msglen=elen;
			addon=0;
				
	    }
	    else
#endif
	    {
	    	printerr("gt_com_api lib can't support msg_type=0x%x now!\n",msg_type);
	    	return -EINVAL;
	    }
	}

	    head=&s->head;
	    buf=s->msg;		
	    send_msglen+=addon;
	    addon=send_msglen+sizeof(struct gt_pkt_head)-4-2+4;//-HEADFLAG(4)-LEN(2)+CRC(4)		
	    put_unaligned(addon,&head->pkt_len);
		head->pkt_len=addon;		
	    crclen=send_msglen;
		
	buf=s->msg;
	head->sync_head[0]=GT_SYNC_HEAD0;
	head->sync_head[1]=GT_SYNC_HEAD1;
	head->sync_head[2]=GT_SYNC_HEAD2;
	head->sync_head[3]=GT_SYNC_HEAD3;
	put_unaligned(GT_COM_VERSION, &head->version);
	put_unaligned(msg_type,&head->msg_type);
	put_unaligned(encrypt_type,&head->encrypt_type);
	put_unaligned(0,&head->reserve0);
	put_unaligned(0,&head->reserve1);

	ph=(DWORD *)head;
	//xor32=ph[1]^ph[2];//数据包的同步头不需要参与异或校验
	ph++;
	xor32=(ph[0])^(ph[1]);//get_unaligned(ph)^get_unaligned(ph+1);
	//printf("ph=%x %x^%x=%x!!!!!!!!!!!!\n",(int)ph,ph[0],ph[1],xor32);
	head->xor32=xor32;//put_unaligned(xor32,&head->xor32);

	buf=s->msg;
	crcval=tab_crc32(buf,crclen);
	memcpy((void*)(buf+crclen),(void*)&crcval,sizeof(crcval));
	crclen+=sizeof(crcval);
	ph=(DWORD*)(buf+crclen);
	put_unaligned(GT_PKT_TAIL,ph);
	crclen+=4;
	crclen+=sizeof(struct gt_pkt_head);


	buf=(char*)s;
	ret=net_write_buf(fd,buf,crclen);
	//printf("send->");//shixin test!!!
	//print_buffer(buf,crclen);
	//printf("\n");
	if(ret>=0)
		ret=0;
	else
		ret=ret;
	if(mallocflag)
	{
		if(s!=NULL)
			free(s);
	}
	return ret;	
}

/*
 * 函数名: gt_cmd_pkt_recv()
 * 功能:  从指定的socket接收一个数据包,进行解密,验证签名等操作后,把明文填充到recv指向的缓冲区
 * 返回值:
 *         0:接收到一包数据
 *         负值:出错
 *		-1:度数据错，可能是网络连接已经断开		
 *      -2:缓冲区溢出
 *		-3:CRC错
 *		-4:不支持的加密格式
 *		-5:解密错误
 *
 *         	其他值待定
 *         recv:已经填充好的数据缓冲区
 * 参数:
 *         fd:  目标socket文件描述符
 *         recv:指向要存放接收数据的缓冲区,缓冲区应该是一个能容纳最长命令的buffer
 *         buf_len:recv缓冲区的大小,当接收数据大于这个值的时候,本函数应该返回出错信息,表示缓冲区溢出(这种情况应该不会发生)
 *         flag:需要以什么属性来执行本函数,具体值跟linux socket编程的相应参数定义相同,只要原样传送给socket函数即可
 *	     tempbuf:(保留)，调用时传NULL就可以了
 * 说明:   调用者应当分配足够大的缓冲区后调用本函数,本函数如果发现接收的数据大于缓冲区大小,应当返回缓冲区溢出信息
 * 	   调用者只需关心返回值和recv->msg字段即可，其他字段对调用者来说是无效的
 *	   recv以及tempbuf两个缓冲区的起始地址必须是4字节对齐的，否则上层应用处理可能会出问题
 */
int  gt_cmd_pkt_recv_env(int fd,struct gt_pkt_struct *recv,int buf_len,void *tempbuf,int flag,int *msg_type,int *encrypt_type)
{
	#define TMPBUF_LEN 1024	//应用程序中定义的tempbuf大小，如果超过这个值则需要重新分配
	#define judge_exit(r)		do	{if((r)<=0){	if((r)==0) errflag=-140;else errflag=r;	goto exit;}}while (0)	
	
	DWORD tbuf[TMPBUF_LEN/4];
	struct gt_pkt_struct *s,*s2;
	struct gt_pkt_head *head;
	DWORD	*ph,xor32,*reccrc,crcval,*tail;
	int	pkt_len,i;
	unsigned char 	*pd,*ptest;
	int elen,plen;
	int mtype,etype;
	int ret;
#ifdef USE_SSL
	void *pbuf,*ebuf;
#endif
	int mallocflag=0;
	int errflag=0;
	if(fd<0)
	{
		printerr("\ngt_cmd_pkt_recv get fd=%d!error\n",fd);
		return -EINVAL;
	}
	if(recv==NULL)
	{
		printerr("\ngt_cmd_pkt_recv get NULL recvbuf!error\n");
		return -EINVAL;
	}
	if((msg_type==NULL)||(encrypt_type==NULL))
	{
		mtype=GT_CMD_NO_AUTH;		//在没有数字信封的情况下不需要加密
		etype=GT_CMD_NO_ENCRYPT;
		s=recv;
	}
	else
	{
		mtype=*msg_type;
		etype=*encrypt_type;
		s=(struct gt_pkt_struct *)tbuf;
		elen=buf_len;
		plen=buf_len;
		
	}
	
	head=&s->head;
	pd=(unsigned char*)head;
getheader:
//	printf("\nprepare to recv cmd:");
	pd=(char*)head->sync_head;
	ret=net_read_buf(fd,pd,1);
	judge_exit(ret);
	if(*pd!=GT_SYNC_HEAD0)//读取并判断第一个同步头
		goto getheader;
	pd++;
	ret=net_read_buf(fd,pd,1);
	judge_exit(ret);
	if(*pd!=GT_SYNC_HEAD1)//读取并判断第二个同步头
		goto getheader;
	pd++;
	ret=net_read_buf(fd,pd,1);
	judge_exit(ret);
	if(*pd!=GT_SYNC_HEAD2)//读取并判断第三个同步头
		goto getheader;
	pd++;
	ret=net_read_buf(fd,pd,1);
	judge_exit(ret);
	if(*pd!=GT_SYNC_HEAD3)//读取并判断第四个同步头
		goto getheader;

	pd++;
	ret=net_read_buf(fd,pd,sizeof(struct gt_pkt_head)-4);
	judge_exit(ret);
	if((ret!=(sizeof(struct gt_pkt_head)-4)))  //接收命令包头(刨除同步头的字节)
		goto getheader;
//	printf("sync ok->");
	pkt_len=head->pkt_len-sizeof(struct gt_pkt_head)+4+2-4;		//去掉crc
	if((int)(pkt_len+sizeof(struct gt_pkt_head)+20)>buf_len)//20 是多加的
	{
		printf("too long pkt len=%d ,droped\n",pkt_len);	//数据包长度超过缓冲区长度
		goto getheader;
	}
	ph=(DWORD *)(&head->pkt_len);
	xor32=get_unaligned(ph) ^ get_unaligned((ph+1));
	


/*	ptest=(char *)ph;
	for(i=0;i<(sizeof(struct gt_pkt_head)-4);i++)
	    printf("%02x,",ptest[i]);
*/
	if(xor32 != get_unaligned(&head->xor32))
	{
		//命令包头校验错误
		printf("packet head xor error %x!=%x!\n",(int)xor32,(int)get_unaligned(&head->xor32));
		print_buffer((void*)head,sizeof(struct gt_pkt_head));
		printf("\n");
		goto getheader;
	}

	if((pkt_len+sizeof(struct gt_pkt_head)+EBUF_ADDON)>TMPBUF_LEN)
	{
		#ifdef _WIN32
			//windows
			//FIXME 对齐的内存分配函数
			ret=0;
			s2=malloc(pkt_len+sizeof(struct gt_pkt_head)+EBUF_ADDON);
		#else
			//linux
			ret=posix_memalign(&s2,sizeof(unsigned long),(pkt_len+sizeof(struct gt_pkt_head)+EBUF_ADDON));
		#endif	
		if((ret!=0)||(s2==NULL))
		{
			printf("gt_cmd_pkt_recv_env posix_memalign error ret=%d:%s\n",ret,strerror(ret));
			goto getheader;
		}
		else
		{
			memcpy(s2,s,sizeof(struct gt_pkt_head));
			s=s2;
			mallocflag=1;	
			printf("gt_cmd_pkt_recv_env posix_memalign s=%x ret=%d\n",(int)s,ret);
		}
	}

		
	pd=s->msg;//包头已经读过了
	ret=net_read_buf(fd,pd,(pkt_len));
	judge_exit(ret);
	pd+=(pkt_len);
	reccrc=(DWORD*)pd;

	ret=net_read_buf(fd,pd,4);
	judge_exit(ret);
	pd+=4;
	
	ret=net_read_buf(fd,pd,4);
	judge_exit(ret);
	
	tail=(DWORD*)pd;
	if(get_unaligned(tail)!=GT_PKT_TAIL)//判断是否是包尾
	{
		printf("recv get %x,didn't tail\n",(int)get_unaligned(tail));
		
 		ptest=(char *)(s);
        	for(i=0;i<(int)(sizeof(struct gt_pkt_head)+pkt_len+4+4);i++)
            	printf("%02x,",ptest[i]);

        	printf("\n");

		goto getheader;
	}
	pd=(char*)s->msg;
	crcval = tab_crc32(pd,pkt_len);//已经去掉了crc的长度
	if(crcval!=get_unaligned(reccrc))
	{//crc校验错误
		
		printf("packet receive crc error rec=%x self=%x!\n",(int)*reccrc,(int)crcval);
		judge_exit(-3);
	}	
	printf("head->msg_type is %d\n",head->msg_type);
	if(head->msg_type==GT_CMD_NO_AUTH)
	{
		//如果不使用数字信封则不需要特殊处理
		if(s!=recv)
		{
			memcpy(recv,s,(sizeof(struct gt_pkt_head)+pkt_len+4+4));
		}
		
	}
	
#ifdef USE_SSL
	else if(head->msg_type==MSG_AUTH_SSL)
	{
		memcpy((void*)recv,(void*)&s->head,sizeof(struct gt_pkt_head));
		pbuf=recv->msg;
		ebuf=s->msg;
		mtype=head->msg_type;
		etype=head->encrypt_type;	
		elen=head->pkt_len+2+4-4-sizeof(s->head);
		plen=buf_len;
		//gtlogerr("env_unpack etype is %d,elen is %d\n",etype,elen);
		ret=env_unpack(etype,ebuf,elen,pbuf,&plen);
		//lc to do 目前会偶发unpack出错，原因难查

		
		if(ret!=0)
		{
			printerr("gt_com_api gt_cmd_pkt_recv_env  env_unpack ret=%d etype=%d elen=%d\n",ret,etype,elen);
                     gtlogerr("gt_cmd_pkt_recv_env  解密错误 !env_unpack ret=%d etype=%d elen=%d\n",ret,etype,elen);
			judge_exit(-5);
		}
		
		//printf("recv envpkt len=%d\n",plen);
		
	}
#endif
	else
	{
		 printerr("gt_com_api lib can't support msg_type=0x%x now!\n",head->msg_type);
		 judge_exit(-4);
	}
	
	if(msg_type!=NULL)
		*msg_type=head->msg_type;
	if(encrypt_type!=NULL)
		*encrypt_type=head->encrypt_type;
	//printf("recv lib receive a cmd pkt\n");	
	errflag=0;
	
exit:
	if(mallocflag)
	{
		if(s!=NULL)
			free(s);
	}
	return errflag;	
	
}



