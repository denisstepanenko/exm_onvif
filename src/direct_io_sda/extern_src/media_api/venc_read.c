
#include <stdio.h>
#include <devinfo.h>
#include <devres.h>
#include <errno.h>
#include <mshmpool.h>
#include <media_api.h>
#include <venc_read.h>
#include <syslog.h>

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



static media_source_t video_enc[MAX_VIDEO_ENCODER];			//视频编码器实例
static media_source_t video_enc_keyframe[MAX_VIDEO_ENCODER];	
static media_source_t video_enc_playback[MAX_VIDEO_ENCODER];//zsk add 回放缓冲池
/**********************************************************************************************
 * 函数名	:init_video_enc()
 * 功能	:初始化视频编码器缓冲池结构
 * 输入	:无
 * 输出	:在库中已经定义了一个静态数组存放视频编码器结构
 * 返回值	:0表示成功，负值表示失败
 * 注		:在程序启动时调用一次
 **********************************************************************************************/
int init_video_enc(void)
{
	int i,ret,total;
	total=get_videoenc_num();
	if(total>MAX_VIDEO_ENCODER)
	{
		total=MAX_VIDEO_ENCODER;
		syslog(LOG_ERR,"get_videoenc_num=%d MAX_VIDEO_ENCODER=%d!!!\n",get_videoenc_num(),MAX_VIDEO_ENCODER);
		printf("get_videoenc_num=%d MAX_VIDEO_ENCODER=%d!!!\n",get_videoenc_num(),MAX_VIDEO_ENCODER);
	}

	for(i=0;i<total;i++)
	
	{
		ret=init_media_rw(&video_enc[i],MEDIA_TYPE_VIDEO,i,MAX_FRAME_SIZE);
		//ret=init_media_rw(&video_enc_keyframe[i],MEDIA_TYPE_VIDEO,i,MAX_FRAME_SIZE);
		ret= init_media_rw(&video_enc_playback[i],MEDIA_TYPE_VIDEO,i,MAX_FRAME_SIZE);
	}
	
	return 0;	
}


/**********************************************************************************************
 * 函数名	:get_video_enc()
 * 功能	:获取视频编码器缓冲信息结构指针
 * 输入	:no:需要访问的视频编码器序号
 * 返回值	:指向对应编号视频编码器的结构指针,出错返回NULL
 **********************************************************************************************/
media_source_t *get_video_enc(IN int no)
{

	if(no>=get_videoenc_num())
		return NULL;	
	return &video_enc[no];
}
/**********************************************************************************************
 * 函数名	:get_video_enc_playback()
 * 功能	:获取视频编码器 缓冲池信息结构回放指针
 * 输入	:no:需要访问的视频编码器序号
 * 返回值	:指向对应编号视频编码器的结构指针,出错返回NULL
 **********************************************************************************************/

media_source_t *get_video_enc_playback(IN int no)
{
	if(no>=get_videoenc_num())
		return NULL;	
	return &video_enc_playback[no];
}

media_source_t *get_video_enc_keyframe(IN int no)
{
	if(no>=get_videoenc_num())
		return NULL;	
	return &video_enc_keyframe[no];
}



/**********************************************************************************************
 * 函数名	:get_video_read_buf()
 * 功能	:获取指定编码器用于读取数据的临时缓冲区指针
 * 输入	:no:需要访问的视频编码器序号
 * 返回值	:指向对应编号视频编码器的结构指针,出错返回NULL
 **********************************************************************************************/
void *get_video_read_buf(IN int no)
{
	if(no>=get_videoenc_num())
		return NULL;	
	return video_enc[no].temp_buf;
}

void *get_video_read_keyframe_buf(IN int no)
{
	if(no>=get_videoenc_num())
		return NULL;	
	return video_enc_keyframe[no].temp_buf;
}


/**********************************************************************************************
 * 函数名	:get_video_read_buf_len()
 * 功能	:获取指定编码器用于读取数据的临时缓冲区长度
 * 输入	:no:需要访问的视频编码器序号
 * 返回值	:临时缓冲区的长度
 **********************************************************************************************/
