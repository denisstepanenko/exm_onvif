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
#include "sys_buf.h"


/***************************************************************************************/
PPSN_CTX * net_buf_fl = NULL;


/***************************************************************************************/
BOOL net_buf_fl_init()
{
	net_buf_fl = pps_ctx_fl_init(128,2048,TRUE);
	if (net_buf_fl == NULL)
		return FALSE;

	log_print("net_buf_fl_init::num = %lu\r\n",net_buf_fl->node_num);

	return TRUE;
}

char * get_idle_net_buf()
{
	return (char *)ppstack_fl_pop(net_buf_fl);
}

void free_net_buf(char * rbuf)
{
	if (rbuf == NULL)
		return;

	if (pps_safe_node(net_buf_fl, rbuf))
		ppstack_fl_push_tail(net_buf_fl,rbuf);	
	else
		free(rbuf);
}

unsigned int idle_net_buf_num()
{
	return net_buf_fl->node_num;
}

void net_buf_fl_deinit()
{
	if (net_buf_fl)
	{
		pps_fl_free(net_buf_fl);
		net_buf_fl = NULL;
	}
}


PPSN_CTX * hdrv_buf_fl = NULL;

BOOL hdrv_buf_fl_init(int num)
{
	hdrv_buf_fl = pps_ctx_fl_init(num,sizeof(HDRV),TRUE);
	if (hdrv_buf_fl == NULL)
		return FALSE;

	log_print("hdrv_buf_fl_init::num = %lu\r\n",hdrv_buf_fl->node_num);
//	pps_ctx_fl_show(hdrv_buf_fl);

	return TRUE;
}

void hdrv_buf_fl_deinit()
{
	if (hdrv_buf_fl)
	{
		pps_fl_free(hdrv_buf_fl);
		hdrv_buf_fl = NULL;
	}
}

HDRV * get_hdrv_buf()
{
	HDRV * p_ret = (HDRV *)ppstack_fl_pop(hdrv_buf_fl);
//	log_print("get_hdrv_buf::num = %d\r\n",hdrv_buf_fl->node_num);
//	pps_ctx_fl_show(hdrv_buf_fl);
	return p_ret;
}

void free_hdrv_buf(HDRV * pHdrv)
{
	if (pHdrv == NULL)
		return;

	pHdrv->header[0] = '\0';
	pHdrv->value_string = NULL;
	ppstack_fl_push(hdrv_buf_fl,pHdrv);
//	log_print("free_hdrv_buf::num = %d,pHdrv=0x%08x\r\n",hdrv_buf_fl->node_num,pHdrv);
//	pps_ctx_fl_show(hdrv_buf_fl);
}

unsigned int idle_hdrv_buf_num()
{
	return hdrv_buf_fl->node_num;
}

void init_ul_hdrv_ctx(PPSN_CTX * ul_ctx)
{
	pps_ctx_ul_init_nm(hdrv_buf_fl,ul_ctx);
}

void free_ctx_hdrv(PPSN_CTX * p_ctx)
{
	HDRV * p_free;

	if (p_ctx == NULL)
		return;

	p_free = (HDRV *)pps_lookup_start(p_ctx);
	while (p_free != NULL) 
	{
		HDRV * p_next = (HDRV *)pps_lookup_next(p_ctx,p_free);
		pps_ctx_ul_del(p_ctx,p_free);
		free_hdrv_buf(p_free);

		p_free = p_next;		
	}
	pps_lookup_end(p_ctx);
}


BOOL sys_buf_init()
{
	if (net_buf_fl_init() == FALSE)
	{
		log_print("sys_buf_init::net_buf_fl_init failed!!!\r\n");
		return FALSE;
	}

	if (hdrv_buf_fl_init(1024) == FALSE)
	{
		log_print("sys_buf_init::hdrv_buf_fl_init failed!!!\r\n");
		return FALSE;
	}
    
	return TRUE;
}

void sys_buf_deinit()
{
	net_buf_fl_deinit();
	hdrv_buf_fl_deinit();
}


