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
#include "word_analyse.h"
#include "rtsp_parse.h"
#include "rua.h"
#include "rfc_md5.h"

/***********************************************************************/
#define HASHLEN 16
typedef unsigned char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN+1];


void CvtHex(HASH Bin, HASHHEX Hex)
{
	unsigned short i;
	unsigned char j;
	for (i = 0; i < HASHLEN; i++) {
		j = (Bin[i] >> 4) & 0xf;

		if (j <= 9)
			Hex[i*2] = (j + '0');
		else
			Hex[i*2] = (j + 'a' - 10);

		j = Bin[i] & 0xf;

		if (j <= 9)
			Hex[i*2+1] = (j + '0');
		else
			Hex[i*2+1] = (j + 'a' - 10);
	};
	Hex[HASHHEXLEN] = '\0';
};

BOOL calc_auth_digest(RUA * p_rua, HD_AUTH_INFO * auth_info, const char * method)
{
    MD5_CTX Md5Ctx;
	HASH HA1;
	HASH HA2;
	HASH HA3;

	HASHHEX HA1Hex;
	HASHHEX HA2Hex;
	HASHHEX HA3Hex;
	

	MD5Init(&Md5Ctx);
	MD5Update(&Md5Ctx, (unsigned char *)auth_info->auth_name, strlen(auth_info->auth_name));
	MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
	MD5Update(&Md5Ctx, (unsigned char *)auth_info->auth_realm, strlen(auth_info->auth_realm));
	MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
	MD5Update(&Md5Ctx, (unsigned char *)auth_info->auth_pwd, strlen(auth_info->auth_pwd));
	MD5Final(HA1, &Md5Ctx);

	CvtHex(HA1, HA1Hex);

	MD5Init(&Md5Ctx);
	MD5Update(&Md5Ctx, (unsigned char *)method, strlen(method));
	MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
	MD5Update(&Md5Ctx, (unsigned char *)auth_info->auth_uri, strlen(auth_info->auth_uri));
	MD5Final(HA2, &Md5Ctx);

	CvtHex(HA2, HA2Hex);

	MD5Init(&Md5Ctx);
	MD5Update(&Md5Ctx, (unsigned char *)HA1Hex, HASHHEXLEN);
	MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
	MD5Update(&Md5Ctx, (unsigned char *)auth_info->auth_nonce, strlen(auth_info->auth_nonce));
	MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
	MD5Update(&Md5Ctx, (unsigned char *)HA2Hex, HASHHEXLEN);
	MD5Final(HA3, &Md5Ctx);

	CvtHex(HA3, HA3Hex);

	strcpy(auth_info->auth_response, HA3Hex);
	
	return TRUE;
}

HRTSP_MSG * rua_build_describe(RUA * p_rua)
{
	HRTSP_MSG * tx_msg = get_rtsp_msg_buf();
	if(tx_msg == NULL)
	{
		log_print("rua_build_describe::get_rtsp_msg_buf return NULL!!!\r\n");
		return NULL;
	}

	tx_msg->msg_type = 0;
	tx_msg->msg_sub_type = RTSP_MT_DESCRIBE;

	add_rtsp_tx_msg_fline(tx_msg,"DESCRIBE", "%s RTSP/1.0",p_rua->uri);

	add_rtsp_tx_msg_line(tx_msg,"CSeq","%u",p_rua->cseq);

    if (p_rua->need_auth)
    {
        if (p_rua->auth_mode == 1)
        {
            calc_auth_digest(p_rua, &p_rua->user_auth_info, "DESCRIBE");
            
    	    add_rtsp_tx_msg_line(tx_msg,"Authorization","Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"",
    			p_rua->user_auth_info.auth_name, p_rua->user_auth_info.auth_realm,
    			p_rua->user_auth_info.auth_nonce, p_rua->user_auth_info.auth_uri, p_rua->user_auth_info.auth_response);
		}
		else if (p_rua->auth_mode == 0)
		{
    		char buff[512] = {'\0'};
    		char basic[512] = {'\0'};
    		sprintf(buff, "%s:%s", p_rua->user_auth_info.auth_name, p_rua->user_auth_info.auth_pwd);
    		base64_encode((unsigned char *)buff, strlen(buff), basic, sizeof(basic));
    		add_rtsp_tx_msg_line(tx_msg,"Authorization","Basic %s",basic);
		}
	}
	
	add_rtsp_tx_msg_line(tx_msg,"Accept","application/sdp");
	add_rtsp_tx_msg_line(tx_msg,"User-Agent","happytimesoft rtsp client");

	return tx_msg;
}

