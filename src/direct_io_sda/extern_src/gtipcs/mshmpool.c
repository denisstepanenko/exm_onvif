/**************************************************************************
 * 文件名: mshmpool.c
 * 功能      : 用共享内存实现的缓冲池  ,可以由不同进程
 			  实现生产者和消费者
 *************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "mshm.h"
#include "mshmpool.h"
#include "msem.h"
#include <syslog.h>

static int newmaxsem=-1;
static int oldmaxsem;
static int places=0;

/**************************************************************************
  *函数名	:NewerEle
  *功能	:返回比cur更新的元素序号
  *参数	: pool:指向缓冲池的指针 
  			  cur  :当前元素序号
  *返回值	: 当前元素的下一个元素序号 ,该值永远合法
  *注		:pool的合法性以及cur的合法性由调用者保证
  *************************************************************************/
static __inline__ int NewerEle(MSHM_POOL *pool,int cur)
{
	if(++cur>=SHPOOL_MAX_ELE)
		cur=0;
	return cur;
}

/**************************************************************************
  *函数名	:OlderEle
  *功能	:返回比cur更老的元素序号
  *参数	: pool:指向缓冲池的指针 
  			  cur  :当前元素序号
  *返回值	: 当前元素的下一个元素序号 ,该值永远合法
  *注		:pool的合法性以及cur的合法性由调用者保证
  *************************************************************************/
static __inline__  int OlderEle(MSHM_POOL *pool,int cur)
{
	if(--cur<0)
		cur=SHPOOL_MAX_ELE-1;
	return cur;
}

/**************************************************************************
  *函数名	:MShmDistanceEle
  *功能	:返回两元素的距离
  *参数	: pool:指向缓冲池的指针 
  			  a     :第一个元素序号
  			  b     :第二个元素序号
  *返回值	: 两个元素的距离,逻辑上相当于b-a ,0表示相等正值表示b>a
  *注		:pool的合法性以及a,b的合法性由调用者保证
  *************************************************************************/
