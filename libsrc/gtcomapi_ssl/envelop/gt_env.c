/*************************************************************
File:	gt_env.c
Author: Zhangtao 
Date: 	2005-2-23
Description:数字信封工具,提供数字信封打包和解包功能
*************************************************************/
#include <openssl/e_os2.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include "env_os.h"
#include "gt_env.h"
#include "loadkeys.h"
#ifdef WIN32
#include "LinkList.h"
#include "cert_cache.h"
#endif 

EVP_PKEY * g_pub_key = NULL;//< 缓存的公钥
EVP_PKEY * g_prv_key = NULL;//< 缓存的私钥

int			g_pub_key_size = 0;	//< 公钥长度

#ifdef WIN32
	LinkList g_pub_key_list = NULL;
#endif
#ifndef WIN32
	#define _alloca alloca 
#endif

#define IV_SIZE 8	        //< 初始向量的长度
#define MAX_DATA_LEN 32000	//< 本工具最大可处理的数据长度
#define LENGTH_BYTES 4      //< 数据包中,表示长度的字节数
#define ENV_BUFFER_MIN(plen) (plen + g_pub_key_size + 2 * IV_SIZE + 12)//< 计算输入数据需要的最小Buffer长度

/// 通过标志得到信封算法结构
const EVP_CIPHER * get_cipher(int type)
{
	switch(type)
	{
	case 0:
		return EVP_enc_null();
	case 1:
		return EVP_des_ede_cbc();
	case 2:
		return EVP_des_ede3_cbc();
	case 3:
		return EVP_idea_cbc();
	case 4:
		return EVP_rc2_cbc();
	case 5:
		return EVP_bf_cbc();
	case 6:
		return EVP_cast5_cbc();
	case 7:
		return EVP_rc5_32_12_16_cbc();
	default:
		return EVP_enc_null();
	}
}

/// 计算数字信封,得到加密密钥和初始向量
int do_seal(const EVP_CIPHER * cipher,
			unsigned char * pbuf,int plen,
			unsigned char * ebuf,int * elen,
			unsigned char * iv,
			unsigned char *kbuf,int* klen)
{
	EVP_CIPHER_CTX ectx; 
	EVP_PKEY *pub_key[1];
	unsigned char *ekey[1]; 
	int tmp_len = 0;
	int ret = 0;
	
	RAND_seed("gt alarm system",10);
	pub_key[0] = NULL;
	ekey[0] = kbuf;

	if(!g_pub_key)
		return ERR_ENV_CERT_INVALID;

	EVP_CIPHER_CTX_init(&ectx);//TaoNote: 如果不调用EVP_CIPHER_CTX_init,不影响运算,但有内存泄漏

	do{
		pub_key[0] = g_pub_key;
		if(1 != EVP_SealInit(&ectx,cipher,ekey,klen,iv,pub_key,1)) 
		{ 
			ret = ERR_ENV_ENCRYPT_FAILED;
			break;
		}

		*elen = 0;
		if(1 != EVP_SealUpdate(&ectx, ebuf+*elen,elen,pbuf,plen))
		{
			ret = ERR_ENV_ENCRYPT_FAILED;
			break;  
		}

		if(1 != EVP_SealFinal(&ectx, ebuf + *elen,&tmp_len)) 
		{
			ret = ERR_ENV_ENCRYPT_FAILED;
			break;          
		}

		*elen = *elen + tmp_len; 		
	}while(0);

    EVP_CIPHER_CTX_cleanup(&ectx);

	return ret;
}

/// 使用加密密钥和初始向量解开数字信封,得到明文
int do_unseal(const EVP_CIPHER * cipher,
			  unsigned char * ebuf,int ebuflen,
			  unsigned char * iv,
			  unsigned char * kbuf,int klen,
			  unsigned char * dbuf,int * dlen)
{
    EVP_CIPHER_CTX ectx;
	unsigned char *ekey[1]; 
	int tmp_len;
	int ret = 0;
	int fd;

	ekey[0] = NULL;

	if (!g_prv_key)
		return ERR_ENV_KEY_INVALID;

	EVP_CIPHER_CTX_init(&ectx);

	do{
		ekey[0] = _alloca(klen);
		if(ekey[0] == NULL)
		{
			ret = ERR_ENV_MEMORY;
			break;
		}
		
		memcpy(ekey[0],kbuf,klen);

		if(1 != EVP_OpenInit(&ectx,cipher,ekey[0],klen,iv,g_prv_key)) 
		{
		/*
			if(ebuflen == 24)
			{
				fd = open("/hqdata/24err.ek",O_RDWR|O_CREAT|O_TRUNC);
				write(fd,kbuf,klen);
				close(fd);
			}
			if(ebuflen == 32)
			{
				fd = open("/hqdata/32err.ek",O_RDWR|O_CREAT|O_TRUNC);
				write(fd,kbuf,klen);
				close(fd);
			}
		*/	
			printf("EVP_OpenInit err!\n");
			ret = ERR_ENV_DECRYPT_FAILED;
			break; 	
		}
/*
			if(ebuflen == 24)
			{
				fd = open("/hqdata/24noerr.ek",O_RDWR|O_CREAT|O_TRUNC);
				write(fd,kbuf,klen);
				close(fd);
			}
			if(ebuflen == 32)
			{
				fd = open("/hqdata/32noerr.ek",O_RDWR|O_CREAT|O_TRUNC);
				write(fd,kbuf,klen);
				close(fd);
			}
*/		
		*dlen = 0; 
		if(1 != EVP_OpenUpdate(&ectx, dbuf + *dlen, &tmp_len, ebuf, ebuflen))
		{
			printf("EVP_OpenUpdate err!\n");
			ret = ERR_ENV_DECRYPT_FAILED;
			break;
		}

		*dlen = *dlen + tmp_len;    
		if(1 != EVP_OpenFinal(&ectx, dbuf+*dlen,&tmp_len)) 
		{
			printf("EVP_OpenFinal err!\n");
			ret = ERR_ENV_DECRYPT_FAILED;
			break;
		}

		*dlen = *dlen + tmp_len;
		dbuf[*dlen] = 0x0;  		
	}while(0);

	EVP_CIPHER_CTX_cleanup(&ectx);
	return ret;
}

