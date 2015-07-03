/*
 *  GT1000服务器和远程计算机通讯中用到的错误码定义
 *
 */

#ifndef GT_ERRLIST_H
#define GT_ERRLIST_H

#ifdef _WIN32			//add by scott
#include <windows.h>
//#define __inline__
#endif //_WIN32

#include <errno.h>

//由嵌入式设备发出的错误码
#define RESULT_SUCCESS			0	//成功
#define ERR_DVC_INTERNAL		0x1001	//设备内部错
#define ERR_DVC_INVALID_REQ	0x1002	//客户请求数据格式错
#define ERR_DVC_BUSY			0X1003	//设备忙
#define ERR_DVC_FAILURE		0x1004  //设备故障
#define ERR_EVC_CRC_ERR		0x1005	//设备收到一个crc错误的数据包
#define ERR_EVC_NOT_SUPPORT	0x1006  //设备收到一个不支持的命令
#define ERR_ENC_NOT_ALLOW		0x1007  //设备收到一个不允许的命令
#define ERR_DVC_NO_RECORD_INDEX			0x1008	//设备没有查询到索引
#define ERR_DVC_NO_DISK		0x1009	//设备没有硬盘 
#define	ERR_DVC_NO_AUDIO	ERR_EVC_NOT_SUPPORT
#define	ERR_DVC_NO_TRIG		ERR_EVC_NOT_SUPPORT

//update errortype definition 
#define ERR_DVC_INVALID_NAME	0x1010  //升级文件名字格式错误
#define ERR_DVC_LOGIN_FTP    	0x1011  //无法登录ftp服务器
#define ERR_DVC_NO_FILE      		0x1012  //ftp服务器上无指定的文件或用户对其无读权限
#define ERR_DVC_UNTAR        		0x1013  //解压文件失败
#define ERR_NO_SPACE         		0x1014  //设备存储空间不够，无法升级
#define ERR_DVC_PKT_NO_MATCH	0x1015	//升级包与设备型号不匹配
#define ERR_DVC_UPDATE_FILE	0x1016	//更新设备文件(证书，配置文件)错误
#define ERR_DVC_WRONG_SIZE	0x1017		//文件大小不符

//rt errortype definition
#define ERR_RT_NO_CONN      0x1201  //无法连接服务器
#define ERR_RT_SEND_ERR     0x1202 //发送数据头失败
#define ERR_RT_ENC_NOT_OK          0x1203

//和xvs相关的错误码
#define ERR_XVS_NOT_ACCESSABLE	0x2001	//外围设备不可见或无法连接

/**********************************************************************************************
 * 函数名	:get_gt_errname()
 * 功能	:将与远程计算机通讯的错误码转换为字符串的值
 * 输入	:err:错误码
 * 返回值	:错误码的字符串描述
 **********************************************************************************************/
#ifdef _WIN32
static  char* get_gt_errname(WORD err)
#else
static __inline__ char* get_gt_errname(WORD err)
#endif
{
	switch(err)
	{
		case RESULT_SUCCESS:				//成功
			return "RESULT_SUCCESS";
		break;
		case ERR_DVC_INTERNAL:				//设备故障??
			return "ERR_DVC_INTERNAL";
		break;
		case ERR_DVC_INVALID_REQ:			//客户请求数据格式错
			return "ERR_DVC_INVALID_REQ";
		break;
		case ERR_DVC_BUSY:
			return "ERR_DVC_BUSY";
		break;
		case ERR_DVC_FAILURE:
			return "ERR_DVC_FAILURE";
		break;
		case ERR_EVC_CRC_ERR:
			return "ERR_EVC_CRC_ERR";
		break;
		case ERR_EVC_NOT_SUPPORT:
			return "ERR_EVC_NOT_SUPPORT";
		break;
		case ERR_ENC_NOT_ALLOW:
			return "ERR_ENC_NOT_ALLOW";
		break;
		case ERR_DVC_NO_RECORD_INDEX:
			return "ERR_DVC_NO_RECORD_INDEX";
		break;





		case ERR_DVC_INVALID_NAME:
			return "ERR_DVC_INVALID_NAME";
		break;
		case ERR_DVC_LOGIN_FTP:
			return "ERR_DVC_LOGIN_FTP";
		break;
		case ERR_DVC_NO_FILE:
			return "ERR_DVC_NO_FILE";
		break;
		case ERR_DVC_UNTAR:
			return "ERR_DVC_UNTAR";
		break;
		case ERR_NO_SPACE:
			return "ERR_NO_SPACE";
		break;
		case ERR_DVC_PKT_NO_MATCH:
			return "ERR_DVC_PKT_NO_MATCH";
		break;
		case ERR_DVC_WRONG_SIZE:
			return "ERR_DVC_WRONG_SIZE";
		break;
		case ERR_XVS_NOT_ACCESSABLE:
			return "ERR_XVS_NOT_ACCESSABLE";
		break;	
	}

	return "UNKNOW_ERR";
}



#endif



