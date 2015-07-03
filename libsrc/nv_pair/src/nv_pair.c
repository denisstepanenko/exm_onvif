/*
  nv_pair.c
  
*/
#define  _CRT_SECURE_NO_WARNINGS
#define  _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nv_pair.h"

#define MAGIC	0xeb90eb90

static const char	*nvp_default_seperator	= "^^";	//默认的分隔符
static const char	*nvp_default_equal_mark	= "=="; //默认的相等符
#if 1

typedef	struct{
//描述名值对的数据结构，库函数使用此结构 应用程序使用NVP_T结构
	unsigned long magic;
	 int str_len;			//命令字符串长度
	 int num_nvp;		//名值对的数量
	 char seperator[MAX_SEP_LEN];		// 存放分隔符
	 char equal_mark[MAX_MARK_LEN];	//存放等于号
	 char nvp[MAX_CMD_NUM][MAX_DATA_LEN];	//存放名值对
	 char msg_buf[MAX_CMD_NUM*MAX_DATA_LEN];// 存放需要解析的字符串和已经组织好的字符串
	//FILE *nv_fp;							//保存有键值对的文件句柄
}NVP_T;

/*
*************************************************************************
*函数名	:nvp_set_seperator
*功能	: 设置分隔符 * 
*输入	:  
			NVP_TP	*nv,          之前使用nvp_create得到的指针
			const char * seperator < 分隔符字串 
*输出	: 正确0  错误码
*修改日志:
*************************************************************************
*/
int nvp_set_seperator(NVP_TP *nv,  const char * seperator)
{
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;
	if(nv==NULL||(strlen(seperator)>MAX_SEP_LEN))
	{
		return -NVP_PARA_ERR;
	}
	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return -NVP_PARA_ERR;	// not an initilized struct pointer 
	} 

	memset(nt->seperator, 0 , MAX_SEP_LEN);
	memcpy(nt->seperator , seperator , strlen(seperator));
	return NVP_SUCCESS;
}
/*
*************************************************************************
*函数名	:nvp_get_pair_index  内部函数
*功能	:  在已经设置的名值对中查找并返回索引
*输入	:  
		NVP_TP	*nv,          	之前使用nvp_create得到的指针
		const char * name, 	< 名 
*输出	: 成功返回索引号 负值表示出错
*修改日志:
*************************************************************************
*/
//static __inline__ int nvp_get_pair_index(NVP_T *nv, const	char * name)
static int nvp_get_pair_index(NVP_T *nv, const	char * name)
{
	int i;
	char* p= NULL;
	char* index= NULL;
	
	if((nv==NULL)||(name==NULL)||(strlen(name)>MAX_DATA_LEN))
	{
		return -NVP_PARA_ERR;
	} 

	for(i=0;i<nv->num_nvp;i++)
	{
		p=nv->nvp[i];
//// lsk 2008 -11-28 
		if(strncmp(p, nv->seperator,strlen(nv->seperator)))
			continue;
		p+=strlen(nv->seperator);
		if(strncmp(p, name,strlen(name)))
			continue;
		p+=strlen(name);
		if(strncmp(p, nv->equal_mark,strlen(nv->equal_mark)))
			continue;
		return i;
#if 0		
		index=strstr(p,name);
		if(index!=NULL)
		return i;
#endif
	}

	return -NVP_PARA_ERR;
}

/*
*************************************************************************
*函数名	:nvp_set_equal_mark
*功能	: 相等符
*输入	:  
			NVP_TP	*nv,          之前使用nvp_create得到的指针
			const char * mark 	  分隔符字串 
*输出	: 正确0  错误码
*修改日志:
*************************************************************************
*/
int nvp_set_equal_mark( NVP_TP	*nv, const char * mark)
{
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;
	if(nv==NULL||(strlen(mark)>MAX_MARK_LEN))
	{
		return -NVP_PARA_ERR;
	}
	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return -NVP_PARA_ERR;	// not an initilized struct pointer 
	} 

	memset(nt->equal_mark, 0 ,MAX_MARK_LEN);
	memcpy(nt->equal_mark , mark , strlen(mark));
	return 0;
}

