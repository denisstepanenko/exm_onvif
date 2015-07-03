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
#include "hxml.h"
#include "xml_node.h"
#include "onvif_probe.h"
#include "onvif.h"
#include "soap_parser.h"
#include "onvif_utils.h"


/***************************************************************************************/
#define MAX_PROBE_FD	8
#define MAX_PROBE_INTERVAL 10

/***************************************************************************************/
onvif_probe_cb g_probe_cb = 0;
void * g_probe_cb_data = 0;
pthread_t g_probe_thread = 0;
int g_probe_fd[MAX_PROBE_FD];
int g_probe_interval = 30;
BOOL g_probe_running = FALSE;
BOOL g_probe_once = FALSE;

/***************************************************************************************/
int onvif_probe_init(unsigned int ip)
{	
	int opt = 1;
	SOCKET fd;
	struct sockaddr_in addr;
	struct ip_mreq mcast;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
	{
		log_print("socket SOCK_DGRAM error!\n");
		return -1;
	}
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3702);
    addr.sin_addr.s_addr = ip;
    
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        // if port 3702 already occupied, only receive unicast message
        addr.sin_port = 0;
        if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        {
            closesocket(fd);
            log_print("bind error! %s\n", sys_os_get_socket_error());
    		return -1;
		}
    }
    
	/* reuse socket addr */  
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt))) 
    {  
        printf("setsockopt SO_REUSEADDR error!\n");
    }
                  
    memset(&mcast, 0, sizeof(mcast));
	mcast.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
	mcast.imr_interface.s_addr = ip;

    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0)
	{
#if __WIN32_OS__
	    if(setsockopt(fd, IPPROTO_IP, 5, (char*)&mcast, sizeof(mcast)) < 0)
#endif	    
	    {
	        closesocket(fd);
    		log_print("setsockopt IP_ADD_MEMBERSHIP error! %s\n", sys_os_get_socket_error());
    		return -1;
		}
	}

	return fd;
}

char probe_req[] = 
	"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
	            "xmlns:a=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\">"
	"<s:Header>"
	"<a:Action s:mustUnderstand=\"1\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</a:Action>"
	"<a:MessageID>urn:uuid:%s</a:MessageID>"
	"<a:ReplyTo>"
	"<a:Address>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</a:Address>"
	"</a:ReplyTo>"
	"<a:To s:mustUnderstand=\"1\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</a:To>"
	"</s:Header>"
	"<s:Body>"
	"<Probe xmlns=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\">"
	"<d:Types xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
			 "xmlns:dp0=\"%s\">%s</d:Types>"
	"</Probe>"
	"</s:Body>"
	"</s:Envelope>";
			
