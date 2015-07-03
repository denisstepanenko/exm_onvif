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
#include "word_analyse.h"
#include "rtsp_parse.h"


const RREQMTV rtsp_req_mtvs[]={
	{RTSP_MT_DESCRIBE,		"DESCRIBE",		8},
	{RTSP_MT_ANNOUNCE,		"ANNOUNCE",		8},
	{RTSP_MT_OPTIONS,		"OPTIONS",		7},
	{RTSP_MT_PAUSE,			"PAUSE",		5},
	{RTSP_MT_PLAY,			"PLAY",			4},
	{RTSP_MT_RECORD,		"RECORD",		6},
	{RTSP_MT_REDIRECT,		"REDIRECT",		8},
	{RTSP_MT_SETUP,			"SETUP",		5},
	{RTSP_MT_SET_PARAMETER,	"SET_PARAMETER",13},
	{RTSP_MT_GET_PARAMETER,	"GET_PARAMETER",13},
	{RTSP_MT_TEARDOWN,		"TEARDOWN",		8}
};


BOOL is_rtsp_msg(char * msg_buf)
{
	unsigned int i;
	for(i=0; i<sizeof(rtsp_req_mtvs)/sizeof(RREQMTV);i++)
	{
		if(memcmp(msg_buf, rtsp_req_mtvs[i].msg_str, rtsp_req_mtvs[i].msg_len) == 0)
		{
			return TRUE;
		}
	}

	if(memcmp(msg_buf,"RTSP/1.0",strlen("RTSP/1.0")) == 0)
		return TRUE;

	return FALSE;
}

void rtsp_headl_parse(char * pline, int llen, HRTSP_MSG * p_msg)
{
	char	word_buf[256];
	int		word_len;
	int		next_word_offset;
	BOOL	bHaveNextWord;

	bHaveNextWord = GetLineWord(pline,0,llen,word_buf,
		sizeof(word_buf),&next_word_offset,WORD_TYPE_STRING);
	word_len = strlen(word_buf);
	if(word_len > 0 && word_len < 31)
	{
		memcpy(p_msg->first_line.header,pline,word_len);
		p_msg->first_line.header[word_len] = '\0';

		while(pline[next_word_offset] == ' ')next_word_offset++;

		p_msg->first_line.value_string = pline+next_word_offset;

		if(stricmp(word_buf,"RTSP/1.0") == 0)
		{
			if(bHaveNextWord)
			{
				word_len = sizeof(word_buf);
				bHaveNextWord = GetLineWord(pline,next_word_offset,llen,
											word_buf,sizeof(word_buf),&next_word_offset,WORD_TYPE_NUM);
				word_len = strlen(word_buf);
				if(word_len > 0)
				{
					p_msg->msg_type = 1;
					p_msg->msg_sub_type = atoi(word_buf);
				}
			}
		}
		else
		{
			p_msg->msg_type = 0;
			unsigned int i;
			for(i=0; i<sizeof(rtsp_req_mtvs)/sizeof(RREQMTV);i++)
			{
				if(stricmp(word_buf,(char *)(rtsp_req_mtvs[i].msg_str)) == 0)
				{
					p_msg->msg_sub_type = rtsp_req_mtvs[i].msg_type;
					break;
				}
			}
		}
	}
}