HRTSP_MSG * rua_build_setup(RUA * p_rua,int type)
{
	HRTSP_MSG * tx_msg = get_rtsp_msg_buf();
	if(tx_msg == NULL)
	{
		log_print("rua_build_setup::get_rtsp_msg_buf return NULL!!!\r\n");
		return NULL;
	}

	tx_msg->msg_type = 0;
	tx_msg->msg_sub_type = RTSP_MT_SETUP;

	if(type == 0)
	{
		if (strncmp(p_rua->v_ctl, "rtsp://", 7) == 0)
			add_rtsp_tx_msg_fline(tx_msg,"SETUP", "%s RTSP/1.0",p_rua->v_ctl);
		else	
		{
			int len = strlen(p_rua->uri);
			if (p_rua->uri[len-1] == '/')
			{
				add_rtsp_tx_msg_fline(tx_msg,"SETUP", "%s%s RTSP/1.0",p_rua->uri,p_rua->v_ctl);
			}

			else
			{
				add_rtsp_tx_msg_fline(tx_msg,"SETUP", "%s/%s RTSP/1.0",p_rua->uri,p_rua->v_ctl);
			}
		}	
	}	
	else
	{
		if (strncmp(p_rua->a_ctl, "rtsp://", 7) == 0)
			add_rtsp_tx_msg_fline(tx_msg,"SETUP", "%s RTSP/1.0",p_rua->a_ctl);
		else
		{
			int len = strlen(p_rua->uri);
			if (p_rua->uri[len-1] == '/')
			{
				add_rtsp_tx_msg_fline(tx_msg,"SETUP", "%s%s RTSP/1.0",p_rua->uri,p_rua->a_ctl);
			}

			else
			{
				add_rtsp_tx_msg_fline(tx_msg,"SETUP", "%s/%s RTSP/1.0",p_rua->uri,p_rua->a_ctl);
			}
		}	
	}
	
	add_rtsp_tx_msg_line(tx_msg,"CSeq","%u",p_rua->cseq);

	if (p_rua->sid[0] != '\0')
		add_rtsp_tx_msg_line(tx_msg,"Session","%s",p_rua->sid);

    if (p_rua->need_auth)
    {
        if (p_rua->auth_mode == 1)
        {
            calc_auth_digest(p_rua, &p_rua->user_auth_info, "SETUP");
        
    	    add_rtsp_tx_msg_line(tx_msg,"Authorization","Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"",
    			p_rua->user_auth_info.auth_name, p_rua->user_auth_info.auth_realm,
    			p_rua->user_auth_info.auth_nonce, p_rua->user_auth_info.auth_uri, p_rua->user_auth_info.auth_response);
		}
		else if (p_rua->auth_mode == 0)
		{
    		char buff[512] = {'\0'};
    		char basic[512] = {'\0'};
    		sprintf(buff, "%s:%s", p_rua->user_auth_info.auth_name, p_rua->user_auth_info.auth_pwd);
    		base64_encode((unsigned char *)buff, strlen(buff), basic, sizeof(basic));
    		add_rtsp_tx_msg_line(tx_msg,"Authorization","Basic %s",basic);
		}
	}
	
	if(type == 0)
		add_rtsp_tx_msg_line(tx_msg,"Transport","RTP/AVP/TCP;unicast;interleaved=%u-%u",p_rua->v_interleaved,p_rua->v_interleaved+1);
	else
		add_rtsp_tx_msg_line(tx_msg,"Transport","RTP/AVP/TCP;unicast;interleaved=%u-%u",p_rua->a_interleaved,p_rua->a_interleaved+1);

	add_rtsp_tx_msg_line(tx_msg,"User-Agent","happytimesoft rtsp client");

	return tx_msg;
}

