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

#ifndef _RTSP_H_
#define _RTSP_H_

#include "sys_buf.h"
#include "rua.h"

typedef int (*notify_cb)(int, void *);
typedef int (*video_cb)(LPBYTE, int, unsigned int, unsigned short, void *);
typedef int (*audio_cb)(LPBYTE, int, void *);

#define RTSP_EVE_STOPPED    0
#define RTSP_EVE_CONNECTING 1
#define RTSP_EVE_CONNFAIL   2
#define RTSP_EVE_CONNSUCC   3
#define RTSP_EVE_NOSIGNAL   4
#define RTSP_EVE_RESUME     5
#define RTSP_EVE_AUTHFAILED 6
#define RTSP_EVE_NODATA   	7


class CRtsp
{
public:
	CRtsp(void);
	~CRtsp(void);

public:
	bool    rtsp_start(char* url, char* ip, int port, const char * user, const char * pass);
	bool    rtsp_getsdp_start(char* url, char* ip, int port, char* audio_desc_buf, const char * user, const char * pass);
    bool    rtsp_play();
	bool    rtsp_stop();
	bool    rtsp_pause();
	bool    rtsp_close();

	RUA *   get_rua() {return &m_rua;}
	char *  get_url() {return m_url;}
	char *  get_ip() {return  m_ip;}
	int     get_port() {return m_nport;}
	void    set_notify_cb(notify_cb notify, void * userdata) {m_pNotify = notify; m_pUserdata = userdata;}
	void    set_video_cb(video_cb cb) {m_pVideoCB = cb;}
	void    set_audio_cb(audio_cb cb) {m_pAudioCB = cb;}
	void 	get_sps_pps_para();
	
    void    rx_thread();		

private:
    int     rtsp_pkt_find_end(char * p_buf);
    bool    rtsp_client_start();
	bool    rua_init_connect(RUA * p_rua);
	void    rtsp_client_stop(RUA * p_rua);
	int     rtsp_client_state(RUA * p_rua, HRTSP_MSG * rx_msg);	
	int     h264_fu_rtp_rx(LPBYTE lpData, int len);
    int     rtsp_tcp_rx();
	void    audio_rtp_rx(RUA * p_rua, LPBYTE lpData, int rlen);
	void    video_rtp_rx(RUA * p_rua, LPBYTE lpData, int rlen);
	void    video_rtp_rx_post(RUA * p_rua, LPBYTE lpH264Data, int rlen, unsigned int ts, unsigned short seq);
    void    send_notify(int event);
    void 	rtsp_send_sps_pps_para(RUA * p_rua);
    bool    parse_sdp_audioinfo(RUA * p_rua);
    
private:
	RUA		        m_rua;
	char            m_url[256];
	char            m_ip[32];
	int             m_nport;
	notify_cb       m_pNotify;
	void *          m_pUserdata;
	video_cb        m_pVideoCB;
	audio_cb        m_pAudioCB;

	unsigned char   m_ph264_fu_buf[1024*1024*2];
	unsigned char	m_r264_fu_s;		
	unsigned char	m_r264_fu_nalu;		
	unsigned char	m_r264_fu_header;	
	unsigned int	m_r264_fu_offset;

	bool            m_bRunning;
	pthread_t       m_rtspRxTid;
    //lc add just to get sdp
    bool            m_bsdpgetting;
    char            m_audiofromsdp[100];
};



#endif	// _RTSP_H_