int rtsp_line_parse(char * p_buf, int max_len, char sep_char, PPSN_CTX * p_ctx)
{
	char word_buf[256];
	BOOL bHaveNextLine = TRUE;
	int line_len = 0;
	int parse_len = 0;

	char * ptr = p_buf;

	do{
		if(GetSipLine(ptr, max_len, &line_len, &bHaveNextLine) == FALSE)
		{
			// log_print("rtsp_line_parse::get sip line error!!!\r\n");
			return -1;
		}

		if(line_len == 2)
		{
			return(parse_len + 2);
		}

		int	next_word_offset = 0;
		GetLineWord(ptr,0,line_len-2,word_buf,sizeof(word_buf),&next_word_offset,WORD_TYPE_STRING);
		char nchar = *(ptr + next_word_offset);
		if(nchar != sep_char) // SIP is ':',SDP is '='
		{
			log_print("rtsp_line_parse::format error!!!\r\n");
			return -1;
		}

		next_word_offset++;
		while(ptr[next_word_offset] == ' ') next_word_offset++;

		HDRV * pHdrV = get_hdrv_buf();
		if(pHdrV == NULL)
		{
			log_print("rtsp_line_parse::get_hdrv_buf return NULL!!!\r\n");
			return -1;
		}

		strncpy(pHdrV->header,word_buf,32);
		pHdrV->value_string = ptr+next_word_offset;
		pps_ctx_ul_add(p_ctx, pHdrV);

		ptr += line_len;
		max_len -= line_len;
		parse_len += line_len;

	}while(bHaveNextLine);

	return parse_len;
}

int rtsp_ctx_parse(HRTSP_MSG * p_msg)
{
	int flag = 0;
	RTSPCTXT w_ctx_type;

	HDRV * pHdrV = (HDRV *)pps_lookup_start(&(p_msg->rtsp_ctx));
	while (pHdrV != NULL)
	{
		if(stricmp(pHdrV->header,"Content-Length") == 0)
		{
			p_msg->ctx_len = atol(pHdrV->value_string);
			flag++;
		}
		else if(stricmp(pHdrV->header,"Content-Type") == 0)
		{
			char type_word[64];
			int  next_tmp;
			GetLineWord(pHdrV->value_string,0,strlen(pHdrV->value_string),type_word,sizeof(type_word),&next_tmp,WORD_TYPE_STRING);

			if(stricmp(type_word,"application/sdp") == 0)
				w_ctx_type = RTSP_CTX_SDP;
			else
				w_ctx_type = RTSP_CTX_NULL;

			p_msg->ctx_type = w_ctx_type;
			flag++;
		}
		pHdrV = (HDRV *)pps_lookup_next(&(p_msg->rtsp_ctx),pHdrV);
	}
	pps_lookup_end(&(p_msg->rtsp_ctx));

	if(p_msg->ctx_type && p_msg->ctx_len)
		return 1;

	return 0;
}

int rtsp_msg_parse(char * msg_buf,int msg_buf_len,HRTSP_MSG * msg)
{
	BOOL bHaveNextLine;
	int line_len = 0;
	char * p_buf = msg_buf;

	msg->msg_type = -1;	

	if(GetSipLine(p_buf, msg_buf_len,&line_len,&bHaveNextLine) == FALSE)
		return -1;
	if(line_len > 0)	
		rtsp_headl_parse(p_buf, line_len-2, msg);
	if(msg->msg_type == -1)	
		return -1;

	p_buf += line_len;
	msg->rtsp_len = rtsp_line_parse(p_buf, msg_buf_len-line_len, ':', &(msg->rtsp_ctx));
	if(msg->rtsp_len <= 0)
		return -1;

	p_buf += msg->rtsp_len;
	if(rtsp_ctx_parse(msg) == 1 && msg->ctx_len > 0)
	{
		msg->sdp_len = rtsp_line_parse(p_buf, msg->ctx_len, '=', &(msg->sdp_ctx));
		if(msg->sdp_len < 0)
			return -1;
	}

	return (line_len + msg->rtsp_len + msg->sdp_len);
}

int rtsp_msg_parse_part1(char * p_buf,int buf_len,HRTSP_MSG * msg)
{
	BOOL bHaveNextLine;
	int line_len = 0;

	msg->msg_type = -1;	

	if(GetSipLine(p_buf, buf_len,&line_len,&bHaveNextLine) == FALSE)
		return -1;
	if(line_len > 0)
		rtsp_headl_parse(p_buf, line_len-2, msg);
	if(msg->msg_type == -1)
		return -1;

	p_buf += line_len;
	msg->rtsp_len = rtsp_line_parse(p_buf, buf_len-line_len, ':', &(msg->rtsp_ctx));
	if(msg->rtsp_len <= 0)
		return -1;

	rtsp_ctx_parse(msg);

	return (line_len + msg->rtsp_len);
}

