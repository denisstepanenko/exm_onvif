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
#include "http.h"
#include "onvif.h"
#include "http_srv.h"
#include "onvif_event.h"

/***************************************************************************************/
unsigned long http_cln_index(HTTPSRV * p_srv, HTTPCLN * p_cln)
{
	return pps_get_index(p_srv->cln_fl, p_cln);
}

HTTPCLN * http_get_cln_by_index(HTTPSRV * p_srv, unsigned long index)
{
	return (HTTPCLN *)pps_get_node_by_index(p_srv->cln_fl, index);
}

HTTPCLN * http_get_idle_cln(HTTPSRV * p_srv)
{	
	HTTPCLN * p_cln = (HTTPCLN *)ppstack_fl_pop(p_srv->cln_fl);
	if(p_cln)
	{
		memset(p_cln, 0, sizeof(HTTPCLN));
		p_cln->guid = p_srv->guid++;
	}

	return p_cln;
}

void http_free_used_cln(HTTPSRV * p_srv, HTTPCLN * p_cln)
{	
	if(p_cln->dyn_recv_buf)
		free(p_cln->dyn_recv_buf);
	if(p_cln->cfd > 0)
	{
		closesocket(p_cln->cfd);
		p_cln->cfd = 0;
	}
	
	ppstack_fl_push_tail(p_srv->cln_fl, p_cln);
}

HTTPCLN * rpc_find_cln(HTTPSRV * p_srv, unsigned int rip, unsigned short rport)
{
	HTTPCLN * p_find = NULL;
	HTTPCLN * p_cln = (HTTPCLN *)pps_lookup_start(p_srv->cln_ul);
	while(p_cln)
	{
		if(p_cln->rip == rip && p_cln->rport == rport)
		{
			p_find = p_cln;
			break;
		}
		pps_lookup_next(p_srv->cln_ul, p_cln);
	}
	pps_lookup_end(p_srv->cln_ul);

	return p_find;
}

void http_soap_process(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
    char * p_xml;
	XMLN * p_node;
	XMLN * p_body;

	p_xml = get_http_ctt(rx_msg);
	if (NULL == p_xml)
	{
		log_print("http_soap_process::get_http_ctt ret null!!!\r\n");
		return;
	}

	log_print("http_soap_process::rx xml:\r\n%s\r\n", p_xml);

	p_node = xxx_hxml_parse(p_xml, strlen(p_xml));
	if (NULL == p_node || NULL == p_node->name)
	{
		log_print("http_soap_process::xxx_hxml_parse ret null!!!\r\n");
		return;
	}
	
	if (soap_strcmp(p_node->name, "Envelope") != 0)
	{
		log_print("http_soap_process::node name[%s] != [s:Envelope]!!!\r\n", p_node->name);
		xml_node_del(p_node);
		return;
	}
	
	p_body = xml_node_soap_get(p_node, "Body");
	if (NULL == p_body)
	{
		log_print("http_soap_process::xml_node_soap_get[s:Body] ret null!!!\r\n");
		xml_node_del(p_node);
		return;
	}

	if (NULL == p_body->f_child)
	{
		log_print("http_soap_process::body first child node is null!!!\r\n");
	}	
	else if (NULL == p_body->f_child->name)
	{
		log_print("http_soap_process::body first child node name is null!!!\r\n");
	}	
	else
	{
		log_print("http_soap_process::body first child node name[%s].\r\n", p_body->f_child->name);
	}

	if (soap_strcmp(p_body->f_child->name, "Notify") == 0)
	{
	    XMLN * p_Notify = xml_node_soap_get(p_body, "Notify");
	    assert(p_Notify);

		onvif_event_notify(p_Notify);
	}

	xml_node_del(p_node);
}

/***************************************************************************************/
void http_commit_rx_msg(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	http_soap_process(p_user, rx_msg);	

	closesocket(p_user->cfd);
	p_user->cfd = 0;
}

