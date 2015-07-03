/**************************************************************************
 * 文件名 : mshm.c
 * 功能	: 将共享内存的调用接口进行包装，提供易于使
 			   用的接口   
 *************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <pthread.h>
#include "mshm.h"
#include "msem.h"


#define MSHM_MAGIC      0x5500aa00              ///共享内存标志
static const struct sembuf s_DoLock={//lock用信号量结构初始化
			sem_num:0,
			sem_op:-1,
	#ifndef VENC_TEST
			sem_flg:SEM_UNDO,		
	#else
			sem_flg:(IPC_NOWAIT |SEM_UNDO),
	#endif
};

static const struct sembuf s_DoUnLock={//unlock用信号量结构初始化
			sem_num:0,
			sem_op:1,
	#ifndef VENC_TEST
			sem_flg:SEM_UNDO,
	#else
			sem_flg:(IPC_NOWAIT |SEM_UNDO),
	#endif
};


/**************************************************************************
  *函数名	:MShmCoreCreate
  *功能	:以给定的key和usersize创建共享内存
  *参数	: mkey:共享内存的key 
  *			  usersize:用户程序需要的空间
  *			  name:要创建的共享内存的名字
  *                   u_ptr:输出值,填充用户可用的内存区域起始地址,NULL表示不需要填充
  *返回值	: 返回NULL表示有错误 错误码存在errno
  *			  返回连接好的内存地址MemCore指针,
  *			  以后用这个指针作为参数调用其它函数
  *注:本函数并不会把分配到的内存初始化为0
  *************************************************************************/
MEM_CORE * MShmCoreCreate(char *name,key_t mkey,size_t usersize,void **u_ptr)
{
	int err;
	int FirstCreateFlag=0;	//首次创建该key的共享内存标志
	MEM_CORE *mc=NULL;
	key_t semkey;
	size_t tsize;				//实际分配的共享内存大小
	size_t pagesize;
	void *area = ((void *)-1);	//连接好的内存地址
	int fdmem=-1;			//共享内存句柄
	int fdsem=-1;			//信号量句柄
	struct shmid_ds shmbuf;
//	union semun semopts;
	if ((usersize <= 0)||(mkey<0))
	{
		errno=EINVAL;
		return NULL;
	} 	

#ifdef FOR_3022
while(1)
{
#endif
/******************分配共享内存***************************/
	pagesize = sysconf(_SC_PAGESIZE);					//系统页大小
    	tsize = (((usersize-1)/pagesize+1)+1)*pagesize;		//实际分配时保留一页作为共享内存描述的头
	//创建共享内存
	fdmem=shmget(mkey, tsize, (SHM_R|SHM_W|IPC_CREAT|IPC_EXCL));
	if(fdmem<0)
	{
		err=errno;
		if(err==EEXIST)
		{	//已经存在
			fdmem=shmget(mkey, 0, 0);
		
#ifdef FOR_3022
			if(fdmem<0)
			{
				printf("failed create share memory 0x%x\n",mkey);
				return NULL;
			}
			shmctl(fdmem, IPC_STAT, &shmbuf);
			if(shmbuf.shm_segsz < tsize)
			{
				///先删除原来的，再重新建立新的共享内存
				err=shmctl(fdmem, IPC_RMID, 0);
				if(err<0)
				{
					printf("有进程在使用共享内存\n");
				}
				continue;	
			}
#endif
		}
		if(fdmem<0)
		{
			printf("failed create share memory 0x%x,err=%d\n",mkey,err);
			return NULL;
		}
	}
	else
	{
		FirstCreateFlag=1;
	}
	//连接共享内存
	area = (void *)shmat(fdmem, NULL, 0);
	if(area==((void*)-1))
	{
		printf("failed attach share memory 0x%x\n",mkey);
		return NULL;
	}
	if(FirstCreateFlag)
	{//如果是第一次创建则将共享内存初始化为0
		memset((void*)area,0,tsize);
	}
	//设置共享内存	
     	shmctl(fdmem, IPC_STAT, &shmbuf);
    	shmbuf.shm_perm.uid = getuid();
    	shmbuf.shm_perm.gid = getgid();
    	shmctl(fdmem, IPC_SET, &shmbuf);
 //   if (shmctl(fdmem, IPC_RMID, NULL) == -1)
   //     FAIL(MM_ERR_CORE|MM_ERR_SYSTEM, "failed to remove shared memory in advance");



	semkey=mkey;	//让信号量的key与共享内存的key相同

	fdsem=SemCreate(semkey,1);
	if(fdsem<0)
	{
		printf("failed create share memory sem: 0x%x\n",semkey);
              shmdt(area);
		return NULL;
	}
	//semopts.val=1;
	//semctl(fdsem, 0, SETVAL, semopts); 
	SemSetVal(fdsem,0,1);
	/*
	 * Configure the memory core parameters
	 */
	mc = (MEM_CORE *)area;
	mc->mkey=mkey;
	mc->tsize     = tsize;
	mc->usize    = usersize;	
	mc->memid  = fdmem;
	mc->semid	   = fdsem;
	mc->uoffset  = pagesize;
	if(name!=NULL)
	{
		sprintf(mc->name,"%s",name);
	}
	else
		sprintf(mc->name,"%s","NONAME");
       if(u_ptr!=NULL)
       {
               *u_ptr=(void*)((void*)mc+mc->uoffset);
       }
       mc->magic=MSHM_MAGIC;
	return mc;
#ifdef FOR_3022
}
#endif
}

