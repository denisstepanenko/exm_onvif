#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <netdb.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>

#include <fcntl.h>      
#include <sys/file.h>
#include <termios.h>   
#include <sys/ioctl.h>
#include <time.h>
#include "hi_rtc.h"

int rtcfd = 0;


int main(int argc, char *argv[])
{
	rtc_time_t rtc_val;
	rtc_time_t rt;
	time_t sysTime;
	struct tm ntime;
	
	rtcfd = open("/dev/hirtc", O_WRONLY);
	if (rtcfd < 0)
	{
		printf("%s %d %s\n", __FILE__, __LINE__, strerror(errno));
		return (-1);
	}

	rtc_val.year=82 + 1900;
	rtc_val.month=11 + 1;
	rtc_val.date=29;
	rtc_val.hour=8;
	rtc_val.minute=8;
	rtc_val.second=8;
	rtc_val.weekday=1;

	//printf("set_dev_time %d-%d-%d-%d-%d-%d!\n",rtc_val.tm_year,rtc_val.tm_mon,rtc_val.tm_mday,rtc_val.tm_hour,rtc_val.tm_min,rtc_val.tm_sec);
	if(ioctl(rtcfd, HI_RTC_SET_TIME, &rtc_val) < 0)
	{
		printf("%s %d %s\n", __FILE__, __LINE__, strerror(errno));
		close(rtcfd);
		return (-1);
	}
    close(rtcfd);


	rtcfd = open("/dev/hirtc", O_RDONLY);
	if (rtcfd < 0)
	{
		printf("%s %d %s\n", __FILE__, __LINE__, strerror(errno));
		return (-1);
	}
	if(ioctl(rtcfd, HI_RTC_RD_TIME, &rt) < 0)
	{
		printf("%s %d %s\n", __FILE__, __LINE__, strerror(errno));
		close(rtcfd);
		return (-1);
	}

	printf("get_dev_time %d-%d-%d-%d-%d-%d!\n",rt.year,rt.month,rt.date,rt.hour,rt.minute,rt.second);
	ntime.tm_year = rt.year - 1900;
	ntime.tm_mon  = rt.month - 1;
	ntime.tm_mday = rt.date;
	ntime.tm_hour = rt.hour;
	ntime.tm_min  = rt.minute;
	ntime.tm_sec  = rt.second;
	sysTime = mktime(&ntime);
	stime(&sysTime);
	close(rtcfd);
}
	