/*
*************************************************************************
*函数名	:nvp_create
*功能	: 创建一个名值对结构
*输入	:  无
*输出	: 
*修改日志:
*************************************************************************
*/
NVP_TP	*nvp_create(void)
{
	NVP_T* nvp_st=NULL;
	int i;
	nvp_st = malloc(sizeof(NVP_T));
	if(nvp_st == NULL)
	{
		//errno=NVP_NO_MEM;
		return NULL;
	}
	memset(nvp_st->msg_buf, 0 , sizeof(nvp_st->msg_buf));
	for(i=0;i<MAX_CMD_NUM;i++)
	{
		memset(&nvp_st->nvp[i], 0 , MAX_DATA_LEN);
	}
	nvp_st->magic = MAGIC;	// set magic byte 
	nvp_st->num_nvp = 0;
	nvp_st->str_len = 0;
	nvp_set_equal_mark(nvp_st, nvp_default_equal_mark);
	nvp_set_seperator(nvp_st, nvp_default_seperator);
	

	
	return nvp_st;
}

/*
*************************************************************************
*函数名	:nvp_set_pair_str
*功能	: 压入名值对
* 注意: 如果相同键值的数据存在则进行替换原有值
*输入	:  
		NVP_TP	*nv,          	之前使用nvp_create得到的指针
		const char * name, 	< 名 
		const char * value 	< 值 
*输出	: 0表示成功负值表示出错
*修改日志: 2006 -9 -26 nvp_set_pair -> nvp_set_pair_str
*************************************************************************
*/
int nvp_set_pair_str(NVP_TP *nv, const char * name, const char * value 	)
{
	int index = 0;
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;
	if((nv==NULL)||(name==NULL)||(value==NULL))
	{
		return -NVP_PARA_ERR;
	}
	if((strlen(name)+strlen(value))>MAX_DATA_LEN)
	{
		return -NVP_PARA_ERR;
	}
	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return -NVP_PARA_ERR;	// not an initilized struct pointer 
	} 
	
	// 查找同名的值对
	index = nvp_get_pair_index(nt, name); 
	if(index < 0)	// 没有发现有同名的值对
	{
		sprintf(nt->nvp[nt->num_nvp],"%s%s%s%s", 
			nt->seperator , name, nt->equal_mark, value);
		nt->num_nvp++;
		if(nt->num_nvp>MAX_CMD_NUM)
		{
			return -NVP_NO_MEM;
		}
	}
	else		//刷新原来的内容
	{
		memset(nt->nvp[index], 0 , MAX_DATA_LEN);
		sprintf(nt->nvp[index],"%s%s%s%s",nt->seperator ,
		name, nt->equal_mark, value);
	}
	return NVP_SUCCESS;
}
/*
*************************************************************************
*函数名	:nvp_set_pair_int
*功能	: 压入名值对(整数)
* 注意: 如果相同键值的数据存在则进行替换原有值
*输入	:  
		NVP_TP	*nv,          	之前使用nvp_create得到的指针
		const char * name, 	< 名 
		int value 	< 值 
*输出	: 0表示成功负值表示出错
*修改日志:
*************************************************************************
*/
int nvp_set_pair_int(NVP_TP *nv, const char * name, int value)
{
	char buf[20];
	memset(buf, 0 , 20);
	sprintf(buf, "%d",value);
	return(nvp_set_pair_str(nv, name, (const char*)buf));
}

/*
*************************************************************************
*函数名	:nvp_get_pair_str
*功能	: 根据名称得到值,如果未找到则返回dev_val
*输入	:  
		NVP_TP	*nv,          	之前使用nvp_create得到的指针
		const char * name, 	< 名 
		const char * def_val 	< 默认值 
*输出	: 0表示成功负值表示出错
*修改日志:2006 -9 -26  nvp_get_pair -> nvp_get_pair_str
*************************************************************************
*/
const char * nvp_get_pair_str(NVP_TP *nv, const	char * name , const	char * def_val)
{
	int i;
	int len =strlen(name); //// lsk 2008 -11-28
	char* p= NULL;
	char* s = NULL;
	char* index= NULL;
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;
	
	if((nv==NULL)||(name==NULL)||(len>MAX_DATA_LEN))
	{
		return NULL;
	} 
	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return NULL;	// not an initilized struct pointer 
	}
	i=nvp_get_pair_index(nt, name);
	if(i<0)
		return def_val;
	p=nt->nvp[i];
	s=strstr(p,nt->equal_mark);
	if(s)
		s+=strlen(nt->equal_mark);
	else
		return def_val;
	if(*s=='\0')
	{
		return def_val;
	}
	return s;	