int rtsp_msg_parse_part2(char * p_buf,int buf_len,HRTSP_MSG * msg)
{
	msg->sdp_len = rtsp_line_parse(p_buf, buf_len, '=', &(msg->sdp_ctx));
	if(msg->sdp_len < 0)
		return -1;

	return msg->sdp_len;
}

HDRV * find_rtsp_headline(HRTSP_MSG * msg, const char * head)
{
	if(msg == NULL || head == NULL)
		return NULL;

	HDRV * line = (HDRV *)pps_lookup_start(&(msg->rtsp_ctx));
	while (line != NULL)
	{
		if(stricmp(line->header,head) == 0)
			return line;
		
		line = (HDRV *)pps_lookup_next(&(msg->rtsp_ctx),line);
	}
	pps_lookup_end(&(msg->rtsp_ctx));

	return NULL;
}

BOOL get_rtsp_headline_string(HRTSP_MSG * rx_msg,char * head, char * p_value, int size)
{
	HDRV * rx_head = find_rtsp_headline(rx_msg,head);
	if(rx_head == NULL || p_value == NULL)
		return FALSE;

	if(rx_head->value_string == NULL)
		return FALSE;

	p_value[0] = '\0';

	int len = strlen(rx_head->value_string);
	if(len >= size)
	{
		log_print("get_rtsp_headline_string::%s, value_string(%s) len(%d) > size(%d)\r\n",
			head, rx_head->value_string, len, size);
		return FALSE;
	}

	strcpy(p_value, rx_head->value_string);
	return TRUE;
}

BOOL get_rtsp_headline_uri(HRTSP_MSG * rx_msg,char * p_uri, int size)
{
	char * p_ptr = rx_msg->first_line.value_string;
	if(p_ptr == NULL)
		return FALSE;
	
	char * p_end = p_ptr;
	while(*p_end != ' ') p_end++;

	int len = p_end - p_ptr;
	if(len >= size)
		return FALSE;

	memcpy(p_uri, p_ptr, len);
	p_uri[len] = '\0';
	return TRUE;
}

HDRV * find_rtsp_sdp_headline(HRTSP_MSG * msg,const char * head)
{
	if(msg == NULL || head == NULL)
		return NULL;

	HDRV * line = (HDRV *)pps_lookup_start(&(msg->sdp_ctx));
	while (line != NULL)
	{
		if(stricmp(line->header,head) == 0)
			return line;
		
		line = (HDRV *)pps_lookup_next(&(msg->sdp_ctx),line);
	}
	pps_lookup_end(&(msg->sdp_ctx));

	return NULL;
}


BOOL rtsp_msg_with_sdp(HRTSP_MSG * msg)
{
	if(msg == NULL)
		return FALSE;

	if(msg->sdp_ctx.node_num == 0)
		return FALSE;

	return TRUE;
}

BOOL get_rtsp_msg_session(HRTSP_MSG * rx_msg,char *session_buf,int len)
{
	session_buf[0] = '\0';

	HDRV * rx_id = find_rtsp_headline(rx_msg,"Session");
	if(rx_id == NULL || len <= 0)
		return FALSE;

	int	 next_word_offset;

	GetLineWord(rx_id->value_string,0,strlen(rx_id->value_string),
		session_buf,len,&next_word_offset,WORD_TYPE_STRING);

	return TRUE;
}

BOOL match_rtsp_msg_session(HRTSP_MSG * rx_msg,HRTSP_MSG * tx_msg)
{
	HDRV * rx_id = find_rtsp_headline(rx_msg,"Session");
	HDRV * tx_id = find_rtsp_headline(tx_msg,"Session");

	if(rx_id == NULL || tx_id == NULL)
		return FALSE;

	char rx_word_buf[256],tx_word_buf[256];
	int	 next_word_offset;

	GetLineWord(rx_id->value_string,0,strlen(rx_id->value_string),
		rx_word_buf,sizeof(rx_word_buf),&next_word_offset,WORD_TYPE_STRING);
	GetLineWord(tx_id->value_string,0,strlen(tx_id->value_string),
		tx_word_buf,sizeof(tx_word_buf),&next_word_offset,WORD_TYPE_STRING);

	if(strcmp(rx_word_buf,tx_word_buf) != 0)
		return FALSE;

	return TRUE;
}

