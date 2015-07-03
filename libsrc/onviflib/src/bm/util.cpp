/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2010-2014, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#include "sys_inc.h"
#include "word_analyse.h"
#include "util.h"

/***************************************************************************************/
int get_if_nums()
{
#if __WIN32_OS__

	char ipt_buf[512];
	MIB_IPADDRTABLE *ipt = (MIB_IPADDRTABLE *)ipt_buf;
	ULONG ipt_len = sizeof(ipt_buf);
	DWORD fr = GetIpAddrTable(ipt,&ipt_len,FALSE);
	if (fr != NO_ERROR)
		return 0;
		
	return ipt->dwNumEntries;
	
#elif __LINUX_OS__

	int socket_fd;
	struct ifconf conf;
	char buff[BUFSIZ];
	int num;
	
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd <= 0)
	{
		return 0;
	}
	
	conf.ifc_len = BUFSIZ;
	conf.ifc_buf = buff;
	
	ioctl(socket_fd, SIOCGIFCONF, &conf);
	
	num = conf.ifc_len / sizeof(struct ifreq);

	closesocket(socket_fd);
	
	return num;
	
#endif

	return 0;
}

unsigned int get_if_ip(int index)
{
#if __WIN32_OS__

	char ipt_buf[512];
	DWORD i;
	MIB_IPADDRTABLE *ipt = (MIB_IPADDRTABLE *)ipt_buf;
	ULONG ipt_len = sizeof(ipt_buf);
	DWORD fr = GetIpAddrTable(ipt,&ipt_len,FALSE);
	if (fr != NO_ERROR)
		return 0;

	for (i=0; i<ipt->dwNumEntries; i++)
	{
		if (i == index)
			return ipt->table[i].dwAddr;
	}
	
#elif __LINUX_OS__

	int i;
	int socket_fd;
	struct sockaddr_in *sin;
	struct ifreq *ifr;
	struct ifconf conf;
	char buff[BUFSIZ];
	int num;
	
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	conf.ifc_len = BUFSIZ;
	conf.ifc_buf = buff;
	
	ioctl(socket_fd, SIOCGIFCONF, &conf);
	
	num = conf.ifc_len / sizeof(struct ifreq);
	ifr = conf.ifc_req;

	unsigned int ip_addr = 0;
	
	for (i=0; i<num; i++)
	{
		if (i == index)
		{
			struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

			ioctl(socket_fd, SIOCGIFFLAGS, ifr);
			if ((ifr->ifr_flags & IFF_LOOPBACK) != 0)
			{
				ip_addr = 0;
			}
			else
			{
				ip_addr = sin->sin_addr.s_addr;
			}

			break;
		}
		
		ifr++;
	}

	closesocket(socket_fd);
	
	return ip_addr;
	
#endif

	return 0;
}

