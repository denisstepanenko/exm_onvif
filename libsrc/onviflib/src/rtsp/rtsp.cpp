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

#include <string.h>
#include <stdio.h>
#include "sys_inc.h"
#include "rtp.h"
#include "rtsp.h"


void * rtsp_rx_thread(void * argv)
{
	CRtsp * pRtsp = (CRtsp *)argv;

	pRtsp->rx_thread();

	return NULL;
}


CRtsp::CRtsp(void)
{
    m_bRunning = true;
	m_rtspRxTid = 0;
	
	m_pNotify = 0;
	m_pUserdata = 0;
	m_pVideoCB = 0;
	m_pAudioCB = 0;
	
	m_r264_fu_s = 0;
	m_r264_fu_nalu = 0;
	m_r264_fu_header = 0;
	m_r264_fu_offset = 0;

    m_bsdpgetting = false;
    memset(m_audiofromsdp,0,sizeof(m_audiofromsdp));
    
	memset(&m_rua, 0, sizeof(RUA));

}

CRtsp::~CRtsp(void)
{
	rtsp_close();
}

bool CRtsp::rtsp_client_start()
{	
	m_rua.rport = m_nport;
	strcpy(m_rua.ripstr, m_ip);
	strcpy(m_rua.uri, m_url);

	if (rua_init_connect(&m_rua) == false)
	{
		log_print("rua_init_connect fail!!!\r\n");
		return false;
	}

	this->m_rua.cseq = 1;
	HRTSP_MSG * tx_msg = rua_build_describe(&m_rua);
	if (tx_msg)
	{
		send_free_rtsp_msg(&m_rua, tx_msg);
	}

	this->m_rua.state = RCS_DESCRIBE;

	return true;
}

bool CRtsp::rua_init_connect(RUA * p_rua)
{
	SOCKET fd = socket(AF_INET,SOCK_STREAM,0);
	if (fd == -1)
	{
		log_print("rua_init_connect::socket fail!!!\r\n");
		return false;
	}

	log_print("rua_init_connect::fd = %d\r\n",fd);

	if (p_rua->lport != 0)
	{
		struct sockaddr_in addr;

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(p_rua->lport);

		if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		{
			log_print("rua_init_connect::bind lport[%u] fail.\n", p_rua->lport);
			closesocket(fd);
			return false;
		}
	}

	struct sockaddr_in raddr;

	raddr.sin_family = AF_INET;
	raddr.sin_addr.s_addr = inet_addr(p_rua->ripstr);
	raddr.sin_port = htons(p_rua->rport);

    struct timeval timeo = {5, 0};
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeo, sizeof(timeo));
	
	if (connect(fd, (struct sockaddr *)&raddr, sizeof(struct sockaddr_in)) == -1)
	{
		log_print("rua_init_connect::connect %s:%u fail!!!\r\n", p_rua->ripstr, p_rua->rport);
		closesocket(fd);
		return false;
	}

	int len = 1024*1024;
	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&len, sizeof(int)))
	{
		log_print("setsockopt SO_RCVBUF error!\n");
		return false;
	}

	p_rua->fd = fd;

	return true;
}


void CRtsp::rtsp_send_sps_pps_para(RUA * p_rua)
{
	int pt;
	char sps[1000], pps[1000] = {'\0'};
	
	if (!get_sdp_h264_desc(p_rua, &pt, sps, sizeof(sps)))
	{
		return;
	}

	char * ptr = strchr(sps, ',');
	if (ptr && ptr[1] != '\0')
	{
		*ptr = '\0';
		strcpy(pps, ptr+1);
	}

	unsigned char sps_pps[1000];
	sps_pps[0] = 0x0;
	sps_pps[1] = 0x0;
	sps_pps[2] = 0x0;
	sps_pps[3] = 0x1;
	
	int len = base64_decode(sps, sps_pps+4, sizeof(sps_pps)-4);
	if (len <= 0)
	{
		return;
	}

	if (m_pVideoCB)
	{
		m_pVideoCB(sps_pps, len+4, 0, 0, m_pUserdata);
	}
	
	if (pps[0] != '\0')
	{		
		len = base64_decode(pps, sps_pps+4, sizeof(sps_pps)-4);
		if (len > 0)
		{
			if (m_pVideoCB)
			{
				m_pVideoCB(sps_pps, len+4, 0, 0, m_pUserdata);
			}
		}
	}
}


