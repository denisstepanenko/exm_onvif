/**  @file	gt_netsdk_errno.h
 *   @brief gt_netsdk中的错误码定义
 *   @date 	2007.06
 */
#ifndef GT_NETSDK_ERRNO_H
#define	GT_NETSDK_ERRNO_H

//由嵌入式设备发出的错误码
#define RESULT_SUCCESS			0	//成功
#define ERR_DVC_INTERNAL		0x1001	//设备内部错
#define ERR_DVC_INVALID_REQ		0x1002	//客户请求数据格式错
#define ERR_DVC_BUSY			0X1003	//设备忙
#define ERR_DVC_FAILURE			0x1004  //设备故障
#define ERR_EVC_CRC_ERR			0x1005	//设备收到一个crc错误的数据包
#define ERR_EVC_NOT_SUPPORT		0x1006  //设备收到一个不支持的命令
#define ERR_ENC_NOT_ALLOW		0x1007  //设备收到一个不允许的命令
#define ERR_DVC_NO_RECORD_INDEX	0x1008	//设备没有查询到索引


//升级相关的错误码 
#define ERR_DVC_INVALID_NAME	0x1010  //升级文件名字格式错误
#define ERR_DVC_LOGIN_FTP    	0x1011  //无法登录ftp服务器
#define ERR_DVC_NO_FILE      	0x1012  //ftp服务器上无指定的文件或用户对其无读权限
#define ERR_DVC_UNTAR        	0x1013  //解压文件失败
#define ERR_NO_SPACE         	0x1014  //设备存储空间不够，无法升级
#define ERR_DVC_PKT_NO_MATCH	0x1015	//升级包与设备型号不匹配
#define ERR_DVC_UPDATE_FILE		0x1016	//更新设备文件(证书，配置文件)错误
#define ERR_DVC_WRONG_SIZE		0x1017	//文件大小不符




//不是设备发出的错误码错误码
#define	ERR_SDK_ERRNO_BASE		0x2000	///错误码基数
#define	ERR_SDK_EPERM			(ERR_SDK_ERRNO_BASE + 1)	///<Operation not permitted(不允许的操作)
#define	ERR_SDK_ENOENT			(ERR_SDK_ERRNO_BASE + 2)	///<No such file or directory(没有指定的文件或目录)
#define ERR_SDK_EBUSY			(ERR_SDK_ERRNO_BASE + 3)	///<Device or resource busy(设备或资源忙)
#define ERR_SDK_EINVAL			(ERR_SDK_ERRNO_BASE + 4)	///<Invalid argument(参数错误)
#define ERR_SDK_EMFILE			(ERR_SDK_ERRNO_BASE + 5)	///<Too many open files(打开的文件过多)
#define ERR_SDK_EAGAIN			(ERR_SDK_ERRNO_BASE + 6)	///<Try again(稍后重试)
#define ERR_SDK_ENOMEM			(ERR_SDK_ERRNO_BASE + 7)	///<Out of memory(内存不足)
#define ERR_SDK_EFBIG           (ERR_SDK_ERRNO_BASE + 8)    ///<File too large (文件过大)
#define ERR_SDK_UNKNOW			(ERR_SDK_ERRNO_BASE + 9)	///<unknow error(未知错误)
#define ERR_SDK_ECFILE			(ERR_SDK_ERRNO_BASE + 10)	//创建文件失败
#define ERR_SDK_UNKNOW_CERT		(ERR_SDK_ERRNO_BASE	+ 11)	//不支持的证书格式
#define ERR_SDK_OP_TIMEOUT		(ERR_SDK_ERRNO_BASE	+ 12)	//操作超时
#define ERR_SDK_NOT_SUPPORT		(ERR_SDK_ERRNO_BASE	+ 13)	//SDK不支持的操作
///网络相关错误
#define ERR_SDK_ENETDOWN        	(ERR_SDK_ERRNO_BASE + 100)        ///<Network is down  
#define ERR_SDK_ENETUNREACH     	(ERR_SDK_ERRNO_BASE + 101)        ///<Network is unreachable 网络不可达
#define ERR_SDK_ENETRESET       	(ERR_SDK_ERRNO_BASE + 102)       ///<Network dropped connection because of reset	连接断开
#define ERR_SDK_ECONNABORTED    	(ERR_SDK_ERRNO_BASE + 103)       ///<Software caused connection abort	放弃连接
#define ERR_SDK_ECONNRESET      	(ERR_SDK_ERRNO_BASE + 104)       ///<Connection reset by peer 连接复位
#define ERR_SDK_ENOBUFS         	(ERR_SDK_ERRNO_BASE + 105)       ///<No buffer space available	缓冲区不足
#define ERR_SDK_EISCONN         	(ERR_SDK_ERRNO_BASE + 106)       ///<Transport endpoint is already connected 连接已建立
#define ERR_SDK_ENOTCONN        	(ERR_SDK_ERRNO_BASE + 107)       ///<Transport endpoint is not connected 连接未建立
#define ERR_SDK_ESHUTDOWN       	(ERR_SDK_ERRNO_BASE + 108)       ///<Cannot send after transport endpoint shutdown 对方连接断开,不能发送
#define ERR_SDK_ETOOMANYREFS    	(ERR_SDK_ERRNO_BASE + 109)       ///<Too many references: cannot splice 
#define ERR_SDK_ETIMEDOUT       	(ERR_SDK_ERRNO_BASE + 110)       ///<Connection timed out	连接超时
#define ERR_SDK_ECONNREFUSED    	(ERR_SDK_ERRNO_BASE + 111)       ///<Connection refused		连接被拒绝

///USBKEY相关
#define ERR_SDK_NO_KEYDRIVER		(ERR_SDK_ERRNO_BASE + 200)		///<没有安装USBKEY的驱动
#define ERR_SDK_INIT_KEY			(ERR_SDK_ERRNO_BASE + 201)		///<初始化KEY失败
#define ERR_SDK_NO_KEY				(ERR_SDK_ERRNO_BASE + 202)		///<没有插入key
#define ERR_SDK_NO_VALID_CERT		(ERR_SDK_ERRNO_BASE + 203)		///<没有可用的证书
#define ERR_SDK_CERT_VALIDITY		(ERR_SDK_ERRNO_BASE + 204)		///<证书过期

//流转发服务器发来的错误码
#define	ERR_BE_TRA_INTERNAL				0x7000		//内部错
#define ERR_BE_TRA_INVALID_REQ			0X7001		//客户请求数据格式不正确
#define ERR_BE_TRA_UNREACHABLE_DEVICE_D	0X7002		//前端设备联系不上(直接请求)
#define ERR_BE_TRA_INVALID_TOKEN		0X7003		//用户权限验证失败
#define ERR_BE_TRA_PROTOCAL				0X7004		//不支持的传输协议
#define ERR_BE_TRA_INVALID_REQ_TYPE		0x7005		///不支持的请求命令类型
#define ERR_BE_TRA_USER_EXISTS			0x7006		//同名用户已存在
#define ERR_BE_TRA_USER_NOT_EXISTS		0x7007		//没有该用户的连接存在
#define ERR_BE_TRA_DEVICE_TEMP_INVALID	0x7008		//设备暂时不可用
#define ERR_BE_TRA_DEVICE_GUID_INVALID	0x7009		//GUID格式错误
#define ERR_BE_TRA_UNREACHABLE_DEVICE_N	0X700a		//前端设备联系不上(通过网关通知)



#endif