BOOL http_srv_tcp_rx(HTTPCLN * p_user)
{
	int rlen;
	HTTPMSG * rx_msg;
	
	if(p_user->p_rbuf == NULL)
	{
		p_user->p_rbuf = p_user->rcv_buf;
		p_user->mlen = sizeof(p_user->rcv_buf)-1;
		p_user->rcv_dlen = 0;
		p_user->ctt_len = 0;
		p_user->hdr_len = 0;
	}

	rlen = recv(p_user->cfd,p_user->p_rbuf+p_user->rcv_dlen,p_user->mlen-p_user->rcv_dlen,0);
	if(rlen <= 0)
	{
		log_print("http_tcp_rx::recv return = %d, dlen[%d], mlen[%d]\r\n",rlen,p_user->rcv_dlen,p_user->mlen);
		closesocket(p_user->cfd);
		p_user->cfd = 0;
		return FALSE;
	}

	p_user->rcv_dlen += rlen;
	p_user->p_rbuf[p_user->rcv_dlen] = '\0';

rx_analyse_point:

	if(p_user->rcv_dlen < 16)
		return TRUE;

	if(is_http_msg(p_user->p_rbuf) == FALSE)
		return FALSE;

	rx_msg = NULL;

	if(p_user->hdr_len == 0)
	{
		int http_pkt_len;
		int parse_len;

		http_pkt_len = http_pkt_find_end(p_user->p_rbuf);
		if(http_pkt_len == 0)
		{
			return TRUE;
		}
		p_user->hdr_len = http_pkt_len;

		rx_msg = http_get_msg_buf();
		if(rx_msg == NULL)
		{
			log_print("http_tcp_rx::get_msg_buf ret null!!!\r\n");
			return FALSE;
		}

		memcpy(rx_msg->msg_buf,p_user->p_rbuf,http_pkt_len);
		rx_msg->msg_buf[http_pkt_len] = '\0';
		log_print("RX >> %s\r\n\r\n",rx_msg->msg_buf);

		parse_len = http_msg_parse_part1(rx_msg->msg_buf,http_pkt_len,rx_msg);
		if(parse_len != http_pkt_len)
		{
			log_print("http_tcp_rx::http_msg_parse_part1=%d, sip_pkt_len=%d!!!\r\n",parse_len,http_pkt_len);
			free_http_msg(rx_msg);
			return FALSE;
		}
		p_user->ctt_len = rx_msg->ctt_len;
	}

	if((p_user->ctt_len + p_user->hdr_len) > p_user->mlen)
	{
		if(p_user->dyn_recv_buf)
		{
			log_print("http_tcp_rx::dyn_recv_buf=%p, mlen=%d!!!\r\n",p_user->dyn_recv_buf, p_user->mlen);
			free(p_user->dyn_recv_buf);
		}

		p_user->dyn_recv_buf = (char *)malloc(p_user->ctt_len + p_user->hdr_len + 1);
		memcpy(p_user->dyn_recv_buf, p_user->rcv_buf, p_user->rcv_dlen);
		p_user->p_rbuf = p_user->dyn_recv_buf;
		p_user->mlen = p_user->ctt_len + p_user->hdr_len;

		if(rx_msg) free_http_msg(rx_msg);
		return TRUE;
	}

	if(p_user->rcv_dlen >= (p_user->ctt_len + p_user->hdr_len))
	{
		if(rx_msg == NULL)
		{
			int nlen;
			int parse_len;
			
			nlen = p_user->ctt_len + p_user->hdr_len;
			if(nlen > 2048)
			{
				rx_msg = http_get_msg_large_buf(nlen+1);
				if(rx_msg == NULL)
					return FALSE;
			}
			else
			{
				rx_msg = http_get_msg_buf();
				if(rx_msg == NULL)
					return FALSE;
			}

			memcpy(rx_msg->msg_buf,p_user->p_rbuf,p_user->hdr_len);
			rx_msg->msg_buf[p_user->hdr_len] = '\0';
			log_print("RX >> %s\r\n\r\n",rx_msg->msg_buf);

			parse_len = http_msg_parse_part1(rx_msg->msg_buf,p_user->hdr_len,rx_msg);
			if(parse_len != p_user->hdr_len)
			{
				log_print("http_tcp_rx::http_msg_parse_part1=%d, sip_pkt_len=%d!!!\r\n",parse_len,p_user->hdr_len);
				free_http_msg(rx_msg);
				return FALSE;
			}

		}

		if(p_user->ctt_len > 0)
		{
			int parse_len;
			
			memcpy(rx_msg->msg_buf+p_user->hdr_len, p_user->p_rbuf+p_user->hdr_len, p_user->ctt_len);
			rx_msg->msg_buf[p_user->hdr_len + p_user->ctt_len] = '\0';
			log_print("%s\r\n\r\n",rx_msg->msg_buf+p_user->hdr_len);

			parse_len = http_msg_parse_part2(rx_msg->msg_buf+p_user->hdr_len,p_user->ctt_len,rx_msg);
			if(parse_len != p_user->ctt_len)
			{
				log_print("http_tcp_rx::http_msg_parse_part2=%d, sdp_pkt_len=%d!!!\r\n",parse_len,p_user->ctt_len);
				free_http_msg(rx_msg);
				return FALSE;
			}
		}

		http_commit_rx_msg(p_user, rx_msg);
		p_user->rcv_dlen -= p_user->hdr_len + p_user->ctt_len;

		if(p_user->dyn_recv_buf == NULL)
		{
			if(p_user->rcv_dlen > 0)
			{
				memmove(p_user->rcv_buf, p_user->rcv_buf+p_user->hdr_len + p_user->ctt_len, p_user->rcv_dlen);
				p_user->rcv_buf[p_user->rcv_dlen] = '\0';
			}
			p_user->p_rbuf = p_user->rcv_buf;
			p_user->mlen = sizeof(p_user->rcv_buf)-1;
			p_user->hdr_len = 0;
			p_user->ctt_len = 0;

			if(p_user->rcv_dlen > 16)
				goto rx_analyse_point;
		}
		else
		{
			free(p_user->dyn_recv_buf);
			p_user->dyn_recv_buf = NULL;
			p_user->hdr_len = 0;
			p_user->ctt_len = 0;
			p_user->p_rbuf = 0;
			p_user->rcv_dlen = 0;
		}
	}

    if (rx_msg)
        free_http_msg(rx_msg);
    
    return TRUE;
}

