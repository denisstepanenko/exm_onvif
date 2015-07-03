/*检测并杀死ipmain进程*/
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

#include <signal.h>
#include <commonlib.h>
#include <syslog.h>
#include <file_def.h>


//在ip1004中仍然使用此方式进行延迟重启，需观察测试
#define VERSION "0.52"
//0.51 处理延时复位
#define gtloginfo(args...) syslog(LOG_INFO,##args)
#define REBOOTD_LOCK_FILE "/lock/ipserver/rebootd"
#define REBOOT_DELAY_FILE "/lock/ipserver/rebootdly"
static void sig_reboot(int signo)
{
	FILE *fp;
	int reboot_time;
	char *pt;
	char readbuf[256];
	reboot_time=0;
        fp=fopen(REBOOT_DELAY_FILE,"r");
        if(fp!=NULL)
        {
             pt=fgets(readbuf,sizeof(readbuf),fp);
             if(pt!=NULL)
                 reboot_time=atoi(pt);
	     fclose(fp);	
	     if(access(REBOOT_DELAY_FILE,F_OK)==0)
	     {
		remove(REBOOT_DELAY_FILE);
	     }	
        }
        if(reboot_time>10)
             reboot_time=5;

	system("/ip1004/rebootd");
	if(signo==SIGUSR1)
	{
		printf("rebootd receive soft reboot signal\n");
		gtloginfo("rebootd receive soft reboot signal\n");
		if(reboot_time!=0)
			sleep(reboot_time);
		system("/ip1004/softreboot");
	}
	if(signo==SIGUSR2)
        {
                printf("rebootd receive hard reboot signal\n");
                gtloginfo("receive hardware reboot signal\n");
		if(reboot_time!=0)
		{
				gtloginfo("sleep for reboot %d\n",reboot_time);
		        sleep(reboot_time);
		}
		system("cp /ip1004/hardreboot /tmp -f");  //??
		//gtloginfo("cp /ip1004/hardreboot /tmp -f done!\n");
		system("/tmp/hardreboot &");
		gtloginfo("/tmp/hardreboot & done!\n");
                //system("/gt1000/hardreboot");
        }

	
}
int main()
{
	int lock_file;
	char pbuf[100];
	openlog("rebootd",LOG_CONS|LOG_NDELAY|LOG_PID,LOG_LOCAL0 );
	lock_file=create_and_lockfile(REBOOTD_LOCK_FILE);
	if(lock_file<0)
	{
		printf("rebootd module are running!!\n");
		//gtloginfo("rebootd module are running!!\n");
		exit(0);
	}	
	printf("start rebootd(ver:%s).......\n",VERSION);
	gtloginfo("start rebootd(ver:%s).......\n",VERSION);

	sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
	write(lock_file,pbuf,strlen(pbuf)+1);

	if(signal(SIGUSR1,sig_reboot)==SIG_ERR)
	{
		printf("can't regist softreboot signal\n");
		gtlogerr("can't regist softreboot signal\n");
	}
        if(signal(SIGUSR2,sig_reboot)==SIG_ERR)
        {
                printf("can't regist softreboot signal\n");
                gtlogerr("can't regist softreboot signal\n");
        }

	while(1)
	{
		sleep(1000);
	}
	return 0;
}
