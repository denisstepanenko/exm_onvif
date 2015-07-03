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
#include "util.h"
#include "hqueue.h"
#include "linked_list.h"
#include "h264_def.h"
#include "rtp.h"
#include "h264_rtp_tx.h"

/***********************************************************************/
unsigned char * h264_split_nalu_tx(unsigned char * e_buf, int e_len, int * d_len)
{
	int e_i = 4;

	*d_len = 0;
	unsigned int tmp_w;
	unsigned int split_w = 0x01000000;

	memcpy(&tmp_w, e_buf, 4); if(tmp_w != split_w) return NULL;

	while(e_i < e_len)
	{
		if(e_i >= (e_len-4))
		{
			e_i = e_len;
			break;
		}

		memcpy(&tmp_w, e_buf+e_i, 4); if(tmp_w == split_w)
		{
			break;
		}

		e_i++;
	}

	*d_len = e_i;

	if(e_i >= e_len)
		return NULL;

	return (e_buf+e_i);	
}

int h264_rtp_build_tcp(int vfd, UA_RTP_INFO * p_uari, unsigned char * p_data, int len, unsigned char v_chan)
{	
	RILF * p_rilf = (RILF *)(p_data - 12 - 4);	
	p_rilf->magic = 0x24;
	p_rilf->channel = v_chan;
	p_rilf->rtp_len = htons(12 + len);

	rtp_hdr_t * p_rtp_hdr = (rtp_hdr_t *)(p_data - 12);

	p_rtp_hdr->version = RTP_VERSION;
	p_rtp_hdr->p = 0;
	p_rtp_hdr->x = 0;
	p_rtp_hdr->cc = 0;
	p_rtp_hdr->pt = p_uari->rtp_pt;
	p_rtp_hdr->ssrc = p_uari->rtp_ssrc;
	p_rtp_hdr->seq = htons((unsigned short)(p_uari->rtp_cnt));
	p_rtp_hdr->ts = htonl(p_uari->rtp_ts * 90);

	if(p_uari->rtp_cnt == 0)
	{
		p_rtp_hdr->m = 1;
	}
	else
		p_rtp_hdr->m = 0;

	send(vfd, (char *)(p_data-16), len+16, 0);

	p_uari->rtp_cnt++;

	return 0;
}

int h264_rtp_build(int vfd, UA_RTP_INFO * p_uari, unsigned char * p_data, int len, 
		unsigned int rip, unsigned short rport, int is_tcp, unsigned char v_chan)
{
	int slen = 0;
	unsigned char * p_rtp_ptr = p_data - 12;	

	if(is_tcp == 1)
	{
		RILF * p_rilf = (RILF *)(p_rtp_ptr - 4);
		p_rilf->magic = 0x24;
		p_rilf->channel = v_chan; //p_rua->v_interleaved;
		p_rilf->rtp_len = htons(12 + len);
	}

	rtp_hdr_t * p_rtp_hdr = (rtp_hdr_t *)p_rtp_ptr;

	p_rtp_hdr->version = RTP_VERSION;
	p_rtp_hdr->p = 0;
	p_rtp_hdr->x = 0;
	p_rtp_hdr->cc = 0;
	p_rtp_hdr->pt = p_uari->rtp_pt;
	p_rtp_hdr->ssrc = p_uari->rtp_ssrc;
	p_rtp_hdr->seq = htons((unsigned short)(p_uari->rtp_cnt));
	p_rtp_hdr->ts = htonl(p_uari->rtp_ts * 90);

	if(p_uari->rtp_cnt == 0)
	{
		p_rtp_hdr->m = 1;
	}
	else
		p_rtp_hdr->m = 0;

	if(is_tcp == 1)
	{
		slen = send(vfd, (char *)(p_rtp_ptr-4), 4+12+len, 0);
	}
	else
	{
		slen = v_rtp_tx(vfd, rip, rport, (char *)p_rtp_ptr,12+len);
	}

	p_uari->rtp_cnt++;

	return slen;
}


