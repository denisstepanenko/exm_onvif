#include <stdio.h>
#include <errno.h>
#include <devinfo.h>
#include "devtype.h"
//设备上用到的共享内存key定义
#define         VIDEO_ENC0_KEY          0x30000          //编码器0的共享缓冲池key
#define         VIDEO_ENC1_KEY          0x30001          //编码器1的共享缓冲池key
#define         VIDEO_ENC2_KEY          0x30002          //编码器2的共享缓冲池key
#define         VIDEO_ENC3_KEY          0x30003          //编码器3的共享缓冲池key
#define         VIDEO_ENC4_KEY          0x30004          //编码器4的共享缓冲池key
#define         VIDEO_ENC5_KEY          0x30005          //编码器5的共享缓冲池key

#define         AUDIO_ENC0_KEY          0x50000        //音频编码器0的输出数据共享缓冲池key
#define         AUDIO_ENC1_KEY          0x50001	    //音频编码器1的输出数据共享缓冲池key
#define         AUDIO_ENC2_KEY		    0x50002	    //音频编码器2的输出数据共享缓冲池key
#define         AUDIO_ENC3_KEY          0x50003	    //音频编码器3的输出数据共享缓冲池key

#define         AUDIO_DEC0_KEY          0x52000          //音频解码器0的输入共享缓冲池key
#define         AUDIO_DEC1_KEY          0x52001          //音频解码器0的输入共享缓冲池key
#define         AUDIO_DEC2_KEY          0x52002          //音频解码器0的输入共享缓冲池key
#define         AUDIO_DEC3_KEY          0x52003          //音频解码器0的输入共享缓冲池key


#define         ONVIF_MASTER0_KEY          0x30000       
#define         ONVIF_MASTER1_KEY          0x30001       
#define         ONVIF_MASTER2_KEY          0x30002      
#define         ONVIF_MASTER3_KEY          0x30003     
#define         ONVIF_MASTER4_KEY          0x30004    
#define         ONVIF_MASTER5_KEY          0x30005   
#define         ONVIF_MASTER6_KEY          0x30006  
#define         ONVIF_MASTER7_KEY          0x30007 

#define         ONVIF_SLAVE0_KEY          0x31000       
#define         ONVIF_SLAVE1_KEY          0x31001       
#define         ONVIF_SLAVE2_KEY          0x31002      
#define         ONVIF_SLAVE3_KEY          0x31003     
#define         ONVIF_SLAVE4_KEY          0x31004    
#define         ONVIF_SLAVE5_KEY          0x31005   
#define         ONVIF_SLAVE6_KEY          0x31006  
#define         ONVIF_SLAVE7_KEY          0x31007 











#define		HQ_ENC_QUEUE_KEY	0x60000		//高清晰录像通道0使用的消息队列的key
//#define		HQ_ENC_QUEUE_KEY1	0x61000		//高清晰录像通道1使用的消息队列的key
//#define		HQ_ENC_QUEUE_KEY2	0x62000		//高清晰录像通道2使用的消息队列的key
//#define		HQ_ENC_QUEUE_KEY3	0x63000		//高清晰录像通道3使用的消息队列的key
//#define		HQ_ENC_QUEUE_KEY4	0x64000		//高清晰录像通道4使用的消息队列的key


#define		VIDEO_ENC0_BUF_SIZE	(1024*1024*2)	//编码器0使用的共享内存大小
#define		VIDEO_ENC1_BUF_SIZE	(1024*1024*2)	//编码器1使用的共享内存大小
#define		VIDEO_ENC2_BUF_SIZE	(1024*1024*2)	//编码器0使用的共享内存大小
#define		VIDEO_ENC3_BUF_SIZE	(1024*1024*2)	//编码器0使用的共享内存大小
#define		VIDEO_ENC4_BUF_SIZE	(1024*1024*2)	//编码器0使用的共享内存大小



#define		VIDEO_ENC0_INI_SEC	"netencoder"
#define		VIDEO_ENC1_INI_SEC	"hqenc0"
#define		VIDEO_ENC2_INI_SEC	"hqenc1"
#define		VIDEO_ENC3_INI_SEC	"hqenc2"
#define		VIDEO_ENC4_INI_SEC	"hqenc3"