//
/**************************************************************************
  *函数名	:MShmCoreAttach
  *功能	:按照给定的key打开并连接共享内存
  *参数	: mkey:共享内存的key 
  *                   u_ptr:输出值,填充用户可用的内存区域起始地址,NULL表示不需要填充
  *返回值	: 返回NULL表示有错误 错误码存在errno ENOENT表示该
  				key的共享内存还没有被创建
  			  返回连接好的内存地址MemCore指针,
  			  以后用这个指针作为参数调用其它函数
  *************************************************************************/
MEM_CORE * MShmCoreAttach(key_t mkey,void **u_ptr)
{
	MEM_CORE *mc=NULL;
	void *area = ((void *)-1);	//连接好的内存地址
	int fdmem=-1;			//共享内存句柄
	int pagesize = sysconf(_SC_PAGESIZE);					//系统页大小

	if (mkey<0)
	{
		errno=EINVAL;
		return NULL;
	} 	

	//打开已经被创建的共享内存
	fdmem=shmget(mkey, 0, 0);
	if(fdmem<0)
	{
		//printf("share memory 0x%x not be created\n",mkey);
		return NULL;
	}

	//连接共享内存
	area = (void *)shmat(fdmem, NULL, 0);
	if(area==((void*)-1))
	{
		printf("failed attach share memory 0x%x\n",mkey);
		return NULL;
	}
	mc = (MEM_CORE *)area;
       if(mc->uoffset!=pagesize)
       {///说明可能还没有正常初始化
            errno=EAGAIN;
            printf("mc->uoffset=%d pagesize=%d!!\n",(int)mc->uoffset,(int)pagesize);
            MShmCoreDetach(mc);
            return NULL;
       }
       if(u_ptr!=NULL)
       {
               *u_ptr=(void*)((void*)mc+mc->uoffset);
       }
	return mc;
}
/**************************************************************************
  *函数名	:MShmCoreDetach
  *功能	:退出到指定共享内存的连接
  *参数	: mc:共享内存的首地址
  *返回值	: 0表示成功
  *************************************************************************/
int MShmCoreDetach(MEM_CORE *mc)
{
		if(mc==NULL)
			return -EINVAL;
		return shmdt(mc);
}

/**************************************************************************
  *函数名	:MShmLock
  *功能	:取得对指定共享内存的访问权
  *参数	: mc :共享内存的起始地址(由MShmCoreCreate或MShmCoreAttach返回)
  *返回值	: 0表示成功 负值表示错误 错误码在errno中
  *************************************************************************/
#include <syslog.h>
int MShmLock(MEM_CORE *mc)
{
	int rc;
	if(mc==NULL)
	{
		errno=EINVAL;
		return -1;	
	}
       if(mc->magic!=MSHM_MAGIC)
       {///共享内存区域被破坏,程序退出
                printf("MEM_CORE bad magic:0x%x exit!!\n",(int)mc->magic);
                syslog(LOG_ERR,"MEM_CORE bad magic:0x%x exit!!\n",(int)mc->magic);
                exit(1);
       }
	//printf("lock sem val=%d\n",SemGetVal(mc->semid,0));

	while (((rc = semop(mc->semid, (struct sembuf *)&s_DoLock, 1)) < 0))
	{
		rc=-errno;

#ifdef VENC_TEST
		//printf("MShmLock卡住了rc=[%d]...\n",rc);
		pthread_testcancel();
		usleep(50);
#endif

		if(rc==-EINTR)
		{
			//pthread_testcancel();			
			continue;
		}
		else
		{
#ifdef VENC_TEST
			if(rc!=-EAGAIN)	////zw-test
#endif
				break;
		}
	}
	return rc;

}
/**************************************************************************
  *函数名	:MShmLock
  *功能	:释放对指定共享内存的访问权
  *参数	: mc :共享内存的起始地址(由MShmCoreCreate或MShmCoreAttach返回)
  *返回值	: 0表示成功 负值表示错误 错误码在errno中
  *************************************************************************/
int MShmUnLock(MEM_CORE *mc)
{
	int rc;
	if(mc==NULL)
	{
		errno=EINVAL;
		return -1;	
	}
       if(mc->magic!=MSHM_MAGIC)
       {///共享内存区域被破坏,程序退出
                printf("MEM_CORE bad magic:0x%x exit!!\n",(int)mc->magic);
                syslog(LOG_ERR,"MEM_CORE bad magic:0x%x exit!!\n",(int)mc->magic);
                exit(1);
       }    
	while (((rc = semop(mc->semid, (struct sembuf *)&s_DoUnLock, 1)) < 0))
	{
		rc=-errno;

#ifdef VENC_TEST
		//printf("MShmUnLock 卡住了rc=[%d]...\n",rc);
		pthread_testcancel();
		usleep(50);
#endif

		if(rc==-EINTR)
		{
//			pthread_testcancel();
			continue;
		}
		else
		{
#ifdef VENC_TEST
			if(rc!=-EAGAIN)	////zw-test
#endif
				break;
		}
	}
	return rc;
}


