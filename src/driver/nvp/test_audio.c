#include <stdio.h>
#include "nvp1918.h"
int main(void)
{
	int fd=-1;	
	int ret;
	vdec_audio_volume_parm parm;
	memset(&parm,0,sizeof(vdec_audio_volume_parm));
	parm.AIN_0=15;
	parm.AIN_1=15;
	parm.AIN_2=15;
	parm.AIN_3=15;
	parm.AOUT=15;	
	fd = open("/dev/nc_vdec", 0);
	if(fd<0)
	{
		printf("open /dev/gpioi2c error\n");
		return ;
	}
	ret=ioctl(fd,IOC_VDEC_SET_AUDIO_PARM,(unsigned long*)&parm);
	if(ret<0)
	{
			printf("ioctl error\n");
			close(fd);
	}

	close(fd);
	return ;

}