//进程状态共享内存定义
#define         VSMAIN_STAT_KEY         	0x40000          //存放vsmain所有参数，状态的共享内存key
#define         TCPRTIMG_STAT_KEY       	0x41000          //存放tcprtimg2的工作状态的共享内存key
extern	GTSeriesDVSR    *get_current_dvsr(void);



//sub_type:子设备类型
//no:子设备的序号
static char *get_sub_dev_node(int sub_type,int no)
{
	GTSeriesDVSR	*dvsr=get_current_dvsr();	
	int				total=dvsr->list_num;
	int				i;
	DevType_T		**list=dvsr->list;
	DevType_T		*dev=NULL;
	for(i=0;i<total;i++)
	{
		dev=list[i];
		if(dev->type==sub_type)
		{
			if(dev->no==no)
			{
				return dev->node;
			}
		}
		
	}
	return NULL;
		
}
//sub_type:子设备类型
//no:子设备的序号
static char *get_sub_dev_driver(int sub_type,int no)
{
	GTSeriesDVSR	*dvsr=get_current_dvsr();	
	int				total=dvsr->list_num;
	int				i;
	DevType_T		**list=dvsr->list;
	DevType_T		*dev=NULL;
	for(i=0;i<total;i++)
	{
		dev=list[i];
		if(dev->type==sub_type)
		{
			if(dev->no==no)
			{
				return dev->driver;
			}
		}
	}
	return NULL;
		
}

//sub_type:子设备类型
//no:子设备的序号
static char *get_sub_dev_name(int sub_type,int no)
{
	GTSeriesDVSR	*dvsr=get_current_dvsr();	
	int				total=dvsr->list_num;
	int				i;
	DevType_T		**list=dvsr->list;
	DevType_T		*dev=NULL;
	for(i=0;i<total;i++)
	{
		dev=list[i];
		if(dev->type==sub_type)
		{
			if(dev->no==no)
			{
				return dev->name;
			}
		}
	}
	return NULL;
		
}












/**********************************************************************************************^M
 * 函数名       :get_video_enc_key()^M
 * 功能 		:获取指定编号的视频编码器使用的缓冲池的key
 * 输入 		:no 视频编码器号
 * 返回值       	:非负值表示对应视频编码器的key
 *				负值表示出错
 *					-EINVAL表示参数非法
 **********************************************************************************************/
int get_video_enc_key(int no)
{
	switch(no)
	{
		case 0:
			return VIDEO_ENC0_KEY;
		break;
		case 1:
			return VIDEO_ENC1_KEY;
		break;
		case 2:
			return VIDEO_ENC2_KEY;
		break;
		case 3:
			return VIDEO_ENC3_KEY;
		break;
		case 4:
			return VIDEO_ENC4_KEY;
		break;
		case 5:
			return VIDEO_ENC5_KEY;
		break;
		default:
			return -EINVAL;
		break;
	}
	return -EINVAL;
}

int get_onvif_pool_key(no,type)
{
	if(type==0) //master
	{
		switch(no)
		{
			case 0:
				return ONVIF_MASTER0_KEY;
			break;
			case 1:
				return ONVIF_MASTER1_KEY;
			break;
			case 2:
				return ONVIF_MASTER2_KEY;
			break;
			case 3:
				return ONVIF_MASTER3_KEY;
			break;
			case 4:
				return ONVIF_MASTER4_KEY;
			break;
			case 5:
				return ONVIF_MASTER5_KEY;
			break;
			case 6:
				return ONVIF_MASTER6_KEY;
			break;
			case 7:
				return ONVIF_MASTER7_KEY;
			break;
			default:
				return -EINVAL;
			break;
		}
	}
	else if(type==1)
	{
		switch(no)
		{
			case 0:
				return ONVIF_SLAVE0_KEY;
			break;
			case 1:
				return ONVIF_SLAVE1_KEY;
			break;
			case 2:
				return ONVIF_SLAVE2_KEY;
			break;
			case 3:
				return ONVIF_SLAVE3_KEY;
			break;
			case 4:
				return ONVIF_SLAVE4_KEY;
			break;
			case 5:
				return ONVIF_SLAVE5_KEY;
			break;
			case 6:
				return ONVIF_SLAVE6_KEY;
			break;
			case 7:
				return ONVIF_SLAVE7_KEY;
			break;
			default:
				return -EINVAL;
			break;
		}
	}
	
	return -EINVAL;
}