unsigned int get_route_if_ip(unsigned int dst_ip)
{
#if __WIN32_OS__

	DWORD i;
	DWORD dwIfIndex,fr;
	char ipt_buf[512];
	MIB_IPADDRTABLE *ipt;
	ULONG ipt_len;

	fr = GetBestInterface(dst_ip,&dwIfIndex);
	if (fr != NO_ERROR)
		return 0;
	
	ipt = (MIB_IPADDRTABLE *)ipt_buf;
	ipt_len = sizeof(ipt_buf);
	fr = GetIpAddrTable(ipt,&ipt_len,FALSE);
	if (fr != NO_ERROR)
		return 0;

	for (i=0; i<ipt->dwNumEntries; i++)
	{
		if (ipt->table[i].dwIndex == dwIfIndex)
			return ipt->table[i].dwAddr;
	}

#elif __VXWORKS_OS__

	char tmp_buf[24];
	char ifname[32];
	STATUS ret;
	
	sprintf(ifname,"%s%d",hsip.ifname,0);
	ret = ifAddrGet(ifname,tmp_buf);
	if (ret == OK)
	{
		return inet_addr(tmp_buf);
	}
	
#elif __LINUX_OS__

	int i;
	int socket_fd;
	struct sockaddr_in *sin;
	struct ifreq *ifr;
	struct ifconf conf;
	char buff[BUFSIZ];
	int num;
	
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	conf.ifc_len = BUFSIZ;
	conf.ifc_buf = buff;
	
	ioctl(socket_fd, SIOCGIFCONF, &conf);
	
	num = conf.ifc_len / sizeof(struct ifreq);
	ifr = conf.ifc_req;

	unsigned int ip_addr = 0;
	
	for (i=0; i<num; i++)
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

		ioctl(socket_fd, SIOCGIFFLAGS, ifr);
		if (((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
		{
			ip_addr = sin->sin_addr.s_addr;
			break;
		}
		
		ifr++;
	}

	closesocket(socket_fd);	
	return ip_addr;
	
#endif

	return 0;
}

unsigned int get_default_if_ip()
{
	return get_route_if_ip(0);
}

int get_default_if_mac(unsigned char * mac)
{
#if __WIN32_OS__

	IP_ADAPTER_INFO AdapterInfo[16];            // Allocate information for up to 16 NICs  
    DWORD dwBufLen = sizeof(AdapterInfo);       // Save the memory size of buffer  
  
    DWORD dwStatus = GetAdaptersInfo(           // Call GetAdapterInfo  
        AdapterInfo,                            // [out] buffer to receive data  
        &dwBufLen);                             // [in] size of receive data buffer  
  
    PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info  
	if (pAdapterInfo)
	{
        memcpy(mac, pAdapterInfo->Address, 6);	
        return 0;
    }  
    
#elif __LINUX_OS__

	int i;
	int socket_fd;
	struct sockaddr_in *sin;
	struct ifreq *ifr;
	struct ifreq ifreq;
	struct ifconf conf;
	char buff[BUFSIZ];
	int num;
	
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	conf.ifc_len = BUFSIZ;
	conf.ifc_buf = buff;
	
	ioctl(socket_fd, SIOCGIFCONF, &conf);
	
	num = conf.ifc_len / sizeof(struct ifreq);
	ifr = conf.ifc_req;
	
	for (i=0; i<num; i++)
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

		if (ifr->ifr_addr.sa_family != AF_INET)
		{
			ifr++;
			continue;
		}
		
		ioctl(socket_fd, SIOCGIFFLAGS, ifr);
		if ((ifr->ifr_flags & IFF_LOOPBACK) != 0)
		{
			ifr++;
			continue;
		}

		strncpy(ifreq.ifr_name, ifr->ifr_name, sizeof(ifreq.ifr_name));
        if (ioctl(socket_fd, SIOCGIFHWADDR, &ifreq) < 0) 
        {
        	ifr++;
            continue;
        }

		memcpy(mac, &ifreq.ifr_hwaddr.sa_data, 6);		

		close(socket_fd);
		return 0;
	}

	close(socket_fd);
#endif	

	return -1;
}

const char * get_default_gateway()
{   
	static char gateway[32];	
	
#if __WIN32_OS__

	IP_ADAPTER_INFO AdapterInfo[16];            // Allocate information for up to 16 NICs  
    DWORD dwBufLen = sizeof(AdapterInfo);       // Save the memory size of buffer  
  
    DWORD dwStatus = GetAdaptersInfo(           // Call GetAdapterInfo  
        AdapterInfo,                            // [out] buffer to receive data  
        &dwBufLen);                             // [in] size of receive data buffer  
  
    PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info  
	if (NULL == pAdapterInfo)
	{		
        return NULL;
    } 

	memset(gateway, 0, sizeof(gateway));
	
    strncpy(gateway, pAdapterInfo->GatewayList.IpAddress.String, sizeof(gateway)-1);

#elif __LINUX_OS__
	
    char line[100], *p, *c, *g, *saveptr;
	int ret = 0;
	
    FILE *fp = fopen("/proc/net/route" , "r");
	if (NULL == fp)
	{
		return NULL;
	}

	memset(gateway, 0, sizeof(gateway));
	
    while (fgets(line, 100, fp))
    {
        p = strtok_r(line, " \t", &saveptr);
        c = strtok_r(NULL, " \t", &saveptr);
        g = strtok_r(NULL, " \t", &saveptr);

        if (p != NULL && c != NULL)
        {
            if (strcmp(c, "00000000") == 0)
            {
                if (g)
                {
                    char *p_end;
                    int gw = strtol(g, &p_end, 16);
                    
                    struct in_addr addr;
                    addr.s_addr = gw;
                    
                    strcpy(gateway, inet_ntoa(addr));
                    ret = 1;
                }
                
                break;
            }
        }
    }

	fclose(fp);
	
    if (ret == 0)
    {
    	return NULL;
    }    

#endif

	return gateway;
}

const char * get_dns_server()
{
	static char dns[32];

#if __WIN32_OS__

	IP_ADAPTER_ADDRESSES addr[16], *paddr;
	DWORD len = sizeof(addr);

	memset(dns, 0, sizeof(dns));
	
	if (NO_ERROR == GetAdaptersAddresses(AF_INET, 0, 0, addr, &len) && len >= sizeof(IP_ADAPTER_ADDRESSES))
	{
		paddr = addr;
		while (paddr)
		{
			PIP_ADAPTER_DNS_SERVER_ADDRESS p_ipaddr;
				
			if (paddr->IfType & IF_TYPE_SOFTWARE_LOOPBACK)
			{
				paddr = paddr->Next;
				continue;
			}

			p_ipaddr = paddr->FirstDnsServerAddress;
			if (p_ipaddr)
			{
				struct sockaddr_in * p_inaddr = (struct sockaddr_in *)p_ipaddr->Address.lpSockaddr;
				strcpy(dns, inet_ntoa(p_inaddr->sin_addr));

				break;
			}	

			paddr = paddr->Next;
		}
	}	

#elif __LINUX_OS__

	memset(dns, 0, sizeof(dns));

	// todo : parese /etc/resolv.conf file to get dns server

	// just return 192.168.1.1
	strcpy(dns, "192.168.1.1");
	
#endif

	return dns;
}

const char * get_mask_by_prefix_len(int len)
{
	int i;
    static char mask_str[32] = {'\0'};    
    unsigned int mask = 0;
	
    for (i = 0; i < len; i++)
    {
        mask |= (1 << (31 - i));
    }

    memset(mask_str, 0, sizeof(mask_str));
    sprintf(mask_str, "%u.%u.%u.%u", (mask & 0xFF000000) >> 24, (mask & 0x00FF0000) >> 16, 
        (mask & 0x0000FF00) >> 8, (mask & 0x000000FF));

    return mask_str;    
}

int get_prefix_len_by_mask(const char *mask)
{
	int i;
	int len = 0;
    unsigned int n = inet_addr(mask);
	
    n = ntohl(n);    

    for (i = 0; i < 32; i++)
    {
        if (n & (1 << (31 - i)))
        {
            len++;
        }
        else
        {
            break;
        }
    }

    return len;
}

unsigned int get_address_by_name(char *host_name)
{
	unsigned int addr = 0;

	if (is_ip_address(host_name))
		addr = inet_addr(host_name);
	else
	{
		struct hostent * remoteHost = gethostbyname(host_name);
		if (remoteHost)
			addr = *(unsigned long *)(remoteHost->h_addr);
	}

	return addr;
}

char * lowercase(char *str) 
{
	unsigned int i;
	
	for (i = 0; i < strlen(str); ++i)
		str[i] = tolower(str[i]);
	
	return str;
}

char * uppercase(char *str)
{
	unsigned int i;
	
	for (i = 0; i < strlen(str); ++i)
		str[i] = toupper(str[i]);
	
	return str;
}

#define MIN(a, b)  ((a) < (b) ? (a) : (b))

int unicode(char **dst, char *src) 
{
	char *ret;
	int l, i;
	
	if (!src) 
	{
		*dst = NULL;
		return 0;
	}
	
	l = MIN(64, strlen(src));
	ret = (char *)XMALLOC(2*l);
	for (i = 0; i < l; ++i)
	{
		ret[2*i] = src[i];
		ret[2*i+1] = '\0';
	}

	*dst = ret;
	return 2*l;
}

static char hextab[17] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 0};
static int hexindex[128] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

