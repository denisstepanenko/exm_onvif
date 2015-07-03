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
#include "onvif_pkt.h"
#include "soap.h"
#include "http_cln.h"

/***************************************************************************************/
BOOL http_tcp_rx(HTTPREQ * p_user)
{
	int rlen;
	HTTPMSG * rx_msg;

	if (p_user->p_rbuf == NULL)
	{
		p_user->p_rbuf = p_user->rcv_buf;
		p_user->mlen = sizeof(p_user->rcv_buf)-1;
		p_user->rcv_dlen = 0;
		p_user->ctt_len = 0;
		p_user->hdr_len = 0;
	}

	rlen = recv(p_user->cfd,p_user->p_rbuf+p_user->rcv_dlen,p_user->mlen-p_user->rcv_dlen,0);
	if (rlen <= 0)
	{
		log_print("http_tcp_rx::recv return = %d, dlen[%d], mlen[%d]\r\n",rlen,p_user->rcv_dlen,p_user->mlen);	
		closesocket(p_user->cfd);
		p_user->cfd = 0;
		return FALSE;
	}

	p_user->rcv_dlen += rlen;
	p_user->p_rbuf[p_user->rcv_dlen] = '\0';

rx_analyse_point:

	if (p_user->rcv_dlen < 16)
		return TRUE;

	if (is_http_msg(p_user->p_rbuf) == FALSE)	
		return FALSE;

	rx_msg = NULL;

	if (p_user->hdr_len == 0)
	{
		int parse_len;
		int http_pkt_len;

		http_pkt_len = http_pkt_find_end(p_user->p_rbuf);
		if (http_pkt_len == 0) 
		{
			return TRUE;
		}
		p_user->hdr_len = http_pkt_len;

		rx_msg = http_get_msg_buf();
		if (rx_msg == NULL)
		{
			log_print("http_tcp_rx::get_msg_buf ret null!!!\r\n");
			return FALSE;
		}

		memcpy(rx_msg->msg_buf,p_user->p_rbuf,http_pkt_len);
		rx_msg->msg_buf[http_pkt_len] = '\0';
		//log_print("%s\r\n\r\n\r\n",rx_msg->msg_buf);

		parse_len = http_msg_parse_part1(rx_msg->msg_buf,http_pkt_len,rx_msg);
		if (parse_len != http_pkt_len)
		{
			log_print("http_tcp_rx::http_msg_parse_part1=%d, sip_pkt_len=%d!!!\r\n",parse_len,http_pkt_len);
			free_http_msg(rx_msg);
			return FALSE;
		}
		p_user->ctt_len = rx_msg->ctt_len;
	}

	if ((p_user->ctt_len + p_user->hdr_len) > p_user->mlen)
	{
		if (p_user->dyn_recv_buf)
		{
			free(p_user->dyn_recv_buf);
		}

		p_user->dyn_recv_buf = (char *)malloc(p_user->ctt_len + p_user->hdr_len + 1);
		if (NULL == p_user->dyn_recv_buf)
		{
		    if(rx_msg) free_http_msg(rx_msg);
		    return FALSE;
		}
		
		memcpy(p_user->dyn_recv_buf, p_user->rcv_buf, p_user->rcv_dlen);
		p_user->p_rbuf = p_user->dyn_recv_buf;
		p_user->mlen = p_user->ctt_len + p_user->hdr_len;

		if(rx_msg) free_http_msg(rx_msg);
		return TRUE;
	}

	if (p_user->rcv_dlen >= (p_user->ctt_len + p_user->hdr_len))
	{
		if (rx_msg == NULL)
		{
			int nlen;
			int parse_len;

			nlen = p_user->ctt_len + p_user->hdr_len;
			if (nlen >= 2048)
			{
				rx_msg = http_get_msg_large_buf(nlen+1);
				if (rx_msg == NULL)
					return FALSE;
			}
			else
			{
				rx_msg = http_get_msg_buf();
				if (rx_msg == NULL)
					return FALSE;
			}

			memcpy(rx_msg->msg_buf,p_user->p_rbuf,p_user->hdr_len);
			rx_msg->msg_buf[p_user->hdr_len] = '\0';
			//log_print("%s\r\n\r\n\r\n",rx_msg->msg_buf);

			parse_len = http_msg_parse_part1(rx_msg->msg_buf,p_user->hdr_len,rx_msg);
			if (parse_len != p_user->hdr_len)
			{
				log_print("http_tcp_rx::http_msg_parse_part1=%d, sip_pkt_len=%d!!!\r\n",parse_len,p_user->hdr_len);
				free_http_msg(rx_msg);
				return FALSE;
			}
		}

		if (p_user->ctt_len > 0)
		{
			int parse_len;
			
			memcpy(rx_msg->msg_buf+p_user->hdr_len, p_user->p_rbuf+p_user->hdr_len, p_user->ctt_len);
			rx_msg->msg_buf[p_user->hdr_len + p_user->ctt_len] = '\0';
			//log_print("%s",rx_msg->msg_buf+p_user->hdr_len);

			parse_len = http_msg_parse_part2(rx_msg->msg_buf+p_user->hdr_len,p_user->ctt_len,rx_msg);
			if (parse_len != p_user->ctt_len)
			{
				log_print("http_tcp_rx::http_msg_parse_part2=%d, sdp_pkt_len=%d!!!\r\n",parse_len,p_user->ctt_len);
				free_http_msg(rx_msg);
				return FALSE;
			}
		}

		p_user->rx_msg = rx_msg;
	}

	return TRUE;
}