#if 0
	for(i=0;i<nt->num_nvp;i++)
	{
		p=nt->nvp[i];

		index=strstr(p,name);
		if(index==NULL)
			continue;
		else
		{
			s = index + len;
	  		if(strncmp(s,nt->equal_mark ,strlen(nt->equal_mark))==0)
	  		{
	  			s+=strlen(nt->equal_mark);
	  			if(*s=='\0')
				{
					return def_val;
				}
				return s;
	  		}
			continue;	//不是分隔符就继续查找
		}
	}
	return def_val;
#endif
}
/*
*************************************************************************
*函数名	:nvp_get_pair_int
*功能	: 根据名称得到值,如果未找到则返回dev_val
*输入	:  
		NVP_TP	*nv,          	之前使用nvp_create得到的指针
		const char * name, 	< 名 
		const char * def_val 	< 默认值 
*输出	: 查找到的整数值
*修改日志:2006 -9 -26  nvp_get_pair -> nvp_get_pair_str
*************************************************************************
*/
int nvp_get_pair_int(NVP_TP *nv, const	char * name , const int def_val)
{
	const char* ret=NULL; 
	ret = nvp_get_pair_str(nv, name,NULL);
	if(ret == NULL)
	{
		return def_val;
	}
	else
	{
		return atoi(ret);
	}
}


/*
************************************************************************
*函数名	:nvp_get_string
*功能	: 得到所有名值对的串接格式
*输入	:  
			NVP_TP	*nv,          之前使用nvp_create得到的指针
*输出	: 字符串指针
*修改日志:
*************************************************************************
*/
const char *nvp_get_string(NVP_TP	*nv)
{
	int i;
	int len=0;
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;
	if(nv==NULL)
	{
		return NULL;
	} 

	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return NULL;	// not an initilized struct pointer 
	} 

	nt->str_len = 0;
	len = 0;
	//清除缓冲区
	memset(nt->msg_buf , 0 , MAX_CMD_NUM*MAX_DATA_LEN);
	// 将所有的名值对填充到缓冲区
	for(i=0; i<nt->num_nvp; i++)
	{
		len = strlen(nt->nvp[i]);
		memcpy(&nt->msg_buf[nt->str_len] , nt->nvp[i] , len);
		nt->str_len+=len;
	}
	return nt->msg_buf+strlen(nt->seperator);
//	return nt->msg_buf;
}

 /*
************************************************************************
*函数名	:nvp_parse_string
*功能	:  解析名值对的串接格式
*输入	:  
			NVP_TP	*nv,          之前使用nvp_create得到的指针
			const char *  str      名值对的串接格式 
*输出	:  解析得到名值对的数量
*修改日志:
*************************************************************************
*/
int nvp_parse_string(NVP_TP *nv, const char *  str )
{
	char* p = NULL;
	char* s = NULL;
	char* index = NULL;
	int offset =0;
	int i = 0;
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;
	if((nv==NULL)||(str==NULL))
	{
		return -NVP_PARA_ERR;
	} 
	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return -NVP_PARA_ERR;	// not an initilized struct pointer 
	} 

	if((strlen(str))>(MAX_CMD_NUM*MAX_DATA_LEN))
	{
		return -NVP_NO_MEM;
	} 
	//清除缓冲区
	memset(nt->msg_buf , 0 , MAX_CMD_NUM*MAX_DATA_LEN);
	for(i=0; i<nt->num_nvp; i++)
	{
		memset(nt->nvp[i], 0 , MAX_DATA_LEN);
	}
	////前面的不是分隔符
	if(strncmp(str, nt->seperator,strlen(nt->seperator))!=0)
	{
	////加上分隔符
		memcpy(nt->msg_buf, nt->seperator, strlen(nt->seperator));
		memcpy(&nt->msg_buf[strlen(nt->seperator)], str, strlen(str));
	}
	else
	{
		memcpy(nt->msg_buf, str, strlen(str));
	}
	nt->str_len = strlen(nt->msg_buf);
	nt->num_nvp = 0;
