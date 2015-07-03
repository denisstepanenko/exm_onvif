/*************************************************************
File:	env_test.c
Author: Zhangtao 
Date: 	2005-2-23
Description:数字信封工具测试
*************************************************************/
#include <time.h>
#include "env_os.h"
#include "gt_env.h"
#include "loadkeys.h"

int  g_try_times = 1;
int  g_crypt_type = 1;

char * cert = "cert.pem";
char * key = "privkey.pem";
char * message = "Hello , world!  This is a test for digital envelop!";
double time_env,time_unenv;

void * pack_unpack(void * argv);

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf("usage:	 env_test.exe crypt_type try_times\n");
		printf("example: env_test.exe 1 100\n");
		return 0;
	}

	g_try_times = atoi(argv[2]);
	g_crypt_type = atoi(argv[1]);

	time_env = 0;
	time_unenv = 0;

	env_init(cert,key);

	printf(">>> to test %d times with crypt %d\n",g_try_times,g_crypt_type);
	pack_unpack(NULL);

	printf("\nTotal time ( %1.5f ) for %d times envelop \n",time_env,g_try_times);
	printf("time for once envelop: %1.5f secondes \n",time_env/g_try_times);
	printf("\nTotal time ( %1.5f ) for %d times unenvelop \n",time_unenv,g_try_times);
	printf("time for once unenvelop: %1.5f secondes \n",time_unenv/g_try_times);

	env_release();	
	return 0;		
}

void * pack_unpack(void * argv)
{
	int c = g_try_times;
	char ebuf[1024];
	char pbuf[512];
	int  elen = 0;
	int  plen = 0;
	int  ret = 0;
	time_t   start, finish;
	
	printf("intput data:\n %s\n",message);

	memcpy(pbuf,"hello      ",8);
	plen=8;
	elen=1024;
	ret = env_pack(g_crypt_type,pbuf,plen,ebuf,&elen);
	printf("pack g_crypt_type=%d ,plen=%d,elen=%d ,ret=%d\n",g_crypt_type,plen,elen,ret);
	plen=512;
	ret = env_unpack(g_crypt_type,ebuf,elen,pbuf,&plen);
	 printf("unpack g_crypt_type=%d ,plen=%d,elen=%d ,ret=%d\n",g_crypt_type,plen,elen,ret);
	


	do{
		memset(pbuf,0x0,sizeof(pbuf));
		memset(ebuf,0x0,sizeof(ebuf));

		//打包
		time( &start );


		while(c--)
		{			
			printf(".");
			fflush(stdout);
			plen = strlen(message);
			elen = sizeof(ebuf);
			memcpy(pbuf,message,plen);
			ret = env_pack(g_crypt_type,pbuf,plen,ebuf,&elen);
			if(ret != 0) break;
		}

		time( &finish);
		time_env += difftime(finish,start);
		printf("\ncount remain: %d\n",c);
		printf("envelop length: %d\n",elen);
 
		//解包
		time( &start );
		c = g_try_times;
		while(c--)
		{
			printf(".");
			fflush(stdout);
			memset(pbuf,0x0,sizeof(pbuf));
			plen = sizeof(pbuf);
			ret = env_unpack(g_crypt_type,ebuf,elen,pbuf,&plen);
			if(ret != 0) break;
		}

		time( &finish);
		time_unenv += difftime(finish,start);
		printf("\ncount remain: %d\n",c);
		printf("plain data : \n %s \ns",pbuf);

	}while(0);
	
	printf("ret : %d \n" , ret);
	return NULL;
}
