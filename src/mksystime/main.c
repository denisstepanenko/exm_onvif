
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
//#include <linux/rtc.h>
#include "hi_rtc.h"
#if 0
unsigned char inline bcdtobin(unsigned char hex)
{
	return (  (hex>>4)*10 + (hex&0xf) );
}

unsigned char inline bintobcd(unsigned char bin)
{
	return ( bin/10*16+bin%10  );
}
#endif
int main(int argc,char **argv)
{
	int oc;
	int savemode = 0;
	while((oc=getopt(argc,argv,"s"))>=0)
        {
                switch(oc)
                {
                     case 's':
                            printf("mksystime -s\n");
							savemode = 1;
                     break;
			default:
			break;
                }
        }

	int fd = open("/dev/hi_rtc",0);
	if(fd<0)
	{
		printf("open hi rtc error!\n");
		return;
	}

        int ret;
        struct tm nowtime;
        time_t sysTime;
        struct tm ntime;
	rtc_time_t tm;
        //unsigned char tm;

	if(savemode == 0)
	{
		ret = ioctl(fd, HI_RTC_RD_TIME, &tm);
		if (ret < 0)
		{
                	printf("ioctl: HI_RTC_RD_TIME failed\n");
                        goto err1;
                }
		close(fd);

		printf("get_dev_time %d-%d-%d-%d-%d-%d!\n",tm.year,tm.month,tm.date,tm.hour,tm.minute,tm.second);

		ntime.tm_year = tm.year-1900;
        	ntime.tm_mon  = tm.month-1;
        	ntime.tm_mday = tm.date;
	        ntime.tm_hour = tm.hour;
	        ntime.tm_min  = tm.minute;
        	ntime.tm_sec  = tm.second;

	        sysTime = mktime(&ntime);

	        stime(&sysTime);
	
		return 0;	
	}	
	else
	{
		tm.year = 2014;
		tm.month = 4;
		tm.date = 1;
		tm.hour = 0;
		tm.minute = 0;
		tm.second = 0;

		ret = ioctl(fd, HI_RTC_SET_TIME, &tm);
                if (ret < 0) {
                      printf("ioctl: HI_RTC_SET_TIME failed\n");
                      goto err1;
                }
		return 0;
	}

err1:
	close(fd);
	return 0;
}