#if 1
	index = nt->msg_buf;
	for(i=0;i<MAX_CMD_NUM;i++)
	{
		p = strstr(index, nt->seperator);
		if(p!=NULL)
		{
			index = p+strlen(nt->seperator);
			s = strstr(index, nt->seperator);
			if(s!=NULL)		// 没有到结尾
			{
				offset = (int )(s - p);
				if(offset>(int)strlen(nt->seperator))	//不是两个连续的分隔符
				{
					memcpy(nt->nvp[nt->num_nvp], p, offset);
					nt->num_nvp++;
				}
			}
			else				// 到了数据包的结尾
			{
				offset = (int )(nt->msg_buf+(nt->str_len) - p);
				memcpy(nt->nvp[nt->num_nvp], p, offset);
				nt->num_nvp++;
			}
		}
		else 
		{
			if(nt->num_nvp==0)
			{
				return -NVP_PARA_ERR;
			}
			break;
		}
	}
#endif	
	return nt->num_nvp;
}

/*
************************************************************************
*函数名	:nvp_dump
*功能	:  打印所有名值对的内容(调试用)
*输入	:  
			NVP_TP	*nv,          之前使用nvp_create得到的指针
*输出	:  解析得到名值对的数量
*修改日志:
*************************************************************************
*/
int nvp_dump(NVP_TP *nv )
{
	int i;
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;

	if((nv==NULL))
	{
		return -NVP_PARA_ERR;
	} 
	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return -NVP_PARA_ERR;	// not an initilized struct pointer 
	} 

	printf("there are %d pairs in buffer\n",  nt->num_nvp);
	for(i=0;i<nt->num_nvp;i++)
	{
		printf("%s\n",nt->nvp[i]);
	}
	return NVP_SUCCESS;
}
/*
************************************************************************
*函数名	:nvp_get_count
*功能	:  得到名值对的数量
*输入	:  
			NVP_TP	*nv,          之前使用nvp_create得到的指针
*输出	:  解析得到名值对的数量
*修改日志:
*************************************************************************
*/
int nvp_get_count(NVP_TP *nv)
{
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;
	if((nv==NULL))
	{
		return -NVP_PARA_ERR;
	}
	// check for magic byte 
	if((nt->magic)!=MAGIC)
	{
		return -NVP_PARA_ERR;	// not an initilized struct pointer 
	} 

	return nt->num_nvp;
}

 /*
************************************************************************
*函数名	:nvp_destroy
*功能	:  销毁一个已经使用过的nvp结构
*输入	:  
			NVP_TP	*nv,          之前使用nvp_create得到的指针
*输出	:  无
*修改日志:
*************************************************************************
*/
void nvp_destroy(NVP_TP	*nv)
{
	NVP_T* nt=NULL;
	nt = (NVP_T*)nv;

	if((nv!=NULL)&&((nt->magic)==MAGIC))
	{
		nt->magic=0;	
		free(nv);
	}
}

 /*
************************************************************************
*函数名	:nvp_get_error_str
*功能	:  获取错误码的字符串描述
*输入	:  
			int errno, 接口返回的错误代码的绝对值
*输出	:     错误描述字符串指针
*修改日志:
*************************************************************************
*/
const char *nvp_get_error_str(int errno)
{
	switch(errno)
	{
		case NVP_NO_MEM: 
			return "not enough memory";
			break;
		case NVP_PARA_ERR: 
			return"parameters error";
			break;
		default:
			return "unknow nvp errno";
			break;
	};
}

#if 0
/************************************************************************
*函数名	:nvp_open_file
*功能	:  打开一个键值对文件
*输入	:  文件名
			int errno, 接口返回的错误代码的绝对值
*输出	:     错误描述字符串指针
*修改日志:
*************************************************************************/
int nvp_open_file(NVP_TP *nv,char *filename)
{
	FILE *fp;

	if(filename==NULL)
	{
		gtlogerr("文件[%s]为空\n",filename);
		printf("文件 [%s]为空\n",filename);
		return -1;
	}
	fp=fopen(filename,"r");
	if(fp==NULL)
	{
		gtlogerr("打开文件[%s]获取nvp信息失败\n",filename);
		printf("打开文件[%s]获取nvp信息失败\n",filename);
		return -1;
	}

	return 0;
}
#endif

 
#endif
