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
#include "watchdog.h"


int wdfd = 0;

int main(int argc, char *argv[])
{
		int timeOut = 30;

        wdfd = open("/dev/watchdog",O_RDWR);
        if(wdfd < 0)
                printf("err open watchdog!\n");

        if (ioctl(wdfd ,WDIOC_SETTIMEOUT, &timeOut) < 0)
                printf("err set timeout!\n");


		while(1)
		{
			sleep(10);
			if (ioctl(wdfd, WDIOC_KEEPALIVE,0) < 0)
				printf("err feed dog!\n");
		}

		close(wdfd);


}
