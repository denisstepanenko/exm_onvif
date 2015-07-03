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

#ifndef	__H_RTSP_PARSE_H__
#define	__H_RTSP_PARSE_H__

/*************************************************************************/
typedef enum rtsp_req_mt
{
	RTSP_MT_NULL = 0,
	RTSP_MT_DESCRIBE,	//
	RTSP_MT_ANNOUNCE,	//
	RTSP_MT_OPTIONS,	//
	RTSP_MT_PAUSE,		//
	RTSP_MT_PLAY,		//
	RTSP_MT_RECORD,		//
	RTSP_MT_REDIRECT,	//
	RTSP_MT_SETUP,		
	RTSP_MT_SET_PARAMETER,
	RTSP_MT_GET_PARAMETER,
	RTSP_MT_TEARDOWN	//
}RTSP_RMT;

typedef struct rtsp_req_message_type_value
{
	RTSP_RMT	msg_type;
	char		msg_str[32];
	int			msg_len;
}RREQMTV;

typedef enum rtsp_context_type
{
	RTSP_CTX_NULL = 0,
	RTSP_CTX_RTSP,
	RTSP_CTX_SDP,
	RTSP_CTX_TXT,
	RTSP_CTX_HTM,
	RTSP_CTX_PIDF,
	RTSP_CTX_DTMFR
}RTSPCTXT;

typedef struct hrtsp_msg_content
{
	unsigned int	msg_type;		//0:request,1:response
	unsigned int	msg_sub_type;	
	HDRV 			first_line;

	PPSN_CTX		rtsp_ctx;
	PPSN_CTX		sdp_ctx;

	int				rtsp_len;
	int				sdp_len;

	RTSPCTXT		ctx_type;
	int				ctx_len;

	char *			msg_buf;
	int				buf_offset;

	unsigned long	remote_ip;
	unsigned short	remote_port;
	unsigned short	local_port;
}HRTSP_MSG;

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************/
BOOL 	is_rtsp_msg(char * msg_buf);

void 	rtsp_headl_parse(char * pline, int llen, HRTSP_MSG * p_msg);
int 	rtsp_line_parse(char * p_buf, int max_len, char sep_char, PPSN_CTX * p_ctx);
int 	rtsp_ctx_parse(HRTSP_MSG * p_msg);
int 	rtsp_msg_parse(char * msg_buf,int msg_buf_len,HRTSP_MSG * msg);
int 	rtsp_msg_parse_part1(char * p_buf,int buf_len,HRTSP_MSG * msg);
int 	rtsp_msg_parse_part2(char * p_buf,int buf_len,HRTSP_MSG * msg);

/*************************************************************************/
HDRV  * find_rtsp_headline(HRTSP_MSG * msg,const char * head);
HDRV  * find_rtsp_sdp_headline(HRTSP_MSG * msg,const char * head);
BOOL 	rtsp_msg_with_sdp(HRTSP_MSG * msg);

BOOL 	match_rtsp_msg_session(HRTSP_MSG * rx_msg,HRTSP_MSG * tx_msg);
BOOL 	match_rtsp_msg_cseq(HRTSP_MSG * rx_msg,HRTSP_MSG * tx_msg);

BOOL 	get_rtsp_headline_uri(HRTSP_MSG * rx_msg,char * p_uri, int size);
BOOL 	get_rtsp_headline_string(HRTSP_MSG * rx_msg,char * head, char * p_value, int size);

BOOL 	get_rtsp_msg_session(HRTSP_MSG * rx_msg,char *session_buf,int len);
BOOL 	get_rtsp_msg_cseq(HRTSP_MSG * rx_msg,char *cseq_buf,int len);
BOOL 	get_rtsp_user_agent_info(HRTSP_MSG * rx_msg,char * agent_buf,int max_len);
BOOL 	get_rtsp_session_info(HRTSP_MSG * rx_msg,char * session_buf,int max_len);
BOOL 	get_rtsp_cbase_info(HRTSP_MSG * rx_msg,char * cbase_buf,int max_len);

BOOL 	rtsp_get_digest_info(HRTSP_MSG * rx_msg,HD_AUTH_INFO * auth_info);

void 	add_rtsp_tx_msg_line(HRTSP_MSG * tx_msg,const char * msg_hdr,const char * msg_fmt,...);
void 	add_rtsp_tx_msg_sdp_line(HRTSP_MSG * tx_msg,const char * msg_hdr,const char * msg_fmt,...);
void 	add_rtsp_tx_msg_fline(HRTSP_MSG * tx_msg,const char * msg_hdr,const char * msg_fmt,...);

void 	copy_rtsp_msg_line(HRTSP_MSG * src_msg,HRTSP_MSG * dst_msg,char * msg_hdr);

void 	free_rtsp_msg(HRTSP_MSG * msg);
void 	free_rtsp_msg_content(HRTSP_MSG * msg);

/*************************************************************************/
BOOL 	get_rtsp_remote_media_ip(HRTSP_MSG * rx_msg,unsigned long * media_ip);

HDRV  * find_rtsp_mdesc_point(HRTSP_MSG * rx_msg,HDRV * pStartHdr,const char * cap_name,int * next_offset);

BOOL 	get_rtsp_remote_cap(HRTSP_MSG * rx_msg,const char * cap_name,int * cap_count,unsigned char * cap_array,unsigned short * rtp_port);
BOOL 	get_rtsp_remote_cap_desc(HRTSP_MSG * rx_msg,const char * cap_name,char cap_desc[][MAX_AVDESCLEN]);

BOOL 	find_rtsp_sdp_control(HRTSP_MSG * rx_msg,char * ctl_buf,char * tname,int max_len);

/*************************************************************************/
BOOL 	rtsp_msg_buf_fl_init(int num);
HRTSP_MSG * get_rtsp_msg_buf();
void 	rtsp_msg_ctx_init(HRTSP_MSG * msg);
void 	free_rtsp_msg_buf(HRTSP_MSG * msg);
uint32 	idle_rtsp_msg_buf_num();
BOOL 	rtsp_parse_buf_init();


#ifdef __cplusplus
}
#endif

#endif	//	__H_RTSP_PARSE_H__