HRTSP_MSG * rua_build_play(RUA * p_rua)
{
	HRTSP_MSG * tx_msg = get_rtsp_msg_buf();
	if(tx_msg == NULL)
	{
		log_print("rua_build_play::get_rtsp_msg_buf return NULL!!!\r\n");
		return NULL;
	}

	tx_msg->msg_type = 0;
	tx_msg->msg_sub_type = RTSP_MT_PLAY;

	add_rtsp_tx_msg_fline(tx_msg,"PLAY", "%s RTSP/1.0",p_rua->uri);

	add_rtsp_tx_msg_line(tx_msg,"CSeq","%u",p_rua->cseq);

    if (p_rua->need_auth)
    {
        if (p_rua->auth_mode == 1)
        {
            calc_auth_digest(p_rua, &p_rua->user_auth_info, "PLAY");
        
    	    add_rtsp_tx_msg_line(tx_msg,"Authorization","Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"",
    			p_rua->user_auth_info.auth_name, p_rua->user_auth_info.auth_realm,
    			p_rua->user_auth_info.auth_nonce, p_rua->user_auth_info.auth_uri, p_rua->user_auth_info.auth_response);
		}
		else if (p_rua->auth_mode == 0)
		{
    		char buff[512] = {'\0'};
    		char basic[512] = {'\0'};
    		sprintf(buff, "%s:%s", p_rua->user_auth_info.auth_name, p_rua->user_auth_info.auth_pwd);
    		base64_encode((unsigned char *)buff, strlen(buff), basic, sizeof(basic));
    		add_rtsp_tx_msg_line(tx_msg,"Authorization","Basic %s",basic);
		}
	}
	
	add_rtsp_tx_msg_line(tx_msg,"Session","%s",p_rua->sid);
	add_rtsp_tx_msg_line(tx_msg,"Range","npt=0.0-");
	add_rtsp_tx_msg_line(tx_msg,"User-Agent","happytimesoft rtsp client");

	return tx_msg;
}

HRTSP_MSG * rua_build_pause(RUA * p_rua)
{
	HRTSP_MSG * tx_msg = get_rtsp_msg_buf();
	if(tx_msg == NULL)
	{
		log_print("rua_build_play::get_rtsp_msg_buf return NULL!!!\r\n");
		return NULL;
	}

	tx_msg->msg_type = 0;
	tx_msg->msg_sub_type = RTSP_MT_PAUSE;

	add_rtsp_tx_msg_fline(tx_msg,"PAUSE", "%s RTSP/1.0",p_rua->uri);

	add_rtsp_tx_msg_line(tx_msg,"CSeq","%u",p_rua->cseq);

    if (p_rua->need_auth)
    {
        if (p_rua->auth_mode == 1)
        {
            calc_auth_digest(p_rua, &p_rua->user_auth_info, "PAUSE");
        
    	    add_rtsp_tx_msg_line(tx_msg,"Authorization","Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"",
    			p_rua->user_auth_info.auth_name, p_rua->user_auth_info.auth_realm,
    			p_rua->user_auth_info.auth_nonce, p_rua->user_auth_info.auth_uri, p_rua->user_auth_info.auth_response);
		}
		else if (p_rua->auth_mode == 0)
		{
    		char buff[512] = {'\0'};
    		char basic[512] = {'\0'};
    		sprintf(buff, "%s:%s", p_rua->user_auth_info.auth_name, p_rua->user_auth_info.auth_pwd);
    		base64_encode((unsigned char *)buff, strlen(buff), basic, sizeof(basic));
    		add_rtsp_tx_msg_line(tx_msg,"Authorization","Basic %s",basic);
		}
	}
	
	add_rtsp_tx_msg_line(tx_msg,"Session","%s",p_rua->sid);
	add_rtsp_tx_msg_line(tx_msg,"User-Agent","happytimesoft rtsp client");

	return tx_msg;
}


