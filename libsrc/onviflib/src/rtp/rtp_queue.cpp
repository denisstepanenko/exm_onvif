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
#include "rtp.h"
#include "h264_rtp_tx.h"
#include "rtp_queue.h"

/***********************************************************************/
int rtp_queue_init(RRXQI * p_rrxqi, int rtp_num, int rtp_size)
{
	memset(p_rrxqi, 0, sizeof(RRXQI));
	p_rrxqi->rtp_fl = pps_ctx_fl_init(rtp_num, rtp_size, TRUE);
	if(p_rrxqi->rtp_fl == NULL)
		return -1;

	p_rrxqi->rtp_ul = pps_ctx_ul_init(p_rrxqi->rtp_fl, TRUE);
	if(p_rrxqi->rtp_ul == NULL)
	{
		pps_fl_free(p_rrxqi->rtp_fl);
		return -1;
	}

	p_rrxqi->queue_nnulEvent = sys_os_create_sig();
	p_rrxqi->rq_running = 1;

	return 0;
}

void rtp_queue_deinit(RRXQI * p_rrxqi)
{
	p_rrxqi->rq_running = 0;

	if(p_rrxqi->queue_nnulEvent)
	{
		sys_os_destroy_sig_mutx(p_rrxqi->queue_nnulEvent);
		p_rrxqi->queue_nnulEvent = NULL;
	}

	if(p_rrxqi->rtp_ul)
	{
		pps_ul_free(p_rrxqi->rtp_ul);
		p_rrxqi->rtp_ul = NULL;
	}

	if(p_rrxqi->rtp_fl)
	{
		pps_fl_free(p_rrxqi->rtp_fl);
		p_rrxqi->rtp_fl = NULL;
	}
}

RQRN * rq_pop_rtp_idle_buffer(RRXQI * p_rrxqi, int * p_size)
{
	if(p_rrxqi->rtp_fl == NULL)
		return NULL;

	*p_size = p_rrxqi->rtp_fl->unit_size - 16;
	RQRN * p_rqn = (RQRN *)ppstack_fl_pop(p_rrxqi->rtp_fl);
	if(p_rqn == NULL) 
	{
		return NULL;
	}

	return p_rqn;
}

void rq_update_seq(RRXQI * p_rrxqi)
{
	if(p_rrxqi->rtp_ul->head_node == NULL)
		return;

	RQRN * p_rqn_head = (RQRN *)_pps_node_get_data((PPSN *)(p_rrxqi->rtp_ul->head_node));
	RQRN * p_rqn_tail = (RQRN *)_pps_node_get_data((PPSN *)(p_rrxqi->rtp_ul->tail_node));

	p_rrxqi->min_ceq = p_rqn_head->seq;
	p_rrxqi->max_ceq = p_rqn_tail->seq;
}


BOOL rq_seq_mid(unsigned short pkt_seq, unsigned short rq_max_seq, unsigned short rq_min_seq)
{
	unsigned short t_cnt = rq_max_seq - rq_min_seq;
	unsigned short i_cnt = rq_max_seq - pkt_seq;
	return (t_cnt > i_cnt);
//	return ((rq_max_seq - pkt_seq) <= (rq_max_seq - rq_min_seq));
}

BOOL rq_seq_left(unsigned short pkt_seq, unsigned short rq_max_seq, unsigned short rq_min_seq)
{
	unsigned short i_cnt = rq_min_seq - pkt_seq;
	unsigned short a_cnt = pkt_seq - rq_max_seq;
	return (i_cnt < a_cnt);
//	return ((rq_min_seq - pkt_seq) < (pkt_seq - rq_max_seq));
}

