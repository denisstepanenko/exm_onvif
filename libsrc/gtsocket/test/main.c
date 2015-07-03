#include <stdio.h>
#include <gtsocket.h>
int main(int argc,char *argv[])
{
	SOCK_FD fd,fd2;
	struct sockaddr addr;
	int addrlen=sizeof(addr);
	unsigned char buf[100];
	int ret;
#if 0
	fd=tcp_connect_addr("192.168.0.60",23,10);


//zw-test
	if((int)fd<0)
	{
		printf("tcp connect error\n");
	}
//zw-test 
#endif

#if 1
	fd=create_tcp_listen_port(INADDR_ANY,7200);
	if(fd==INVALID_SOCKET)
	{
		printf("can't connect remote!!\n");
		return(1);
	}
	listen(fd,10);
	fd2=accept(fd,&addr,&addrlen);
	printf("remote connect fd=%d!!\n",fd);
#endif
	while(1)
	{
		ret=net_read_buf(fd,buf,1);
		if(ret>0)
			printf("%02x ",(int)buf[0]);
		else
		{
			printf("ret=%d!!!\n",ret);
			break;
		}
	}
	return 0;
}