/***********************************************************************
*
* Close RUA
*
************************************************************************/
void CRtsp::rtsp_client_stop(RUA * p_rua)
{
	if (p_rua->fd > 0)
	{
		HRTSP_MSG * tx_msg = rua_build_teardown(p_rua);
		if (tx_msg)
			send_free_rtsp_msg(p_rua,tx_msg);
	}
}

bool CRtsp::parse_sdp_audioinfo(RUA * p_rua)
{






}



int CRtsp::rtsp_client_state(RUA * p_rua, HRTSP_MSG * rx_msg)
{
	HRTSP_MSG * tx_msg = NULL;
    int i = 0;

	if (rx_msg->msg_type == 0)	// Request message?
	{
		return -1;
	}

	switch (p_rua->state)
	{
	case RCS_NULL:
		break;

	case RCS_DESCRIBE:
		if (rx_msg->msg_sub_type == 200)
		{
			//Session
			get_rtsp_session_info(rx_msg, p_rua->sid, sizeof(p_rua->sid)-1);

			//if(p_rua->sid[0] == '\0')
			//{
			//	sprintf(p_rua->sid, "%x%x", rand(), rand());
			//}

			char cseq_buf[32];
			char cbase[256];

			get_rtsp_msg_cseq(rx_msg, cseq_buf, sizeof(cseq_buf)-1);

			//Content-Base			
			if (get_rtsp_cbase_info(rx_msg, cbase, sizeof(cbase)-1))
			{
				strncpy(p_rua->uri, cbase, sizeof(p_rua->uri)-1);
			}

			find_rtsp_sdp_control(rx_msg, p_rua->v_ctl, (char *)"video", sizeof(p_rua->v_ctl)-1);
			find_rtsp_sdp_control(rx_msg, p_rua->a_ctl, (char *)"audio", sizeof(p_rua->a_ctl)-1);

			get_rtsp_media_info(p_rua, rx_msg);

            if(m_bsdpgetting)
            {
                for(i=0;i<MAX_AVN;++i)
                {
                    if(p_rua->remote_audio_cap_desc[i][0]!='\0')
                    {
                        memset(m_audiofromsdp,0,sizeof(m_audiofromsdp));
                        strcpy(m_audiofromsdp,(const char*)p_rua->remote_audio_cap_desc[i]);
                        if(NULL!=strstr(m_audiofromsdp,"rtpmap"))
                        {
                            //found target str
                            break;
                        }
                    }
                }
                m_bsdpgetting = false;
                return -1;
            }
			//Send SETUP
			if (p_rua->v_ctl[0] != '\0')
			{
				p_rua->v_interleaved = 0;
				p_rua->cseq++;
				tx_msg = rua_build_setup(p_rua,0);
				if(tx_msg)
					send_free_rtsp_msg(p_rua,tx_msg);
			}

			// rua_notify_emsg(p_rua,PUEVT_ALERT);

			p_rua->state = RCS_INIT_V;
		}
		else if (rx_msg->msg_sub_type == 401)
		{
		    p_rua->need_auth = TRUE;
		    
		    if (rtsp_get_digest_info(rx_msg, &(p_rua->user_auth_info)))
			{
			    p_rua->auth_mode = 1;
				sprintf(p_rua->user_auth_info.auth_uri, "%s", p_rua->uri);                
			}
			else
			{
			    p_rua->auth_mode = 0;
			}

			HRTSP_MSG * tx_msg = rua_build_describe(&m_rua);
        	if (tx_msg)
        	{
        		send_free_rtsp_msg(&m_rua, tx_msg);
        	}
		}
		break;

	case RCS_INIT_V:
		if (rx_msg->msg_sub_type == 200)
		{
			char cbase[256];
			
			//Content-Base			
			if (get_rtsp_cbase_info(rx_msg, cbase, sizeof(cbase)-1))
			{
				strncpy(p_rua->uri, cbase, sizeof(p_rua->uri)-1);
				sprintf(p_rua->user_auth_info.auth_uri, "%s", p_rua->uri);
			}
			
			//Session
			if(p_rua->sid[0] == '\0')
				get_rtsp_session_info(rx_msg, p_rua->sid, sizeof(p_rua->sid)-1);

			if(p_rua->sid[0] == '\0')
			{
				sprintf(p_rua->sid, "%x%x", rand(), rand());
			}
			
			p_rua->cseq++;

			if (p_rua->a_ctl[0] != '\0')
			{
				p_rua->a_interleaved = 2;
				tx_msg = rua_build_setup(p_rua, 1);
				if (tx_msg)
					send_free_rtsp_msg(p_rua,tx_msg);

				p_rua->state = RCS_INIT_A;
			}
			else
			{
				//only video without audio
				tx_msg = rua_build_play(p_rua);
				if (tx_msg)
					send_free_rtsp_msg(p_rua,tx_msg);
				p_rua->state = RCS_READY;
			}
		}
		break;

	case RCS_INIT_A:
		if (rx_msg->msg_sub_type == 200)
		{
			//Session
			if(p_rua->sid[0] == '\0')
				get_rtsp_session_info(rx_msg, p_rua->sid, sizeof(p_rua->sid)-1);
				
			p_rua->cseq++;
			tx_msg = rua_build_play(p_rua);
			if (tx_msg)
				send_free_rtsp_msg(p_rua,tx_msg);
			p_rua->state = RCS_READY;
		}
		else
		{
			//error handle
		}
		break;

	case RCS_READY:
		if (rx_msg->msg_sub_type == 200)
		{
			p_rua->state = RCS_PLAYING;
			p_rua->start_time = sys_os_get_ms();
			
			send_notify(RTSP_EVE_CONNSUCC);

			rtsp_send_sps_pps_para(p_rua);
		}
		else
		{
			//error handle
			return -1;
		}
		break;

	case RCS_PLAYING:
		break;

	case RCS_RECORDING:
		break;
	}		

	return 0;
}


