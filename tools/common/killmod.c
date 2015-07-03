/*结束应用程序模块*/
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <commonlib.h>
#include <signal.h>
#include <file_def.h>
#define 	VERSION		"0.2"
//0.2 加入将执行命令的信息记录日志

//第一个参数是应用程序的名字
//第二个参数是应用程序对应的锁文件路径
int main(int argc,char *argv[])
{
	int lfd,ret;
	char line[256];
	char sbuf[256];
	char *mod_name=NULL;
	char *lock_file=NULL;
	printf("killmod version:%s:\n",VERSION);
	if(argc<2)
	{
		printf("Usage :killmod [mod_name] [lock_file]\n");
		exit(1);
	}
	gtopenlog("killmod");	
	mod_name=argv[1];
	if(argc>=3)
	{//传入了锁文件路径
		lock_file=argv[2];
		lfd=create_and_lockfile(lock_file);
		if(lfd>=0)
		{
			printf("maybe no %s running ,run force kill!!\n",mod_name);
			unlock_file(lfd);
			close(lfd);
		}		
		else
		{
			lfd=open(lock_file,O_RDWR,0640);
			if(lfd>=0)
			{
				ret=read(lfd,line,20);
				if(ret>0)
				{
					ret=kill(atoi(line),SIGKILL);
					printf("kill %s(%d) ret=%d\n",mod_name,atoi(line),ret);
				}
				close(lfd);
			}	
		}
	}
	sprintf(sbuf,"killall -9 %s 2>/dev/null",mod_name);
	system(sbuf);
	printf("%s killed!\n",mod_name);
	gtloginfo("%s killed!!\n",mod_name);
	exit(0);
	
}
