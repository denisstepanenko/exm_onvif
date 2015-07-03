#ifndef VENCAPI_H
#define VENCAPI_H

#include <mshmpool.h>
#include <AVIEncoder.h>

#ifndef FRAMETYPE_I
#define FRAMETYPE_I		0x0		// IME6410 Header - I Frame
#define FRAMETYPE_P		0x1		// IME6410 Header - P Frame
#define FRAMETYPE_B		0x2
#define FRAMETYPE_PCM	0x5		// IME6410 Header - Audio Frame
#define FRAMETYPE_ADPCM	0x6		// IME6410 Header - Audio Frame
#define FRAMETYPE_MD	0x8
#define VSIG_I			0xb0010000
#define VSIG_P			0xb6010000
#endif

#define 	ENC_NO_INIT		0		//编码器没有初始化
#define	ENC_STAT_OK		1		//编码器工作正常
#define	ENC_STAT_ERR		2		//编码器故障	
typedef struct{//编码器属性结构
	int				EncType;	//编码器类型
	int				State;		//编码器状态  0:表示未初始化 1表示正常 2表示故障
	struct defAVIVal 	EncVal;		//视频格式(avi文件会用到)
}ENC_ATTRIB;


/**************************************************************************
  *函数名	:InitVEncVar
  *功能	:按照指定通道号初始化视频编码器用的enc结构中的字段
  *参数	: encno:编码器编号 0,1,2...
  			  Enc:希望初始化填充的数据结构  		
  *返回值	:返回0表示成功 负值表示出错
  *注:本函数在直接与设备打交道的程序中才需要使用(调用
  *CreateVEncDevice函数的程序)其他程序(调用AttachVEncDevice的程序)
  *不需要
  *************************************************************************/
int InitVEncVar(int encno,struct compress_struct *Enc);
/**************************************************************************
  *函数名	:GetVEncIniSec
  *功能	:获取指定视频编码器相关的配置文件的节名
  *参数	: enc:已经初始化好的描述编码器信息的结构
  *返回值	:配置文件(ini)中的配置节名
  *注:本函数在直接与设备打交道的程序中才需要使用(调用
  *CreateVEncDevice函数的程序)其他程序(调用AttachVEncDevice的程序)
  *不需要
  *************************************************************************/
//char *GetVEncIniSec(struct compress_struct *enc);

/**************************************************************************
  *函数名	:RefreshVencDevicePara
  *功能	:更新设备参数到共享内存
  *参数	:
  *			 Enc:已经初始化好的描述编码器信息的结构
  *			 Pool:一个已经初始化的描述缓冲池信息的结构
   *返回值	:0表示正常打开设备缓冲区
  *			 -EINVAL:参数错误
  *			
  *			-
  *注:本函数应该由直接与设备打交道的程序中调用
  *************************************************************************/
int RefreshVencDevicePara(struct compress_struct   *Enc,MSHM_POOL *Pool);

/**************************************************************************
  *函数名	:CreateVEncDevice
  *功能	:创建一个视频编码器设备的共享缓冲区,并打开编码器
  *参数	:name: 创建设备的程序名
  *			 Enc:已经初始化好的描述编码器信息的结构
  *			 Pool:一个没有初始化的描述缓冲池信息的结构
  *			 bytes:希望分配的共享缓冲区大小 0表示自动选择大小
  *返回值	:0表示正常打开设备及缓冲区
  *			 -EINVAL:参数错误
  *			 -ENODEV:打不开设备
  *			-
  *注:本函数应该由直接与设备打交道的程序中调用
  *************************************************************************/
int CreateVEncDevice(char *name,struct compress_struct   *Enc,MSHM_POOL *Pool,int bytes);
/**************************************************************************
  *函数名	:OpenVEncDevice
  *功能	:打开一个视频编码器设备
  *参数	:
  *			 Enc:已经初始化好的描述编码器信息的结构
  *			 Pool:一个已经初始化的描述缓冲池信息的结构
  *			
  *返回值	:>=0表示正常打开设备
  *			 -EINVAL:参数错误
  *			 -ENODEV:打不开设备
  *			-
  *注:本函数应该由直接与设备打交道的程序中调用
  *************************************************************************/
  int OpenVEncDevice(struct compress_struct   *Enc,MSHM_POOL *Pool);

/**************************************************************************
  *函数名	:AttachVEncDevice
  *功能	:连接到一个已经打开的缓冲池
  *参数	:name: 创建设备的程序名
  *			 Enc:已经初始化好的描述编码器信息的结构
  *			 Pool:一个没有初始化的描述缓冲池信息的结构
  *			 bytes:希望分配的共享缓冲区大小 0表示自动选择大小
  *返回值	:0表示正常连接设备缓冲区
  *			 -EINVAL:参数错误
  *			 -EAGAIN:设备正在初始化返回
  *			 --ENODEV:设备故障
  *			-
  *注:本函数应该由从缓冲区中读取数据的程序调用
  *************************************************************************/
int AttachVEncDevice(int EncNo,MSHM_POOL *Pool,char *UsrName,int type);
/**************************************************************************
  *函数名	:FreeVEncDevice
  *功能	:释放已经打开的共享缓冲池
  *参数	: pool:缓冲池结构指针
  *返回值	:返回0表示成功 负值表示失败错误是负的errno
  *************************************************************************/
int FreeVEncDevice(MSHM_POOL *Pool);
/**************************************************************************
  *函数名	:PutVFrame2Pool
  *功能	:将一真数据放入缓冲池
  *参数	: pool:缓冲池结构指针,Frame填充好的媒体结构数据
  *返回值	:返回0表示成功 负值表示失败错误是负的errno
  *************************************************************************/
int PutVFrame2Pool(MSHM_POOL *Pool,struct stream_fmt_struct *Frame);

//int MShmPoolGetResource(MSHM_POOL *pool,void *buf,int buflen,int *elelen,int *flag);
/**************************************************************************
  *函数名	:GetVFrameFromPool
  *功能	:从缓冲池中获取一包数据
  *参数	: pool:缓冲池结构指针,
  *			 Frame需要填充的缓冲区
  *			 fblen待填充缓冲区的长度
  *			 eleseq:读取的元素序号(输出)
  *			 flag:读取的元素的标志(输出)
  *返回值	:返回正值表示读取的元素大小表示成功 负值表示失败错误是负的errno
  *************************************************************************/
static __inline__ int GetVFrameFromPool(MSHM_POOL *pool,void *frame,int fblen,int *eleseq,int *flag)
{
	return MShmPoolGetResource(pool,frame,fblen,eleseq,flag);
}

#define	GetVEncUsrValid	GetUsrValid
#endif