int onvif_probe_req_tx(int fd)
{
	int len;
	int rlen;
	char send_buffer[1024 * 10];
	struct sockaddr_in addr;

	sprintf(send_buffer, probe_req, 
	    onvif_uuid_create(),
		"http://www.onvif.org/ver10/network/wsdl", 
		"dp0:NetworkVideoTransmitter");

	memset(&addr, 0, sizeof(addr));
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("239.255.255.250");
	addr.sin_port = htons(3702);

	len = strlen(send_buffer);
	rlen = sendto(fd, send_buffer, len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (rlen != len)
	{
		log_print("onvif_probe_req_tx::rlen = %d,slen = %d\r\n", rlen, len);
	}

    /* lc invalid for now
	sprintf(send_buffer, probe_req, 
	    onvif_uuid_create(),
		"http://www.onvif.org/ver10/device/wsdl", 
		"dp0:Device");
		
	len = strlen(send_buffer);
	rlen = sendto(fd, send_buffer, len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (rlen != len)
	{
		log_print("onvif_probe_req_tx::rlen = %d,slen = %d\r\n", rlen, len);
	}		
*/
	return rlen;
}


int onvif_probe_res(XMLN * p_node, DEVICE_BINFO * p_res)
{
    XMLN * p_body = xml_node_soap_get(p_node, "s:Body");
	if (p_body)
	{
		XMLN * p_ProbeMatches = xml_node_soap_get(p_body, "d:ProbeMatches");
		if (p_ProbeMatches)
		{
			XMLN * p_ProbeMatch = xml_node_soap_get(p_ProbeMatches, "d:ProbeMatch");
			if (p_ProbeMatch)
			{
				XMLN * p_EndpointReference;
				XMLN * p_Types;
				XMLN * p_XAddrs;

				p_EndpointReference = xml_node_soap_get(p_ProbeMatch, "EndpointReference");
				if (p_EndpointReference)
				{
					XMLN * p_Address = xml_node_soap_get(p_EndpointReference, "Address");
					if (p_Address && p_Address->data)
					{
						strncpy(p_res->EndpointReference, p_Address->data, sizeof(p_res->EndpointReference)-1);
					}
				}
				
                p_Types = xml_node_soap_get(p_ProbeMatch, "d:Types");
			    if (p_Types && p_Types->data)
			    {
			        p_res->type = parse_DeviceType(p_Types->data);
			    }
			    
			    p_XAddrs = xml_node_soap_get(p_ProbeMatch, "d:XAddrs");
			    if (p_XAddrs && p_XAddrs->data)
			    {
			        parse_XAddr(p_XAddrs->data, &p_res->xaddr);
			    }
			    else
				{
				    return -1;
				}

			    return 0;
			}
		}
		else
		{
		    XMLN * p_Hello = xml_node_soap_get(p_body, "d:Hello");
		    if (p_Hello)
		    {
		        XMLN * p_Types;
				XMLN * p_XAddrs;

				p_Types = xml_node_soap_get(p_Hello, "d:Types");
			    if (p_Types && p_Types->data)
			    {
			        p_res->type = parse_DeviceType(p_Types->data);
			    }
			    
			    p_XAddrs = xml_node_soap_get(p_Hello, "d:XAddrs");
			    if (p_XAddrs && p_XAddrs->data)
			    {
			        parse_XAddr(p_XAddrs->data, &p_res->xaddr);
			    }
			    else
				{
				    return -1;
				}

			    return 0;
		    }
		}
	}

	return -1;
}

int onvif_probe_net_rx()
{
	int i;
    int ret;
    int	maxfd = 0;
    int	fd = 0;
    char rbuf[65535];
    fd_set fdread;
    struct timeval tv = {1, 0};
    

    FD_ZERO(&fdread);
    memset(rbuf,0,65535);

    for (i = 0; i < MAX_PROBE_FD; i++)
    {
    	if (g_probe_fd[i] > 0)
    	{
    		FD_SET(g_probe_fd[i], &fdread); 

    		if (g_probe_fd[i] > maxfd)
    		{
    			maxfd = g_probe_fd[i];
    		}
    	}
    }
    
    ret = select(maxfd+1, &fdread, NULL, NULL, &tv); 
    if (ret == 0) // Time expired 
    { 
        return 0; 
    }

    for (i = 0; i < MAX_PROBE_FD; i++)
    {
    	if (g_probe_fd[i] > 0 && FD_ISSET(g_probe_fd[i], &fdread))
    	{
    		int rlen;
    		int addr_len;
    		struct sockaddr_in addr;
			unsigned int src_ip;
			unsigned int src_port;
			XMLN * p_node;
			
    		fd = g_probe_fd[i];
    		
			addr_len = sizeof(struct sockaddr_in);
			rlen = recvfrom(fd, rbuf, sizeof(rbuf), 0, (struct sockaddr *)&addr, (socklen_t*)&addr_len);
			if (rlen <= 0)
			{
			    log_print("onvif_probe_net_rx::rlen = %d, fd = %d\r\n", rlen, fd);
				continue;
		    }
		    
			src_ip = addr.sin_addr.s_addr;
			src_port = addr.sin_port;

			p_node = xxx_hxml_parse(rbuf, rlen);
			if (p_node == NULL)
			{
			    log_print("onvif_probe_net_rx::hxml parse err!!!\r\n", rlen, fd);
			}	
			else
			{
			    DEVICE_BINFO res;
			    memset(&res, 0, sizeof(DEVICE_BINFO));
			    
				if (onvif_probe_res(p_node, &res) >= 0)
				{
				    res.ip = src_ip;

				    if (g_probe_cb)
				    {
				        g_probe_cb(&res, g_probe_cb_data);
				    }
				}		
			}

			xml_node_del(p_node);
    	}
    }

	return 0;
}

void * onvif_probe_thread(void * argv)
{
    int count = 0;

	int i = 0;
	int j = 0;
	
    for (; i < get_if_nums() && j < MAX_PROBE_FD; i++, j++)
    {
    	unsigned int ip = get_if_ip(i);    	
        struct in_addr inip;
        inip.s_addr = ip;
    	if (ip != 0 && ip != inet_addr("127.0.0.1"))
    	{
            printf("probe use ip %s\n",inet_ntoa(inip));
    		g_probe_fd[j] = onvif_probe_init(ip);
    	}
    }

    //lc modify to support probe once
	if (g_probe_interval < 10)
	{
	    //g_probe_interval = 30;
        g_probe_interval = MAX_PROBE_INTERVAL;
	}

	for (i = 0; i < MAX_PROBE_FD; i++)
	{
		if (g_probe_fd[i] > 0)
		{
			onvif_probe_req_tx(g_probe_fd[i]);	
		}
	}
	
	while (g_probe_running)
	{
		onvif_probe_net_rx();

		if (++count >= g_probe_interval)
		{
		    count = 0;

            //lc add
            if(g_probe_once)
                break;

		    for (i = 0; i < MAX_PROBE_FD; i++)
			{
				if (g_probe_fd[i] > 0)
				{
					onvif_probe_req_tx(g_probe_fd[i]);	
				}
			}	    
		}

		usleep(1000);
	}

    for (i = 0; i < MAX_PROBE_FD; i++)
	{
		if (g_probe_fd[i] > 0)
		{
            close(g_probe_fd[i]);
            g_probe_fd[i] = -1;
		}
	}

    g_probe_thread = 0;
    g_probe_running = false;
    
	return NULL;
}

void set_probe_cb(onvif_probe_cb cb, void * pdata)
{
    g_probe_cb = cb;
    g_probe_cb_data = pdata;
}


void send_probe_req()
{
	int i;	
    for (i = 0; i < MAX_PROBE_FD; i++)
	{
		if (g_probe_fd[i] > 0)
		{
			onvif_probe_req_tx(g_probe_fd[i]);	
		}
	}
}


int start_probe(int interval,bool once)
{
    //lc add for multiple probe
    if(g_probe_running)
        return -1;

    g_probe_running = TRUE;
	g_probe_interval = interval;
    g_probe_once = once;
    
    g_probe_thread = sys_os_create_thread((void *)onvif_probe_thread, NULL);
    if (g_probe_thread)
    {
        return 0;
    }

    return -1;
}

void stop_probe()
{
	int i;
	
    g_probe_running = FALSE;

	for (i = 0; i < MAX_PROBE_FD; i++)
	{
		if (g_probe_fd[i] > 0)
		{
			closesocket(g_probe_fd[i]);
        	g_probe_fd[i] = 0;
		}
	}

    while (g_probe_thread)
    {
        usleep(100);
    }
}