ENVELOP_API int env_pack(
			 IN	int etype,
			 IN char * pbuf,IN int plen,
			 IN char * ebuf,IN OUT int * elen)
{
 	unsigned char *kbuf = NULL;   //密钥
	unsigned char *enc_buf = NULL;//密文
	unsigned char iv[IV_SIZE];    //初始化向量

	int klen = 0;
	int enc_len = 0;
	unsigned long tmp_len = 0;
	int rlen = 0;
	int ret;
	
	if(plen > MAX_DATA_LEN)
		return ERR_ENV_DATA_EXCEED;

	if(*elen < ENV_BUFFER_MIN(plen))
	{
		*elen =  ENV_BUFFER_MIN(plen);
		return ERR_ENV_BUF_INVALID;
	}

	do
	{
		kbuf = malloc(g_pub_key_size);	 
		enc_buf = malloc(plen + 8);
		if(kbuf == NULL) 
			return ERR_ENV_MEMORY;
		if(enc_buf == NULL)
			return ERR_ENV_MEMORY;

		ret = do_seal(get_cipher(etype),pbuf,plen,enc_buf,&enc_len,iv,kbuf,&klen);

		if(ret != 0)
			break;

		//组装信封
		memset(ebuf,0x0,*elen);

		tmp_len = sizeof(iv); 
		memcpy(ebuf,&tmp_len,LENGTH_BYTES);
		rlen = rlen + LENGTH_BYTES;
		memcpy(ebuf + rlen,iv,tmp_len);
		rlen = rlen + tmp_len;

		tmp_len = klen;
		memcpy(ebuf + rlen,&tmp_len,LENGTH_BYTES);
		rlen = rlen + LENGTH_BYTES;
		memcpy(ebuf + rlen,kbuf,tmp_len);
		rlen = rlen + tmp_len;

		tmp_len = enc_len;
		memcpy(ebuf + rlen,&tmp_len,LENGTH_BYTES);
		rlen = rlen + LENGTH_BYTES;
		memcpy(ebuf + rlen,enc_buf,tmp_len);
		rlen = rlen + tmp_len;

		*elen = rlen;
	}while(0);

	if(kbuf != NULL)
		free(kbuf);
	if(enc_buf != NULL)
		free(enc_buf);

	return ret;
}

ENVELOP_API int env_unpack(IN int etype,
			   IN char * ebuf,IN int elen,
			   IN char * pbuf,IN OUT int * plen)
{
 	unsigned char *kbuf = NULL;   //密钥
	unsigned char *enc_buf = NULL;//密文
	unsigned char iv[IV_SIZE];	  //初始化向量
	int klen = 0;
	int enc_len = 0;
	unsigned long tmp_len = 0;
	int rlen = 0;
	int ret;

	if(elen > MAX_DATA_LEN )
		return ERR_ENV_DATA_EXCEED;
	if(*plen < elen - g_pub_key_size || elen < 1)
	{
		*plen = elen - g_pub_key_size;
		return ERR_ENV_BUF_INVALID;
	}

	//打开信封,得到初始化向量,加密Key和密文
	memcpy(&tmp_len,ebuf,LENGTH_BYTES);
	if(tmp_len != IV_SIZE)
		return ERR_ENV_ENV_INVALID; 
	rlen = rlen + LENGTH_BYTES;
	memcpy(iv,ebuf + rlen,tmp_len);
	rlen = rlen + tmp_len;  

	memcpy(&tmp_len,ebuf+rlen,LENGTH_BYTES);
	if(tmp_len > (unsigned int)(elen - rlen))
		return ERR_ENV_ENV_INVALID;
	rlen = rlen + LENGTH_BYTES;
	kbuf = ebuf +rlen;
	klen = tmp_len;
	rlen = rlen + tmp_len; 

	memcpy(&tmp_len,ebuf+rlen,LENGTH_BYTES);
	if(tmp_len > (unsigned int)(elen - rlen))
		return ERR_ENV_ENV_INVALID;
	rlen = rlen + LENGTH_BYTES;
	enc_buf = ebuf +rlen;
	enc_len = tmp_len;
	rlen = rlen + tmp_len; 

	if(rlen != elen)
		return ERR_ENV_ENV_INVALID;

	printf("do_unseal enc_len is %d,klen is %d\n",enc_len,klen);
	ret = do_unseal(get_cipher(etype),
		enc_buf,enc_len,
		iv,
		kbuf,klen,
		pbuf,plen);

	return ret;
}