int CRtsp::rtsp_pkt_find_end(char * p_buf)
{
	int end_off = 0;
	int sip_pkt_finish = 0;

	while (p_buf[end_off] != '\0')
	{
		if ((p_buf[end_off] == '\r' && p_buf[end_off+1] == '\n') &&
			(p_buf[end_off+2] == '\r' && p_buf[end_off+3] == '\n'))
		{
			sip_pkt_finish = 1;
			break;
		}

		end_off++;
	}

	if (sip_pkt_finish)
		return (end_off + 4);
		
	return 0;
}

int CRtsp::rtsp_tcp_rx()
{
	RUA * p_rua = &(this->m_rua);
	RILF *p_rilf = NULL;
	if (p_rua->fd <= 0)
		return -1;

    int sret, ret = 1;
    fd_set fdread;
    struct timeval tv = {1, 0};
	unsigned short rtp_len = 0;
    FD_ZERO(&fdread);
    FD_SET(p_rua->fd, &fdread); 
    
    sret = select(p_rua->fd+1, &fdread, NULL, NULL, &tv); 
    if (sret == 0) // Time expired 
    { 
        return 0; 
    }
    else if (!FD_ISSET(p_rua->fd, &fdread))
    {
        return 0;
    }
    
	if (p_rua->rtp_rcv_buf == NULL || p_rua->rtp_t_len == 0)
	{
		int rlen = recv(p_rua->fd, p_rua->rcv_buf+p_rua->rcv_dlen, 2048-p_rua->rcv_dlen, 0);
		printf("%p recv use rcv_buf ret %d\n",this,rlen);
		if (rlen <= 0)
		{
			//log_print("RTST recv thread exit£¬ret = %d, err = %s\r\n",rlen,sys_os_get_socket_error());	//recv error, connection maybe disconn?
			printf("RTSP recv thread exit£¬ret = %d, err = %s\r\n",rlen,sys_os_get_socket_error());	//recv error, connection maybe disconn?
			closesocket(p_rua->fd);
			p_rua->fd = 0;

			return -1;
		}

		p_rua->rcv_dlen += rlen;
	}
	else
	{
		int rlen = recv(p_rua->fd, p_rua->rtp_rcv_buf+p_rua->rtp_rcv_len, p_rua->rtp_t_len-p_rua->rtp_rcv_len, 0);
		printf("%p recv use rtp_rcv_buf ret %d\n",this,rlen);
		if (rlen <= 0)
		{
			log_print("RTST recv thread exit£¬ret = %d, err = %s\r\n",rlen,sys_os_get_socket_error());	//recv error, connection maybe disconn?
			printf("RTSp recv thread exit£¬ret = %d, err = %s\r\n",rlen,sys_os_get_socket_error());	//recv error, connection maybe disconn?
			closesocket(p_rua->fd);
			p_rua->fd = 0;

			return -1;
		}

		p_rua->rtp_rcv_len += rlen;
		if(p_rua->rtp_rcv_len == p_rua->rtp_t_len)
		{
			printf("when rtp_rcv_len == rtp_t_len!\n");
			p_rilf = (RILF *)(p_rua->rtp_rcv_buf);
			if(p_rilf->magic != 0x24)
			{
				log_print("rtsp_tcp_rx::p_rilf->magic[0x%02X]!!!\r\n", p_rilf->magic);
				printf("%p rtsp_tcp_rx::p_rilf->magic[0x%02X]!!! at %d\r\n", this,p_rilf->magic,__LINE__);
			}
			
			rtp_len = ntohs(p_rilf->rtp_len);
			if (p_rilf->channel == p_rua->v_interleaved)
				printf("%p rtp data for video len is %d\n",this,rtp_len);
			else if (p_rilf->channel == p_rua->a_interleaved)
				printf("%p rtp data for audio len is %d\n",this,rtp_len);
			else
			{
				printf("%p rtp wrong data len is %d \n",this,rtp_len);
				free(p_rua->rtp_rcv_buf);
				p_rua->rtp_rcv_buf = NULL;
				p_rua->rtp_rcv_len = 0;
				p_rua->rtp_t_len = 0;

				return 2;
			}
			
			video_rtp_rx(p_rua, (LPBYTE)(p_rua->rtp_rcv_buf+4), p_rua->rtp_rcv_len-4);

			free(p_rua->rtp_rcv_buf);
			p_rua->rtp_rcv_buf = NULL;
			p_rua->rtp_rcv_len = 0;
			p_rua->rtp_t_len = 0;

			ret = 2;
		}
		
		return ret;
	}

rx_point:

	if (is_rtsp_msg(p_rua->rcv_buf))	//Is RTSP Packet?
	{
		int rtsp_pkt_len = rtsp_pkt_find_end(p_rua->rcv_buf);
		if (rtsp_pkt_len == 0) // wait for next recv
			return ret;

		HRTSP_MSG * rx_msg = get_rtsp_msg_buf();
		if (rx_msg == NULL)
		{
			log_print("rtsp_tcp_rx::get_rtsp_msg_buf return null!!!\r\n");
			printf("rtsp_tcp_rx::get_rtsp_msg_buf return null!!!\r\n");
			return -1;
		}

		memcpy(rx_msg->msg_buf, p_rua->rcv_buf, rtsp_pkt_len);
		rx_msg->msg_buf[rtsp_pkt_len] = '\0';
		// log_print("%s\r\n", rx_msg->msg_buf);

		int parse_len = rtsp_msg_parse_part1(rx_msg->msg_buf,rtsp_pkt_len,rx_msg);
		if (parse_len != rtsp_pkt_len)	//parse error
		{
			log_print("rtsp_tcp_rx::rtsp_msg_parse_part1=%d, rtsp_pkt_len=%d!!!\r\n",parse_len,rtsp_pkt_len);
			printf("rtsp_tcp_rx::rtsp_msg_parse_part1=%d, rtsp_pkt_len=%d!!!\r\n",parse_len,rtsp_pkt_len);
			free_rtsp_msg(rx_msg);

			p_rua->rcv_dlen = 0;
			return ret;
		}
		
		if(rx_msg->ctx_len > 0)	
		{
			if(p_rua->rcv_dlen < (parse_len + rx_msg->ctx_len))
			{
				free_rtsp_msg(rx_msg);
				return ret;
			}

			memcpy(rx_msg->msg_buf+rtsp_pkt_len, p_rua->rcv_buf+rtsp_pkt_len, rx_msg->ctx_len);
			rx_msg->msg_buf[rtsp_pkt_len + rx_msg->ctx_len] = '\0';
			// log_print("%s\r\n",rx_msg->msg_buf+rtsp_pkt_len);

			int sdp_parse_len = rtsp_msg_parse_part2(rx_msg->msg_buf+parse_len,rx_msg->ctx_len,rx_msg);
			if(sdp_parse_len != rx_msg->ctx_len)
			{
				//log_print("rtsp_tcp_rx::rtsp_msg_parse_part2=%d, sdp_pkt_len=%d!!!\r\n",sdp_parse_len,rx_msg->ctx_len);				
				//free_rtsp_msg(rx_msg);
				
				//p_rua->rcv_dlen = 0;
				//return 1;
			}
			parse_len += rx_msg->ctx_len;
		}
		
		if (parse_len < p_rua->rcv_dlen)
		{
			while(p_rua->rcv_buf[parse_len] == ' ' || 
				p_rua->rcv_buf[parse_len] == '\r' || p_rua->rcv_buf[parse_len] == '\n') parse_len++;

			memmove(p_rua->rcv_buf, p_rua->rcv_buf + parse_len, p_rua->rcv_dlen - parse_len);
			p_rua->rcv_dlen -= parse_len;
			p_rua->rcv_buf[p_rua->rcv_dlen] = '\0';
		}
		else
			p_rua->rcv_dlen = 0;

		rtsp_client_state(p_rua, rx_msg);
		free_rtsp_msg(rx_msg);

		if (p_rua->rcv_dlen > 16)
		{
			goto rx_point;
		}
	}
	else
	{
		int offset = 0;
		while ((offset + 16) < p_rua->rcv_dlen)
		{
			p_rilf = (RILF *)(p_rua->rcv_buf + offset);
			if(p_rilf->magic != 0x24)
			{
				if (is_rtsp_msg(p_rua->rcv_buf + offset))
				{
					if (offset > 0)
					{
						memmove(p_rua->rcv_buf, p_rua->rcv_buf+offset, p_rua->rcv_dlen-offset);
						p_rua->rcv_dlen -= offset;
						offset = 0;
					}
					
					goto rx_point;
				}
			
				log_print("rtsp_tcp_rx::p_rilf->magic[0x%02X]!!!\r\n", p_rilf->magic);
				printf("rtsp_tcp_rx::p_rilf->magic[0x%02X]!!!\r\n", p_rilf->magic);
				p_rua->rcv_dlen = 0;
				return ret;
			}
			
			rtp_len = ntohs(p_rilf->rtp_len);
			if (p_rilf->channel == p_rua->v_interleaved)
				printf("%p rtp data for video len is %d\n",this,rtp_len);
			else if (p_rilf->channel == p_rua->a_interleaved)
				printf("%p rtp data for audio len is %d\n",this,rtp_len);
			else
				printf("%p rtp wrong data len is %d \n",this,rtp_len);

	
	
			//if (p_rilf->channel == p_rua->v_interleaved || p_rilf->channel == p_rua->a_interleaved)
			{
				ret = 2;
				
				if (rtp_len > (p_rua->rcv_dlen - offset - 4))
				{
					if (offset > 0)
					{
						memmove(p_rua->rcv_buf, p_rua->rcv_buf+offset, p_rua->rcv_dlen-offset);
						p_rua->rcv_dlen -= offset;
						offset = 0;
					}
				
					p_rua->rtp_rcv_buf = (char *)malloc(rtp_len+4);

					if(p_rua->rtp_rcv_buf == NULL) 
					{
						if(p_rua->rcv_dlen<0)
							printf("%p, %d: p_rua rcv dlen is %d,offset is %d\n",this,__LINE__,p_rua->rcv_dlen,offset);
					    return -1;
					}
					memcpy(p_rua->rtp_rcv_buf, p_rua->rcv_buf+offset, p_rua->rcv_dlen-offset);
					p_rua->rtp_rcv_len = p_rua->rcv_dlen-offset;
					p_rua->rtp_t_len = rtp_len+4;
				
					p_rua->rcv_dlen = 0;

					return 2;
				}
				else
				{
					rtp_hdr_t * p_rtp = (rtp_hdr_t *)((char *)p_rilf + 4);
					if (p_rilf->channel == p_rua->v_interleaved)
					{
						printf("%p when recv video from interleaved!\n",this);
						video_rtp_rx(p_rua, (LPBYTE)p_rtp, rtp_len);
					}
					else if (p_rilf->channel == p_rua->a_interleaved)
					{
                        //lc change valid this
						printf("%p when recv audio from interleaved!\n",this);
						audio_rtp_rx(p_rua, (LPBYTE)p_rtp, rtp_len);
					}
				}
			}
			//else
			/*{
				printf("%p no a/v interleaved !\n",this);
				// log_print("rtsp_tcp_rx::p_rilf->channel = %d!!!\r\n", p_rilf->channel);
			}*/

			offset += rtp_len + 4;
		}

		p_rua->rcv_dlen -= offset;
		if(p_rua->rcv_dlen<0)
			  printf("%p,%d: p_rua rcv dlen is %d,offset is %d,rtp len is %d,rtp_rev_buf is %p,rcvbuf is %p,RILF is %p\n",this,__LINE__,p_rua->rcv_dlen,offset,rtp_len,p_rua->rtp_rcv_buf,p_rua->rcv_buf,p_rilf);
		if (p_rua->rcv_dlen > 0)
		{
			memcpy(p_rua->rcv_buf, p_rua->rcv_buf+offset, p_rua->rcv_dlen);
		}
	}

	return ret;
}