BOOL match_rtsp_msg_cseq(HRTSP_MSG * rx_msg,HRTSP_MSG * tx_msg)
{
	HDRV * rx_cseq = find_rtsp_headline(rx_msg,"CSeq");
	HDRV * tx_cseq = find_rtsp_headline(tx_msg,"CSeq");

	if(rx_cseq == NULL || tx_cseq == NULL)
		return FALSE;

	char rx_word_buf[256],tx_word_buf[256];
	int	 next_offset;

	GetLineWord(rx_cseq->value_string,0,strlen(rx_cseq->value_string),
		rx_word_buf,sizeof(rx_word_buf),&next_offset,WORD_TYPE_NUM);
	GetLineWord(tx_cseq->value_string,0,strlen(tx_cseq->value_string),
		tx_word_buf,sizeof(tx_word_buf),&next_offset,WORD_TYPE_NUM);

	if(strcmp(rx_word_buf,tx_word_buf) != 0)
		return FALSE;

	GetLineWord(rx_cseq->value_string,next_offset,strlen(rx_cseq->value_string),
		rx_word_buf,sizeof(rx_word_buf),&next_offset,WORD_TYPE_STRING);
	GetLineWord(tx_cseq->value_string,next_offset,strlen(tx_cseq->value_string),
		tx_word_buf,sizeof(tx_word_buf),&next_offset,WORD_TYPE_STRING);

	if(stricmp(rx_word_buf,tx_word_buf) != 0)
		return FALSE;

	return TRUE;
}

BOOL get_rtsp_msg_cseq(HRTSP_MSG * rx_msg,char *cseq_buf,int len)
{
	HDRV * rx_cseq = find_rtsp_headline(rx_msg,"CSeq");
	if((rx_cseq == NULL) || len <= 0)
		return FALSE;

	int	 next_offset;

	GetLineWord(rx_cseq->value_string,0,strlen(rx_cseq->value_string),
		cseq_buf,len,&next_offset,WORD_TYPE_NUM);

	return TRUE;
}

BOOL get_rtsp_user_agent_info(HRTSP_MSG * rx_msg,char * agent_buf,int max_len)
{
	if(agent_buf == NULL || max_len <= 0)
		return FALSE;

	agent_buf[0] = '\0';

	HDRV * rx_line = find_rtsp_headline(rx_msg,"User-Agent");
	if(rx_line == NULL)
		return FALSE;

	strncpy(agent_buf,rx_line->value_string,max_len);
	return TRUE;
}

BOOL get_rtsp_session_info(HRTSP_MSG * rx_msg,char * session_buf,int max_len)
{
	if(session_buf == NULL || max_len <= 0)
		return FALSE;

	session_buf[0] = '\0';

	HDRV * rx_line = find_rtsp_headline(rx_msg,"Session");
	if(rx_line == NULL)
		return FALSE;

	if (rx_line->value_string)
	{
		char * p = strchr(rx_line->value_string, ';');
		if (!p)
		{
			strncpy(session_buf,rx_line->value_string,max_len);
		}
		else
		{
			*p = '\0';
			strncpy(session_buf,rx_line->value_string,max_len);
			*p = ';';
		}
	}	
	
	return TRUE;
}

BOOL get_rtsp_cbase_info(HRTSP_MSG * rx_msg,char * cbase_buf,int max_len)
{
	if(cbase_buf == NULL || max_len <= 0)
		return FALSE;

	cbase_buf[0] = '\0';

	HDRV * rx_line = find_rtsp_headline(rx_msg,"Content-Base");
	if(rx_line == NULL)
		return FALSE;

	strncpy(cbase_buf,rx_line->value_string,max_len);
	return TRUE;
}

