
#include <stdlib.h>
#include <stdio.h>
#include <typedefine.h>
#include "rtpool.h"
#include <pthread.h>
#include <string.h>
/**********************************************************************************************
 * 函数名	:mkrtpool()
 * 功能	:创建一个元素长为plen,可容纳num个元素的缓冲池
 * 输入	:plen:缓冲池容纳的最大元素字节数
 *			 num:缓冲池中的元素总数
 * 输出	:head:描述缓冲池结构的指针，返回时填充内容
 * 返回值	:0表示成功负值表示出错
 **********************************************************************************************/
int mkrtpool(struct pool_head_struct *head,int plen,int num)
{//创建缓冲池
	int mem_len,i,ret,j;
	struct pool_ele_struct *newele,*oldele;
	struct pool_ele_struct **pool;
	pthread_mutex_init(&head->mutex, NULL);//使用缺省设置
	head->total_size=num;
	head->count=0;
	head->plen=plen;
	head->free=NULL;
	head->active=NULL;
	head->pool=NULL;
	// Init Semaphore..
	ret = sem_init(&head->ssema, 0, 0);
	if(ret)
		return -1;
	oldele=NULL;
	mem_len=sizeof(struct pool_ele_struct)-4+plen;
	pool=(struct pool_ele_struct **)calloc(sizeof(struct pool_ele_struct *),num);//缓冲池
	if(pool==NULL)
		return -1;
	
	for(i=0;i<num;i++)
	{
		newele=(struct pool_ele_struct*)calloc(4,mem_len/4);//change malloc(mem_len);
		if(newele==NULL)
		{
			for(j=0;j<i;j++)
				free(pool[j]);
			if(pool!=NULL)
				free(pool);
			head->pool=NULL;
			//not enough memory
			return -1;
		}
		pool[i]=newele;
		memset((void*)newele,0,mem_len);
		if(head->free==NULL)
		{
			head->free=newele;
			oldele=newele;			
		}
		else
		{
			oldele->next=newele;			
			oldele=newele;
		}
		
	}
	newele->next=NULL;	
	head->pool=pool;

	initrtpool(head);
	return 0;
	
}

/**********************************************************************************************
 * 函数名	:initrtpool()
 * 功能	:初始化一个缓冲池,
 * 输出	:head:描述缓冲池结构的指针，返回时填充内容
 * 返回值	:0表示成功负值表示出错
 * 注		:必须在mkrtpool之后调用
 **********************************************************************************************/
int initrtpool(struct pool_head_struct *head)
{//NOT test
	int i;
	struct pool_ele_struct *cur;
	struct pool_ele_struct **pool;
	if(head==NULL)
		return -1;
	pthread_mutex_lock(&head->mutex);
	if(head->pool==NULL)
		return -1;
	pool=head->pool;
	head->active=NULL;
	head->free=NULL;
	for(i=0;i<head->total_size;i++)
	{
		cur=pool[i];
		if(cur==NULL)
			continue;
		cur->next=head->free;
		head->free=cur;
	}

	head->count=0;
	pthread_mutex_unlock(&head->mutex);
	return 0;
}

/**********************************************************************************************
 * 函数名	:get_free_eleroom()
 * 功能	:从缓冲池获得一个空闲的空间
 * 输入	:head:描述缓冲池结构的指针
 * 返回值	:指向剩余空间的指针 返回NULL表示没有空闲空间
 **********************************************************************************************/
struct pool_ele_struct *get_free_eleroom(struct pool_head_struct *head)
{
	struct pool_ele_struct *pret;
	//int total,i;
	if(head==NULL)
		return NULL;
	pret=NULL;
	pthread_mutex_lock(&head->mutex);
	if(head->free==NULL)
	{
		pret=NULL;
	}
	else
	{
		pret=head->free;		
		head->free=pret->next;
		pret->next=NULL;
		pret->ele_type=0;
	}


	
	pthread_mutex_unlock(&head->mutex);
	return pret;		
}

/**********************************************************************************************
 * 函数名	:put_active_ele()
 * 功能	:将元素放入有效元素队列尾部
 * 输入	:head:描述缓冲池结构的指针
 *			 active:指向要放入的元素指针
 * 返回值	:0表示成功，负值表示出错
 **********************************************************************************************/
