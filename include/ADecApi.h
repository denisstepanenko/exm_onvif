#ifndef ADECAPI_H
#define ADECAPI_H
#include <mshmpool.h>
#include <soundapi.h>

#define 	ADEC_NO_INIT		0		//编码器没有初始化
#define		ADEC_STAT_OK		1		//编码器工作正常
#define		ADEC_STAT_ERR		2		//编码器故障	
//编码器属性结构
typedef struct{
	int				EncType;	//编码器类型
	int				State;		//编码器状态  0:表示未初始化 1表示正常 2表示故障
}ADEC_ATTRIB;


int AttachADecDevice(int DecNo,MSHM_POOL *Pool,char *UsrName,int type);
int GetADecState (MSHM_POOL *Pool);
int PutAData2DecPool(MSHM_POOL *Pool,void *data,int datalen);

static __inline__ int GetADataFromDecPool(MSHM_POOL *pool,void *data,int fblen,int *eleseq,int *flag)
{
	return MShmPoolGetResource(pool,data,fblen,eleseq,flag);
}
#endif



