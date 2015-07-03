#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "netinfo.h"
#include <linux/sockios.h>
#include <sys/ioctl.h>


#define LINKTEST_GLINK 0x0000000a
struct linktest_value {
        unsigned int    cmd;
        unsigned int    data;
};
#if 0
#define get_ip_tempfile  "/var/tmp/ip"
static in_addr_t get_net_info(char *dev,char *info)
{
	int ipfile;
        int len,slen;
	in_addr_t ip;
	char *p,*t;
	char cmdbuf[100];
	if((dev==NULL)||(info==NULL))
		return -1;
	memset((char*)&ip,0,sizeof(in_addr_t));
	sprintf(cmdbuf,"ifconfig %s 2>/dev/zero |grep %s >%s\n",dev,info,get_ip_tempfile);
	system(cmdbuf);
	ipfile=open(get_ip_tempfile,O_RDONLY|O_NONBLOCK,0640);
	if(ipfile<0)
                return -1;
	len=read(ipfile,cmdbuf,100);
        if(len<0)
                return -1;
	p=strstr(cmdbuf,info);
	if(p==NULL)
                return 0;	
	slen=strlen(info);
	if(slen>10)
		return -1;
	p=index(p,info[slen-1]);
	if(p==NULL)
            return -1;
	p++;
	t=index(p,' ');
        if(t==NULL)
	{
	    t=index(p,'\r');
	    if(t==NULL)
	    {
		t=index(p,'\n');
		if(t==NULL)
			return -1;
	    }
	}
        *t='\0';
	ip=inet_addr(p);
	close(ipfile);
	return ip;	
}
#endif

#if 0
in_addr_t get_net_dev_ip(char *dev)
{
	return get_net_info(dev,"addr:");
}
in_addr_t get_net_dev_mask(char *dev)
{
        return get_net_info(dev,"Mask:");
}

in_addr_t get_dev_ip(char *dev)
{
	int ipfile;
	int len;
	char cmdbuf[100];
	char *p,*t;
	in_addr_t ip;
	sprintf(cmdbuf,"ifconfig %s |grep addr: >%s\n",dev,get_ip_tempfile);
	system(cmdbuf);
	ipfile=open(get_ip_tempfile,O_RDONLY|O_NONBLOCK,0640);
	if(ipfile<0)
		return -1;
	len=read(ipfile,cmdbuf,100);
	if(len<0)
		return -1;
	p=strstr(cmdbuf,"addr:");
	if(p==NULL)
		return -1;
	p=index(p,':');
	if(p==NULL)
	    return -1;
	p++;
	t=index(p,' ');
	if(t==NULL)
		return -1;
	*t='\0';
	ip=inet_addr(p);
	close(ipfile);
	return ip;
}
#endif



#include <net/if.h>  
#include <netinet/in.h>  
#include <net/if_arp.h>  
#include <sys/ioctl.h>

static int info_fd=-1;//系统网络信息描述符
int get_net_dev_info(char *dev,int cmd,struct ifreq *req)
{//info_fd打开后不会关闭
	#define MAX_NET_IF	32//最多32个网络设备接口
	struct ifreq ifr[MAX_NET_IF];
	struct ifconf ifc;
	int  i,rc,ifs;
	int fd;

	if((dev==NULL)||(req==NULL))
	{
		return -1;
	}
	if(info_fd<0)
		info_fd = socket( AF_INET, SOCK_DGRAM, 0 );
	fd=info_fd;
	if(fd<0)
		return -2;
	ifc.ifc_len = sizeof (ifr);
	ifc.ifc_req = ifr;
	rc = ioctl( fd, SIOCGIFCONF, &ifc );
	
	if(rc<0)
	{
		perror("get_net_dev_info\n");
		//close(fd);
		return -3;
	}
	ifs = ifc.ifc_len / sizeof (struct ifreq); 
	for(i=0;i<ifs;i++)
	{
		if(memcmp(ifr[i].ifr_name,dev,strlen(ifr[i].ifr_name))==0)
		{
			rc = ioctl( fd, cmd, &ifr[i] );
			memcpy((void*)req,(void*)&ifr[i],sizeof(struct ifreq));
			//close(fd);
			return 0;
		}
	}
	//close(fd);
	return -4;//没有找到	
}

in_addr_t get_net_dev_ip(char *dev)
{
	int rc;
	struct ifreq ifr;
	struct sockaddr_in *addr;
	in_addr_t ip;
	rc=get_net_dev_info(dev,SIOCGIFADDR,&ifr);
	if(rc<0)
		return -1;
	addr=(struct sockaddr_in*)(&ifr.ifr_addr);
		//->sin_addr)
	memcpy((void*)&ip,(void*)&addr->sin_addr,sizeof(in_addr_t));
	return ip;
}