HRTSP_MSG * rua_build_get_parameter(RUA * p_rua)
{
	HRTSP_MSG * tx_msg = get_rtsp_msg_buf();
	if(tx_msg == NULL)
	{
		log_print("rua_build_get_parameter::get_rtsp_msg_buf return NULL!!!\r\n");
		return NULL;
	}

	tx_msg->msg_type = 0;
	tx_msg->msg_sub_type = RTSP_MT_GET_PARAMETER;

	add_rtsp_tx_msg_fline(tx_msg,"GET_PARAMETER", "%s RTSP/1.0",p_rua->uri);

	add_rtsp_tx_msg_line(tx_msg,"CSeq","%u",p_rua->cseq);

    if (p_rua->need_auth)
    {
        if (p_rua->auth_mode == 1)
        {
            calc_auth_digest(p_rua, &p_rua->user_auth_info, "GET_PARAMETER");
        
    	    add_rtsp_tx_msg_line(tx_msg,"Authorization","Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"",
    			p_rua->user_auth_info.auth_name, p_rua->user_auth_info.auth_realm,
    			p_rua->user_auth_info.auth_nonce, p_rua->user_auth_info.auth_uri, p_rua->user_auth_info.auth_response);
		}
		else if (p_rua->auth_mode == 0)
		{
    		char buff[512] = {'\0'};
    		char basic[512] = {'\0'};
    		sprintf(buff, "%s:%s", p_rua->user_auth_info.auth_name, p_rua->user_auth_info.auth_pwd);
    		base64_encode((unsigned char *)buff, strlen(buff), basic, sizeof(basic));
    		add_rtsp_tx_msg_line(tx_msg,"Authorization","Basic %s",basic);
		}
	}
	
	add_rtsp_tx_msg_line(tx_msg,"Session","%s",p_rua->sid);
	add_rtsp_tx_msg_line(tx_msg,"User-Agent","happytimesoft rtsp client");

	return tx_msg;
}


HRTSP_MSG * rua_build_option(RUA * p_rua)
{
    HRTSP_MSG *tx_msg = get_rtsp_msg_buf();
    if(tx_msg == NULL)
	{
		log_print("rua_build_option::get_rtsp_msg_buf return NULL!!!\r\n");
		return NULL;
	}

	tx_msg->msg_type = 0;
	tx_msg->msg_sub_type = RTSP_MT_OPTIONS;

	add_rtsp_tx_msg_fline(tx_msg,"OPTIONS", "%s RTSP/1.0",p_rua->uri);

	add_rtsp_tx_msg_line(tx_msg,"CSeq","%u",p_rua->cseq);
/*
    if (p_rua->need_auth)
    {
        if (p_rua->auth_mode == 1)
        {
            calc_auth_digest(p_rua, &p_rua->user_auth_info, "OPTIONS");
        
    	    add_rtsp_tx_msg_line(tx_msg,"Authorization","Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"",
    			p_rua->user_auth_info.auth_name, p_rua->user_auth_info.auth_realm,
    			p_rua->user_auth_info.auth_nonce, p_rua->user_auth_info.auth_uri, p_rua->user_auth_info.auth_response);
		}
		else if (p_rua->auth_mode == 0)
		{
    		char buff[512] = {'\0'};
    		char basic[512] = {'\0'};
    		sprintf(buff, "%s:%s", p_rua->user_auth_info.auth_name, p_rua->user_auth_info.auth_pwd);
    		base64_encode((unsigned char *)buff, strlen(buff), basic, sizeof(basic));
    		add_rtsp_tx_msg_line(tx_msg,"Authorization","Basic %s",basic);
		}
	}
*/	
	//add_rtsp_tx_msg_line(tx_msg,"Session","%s",p_rua->sid);
	add_rtsp_tx_msg_line(tx_msg,"User-Agent","happytimesoft rtsp client");

	return tx_msg;

}

