#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "../mshmpool.h"

static __inline__ int NewerEle(MSHM_POOL *pool,int cur)
{
        if(++cur>=SHPOOL_MAX_ELE)
                cur=0;
        return cur;
}


void printele(MSHM_POOL *pool,SHPOOL_ELE		*ele)
{//没有处理缓冲区拐弯
	int i;
	unsigned long *p;
	SHPOOL_HEAD	*ph        = pool->ph;
	int 				startblock=ele->bblock;
	int				tailblocks=ph->tblocks-startblock;	//尾部剩余的空闲块
	int				tailbytes=tailblocks*SHPOOL_BLOCK_SIZE;
	printf("ele seq=%d usrflag=%x datalen=%d startblock=%d blocks=%d\n",(int)ele->num,(int)ele->userflag,(int)ele->datalen,(int)ele->bblock,(int)ele->blocks);
	
	if(ele->datalen<=tailbytes)
	{
		p=(unsigned long*)&pool->eledata[ele->bblock];
		for(i=0;i<(ele->datalen/4);i++)
		{
			if(i%4==0)
				printf("\n");
			printf("%08x ",(int)*p);
			p++;
		}
	}
	else
	{
		p=(unsigned long*)&pool->eledata[ele->bblock];
		for(i=0;i<(tailbytes/4);i++)
		{
			if(i%4==0)
				printf("\n");
			printf("%08x ",(int)*p);
			p++;
		}	
		p=(unsigned long*)&pool->eledata[0];
		for(i=0;i<((ele->datalen-tailbytes)/4);i++)
		{
			if(i%4==0)
				printf("\n");
			printf("%08x ",(int)*p);
			p++;
		}			
	}
	printf("\n");
}
int main(void)
{
	int ret;
	int i;
	int curele=0;
	MSHM_POOL mpool;
	MSHM_POOL *pool=&mpool;
	SHPOOL_HEAD	*ph;
	SHPOOL_USR   	*usr;
	SHPOOL_ELE		*ele;

	printf("this program will print all eles of pool\n");
	ret=MSHmPoolAttach(&mpool,0x30000);
	if(ret<0)
	{
		printf("can't attach pool!!\n");
		exit(1);
	}
	ph=pool->ph;
	usr=ph->users;
	printf("total %d eles \n",ph->count);
	MShmLock(pool->mc);	
	if(ph->count==0)
		exit(0);
	else
	{
		curele=ph->head;
		for(i=0;i<ph->count;i++)
		{
			ele=&ph->eles[curele];
			printele(pool,ele);			
			curele=NewerEle(pool,curele);
		}
	}
	MShmUnLock(pool->mc);
	printf("\n");
	exit(0);
}



