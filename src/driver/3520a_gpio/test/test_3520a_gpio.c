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

#if 1
	gpio_groupbit_info a1;
	int fd;

	unsigned int gpiogroup,gpionum,value;
	if(argc<4)
	{
		printf("usage:./%s gpiogroup gpionum value",argv[0]);
		return 0;
	}

	if((gpiogroup=atoi(argv[1]))<0)
		printf("usage:./%s gpiogroup gpionum value",argv[0]);
	if((gpionum=atoi(argv[2]))<0)
		printf("usage:./%s gpiogroup gpionum value",argv[0]);
	if((value=atoi(argv[3]))<0)
		printf("usage:./%s gpiogroup gpionum value",argv[0]);



	if((fd=open("/dev/hi_gpio",O_RDWR))<0)
			perror("open");



	a1.groupnumber=gpiogroup;
	a1.bitnumber=gpionum;
	a1.value=0; /*方向*/
	if(ioctl(fd,GPIO_SET_DIR,&a1)<0)
		perror("set dir ioctl");
	if(ioctl(fd,GPIO_GET_DIR,&a1)<0)
		perror("get dir ioctl");
	printf("get set dir value:%#X \n",a1.value);



	a1.groupnumber=gpiogroup;
	a1.bitnumber=gpionum;
	a1.value=value; //输出的值
	if (ioctl(fd, GPIO_WRITE_BIT, &a1) < 0)
		perror("ioctl");
	if(ioctl(fd,GPIO_READ_BIT,&a1)<0)
			perror("ioctl");
	printf("get BIT:%#X \n",a1.value);
#else
	gpio_groupbit_info a1;
	int fd;
	if((fd=open("/dev/hi_gpio",O_RDWR))<0)
			perror("open");



	a1.groupnumber=12;
	a1.bitnumber=2;
	a1.value=0; /*方向*/
	if(ioctl(fd,GPIO_READ_BIT,&a1)<0)
			perror("ioctl");
	printf("get BIT:%#X \n",a1.value);

#endif





	return 0;

}