in_addr_t get_net_dev_mask(char *dev)
{
	int rc;
	struct ifreq ifr;
	struct sockaddr_in *addr;
	in_addr_t mask;
	rc=get_net_dev_info(dev,SIOCGIFNETMASK,&ifr);
	if(rc<0)
		return -1;
	addr=(struct sockaddr_in*)(&ifr.ifr_netmask);
		//->sin_addr)
	memcpy((void*)&mask,(void*)&addr->sin_addr,sizeof(in_addr_t));
	return mask;	
}

int get_net_dev_mac(char *dev,unsigned char *buf)
{
	int rc;
	struct ifreq ifr;
	if((dev==NULL)||(buf==NULL))
		return -1;
	memset(buf,0xff,6);
	rc=get_net_dev_info(dev,SIOCGIFHWADDR,&ifr);
	if(rc<0)
		return -1;
	buf[0]=ifr.ifr_hwaddr.sa_data[5];
	buf[1]=ifr.ifr_hwaddr.sa_data[4];
	buf[2]=ifr.ifr_hwaddr.sa_data[3];
	buf[3]=ifr.ifr_hwaddr.sa_data[2];
	buf[4]=ifr.ifr_hwaddr.sa_data[1];
	buf[5]=ifr.ifr_hwaddr.sa_data[0];
	return 0;
}

//检测网线的连接状态 0表示没有连接 1表示连接上了 负值表示出错
int get_link_stat(char * dev)
{
        struct ifreq ifr;
        struct linktest_value edata;
        int fd;
	int ret=0;
        /* setup our control structures. */
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, dev);

        /* open control socket. */
        fd=socket(AF_INET, SOCK_DGRAM, 0);
        if(fd < 0 ) {
                return -1;
        }

        edata.cmd = LINKTEST_GLINK;
        ifr.ifr_data = (caddr_t)&edata;

        if(!ioctl(fd, SIOCETHTOOL, &ifr)) 
	{
               if(edata.data) {
			ret=1;///有连接
                }
 		else 
		{
			ret=0;//无连接
                }
        }
	else
		ret=-2;
	close(fd);
        return ret;
}

#if 0

int getip(void)
{
	struct ifreq ifr[5];
	struct ifconf ifc;
	int num, i,rc;
	int fd = socket( AF_INET, SOCK_DGRAM, 0 );
	ifc.ifc_len = sizeof ifr;
	ifc.ifc_req = ifr;
	rc = ioctl( fd, SIOCGIFCONF, &ifc );
	num = ifc.ifc_len / sizeof (struct ifreq); 
	for( i = 0; i < num; i++){
		printf ("net device %s\n", ifr[i].ifr_name);  

	    if (!(ioctl (fd, SIOCGIFFLAGS, (char *) &ifr[i]))) {  
               if (ifr[i].ifr_flags & IFF_PROMISC) 
		{  
                  printf("the interface is PROMISC");  
                  //retn++;  
               }  
            } else {  
               printf ("cpm: ioctl device %s", ifr[i].ifr_name);  
            }  

/*Jugde whether the net card status is up       */  
            if (ifr[i].ifr_flags & IFF_UP) {  
                printf("the interface status is UP\n");  
               }  
            else {  
                printf("the interface status is DOWN\n");  
            }  
/*Get IP of the net card */  
            if (!(ioctl (fd, SIOCGIFADDR, (char *) &ifr[i])))  
                {  
                 printf ("IP address is:");  
                 printf ("%s\n",inet_ntoa(((struct sockaddr_in*)(&ifr[i].ifr_addr))->sin_addr));  
                   //puts (buf[intrface].ifr_addr.sa_data);  
                }  
            else {  
               printf ("cpm: ioctl device %s", ifr[i].ifr_name);  
           }  


/*Get HW ADDRESS of the net card */  
            if (!(ioctl (fd, SIOCGIFHWADDR, (char *) &ifr[i])))  
                {  
                 printf("HW address is:");  

                 printf("%02x:%02x:%02x:%02x:%02x:%02x\n",  
                                (unsigned char)ifr[i].ifr_hwaddr.sa_data[0],  
                                (unsigned char)ifr[i].ifr_hwaddr.sa_data[1],  
                                (unsigned char)ifr[i].ifr_hwaddr.sa_data[2],  
                                (unsigned char)ifr[i].ifr_hwaddr.sa_data[3],  
                                (unsigned char)ifr[i].ifr_hwaddr.sa_data[4],  
                                (unsigned char)ifr[i].ifr_hwaddr.sa_data[5]);  

                   
                 printf("\n");  
                }  

            else {  
               printf ("cpm: ioctl device %s", ifr[i].ifr_name);  
           }  


	}
/* process each ifr[i] */
}



#endif










#if 0 
int main(void)
{
	int ret,i;
	char *addr;
	printf("ip= %08x\n",get_net_dev_ip("eth0"));
	printf("mask= %08x\n",get_net_dev_mask("eth0"));
	return 0;
}
#endif
