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
#include "time.h"
#include "gpio_i2c.h"



/*把bcd码转成十进制*/
unsigned char inline bcdtobin(unsigned char hex)
{
	return (  (hex>>4)*10 + (hex&0xf) );
}

unsigned char inline bintobcd(unsigned char bin)
{
	return ( bin/10*16+bin%10  );
}
/*如果rtc没有工作就让它工作，主要是寄存器0最高位置0*/
void init()
{
	int fd = open("/dev/gpioi2c", 0);
    if(fd<0)
    {
    	printf("Open gpioi2c error!\n");
    	return ;
    }

	int value=0;
	int ret;

	/*读取值*/
	value = 0;
	value |=0xd0<<24;	//device addr
	value |=0x0<<16;	//reg addr
	ret = ioctl(fd, GPIO_I2C_READ, &value);//年
	if(ret<0)
			printf("ioctl write2\n");
	//printf("value:%#X \n",value);

	if(value&0x80)//如果最高位为1则置0写入
	{
		/*写入值*/
		value &=~0x80;  //把rtc寄存器的最高位置0，让rtc开始工作
		//value |=0x80; //把rtc寄存器的最高位置1，让rtc停止工作
		ret = ioctl(fd, GPIO_I2C_WRITE, &value);//年
		if(ret<0)
				printf("ioctl write2\n");
		//printf("value:%#X \n",value);
	}

	//释放资源
	close(fd);
}
void readtime(struct tm *time)
{
	int fd = open("/dev/gpioi2c", 0);
    if(fd<0)
    {
    	printf("Open gpioi2c error!\n");
    	return ;
    }

	int value=0;
	int ret;


	value = 0;
	value |=0xd0<<24;
	value |=0x6<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//年
	if(ret<0)
			printf("ioctl write2\n");
	value &=0xff;
	printf("year:%#X \n",value);
	time->tm_year=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x5<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//月
	if(ret<0)
			printf("ioctl write2\n");
	value &=0x1f;//月只是低5位有效
	printf("month:%#X \n",value);
	time->tm_mon=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x4<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//日
	if(ret<0)
			printf("ioctl write2\n");
	value &=0x3f;//日只是低6位有效
	printf("year:%#X	\n",value);
	time->tm_mday=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x3<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//周
	if(ret<0)
			printf("ioctl write2\n");
	value &=0xff;
	printf("week:%#X	\n",value);


	value = 0;
	value |=0xd0<<24;
	value |=0x2<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//时
	if(ret<0)
			printf("ioctl write2\n");
	value &=0x3f;//时只是低7位有效
	printf("hour:%#X \n",value);
	time->tm_hour=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x1<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//分
	if(ret<0)
			printf("ioctl write2\n");
	value &=0x7f;//分只是低7位有效
	printf("min:%#X \n",value);
	time->tm_min=bcdtobin((unsigned char)value);

	value = 0;
	value |=0xd0<<24;
	value |=0x0<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);//秒
	if(ret<0)
			printf("ioctl write2\n");
	value &=0x7f;//秒只是低7位有效
	printf("sec:%#X \n",value);
	time->tm_sec=bcdtobin((unsigned char)value);


	//释放资源
	close(fd);

}


void settime(struct tm *time)
{
	int fd = open("/dev/gpioi2c", 0);
    if(fd<0)
    {
    	printf("Open gpioi2c error!\n");
    	return ;
    }

	unsigned int value=0;
	int ret;
	unsigned char tm;


	tm=bintobcd(time->tm_year);
	value = tm;
	value |=0xd0<<24;
	value |=0x6<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//年
	if(ret<0)
			printf("ioctl write2\n");
	value &=0xff;
	printf("year:%#X \n",value);
	time->tm_year=bcdtobin((unsigned char)value);

	tm=bintobcd(time->tm_mon);
	value = tm;
	value &=0x1f;//月只是低5位有效
	value |=0xd0<<24;
	value |=0x5<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//月
	if(ret<0)
			printf("ioctl write2\n");
	printf("month:%#X \n",value);

	tm=bintobcd(time->tm_mday);
	value = tm;
	value &=0x3f;//日只是低6位有效
	value |=0xd0<<24;
	value |=0x4<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//日
	if(ret<0)
			printf("ioctl write2\n");
	printf("year:%#X	\n",value);
/*
	tm=bintobcd(time->tm_year);
	value = tm;
	value |=0xd0<<24;
	value |=0x3<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//周
	if(ret<0)
			printf("ioctl write2\n");
*/
	tm=bintobcd(time->tm_hour);
	value = tm;
	value &=0x3f;//时只是低7位有效
	value &= ~(1L<<6);//第6位置0是24小时制
	value |=0xd0<<24;
	value |=0x2<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//时
	if(ret<0)
			printf("ioctl write2\n");
	printf("hour:%#X \n",value);

	tm=bintobcd(time->tm_min);
	value = tm;
	value &=0x7f;//分只是低7位有效
	value |=0xd0<<24;
	value |=0x1<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//分
	if(ret<0)
			printf("ioctl write2\n");
	printf("min:%#X \n",value);

	tm=bintobcd(time->tm_sec);
	value = tm;
	value &=0x7f;//秒只是低7位有效
	value |=0xd0<<24;
	value |=0x0<<16;
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);//秒
	if(ret<0)
			printf("ioctl write2\n");
	printf("sec:%#X \n",value);



	//释放资源
	close(fd);

}


int main()
{
	struct tm time;

	init();
	 //readtime(&time );

	 //printf("year:%d,mon:%d,day:%d,  hour:%d,min:%d,sec:%d\n",time.tm_year,time.tm_mon,time.tm_mday,time.tm_hour,time.tm_min,time.tm_sec);

	 //time.tm_hour +=1;

	if(0)
	{
		 time.tm_year=0; time.tm_mon=2;time.tm_mday=28; time.tm_hour=23; time.tm_min=58; time.tm_sec=59;
		 settime(&time );
	}

	 readtime(&time );
	 printf("year:%d,mon:%d,day:%d,  hour:%d,min:%d,sec:%d\n",time.tm_year,time.tm_mon,time.tm_mday,time.tm_hour,time.tm_min,time.tm_sec);

}
