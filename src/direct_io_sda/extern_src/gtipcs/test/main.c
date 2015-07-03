#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "mshm.h"




int main(void)
{
	int ret;
	MEM_CORE *mc;
	mc=MShmCoreCreate("test share mem",1026,102400,NULL);
	printf("MmCoreCreate mc=%x:%s\n",(int)mc,strerror(errno));
	while(1)
	{
		//ret=MShmLock(mc);
		//printf("MShmLock ret=%d:%s\n",ret,strerror(errno));
		//ret=MShmUnLock(mc);
		//printf("MShmUnLock ret=%d:%s\n",ret,strerror(errno));
		
		ret=MShmLock(mc);
		printf("MShmLock ret=%d:%s\n",ret,strerror(errno));
		//sleep(1);
	}
	exit(0);
}
