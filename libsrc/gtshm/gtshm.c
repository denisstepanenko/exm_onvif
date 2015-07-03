/*************************************************
 * 
 * 文件名称:
 * 创建者:
 * 创建日期:
 * 简要描述:
 * 
 ************************************************/
/*************************************************
 * 
 * 函数名：
 * 简要描述： // 函数目的、功能等的描述
 * 输入： // 输入参数说明，包括每个参数的作用、取值说明及参数间关系，
 * 输出： // 输出参数的说明， 返回值的说明
 *
 *************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>


//存放共享内存信息的结构
struct GShmStruct
{
	key_t	shm_key;
	struct 	shmid_ds shmbuf;
	int 	fd_sem;	//用于描述互斥锁的描述符
	
};

//锁共享内存的方式类型定义
typedef enum {
    GSH_LOCK_RD, GSH_LOCK_RW
} GShmLockMode;


static void GShmInit(void)
{
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
    }
    return;
}

/*************************************************
 * 
 * 函数名：CreateShm()
 * 简要描述：	按照指定的key创建指定大小的共享内存区域
 * 输入： 	// key:要创建的共享内存的key值
 * 		   size:要创建的共享内存的大小
 * 		   mode:要创建共享内存的模式 1表示创建一个新的共享内存
 * 		   			     0表示连接到已经建立好的共享内存 * 		   			     
 * 输出： 	// 返回值：0表示操作成功 sh中存放着已经填充好的信息，以后用它来进行其他操作
 * 			 其他值表示有错误发生
 *
 *************************************************/
int CreateShm(key_t key,struct GShmStruct *sh,size_t size,int mode)
{
    if (size <= 0) 
    {
	    return -EINVAL;
    }
    GShmInit();
	return 0;
}

/*************************************************
 * 
 * 函数名：LockShm()
 * 简要描述：	按指定的模式锁住共享内存
 * 输入： 	// sh:存放要被锁定的共享内存区域的结构		  
 * 		   mode:要锁定共享内存的方式，取值可以是GSH_LOCK_RD和GSH_LOCK_WR  		   			     
 * 输出： 	// 返回值：0表示操作成功 
 * 		   其他值表示有错误发生
 *
 *************************************************/
int LockShm(struct GShmStruct *sh,GShmLockMode mode)
{
	return 0;
}

/*************************************************
 * 
 * 函数名：UnlockShm()
 * 简要描述：	将锁住的共享内存解锁
 * 输入： 	// sh:存放要被解锁的共享内存区域的结构 		  
 * 输出： 	// 返回值：0表示操作成功 
 * 		   其他值表示有错误发生
 *
 *************************************************/
int UnlockShm(struct GShmStruct *sh)
{
	return 0;
}

/*************************************************
 * 
 * 函数名：FreeShm()
 * 简要描述：	释放共享内存
 * 输入： 	// sh:存放要被释放的共享内存区域的结构 		  
 * 输出： 	// 返回值：0表示操作成功 
 * 		   其他值表示有错误发生
 *
 *************************************************/
int FreeShm(struct GShmStruct *sh)
{
	return 0;
}




int main(void)
{
	int rc;
	struct GShmStruct sh;
	rc=CreateShm(1,&sh,1024*16,1);
	printf("CreateShm rc=%d\n",rc);
	rc=LockShm(&sh,GSH_LOCK_RW);
	printf("LockShm rc=%d\n",rc);
	return 0;
}
