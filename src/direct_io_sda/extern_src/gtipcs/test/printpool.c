#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "mshmpool.h"

static __inline__ int NewerEle(MSHM_POOL *pool,int cur)
{
	if(++cur>=SHPOOL_MAX_ELE)
		cur=0;
	return cur;
}

static __inline__  int OlderEle(MSHM_POOL *pool,int cur)
{
	if(--cur<0)
		cur=SHPOOL_MAX_ELE-1;
	return cur;
}

static __inline__ int MShmDistanceEle(MSHM_POOL *pool,int a,int b)
{
	SHPOOL_HEAD	*ph=pool->ph;
	int head=ph->head;
	//int tail=ph->tail;
	int distb=0;
	int dista=0;
	if(a==b)
		return 0;
	if(b>=head)
		distb=b-head;
	else
		distb=SHPOOL_MAX_ELE-b+head;

	if(a>=head)
		dista=a-head;
	else
		dista=SHPOOL_MAX_ELE-a+head;
	return (distb-dista);
}

static int poolkey=0x30000;
int main(int argc,char *argv[])
{
	int ret;
	int i;
	int usedblock=0;
	int curele=0;
	time_t curtime;
	MSHM_POOL mpool;
	MSHM_POOL *pool=&mpool;
	SHPOOL_HEAD	*ph;
	SHPOOL_USR   	*usr;
	SHPOOL_ELE		*ele;

//	printf("argv0=%s\n",argv[0]);
	if(argc>=2)
		poolkey=atoi(argv[1]);
	printf("poolkey=%d!\n",poolkey);
	ret=MSHmPoolAttach(&mpool,poolkey);
	if(ret<0)
	{
		printf("can't attach pool!!\n");
		exit(1);
	}
	ph=pool->ph;
	usr=ph->users;
	printf("total %d users are using the pool\n",MShmPoolGetTotalUser(pool));
	MShmLock(pool->mc);
	if(ph->count==0)
		usedblock=0;
	else
	{
		curele=ph->head;
		for(i=0;i<ph->count;i++)
		{
			ele=&ph->eles[curele];
			usedblock+=ele->blocks;
			curele=NewerEle(pool,curele);
		}
	}
	MShmUnLock(pool->mc);
	printf("head=%d tail=%d count=%d tblock=%d used=%d fblock=%d fbstart=%d\n",ph->head,ph->tail,ph->count,ph->tblocks,usedblock,ph->fblocks,ph->fbstart);
	printf("_____________________________________________________________________\n");
	printf("userno\t\t\t\tusername\t not read \tconnect time\n");
	curtime=time(NULL);
	for (i=0;i<SHPOOL_MAX_USERS;i++)
	{
		if(usr->valid)
		{
			printf("%d\t",i);//userno
			printf("%32s\t",usr->name);
			printf("%8d\t",MShmDistanceEle(pool,usr->curele,ph->tail));
			printf("    %d(s)\t",(int)(curtime-usr->stime));

			printf("\n");
		}
		usr++;
	}
	printf("\n");
	printf("_____________________________________________________________________\n");

	exit(0);
}