static __inline__ int MShmDistanceEle(MSHM_POOL *pool,int a,int b)
{
	SHPOOL_HEAD	*ph=pool->ph;
	int head=ph->head;
	//int tail=ph->tail;
	int distb=0;//距离
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
#if 0
/**************************************************************************
  *函数名	:MShmEleRemain
  *功能	:返回还有多少元素没有读取
  *参数	: pool:指向缓冲池的指针 
  *返回值	: 当前用户还有多少数据没有读取
  *注		:pool的合法性由调用者保证
  *************************************************************************/
static __inline__ int MShmEleRemain(MSHM_POOL *pool)
{
	SHPOOL_HEAD     	*ph=pool->ph;
	SHPOOL_USR	 	*usr=&ph->users[pool->userno];
	SHPOOL_ELE	      	*ele=&ph->eles[usr->curele];
	if(usr->curele==ph->tail)
		return 0;		//FIXME 当num超过unsigned long 表示的范围时应特殊处理
	return (ph->num-ele->num);	
}

#endif
/**************************************************************************
  *函数名	:SemAddAll
  *功能	:向缓冲池中所有用户发送资源信号量
  *参数	: pool:指向缓冲池的指针
  *返回值	:无
  *注:要求上一层函数锁共享内存,参数合法性由上一层函
  		数负责检查
  *************************************************************************/
static __inline__ void SemAddAll(MSHM_POOL *pool)
{
	int i;
	int rc;
	int SemVal;
	MEM_CORE 		*mc=pool->mc;
	SHPOOL_HEAD 	*ph=pool->ph;
	SHPOOL_USR   	*usr=ph->users;
	int			  	semid=ph->semid;

	
	if(newmaxsem!=-1)
	{
		ph->maxsem=newmaxsem;
	}
	else
	{
		ph->maxsem=oldmaxsem;
	}


	for(i=0;i<SHPOOL_MAX_USERS;i++)
	{
		if(usr->valid)
		{
			rc=SemAdd(semid,i);
			SemVal=SemGetVal(semid,i);
			//yk del 20130417 printf("Semgetval[%d]=====[%d]\n",i,SemVal);
			if(SemVal>ph->maxsem)
			{
				//yk del 20130417 printf("%s user:%d(%s) maybe exited(%d) ,delete it!\n",mc->name,i,usr->name,SemVal);
				if((SemVal)==(ph->maxsem+1))
				{
					syslog(LOG_INFO,"%s user:%d(%s) maybe exited(%d) ,delete it!\n",mc->name,i,usr->name,SemVal);
				}
				usr->valid=0;//用户可能已经不存在了,删除用户
			}
			if(rc<0)
			{
				printf("SemAddAll:SemAdd %d:%d --errno=%d %s  cur cnt=%d err\n",semid,i,rc,strerror(-rc),SemGetVal(semid,i));
				if(rc==-ERANGE)
				{
					//yk del 20130417printf("%s user:%d maybe exited ,delete it!\n",mc->name,i);
					usr->valid=0;//用户可能已经不存在了,删除用户
					
				}
				
			}
		}
		usr++;
			
	}
	return ;
}

/**************************************************************************
  *函数名	:GetAFreeUsrNo
  *功能	:获取一个空闲的用户序号
  *参数	: pool:指向缓冲池的指针 
  *返回值	: 负值表示出错 其他表示可用的用户序号
  *注		:pool的合法性由调用者保证
  *************************************************************************/
 static int GetAFreeUsrNo(MSHM_POOL *pool)
{//用户号每次都会递加
	int i;
	SHPOOL_HEAD *ph=pool->ph;
	SHPOOL_USR   *usr=NULL;
	if(ph->last_usr_no<0)
	{
		ph->last_usr_no=0;		
	}
	for(i=(ph->last_usr_no+1);i<SHPOOL_MAX_USERS;i++)
	{
		usr=&ph->users[i];
		if(!usr->valid)
		{
			ph->last_usr_no=i;
			return i;
		}
		//usr++;
	}
	for(i=0;i<=ph->last_usr_no;i++)
	{
		usr=&ph->users[i];
		if(!usr->valid)
		{
			ph->last_usr_no=i;
			return i;
		}
		//usr++;	
	}
	return -EBUSY;	
}
#if 0
//释放一个用户号指定的用户
//由上一层负责锁共享内存
static void FreeAUsrNo(MSHM_POOL *pool,int no)
{
	SHPOOL_HEAD *ph=pool->ph;
	SHPOOL_USR   *usr=ph->users;
	usr+=no;
	usr->valid=1;
	usr->upid=0;
}
#endif


int MShmPoolSetUserName(MSHM_POOL *pool,int userno,char *name)
{
	SHPOOL_HEAD	*ph;
	SHPOOL_USR   	*usr;
	if((pool==NULL)||(userno<0)||(userno>=SHPOOL_MAX_USERS)||(name==NULL))
		return -EINVAL;
	ph=pool->ph;
	if(ph==NULL)
		return -EINVAL;
	usr=&ph->users[userno];
	strncpy(usr->name,name,(sizeof(usr->name)-1));
	usr->name[sizeof(usr->name)-1]='\0';
	return 0;
}
//获取指定用户号的用户名 并将地址放入name中
int MShmPoolGetUsrName(MSHM_POOL *pool,int userno,char **name)
{
	SHPOOL_HEAD	*ph;
	SHPOOL_USR   	*usr;
	if((pool==NULL)||(userno<0)||(userno>=SHPOOL_MAX_USERS)||(name==NULL))
		return -EINVAL;	
	if(ph==NULL)
		return -EINVAL;
	usr=&ph->users[userno];
	*name=usr->name;
	return 0;
}

/**************************************************************************
  *函数名	:FreeBlocks
  *功能	:释放最老的指定块数的块
  *参数	: pool:指向缓冲池的指针 
  			  needfree:需要释放的块数
  *返回值	: 返回释放的块数
  *注		:pool的合法性以由调用者保证
  *************************************************************************/
static __inline__  int FreeBlocks(MSHM_POOL *pool,int needfree)
{
	SHPOOL_HEAD	*ph=pool->ph;
	SHPOOL_ELE		*ele;
	int head;
	int freecnt=ph->fblocks;

	while(freecnt<needfree)
	{
		if(ph->count>0)
		{			
			head=ph->head;
			ele=&ph->eles[head];
			freecnt+=ele->blocks;
			ph->fblocks+=ele->blocks;
			ph->count--;
			ele->datalen=0;
			ele->blocks=0;
			ph->head=NewerEle(pool,ph->head);
		}
		else
		{//没有足够的空间，可能是请求释放的块太多了
			return -ENOMEM;
		}
	}
	return freecnt;
	
}
//释放缓冲池头元素
//返回释放的块数
static __inline__  int FreeHeadEle(MSHM_POOL *pool)
{
	SHPOOL_HEAD	*ph=pool->ph;
	SHPOOL_ELE		*ele;
	int head;
	int freecnt;
	
	head=ph->head;
	ele=&ph->eles[head];
	freecnt=ele->blocks;
	ph->fblocks+=ele->blocks;
	ph->count--;
	ele->datalen=0;
	ele->blocks=0;
	ph->head=NewerEle(pool,ph->head);
	return freecnt;
	
}
/**************************************************************************
  *函数名	:MShmPoolReqBlocks
  *功能	:请求空闲的内存区块
  *参数	: pool:指向缓冲池的指针 
  			  need:需要的块数
  *返回值	: 返回空闲块的起始编号
  *注		:为提高效率 pool和need参数的合法性应由上层函数保证
  			 要求上一层函数锁共享内存
  *************************************************************************/
static __inline__ int MShmPoolReqBlocks(MSHM_POOL *pool,int need)
{
	SHPOOL_HEAD	*ph;
	int ret;
	ph=pool->ph;

	do
	{		
		if(ph->fblocks>need)
		{
			break;
		}
		ret=FreeBlocks(pool,need*2);//释放需求两倍的块
		if(ret>0)
			break;
		else
		{
			return -ENOMEM;	
		}
	}while(0);
	ret=ph->fbstart;
	ph->fbstart=(ph->fbstart+need)%(ph->tblocks);	
	return ret;
}

/**************************************************************************
  *函数名	:InsertData2Ele
  *功能	:将数据放入缓冲池的指定元素中(元素必须已经分配好)
  *参数	: pool:指向缓冲池的指针 
  			  ele  :指向要填入数据的元素
  			  buf:数据缓冲区的首地址
  			  len:buf中有效数据的长度
  *返回值	:返回0表示成功
  *注		:参数有效性由上一层调用者保证
  			 要求上一层函数锁共享内存
  *************************************************************************/
static __inline__ int InsertData2Ele(MSHM_POOL *pool,SHPOOL_ELE *ele,void *buf,int len)
{
	SHPOOL_HEAD	*ph        = pool->ph;
	SHPOOL_BLOCK 	*dblock = pool->eledata;
	int 				startblock=ele->bblock;
	int 				blocks=ele->blocks;
	int				tailblocks=ph->tblocks-startblock;	//尾部剩余的空闲块
	int				tailbytes=tailblocks*SHPOOL_BLOCK_SIZE;
	if(tailbytes>=len)
	{
		memcpy((void*)&dblock[startblock],buf,len);
	}
	else
	{
		memcpy((void*)&dblock[startblock],buf,tailbytes);
		memcpy((void*)&dblock[0],(buf+tailbytes),(len-tailbytes));
	}
	ele->datalen=len;
	ph->count++;
	ph->fblocks-=blocks;

	ph->send_bytes+=len;	//数据总量

	return 0;
}

/**************************************************************************
  *函数名	:SaveEle2Buf
  *功能	:将缓冲池的指定元素数据数据拷贝到缓冲区
  *参数	: pool:指向缓冲池的指针 
  			  buf:目标数据缓冲区的首地址
  			  buflen:缓冲区的长度
  			  eleno:要拷贝的元素序号
	输出   :eleseq:元素的序号
			 flag:元素的标志
  *返回值	:成功则返回拷贝的字节数(元素的有效长度)
  			 若用户缓冲区大小不够则返回 -ENOMEM
  *注		:参数有效性由上一层调用者保证
  			 要求上一层函数锁共享内存
  *************************************************************************/
static __inline__ int SaveEle2Buf(MSHM_POOL *pool,void *buf,int buflen,int eleno,int *eleseq,int *flag)
{
	SHPOOL_HEAD	*ph=pool->ph;	
	SHPOOL_ELE		*ele=&ph->eles[eleno];
	int 				 startblock=ele->bblock;
	int				tailblocks=ph->tblocks-startblock;	//尾部剩余的空闲块
	int				tailbytes=tailblocks*SHPOOL_BLOCK_SIZE;
	int				elelen=ele->datalen;//元素实际长度
	void				*cptr=NULL;			//拷贝数据的起始地址指针
	if(buflen<elelen)
	{
		printf("buflen=%d elen=%d!!!\n",buflen,elelen);
		return -ENOMEM;
	}
	cptr=&pool->eledata[startblock];
	if(elelen<=tailbytes)
	{
		memcpy(buf,cptr,elelen);
	}
	else
	{
		memcpy(buf,cptr,tailbytes);
		memcpy(buf+tailbytes,(void*)pool->eledata,(elelen-tailbytes));
	}
	*eleseq=ele->num;
	*flag=ele->userflag;
	return elelen;
}











/*************************以下函数是需要导出的****************************/

/**************************************************************************
  *函数名	:MShmPoolGetResource
  *功能	:从缓冲池中取出一个元素放入缓冲区
  *参数	: pool:指向缓冲池的指针 
  			  buf:目标数据缓冲区的首地址
  			  buflen:缓冲区的长度
	输出   :eleseq:元素的序号
			 flag:元素的标志
  *返回值	:成功则返回拷贝的字节数(元素的有效长度)
  			 若用户缓冲区大小不够则返回 -ENOMEM
  *指针参数的空指针情况应在上一层保证,如果参数为NULL则会出段错误
  *************************************************************************/
int MShmPoolGetResource(MSHM_POOL *pool,void *buf,int buflen,int *eleseq,int *flag)
{
	SHPOOL_HEAD	*ph;
	MEM_CORE		*mc;
	SHPOOL_USR		*usr;
	int cur;	//头，尾，当前位置的序号
	int rc;
	int cnt=0;

	if(buflen<0)	//其它指针参数的空指针情况应在上一层保证,如果参数为NULL则会出段错误
		return -EINVAL;
	mc=pool->mc;
	ph=pool->ph;
	usr=&ph->users[pool->userno];
	cur=usr->curele;
	if(mc==NULL)
		return -EINVAL;

	
	//while((ph->tail==ph->head)||(cur==ph->tail)||(abs(MShmDistanceEle(pool,cur,ph->tail))==places))
	{		
		rc=SemReq(ph->semid,pool->userno);	
		if(rc<0)
		{
			printf(" SemReq rc=%d ph->semid=%d userno=%d\n",rc,ph->semid,pool->userno);
			return rc;
		}
		// zsk fixbug  这样设置会出现写的多读的少
		//rc = SemSetVal(ph->semid,pool->userno,0);
		if(ph->head==0)
		{
			if(abs(MShmDistanceEle(pool,ph->head,ph->tail))<5)
			{//缓冲池已经被生产者重新初始化
				if(usr->curele>=ph->tail)
				{
					do
					{
						
						pthread_testcancel();
						rc=MShmLock(mc);
						if(rc!=0)
						{
							printf("MShmLock rc=%d!!\n",rc);
						}
					}while(rc!=0);
					usr->curele=ph->tail-1;
					MShmUnLock(mc);
					SemSetVal(ph->semid,pool->userno,0);
					
				}
			}
		}
		cur=usr->curele;

	}	
	do
	{
		pthread_testcancel();

		rc=MShmLock(mc);
		if(rc!=0)
		{
			printf("MShmLock rc=%d!!\n",rc);
		}
	}while(rc!=0);

	rc=SaveEle2Buf(pool,buf,buflen,cur,eleseq,flag);
	if(rc>=0)
	{
		usr->curele=NewerEle(pool,cur);	
	}
	MShmUnLock(mc);

	return rc;	
}

/**************************************************************************
  *函数名	:MShmPoolAddResource
  *功能	:将指定缓冲区的数据作为元素放入缓冲池尾部
  *参数	: pool:指向缓冲池的指针 
  			  buf:源数据缓冲区的首地址
  			  buflen:缓冲区的有效数据长度
			 flag:要加入的元素的标志
  *返回值	:返回0表示成功 负值表示出错
  *
  *************************************************************************/
 int MShmPoolAddResource(MSHM_POOL *pool,void *buf,int buflen,int flag)
{
	int needblock;
	int fstartb;//空闲块起始序号
	SHPOOL_HEAD	*ph;
	SHPOOL_ELE		*tail;
	MEM_CORE		*mc;
	int 				 retval=0;
	int				rc;
	if((pool==NULL)||(buf==NULL)||(buflen<0))
		return -EINVAL;       
	mc=pool->mc;
	do
	{
		pthread_testcancel();
		rc=MShmLock(mc);
		if(rc!=0)
		{
			printf("MShmLock rc=%d!!\n",rc);
		}
	}while(rc!=0);
	do
	{
		ph=pool->ph;
		tail=&ph->eles[ph->tail];
		needblock=(buflen-1)/SHPOOL_BLOCK_SIZE+1;	//数据需要的快数
		fstartb=MShmPoolReqBlocks(pool,needblock);
		if(fstartb<0)
		{
			retval=-ENOMEM;
			break;
		}
		ph->num++;


		tail->num=ph->num;
		tail->userflag=flag;
		tail->bblock=fstartb;
		tail->blocks=needblock;
		if((fstartb+needblock)>=ph->tblocks)
		{
			tail->eblock=(fstartb+needblock-1)-ph->tblocks;
		}
		else
		{
			tail->eblock=fstartb+needblock-1;
		}
		InsertData2Ele(pool,tail,buf,buflen);	//将数据填入尾部
		ph->tail=NewerEle(pool,ph->tail);
		if(ph->tail==ph->head)
		{
			FreeHeadEle(pool);	//缓冲池满时释放最老的元素
		}
	}while(0);

	if(retval==0)
		SemAddAll(pool);
	MShmUnLock(mc);

	return retval;
	
}




/**************************************************************************
  *函数名	:MShmPoolSetMaxNum
  *功能	:设置信号缓冲池中判断用户超时的元素数
  *参数	: pool:指向缓冲池的指针 
  			  num: 判断超时的元素个数
  *返回值	:返回0表示成功 负值表示失败错误是负的errno
  *                   本函数由调用MShmPoolCreate的进程调用,当用户有超过
  *                   num个元素没有读取时将会认为用户已经退出，踢出用户
  *************************************************************************/
int MShmPoolSetMaxNum(MSHM_POOL *pool,int num)
{
       int                         rc;
	MEM_CORE		*mc=NULL;
	SHPOOL_HEAD	*ph=NULL;
	if((mc==NULL)||(ph==NULL))
		return -EINVAL;
       mc=pool->mc;
       ph=pool->ph;

       rc=MShmLock(mc);
       ph->maxsem=num;
	  oldmaxsem=ph->maxsem;
       rc=MShmUnLock(mc);

       return 0;
}


void MSHmPoolSetMaxNum_t(int num)
{
	newmaxsem=num;
}
/**************************************************************************
  *函数名	:MShmPoolCreate
  *功能	:按照给定的pkey创建指定大小的共享缓冲池
  *参数	: name:缓冲池的名字
  			  pkey: 缓冲池的key(本函数将以这个key创建共享内存
  			           互斥锁信号量,并以(key|0x50000000)创建资源信号量)
  			  bytes:希望创建 的缓冲区中的实际数据容量
  *输出	:本函数将初始化pool结构中的字段
  *返回值	:返回0表示成功 负值表示失败错误是负的errno
  *
  *************************************************************************/
int MShmPoolCreate(char *name,MSHM_POOL *pool,key_t pkey,int bytes)
{
	int retval=0;
	int i;
	union semun semopts;
	MEM_CORE		*mc;
	SHPOOL_HEAD	*ph;
	long realsize;		//实际需要分配的字节数
	long pagesize;		//页面大小
	long offset;			//数据块的偏移
	void *p;
	int	rc;
	if((pool==NULL)||(pkey<0)||(bytes<=0))
	{
		return -EINVAL;
	}
	pagesize = sysconf(_SC_PAGESIZE);
	realsize=((sizeof(SHPOOL_HEAD)-1)/pagesize +1)+((bytes+1)/pagesize +1);//缓冲池的头占用的页面数+需要申请的数据空间数
	realsize*=pagesize;
	mc=MShmCoreCreate(name,pkey,realsize,NULL);
	if(mc==NULL)
		return -errno;//errno已经在MShmCoreCreate函数中设置

	pool->userno=0;
	do
	{
		pthread_testcancel();
		rc=MShmLock(mc);
		if(rc!=0)
		{
			printf("MShmLock rc=%d!!\n",rc);
		}
	}while(rc!=0);
	do{
		//初始化 MSHM_POOL结构中的指针
		p=(void *)mc;
		p+=mc->uoffset;
		pool->mc=mc;
		pool->ph=(SHPOOL_HEAD*)p;
		ph=(SHPOOL_HEAD*)pool->ph;
		offset=((sizeof(SHPOOL_HEAD)-1)/pagesize +1)*pagesize;//数据块按照页面对齐
		ph->boffset=offset;
		p=(void*)(ph);
		p+=ph->boffset;
		pool->eledata=(SHPOOL_BLOCK *)p;
		//初始化ph中的变量
		ph=(SHPOOL_HEAD*)pool->ph;
		ph->maxsem=100;//
		oldmaxsem=ph->maxsem;
		ph->num=0;
            ///20070509 shixin fix
		ph->tblocks=(((bytes-1)/pagesize + 1)*pagesize)/SHPOOL_BLOCK_SIZE;	//实际分配到的内存是按照页面对齐的

              ph->fblocks=ph->tblocks;
		ph->fbstart=0;
		//boffset字段已经在上面设置好了
		ph->count=0;
		ph->head=0;
		ph->tail=0;
		ph->last_usr_no=-1;//还没有用户
		ph->semkey=mc->mkey|0x50000000;
		ph->semid   = semget(ph->semkey, SHPOOL_MAX_USERS, IPC_CREAT|IPC_EXCL|0600);
		if (ph->semid <0)
		{
			if(errno == EEXIST)
			{
				ph->semid = semget(ph->semkey, 0, IPC_EXCL|0600);
			}
			if(ph->semid<0)
			{
				printf("failed create shar pool sem: 0x%x\n",ph->semkey);
				retval=-ENAVAIL;
				break;
			}
		}
		else
		{
			memset((void*)ph->pinfo,0,sizeof(ph->pinfo));
                    
		}
		for(i=0;i<SHPOOL_MAX_USERS;i++)
		{
			semopts.val=0;						//将所有用户的资源都设置为0
			semctl(ph->semid , 0, SETVAL, semopts); 
		}
		memset((void*)ph->eles,0,sizeof(ph->eles));//清空元素缓冲区
		memset((void*)pool->eledata,0,(SHPOOL_BLOCK_SIZE*ph->tblocks));//初始化数据区
		ph->send_bytes=0;
		pool->userflag=1;

	}while(0);
	MShmUnLock(mc);

	return retval;
}

/**************************************************************************
  *函数名	:MSHmPoolAttach
  *功能	:按照给定的pkey连接到缓冲池
  *参数	: pkey:要连接的 缓冲池的key
  *输出	:本函数将初始化pool结构中的字段
  *返回值	:返回0表示成功 负值表示失败错误是负的errno
  *注		:本函数主要用于监控缓冲池的运行状态的程序
  *************************************************************************/
int MSHmPoolAttach(MSHM_POOL *pool,key_t pkey)
{
	MEM_CORE		*mc=NULL;
	SHPOOL_HEAD	*ph=NULL;
	void 			*p=NULL;
	
	if((pool==NULL)||((int)pkey<0))
	{
		return -EINVAL;
	}
	errno=0;
	mc=MShmCoreAttach(pkey,NULL);
	if(mc==NULL)
		return -errno;//errno已经在MShmCoreCreate函数中设置
	if(mc->uoffset<=0)
	{
		//创建者还没有初始化完
		MShmCoreDetach(mc);
		return -EPERM;
	}

	//初始化 MSHM_POOL结构中的指针
	p=(void *)mc;
	p+=mc->uoffset;
	pool->mc=mc;
	pool->ph=(SHPOOL_HEAD*)p;
	ph=(SHPOOL_HEAD*)pool->ph;
	if(ph->boffset<=0)
	{
		//创建者还没有初始化完
		MShmCoreDetach(mc);
		return -EPERM;		
	}
	p=(void*)(ph);
	p+=ph->boffset;
	pool->eledata=(SHPOOL_BLOCK *)p;


	pool->userflag=2;
	return 0;
}
/**************************************************************************
  *函数名	:MShmPoolMvUsrPlace
  *功能	:将用户当前读取数据的指针按要求移动
  *参数	: pool:指向缓冲池的指针 
  			  place:要移动的数量
  			  	负值表示向前移动|place|格
  			  	正值表示向后移动place格
  			  	0表示不用移动
  *返回值	:返回>=0表示移动的单元数, 负值表示失败错误是负的errno
  *************************************************************************/
int MShmPoolMvUsrPlace(MSHM_POOL *pool,int place)
{
	int i;
	int num=0;	//
	int temp;
	int moved=0;
	MEM_CORE		*mc=pool->mc;
	SHPOOL_HEAD	*ph=pool->ph;
	SHPOOL_USR   	*usr=NULL;
	if((mc==NULL)||(ph==NULL))
		return -EINVAL;
	usr=&ph->users[pool->userno];
	if(place<0)
	{//从当前位置向前
		num=-place;
		temp=usr->curele;
		for(i=0;i<num;i++)
		{	//向前查找older_nums个老的元素
			temp=OlderEle(pool, temp);
			moved++;
			if(temp==ph->head)		//到头了
					break;	
		}
		//MShmLock(mc);
		usr->curele=temp;				//找到了		
		//MShmUnLock(mc);
	}
	else if(place>0)
	{
		num=place;
		temp=usr->curele;
		for(i=0;i<num;i++)
		{	//向前查找older_nums个老的元素
			temp=NewerEle(pool, temp);
			moved++;
			if(temp==ph->tail)		//到尾了
					break;	
		}
		//MShmLock(mc);
		usr->curele=temp;				//找到了		
		//MShmUnLock(mc);
	}
	places=moved;
	return moved;
		
}
/**************************************************************************
  *函数名	:MShmPoolReq
  *功能	:按照给定的pkey请求一个缓冲池,将结果填充到pool
  *参数	: pkey:要请求的 缓冲池的key
  			  name:请求内存池的用户名
  			  type:用户类型由上层函数传下来
  *输出	:本函数将初始化pool结构中的字段
  *返回值	:返回0表示成功 负值表示失败错误是负的errno
  *************************************************************************/
int MShmPoolReq(MSHM_POOL *pool,key_t pkey,char *name,int type)
{
	
	int retval=0;
	
	SHPOOL_HEAD	*ph=NULL;
	SHPOOL_USR   	*usr=NULL;
	MEM_CORE		*mc=NULL;
	int rc;
	retval=MSHmPoolAttach(pool,pkey);
	if(retval<0)
		return retval;
	mc=pool->mc;
	ph=pool->ph;
	do
	{
		pthread_testcancel();
		rc=MShmLock(mc);
		if(rc!=0)
		{
			printf("MShmLock rc=%d!!\n",rc);
		}
	}while(rc!=0);
	do{
		//初始化 MSHM_POOL结构中的指针
		retval=GetAFreeUsrNo(pool);
		if(retval<0)
		{
			pool->ph=NULL;
			pool->mc=NULL;
			pool->eledata=NULL;			
			break;
		}
		pool->userno=retval;
		SemSetVal(ph->semid,pool->userno,0);
		retval=0;
		usr=&ph->users[pool->userno];
		usr->valid=1;
		usr->curele=ph->tail;	//使最新元素成为当前元素
		usr->type=type;
		usr->upid=getpid();
		usr->stime=time(NULL);
		if(name==NULL)
			name="normal user";
		MShmPoolSetUserName(pool,pool->userno,name);
		pool->userflag=0;

	}while(0);
	MShmUnLock(mc);	

	if(retval!=0)
		MShmCoreDetach(mc);

	return retval;
}

/**************************************************************************
  *函数名	:MShmPoolReqFree
  *功能	:释放已经申请共享缓冲池
  *参数	: pool:缓冲池结构指针
  *返回值	:返回0表示成功 负值表示失败错误是负的errno
  *************************************************************************/
int MShmPoolReqFree(MSHM_POOL *pool)
{
	SHPOOL_HEAD	*ph;
	SHPOOL_USR   	*usr;
	int rc;
	if(pool==NULL)
		return -EINVAL;
	ph=pool->ph;
	if(ph==NULL)
		return -EINVAL;
	if(pool->userno>=SHPOOL_MAX_USERS)
		return -EINVAL;
	if(pool->userflag==MSHM_POOL_NORMAL_USR)
	{
		do
		{
			pthread_testcancel();

			rc=MShmLock(pool->mc);
			if(rc!=0)
			{
				printf("MShmLock rc=%d!!\n",rc);
			}
		}while(rc!=0);
		usr=&ph->users[pool->userno];
		usr->valid=0;
		MShmUnLock(pool->mc);

	}
	if(pool->mc!=NULL)
	{
		MShmCoreDetach(pool->mc);
		pool->mc=NULL;
	}
	return 0;
}
//

/**************************************************************************
  *函数名	:MShmPoolDelUsrByPid
  *功能	:删除指定进程号的用户
  *参数	: pool:缓冲池结构指针
  *返回值	:返回1 表示删除了用户
  *			 返回0表示用户原来就不存在
  *			 负值表示出错
  *************************************************************************/
int MShmPoolDelUsrByPid(MSHM_POOL *pool,pid_t pid)
{
	int i,Ret=0;
	SHPOOL_HEAD	*ph;
	SHPOOL_USR   	*usr;
	if(pool==NULL)
		return -EINVAL;
	ph=pool->ph;
	if(ph==NULL)
		return -EINVAL;

	usr=ph->users;
	for(i=0;i<SHPOOL_MAX_USERS;i++)
	{
		if(usr->upid==pid)
		{
			if(usr->valid)
			{				
				Ret=1;
			}
			usr->valid=0;
			break;			
		}
		usr++;
	}
	return 0;
}


/*******************信息维护************************************/

/**************************************************************************
  *函数名	:MShmPoolGetUsrInfo
  *功能	:获取指定用户号的用户信息并将地址放入info中
  *参数	: pool:缓冲池结构指针
  			: userno 用户号
  *输出	: info 返回时将info中填入缓冲区的地址
  *返回值	:返回负值表示出错，正值表示info缓冲区的大小
  *注		:info缓冲区中的信息内容及结构是由上层应用程
  			 序定义的
  			 在需要时由上层函数负责锁内存
  *************************************************************************/
int MShmPoolGetUsrInfo(MSHM_POOL *pool,int userno,void **info)
{
	SHPOOL_HEAD	*ph;
	SHPOOL_USR   	*usr;
	if((pool==NULL)||(userno<0)||(userno>=SHPOOL_MAX_USERS)||(info==NULL))
		return -EINVAL;	
	ph=pool->ph;
	if(ph==NULL)
		return -EINVAL;
	usr=&ph->users[userno];
	*info=usr->info;
	return sizeof(usr->info);
}

/**************************************************************************
  *函数名	:GetUsrValid
  *功能	:获取当前用户缓冲池是否仍然有效标志
  *参数	: pool:缓冲池结构指针
  *返回值	:返回负值表示出错，0表示无效 1表示有效

  *************************************************************************/
int GetUsrValid(MSHM_POOL *pool)
{
	SHPOOL_HEAD	*ph;
	SHPOOL_USR   	*usr;

	if(pool==NULL)
		return -EINVAL;	
	if(pool->userflag!=MSHM_POOL_NORMAL_USR)
		return -EINVAL;
	ph=pool->ph;
	if(ph==NULL)
		return -EINVAL;	
	usr=&ph->users[pool->userno];
	return usr->valid;
}
/**************************************************************************
  *函数名	:MshmPoolGetTotalUser
  *功能	:获取当前使用缓冲池的用户总数
  *参数	: pool:缓冲池结构指针
  *返回值	:返回缓冲池的用户总数,负值表示出错
  *************************************************************************/
int MShmPoolGetTotalUser(MSHM_POOL *pool)
{
	int i,total=0;
	SHPOOL_HEAD	*ph;
	SHPOOL_USR   	*usr;
	if(pool==NULL)
		return -EINVAL;
	
	ph=pool->ph;
	usr=ph->users;
	for(i=0;i<SHPOOL_MAX_USERS;i++)
	{
		if(usr->valid)
		{
			total++;
		}
		usr++;
	}
	return total;
}

/**************************************************************************
  *函数名	:MShmPoolGetInfo
  *功能	:获取当前使用缓冲池的应用自定义信息
  *参数	: pool:缓冲池结构指针
  *返回值	:返回应用自定义的信息地址
  *			如果需要则上层调用应负责将内存加锁
  *************************************************************************/
void *MShmPoolGetInfo(MSHM_POOL *pool)
{
	SHPOOL_HEAD	*ph;
	if(pool==NULL)
		return NULL;
	if(pool->ph==NULL)
		return NULL;
	ph=pool->ph;
	return (void*)ph->pinfo;
}

/**************************************************************************
  *函数名	:MShmPoolReActiveUsr
  *功能	:重新激活用户,如果用户长时间不能接收数据
  			:则缓冲池发送程序会把认为它退出了,所以接收者应
  			 定期调用此函数,重新激活用户(防止意外)
  *参数	: pool:缓冲池结构指针
  *返回值	:0表示正确 负值表示错误 1表示用户已经被删除，
  			现在重新激活
  *注		:本函数不会锁共享内存			
  *************************************************************************/
int	MShmPoolReActiveUsr(MSHM_POOL *pool)
{
	SHPOOL_USR   	*usr;
	int				rc;
	if(pool==NULL)
		return -EINVAL;
	if(pool->mc==NULL)
		return -EINVAL;
	
	if(pool->userflag!=MSHM_POOL_NORMAL_USR)
		return -EINVAL;
	if(pool->userno>=SHPOOL_MAX_USERS)
		return -EINVAL;
	usr=pool->ph->users;
	usr+=pool->userno;
	if(!usr->valid)
	{
		do
		{
			pthread_testcancel();

			rc=MShmLock(pool->mc);
			if(rc!=0)
			{
				printf("MShmLock rc=%d!!\n",rc);
			}
		}while(rc!=0);
		usr->valid=1;
		MShmUnLock(pool->mc);

		return 1;
	}
	return 0;
}