int http_tcp_listen_rx(HTTPSRV * p_srv)
{
	struct sockaddr_in caddr;
	socklen_t size = sizeof(struct sockaddr_in);
	int cfd;
	HTTPCLN * p_cln;

	cfd = accept(p_srv->sfd, (struct sockaddr *)&caddr, &size);
	if(cfd < 0)
	{
		perror("http_tcp_listen_rx::accept");
		return -1;
	}

	p_cln = http_get_idle_cln(p_srv);
	if(p_cln == NULL)
	{
		printf("http_get_idle_cln::ret null!!!\r\n");
		closesocket(cfd);
		return -1;
	}

	p_cln->cfd = cfd;
	p_cln->rip = caddr.sin_addr.s_addr;
	p_cln->rport = ntohs(caddr.sin_port);

	pps_ctx_ul_add(p_srv->cln_ul, p_cln);

	printf("http user over tcp from[0x%08x,%u]\r\n", p_cln->rip, p_cln->rport);

	return 0;
}

void * http_rx_thread(void * argv)
{
	fd_set fdr;
	HTTPSRV * p_srv = (HTTPSRV *)argv;
	if(p_srv == NULL)
		return NULL;

	printf("http_rx_thread start.\r\n");

	while(p_srv->r_flag == 1)
	{
		int sret;
		int max_fd;
		HTTPCLN * p_cln;
		struct timeval tv;
		
		FD_ZERO(&fdr);		
		FD_SET(p_srv->sfd, &fdr);

		max_fd = p_srv->sfd;
		
		p_cln = (HTTPCLN *)pps_lookup_start(p_srv->cln_ul);
		while(p_cln)
		{
			if(p_cln->cfd > 0)
			{
				FD_SET(p_cln->cfd, &fdr);
				max_fd = (p_cln->cfd > max_fd)? p_cln->cfd : max_fd;
			}
			else
			{
				HTTPCLN * p_next_cln = (HTTPCLN *)pps_ctx_ul_del_unlock(p_srv->cln_ul, p_cln);

                http_free_used_cln(p_srv, p_cln);
                p_cln = p_next_cln;
                continue;
			}

			p_cln = (HTTPCLN *)pps_lookup_next(p_srv->cln_ul, p_cln);
		}
		pps_lookup_end(p_srv->cln_ul);
		
		tv.tv_sec = 0;
		tv.tv_usec = 100 * 1000;
		
		sret = select(max_fd+1, &fdr,NULL,NULL,&tv);
		if(sret == 0)
		{
		//	printf("http_rx_thread::select err[%s], max fd[%d] timeout!!!\r\n", sys_os_get_socket_error(), max_fd);
			continue;
		}
		else if(sret < 0)
		{
			printf("http_rx_thread::select err[%s], max fd[%d], sret[%d]!!!\r\n", sys_os_get_socket_error(), max_fd, sret);
			break;
		}

		if(FD_ISSET(p_srv->sfd, &fdr))
		{
			http_tcp_listen_rx(p_srv);
		}
		
		p_cln = (HTTPCLN *)pps_lookup_start(p_srv->cln_ul);
		while(p_cln)
		{
			if(p_cln->cfd > 0 && FD_ISSET(p_cln->cfd, &fdr))
			{
				// printf("FD_ISSET cfd=%d.\r\n", p_cln->cfd);
				
				if(http_srv_tcp_rx(p_cln) == FALSE)
				{
					HTTPCLN * p_next_cln = (HTTPCLN *)pps_ctx_ul_del_unlock(p_srv->cln_ul, p_cln);

                    http_free_used_cln(p_srv, p_cln);
                    
					p_cln = p_next_cln;
					continue;
				}
			}
			else if (p_cln->cfd == 0)
			{
			    HTTPCLN * p_next_cln = (HTTPCLN *)pps_ctx_ul_del_unlock(p_srv->cln_ul, p_cln);

                http_free_used_cln(p_srv, p_cln);
                
			    p_cln = p_next_cln;
			    continue;
			}

			p_cln = (HTTPCLN *)pps_lookup_next(p_srv->cln_ul, p_cln);
		}
		pps_lookup_end(p_srv->cln_ul);
	}

	p_srv->rx_tid = 0;
	
	printf("http_rx_thread exit.\r\n");

	return NULL;
}

