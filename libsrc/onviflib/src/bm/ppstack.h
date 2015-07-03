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

#ifndef	__H_PPSTACK_H__
#define __H_PPSTACK_H__


/***************************************************************************************/

typedef struct PPSN	//ppstack_node
{
	unsigned long		prev_node;
	unsigned long		next_node;
	unsigned long		node_flag;	//0:idle£»1:in FreeList 2:in UsedList 
}PPSN;

typedef struct PPSN_CTX
{
	unsigned long		fl_base;	
	unsigned int		head_node;	
	unsigned int		tail_node;
	unsigned int		node_num;
	unsigned int		low_offset;
	unsigned int		high_offset;
	unsigned int		unit_size;
	void	*			ctx_mutex;
	unsigned int		pop_cnt;
	unsigned int		push_cnt;
}PPSN_CTX;

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************/

void 	  * _pps_node_get_data(PPSN * p_node);
PPSN 	  * _pps_data_get_node(void * p_data);

PPSN_CTX  * pps_ctx_fl_init(unsigned long node_num,unsigned long content_size,BOOL bNeedMutex);
PPSN_CTX  * pps_ctx_fl_init_assign(unsigned long mem_addr, unsigned long mem_len, unsigned long node_num, unsigned long content_size, BOOL bNeedMutex);

void 		pps_fl_free(PPSN_CTX * fl_ctx);
void 		pps_fl_reinit(PPSN_CTX * fl_ctx);

void 		pps_ctx_show(PPSN_CTX * pps_ctx);
BOOL 		ppstack_fl_push(PPSN_CTX * pps_ctx,void * content_ptr);
BOOL 		ppstack_fl_push_tail(PPSN_CTX * pps_ctx,void * content_ptr);
void 	  * ppstack_fl_pop(PPSN_CTX * pps_ctx);

PPSN_CTX  * pps_ctx_ul_init(PPSN_CTX * fl_ctx,BOOL bNeedMutex);
BOOL 		pps_ctx_ul_init_assign(PPSN_CTX * ul_ctx, PPSN_CTX * fl_ctx,BOOL bNeedMutex);
BOOL 		pps_ctx_ul_init_nm(PPSN_CTX * fl_ctx,PPSN_CTX * ul_ctx);

void 		pps_ul_reinit(PPSN_CTX * ul_ctx);
void 		pps_ul_free(PPSN_CTX * ul_ctx);

BOOL 		pps_ctx_ul_del(PPSN_CTX * ul_ctx,void * content_ptr);

PPSN 	  * pps_ctx_ul_del_node_unlock(PPSN_CTX * ul_ctx,PPSN * p_node);
void 	  * pps_ctx_ul_del_unlock(PPSN_CTX * ul_ctx,void * content_ptr);

BOOL 		pps_ctx_ul_add(PPSN_CTX * ul_ctx,void * content_ptr);
BOOL 		pps_ctx_ul_add_head(PPSN_CTX * ul_ctx,void * content_ptr);

unsigned long pps_get_index(PPSN_CTX * pps_ctx,void * content_ptr);
void 	  * pps_get_node_by_index(PPSN_CTX * pps_ctx,unsigned long index);

PPSN 	  * _pps_node_head_start(PPSN_CTX * pps_ctx);
PPSN 	  * _pps_node_tail_start(PPSN_CTX * pps_ctx);
PPSN 	  * _pps_node_next(PPSN_CTX * pps_ctx, PPSN * p_node);
PPSN 	  * _pps_node_prev(PPSN_CTX * pps_ctx, PPSN * p_node);
void 		_pps_node_end(PPSN_CTX * pps_ctx);

/***************************************************************************************/
void 	  * pps_lookup_start(PPSN_CTX * pps_ctx);
void 	  * pps_lookup_next(PPSN_CTX * pps_ctx, void * ct_ptr);
void		pps_lookup_end(PPSN_CTX * pps_ctx);

void 	  * pps_lookback_start(PPSN_CTX * pps_ctx);
void 	  * pps_lookback_next(PPSN_CTX * pps_ctx, void * ct_ptr);
void 		pps_lookback_end(PPSN_CTX * pps_ctx);

void 		pps_wait_mutex(PPSN_CTX * pps_ctx);
void 		pps_post_mutex(PPSN_CTX * pps_ctx);

BOOL 		pps_safe_node(PPSN_CTX * pps_ctx,void * content_ptr);
BOOL 		pps_idle_node(PPSN_CTX * pps_ctx,void * content_ptr);
BOOL 		pps_exist_node(PPSN_CTX * pps_ctx,void * content_ptr);
BOOL 		pps_used_node(PPSN_CTX * pps_ctx,void * content_ptr);

/***************************************************************************************/
int 		pps_node_count(PPSN_CTX * pps_ctx);
void 	  * pps_get_head_node(PPSN_CTX * pps_ctx);
void 	  * pps_get_tail_node(PPSN_CTX * pps_ctx);
void 	  * pps_get_next_node(PPSN_CTX * pps_ctx, void * content_ptr);
void	  * pps_get_prev_node(PPSN_CTX * pps_ctx, void * content_ptr);

#ifdef __cplusplus
}
#endif

#endif /* __H_PPSTACK_H__ */


