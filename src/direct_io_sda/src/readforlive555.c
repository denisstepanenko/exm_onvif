#include "blocks.h"
#include <signal.h>
#include <sys/time.h>
static	long long seek;





extern int sec_read_data();



//#define SEG

#ifdef SEG
#include <sys/sem.h>
#include <errno.h>
#include <stdio.h>
union semun {
         int val;                  		/* value for SETVAL */
         struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
         unsigned short *array;    /* array for GETALL, SETALL */
                                   		/* Linux specific part: */
         struct seminfo *__buf;    /* buffer for IPC_INFO */
};

//请求指定的信号量集中的指定信号量资源
static __inline__ int SemReq(int semid,int sem_num)
{
	int rc=0;
	struct sembuf sem={sem_num,-1,0};
	while ((rc = semop(semid, (struct sembuf *)&sem, 1)) < 0)
	{
		rc=-errno;
		if(errno==-EINTR)
		{
//			pthread_testcancel();
			continue;
		}
		else
			break;
	}
	return rc;
}
//设置指定信号量集中的信号量的值
static __inline__ int SemSetVal(int semid,int sem_num,int val)
{
	union semun semopts;
	semopts.val=val;
	return semctl(semid,sem_num,SETVAL,semopts);
}
#else
#define REALTIME
void init_sigaction(void)
{
	struct sigaction act;

	act.sa_handler = (void *)sec_read_data;
	act.sa_flags   = 0;
	sigemptyset(&act.sa_mask);
#ifdef REALTIME
	sigaction(SIGALRM, &act, NULL);
#else
	sigaction(SIGPROF, &act, NULL);
#endif
}

/* init */
void init_time(void)
{
	struct itimerval val;

	val.it_value.tv_sec = 0;
	val.it_value.tv_usec = 40000;
	val.it_interval = val.it_value;
#ifdef REALTIME
	setitimer(ITIMER_REAL, &val, NULL);
#else
	setitimer(ITIMER_PROF, &val, NULL);
#endif
}
#endif

int main(int argc, char **argv)
{
	int ret;



	printf("time:%d::%d\n",time(NULL),time(NULL)%SECOFDAY);


	if(init_sda()<0)
	{
		printf("init sda error and exit!!!\n");
		return -1;
	}
	/*用法参数：想从什么时间开始看*/
	if(argc !=2 )
	{
		printf("usage %s time\n",argv[0]);
		exit(1);
	}

	/*获取时间，想从什么时间开始看*/
	time_t want_time=atoi(argv[1]);
	printf("debug:time:%d\n",want_time);

	/*实始化年块*/
	if(year_read_init()!=0)
	{
		printf("read year block error,maybe the disk have not run write sda\n");
		goto error;
	}

	printf("debug:1\n\n\n\n");


	/*初始化天块*/
	seek=day_read_init(want_time);
	if(seek<0)
	{
		printf("read day block error,maybe disk io error\n");
		goto error;
	}
	printf("debug:2\n\n\n\n\n\n\n");

#if 1
	if(fifo_init()<0)
	{
		printf("fifo init error\n");
		return -1;
	}
#endif
	set_seek(seek);
#ifndef SEG
	init_sigaction();
	init_time();
	while(1);
#else

	key_t key=0x30000;
	key = key|0x50000000;;
	int no = 1;
	int semid=semget(key, 0, IPC_EXCL|0600);
	if(semid<0)
	{
		perror("semget error\n");
		return -1;
	}



	while(1)
	{
		 ret=SemReq(semid,no);
		 if(ret<0)
		 {
			 perror("SemReq error\n");
			 return -1;
		 }
		 ret=SemSetVal(semid,no,0);
		 if(ret<0)
		 {
			 perror("SemReq error\n");
			 return -1;
		 }
		 sec_read_data();
	}
#endif

error:
	free_sda();
	return -1;

}
