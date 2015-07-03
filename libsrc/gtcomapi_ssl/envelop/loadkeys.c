/*************************************************************
File:   loadkeys.c
Author: Zhangtao 
Date: 	2005-2-23
Description:加载证书和私钥
*************************************************************/
#include "env_os.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/pem.h>

/// 缺省的私钥密码
char * pass = "gt.123.dev";

EVP_PKEY * read_pub_key(const char *certfile)
{
	BIO *key = NULL;
	X509 *x509 = NULL;
	EVP_PKEY *pkey = NULL;

	do{
		key=BIO_new(BIO_s_file());
		if(key == NULL) 
			break;

		if (BIO_read_filename(key,certfile) <= 0)
			break;

		x509 = PEM_read_bio_X509_AUX(key, NULL,NULL,NULL);
		if (x509 == NULL) 
			break;

		pkey=X509_extract_key(x509);
	}while(0);

	if(key)
		BIO_free(key);
	if(x509)
		X509_free(x509);

	return pkey; 
}

EVP_PKEY *read_private_key(const char *keyfile)
{
    BIO *key=NULL;
	EVP_PKEY *pkey = NULL;

	do{
		key=BIO_new(BIO_s_file());
		if(key == NULL) 
			break;

		if (BIO_read_filename(key,keyfile) <= 0)
			break;

		pkey = PEM_read_bio_PrivateKey(key,NULL, NULL, pass);
		if(pkey == NULL)
			pkey = PEM_read_bio_PrivateKey(key,NULL, NULL, NULL);
	}while(0);

	if(key)
		BIO_free(key);

	return pkey;
}