ENVELOP_API int env_check(IN const char * cert,IN const char * key)
{
	EVP_PKEY * pub_key = NULL;
	EVP_PKEY * prv_key = NULL;

	pub_key = read_pub_key(cert); 
	if(!pub_key)
		return ERR_ENV_CERT_INVALID;
	EVP_PKEY_free(pub_key);

	prv_key = read_private_key(key);
	if(!prv_key)
		return ERR_ENV_KEY_INVALID;

	EVP_PKEY_free(prv_key);
	return 0;
}

ENVELOP_API int env_init(IN const char * cert,IN const char * key)
{
	//初始化Openssl
	do { 
		CRYPTO_malloc_init(); 
		OpenSSL_add_all_algorithms();
		OpenSSL_add_all_ciphers();
		OpenSSL_add_all_digests();
		ERR_load_PEM_strings();//TaoNote ,如果不调这个函数,则在读读取PEM时会异常
		ERR_load_crypto_strings(); 
	} while(0);	 

	//检查证书文件是否可用
	if(cert != NULL)
	{
		if(g_pub_key != NULL)
			EVP_PKEY_free(g_pub_key);

		g_pub_key = read_pub_key(cert); 
		if(!g_pub_key)
			return ERR_ENV_CERT_INVALID;
		
		g_pub_key_size = EVP_PKEY_size(g_pub_key);
	}

	//检查私钥文件是否可用
	if(g_prv_key != NULL)
		EVP_PKEY_free(g_prv_key);

	g_prv_key = read_private_key(key);
	if (!g_prv_key)
		return ERR_ENV_KEY_INVALID;

	return 0;
}

ENVELOP_API void env_release()
{
#ifdef WIN32
	if(0 == env_release_cert_list() && g_pub_key != NULL)
		EVP_PKEY_free(g_pub_key);
#else
	if(g_pub_key != NULL)
		EVP_PKEY_free(g_pub_key);
#endif

	if(g_prv_key != NULL)
		EVP_PKEY_free(g_prv_key);

	//释放Openssl
	do { 
		EVP_cleanup(); 
		CRYPTO_cleanup_all_ex_data(); 
		ERR_remove_state(0); 
		ERR_free_strings(); 
	} while(0);	
}

#ifdef WIN32
///仅使用私钥初始化信封函数库
ENVELOP_API int env_init_only_key(IN const char * key)
{
	g_pub_key_list = (LinkList)malloc(sizeof(LNode));	
	memset(g_pub_key_list,0,sizeof(LNode));

	return env_init(NULL,key);
}

///设置对方的证书文件
ENVELOP_API int env_set_peer_cert(
							  IN int id,   ///< 由对方的ID
							  IN const char * name, 
							  IN const char * cert///< 对方的证书文件名
							  )
{
	int ret = 0;
	char path[128];
	EVP_PKEY *  pub_key ;
	LinkList node = NULL;

	cc_get_path(name,path);
	
	ret = cc_save(path,cert);
	if(ret != 0)
		return ret;

	pub_key = read_pub_key(path);
	if(!pub_key)
		return ERR_ENV_CERT_INVALID;
	
	node = (LinkList)malloc(sizeof(LNode));
	node->id = id;
	node->data = pub_key;

	ListAppend_L(g_pub_key_list,node);
	return 0;
}

///指定对方ID,进行信封打包
ENVELOP_API int env_pack_by_peer_id(
									IN int id,   ///< 对方的ID
									IN int etype,       ///< 加密算法
									IN char * pbuf,     ///< 明文
									IN int plen,        ///< 明文长度
									IN char * ebuf,     ///< 用于输出密文(信封)的缓存f,
									IN OUT int * elen   ///< 密文(信封)长度,输入为ebuf的大小,输出为实际的密文大小
									)
{
	LinkList node = NULL;
	node = ListFind_L(g_pub_key_list,id);
	if(node == NULL)
		return ERR_ENV_NO_CERT;

	g_pub_key = (EVP_PKEY *) node->data;
	g_pub_key_size = EVP_PKEY_size(g_pub_key);
	return env_pack(etype,pbuf,plen,ebuf,elen);
}			

int env_release_cert_list()
{
	LinkList p;
	EVP_PKEY * pkey;
	int count = 0;

	if(g_pub_key_list == NULL)
		return count;

	p = g_pub_key_list->next;	

	while(p)
	{
		if(p->data != NULL)
		{
			pkey = (EVP_PKEY *)p->data;
			EVP_PKEY_free(pkey);
			count ++;
		}
		p = p->next;
	}	

	ReleaseList_L(g_pub_key_list);
	g_pub_key_list = NULL;
	return count;
}
#endif	 			