BOOL http_tcp_tx(HTTPREQ * p_user, char * p_data, int len)
{
	int slen;
	
	if (p_user->cfd <= 0)
		return FALSE;

	slen = send(p_user->cfd, p_data, len, 0);
	if (slen != len)
		return FALSE;

	return TRUE;
}

BOOL http_onvif_req(HTTPREQ * p_user, char * action, char * p_xml, int len)
{
	int offset = 0;
	char bufs[11*1024];
	
	if (len > 10*1024)
	{
		return FALSE;
	}	
	
	offset += sprintf(bufs+offset, "POST %s HTTP/1.1\r\n", p_user->url);
	offset += sprintf(bufs+offset, "Host: %s:%d\r\n", p_user->host, p_user->port);
	offset += sprintf(bufs+offset, "User-Agent: Happytimesoft/1.0\r\n");
	offset += sprintf(bufs+offset, "Content-Type: application/soap+xml; charset=utf-8; action=\"%s\"\r\n", action);
	offset += sprintf(bufs+offset, "Content-Length: %d\r\n", len);
	offset += sprintf(bufs+offset, "Connection: close\r\n\r\n");

	memcpy(bufs+offset, p_xml, len);
	offset += len;
	bufs[offset] = '\0';

    // log_print("TX >> %s\r\n", bufs);
    
	return http_tcp_tx(p_user, bufs, offset);
}


BOOL http_onvif_trans(HTTPREQ * p_http, int timeout, eOnvifAction act, ONVIF_DEVICE * p_dev, void * p_req, void * p_res)
{
	int  addr_len;
	int  count = 0;
	int  xml_len;
	BOOL ret =  FALSE;
	char p_buf[1024*10];
	char * rx_xml = NULL;
	XMLN * p_node = NULL;
	XMLN * p_body = NULL;	
	OVFACTS * p_ent;
	struct sockaddr_in local_addr;

	p_ent = onvif_find_action_by_type(act);
	if (p_ent == NULL)
	{
		return FALSE;
	}
	
	// build xml bufs	
	xml_len = build_onvif_req_xml(p_buf, sizeof(p_buf), act, p_dev, p_req);

	// connect onvif device
	p_http->cfd = tcp_connect_timeout(inet_addr(p_http->host), p_http->port, timeout);
	if (p_http->cfd <= 0)
	{
		return FALSE;
	}

	addr_len = sizeof(local_addr);
    if (getsockname(p_http->cfd, (struct sockaddr *)&local_addr, (socklen_t*) &addr_len) == 0)
    {
        p_dev->local_ip = local_addr.sin_addr.s_addr;
    }
	
	if (FALSE == http_onvif_req(p_http, p_ent->action_url, p_buf, xml_len))
	{
		goto FAILED;
	}

	while (1)
	{
		int sret;
		fd_set fdr;
		struct timeval tv;

		tv.tv_sec = 0;
		tv.tv_usec = 100 * 1000;		
		
		FD_ZERO(&fdr);
		FD_SET(p_http->cfd, &fdr);

		sret = select(p_http->cfd+1, &fdr, NULL, NULL, &tv);
		if (sret == 0)
		{
		    count++;
		    if (count >= timeout / 100)
		    {
		        log_print("http_onvif_trans::timeout!!!\r\n");
		        break;
		    }
			continue;
		}
		else if(sret < 0)
		{
			log_print("http_rx_thread::select err[%s], sret[%d]!!!\r\n", sys_os_get_socket_error(), sret);
			break;
		}
		
		if (FD_ISSET(p_http->cfd, &fdr))
		{
			if (http_tcp_rx(p_http) == FALSE)
			{
				break;
			}	
			else if (p_http->rx_msg != NULL)
			{
				break;
			}	
		}
	}

	if (p_http->rx_msg == NULL)
	{
		goto FAILED;
	}
	
	if (p_http->rx_msg->msg_sub_type != 200)
	{
		goto FAILED;
	}
	
	if (p_http->rx_msg->ctt_type != CTT_XML)
	{
		goto FAILED;
	}

	rx_xml = get_http_ctt(p_http->rx_msg);
	if (NULL == rx_xml)
	{
		goto FAILED;
	}

    p_node = xxx_hxml_parse(rx_xml, p_http->rx_msg->ctt_len);
	if (p_node == NULL || p_node->name == NULL)
	{
		log_print("http_onvif_trans::xxx_hxml_parse ret null!!!\r\n");
		goto FAILED;
	}

    if (soap_strcmp(p_node->name, "s:Envelope") != 0)
	{	
		log_print("http_onvif_trans::node name[%s] != [s:Envelope]!!!\r\n", p_node->name);
		goto FAILED;
	}

	p_body = xml_node_soap_get(p_node, "s:Body");
	if (p_body == NULL)
	{
		log_print("http_onvif_trans::xml_node_soap_get[s:Body] ret null!!!\r\n");
		goto FAILED;
	}

	if (!onvif_rly_handler(p_body, act, p_dev, p_res))
	{
		goto FAILED;
	}
    
	ret = TRUE;

FAILED:

	if (p_http->cfd > 0)
	{
		closesocket(p_http->cfd);
		p_http->cfd = 0;
	}

	if (p_http->dyn_recv_buf)
	{
		free(p_http->dyn_recv_buf);
		p_http->dyn_recv_buf = NULL;
	}
	
	if (p_node)
	{
		xml_node_del(p_node);
	}
	
	if (p_http->rx_msg)
	{
		free_http_msg(p_http->rx_msg);
		p_http->rx_msg = NULL;
	}
	
	return ret;	
}