int rq_push_rtp_pkt(RRXQI * p_rrxqi, RQRN * p_rqn)
{
	unsigned short cseq = p_rqn->seq;

	p_rrxqi->cnt_byte += p_rqn->len;
	p_rrxqi->cnt_pkt++;

	unsigned int cur_time = sys_os_get_ms();
	if(cur_time >= (p_rrxqi->last_pkt_time + 10 * 1000))
	{
		p_rrxqi->rate_byte = p_rrxqi->cnt_byte - p_rrxqi->pre_cnt_byte;
		p_rrxqi->rate_pkt = p_rrxqi->cnt_pkt - p_rrxqi->pre_cn_pkt;

		p_rrxqi->pre_cnt_byte = p_rrxqi->cnt_byte;
		p_rrxqi->pre_cn_pkt = p_rrxqi->cnt_pkt;
		p_rrxqi->last_pkt_time = cur_time;

		log_print("rq_push_rtp_pkt::10s cnt info,byte[%u, %u],pkt[%u, %u]\r\n",
			p_rrxqi->cnt_byte, p_rrxqi->rate_byte, p_rrxqi->cnt_pkt, p_rrxqi->rate_pkt);
	}

	if(pps_safe_node(p_rrxqi->rtp_fl, p_rqn) == FALSE)
	{
		log_print("rq_push_rtp_pkt::pps_safe_node is false, p_rqn[%p]!!!\r\n", p_rqn);
		return -1;
	}

//	if(cseq >= p_rrxqi->min_ceq && cseq <= p_rrxqi->max_ceq) // 中间的包
	if(rq_seq_mid(cseq, p_rrxqi->max_ceq, p_rrxqi->min_ceq))
	{
		rq_insert_rtp_pkt(p_rrxqi, p_rqn);
	}
//	else if(cseq < p_rrxqi->min_ceq) // 前面的包
	else if(rq_seq_left(cseq, p_rrxqi->max_ceq, p_rrxqi->min_ceq))
	{
		unsigned short l_cnt = p_rrxqi->min_ceq - cseq;
		if(l_cnt > 128)
//		if((p_rrxqi->min_ceq - cseq) > 64)
		{
			if((p_rqn->data[0] & 0x1F) != 28)
				log_print("< min_seq[%u]del pkt[%u,%u] len[%u] ltime[%u] [%02X %02X]\r\n",
						p_rrxqi->min_ceq,
						p_rqn->seq, p_rqn->ts, p_rqn->len, p_rqn->ltime, p_rqn->data[0], p_rqn->data[1]);
			else
			{
				BOOL s_flag = ((p_rqn->data[1] & 0x80) == 0x80);
				BOOL e_flag = ((p_rqn->data[1] & 0x40) == 0x40);
				log_print("< min_seq[%u]del pkt[%u,%u] [S=%d E=%d] len[%u] ltime[%u] [%02X %02X]\r\n",
						p_rrxqi->min_ceq,
						p_rqn->seq, p_rqn->ts, s_flag, e_flag, p_rqn->len, p_rqn->ltime, p_rqn->data[0], p_rqn->data[1]);
			}

			ppstack_fl_push(p_rrxqi->rtp_fl, p_rqn);
			return -1;
		}
		rq_insert_rtp_pkt(p_rrxqi, p_rqn);
	}
	else //(cseq > p_rrxqi->max_ceq)
	{
		/*
		unsigned short r_cnt = cseq - p_rrxqi->max_ceq;
		if(r_cnt > 128)
		{
			if((p_rqn->data[0] & 0x1F) != 28)
				log_print("> max_seq[%u]del pkt[%u,%u] len[%u] ltime[%u] [%02X %02X]\r\n",
						p_rrxqi->max_ceq,
						p_rqn->seq, p_rqn->ts, p_rqn->len, p_rqn->ltime, p_rqn->data[0], p_rqn->data[1]);
			else
			{
				BOOL s_flag = ((p_rqn->data[1] & 0x80) == 0x80);
				BOOL e_flag = ((p_rqn->data[1] & 0x40) == 0x40);
				log_print("> max_seq[%u]del pkt[%u,%u] [S=%d E=%d] len[%u] ltime[%u] [%02X %02X]\r\n",
						p_rrxqi->max_ceq,
						p_rqn->seq, p_rqn->ts, s_flag, e_flag, p_rqn->len, p_rqn->ltime, p_rqn->data[0], p_rqn->data[1]);
			}

			ppstack_fl_push(p_rrxqi->rtp_fl, p_rqn);
			return -1;
		}
		*/
		rq_insert_rtp_pkt(p_rrxqi, p_rqn);
	}

	return 0;
}

