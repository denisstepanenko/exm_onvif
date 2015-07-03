/*
 * test_alarm_in.c
 *
 *  Created on: 2013-4-11
 *      Author: xy
 */


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
#include "hi_gpio.h"

int main(int argc, char *argv[])
{

	if(argc!=2){printf("usage:%s 0xstate",argv[0]);exit(1);}

	int fd;

	TimeDelay b={3,0};


	if((fd=open("/dev/hi_gpio",O_RDWR))<0)
			perror("open");




	unsigned long a=0x00;
	sscanf(argv[1],"0x%x",&a);
	printf("state=%#x\n",a);
	ioctl(fd, GPIO_SET_INPUT ,&a);
	ioctl(fd,GPIO_SET_DELAY,&b);

	a=0;
	while(1)
	{

	if (ioctl(fd, GPIO_GET_DWORD, &a) < 0)
		perror("ioctl");
	printf("::%#X\n",a);



	sleep(1);
	}






	return 0;

}