void add_rtsp_tx_msg_line(HRTSP_MSG * tx_msg,const char * msg_hdr,const char * msg_fmt,...)
{
	va_list argptr;
	int slen;

	if(tx_msg == NULL || tx_msg->msg_buf == NULL)
		return;

	HDRV *pHdrV = get_hdrv_buf();
	if(pHdrV == NULL)
	{
		log_print("add_tx_msg_line::get_hdrv_buf return NULL!!!\r\n");
		return;
	}

	pHdrV->value_string = tx_msg->msg_buf + tx_msg->buf_offset;

	strncpy(pHdrV->header,msg_hdr,31);

	va_start(argptr,msg_fmt);

#if	__LINUX_OS__
	slen = vsnprintf(pHdrV->value_string,1600-tx_msg->buf_offset,msg_fmt,argptr);
#else
	slen = vsprintf(pHdrV->value_string,msg_fmt,argptr);
#endif

	va_end(argptr);

	if(slen < 0)
	{
		log_print("add_tx_msg_line::vsnprintf return %d !!!\r\n",slen);
		free_hdrv_buf(pHdrV);
		return;
	}

	pHdrV->value_string[slen] = '\0';
	tx_msg->buf_offset += slen + 1;

	pps_ctx_ul_add(&(tx_msg->rtsp_ctx),pHdrV);
}

void add_rtsp_tx_msg_sdp_line(HRTSP_MSG * tx_msg,const char * msg_hdr,const char * msg_fmt,...)
{
	va_list argptr;
	int slen;

	if(tx_msg == NULL || tx_msg->msg_buf == NULL)
		return;

	HDRV *pHdrV = get_hdrv_buf();
	if(pHdrV == NULL)
	{
		log_print("add_tx_msg_sdp_line::get_hdrv_buf return NULL!!!\r\n");
		return;
	}

	pHdrV->value_string = tx_msg->msg_buf + tx_msg->buf_offset;

	strncpy(pHdrV->header,msg_hdr,31);

	va_start(argptr,msg_fmt);

#if	__LINUX_OS__
	slen = vsnprintf(pHdrV->value_string,1600-tx_msg->buf_offset,msg_fmt,argptr);
#else
	slen = vsprintf(pHdrV->value_string,msg_fmt,argptr);
#endif

	va_end(argptr);

	if(slen < 0)
	{
		log_print("add_tx_msg_sdp_line::vsnprintf return %d !!!\r\n",slen);
		free_hdrv_buf(pHdrV);
		return;
	}

	pHdrV->value_string[slen] = '\0';
	tx_msg->buf_offset += slen + 1;

	pps_ctx_ul_add(&(tx_msg->sdp_ctx),pHdrV);
}

void add_rtsp_tx_msg_fline(HRTSP_MSG * tx_msg,const char * msg_hdr,const char * msg_fmt,...)
{
	va_list argptr;
	int slen;

	if(tx_msg == NULL || tx_msg->msg_buf == NULL)
		return;

	strcpy(tx_msg->first_line.header,msg_hdr);
	tx_msg->first_line.value_string = tx_msg->msg_buf + tx_msg->buf_offset;

	va_start(argptr,msg_fmt);
#if	__LINUX_OS__
	slen = vsnprintf(tx_msg->first_line.value_string,1600-tx_msg->buf_offset,msg_fmt,argptr);
#else
	slen = vsprintf(tx_msg->first_line.value_string,msg_fmt,argptr);
#endif
	va_end(argptr);

	if(slen < 0)
	{
		log_print("add_tx_msg_fline::vsnprintf return %d !!!\r\n",slen);
		return;
	}

	tx_msg->first_line.value_string[slen] = '\0';
	
	tx_msg->buf_offset += slen + 1;
}