int rq_insert_rtp_pkt(RRXQI * p_rrxqi, RQRN * p_nrn)
{
	PPSN * p_node, * p_new;
	int insert_flag = 0;

	if((p_nrn->data[0] & 0x1F) != 28)
		log_print("insert pkt[%u,%u] len[%u] ltime[%u] [%02X %02X]\r\n",
				p_nrn->seq, p_nrn->ts, p_nrn->len, p_nrn->ltime, p_nrn->data[0], p_nrn->data[1]);
	else
	{
		BOOL s_flag = ((p_nrn->data[1] & 0x80) == 0x80);
		BOOL e_flag = ((p_nrn->data[1] & 0x40) == 0x40);
		log_print("insert pkt[%u,%u] [S=%d E=%d] len[%u] ltime[%u] [%02X %02X]\r\n",
				p_nrn->seq, p_nrn->ts, s_flag, e_flag, p_nrn->len, p_nrn->ltime, p_nrn->data[0], p_nrn->data[1]);
	}

	if(p_nrn->seq < p_rrxqi->play_cseq) 
	{
		log_print("rq_insert_err::seq[%u] < play_cseq[%u]\r\n",p_nrn->seq,p_rrxqi->play_cseq);
		goto rq_insert_err_exit;
	}

	p_new = _pps_data_get_node(p_nrn);
	if(p_new == NULL)
	{
		log_print("rq_insert_err::seq[%u] _pps_data_get_node ret null[%p]\r\n",p_nrn->seq,p_nrn);
		goto rq_insert_err_exit;
	}

	if(p_new->node_flag == 2)
	{
		log_print("rq_insert_rtp_pkt::unit node already in UsedList!!!\r\n");
		goto rq_insert_err_exit;
	}

	p_new->next_node = p_new->prev_node = 0;

	p_node = _pps_node_tail_start(p_rrxqi->rtp_ul);
	while(p_node)
	{
		RQRN * p_rqn = (RQRN *)_pps_node_get_data(p_node);
		if(p_nrn->seq > p_rqn->seq) 
		{
			p_node->next_node = (unsigned long)p_new;
			p_new->prev_node = (unsigned long)p_node;
			insert_flag = 1;

			if((unsigned long)p_node == p_rrxqi->rtp_ul->tail_node)
				p_rrxqi->rtp_ul->tail_node = (unsigned long)p_new;

			break;
		}
		else if(p_rqn->seq == p_nrn->seq)
		{
			_pps_node_end(p_rrxqi->rtp_ul);
			goto rq_insert_err_exit;
		}

		p_node = _pps_node_prev(p_rrxqi->rtp_ul, p_node);
	}

	if(insert_flag == 0)
	{
		if(p_rrxqi->rtp_ul->head_node == NULL)
		{
			p_rrxqi->rtp_ul->head_node = (unsigned long)p_new;
			p_rrxqi->rtp_ul->tail_node = (unsigned long)p_new;
		}
		else 
		{
			p_node = (PPSN *)(p_rrxqi->rtp_ul->head_node);
			p_node->prev_node = (unsigned long)p_new;
			p_new->next_node = (unsigned long)p_node;
			p_rrxqi->rtp_ul->head_node = (unsigned long)p_new;
		}
	}

	p_new->node_flag = 2;
	p_rrxqi->rtp_ul->node_num++;

	rq_update_seq(p_rrxqi);

	_pps_node_end(p_rrxqi->rtp_ul);

	sys_os_sig_sign(p_rrxqi->queue_nnulEvent);

	return 0;

rq_insert_err_exit:
	ppstack_fl_push(p_rrxqi->rtp_fl, p_nrn);
	return -1;
}

int rq_del_rtp_pkt(RRXQI * p_rrxqi, RQRN * p_rqn)
{
	if(pps_safe_node(p_rrxqi->rtp_fl, p_rqn) == FALSE)
		return -1;

	pps_ctx_ul_del(p_rrxqi->rtp_ul, p_rqn);
	ppstack_fl_push(p_rrxqi->rtp_fl, p_rqn);
	return 0;
}