int get_video_read_buf_len(IN int no)
{
	if(no>=get_videoenc_num())
		return -EINVAL;	
	return video_enc[no].buflen;
}
int get_video_read_keyframe_buf_len(IN int no)
{
	if(no>=get_videoenc_num())
		return -EINVAL;	
	return video_enc_keyframe[no].buflen;
}
/**********************************************************************************************
 * 函数名	:connect_video_enc()
 * 功能	:连接到指定编号的视频编码器
 * 输入	:no:需要连接的视频编码器序号
 *			name:用户名
 *			pre_sec:从前面邋pre_sec秒的位置开始连接
 *					0:表示从最新的元素连接
 *				      >0:表示要提前秒数
 *					   如果pre_sec的值大于缓冲池中的总帧数则从最早的元素进行连接
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int connect_video_enc(IN int no,IN char *name,IN int pre_sec)
{
	int ret;
	int rc;
	int stat;
	int frate=0;
	media_attrib_t *attrib=NULL;
	video_format_t *video=NULL;
	if(no>=get_videoenc_num())
		return -EINVAL;
	ret=connect_media_read(&video_enc[no], get_video_enc_key(no),name,MSHMPOOL_NET_USR);
	//ret=connect_media_read(&video_enc[no], key,name,MSHMPOOL_NET_USR);
	if(ret>=0)
	{
		stat=get_venc_stat(no);
		if(stat==ENC_STAT_OK)
		{//编码器状态正常
			attrib=get_venc_attrib(no);
			if(attrib!=NULL)
			{
				video=&attrib->fmt.v_fmt;
				frate=video->v_frate;
				rc=move_media_place(&video_enc[no],-(frate*pre_sec));				
			}
		}
	}
	return ret;
}
/** 
 *   @brief     连接视频编码器直到成功
 *   @param  no 视频编码器编号
 *   @param  name 应用程序名
 *   @return   非负表示成功,负值表示失败
 */ 