int h264_fu_rtp_build(int vfd, UA_RTP_INFO * p_uari, unsigned char * p_data, int len, 
		unsigned int rip, unsigned short rport, int is_tcp, unsigned char v_chan)
{
	int slen = 0;
	unsigned char tmp_buf[32];

	int first_flag = 1;
	unsigned char * p_start = p_data + 1;
	unsigned char nalu = p_data[0];
	int remain_len = len - 1;

	while(remain_len > 0)	
	{
		unsigned char * p_rtp_dst = p_start-12-2;	
		unsigned char * p_rtp_ptr = tmp_buf+4;
	
		rtp_hdr_t * p_rtp_hdr = (rtp_hdr_t *)p_rtp_ptr;
		p_rtp_hdr->version = RTP_VERSION;
		p_rtp_hdr->p = 0;
		p_rtp_hdr->x = 0;
		p_rtp_hdr->cc = 0;
		p_rtp_hdr->pt = p_uari->rtp_pt;
		p_rtp_hdr->ssrc = p_uari->rtp_ssrc;
		p_rtp_hdr->ts = htonl(p_uari->rtp_ts * 90);

/*
The FU indicator octet has the following format:

      +---------------+
      |0|1|2|3|4|5|6|7|
      +-+-+-+-+-+-+-+-+
      |F|NRI|  Type   |
      +---------------+

The FU header has the following format:

      +---------------+
      |0|1|2|3|4|5|6|7|
      +-+-+-+-+-+-+-+-+
      |S|E|R|  Type   |
      +---------------+
*/

		p_rtp_ptr[12] = (nalu & 0x60) | 28;
		p_rtp_ptr[13] = (nalu & 0x1F);

		if(first_flag == 1)
		{
			p_rtp_ptr[13] |= 0x80;
			first_flag = 0;
		}

		if(remain_len <= H264_RTP_MAX_LEN)
			p_rtp_ptr[13] |= 0x40;

		slen = (remain_len <= H264_RTP_MAX_LEN) ? remain_len : H264_RTP_MAX_LEN;

		remain_len -= slen;
		p_start += slen;

		if(p_uari->rtp_cnt == 0)
			p_rtp_hdr->m = 1;
		else
			p_rtp_hdr->m = 0;

		p_rtp_hdr->seq = htons((unsigned short)(p_uari->rtp_cnt));

		if(is_tcp == 1)
		{
			RILF * p_rilf = (RILF *)(p_rtp_ptr-4);
			p_rilf->magic = 0x24;
			p_rilf->channel = v_chan;	//p_rua->v_interleaved;
			p_rilf->rtp_len = htons(12+2+slen);

			memcpy(p_rtp_dst-4, p_rtp_ptr-4, 12+2+4);
			slen = send(vfd, (char *)(p_rtp_dst-4), 4+12+2+slen, 0);
		}
		else
		{
			memcpy(p_rtp_dst, p_rtp_ptr, 12+2);
			slen = v_rtp_tx(vfd, rip, rport, (char *)p_rtp_dst,12+2+slen);
		}

		p_uari->rtp_cnt++;
		usleep(1000);
	}

	return 0;
}


