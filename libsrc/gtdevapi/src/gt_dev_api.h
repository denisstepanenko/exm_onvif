/*	GT1000系列视频服务器组播命令(UDP)协议底层库函数
 *		 2006.9 shixin
 *    本函数库提供了实现嵌入式系统与远程计算机使用组播方式通讯的底层接口
 *    上层的应用接口及数据结构不属于本库的实现范围
 *    本函数库提供的功能是
 *    1.将明文数据报按照GT系列视频服务器的组播通讯协议格式进行打包，加密，crc32校验，之后发送给指定的地址
 *    2.从指定的socket中接收数据包，进行crc校验，将密文变换成明文返回给调用进程，并将数据包的发送者信息返回调用者
 *    
 */
#ifndef GTDEV_API_H
#define	GTDEV_API_H

//#pragma once  //lsk 2007-4-10  win32下编译时用到的定义语句 只将库编译一次

#ifndef IN
	#define	IN
	#define	OUT
	#define	IO
#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifdef _WIN32
	//windows 使用
#pragma once  //lsk 2007-4-10
	#define EXPORT_DLL __declspec(dllexport)
#else
	//linux 使用
	#define EXPORT_DLL
#endif

//加密方式定义
#define GTDEV_ENC_NONE		0		//不需要加密

#define GUID_LEN                8  //GUID占用的字节数,接口中用到的id长度都是该值
/**********************************************************************************************
* 函数名	:send_dev_pkt()
* 功能	:将一段明文按照指定的加密方式形成命令数据包,发送到指定的socket地址
*
 输入	:	fd:目标socket文件描述符
* 		sin:描述目标地址信息的结构指针
* 		target_id:目标id号,长度为GUID_LEN
* 		source_id:发送者id号,长度为GUID_LEN
* 		buf:要发送出去的数据缓冲区
* 		buflen:缓冲区中的有效字节数
* 		enc_type:要使用的加密方式()
* 		flag:发送选项，保留，暂时写0
* 返回值:	
* 		正值表示发送出去的字节数，负值表示出错
**********************************************************************************************/
EXPORT_DLL int send_dev_pkt(IN int fd,IN struct sockaddr_in *sin,IN unsigned char *target_id,IN unsigned char *source_id,IN void *buf,int buflen,IN int enc_type,IN int flag);

/**********************************************************************************************
* 函数名	:recv_dev_pkt()
* 功能	:	从指定接口接收一包数据，如果没有数据则阻塞
* 输入	:	fd:要接收文件的socket文件描述符
*		selfid:自己的id号，长度为 GUID_LEN
*		buflen:接收信息的缓冲区长度
*		flag:接收标志，保留，暂时写0
* 输出	:	
* 		sin:描述发送者地址信息的结构指针
* 		source_id:发送者id号 长度为GUID_LEN
* 		msgbuf:接收到的信息
* 		enc_type:接收到的数据包的加密类型
* 返回值:	
* 		正值表示接收到msgbuf中的有效字节数，负值表示出错
*		-ENOMEM 表示缓冲区不足
**********************************************************************************************/
EXPORT_DLL int recv_dev_pkt(IN int fd,OUT struct sockaddr_in *sin,IN unsigned char *selfid,OUT unsigned char *sourceid,OUT unsigned char *msgbuf,IN int buflen,OUT int *enc_type,IN int flag);

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
EXPORT_DLL int dual_id_recv_dev_pkt(IN int fd,OUT struct sockaddr_in *sin,IN unsigned char *selfid, IN unsigned char *selfid1,OUT unsigned char *sourceid,OUT unsigned char *recv_id, OUT unsigned char *msgbuf,IN int buflen,OUT int *enc_type,IN int flag);

#ifdef __cplusplus
}
#endif

#endif