bool CRtsp::rtsp_getsdp_start(char* url, char* ip, int port, char* audio_desc_buf, const char * user, const char * pass)
{
    int ret = 0;
    int timeoutcount = 0;

    if(url==NULL)
        return false;

    m_bsdpgetting = true;

	memset(&m_rua, 0, sizeof(RUA));
	memset(m_url, 0, sizeof(m_url));
	memset(m_ip, 0, sizeof(m_ip));

	if (user)
	{
		strcpy(m_rua.user_auth_info.auth_name, user);
	}

	if (pass)
	{
		strcpy(m_rua.user_auth_info.auth_pwd, pass);
	}
	
	if (url)
	{
		strcpy(m_url, url);
	}
	
	if (ip)
	{
		strcpy(m_ip, ip);
	}
    m_nport = port;
    
    rtsp_client_start();

    while(m_bsdpgetting)
    {
        ret = rtsp_tcp_rx();
        if (ret < 0)
        {
            printf("rtsp tcp rx once in sdp failed!\n");
            goto clean;
        }
        else if (ret = 0)
        {
            timeoutcount++;
	        if (timeoutcount >= 10)    // 10s without signal
	        {
                printf("10 secs without any data from describe!\n");
                goto clean;
	        }
        }
        else
            timeoutcount = 0;
    }
    
    if(strstr(m_audiofromsdp,"rtpmap")!=NULL)
        strcpy(audio_desc_buf,m_audiofromsdp);

    rtsp_client_stop(&m_rua);

clean:
    if (m_rua.fd)
	{
		closesocket(m_rua.fd);
		m_rua.fd = 0;
	}

    if (m_rua.rtp_rcv_buf)
    {
        free(m_rua.rtp_rcv_buf);
        m_rua.rtp_rcv_buf = NULL;
    }
    
    return true;
}


