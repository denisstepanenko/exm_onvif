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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/file.h>
#include <termios.h>   
#include <sys/ioctl.h>

#include <time.h>
#include <sys/time.h>
#include <commonlib.h>
#include "hi_rtc.h"

int testRTC()
{
//lc 3520D rtc test changed!!
   struct tm ntime;
   int   ret;
   int   fd;
   rtc_time_t tm;

   fd = open("/dev/hi_rtc",0);
   if(fd<0)
   {
	   printf("open hi rtc error!\n");
	   return -1;
   }
   ret = ioctl(fd, HI_RTC_RD_TIME, &tm);
   if (ret < 0)
   {
	   printf("%s %d %s\n", __FILE__, __LINE__, "读时间出错");
	   close(fd);
	   return -1;
   }      

   printf("get_dev_time %d-%d-%d-%d-%d-%d!\n",tm.year,tm.month,tm.date,tm.hour,tm.minute,tm.second);
   // wait for it getin
   sleep(2);
   tm.year = 2014;
   tm.month = 6;
   tm.date = 1;
   tm.hour = 0;
   tm.minute = 0;
   tm.second = 0;

	ret = ioctl(fd, HI_RTC_SET_TIME, &tm);
	if (ret < 0) {
        printf("%s %d %s\n", __FILE__, __LINE__, "设置时间出错");
		close(fd);
		return -1;
	}

    /*等待2秒，看时间是否在走*/
    sleep(2);
    ret = ioctl(fd, HI_RTC_RD_TIME, &tm);
	printf("get_dev_time %d-%d-%d-%d-%d-%d!\n",tm.year,tm.month,tm.date,tm.hour,tm.minute,tm.second);
	if(ret < 0)
	{
	   printf("%s %d %s\n", __FILE__, __LINE__, "读时间出错");
	   close(fd);
	   return -1;
	}

    if((tm.second > 3)||(tm.second < 1))
    {
        printf("%s %d %s\n", __FILE__, __LINE__, "时间没走");
		close(fd);
        return (-1);
    }

    return 0;
    
}
    

