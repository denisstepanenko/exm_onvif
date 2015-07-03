/*by wsy,for audio_pool connect and read etc @Dec 2006*/

#include <stdio.h>
#include <devinfo.h>
#include <devres.h>
#include <errno.h>
#include <mshmpool.h>
#include <media_api.h>
#include <aenc_read.h>
#include <syslog.h>

//打开日志记录功能
//name表示日志信息中的名字
#define gtopenlog(name) openlog(name,LOG_CONS|LOG_NDELAY|LOG_PID,LOG_LOCAL0 );//LOG_USER);

//#define gtlog  syslog		//系统日志信息记录
#define gtlog syslog
//一般性信息
#define gtloginfo(args...) syslog(LOG_INFO,##args)	//记录一般信息
//严重的错误信息
#define gtlogfault(args...) syslog(LOG_CRIT,##args)	//
//错误信息
#define gtlogerr(args...) syslog(LOG_ERR,##args)	//
//警告信息
#define gtlogwarn(args...) syslog(LOG_WARNING,##args)

#define gtlogdebug(args...) syslog(LOG_DEBUG,##args)


static media_source_t audio_enc[MAX_AUDIO_ENCODER];			//音频编码器实例
/**********************************************************************************************
 * 函数名	:init_audio_enc()
 * 功能	:初始化音频编码器缓冲池结构
 * 输入	:无
 * 输出	:在库中已经定义了一个静态数组存放音频编码器结构
 * 返回值	:0表示成功，负值表示失败
 * 注		:在程序启动时调用一次
 **********************************************************************************************/
int init_audio_enc(void)
{
	
	int i,ret,total;
	total = get_audio_num();
	if(total>MAX_AUDIO_ENCODER)
	{
		total=MAX_AUDIO_ENCODER;
		syslog(LOG_ERR,"get_audio_num=%d MAX_AUDIO_ENCODER=%d!!!\n",get_audio_num(),MAX_AUDIO_ENCODER);
		printf("get_audio_num=%d MAX_AUDIO_ENCODER=%d!!!\n",get_audio_num(),MAX_AUDIO_ENCODER);
	}
	
	for(i=0;i<total;i++)
	{
		ret=init_media_rw(&audio_enc[i],MEDIA_TYPE_AUDIO,i,1024);

	}
	
	return 0;	
}


/**********************************************************************************************
 * 函数名	:get_audio_enc()
 * 功能	:获取音频编码器缓冲信息结构指针
 * 输入	:no:需要访问的音频编码器序号
 * 返回值	:指向对应编号音频编码器的结构指针,出错返回NULL
 **********************************************************************************************/
media_source_t *get_audio_enc(IN int no)
{
	//if(no>=1)//fixme 当有多路音频芯片时
	//	return NULL;	
	return &audio_enc[no];
}

/**********************************************************************************************
 * 函数名	:get_audio_read_buf()
 * 功能	:获取指定编码器用于读取数据的临时缓冲区指针
 * 输入	:no:需要访问的音频编码器序号
 * 返回值	:指向对应编号音频编码器的结构指针,出错返回NULL
 **********************************************************************************************/
void *get_audio_read_buf(IN int no)
{
	if(no>=1)//fixme 当有多路音频芯片时
		return NULL;	
	return audio_enc[no].temp_buf;
}
/**********************************************************************************************
 * 函数名	:get_audio_read_buf_len()
 * 功能	:获取指定编码器用于读取数据的临时缓冲区长度
 * 输入	:no:需要访问的音频编码器序号
 * 返回值	:临时缓冲区的长度
 **********************************************************************************************/
int get_audio_read_buf_len(IN int no)
{
	if(no>=1)//fixme 当有多路音频芯片时
		return -EINVAL;	
	return audio_enc[no].buflen;
}

/**********************************************************************************************
 * 函数名	:connect_audio_enc()
 * 功能	:连接到指定编号的音频编码器
 * 输入	:no:需要连接的音频编码器序号
 *			name:用户名
 *			pre_sec:从前面邋pre_sec秒的位置开始连接
 *					0:表示从最新的元素连接
 *				      >0:表示要提前秒数
 *					   如果pre_sec的值大于缓冲池中的总帧数则从最早的元素进行连接
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int connect_audio_enc(IN int no,IN char *name,IN int pre_sec)
{
	int ret;
	int rc;
	int stat;
	int frate=0;
	media_attrib_t *attrib=NULL;
	media_format_t *media=NULL;
	audio_format_t *audio=NULL;
	if(no>=get_audio_num())
		return -EINVAL;
	ret=connect_media_read(&audio_enc[no], get_audio_enc_key(no),name,MSHMPOOL_NET_USR);
	if(ret>=0)
	{
		stat=get_aenc_stat(no);
		if(stat==ENC_STAT_OK)
		{//编码器状态正常
			attrib=get_aenc_attrib(no);
			if(attrib!=NULL)
			{
				media=&attrib->fmt;
				audio=&media->a_fmt;
				frate=(audio->a_frate);
				rc=move_media_place(&audio_enc[no],-(frate*pre_sec));
			}
		}
	}
	return ret;
}
/** 
 *   @brief     连接音频码器直到成功
 *   @param  no 音频编码器编号
 *   @param  name 应用程序名
 *   @return   非负表示成功,负值表示失败
 */ 
