#ifndef AENCAPI_H
#define AENCAPI_H
#include <mshmpool.h>
#include <AVIEncoder.h>
#include <soundapi.h>
#ifndef FRAMETYPE_I
#define FRAMETYPE_I		0x0		// IME6410 Header - I Frame
#define FRAMETYPE_P		0x1		// IME6410 Header - P Frame
#define FRAMETYPE_B		0x2
#define FRAMETYPE_PCM	0x5		// IME6410 Header - Audio Frame
#define FRAMETYPE_ADPCM	0x6		// IME6410 Header - Audio Frame
#define FRAMETYPE_AAC   0xFF
#define FRAMETYPE_MD	0x8
#define VSIG_I			0xb0010000
#define VSIG_P			0xb6010000
#endif

#define 	AENC_NO_INIT		0		//编码器没有初始化
#define	AENC_STAT_OK		1		//编码器工作正常
#define	AENC_STAT_ERR		2		//编码器故障	
typedef struct{//编码器属性结构
	int				EncType;	//编码器类型
	int				State;		//编码器状态  0:表示未初始化 1表示正常 2表示故障
}AENC_ATTRIB;


int CreateAEncDevice(char *name,AUDIO_CODEC   *Enc,MSHM_POOL *Pool,int bytes);
int AttachAEncDevice(int EncNo,MSHM_POOL *Pool,char *UsrName,int type);
int FreeAEncDevice(MSHM_POOL *Pool);	
int PutAFrame2Pool(MSHM_POOL *Pool,struct stream_fmt_struct *Frame);
static __inline__ int GetAFrameFromPool(MSHM_POOL *pool,void *frame,int fblen,int *elelen,int *flag)
{
	return MShmPoolGetResource(pool,frame,fblen,elelen,flag);
}
#endif