int put_active_ele(struct pool_head_struct *head,struct pool_ele_struct *active)
{
	int i,total;
	int rc;
	struct pool_ele_struct *cur;
	if(head==NULL)
		return -1;
	if(active==NULL)
		return -1;
	pthread_mutex_lock(&head->mutex);

	total=head->total_size;
	cur=head->active;
	rc=0;
	for(i=0;i<total;i++)
	{
		if(head->active==NULL)
		{
			head->active=active;
			active->next=NULL;
			head->count=1;
			break;
		}		
		if(cur->next==NULL)
		{
			cur->next=active;
			active->next=NULL;
			head->count++;
			break;
		}
		cur=cur->next;
	}
	if(i==total)
	{
		rc=-1;
		//出错
	}
	
	pthread_mutex_unlock(&head->mutex);
	return rc;
}
#define GET_ALL_ELE_TYPE	-1
//取出有效元素队列中第一个指定类型的元素
static struct pool_ele_struct *get_active_type_ele(struct pool_head_struct *head,DWORD type)
{
	struct pool_ele_struct *pret,*prev,*cur;
	int i;
	if(head==NULL)
		return NULL;
	pret=NULL;
	pthread_mutex_lock(&head->mutex);
	prev=head->active;
	if(prev!=NULL)
	{
		if((type==GET_ALL_ELE_TYPE)||(prev->ele_type==type))
		{
			pret=head->active;
			head->active=head->active->next;
			if(head->count>0)
				head->count--;
			goto get_active_type_ele_end;
		}
	}
	else
		goto get_active_type_ele_end;
	for(i=0;i<head->total_size;i++)
	{
		cur=prev->next;
		if(cur==NULL)
			break;
		if(cur->ele_type==type)
		{
			pret=cur;
			prev->next=cur->next;
			if(head->count>0)
				head->count--;
			goto get_active_type_ele_end;				
		}
		prev=prev->next;
		if(prev==NULL)
			break;	
	}





get_active_type_ele_end:
	pthread_mutex_unlock(&head->mutex);	
	return pret;
}

/**********************************************************************************************
 * 函数名	:get_active_ele()
 * 功能	:取出缓冲池中第一个有效元素
 * 输入	:head:描述缓冲池结构的指针
 * 返回值	:指向第一个有效元素的指针 NULL表示没有有效元素
 **********************************************************************************************/
struct pool_ele_struct *get_active_ele(struct pool_head_struct *head)
{

	return get_active_type_ele(head,GET_ALL_ELE_TYPE);

}


/**********************************************************************************************
 * 函数名	:get_pool_active_num()
 * 功能	:获取缓冲池中有效元素的个数
 * 输入	:head:描述缓冲池结构的指针
 * 返回值	:>=0表示缓冲池中有效元素的个数 负值表示失败
 **********************************************************************************************/
int	get_pool_active_num(struct pool_head_struct *head)
{
	int num,i;
	struct pool_ele_struct *cur;
	if(head==NULL)
		return -1;
	num=0;
	pthread_mutex_lock(&head->mutex);
	cur=head->active;
	for(i=0;i<head->total_size;i++)
	{
		if(cur==NULL)
			break;
		cur=cur->next;
		num++;
	}
	pthread_mutex_unlock(&head->mutex);
	return num;
}

/**********************************************************************************************
 * 函数名	:get_pool_free_num()
 * 功能	:获取缓冲池剩余元素空间的个数
 * 输入	:head:描述缓冲池结构的指针
 * 返回值	:>=0表示缓冲池中的剩余空间个数  负值表示失败
 **********************************************************************************************/
int	get_pool_free_num(struct pool_head_struct *head)
{	
	int num,i;
	struct pool_ele_struct *cur;
	if(head==NULL)
		return -1;
	num=0;
	pthread_mutex_lock(&head->mutex);
	cur=head->free;
	for(i=0;i<head->total_size;i++)
	{
		if(cur==NULL)
			break;
		cur=cur->next;
		num++;
	}
	pthread_mutex_unlock(&head->mutex);
	return num;
}

/**********************************************************************************************
 * 函数名	:free_ele()
 * 功能	:将一个用过的元素放入缓冲池的空闲队列
 * 输入	:head:描述缓冲池结构的指针
 *			 ele:指向空闲元素的指针
 * 返回值	:0表示成功 负值表示失败
 **********************************************************************************************/
int free_ele(struct pool_head_struct *head,struct pool_ele_struct *ele)
{
	//struct pool_ele_struct *cur;
	//int i;
	if(head==NULL)
		return -1;
	if(ele==NULL)
		return -1;
	pthread_mutex_lock(&head->mutex);
	
	ele->next=head->free;
	ele->ele_type=0;
	head->free=ele;
	pthread_mutex_unlock(&head->mutex);
	return 0;
}

/**********************************************************************************************
 * 函数名	:drop_ele_type()
 * 功能	:从有效元素缓冲区中删除num个type类型的元素
 * 输入	:head:描述缓冲池结构的指针
 *			 type:要丢弃的元素类型
 *			 num:要丢弃的元素数量
 * 返回值	:正值表示成功丢弃的元素个数，负值表示出错
 **********************************************************************************************/
int drop_ele_type(struct pool_head_struct *head,DWORD type,int num)
{
	int i,ret;//,total;
	struct pool_ele_struct *cur;//,*prev,*erase;
	if(head==NULL)
		return -1;
	ret=0;

	for(i=0;i<num;i++)
	{

		cur=get_active_type_ele(head,type);
		if(cur==NULL)
			break;
		if(free_ele(head,cur)==0)
			ret++;

	}	
	return ret;

}

