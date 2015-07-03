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


	int fd;
	if((fd=open("/dev/hi_gpio",O_RDWR))<0)
			perror("open");
	relay_struct a={0,1};

#if 1
	if(argc!=3){printf("usage:%s ch num",argv[0]);exit(1);}
	a.ch=atoi(argv[1]);
	a.result=atoi(argv[2]);
	printf("ch=%d,result=%d\n",a.ch,a.result);
	if(ioctl(fd, GPIO_SET_OUTPUT_VALUE ,&a)<0)
		perror("ioctl");
#else
	gpio_groupbit_info b;
	b.groupnumber=11;
	b.bitnumber=4;
	b.value=0;
	if(ioctl(fd, GPIO_SET_DIR ,&b)<0)
		perror("ioctl");
	if(ioctl(fd, GPIO_WRITE_BIT ,&b)<0)
		perror("ioctl");
#endif
	close(fd);
}
