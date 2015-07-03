
#include "nvp1918.h"

main()
{
    int fd;
    fd = open("/dev/nc_vdec", 0);
    if (fd < 0)
    {
        printf("open nvp1914 fail\n");
        exit(1);
    }

    vdec_video_loss stat;

	int ch,chip = 0,ret;
	for(ch=0; ch<4; ch++)
	{
		stat.chip=chip;//第几个2867
		stat.ch=ch;		//2867的通道

		ret=ioctl(fd, IOC_VIDEO_GET_VIDEO_LOSS, &stat);
		if(ret<0)
		{
			printf("error in video loss ioctl,and exit\n");
			exit(1);
		}

		printf("lost:%d\n",stat.is_lost);///*丢失了*/

	}


}