void copy_rtsp_msg_line(HRTSP_MSG * src_msg,HRTSP_MSG * dst_msg,char * msg_hdr)
{
	HDRV * src_line = find_rtsp_headline(src_msg,msg_hdr);
	if(src_line == NULL)return;
	
	HDRV * dst_line = get_hdrv_buf();
	if(dst_line == NULL)return;

	strcpy(dst_line->header,src_line->header);

	dst_line->value_string = dst_msg->msg_buf + dst_msg->buf_offset;
	if(dst_line->value_string == NULL)
	{
		free_hdrv_buf(dst_line);
		return;
	}

	strcpy(dst_line->value_string,src_line->value_string);
	dst_msg->buf_offset += strlen(src_line->value_string) + 1;

	pps_ctx_ul_add(&(dst_msg->rtsp_ctx),dst_line);
}

void free_rtsp_msg(HRTSP_MSG * msg)
{
	if(msg == NULL)return;

	free_rtsp_msg_content(msg);
	free_rtsp_msg_buf(msg);
}

void free_rtsp_msg_content(HRTSP_MSG * msg)
{
	if(msg == NULL)	return;

	free_ctx_hdrv(&(msg->rtsp_ctx));
	free_ctx_hdrv(&(msg->sdp_ctx));

	free_net_buf(msg->msg_buf);
}

BOOL get_rtsp_remote_media_ip(HRTSP_MSG * rx_msg,unsigned long * media_ip)
{
	HDRV * pHdr = find_rtsp_sdp_headline(rx_msg,"c");
	if((pHdr != NULL) && (pHdr->value_string != NULL) && (strlen(pHdr->value_string) > 0))
	{
		char tmp_buf[128];
		int next_offset;
		
		GetLineWord(pHdr->value_string,0,strlen(pHdr->value_string),
				tmp_buf,sizeof(tmp_buf),&next_offset,WORD_TYPE_STRING);
		if(stricmp(tmp_buf,"IN") != 0)return FALSE;

		GetLineWord(pHdr->value_string,next_offset,strlen(pHdr->value_string),
				tmp_buf,sizeof(tmp_buf),&next_offset,WORD_TYPE_STRING);
		if(stricmp(tmp_buf,"IP4") != 0)return FALSE;

		GetLineWord(pHdr->value_string,next_offset,strlen(pHdr->value_string),
				tmp_buf,sizeof(tmp_buf),&next_offset,WORD_TYPE_STRING);

		log_print("get_remote_media_ip::media_ip=%s\r\n",tmp_buf);

		if(is_ip_address(tmp_buf))
		{
			*media_ip = inet_addr(tmp_buf);
		//	log_print("get_remote_media_ip::media_ip=0x%08x\r\n",*media_ip);
			return TRUE;
		}
	}

	return FALSE;
}

HDRV * find_rtsp_mdesc_point(HRTSP_MSG * rx_msg,HDRV * pStartHdr,const char * cap_name,int * next_offset)
{
	HDRV * pHdr = pStartHdr;
	char media_type[16];

	for(; pHdr != NULL; pHdr = (HDRV *)pps_lookup_next(&(rx_msg->sdp_ctx),pHdr))
	{
		if(stricmp(pHdr->header,"m") != 0)continue;

		GetLineWord(pHdr->value_string,0,strlen(pHdr->value_string),
				media_type,sizeof(media_type),next_offset,WORD_TYPE_STRING);

		if(stricmp(media_type,cap_name) == 0)
			return pHdr;
	}

	return NULL;
}