int connect_video_enc_succ(int no,char *name,int pre_sec)
{
    int ret;
    int fail_cnt=0;         //失败次数
    if(no>get_videoenc_num())
        return -EINVAL;

    ///连接视频编码器
    while(1)
    {
    	ret=connect_video_enc(no,name,pre_sec);
    	if(ret==0)
    	{
    		printf     ("connect videoenc%d success\n",no);
              gtloginfo("connect videoenc%d success\n",no);
    		break;
    	}
    	else
    	{
    	    if(fail_cnt==0)
    	    {
    	        printf     ("连接视频编码器%d失败\n",no);
    	        gtloginfo("连接视频编码器%d失败\n",no);
    	        
    	    }
           fail_cnt++;
           printf("connect video enc(%d) failed(%d), ret=%d!!\n",no,fail_cnt,ret);
           if(fail_cnt==40)
           {
                printf    ("连接视频编码器%d失败%d次!!!",no,fail_cnt);
                gtlogerr("连接视频编码器%d失败%d次!!!",no,fail_cnt);
           }
    		sleep(2);
    	}
    }	

    ///等待视频编码器正常
    fail_cnt=0;
    while(1)
    {
    	ret=get_venc_stat(no);
    	if(ret==ENC_STAT_OK)
    	{
    		printf("视频编码器%d状态正常!\n",no);
    		break;
    	}
    	else
    	{
    		if(++fail_cnt==15)
    		{
    			printf    ("视频编码器%d状态异常,stat=%d!\n",no,ret);
    			gtlogerr("视频编码器%d状态异常,stat=%d!\n",no,ret);
    		}
    		printf("videoenc%d state=%d!!!\n",no,ret);	
    	}
    	sleep(1);
    }    
    return ret;

}
/**********************************************************************************************
 * 函数名	:connect_video_enc_playback()
 * 功能	:连接到指定编号的视频编码器，录像填充录像回放缓冲区
 * 输入	:no:需要连接的视频编码器序号
 *			name:用户名
 *			pre_sec:从前面邋pre_sec秒的位置开始连接
 *					0:表示从最新的元素连接
 *				      >0:表示要提前秒数
 *					   如果pre_sec的值大于缓冲池中的总帧数则从最早的元素进行连接
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int connect_video_enc_playback(IN int no,IN char *name,IN int pre_sec)
{
	int ret;
	int rc;
	int stat;
	int frate=0;
	media_attrib_t *attrib=NULL;
	video_format_t *video=NULL;
	if(no>=get_videoenc_num())
		return -EINVAL;
	ret=connect_media_read(&video_enc_playback[no], get_video_enc_key(no),name,MSHMPOOL_NET_USR);
	if(ret>=0)
	{
		stat=get_venc_stat(no);
		if(stat==ENC_STAT_OK)
		{//编码器状态正常
			attrib=get_venc_attrib(no);
			if(attrib!=NULL)
			{
				video=&attrib->fmt.v_fmt;
				frate=video->v_frate;
				rc=move_media_place(&video_enc_playback[no],-(frate*pre_sec));	
				
			}
		}
	}
	return ret;
}

/**********************************************************************************************
 * 函数名	:connect_video_enc_keyframe()
 * 功能	:I帧缓冲池线程专用,连接到指定编号的视频编码器
 * 输入	:no:需要连接的视频编码器序号
 *	name:用户名
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int connect_video_enc_keyframe(IN int no,char *name)
{
	int ret;
	
	if(no>=get_videoenc_num())
		return -EINVAL;
	ret=connect_media_read(&video_enc_keyframe[no], get_video_enc_key(no),name,MSHMPOOL_NET_USR);
	return ret;
}


/**********************************************************************************************
 * 函数名	:disconnect_video_enc()
 * 功能	:断开到指定编码器的连接
 * 输入	:no:需要断开的视频编码器序号
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int disconnect_video_enc(IN int no)
{
	if(no>=get_videoenc_num())
		return -EINVAL;	
	return disconnect_media_read(&video_enc[no]);
}
int disconnect_video_enc_keyframe(IN int no)
{
	if(no>=get_videoenc_num())
		return -EINVAL;	
	return disconnect_media_read(&video_enc_keyframe[no]);
}
/**********************************************************************************************
 * 函数名	:reactive_video_enc()
 * 功能	:重新激活到视频编码器的连接
 * 输入	:no:需要重新激活的视频编码器序号
 * 返回值	:0表示成功，负值表示失败
 * 注		:应用程序应定期调用,防止由于自己一段时间没有响应而被
 *			 videoenc服务断开
 **********************************************************************************************/
int reactive_video_enc(IN int no)
{
	if(no>=get_videoenc_num())
		return -EINVAL;	
	return reactive_media_usr(&video_enc[no]);
}

int get_video_enc_remain(IN int no)
{
    return read_media_remain(&video_enc[no]);
}
int get_video_enc_playback_remain(IN int no)
{
    return read_media_remain(&video_enc_playback[no]);
}

int reactive_video_enc_keyframe(IN int no)
{
	if(no>=get_videoenc_num())
		return -EINVAL;	
	return reactive_media_usr(&video_enc_keyframe[no]);
}

/**********************************************************************************************
 * 函数名	:read_video_frame()
 * 功能	:从指定编号的编码器中读取一帧数据
 * 输入	:no:需要读取数据的视频编码器序号
 *			:buf_len:frame缓冲区的长度,要是一帧数据超过这个长度则会报错
 * 输出	:frame:准备存放视频帧的缓冲区
 *			 eleseq:视频帧的序号
 *			 flag:视频帧的标志
 * 返回值	:正值表示读取到的字节数，负值表示出错
 **********************************************************************************************/
int read_video_frame(IN int no,OUT void *frame,IN int buf_len,OUT int *eleseq,OUT int *flag)
{
	return read_media_resource(&video_enc[no],frame,buf_len,eleseq,flag);
}