int v_rtp_tx(int vfd, unsigned int rip, unsigned short rport, char * p_rtp_data, int len)
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = rip;
	addr.sin_port = htons(rport);

	int rlen = sendto(vfd, p_rtp_data, len,0,
				(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
	if(rlen != len)
	{
		printf("v_rtp_tx::rlen = %d,slen = %d,ip=0x%08x\r\n",rlen,len,ntohl(rip));
	}

	return rlen;
}


int encode_sps_pps_base64(H264SPS * p_avcc)
{
	char tmp_sps[512], tmp_pps[32];

	if(base64_encode(p_avcc->p_sps, p_avcc->sps_len, tmp_sps, sizeof(tmp_sps)-1) == 0)
	{
		log_print("encode_sps_pps_base64::base64_encode failed!!!\r\n");
		return -1;
	}

	log_print("encode_sps_pps_base64::tmp_sps(%d)%s\r\n",strlen(tmp_sps), tmp_sps);

	if(base64_encode(p_avcc->p_pps, p_avcc->pps_len, tmp_pps, sizeof(tmp_pps)-1) == 0)
	{
		log_print("encode_sps_pps_base64::base64_encode failed!!!\r\n");
		return -1;
	}

	log_print("encode_sps_pps_base64::tmp_pps(%d)%s\r\n",strlen(tmp_pps), tmp_pps);

	if((strlen(tmp_sps) + strlen(tmp_pps) + 2) > sizeof(p_avcc->sdp_sps))
	{
		log_print("encode_sps_pps_base64::base64 len > len of sdp_sps!!!\r\n");
		return -1;
	}

	sprintf(p_avcc->sdp_sps, "%s,%s", tmp_sps, tmp_pps);

	sprintf(p_avcc->sdp_profile_id, "%02X%02X%02X", p_avcc->p_sps[1],p_avcc->p_sps[2],p_avcc->p_sps[3]);

	return 0;
}

BOOL decode_sps_pps_base64(char * p_sps_str, H264SPS * p_avcc)
{
	memset(p_avcc, 0, sizeof(H264SPS));

	strcpy(p_avcc->sdp_sps, p_sps_str);

	char * ptr = p_avcc->sdp_sps;
	while(*ptr != ',' && *ptr !='\0') ptr++;
	if(*ptr == '\0')	
		return FALSE;
	*ptr = '\0';
	ptr++;

	int len = base64_decode(p_avcc->sdp_sps, p_avcc->avcc+4, sizeof(p_avcc->avcc)-4);
	if(len <= 0)
		return FALSE;

	p_avcc->avcc[0] = p_avcc->avcc[1] = p_avcc->avcc[2] = 0x00; p_avcc->avcc[3] = 0x01;

	unsigned char nalu = p_avcc->avcc[4] & 0x1F;
	if(nalu == 7)	// SPS
	{
		p_avcc->p_sps = p_avcc->avcc + 4;
		p_avcc->sps_len = len;
	}
	else if(nalu == 8)	// PPS
	{
		p_avcc->p_pps = p_avcc->avcc + 4;
		p_avcc->pps_len = len;
	}

	int avcc_off = len + 8;

	len = base64_decode(ptr, p_avcc->avcc+avcc_off, sizeof(p_avcc->avcc)-avcc_off);
	if(len <= 0)
		return FALSE;

	p_avcc->avcc[avcc_off-4] = p_avcc->avcc[avcc_off-3] = p_avcc->avcc[avcc_off-2] = 0x00; p_avcc->avcc[avcc_off-1] = 0x01;

	nalu = p_avcc->avcc[avcc_off] & 0x1F;
	if(nalu == 7)	// SPS
	{
		p_avcc->p_sps = p_avcc->avcc + avcc_off;
		p_avcc->sps_len = len;
	}
	else if(nalu == 8)	// PPS
	{
		p_avcc->p_pps = p_avcc->avcc + avcc_off;
		p_avcc->pps_len = len;
	}

	if(p_avcc->p_pps && p_avcc->p_sps)
	{
		strcpy(p_avcc->sdp_sps, p_sps_str);	
		return TRUE;
	}

	return FALSE;
}

void print_avcc(H264SPS * p_avcc)
{
	int i;
	char str_tmp[512];

	for(i=0; i<p_avcc->sps_len; i++) sprintf(str_tmp+i*3,"%02x ", p_avcc->p_sps[i]);
	log_print("AVCC::SPS[%08x,%d]: %s\r\n", p_avcc->p_sps, p_avcc->sps_len, str_tmp);

	for(i=0; i<p_avcc->pps_len; i++) sprintf(str_tmp+i*3,"%02x ", p_avcc->p_pps[i]);
	log_print("AVCC::PPS[%08x,%d]: %s\r\n", p_avcc->p_pps, p_avcc->pps_len, str_tmp);

	log_print("AVCC::sdp_profile_id[%08x]: %s\r\n", p_avcc->sdp_profile_id, p_avcc->sdp_profile_id);
	log_print("AVCC::sdp_sps[%08x]: %s\r\n", p_avcc->sdp_sps, p_avcc->sdp_sps);
}

int sps_pps_decodec(H264SPS * p_avcc)
{
	print_avcc(p_avcc);

	unsigned char tmp_buf[512];
	unsigned char pre_code[4] = {0x00,0x00,0x00,0x01};

	memcpy(tmp_buf, pre_code, 4);
	memcpy(tmp_buf+4, p_avcc->p_sps, p_avcc->sps_len);

	memcpy(tmp_buf+4+p_avcc->sps_len, pre_code, 4);
	memcpy(tmp_buf+8+p_avcc->sps_len, p_avcc->p_pps, p_avcc->pps_len);

	int len = p_avcc->sps_len + p_avcc->pps_len +8;
/*
	for(int i=0; i<len; i++) sprintf(str_tmp+i*3,"%02x ", tmp_buf[i]);
	log_print("sps_pps_decodec::SPS/PPS: %s\r\n", str_tmp);
*/
//	return ff_h264_decode(g_h264_rxinfo.m_dec_handle, tmp_buf, len);
	return 0;
}

int h264_tx_avcc(H264SPS * p_avcc, unsigned char * p_data, int len)
{
	if(p_avcc->sdp_sps[0] != '\0')	
		return 0;

	unsigned char nal_unit_type = p_data[4] & 0x1F;
	if(nal_unit_type != NALU_TYPE_SPS && nal_unit_type != NALU_TYPE_PPS)
		return 0;

	int nal_len = 0;
	unsigned char * p_cur = p_data;

	while(p_cur)
	{
		unsigned char * p_next = h264_split_nalu_tx(p_cur, len, &nal_len);
		if(nal_len < 5)
			return 0;

		len -= nal_len;

		nal_unit_type = p_cur[4] & 0x1F;

		log_print("h264_tx_avcc-sps-pps::len=%d,[%02x %02x %02x %02x %02x]", nal_len, p_cur[0],p_cur[1],p_cur[2],p_cur[3],p_cur[4]);
		if(nal_unit_type == NALU_TYPE_SPS && p_avcc->sps_len == 0)
		{
			if((nal_len-4) < (512-32))
			{
				int offset = p_avcc->pps_len;
				memcpy(p_avcc->avcc+offset, p_cur+4, nal_len-4);
				p_avcc->p_sps = p_avcc->avcc+offset;
				p_avcc->sps_len = nal_len-4;
			}
		}
		else if(nal_unit_type == NALU_TYPE_PPS && p_avcc->pps_len == 0)
		{
			if(nal_len <16)	
			{
				int offset = p_avcc->sps_len;
				memcpy(p_avcc->avcc+offset, p_cur+4, nal_len-4);
				p_avcc->p_pps = p_avcc->avcc+offset;
				p_avcc->pps_len = nal_len-4;
			}
		}

		log_print("h264_tx_avcc-sps-pps::[%d,%d][%s]",p_avcc->sps_len, p_avcc->pps_len, p_avcc->sdp_sps);
		if(p_avcc->sps_len && p_avcc->pps_len)
		{
			encode_sps_pps_base64(p_avcc);
/*
			H264ParamSet();

			SIPTM stm;
			memset(&stm, 0, sizeof(SIPTM));
			stm.msg_param = PUEVT_M_VIDEO_OK;
			stm.msg_src = MEDIA_VIDEO_SRC;
			hqBufPut(hsip.sip_msg_queue, (char *)&stm);

			log_print("h264_tx_avcc::send PUEVT_M_VIDEO_OK message");
*/
			break;
		}

		p_cur = p_next;
	}

	return 1;
}


