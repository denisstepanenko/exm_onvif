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

#ifndef	__H_RUA_H__
#define	__H_RUA_H__

#include "rtsp_parse.h"
#include "rtp_queue.h"

typedef enum rtsp_client_states
{
	RCS_NULL = 0,	
	RCS_DESCRIBE,	
	RCS_INIT_V,		
	RCS_INIT_A,		
	RCS_READY,		
	RCS_PLAYING,	
	RCS_RECORDING,	
}RCSTATE;

typedef struct rtsp_ua
{
	unsigned int	used_flag	: 1;
	unsigned int	srv_flag	: 1;
	unsigned int	rtp_tcp		: 1;
	unsigned int	rtp_unicast	: 1;
	unsigned int	rtp_tx		: 1;
	unsigned int    need_auth   : 1;
	unsigned int    auth_mode   : 2;    // 0 - baisc; 1 - digest
	unsigned int	reserved	: 24;

	int				state;
	SOCKET			fd;
	unsigned int	mss_len;			// TCP_MAXSEG = 1460
	time_t			start_time;			

	char			ripstr[24];			// remote ip
	unsigned short	rport;				// remote port

	unsigned short	lport;

	unsigned int	cseq;				

	char			sid[64];			// Session ID
	char			uri[256];			// rtsp://221.10.50.195:554/cctv.sdp
	char			cbase[256];			// Content-Base: rtsp://221.10.50.195:554/broadcast.sdp/

	char			v_ctl[256];			// a=control:trackID=3
	char			a_ctl[256];			// a=control:trackID=3

	uint8			v_interleaved;		
	uint8			a_interleaved;		

	char			rcv_buf[2052];		
	int				rcv_dlen;			

	int				rtp_t_len;
	int				rtp_rcv_len;
	char *			rtp_rcv_buf;

	uint32			user_real_ip;
	uint16			user_real_port;

	uint32			remote_media_ip;

	int				self_audio_cap_count;
	uint8			self_audio_cap[MAX_AVN];
	char			self_audio_cap_desc[MAX_AVN][MAX_AVDESCLEN];

	int				self_video_cap_count;
	uint8			self_video_cap[MAX_AVN];
	char			self_video_cap_desc[MAX_AVN][MAX_AVDESCLEN];

	int				remote_audio_cap_count;
	uint8			remote_audio_cap[MAX_AVN];
	char			remote_audio_cap_desc[MAX_AVN][MAX_AVDESCLEN];

	int				remote_video_cap_count;
	uint8			remote_video_cap[MAX_AVN];
	char			remote_video_cap_desc[MAX_AVN][MAX_AVDESCLEN];

	int				same_audio_cap_count;
	uint8			same_audio_cap[MAX_AVN];
	int				same_video_cap_count;
	uint8			same_video_cap[MAX_AVN];

	UA_MEDIA		audio_rtp_media;
	UA_MEDIA		audio_rtcp_media;
	UA_MEDIA		video_rtp_media;
	UA_MEDIA		video_rtcp_media;

	UA_RTP_INFO		a_rtp_info;
	UA_RTP_INFO		v_rtp_info;

    HD_AUTH_INFO 	user_auth_info;
}RUA;


#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************/
HRTSP_MSG * rua_build_describe(RUA * p_rua);
HRTSP_MSG * rua_build_setup(RUA * p_rua,int type);
HRTSP_MSG * rua_build_play(RUA * p_rua);
HRTSP_MSG * rua_build_pause(RUA * p_rua);
HRTSP_MSG * rua_build_teardown(RUA * p_rua);
HRTSP_MSG * rua_build_get_parameter(RUA * p_rua);
HRTSP_MSG * rua_build_option(RUA * p_rua);

/*************************************************************************/
BOOL 		get_rtsp_media_info(RUA * p_rua, HRTSP_MSG * rx_msg);
BOOL 		get_sdp_h264_desc(RUA * p_rua, int * pt, char * p_sps, int max_len);

/*************************************************************************/
void 		send_rtsp_msg(RUA * p_rua,HRTSP_MSG * tx_msg);
#define send_free_rtsp_msg(p_rua,tx_msg) do{send_rtsp_msg(p_rua,tx_msg); free_rtsp_msg(tx_msg);}while(0)


#ifdef __cplusplus
}
#endif

#endif	//	__H_RUA_H__




