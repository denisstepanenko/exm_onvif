#include "audioout_api.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <asm/ioctl.h>
static int audioout_fd = -1;
	
int init_audioout(void)
{
	audioout_fd=open("/dev/audioout",O_RDWR);
	if(audioout_fd<0)
	{
		printf("can't open audioout device !\n");
		return -1;
	}
	else
	{
		#ifdef SHOW_WORK_INFO
			printf("open audioout device success=%d.\n",audioout_fd);	
		#endif
	}
	return 0;
}

char zero = 0;
char one = 1;

int choose_enable(int ch)
{
	switch(ch)
	{
	case 0:
		//全不选通
		ioctl(audioout_fd,SET_0,&zero);
		ioctl(audioout_fd,SET_1,&zero);
		ioctl(audioout_fd,SET_2,&zero);
		break;
	case 1:
		ioctl(audioout_fd,SET_0,&one);
		ioctl(audioout_fd,SET_1,&zero);
		ioctl(audioout_fd,SET_2,&zero);
		break;
	case 2:
		ioctl(audioout_fd,SET_0,&zero);
		ioctl(audioout_fd,SET_1,&one);
		ioctl(audioout_fd,SET_2,&zero);
		break;
	case 3:
		ioctl(audioout_fd,SET_0,&one);
		ioctl(audioout_fd,SET_1,&one);
		ioctl(audioout_fd,SET_2,&zero);
		break;
	case 4:
		ioctl(audioout_fd,SET_0,&zero);
		ioctl(audioout_fd,SET_1,&zero);
		ioctl(audioout_fd,SET_2,&one);
		break;
	case 5:
		ioctl(audioout_fd,SET_0,&one);
		ioctl(audioout_fd,SET_1,&zero);
		ioctl(audioout_fd,SET_2,&one);
		break;
	case 6:
		ioctl(audioout_fd,SET_0,&zero);
		ioctl(audioout_fd,SET_1,&one);
		ioctl(audioout_fd,SET_2,&one);
		break;
	case 7:
		ioctl(audioout_fd,SET_0,&one);
		ioctl(audioout_fd,SET_1,&one);
		ioctl(audioout_fd,SET_2,&one);
		break;	
	default:
		break;
	}
	
	return 0;
	
}
