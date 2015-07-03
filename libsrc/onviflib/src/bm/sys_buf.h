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

#ifndef	__H_SYS_BUF_H__
#define	__H_SYS_BUF_H__

/***************************************************************************************/
#define MAX_AVN				16	
#define MAX_AVDESCLEN		280	
#define MAX_USRL			64	
#define MAX_PWDL			32	
#define MAX_NUML			64
#define MAX_UA_ALT_NUM		8


/***************************************************************************************/
typedef struct header_value
{
	char	header[32];
	char *	value_string;
} HDRV;

typedef struct ua_address_info
{
	unsigned int	ip;
	unsigned short	port;
	unsigned short	type;	// UDP - 0; TCP-PASSIVE - 1; TCP-ACTIVE - 2
	char			user[64];
	char			passwd[64];
} UA_ADDRINFO;

typedef struct ua_media_session_info
{
	int				ua_m_fd;				

	unsigned short	remote_a_port;			
	unsigned short	remote_r_port;			
	unsigned short	local_port;				

	unsigned int	rx_pkt_cnt;				
	unsigned int	tx_pkt_cnt;

	time_t			last_pkt_time;

	unsigned int	internet_local_ip;
	unsigned short	internet_local_port;

	UA_ADDRINFO		remote[MAX_UA_ALT_NUM];
	unsigned int	rip;					
	unsigned short	rport;					
	unsigned int	probe_try_cnt;			

	time_t			v3_echo_rx_time;		
	time_t			v3_echo_tx_time;		
} UA_MEDIA;

typedef struct ua_rtp_info
{
	char *			wav_buf;				
	int				rtp_offset;				
	int				rtp_len;				
	int				rtp_cnt;				
	int				cur_rtp_cnt;			
	unsigned int	rtp_ssrc;				
	unsigned int	rtp_ts;					
	unsigned char	rtp_pt;					
} UA_RTP_INFO;

typedef struct http_digest_auth_info
{
	char			auth_name[MAX_USRL];
	char			auth_pwd[32];
	char			auth_uri[256];			
	char			auth_qop[32];
	char			auth_nonce[64];
	char			auth_cnonce[128];
	char			auth_realm[128];
	int				auth_nc;
	char			auth_ncstr[12];
	char			auth_response[36];
} HD_AUTH_INFO;

extern PPSN_CTX * hdrv_buf_fl;

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************/
BOOL 		 net_buf_fl_init();
void 		 net_buf_fl_deinit();

char 	   * get_idle_net_buf();
void 		 free_net_buf(char * rbuf);
unsigned int idle_net_buf_num();

/***********************************************************************/
BOOL 		 hdrv_buf_fl_init(int num);
void 		 hdrv_buf_fl_deinit();

HDRV       * get_hdrv_buf();
void         free_hdrv_buf(HDRV * pHdrv);
unsigned int idle_hdrv_buf_num();

void         init_ul_hdrv_ctx(PPSN_CTX * ul_ctx);

void         free_ctx_hdrv(PPSN_CTX * p_ctx);

/***********************************************************************/
BOOL 		 sys_buf_init();
void         sys_buf_deinit();
/***********************************************************************/


#ifdef __cplusplus
}
#endif

#endif	//	__H_SYS_BUF_H__