int connect_audio_enc_succ(int no,char *name)
{
    int ret;
    int fail_cnt=0;         //失败次数
    if(no>get_audio_num())
        return -EINVAL;

    ///连接音频编码器
    while(1)
    {
    	ret=connect_audio_enc(no,name,0);
    	if(ret==0)
    	{
    		printf     ("connect auidoenc%d success\n",no);
              gtloginfo("connect auidoenc%d success\n",no);
    		break;
    	}
    	else
    	{
    	    if(fail_cnt==0)
    	    {
    	        printf     ("连接音频编码器%d失败\n",no);
    	        gtloginfo("连接音频编码器%d失败\n",no);
    	        
    	    }
           fail_cnt++;
           printf("connect auido enc(%d) failed(%d), ret=%d!!\n",no,fail_cnt,ret);
           if(fail_cnt==40)
           {
                printf    ("连接音频编码器%d失败%d次!!!",no,fail_cnt);
                gtlogerr("连接音频编码器%d失败%d次!!!",no,fail_cnt);
           }
    		sleep(2);
    	}
    }	

    ///等待视频编码器正常
    fail_cnt=0;
    while(1)
    {
    	ret=get_aenc_stat(no);
    	if(ret==ENC_STAT_OK)
    	{
    		printf("音频编码器%d状态正常!\n",no);
    		break;
    	}
    	else
    	{
    		if(++fail_cnt==15)
    		{
    			printf    ("视频编码器%d状态异常,stat=%d!\n",no,ret);
    			gtlogerr("视频编码器%d状态异常,stat=%d!\n",no,ret);
    		}
    		printf("auidoenc%d state=%d!!!\n",no,ret);	
    	}
    	sleep(1);
    }    
    return ret;

}

/**********************************************************************************************
 * 函数名	:disconnect_audio_enc()
 * 功能	:断开到指定编码器的连接
 * 输入	:no:需要断开的音频编码器序号
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int disconnect_audio_enc(IN int no)
{
	if(no>=get_audio_num())
		return -EINVAL;	
	return disconnect_media_read(&audio_enc[no]);
}

/**********************************************************************************************
 * 函数名	:reactive_audio_enc()
 * 功能	:重新激活到音频编码器的连接
 * 输入	:no:需要重新激活的音频编码器序号
 * 返回值	:0表示成功，负值表示失败
 * 注		:应用程序应定期调用,防止由于自己一段时间没有响应而被
 *			 audioenc服务断开
 **********************************************************************************************/
int reactive_audio_enc(IN int no)
{
	if(no>=1)//fixme 当有多路音频芯片时
		return -EINVAL;	
	return reactive_media_usr(&audio_enc[no]);
}

/**********************************************************************************************
 * 函数名	:read_audio_frame()
 * 功能	:从指定编号的编码器中读取一帧数据
 * 输入	:no:需要读取数据的音频编码器序号
 *			:buf_len:frame缓冲区的长度,要是一帧数据超过这个长度则会报错
 * 输出	:frame:准备存放音频帧的缓冲区
 *			 eleseq:音频帧的序号
 *			 flag:音频帧的标志
 * 返回值	:正值表示读取到的字节数，负值表示出错
 **********************************************************************************************/
int read_audio_frame(IN int no,OUT void *frame,IN int buf_len,OUT int *eleseq,OUT int *flag)
{
	return read_media_resource(&audio_enc[no],frame,buf_len,eleseq,flag);
}

//#define 	ENC_NO_INIT		0		//编码器没有初始化
//#define	ENC_STAT_OK		1		//编码器工作正常
//#define	ENC_STAT_ERR		2		//编码器故障	
/**********************************************************************************************
 * 函数名	:get_aenc_stat()
 * 功能	:获取指定编号的音频编码器状态
 * 输入	:no:需要获取状态的音频编码器编号
 * 返回值	:负值表示出错-EINVAL:参数错误 -ENOENT:设备还没有连接
 *					ENC_NO_INIT:未初始化
 *					ENC_STAT_OK:状态正常
 *					ENC_STAT_ERR:编码器故障
 **********************************************************************************************/
#include <file_def.h>//test !!!
int get_aenc_stat(IN int no)
{
	media_source_t 	*media=&audio_enc[no];
	media_attrib_t	*attrib=NULL;
	if(no>=get_audio_num())
		return -EINVAL;
	if(media->dev_stat<0)
		return -ENOENT;
	attrib=media->attrib;
	if(attrib==NULL)
		return -ENOENT;
	return attrib->stat;
}



/**********************************************************************************************
 * 函数名	:get_aenc_attrib()
 * 功能	:获取指定音频编码器的附加信息结构指针
 * 输入	:no:需要访问的音频编码器编号
 * 返回值	:指向描述音频编码器附加属性的结构指针
 *			 NULL表示出错 ，参数错误，编码器未连接
 **********************************************************************************************/
media_attrib_t *get_aenc_attrib(int no)
{
	media_source_t 	*media=&audio_enc[no];
	if(no>=get_audio_num())
		return NULL;
	if(media->dev_stat<0)
		return NULL;
	return media->attrib;
}

