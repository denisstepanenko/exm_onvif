/**
 * File:   gt_env.h
 * Author: Zhangtao
 * Date: 	2005-2-2
 * Description:	数字信封工具
 */
#ifndef _GT_ALARM_TAO_ENV_20050223_
#define _GT_ALARM_TAO_ENV_20050223_

#ifdef WIN32
#include <windows.h>
#include <string.h>
#ifdef ENVELOP_EXPORTS
#define ENVELOP_API __declspec(dllexport)
#else
#define ENVELOP_API __declspec(dllimport)
#endif
#else
#include <unistd.h>
#include <string.h>
#define ENVELOP_API
#endif

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#define IN 
#define OUT

#define ERR_ENV_BUF_INVALID  1   ///< 用于输出的缓存不够大
#define ERR_ENV_CERT_INVALID 2   ///< 证书无效
#define ERR_ENV_KEY_INVALID  3   ///< 密钥无效
#define ERR_ENV_MEMORY       4   ///< 申请内存失败
#define ERR_ENV_DATA_EXCEED  5   ///< 输入数据长度溢出
#define ERR_ENV_ENV_INVALID  6   ///< 无效的信封
#define ERR_ENV_ENCRYPT_FAILED 7 ///< 加密失败
#define ERR_ENV_DECRYPT_FAILED 8 ///< 解密失败
#define ERR_ENV_NO_CERT	    9    ///< 证书不存在

#define ERR_CREATE_DIR  21
#define ERR_OPEN_FILE  22
#define ERR_WRITE_FILE 23

#ifdef __cplusplus
extern "C" {
#endif

///检查证书可私钥是否可用	
ENVELOP_API int env_check(
						 IN const char * cert, ///< 证书文件名
						 IN const char * key   ///< 私钥文件名
						 ); 

///使用证书和私钥文件初始化信封函数库,在应用程序进入时调用
ENVELOP_API int env_init(
						 IN const char * cert, ///< 对方的证书文件名
						 IN const char * key   ///< 本方的私钥文件名
						 ); 

///信封打包
ENVELOP_API int env_pack(
						 IN	int etype,    ///< 加密算法
						 IN char * pbuf,  ///< 明文
						 IN int plen,     ///< 明文长度
						 IN char * ebuf,  ///< 用于输出密文(信封)的缓存
						 IN OUT int * elen///< 密文(信封)长度,输入为ebuf的大小,输出为实际的密文大小
						 );

///信封解包
ENVELOP_API int env_unpack(
						   IN int etype,   ///< 加密算法
						   IN char * ebuf, ///< 密文(信封)
						   IN int elen,    ///< 密文(信封)的长度
						   IN char * pbuf, ///< 用于输出明文的缓存
						   IN OUT int * plen///< 明文长度,输入为pbuf的大小,输出为实际的明文大小
						   ); 

///释放信封函数库,在应用程序退出时调用
ENVELOP_API void env_release();

#ifdef WIN32
///仅使用私钥初始化信封函数库
ENVELOP_API int env_init_only_key(
								  IN const char * key ///< 本方的私钥文件名
								  ); 

///设置对方的证书文件
ENVELOP_API int	env_set_peer_cert(
							  IN int id,   ///< 对方的ID
							  IN const char * name,///< 对方的证书文件名
							  IN const char * cert
							  );
///指定对方ID,进行信封打包
ENVELOP_API int env_pack_by_peer_id(
									IN int id,    ///< 对方的ID
									IN int etype,       ///< 加密算法
									IN char * pbuf,     ///< 明文
									IN int plen,        ///< 明文长度
									IN char * ebuf,     ///< 用于输出密文(信封)的缓存f,
									IN OUT int *  elen  ///< 密文(信封)长度,输入为ebuf的大小,输出为实际的密文大小
									);

int env_release_cert_list();
#endif
				 			
#ifdef __cplusplus
}
#endif
				 			
#endif//_GT_ALARM_TAO_ENV_20050223_