int rq_del_rtp_head_pkt(RRXQI * p_rrxqi)
{
	RQRN * p_rqn = (RQRN *)pps_lookup_start(p_rrxqi->rtp_ul);
	pps_lookup_end(p_rrxqi->rtp_ul);

	if(p_rqn)
	{
		if((p_rqn->data[0] & 0x1F) != 28)
			log_print("del head pkt[%u,%u] len[%u] ltime[%u] [%02X %02X]\r\n",
					p_rqn->seq, p_rqn->ts, p_rqn->len, p_rqn->ltime, p_rqn->data[0], p_rqn->data[1]);
		else
		{
			BOOL s_flag = ((p_rqn->data[1] & 0x80) == 0x80);
			BOOL e_flag = ((p_rqn->data[1] & 0x40) == 0x40);
			log_print("del head fu pkt[%u,%u] [S=%d E=%d] len[%u] ltime[%u] [%02X %02X]\r\n",
					p_rqn->seq, p_rqn->ts, s_flag, e_flag, p_rqn->len, p_rqn->ltime, p_rqn->data[0], p_rqn->data[1]);
		}

		pps_ctx_ul_del(p_rrxqi->rtp_ul, p_rqn);
		ppstack_fl_push(p_rrxqi->rtp_fl, p_rqn);
	}

	return 0;
}

void rq_del_timeout_pkt(RRXQI * p_rrxqi)
{
	int cnt = 0;
	RQRN * p_rqn = (RQRN *)pps_lookup_start(p_rrxqi->rtp_ul);
	unsigned int cur_ltime = sys_os_get_ms();
	while(p_rqn)
	{
		if(cur_ltime > (4000 + p_rqn->ltime))	
		{
			log_print("Delete Timeout PKT[%u,%u][%02X %02X] ltime[%u,%u,%u]\r\n",
				p_rqn->seq, p_rqn->len, p_rqn->data[0], p_rqn->data[1],
				p_rqn->ltime, cur_ltime, (cur_ltime-p_rqn->ltime));

			RQRN * p_tmp = (RQRN *)pps_ctx_ul_del_unlock(p_rrxqi->rtp_ul, p_rqn);
			ppstack_fl_push(p_rrxqi->rtp_fl, p_rqn);
			if(p_tmp == NULL)
				break;

			p_rqn = p_tmp;
			continue;
		}

		cnt++;
		if(cnt > 256)
		{
			log_print("rq_del_timeout_pkt::rtp_ul link lookback!!!\r\n");
			break;
		}

		p_rqn = (RQRN *)pps_lookup_next(p_rrxqi->rtp_ul, p_rqn);
	}
	pps_lookup_end(p_rrxqi->rtp_ul);

	rq_update_seq(p_rrxqi);
}

PPSN * rq_fu_pkt_check(RRXQI * p_rrxqi, PPSN * p_node, int * loss_flag, unsigned short * p_cur_seq)
{
	RQRN * p_rqn;

	unsigned char nalu, fu_nal;

	int pre_seq = 0;
	int fu_s = 0, fu_e = 0;
	int fu_seq_loss = 0;
	int fu_cnt = 0;
	int retx_cnt = p_rrxqi->retx_seq_cnt;

	*loss_flag = 0;

	while(p_node)
	{
		p_rqn = (RQRN *)_pps_node_get_data(p_node);
		nalu = p_rqn->data[0];

		fu_nal = p_rqn->data[1];

		if(pre_seq == 0)	
		{
			if((fu_nal & 0x80) == 0x80) fu_s = 1;
			if(fu_s == 0)
			{
				fu_seq_loss = 1;
				p_rrxqi->req_retx_seq[p_rrxqi->retx_seq_cnt] = p_rqn->seq - 1; 
				p_rrxqi->retx_seq_cnt++;
			}
		}
		else
		{
			if(p_rqn->seq != (pre_seq + 1))	
			{
				fu_seq_loss = 1;
				p_rrxqi->req_retx_seq[p_rrxqi->retx_seq_cnt] = pre_seq + 1;
				p_rrxqi->retx_seq_cnt++;
			}

			if((fu_nal & 0x80) == 0x80)
			{
			}

			if((fu_nal & 0x40) == 0x40)
			{
				fu_e = 1;
				break;
			}

			if((nalu & 0x1F) != 28)	break;
		}

		pre_seq = p_rqn->seq;
		p_node = _pps_node_next(p_rrxqi->rtp_ul, p_node);
	}

	*p_cur_seq = pre_seq;

	if(fu_seq_loss == 1)
	{
		// RRXQI::retx_seq_cnt,req_retx_seq[]
		*loss_flag = 1;

		log_lock_start("rq_fu_pkt_check::loss fu pkt seq:[");
		for(; retx_cnt<p_rrxqi->retx_seq_cnt; retx_cnt++)
			log_lock_print("%u ",p_rrxqi->req_retx_seq[retx_cnt]);
		log_lock_end("]\r\n");
	}

	return p_node;	
}