HRTSP_MSG * rua_build_teardown(RUA * p_rua)
{
	HRTSP_MSG * tx_msg = get_rtsp_msg_buf();
	if(tx_msg == NULL)
	{
		log_print("rua_build_teardown::get_rtsp_msg_buf return NULL!!!\r\n");
		return NULL;
	}

	tx_msg->msg_type = 0;
	tx_msg->msg_sub_type = RTSP_MT_TEARDOWN;

	add_rtsp_tx_msg_fline(tx_msg,"TEARDOWN", "%s RTSP/1.0",p_rua->uri);

	add_rtsp_tx_msg_line(tx_msg,"CSeq","%u",p_rua->cseq);

    if (p_rua->need_auth)
    {
        if (p_rua->auth_mode == 1)
        {
            calc_auth_digest(p_rua, &p_rua->user_auth_info, "TEARDOWN");
        
    	    add_rtsp_tx_msg_line(tx_msg,"Authorization","Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\"",
    			p_rua->user_auth_info.auth_name, p_rua->user_auth_info.auth_realm,
    			p_rua->user_auth_info.auth_nonce, p_rua->user_auth_info.auth_uri, p_rua->user_auth_info.auth_response);
		}
		else if (p_rua->auth_mode == 0)
		{
    		char buff[512] = {'\0'};
    		char basic[512] = {'\0'};
    		sprintf(buff, "%s:%s", p_rua->user_auth_info.auth_name, p_rua->user_auth_info.auth_pwd);
    		base64_encode((unsigned char *)buff, strlen(buff), basic, sizeof(basic));
    		add_rtsp_tx_msg_line(tx_msg,"Authorization","Basic %s",basic);
		}
	}
	
	add_rtsp_tx_msg_line(tx_msg,"Session","%s",p_rua->sid);
	add_rtsp_tx_msg_line(tx_msg,"User-Agent","happytimesoft rtsp client");

	return tx_msg;
}


BOOL get_rtsp_media_info(RUA * p_rua, HRTSP_MSG * rx_msg)
{
	if (rx_msg == NULL || p_rua == NULL)
		return FALSE;

	if (rtsp_msg_with_sdp(rx_msg))
	{
		get_rtsp_remote_cap(rx_msg,"audio",&(p_rua->remote_audio_cap_count),p_rua->remote_audio_cap,&(p_rua->audio_rtp_media.remote_a_port));
		get_rtsp_remote_cap_desc(rx_msg,"audio",p_rua->remote_audio_cap_desc);

		get_rtsp_remote_cap(rx_msg,"video",&(p_rua->remote_video_cap_count),p_rua->remote_video_cap,&(p_rua->video_rtp_media.remote_a_port));
		get_rtsp_remote_cap_desc(rx_msg,"video",p_rua->remote_video_cap_desc);

		return TRUE;
	}
	
	return FALSE;
}