bool CRtsp::rtsp_start(char* url, char* ip, int port, const char * user, const char * pass)
{
	if (m_rua.state != RCS_NULL)
	{
		rtsp_play();
		return true;
	}

	memset(&m_rua, 0, sizeof(RUA));
	memset(m_url, 0, sizeof(m_url));
	memset(m_ip, 0, sizeof(m_ip));

	if (user)
	{
		strcpy(m_rua.user_auth_info.auth_name, user);
	}

	if (pass)
	{
		strcpy(m_rua.user_auth_info.auth_pwd, pass);
	}
	
	if (url)
	{
		strcpy(m_url, url);
	}
	
	if (ip)
	{
		strcpy(m_ip, ip);
	}
	
	m_nport = port;

    m_bRunning = true;
	m_rtspRxTid = sys_os_create_thread((void *)rtsp_rx_thread, this);
	if (m_rtspRxTid == 0)
	{
		log_print((char *)"start_video::pthread_create rtsp_rx_thread failed!!!\r\n");
		return false;
	}
	
	return true;
}

bool CRtsp::rtsp_play()
{
	m_rua.cseq++;
	HRTSP_MSG * tx_msg = rua_build_play(&m_rua);	
	if (tx_msg)
		send_free_rtsp_msg(&m_rua,tx_msg);
	return true;
}

