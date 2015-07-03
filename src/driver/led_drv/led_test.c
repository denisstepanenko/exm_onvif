#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "leds_api.h"
//#include "leds.h"
int leds_fd = 0;

int main(int argc, char *argv[])
{
	int astate = 0;
	int nstate = 0;

	leds_fd=open("/dev/leddrv",O_RDWR);
	if(leds_fd<0)
	{
		printf("can't open leds device !\n");
		return -1;
	}
	else
	{
		printf("open leds device success=%d.\n",leds_fd);	
	}

	//astate = get_current_alarmled();
	//nstate = get_current_netled();

	//printf("current alarm is %d,net is %d\n",astate,nstate);
	unsigned long errbuffer=1;
	ioctl(leds_fd,2,&errbuffer);
	char iostate1 = 7;
	ioctl(leds_fd,1,&iostate1);
	char iostate2 = 4;
	ioctl(leds_fd,3,&iostate2);

	sleep(3);
	
	//astate = get_current_alarmled();
	//nstate = get_current_netled();

	//printf("current alarm after set is %d,net is %d\n",astate,nstate);
	
	return 0;

}