BOOL get_rtsp_remote_cap(HRTSP_MSG * rx_msg,const char * cap_name,
					int * cap_count,unsigned char * cap_array,unsigned short * rtp_port)
{
	int next_offset = 0;
	char media_port[16],tmp_buf[64];

	*cap_count = 0;

	HDRV * pHdr = (HDRV *)pps_lookup_start(&(rx_msg->sdp_ctx));

	pHdr = find_rtsp_mdesc_point(rx_msg,pHdr,cap_name,&next_offset);
	if(pHdr == NULL)
	{
		pps_lookup_end(&(rx_msg->sdp_ctx));
		return FALSE;
	}

	GetLineWord(pHdr->value_string,next_offset,strlen(pHdr->value_string),
				media_port,sizeof(media_port),&next_offset,WORD_TYPE_NUM);
	
	GetLineWord(pHdr->value_string,next_offset,strlen(pHdr->value_string),
				tmp_buf,sizeof(tmp_buf),&next_offset,WORD_TYPE_STRING);

	if(stricmp(tmp_buf,"RTP/AVP") != 0)
	{
		pps_lookup_end(&(rx_msg->sdp_ctx));
		return FALSE;
	}

	int count = 0;
	BOOL cap_next_flag = TRUE;
	do{
		cap_next_flag = GetLineWord(pHdr->value_string,next_offset,strlen(pHdr->value_string),
					tmp_buf,sizeof(tmp_buf),&next_offset,WORD_TYPE_NUM);
		if(tmp_buf[0] != '\0')
		{
			if(count >= MAX_AVN)
			{
				pps_lookup_end(&(rx_msg->sdp_ctx));
				return FALSE;
			}

			cap_array[count++] = (unsigned char)atol(tmp_buf);
			*cap_count = count;
		}
	}while(cap_next_flag);

	if(count > 0)
	{
		*rtp_port = (unsigned short)atol(media_port);
		pps_lookup_end(&(rx_msg->sdp_ctx));
		return TRUE;
	}

	pps_lookup_end(&(rx_msg->sdp_ctx));

	return FALSE;
}

BOOL rtsp_get_digest_info(HRTSP_MSG * rx_msg,HD_AUTH_INFO * auth_info)
{
	HDRV * chap_id = NULL;
	
	chap_id = find_rtsp_headline(rx_msg,"WWW-Authenticate");
	if(chap_id == NULL)
		return FALSE;

	char word_buf[128];
	int	 next_offset;

//	memset(auth_info,0,sizeof(HD_AUTH_INFO));
	auth_info->auth_response[0] = '\0';
	auth_info->auth_uri[0] = '\0';

	word_buf[0] = '\0';
	GetLineWord(chap_id->value_string,0,strlen(chap_id->value_string),
		word_buf,sizeof(word_buf),&next_offset,WORD_TYPE_STRING);
	if(strcasecmp(word_buf,"digest") != 0)
		return FALSE;
	
	word_buf[0] = '\0';
	if(GetNameValuePair(chap_id->value_string+next_offset,
		strlen(chap_id->value_string)-next_offset,"realm",word_buf,sizeof(word_buf)) == FALSE)
		return FALSE;
	strcpy(auth_info->auth_realm,word_buf);

	word_buf[0] = '\0';
	if(GetNameValuePair(chap_id->value_string+next_offset,
		strlen(chap_id->value_string)-next_offset,"nonce",word_buf,sizeof(word_buf)) == FALSE)
		return FALSE;
	strcpy(auth_info->auth_nonce,word_buf);
	
	return TRUE;
}

BOOL get_rtsp_remote_cap_desc(HRTSP_MSG * rx_msg,const char * cap_name,char cap_desc[][MAX_AVDESCLEN])
{
	int next_offset = 0;
	int index = 0;

	HDRV * pHdr = (HDRV *)pps_lookup_start(&(rx_msg->sdp_ctx));

	pHdr = find_rtsp_mdesc_point(rx_msg,pHdr,cap_name,&next_offset);
	if(pHdr == NULL)
	{
		pps_lookup_end(&(rx_msg->sdp_ctx));
		return FALSE;
	}

	for(index=0; index<MAX_AVN; index++)
		cap_desc[index][0] = '\0';

	index = 0;
	pHdr = (HDRV *)pps_lookup_next(&(rx_msg->sdp_ctx),pHdr);
	while(pHdr != NULL)
	{
		if(stricmp(pHdr->header,"m") == 0)break;
		if(index >= MAX_AVN)break;

		if(memcmp(pHdr->value_string,"control:trackID",strlen("control:trackID")) != 0)
		{
#if	__LINUX_OS__
			snprintf(cap_desc[index], MAX_AVDESCLEN, "%s=%s", pHdr->header,pHdr->value_string);
#else
			sprintf(cap_desc[index], "%s=%s", pHdr->header,pHdr->value_string);
#endif
			char * p_hd = strstr(cap_desc[index],"H263-2000");
			if(p_hd) 
				memcpy(p_hd,"H263-1998",strlen("H263-1998"));

			index++;
		}

		pHdr = (HDRV *)pps_lookup_next(&(rx_msg->sdp_ctx),pHdr);
	}

	pps_lookup_end(&(rx_msg->sdp_ctx));

	return (index != 0);
}

