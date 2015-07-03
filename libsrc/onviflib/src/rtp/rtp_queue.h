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

#ifndef	__H_RTP_QUEUE_H__
#define	__H_RTP_QUEUE_H__

typedef struct _IpCamSearchRequest
{
	int magic;	// 0x2398
	int cmd;	// 0x10 - search request; 0x11 - search response
}IpCamSearchRequest;

typedef struct _IpCamSearchResponse
{
	int magic;	// 0x2398
	int cmd;	// 0x10 - search request; 0x11 - search response
	int ip;		// ip address, network byte order
	int port;	// rtsp stream port, network byte order
}IpCamSearchResponse;


typedef struct rq_rtp_node
{
	unsigned int	ltime;			
	unsigned int	len	: 16;		
	unsigned int	seq	: 16;		
	unsigned int	ts;				
	unsigned int	ssrc;			
	unsigned char	data[1];		
}RQRN;

#define MAX_RETX_PKT_NUM	512

typedef struct rtp_rx_queue_info
{
	unsigned int	rq_running;		

	void *			queue_nnulEvent;

	PPSN_CTX *		rtp_fl;			
	PPSN_CTX *		rtp_ul;			

	unsigned short	play_cseq;		

	unsigned short	min_ceq;		
	unsigned short	max_ceq;		
	unsigned long	min_time;		
	unsigned long	max_time;

	unsigned int	cnt_pkt;		
	unsigned int	cnt_byte;		
	unsigned int	pre_cn_pkt;		
	unsigned int	pre_cnt_byte;	
	unsigned int	rate_pkt;		
	unsigned int	rate_byte;		

	unsigned int	zero_rate_time;	
	unsigned int	last_pkt_time;

	int				retx_seq_cnt;	
	unsigned short	req_retx_seq[MAX_RETX_PKT_NUM];	

}RRXQI;

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************/
int rtp_queue_init(RRXQI * p_rrxqi, int rtp_num, int rtp_size);
void rtp_queue_deinit(RRXQI * p_rrxqi);

RQRN * rq_pop_rtp_idle_buffer(RRXQI * p_rrxqi, int * p_size);

int rq_push_rtp_pkt(RRXQI * p_rrxqi, RQRN * p_rqn);
int rq_insert_rtp_pkt(RRXQI * p_rrxqi, RQRN * p_nrn);

int rq_del_rtp_pkt(RRXQI * p_rrxqi, RQRN * p_rn);
int rq_del_rtp_head_pkt(RRXQI * p_rrxqi);
void rq_del_timeout_pkt(RRXQI * p_rrxqi);

PPSN * rq_fu_pkt_check(RRXQI * p_rrxqi, PPSN * p_node, int * loss_flag, unsigned short * p_cur_seq);
int rq_check_loss_pkt(RRXQI * p_rrxqi);

int rq_fu_pkt_calc_len(RRXQI * p_rrxqi, PPSN * p_node, unsigned char * p_dst);
unsigned char * rq_get_rtp_pkt(RRXQI * p_rrxqi, int * r_size, int * free_flag);

void rq_print_pkt_info(RRXQI * p_rrxqi);

#ifdef __cplusplus
}
#endif

#endif	//	__H_RTP_QUEUE_H__

