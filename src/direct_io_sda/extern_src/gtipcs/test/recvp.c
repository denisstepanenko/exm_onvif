#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "mshmpool.h"

MSHM_POOL pool;
void exit_progs(void)
{
	int ret;
	printf("exit_progs\n");
	ret=MShmPoolReqFree(&pool);
	printf("call MShmPoolReqFree ret=%d\n",ret);
}

static void exit_log(int signo)
{
	switch(signo)
	{
		case SIGPIPE:
			return ;
		break;
		case SIGTERM:
			exit(0);
		break;
		case SIGKILL:
			exit(0);
		break;
		case SIGINT:
			exit(0);
		break;
		case SIGUSR1:
		break;
		case SIGSEGV:
			printf("segmentation fault\n");
			exit(0);
		break;
	}
	return;
}

int main(void)
{
	int ret,i;	
	int recbuf[102400];
	int eleseq,eleflag,oldflag=-1;
	int elelen;
	unsigned long startcnt=0;//³õÊ¼¼ÆÊýÆ÷
	unsigned long *pcnt;
	int readflag=0;
	signal(SIGKILL,exit_log);
	signal(SIGTERM,exit_log);
	signal(SIGINT,exit_log);
	signal(SIGSEGV,exit_log);
	signal(SIGPIPE,exit_log);
	
	ret=MShmPoolReq(&pool,0x30000,"test recv pool",1);
	printf("call MShmPoolReq ret=%d :%s userno=%d\n",ret,strerror(-ret),pool.userno);
	if(ret<0)
		exit(1);
	atexit(exit_progs);
	
	while(1)
	{
			memset((void*)recbuf,0,sizeof(recbuf));
			ret=MShmEleRemain(&pool);
			//printf("MShmEleRemain=%d\n",ret);
			//continue;
			ret=MShmPoolGetResource(&pool,recbuf,sizeof(recbuf),&eleseq,&eleflag);
			printf("\rMShmPoolGetResource ret=%8d flag=%08d seq=%8d\t\t",ret,eleflag,eleseq);
			fflush(stdout);
			if(ret<0)
				printf("err=%s\n",strerror(-ret));
			else
				elelen=ret;
			if(oldflag==eleflag)
			{
				printf("\noldflag=eleflag %x!!!!!!!!!!!\n",eleflag);
				//sleep(10);
			}
			oldflag=eleflag;
			if(!readflag)
			{
				printf("\nfirst start !!!!!!!!\n");
				startcnt=eleflag;
				readflag=1;
			}


			pcnt=(unsigned long *)recbuf;
			for(i=0;i<(elelen/4);i++)
			{
				if(*pcnt!=startcnt)
				{
					MShmLock(pool.mc);
					printf("recv maybe err:pcnt:%x startcnt:%x eleflag=%x\n",(int)*pcnt,(int)startcnt,(int)eleflag);
					for(i=0;i<(100/4);i++)
					{
						if(i%4==0)
							printf("\n");
						printf("%08x ",(int)pcnt[i]);
					}
					printf("\n");
					while(1)
					{
						sleep(1);
					}		
				}
				pcnt++;
				startcnt++;
			}			

	}
	exit(0);
}