bool CRtsp::rtsp_stop()
{
	m_rua.cseq++;
	HRTSP_MSG * tx_msg = rua_build_teardown(&m_rua);
	if (tx_msg)
		send_free_rtsp_msg(&m_rua,tx_msg);	
	m_rua.state = RCS_NULL;
	return true;
}

bool CRtsp::rtsp_pause()
{
	m_rua.cseq++;
	HRTSP_MSG * tx_msg = rua_build_pause(&m_rua);	
	if(tx_msg)
		send_free_rtsp_msg(&m_rua,tx_msg);	
	return true;
}

bool CRtsp::rtsp_close()
{
    m_bRunning = false;
	while(m_rtspRxTid != 0)
	{
		usleep(10 * 1000);	// 10ms wait
	}
	
	m_r264_fu_s = 0;
	m_r264_fu_nalu = 0;
	m_r264_fu_header = 0;
	m_r264_fu_offset = 0;
    
	memset(&m_rua, 0, sizeof(RUA));
	
	return true;
}

int CRtsp::h264_fu_rtp_rx(LPBYTE lpData, int len)
{
	unsigned char nalu = lpData[12];
	unsigned char fu_hdr = lpData[13];

	// 	if(m_ph264_fu_buf==NULL)
	// 	{
	// 		m_ph264_fu_buf = (unsigned char*)malloc(1024*1024);
	// 		memset(m_ph264_fu_buf,0,1024*1024);
	// 	}

	if(m_r264_fu_s == 1)
	{
		if((fu_hdr & 0x80) == 0x80)
		{
			m_r264_fu_s = 0;
			m_r264_fu_offset = 0;
		}
	}

	if((fu_hdr & 0x80) == 0x80)
	{
		m_r264_fu_s = 1;
		m_r264_fu_nalu = lpData[12];
		m_r264_fu_header = lpData[13];

		memcpy(m_ph264_fu_buf+5, lpData+14, len-14);
		m_r264_fu_offset = len - 14;
	}
	else
	{
		if(m_r264_fu_s == 0)
		{
			return -1;
		}

		if((m_r264_fu_offset+5+m_r264_fu_offset+len-14) >= sizeof(m_ph264_fu_buf))
		{
			log_print("Fragment Reassembly Packet Too Big¡¾%d¡¿!!!",m_r264_fu_offset+5+m_r264_fu_offset+len-14);
			return -1;
		}

		memcpy(m_ph264_fu_buf+5+m_r264_fu_offset, lpData+14, len-14);
		m_r264_fu_offset += len - 14;

		if((fu_hdr & 0x40) == 0x40)
		{
			// + 00 00 00 01 + NALU
			m_ph264_fu_buf[0] = 0x00;
			m_ph264_fu_buf[1] = 0x00;
			m_ph264_fu_buf[2] = 0x00;
			m_ph264_fu_buf[3] = 0x01;
			m_ph264_fu_buf[4] = (m_r264_fu_nalu & 0x60) | (m_r264_fu_header & 0x1F);

			int rlen = m_r264_fu_offset+5;

			m_r264_fu_s = 0;
			m_r264_fu_offset = 0;

			return rlen;
		}
	}

	return 0;
}

