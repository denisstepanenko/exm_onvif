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

#include "audioout_api.h"
//#include "leds.h"

int main(int argc, char *argv[])
{
	int ret;
	int state;

	//state = atoi(argv[1]);
	
	ret = init_audioout();
	if(ret < 0)
	{
		printf("init_audioout err!\n");
		return -1;
	}

	while(1)
	{
		scanf("%d",&state);
		printf("choose enable %d\n",state);
		ret = choose_enable(state);
		if(ret < 0)
		{
			printf("choose_enable err at %d\n",state);
		}
		sleep(1);
	}
	
	return 0;

}