int rq_check_loss_pkt(RRXQI * p_rrxqi)
{
	PPSN * p_node;
	RQRN * p_rqn;
	int loss_flag;

	int				loss_cnt = 0;
	unsigned short	loss_seq[MAX_RETX_PKT_NUM];

	unsigned short pre_seq=0;
	unsigned char nalu=0;

	memset(p_rrxqi->req_retx_seq, 0, sizeof(p_rrxqi->req_retx_seq));
	p_rrxqi->retx_seq_cnt = 0;

	p_node =_pps_node_head_start(p_rrxqi->rtp_ul);
	while(p_node)
	{
		p_rqn = (RQRN *)_pps_node_get_data(p_node);
		nalu = p_rqn->data[0] & 0x1F;

		if(nalu == 28)
		{
			p_node = rq_fu_pkt_check(p_rrxqi, p_node, &loss_flag, &pre_seq);
			if(p_node == NULL)
				break;
		}
		else
		{
			if(pre_seq == 0)
			{
			}
			else
			{
				if(p_rqn->seq != (pre_seq + 1))
				{
				//	loss_seq[loss_cnt] = pre_seq + 1; loss_cnt++;
				}
			}

			pre_seq = p_rqn->seq;
		}
		p_node = _pps_node_next(p_rrxqi->rtp_ul, p_node);
	}
	_pps_node_end(p_rrxqi->rtp_ul);
/*
	if(loss_cnt > 0)
	{
		log_lock_start("rq_check_loss_pkt::loss pkt seq:[");
		for(int i=0; i<loss_cnt; i++)
			log_lock_print("%u ",loss_seq[i]);
		log_lock_end("]\r\n");
	}
*/
	return 0;
}

int rq_fu_pkt_calc_len(RRXQI * p_rrxqi, PPSN * p_node, unsigned char * p_dst)
{
	unsigned short seq_s, seq_e;
	int fu_e=0, end_not_fu=0;
	unsigned short cur_seq = 0;
	int fu_len = 1, offset = 1;
	unsigned char nalu = 0, fu_hdr = 0;
	unsigned int fu_ts = 0;
	unsigned int fu_pkt = 0;
	unsigned int fu_ltime = 0;
	unsigned int cur_ltime = sys_os_get_ms();

	PPSN * p_end = p_node;
	RQRN * p_rqn = (RQRN *)_pps_node_get_data(p_node);
	fu_ts = p_rqn->ts;
	fu_hdr = p_rqn->data[1];
	if((fu_hdr & 0x80) != 0x80)
		return -1;

	if(p_dst)
	{
		*(unsigned char *)p_dst = (p_rqn->data[0] & 0x60) | (p_rqn->data[1] & 0x1F);
	}

	fu_ltime = p_rqn->ltime;
	cur_seq = p_rqn->seq -1;
	seq_e = seq_s = p_rqn->seq;

	while(p_end)
	{
		p_rqn = (RQRN *)_pps_node_get_data(p_end);
		if(((p_rqn->data[0] & 0x1F) != 28) || (p_rqn->ts != fu_ts))	
		{
			end_not_fu = 1;
			break;
		}

		fu_hdr = p_rqn->data[1];
		if(p_rqn->seq != (unsigned short)(cur_seq + 1))
		{
			return -1;
		}

		cur_seq = p_rqn->seq;
		fu_len += p_rqn->len - 2;
		fu_pkt++;
		seq_e = p_rqn->seq;

		if(p_dst)
		{
			memcpy(p_dst+offset, p_rqn->data+2, p_rqn->len-2);
			offset += p_rqn->len-2;
		}

		fu_hdr = p_rqn->data[1];
		if((fu_hdr & 0x40) == 0x40) 
			fu_e = 1;

		p_end = (PPSN *)(p_end->next_node);

		if(fu_e == 1)
			break;
	}

	if(end_not_fu == 1) 
	{
		if(cur_ltime > (fu_ltime + 1500))	
			log_print("rq_fu_pkt_calc_len::fu pkt no end, pkt cnt[%d,%u-%u], (%u-%u)=%u\r\n", fu_pkt,seq_s,seq_e, cur_ltime, fu_ltime, (cur_ltime-fu_ltime));
		else
			return -1;
	}
	else
	{
		if(p_end == NULL) return 0;
		if(fu_e != 1) return 0;
	}

	if(p_dst)
	{
		RQRN * p_tmp;
		RQRN * p_del = (RQRN *)_pps_node_get_data(p_node);
		RQRN * p_del_end = (RQRN *)_pps_node_get_data(p_end);

		while(p_del && p_del != p_del_end)
		{
			p_tmp = (RQRN *)pps_ctx_ul_del_unlock(p_rrxqi->rtp_ul, p_del);
			ppstack_fl_push(p_rrxqi->rtp_fl, p_del);
			p_del = p_tmp;
		}
	}

	return fu_len;
}