BOOL find_rtsp_sdp_control(HRTSP_MSG * rx_msg,char * ctl_buf,char * tname,int max_len)
{
	if(rx_msg == NULL || ctl_buf == NULL)
		return FALSE;

	int next_offset = 0;
	int mlen = strlen("control:");
	ctl_buf[0] = '\0';

	HDRV * pHdr = (HDRV *)pps_lookup_start(&(rx_msg->sdp_ctx));

	pHdr = find_rtsp_mdesc_point(rx_msg,pHdr,tname,&next_offset);
	if(pHdr == NULL)
	{
		pps_lookup_end(&(rx_msg->sdp_ctx));
		return FALSE;
	}

	pHdr = (HDRV *)pps_lookup_next(&(rx_msg->sdp_ctx),pHdr);
	while(pHdr != NULL)
	{
		if(stricmp(pHdr->header,"m") == 0)break;

		if(pHdr->value_string && (pHdr->header[0] == 'a') && (memcmp(pHdr->value_string,"control:",mlen) == 0))
		{
			int rlen = strlen(pHdr->value_string) - mlen;
			if(rlen > max_len)rlen = max_len;

			int offset = 0;
			while(pHdr->value_string[offset+mlen] == ' ')offset++;
			strcpy(ctl_buf,pHdr->value_string+offset+mlen);

			pps_lookup_end(&(rx_msg->sdp_ctx));
			return TRUE;
		}

		pHdr = (HDRV *)pps_lookup_next(&(rx_msg->sdp_ctx),pHdr);
	}

	pps_lookup_end(&(rx_msg->sdp_ctx));

	return FALSE;
}


static PPSN_CTX * rtsp_msg_buf_fl = NULL;

BOOL rtsp_msg_buf_fl_init(int num)
{
	rtsp_msg_buf_fl = pps_ctx_fl_init(num,sizeof(HRTSP_MSG),TRUE);
	if(rtsp_msg_buf_fl == NULL)
		return FALSE;

	return TRUE;
}

HRTSP_MSG * get_rtsp_msg_buf()
{
	HRTSP_MSG * tx_msg = (HRTSP_MSG *)ppstack_fl_pop(rtsp_msg_buf_fl);
	if(tx_msg == NULL)
		return NULL;

	memset(tx_msg,0,sizeof(HRTSP_MSG));
	tx_msg->msg_buf = get_idle_net_buf();
	if(tx_msg->msg_buf == NULL)
	{
		free_rtsp_msg_buf(tx_msg);
		return NULL;
	}

	rtsp_msg_ctx_init(tx_msg);

	return tx_msg;
}

void rtsp_msg_ctx_init(HRTSP_MSG * msg)
{
	pps_ctx_ul_init_nm(hdrv_buf_fl,&(msg->rtsp_ctx));
	pps_ctx_ul_init_nm(hdrv_buf_fl,&(msg->sdp_ctx));
}

void free_rtsp_msg_buf(HRTSP_MSG * msg)
{
	ppstack_fl_push(rtsp_msg_buf_fl,msg);
}

unsigned int idle_rtsp_msg_buf_num()
{
	return rtsp_msg_buf_fl->node_num;
}

/***********************************************************************/
BOOL rtsp_parse_buf_init()
{
	BOOL ret = rtsp_msg_buf_fl_init(100);
	return ret;
}