/**********************************************************************************************^M
 * 函数名       :get_audio_enc_key()^M
 * 功能 		:获取指定编号的音频编码器缓冲池的key
 * 输入 		:no 音频编码器号
 * 返回值       	:非负值表示对应视频编码器的key
 *				负值表示出错
 *					-EINVAL表示参数非法
 **********************************************************************************************/
int get_audio_enc_key(int no)
{
	switch(no)
	{
		case 0:
			return AUDIO_ENC0_KEY;
		break;
		case 1:
			return AUDIO_ENC1_KEY;
		break;
		case 2:
			return AUDIO_ENC2_KEY;
		break;
		case 3:
			return AUDIO_ENC3_KEY;
		break;
		default:
			return -EINVAL;
		break;
	}
	return -EINVAL;

}

/**********************************************************************************************^M
 * 函数名       :get_audio_dec_key()^M
 * 功能 		:获取指定编号的音频解码器缓冲池的key
 * 输入 		:no 音频解码器号
 * 返回值       	:非负值表示对应视频编码器的key
 *				负值表示出错
 *					-EINVAL表示参数非法
 **********************************************************************************************/
int get_audio_dec_key(int no)
{
	switch(no)
	{
		case 0:
			return AUDIO_DEC0_KEY;
		break;
		case 1:
			return AUDIO_DEC1_KEY;
		break;
		case 2:
			return AUDIO_DEC2_KEY;
		break;
		case 3:
			return AUDIO_DEC3_KEY;
		break;
		default:
			return -EINVAL;
		break;
	}
	return -EINVAL;
}


/**********************************************************************************************^M
 * 函数名       :get_total_hqenc_num()^M
 * 功能 		:	获取录像通道总数
 * 输入 		:无^M
 * 返回值       	:正值表示录像通道总数M
 *				负值表示出错
 **********************************************************************************************/
int get_total_hqenc_num(void)
{
	GTSeriesDVSR *dvsr=get_current_dvsr();
	return dvsr->hqencnum;
}

/**********************************************************************************************^M
 * 函数名       :get_hqenc_video_ch()^M
 * 功能 		:根据路像通道号获取相应的视频编码器号
 * 输入 		:no 音频解码器号
 * 返回值       	:非负值表示对应视频编码器的key
 *				负值表示出错
 *					-EINVAL表示参数非法
 **********************************************************************************************/
int get_hqenc_video_ch(int hqch)
{
	int hqnum=get_total_hqenc_num();
	int total_enc=get_videoenc_num();
	if(hqch<0)
		return -EINVAL;
	if(hqch>=hqnum)
		return -EINVAL;
	if(hqnum<total_enc)
		return hqch+1;
	else
		return hqch;
}

//FIXME 多路传输网络视频时要修改
/**********************************************************************************************^M
 * 函数名       :get_net_video_ch()^M
 * 功能 		:根据网络图像通道号获取相应的视频编码器号
 * 输入 		:netch:网络视频通道号
 * 返回值       	:非负值表示对应视频编码器编号
 *				负值表示出错
 *					-EINVAL表示参数非法
 **********************************************************************************************/
int get_net_video_ch(int netch)
{	
	if(netch!=0)
		return -EINVAL;
	else 
		return 0;
}

/**********************************************************************************************^M
 * 函数名       :get_hqenc_video_key()
 * 功能 		:获取高清晰录像的视频采集key
 * 输入 		:ch:高清晰路像通道号
 * 返回值       	:非负值表示对应视频编码器的key
 *				负值表示出错
 *					-EINVAL表示参数非法
 **********************************************************************************************/
int get_hqenc_video_key(int ch)
{
	int hqnum=get_total_hqenc_num();
	int total_enc=get_videoenc_num();
	if((ch<0)||(ch>=hqnum))
		return -EINVAL;
	if(hqnum<total_enc)
		return get_video_enc_key(ch+1);
	else
		return get_video_enc_key(ch);
}