void CRtsp::audio_rtp_rx(RUA * p_rua, LPBYTE lpData, int rlen)
{
	if (m_pAudioCB)
    {
        m_pAudioCB(lpData+12, rlen-12, m_pUserdata);
    }
}

void CRtsp::video_rtp_rx(RUA * p_rua, LPBYTE lpData, int rlen)
{
	int rrlen = 0;
	unsigned char * p_rtp_data = NULL;

	rtp_hdr_t * p_rtp = (rtp_hdr_t *)lpData;

	unsigned char nalu = lpData[12];
	if((nalu & 0x1F) == 28)	// Fragment
	{
		rrlen = h264_fu_rtp_rx(lpData, rlen);
		if(rrlen > 5)
		{
			p_rtp_data = m_ph264_fu_buf + 4;
			rrlen -= 4;
		}
		else
			return;
	}
	else
	{
		rrlen = rlen - 12;
		p_rtp_data = lpData + 12;
	}

	video_rtp_rx_post(p_rua, p_rtp_data, rrlen, ntohl(p_rtp->ts), ntohs(p_rtp->seq));
}

void CRtsp::video_rtp_rx_post(RUA * p_rua, LPBYTE lpH264Data, int rlen, unsigned int ts, unsigned short seq)
{
	LPBYTE lpData = lpH264Data - 4;
	lpData[0] = 0;
	lpData[1] = 0;
	lpData[2] = 0;
	lpData[3] = 1;

    if (m_pVideoCB)
    {
        m_pVideoCB(lpData, rlen+4, ts, seq, m_pUserdata);
    }
}