char * printmem(char *src, size_t len, int bitwidth) 
{
	char *tmp;
	unsigned int i;
	
	tmp = (char *)XMALLOC(2*len+1);
	for (i = 0; i < len; ++i) 
	{
		tmp[i*2] = hextab[((unsigned char)src[i] ^ (unsigned char)(7-bitwidth)) >> 4];
		tmp[i*2+1] = hextab[(src[i] ^ (unsigned char)(7-bitwidth)) & 0x0F];
	}
	
	return tmp;
}

char * scanmem(char *src, int bitwidth) 
{
	int h, l, i, bytes;
	char *tmp;
	
	if (strlen(src) % 2)
		return NULL;
	
	bytes = strlen(src)/2;
	tmp = (char *)XMALLOC(bytes+1);
	for (i = 0; i < bytes; ++i) 
	{
		h = hexindex[(int)src[i*2]];
		l = hexindex[(int)src[i*2+1]];
		if (h < 0 || l < 0) 
		{
		        XFREE(tmp);
		        return NULL;
		}
		tmp[i] = ((h << 4) + l) ^ (unsigned char)(7-bitwidth);
	}
	tmp[i] = 0;

	return tmp;
}


time_t get_time_by_string(char * p_time_str)
{
	char * ptr_s;
	struct tm st;
	
	memset(&st, 0, sizeof(struct tm));

	ptr_s = p_time_str;
	while(*ptr_s == ' ' || *ptr_s == '\t') ptr_s++;

	sscanf(ptr_s, "%04d-%02d-%02d %02d:%02d:%02d", &st.tm_year, &st.tm_mon, &st.tm_mday, &st.tm_hour, &st.tm_min, &st.tm_sec);

	st.tm_year -= 1900;
	st.tm_mon -= 1;

	return mktime(&st);
}

