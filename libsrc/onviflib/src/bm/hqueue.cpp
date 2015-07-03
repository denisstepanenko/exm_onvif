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
#include "hqueue.h"

/***************************************************************************************/
HQUEUE * hqCreate(unsigned int unit_num,unsigned int unit_size,unsigned int queue_mode)
{
	unsigned int q_len = unit_num * unit_size + sizeof(HQUEUE);

	HQUEUE *phq = (HQUEUE *)XMALLOC(q_len);
	if(phq == NULL)
	{
		printf("hqCreate XMALLOC HQUEUE fail\n");
		return NULL;
	}
	phq->queue_mode = queue_mode;
	phq->unit_size = unit_size;
	phq->unit_num = unit_num;
	phq->front = 0;
	phq->rear = 0;
	phq->count_put_full = 0;
	phq->queue_buffer = sizeof(HQUEUE);

	if(queue_mode & HQ_NO_EVENT)
	{
	}
	else
	{
		phq->queue_nnulEvent = sys_os_create_sig();
		phq->queue_nfulEvent = sys_os_create_sig();
		phq->queue_putMutex = sys_os_create_mutex();
	}

	return phq;
}

void hqDelete(HQUEUE * phq)
{
	if(phq == NULL)return;

	if(phq->queue_mode & HQ_NO_EVENT)
	{
	}
	else
	{
		sys_os_destroy_sig_mutx(phq->queue_nnulEvent);
		sys_os_destroy_sig_mutx(phq->queue_nfulEvent);
		sys_os_destroy_sig_mutx(phq->queue_putMutex);
	}

	XFREE(phq);
}

/***************************************************************************************/
__inline void hqPutMutexEnter(HQUEUE * phq)
{
	if((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_mutex_enter(phq->queue_putMutex);
	}
}

__inline void hqPutMutexLeave(HQUEUE * phq)
{
	if((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_mutex_leave(phq->queue_putMutex);
	}
}

BOOL hqBufPut(HQUEUE * phq,char * buf)
{
	unsigned int real_rear,queue_count;
	unsigned int mem_addr;

	if(phq == NULL || buf == NULL)
	{
		log_print("hqBufPut::phq=%p,buf=%p!!!",phq,buf);
		return FALSE;
	}

	hqPutMutexEnter(phq);

hqBufPut_start:

	queue_count = phq->rear - phq->front;

	if(queue_count == (phq->unit_num - 1))
	{
		if((phq->queue_mode & HQ_NO_EVENT) == 0)
		{
			if(phq->queue_mode & HQ_PUT_WAIT){
				sys_os_sig_wait(phq->queue_nfulEvent);
				goto hqBufPut_start;
			}
			else {
				phq->count_put_full++;
				hqPutMutexLeave(phq);
				log_print("hqBufPut::queue_count=%d,full!!!",queue_count);
				return FALSE;
			}
		}
		else
		{
			hqPutMutexLeave(phq);
            return FALSE;
		}
	}

	real_rear = phq->rear % phq->unit_num;
	mem_addr = phq->queue_buffer + real_rear*phq->unit_size + (unsigned long)phq;
	memcpy((char *)mem_addr,buf,phq->unit_size);
	phq->rear++;

	if((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_sig_sign(phq->queue_nnulEvent);
	}

	hqPutMutexLeave(phq);

	return TRUE;
}

BOOL hqBufGet(HQUEUE * phq,char * buf)
{
	unsigned int real_front;

	if(phq == NULL || buf == NULL)
		return FALSE;

hqBufGet_start:

	if(phq->front == phq->rear)
	{
		if((phq->queue_mode & HQ_NO_EVENT) == 0)
		{
			if(phq->queue_mode & HQ_GET_WAIT){
				sys_os_sig_wait(phq->queue_nnulEvent);
				goto hqBufGet_start;
			}
			else {
				return FALSE;
			}
		}
		else
			return FALSE;
	}

	real_front = phq->front % phq->unit_num;
	memcpy(buf,((char *)phq) + phq->queue_buffer + real_front*phq->unit_size,phq->unit_size);
	phq->front++;

	if((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_sig_sign(phq->queue_nfulEvent);
	}

	return TRUE;
}

BOOL hqBufIsEmpty(HQUEUE * phq)
{
	if(phq == NULL)
		return TRUE;

	if(phq->front == phq->rear)
		return TRUE;

	return FALSE;
}

char * hqBufGetWait(HQUEUE * phq)
{
	unsigned int real_front;
	char * ptr = NULL;

	if(phq == NULL)
		return NULL;

hqBufGet_start:

	if(phq->front == phq->rear)
	{
		if((phq->queue_mode & HQ_NO_EVENT) == 0)
		{
			if(phq->queue_mode & HQ_GET_WAIT){
				sys_os_sig_wait(phq->queue_nnulEvent);
				goto hqBufGet_start;
			}
			else {
				return NULL;
			}
		}
		else
			return NULL;
	}

	real_front = phq->front % phq->unit_num;

	ptr = ((char *)phq) + phq->queue_buffer + real_front*phq->unit_size;
	return ptr;
}

// memcpy(buf,((char *)phq) + phq->queue_buffer + real_front*phq->unit_size,phq->unit_size);

void hqBufGetWaitPost(HQUEUE * phq)
{
	if(phq == NULL)
		return;

	phq->front++;

	if((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_sig_sign(phq->queue_nfulEvent);
	}
}

BOOL hqBufPeek(HQUEUE * phq,char * buf)
{
	unsigned int real_front;

	if(phq == NULL || buf == NULL)
		return FALSE;

hqBufPeek_start:

	if(phq->front == phq->rear)
	{
		if((phq->queue_mode & HQ_NO_EVENT) == 0)
		{
			if(phq->queue_mode & HQ_GET_WAIT){
				sys_os_sig_wait(phq->queue_nnulEvent);
				goto hqBufPeek_start;
			}
			else {
				return FALSE;
			}
		}
		else
			return FALSE;
	}

	real_front = phq->front % phq->unit_num;
	memcpy(buf,((char *)phq) + phq->queue_buffer + real_front*phq->unit_size,phq->unit_size);

	return TRUE;
}