int http_srv_net_init(HTTPSRV * p_srv)
{
	int val = 1;
	int reuse_ret;
	struct sockaddr_in addr;
	
	p_srv->sfd = socket(AF_INET,SOCK_STREAM,0);
	if(p_srv->sfd < 0)
	{
		log_print("http_srv_net_init::socket err[%s]!!!\r\n",sys_os_get_socket_error());
		return -1;
	}

	reuse_ret = setsockopt(p_srv->sfd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, 4);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = p_srv->saddr;
	addr.sin_port = htons(p_srv->sport);
	
	if(bind(p_srv->sfd,(struct sockaddr *)&addr,sizeof(addr)) == -1)
	{
		log_print("http_srv_net_init::bind tcp socket fail,err[%s]!!!\n",sys_os_get_socket_error());
		closesocket(p_srv->sfd);
		p_srv->sfd = -1;
		return -1;
	}

	if(listen(p_srv->sfd, 10) < 0)
	{
		log_print("rpc_srv_net_init::listen tcp socket fail,err[%s]!!!\r\n",sys_os_get_socket_error());
		closesocket(p_srv->sfd);
		return -1;
	}
	
	return 0;
}

int http_srv_init(HTTPSRV * p_srv, unsigned int saddr, unsigned short sport, int cln_num)
{
	memset(p_srv, 0, sizeof(HTTPSRV));

	p_srv->saddr = saddr;
	p_srv->sport = sport;

	p_srv->cln_fl = pps_ctx_fl_init(cln_num, sizeof(HTTPCLN), TRUE);
	if(p_srv->cln_fl == NULL) 
		return -1;

	p_srv->cln_ul = pps_ctx_ul_init(p_srv->cln_fl, TRUE);
	if(p_srv->cln_ul == NULL)
		return -1;

	if(http_srv_net_init(p_srv) != 0)
		return -1;

	p_srv->r_flag = 1;
	p_srv->rx_tid = sys_os_create_thread((void *)http_rx_thread, p_srv);

	return 0;
}

void http_srv_deinit(HTTPSRV * p_srv)
{
	p_srv->r_flag = 0;

	while (p_srv->rx_tid != 0)
	{
		usleep(1000);
	}

    if (p_srv->cln_ul)
    {
	    pps_ul_free(p_srv->cln_ul);
	    p_srv->cln_ul = NULL;
	}
	
	if (p_srv->cln_fl)
	{
	    pps_fl_free(p_srv->cln_fl);
	    p_srv->cln_fl = NULL;
	}

    if (p_srv->sfd > 0)
    {
    	closesocket(p_srv->sfd);
    	p_srv->sfd = -1;
	}
}




