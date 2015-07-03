#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "mshmpool.h"

#include <time.h>

int main(void)
{
	int ret,i=0;
	MSHM_POOL pool;
	int buf[10240];
	unsigned long *sendp;
	unsigned long  sendcnt=0,senddata;	
	int sendlen=0;
	int randnum;


	ret=MShmPoolCreate("test pool",&pool,1026,1024*1024*5);
	printf("call MShmPoolCreate ret=%d\n",ret);
	srand(time(NULL));
	while(1)
	{
		//for(i=0;i<1000;i++)
		{
			memset((void*)buf,i,sizeof(buf));

			//sendlen=sizeof(buf);
			randnum=rand();
			//randnum=sizeof(buf)-1;
			sendlen=(randnum%sizeof(buf))&0xfffffffc;
			//printf("sendlen=%d\n",sendlen);
			sendp=(unsigned long *)buf;
			senddata=sendcnt;
			for(i=0;(i<sendlen/4);i++)
			{
				*sendp=senddata;
				senddata++;
				sendp++;
			}
				

			ret=MShmPoolAddResource(&pool,buf,sendlen,sendcnt);
			sendcnt+=sendlen/4;
			if(ret<0)
				printf("MShmPoolAddResource %d ret=%d\n",i,ret);
			
			usleep(10);
		}
		//sleep(1);
		i++;
	}
	exit(0);
}