int tcp_connect_timeout(unsigned int rip, int port, int timeout)
{
	int cfd;
	int flag = 0;
	struct sockaddr_in addr;
#if __LINUX_OS__	
	struct timeval tv;
#elif __WIN32_OS__
	unsigned long ul = 1;
#endif

	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if (cfd < 0)
	{
	    printf("tcp_connect_timeout socket failed\n");
		return -1;
    }    
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = rip;
	addr.sin_port = htons((unsigned short)port);

#if __LINUX_OS__
	
	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout%1000) * 1000;
	
	setsockopt(cfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));
	if (connect(cfd, (struct sockaddr *)&addr, sizeof(addr)) == 0)
	{
	    return cfd;
	}
	else
	{
	    closesocket(cfd);
	    return -1;
	}
	
#elif __WIN32_OS__    
    
	ioctlsocket(cfd, FIONBIO, &ul);

	if (connect(cfd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
	{
		fd_set set;
		struct timeval tv;
		
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000) * 1000;
		
		FD_ZERO(&set);
		FD_SET(cfd, &set);

		if (select(cfd+1, NULL, &set, NULL, &tv) > 0)
		{
			int err=0, len=sizeof(int);
			getsockopt(cfd, SOL_SOCKET, SO_ERROR, (char *)&err, (socklen_t*) &len);
			if (err == 0)
				flag = 1;
		}
	}
	else
	{
	    flag = 1;
	}

    ul = 0;
	ioctlsocket(cfd, FIONBIO, &ul);
		
	if (flag == 1)
	{
		return cfd;
	}
	else
	{
		closesocket(cfd);
		return -1;
	}

#endif	
}



