/*************************************************************
File:   loadkeys.h
Author: Zhangtao 
Date: 	2005-2-23
Description:加载证书和私钥
*************************************************************/

#ifndef _GT_ALARM_TAO_LOADKEYS_20050223_
#define _GT_ALARM_TAO_LOADKEYS_20050223_

#include <openssl/evp.h>

//从证书文件(PEM格式)中读取公钥
EVP_PKEY * read_pub_key(const char *certfile);

//从私钥文件(PEM格式)中读取公钥
EVP_PKEY * read_private_key(const char *keyfile);

#endif   //_GT_ALARM_TAO_LOADKEYS_20050223_