/**********************************************************************************************
 * 函数名	:read_video_playback()
 * 功能	:从指定编号的编码器中读取一帧回放数据
 * 输入	:no:需要读取数据的视频编码器序号
 *			:buf_len:frame缓冲区的长度,要是一帧数据超过这个长度则会报错
 * 输出	:frame:准备存放视频帧的缓冲区
 *			 eleseq:视频帧的序号
 *			 flag:视频帧的标志
 * 返回值	:正值表示读取到的字节数，负值表示出错
 **********************************************************************************************/

int read_video_playback(IN int no,OUT void *frame,IN int buf_len,OUT int *eleseq,OUT int *flag)
{
	return read_media_resource(&video_enc_playback[no],frame,buf_len,eleseq,flag);
}

int read_move_media_place(IN int no,int place)
{
	return move_media_place(&video_enc[no], place);
}


/**********************************************************************************************
 * 函数名	:read_video_keyframe()
 * 功能	:从指定编号的编码器中读取一帧数据,供I帧缓冲池专用
 * 输入	:no:需要读取数据的视频编码器序号
 *			:buf_len:frame缓冲区的长度,要是一帧数据超过这个长度则会报错
 * 输出	:frame:准备存放视频帧的缓冲区
 *			 eleseq:视频帧的序号
 *			 flag:视频帧的标志
 * 返回值	:正值表示读取到的字节数，负值表示出错
 **********************************************************************************************/
int read_video_keyframe(IN int no,OUT void *frame,IN int buf_len,OUT int *eleseq,OUT int *flag)
{
	return read_media_resource(&video_enc_keyframe[no],frame,buf_len,eleseq,flag);
}


//#define 	ENC_NO_INIT		0		//编码器没有初始化
//#define	ENC_STAT_OK		1		//编码器工作正常
//#define	ENC_STAT_ERR		2		//编码器故障	
/**********************************************************************************************
 * 函数名	:get_venc_stat()
 * 功能	:获取指定编号的视频编码器状态
 * 输入	:no:需要获取状态的视频编码器编号
 * 返回值	:负值表示出错-EINVAL:参数错误 -ENOENT:设备还没有连接
 *					ENC_NO_INIT:未初始化
 *					ENC_STAT_OK:状态正常
 *					ENC_STAT_ERR:编码器故障
 **********************************************************************************************/
//#include <file_def.h>//test !!!
int get_venc_stat(IN int no)
{
	media_source_t 	*media=&video_enc[no];
	media_attrib_t	*attrib=NULL;
	if(no>=get_videoenc_num())
		return -EINVAL;
	if(media->dev_stat<0)
		return -ENOENT;
	attrib=media->attrib;
	if(attrib==NULL)
		return -ENOENT;
	return attrib->stat;
}


int get_venc_stat_keyframe(IN int no)
{
	media_source_t 	*media=&video_enc_keyframe[no];
	media_attrib_t	*attrib=NULL;
	if(no>=get_videoenc_num())
		return -EINVAL;
	if(media->dev_stat<0)
		return -ENOENT;
	attrib=media->attrib;
	return attrib->stat;
}

/**********************************************************************************************
 * 函数名	:get_venc_attrib()
 * 功能	:获取指定视频编码器的附加信息结构指针
 * 输入	:no:需要访问的视频编码器编号
 * 返回值	:指向描述视频编码器附加属性的结构指针
 *			 NULL表示出错 ，参数错误，编码器未连接
 **********************************************************************************************/
media_attrib_t *get_venc_attrib(int no)
{
	media_source_t 	*media=&video_enc[no];
	if(no>=get_videoenc_num())
		return NULL;
	if(media->dev_stat<0)
		return NULL;
	return media->attrib;
}
media_attrib_t *get_venc_attrib_keyframe(int no)
{
	media_source_t 	*media=&video_enc_keyframe[no];
	if(no>=get_videoenc_num())
		return NULL;
	if(media->dev_stat<0)
		return NULL;
	return media->attrib;
}
