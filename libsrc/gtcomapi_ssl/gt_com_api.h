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
#ifndef GT_COM_API_H
#define GT_COM_API_H
#include <typedefine.h>
#include <gt_errlist.h>

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


#define 	GT_COM_VERSION	2			//版本号
#define	printerr	printf


//MSG_TYPE define
#define	GT_CMD_NO_AUTH	0			//命令使用明文传送
#define   MSG_AUTH_SSL		0x60		//命令使用数字信封传送

#define 	SYS_MSG_TYPE		GT_CMD_NO_AUTH


//ENCRYPT_TYPE define
//数据加密类型选项
#define GT_CMD_NO_ENCRYPT	0		//无加密
#define DES_EDE_CBC			0x01	//des-ede-cbc des-ede-cbc
#define DES_EDE3_CBC		0x02	//des_ede3_cbc des3.des-ede3-cbc
#define IDEA_CBC			0x03	//idea_cbc idea.idea-cbc
#define RC2_CBC				0x04	//rc2_cbc rc2.rc2-cbc
#define BF_CBC				0x05	//bf_cbc bf.blowfish.bf-cbc
#define CAST5_CBC			0x06	//cast5_cbc cast.cast-cbc.cast5-cbc
#define RC5_32_12_16_CBC	0x07	//rc5_32_12_16_cbc rc5.rc5-cbc


#ifndef BYTE
#define BYTE	unsigned char
#define WORD	unsigned short
#define DWORD	unsigned long
#endif



/**************底层协议定义************************************/
struct gt_pkt_head{		//命令包的包头结构定义
	BYTE	sync_head[4];	//数据包同步头,
	WORD	pkt_len;			//去掉sync_head和pkt_len以及tail三个字段后，数据包的长度（字节）
	BYTE	version;			//通讯协议的版本号
	BYTE	msg_type;		//认证类型
	BYTE	encrypt_type;	//加密类型
	BYTE	reserve0;		//保留字段
	WORD	reserve1;		//保留字段
	DWORD	xor32;				//数据包头的xor32校验码(校验从pkt_len到encrypt_len)
};
struct gt_pkt_struct{		//数据包结构
	struct gt_pkt_head head;	//数据包的包头结构
	BYTE	msg[2];			//数据包内容,内容是变长的所以只能定义成这种形式,使用的时候需要把这个缓冲区强制转换成指向这个结构的指针;
	//由于数据包是变长的,所以通讯协议中的以下几个字段都包含在了msg中
	//DWORD	crc32;		//从命令报文第一个字节到数字签名的CRC32校验码
	//BYTE	tail[4];			//数据包结尾标志	
};
#define SIZEOF_GT_USR_CMD_STRUCT 10 
struct gt_usr_cmd_struct{	//PC与GT1000设备通讯的命令结构
	WORD	len;				//cmd以及后面的有效字段的长度
	WORD	cmd;			//命令
	BYTE	en_ack;			//是否需要响应标志
	BYTE	reserve0;		//保留
	WORD	reserve1;
	BYTE	para[2];			//参数
};
/*************************************************************************/


/***********************函数接口定义************************************/

 //用户进行非加密发送，接收数据调用的简化接口

#define gt_cmd_send_noencrypt(fd,buf,len,tbuf,flag)       gt_cmd_pkt_send(fd,(struct gt_pkt_struct *)buf,len,tbuf,flag,GT_CMD_NO_AUTH,GT_CMD_NO_ENCRYPT)
//接收数据包
#define gt_cmd_pkt_recv(fd,recv,buf_len,tempbuf,flag)	 gt_cmd_pkt_recv_env(fd,recv,buf_len,tempbuf,flag,NULL,NULL)
#define gt_cmd_recv 									 gt_cmd_pkt_recv	




 /**********************************************************************************************
 * 函数名	:gt_cmd_pkt_send()
 * 功能	:将一段明文按照指定的加密,签名方式形成命令数据包,发送到指定的socket
 * 输入	:fd:目标socket文件描述符
 *			 send:指向要发送的命令的缓冲区的指针
 *			 msg_len:要发送的缓冲区中的有效信息包的字节数(msg字段,不包括crc,tail等的长度)
 *			 tempbuf:(保留)，调用时传NULL就可以了
 *			 flag:(保留)，调用时传0就可以了
 *			 msg_type:发送数据包的认证类型,0表示不需要认证,MSG_AUTH_SSL表示使用数字信封，其他值待定
 *			 encrypt_type:发送数据包的加密类型,具体定义见"数据加密类型选项"
 * 返回值	:正值表示发送出去的字节数，负值表示出错
 * 注		: 调用者应当分配足够大的缓冲区以容纳要发送的数据,本函数将直接使用该缓冲区进行各种填充,变换等操作
 *	  		  调用着只需填充send->msg字段即可，其它字段由库函数自己填充
 *	 		 send以缓冲区的起始地址必须是4字节对齐的，否则上层应用处理可能会出问题
 **********************************************************************************************/
EXPORT_DLL int  gt_cmd_pkt_send(int fd,struct gt_pkt_struct *send,int msg_len,void *tempbuf,int flag,BYTE msg_type,BYTE encrypt_type);


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

EXPORT_DLL int  gt_cmd_pkt_recv_env(int fd,struct gt_pkt_struct *recv,int buf_len,void *tempbuf,int flag,int *msg_type,int *encrypt_type);

/**********************************************************************************************
 * 函数名	:env_init()
 * 功能	:初始化与服务器通讯用的数字信封，在系统初始化的时候调用一次
 * 输入	:cert:证书文件
 *			 key:私钥文件
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int env_init(const char * cert,const char * key);
///检查证书可私钥是否可用       ^M
int env_check(const char * cert, const char * key); 

#ifdef __cplusplus
}
#endif

#endif