void send_rtsp_msg(RUA * p_rua,HRTSP_MSG * tx_msg)
{
	if (!p_rua->fd) return;

	char * tx_buf;
	int  offset=0;
	char rtsp_tx_buffer[2048];

	if(tx_msg == NULL)return;

	tx_buf = rtsp_tx_buffer;

	offset += sprintf(tx_buf+offset,"%s %s\r\n",tx_msg->first_line.header,tx_msg->first_line.value_string);

	HDRV * pHdrV = (HDRV *)pps_lookup_start(&(tx_msg->rtsp_ctx));
	while (pHdrV != NULL)
	{
		offset += sprintf(tx_buf+offset,"%s: %s\r\n",pHdrV->header,pHdrV->value_string);
		pHdrV = (HDRV *)pps_lookup_next(&(tx_msg->rtsp_ctx),pHdrV);
	}
	pps_lookup_end(&(tx_msg->rtsp_ctx));

	offset += sprintf(tx_buf+offset,"\r\n");

	if(tx_msg->sdp_ctx.node_num != 0)
	{
		HDRV * pHdrV = (HDRV *)pps_lookup_start(&(tx_msg->sdp_ctx));
		while (pHdrV != NULL)
		{
			if((strcmp(pHdrV->header,"pidf") == 0) || (strcmp(pHdrV->header,"text/plain") == 0))
				offset += sprintf(tx_buf+offset,"%s\r\n",pHdrV->value_string);
			else
			{
				if(pHdrV->header[0] != '\0')
					offset += sprintf(tx_buf+offset,"%s=%s\r\n",pHdrV->header,pHdrV->value_string);
				else
					offset += sprintf(tx_buf+offset,"%s\r\n",pHdrV->value_string);
			}

			pHdrV = (HDRV *)pps_lookup_next(&(tx_msg->sdp_ctx),pHdrV);
		}
		pps_lookup_end(&(tx_msg->sdp_ctx));
	}

	// log_print("%s\r\n",tx_buf);

	int slen = send(p_rua->fd,tx_buf,offset,0);
	if(slen <= 0)
	{
		log_print("Send Message Failed!!!\r\n");
		return;
	}
}

BOOL get_sdp_h264_desc(RUA * p_rua, int * pt, char * p_sps, int max_len)
{
	int payload_type = 0, i;
	int rtpmap_len = strlen("a=rtpmap:");

	for(i=0; i<MAX_AVN; i++)
	{
		char * ptr = p_rua->remote_video_cap_desc[i];
		if(memcmp(ptr, "a=rtpmap:", rtpmap_len) == 0)
		{
			char pt_buf[16];
			char code_buf[64];
			int next_offset = 0;
			ptr += rtpmap_len;

			if(GetLineWord(ptr, 0, strlen(ptr), pt_buf, sizeof(pt_buf), &next_offset, WORD_TYPE_NUM) == FALSE)
				return FALSE;
			
			GetLineWord(ptr, next_offset, strlen(ptr)-next_offset, code_buf, sizeof(code_buf),  &next_offset, WORD_TYPE_STRING);
			if(strcmp(code_buf, "H264/90000") == 0)
			{
				payload_type = atoi(pt_buf);
				if(payload_type <= 0)
					return FALSE;
				break;
			}
		}
	}

	if(payload_type == 0)
		return FALSE;

	*pt = payload_type;

	if(p_sps == NULL)	// not need sps, pps parameter
		return TRUE;

	p_sps[0] = '\0';

	char fmtp_buf[32];
	int fmtp_len = sprintf(fmtp_buf,"a=fmtp:%u", payload_type);

	for(i=0; i<MAX_AVN; i++)
	{
		char * ptr = p_rua->remote_video_cap_desc[i];
		if(memcmp(ptr, fmtp_buf, fmtp_len) == 0)
		{
			ptr += rtpmap_len;
			char * p_substr = strstr(ptr, "sprop-parameter-sets=");
			if(p_substr != NULL)
			{
				p_substr += strlen("sprop-parameter-sets=");
				char * p_tmp = p_substr;
				while(*p_tmp != ' ' && *p_tmp != ';' && *p_tmp != '\0') p_tmp++;
				int sps_base64_len = p_tmp - p_substr;
				if(sps_base64_len < max_len)
				{
					memcpy(p_sps, p_substr, sps_base64_len);
					p_sps[sps_base64_len] = '\0';
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}

