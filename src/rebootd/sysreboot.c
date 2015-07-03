/*¼ì²â²¢É±ËÀvsmain½ø³Ì*/
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>

#define REBOOT_DELAY_FILE "/lock/ipserver/rebootdly"

int main(int argc,char **argv)
{
	FILE *fp;
	if(argc>=2)
	{
		fp=fopen(REBOOT_DELAY_FILE,"w+");
		if(fp!=NULL)
		{
			fprintf(fp,"%s\n",argv[1]);
			fclose(fp);
		}
	}
		
#ifdef SOFT_REBOOT
	printf("software rebooting ...\n");
	system("killall -SIGUSR1 rebootd");
	printf("software reboot ok\n");
#endif
#ifdef HARD_REBOOT
	printf("hardware rebooting ...\n");
        system("killall -SIGUSR2 rebootd");
	sleep(1);
	printf("hardware reboot ok\n");
#endif

	return 0;
}