unsigned char * rq_get_rtp_pkt(RRXQI * p_rrxqi, int * r_size, int * free_flag)
{
	PPSN * p_node;
	RQRN * p_rqn;

	unsigned char * p_rtp_data = NULL;
	*r_size = 0;
	*free_flag = 0;

	unsigned char * p_nalu = NULL;
	unsigned char nalu = 0;

	p_rrxqi->retx_seq_cnt = 0;

rq_get_start:
	if(p_rrxqi->rq_running == 0 || p_rrxqi->rtp_ul == NULL || p_rrxqi->queue_nnulEvent == NULL)
		return NULL;

	if(p_rrxqi->rtp_ul->node_num == 0)
	{
		if(sys_os_sig_wait(p_rrxqi->queue_nnulEvent) != 0)
			return NULL;
	}

	pps_wait_mutex(p_rrxqi->rtp_ul);

	p_node = (PPSN *)(p_rrxqi->rtp_ul->head_node);
	p_rqn = (RQRN *)_pps_node_get_data(p_node);
	if(p_node == NULL || p_rqn == NULL)	{	
		pps_post_mutex(p_rrxqi->rtp_ul);
		goto rq_get_end;
	}

	nalu = p_rqn->data[0];
	if((nalu & 0x1F) == 28)	
	{
		int fu_len = rq_fu_pkt_calc_len(p_rrxqi, p_node, NULL);
		if(fu_len > 0)
		{
			unsigned char * p_fu_buf = (unsigned char *)XMALLOC(fu_len+4);
			if(p_fu_buf == NULL)
			{
				log_print("!!!rq_get_rtp_pkt XMALLOC[%d] failed!!!\r\n", fu_len);
				goto rq_get_end;
			}

			rq_fu_pkt_calc_len(p_rrxqi, p_node, p_fu_buf+4);

			p_rtp_data = p_fu_buf+4;
			*free_flag = 1;
			*r_size = fu_len;
		}
		else
		{
			pps_post_mutex(p_rrxqi->rtp_ul);
			rq_del_timeout_pkt(p_rrxqi);
			if(fu_len == -1)
			{
			//	rq_del_rtp_head_pkt(p_rrxqi);
			//	rq_print_pkt_info(p_rrxqi);

			//	rq_check_loss_pkt(p_rrxqi);
				return NULL;
			}
			goto rq_get_start;
		}
	}
	else
	{
		*free_flag = 0;
		*r_size = p_rqn->len;
		p_rtp_data = p_rqn->data;
	}

rq_get_end:

	rq_update_seq(p_rrxqi);

	pps_post_mutex(p_rrxqi->rtp_ul);

	return p_rtp_data;
}

void rq_print_pkt_info(RRXQI * p_rrxqi)
{
	int cnt = 0;
	if(p_rrxqi->rtp_ul->node_num == 0)
		return;

	log_lock_start("rq pkt info::\r\n");
	RQRN * p_rqn = (RQRN *)pps_lookup_start(p_rrxqi->rtp_ul);
	while(p_rqn)
	{
		log_lock_print("[%u,%u] len[%u] ltime[%u] [%02X %02X]\r\n",
			p_rqn->seq, p_rqn->ts, p_rqn->len, p_rqn->ltime, p_rqn->data[0], p_rqn->data[1]);
		
		cnt++;
		if(cnt > 256)
		{
			log_lock_print("rq_print_pkt_info::rtp_ul link lookback!!!\r\n");
			break;
		}

		p_rqn = (RQRN *)pps_lookup_next(p_rrxqi->rtp_ul, p_rqn);
	}
	pps_lookup_end(p_rrxqi->rtp_ul);
	log_lock_end("]\r\n");
}


