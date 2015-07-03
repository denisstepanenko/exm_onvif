/*¼ì²â²¢É±ËÀvsmain½ø³Ì*/
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "crc16.h"


#define    WATCH_SERIAL			"/dev/simcom"
#define QUERY_WATCH_BOARD_TIME	10//20	//..........
#define 	WATCHB_HEAD0	0x55				//....
#define	WATCHB_HEAD1	'@'
#define	WATCHB_TAIL	(('#'<<8)|0xaa)	//......
#define	CRC16_INIT_VAL	0			//CRC...

#define	REQUIRE_RESET			0x0004
#define BYTE unsigned char
#define WORD unsigned short
static int watch_fd=-1;
static int  send_to_watch(BYTE *msg,int msg_len)
{
	WORD pkt_head[2];
	WORD pkt_tail[2];
	WORD crcval;
	int i,rc;
	if(watch_fd<0)
		return -1;
	if(msg==NULL)
		return -1;
	pkt_head[0]=WATCHB_HEAD1<<8|WATCHB_HEAD0;
	pkt_head[1]=htons((msg_len));
	crcval=CRC16_INIT_VAL;

	crcval=UpdateCRC((pkt_head[1]&0xff),crcval);
	crcval=UpdateCRC((pkt_head[1]>>8)&0xff,crcval);
	for(i=0;i<msg_len;i++)
	{
		crcval=UpdateCRC(msg[i],crcval);		
	}
	pkt_tail[0]=htons(crcval);
	pkt_tail[1]=WATCHB_TAIL;
	write(watch_fd,(BYTE*)pkt_head,4);
	write(watch_fd,(BYTE*)msg,msg_len);
	rc=write(watch_fd,(BYTE *)pkt_tail,4);
	return rc;
}
int send_reboot_cmd_51(void)
{
	BYTE cmdbuf[20];
	WORD *cmd;
	WORD *rs;
	watch_fd=open(WATCH_SERIAL,O_RDWR|O_NOCTTY);
	if(watch_fd<0)
		printf("can't open %s!!!\n",WATCH_SERIAL);
	cmd=(WORD *)cmdbuf;
	rs=(WORD*)&cmdbuf[2];
	*cmd=htons(REQUIRE_RESET);	
	*rs=htons(0);
	return send_to_watch(cmdbuf,4);
}
#ifdef USE_VM
#include "../vm_iodrv/gtvm_io_api.h" 
int send_gtvm_reset_cmd(void)
{
	int ret;
	int vmio_fd;
	vmio_fd=open("/dev/vmio",O_WRONLY);
	if(vmio_fd<0)
	{
		printf("can't open vmio device !\n");
		return -1;
	}
	else
	{
		printf("open vmio device success=%d.\n",vmio_fd);	
	}
	ret=ioctl(vmio_fd,RESET_CMD,NULL);
	printf("ioctl RESET_CMD ret=%d!!\n",ret);
	sleep(1);
	close(vmio_fd);
	return ret;
}
#endif
int main(int argc,char **argv)
{
	system("killall -9 watch_proc\n");
	system("killall -9 vsmain\n");
	sleep(1);
	send_reboot_cmd_51();	
#ifdef USE_VM
	send_gtvm_reset_cmd();
#endif
	sleep(2);
	return 0;
}