void CRtsp::rx_thread()
{
	int ret, nosignal_count = 0, nodata_count = 0;
    bool nosignal_notify = false, nodata_notify = false;
    
	send_notify(RTSP_EVE_CONNECTING);
	
	if (!rtsp_client_start())	
	{
		send_notify(RTSP_EVE_CONNFAIL);
		goto rtsp_rx_exit;
	}
	
	send_notify(RTSP_EVE_CONNSUCC);
    
	while (m_bRunning)
	{
	    ret = rtsp_tcp_rx();
	    if (ret < 0)
	    {
            printf("rtsp tcp rx ret %d\n",ret);
	        break;
	    }
	    else if (ret == 0)
	    {
	        nosignal_count++;
	        if (nosignal_count >= 10 && !nosignal_notify)    // 10s without signal
	        {
	            nosignal_notify = true;
	            send_notify(RTSP_EVE_NOSIGNAL);
	        }
	    }
	    else
	    {
	    	if (ret != 2)
	    	{
	    		nodata_count++;
		        if (nodata_count >= 20 && !nodata_notify)    // without data
		        {
		            nodata_notify = true;
		            send_notify(RTSP_EVE_NODATA);
		        }
	        }
	        else
	        {
	        	if (nodata_notify)
		        {
		            nodata_notify = false;
		            send_notify(RTSP_EVE_RESUME);
		        }
		        
	        	nodata_count = 0;
	        }
	        
	        if (nosignal_notify)
	        {
	            nosignal_notify = false;
	            send_notify(RTSP_EVE_RESUME);
	        }
	        
	        nosignal_count = 0;	        
	    }
/*
	    if (m_rua.state == RCS_PLAYING)
	    {
	    	int ms = sys_os_get_ms();
	    	if (ms - m_rua.start_time >= 25 * 1000)
	    	{
	    		m_rua.start_time = ms;
	    		m_rua.cseq++;
	    		//lc some devices do not support GET_PARAMETER ,change to OPTION
                //HRTSP_MSG * tx_msg = rua_build_get_parameter(&m_rua);
                printf("send get option msg!\n");
				HRTSP_MSG* tx_msg = rua_build_option(&m_rua);
	    		if (tx_msg)
            	{
            		send_free_rtsp_msg(&m_rua, tx_msg);
            	}
	    	}
	    }
*/ }

    if (m_rua.fd)
	{
		closesocket(m_rua.fd);
		m_rua.fd = 0;
	}

    if (m_rua.rtp_rcv_buf)
    {
        free(m_rua.rtp_rcv_buf);
        m_rua.rtp_rcv_buf = NULL;
    }
    
	send_notify(RTSP_EVE_STOPPED);

rtsp_rx_exit:

	m_rtspRxTid = 0;
	log_print("rtsp_rx_thread exit\r\n");
}


void CRtsp::send_notify(int event)
{
	if (m_pNotify)
	{
		m_pNotify(event, m_pUserdata);
	}
}

void CRtsp::get_sps_pps_para()
{
	rtsp_send_sps_pps_para(&m_rua);
}