/**********************************************************************************************^M
 * 函数名       :get_hqenc_queue_key()
 * 功能 		:获取高清晰录像使用的消息队列的key
 * 输入 		:ch:高清晰路像通道号
 * 返回值       	:非负值表示对应视频编码器的key
 *				负值表示出错
 *					-EINVAL表示参数非法
 **********************************************************************************************/
int get_hqenc_queue_key(int ch)
{
	int hqnum=get_total_hqenc_num();
	int total_enc=get_videoenc_num();
	if((ch<0)||(ch>=hqnum))
		return -EINVAL;
	
	switch (ch)
	{
		case 0:
			return HQ_ENC_QUEUE_KEY;
		break;
		case 1:
		case 2:
		case 3:
		case 4:
			return (HQ_ENC_QUEUE_KEY+ch*1000);
		break;
		default:
			return -EINVAL;
		break;
	}
	return -EINVAL;
}

/**********************************************************************************************^M
 * 函数名       :get_hqenc_audio_key()
 * 功能 		:获取高清晰录像的音频采集key
 * 输入 		:ch:高清晰路像通道号
 * 返回值       	:非负值表示对应音频编码器的key
 *				负值表示出错
 *				-EINVAL表示参数非法
 **********************************************************************************************/
int get_hqenc_audio_key(int ch)
{
	return get_audio_enc_key(0);	
}

/**********************************************************************************************^M
 * 函数名       :get_tcprtimg_stat_key()
 * 功能         :获取音视频上行服务状态、参数变量的共享内存key
 * 输入         :无
 * 返回值       :音视频上行服务状态的key
 **********************************************************************************************/
int get_tcprtimg_stat_key(void)
{
	return TCPRTIMG_STAT_KEY;
}







/**********************************************************************************************^M
 * 函数名       :get_video_enc_node()
 * 功能 		:获取视频编码器节点名
 * 输入 		:no视频编码器编号
 * 返回值       	:视频编码器设备节点字符串
 *				NULL表示失败(可能是没有初始化devinfo()，或者参数错误)
 **********************************************************************************************/
char *get_video_enc_node(int no)
{
	return get_sub_dev_node(SUB_DEV_VENC,no);
}


/**********************************************************************************************^M
 * 函数名       :get_video_enc_driver()
 * 功能 		:获取视频编码器驱动路径
 * 输入 		:no视频编码器编号
 * 返回值       	:视频编码器驱动程序路径
 *				NULL表示失败(可能是没有初始化devinfo()，或者参数错误)
 **********************************************************************************************/
char *get_video_enc_driver(int no)
{
	return get_sub_dev_driver(SUB_DEV_VENC,no);
}

/**********************************************************************************************^M
 * 函数名       :get_video_enc_name()
 * 功能 		:获取视频编码器的设备名字(卸载时要用)
 * 输入 		:no视频编码器编号
 * 返回值       	:视频编码器驱动程序路径
 *				NULL表示失败(可能是没有初始化devinfo()，或者参数错误)
 **********************************************************************************************/
char *get_video_enc_name(int no)
{
	return get_sub_dev_name(SUB_DEV_VENC,no);
}

/**********************************************************************************************^M
 * 函数名       :get_video_enc_ini_sec()
 * 功能 		:获取视频编码器在ini配置文件中存放参数的节名
 * 输入 		:no视频编码器编号
 * 返回值       	:相应的视频编码器配置文件节名字符串
 *				NULL表示失败(可能是没有初始化devinfo()，或者参数错误)
 **********************************************************************************************/
char *get_video_enc_ini_sec(int no)
{
	switch(no)
	{
		case 0:
			return VIDEO_ENC0_INI_SEC;
		break;
		case 1:
			return VIDEO_ENC1_INI_SEC;
		break;
		case 2:
			return VIDEO_ENC2_INI_SEC;
		break;
		case 3:
			return VIDEO_ENC3_INI_SEC;
		break;
		case 4:
			return VIDEO_ENC4_INI_SEC;
		break;
		default:
			return NULL;
		break;
	}
	return NULL;
}



