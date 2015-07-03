#include <math.h>
#include <commonlib.h>
#include <devinfo.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include  "playback.h"
#include "playback_pool.h"

int playback_connect_media(struct hd_playback_struct *phdplayback)
{

    int ret;
    int frate = 0;
    media_attrib_t *attrib = NULL;
    video_format_t *video = NULL;
    media_source_t *pmedia = NULL;
    int encodeno;
    int pre_sec;

    encodeno = phdplayback->channel;
    pre_sec = phdplayback->pre_connect;

/*
     if(encodeno>= MAX_RECORD_CHANNEL)
  
          return PLAYBACK_ERR_PARAM;
*/
    
  
    //初始化数据
    //不申请内存，外部直接调用
    ret = init_media_rw(&phdplayback->media,MEDIA_TYPE_VIDEO, encodeno, 0);
    if(ret != 0)
    {
        printf("error in connect media read and exit\n");
        return PLAYBACK_ERR_CONNECT;
    }

    //frame = (struct stream_fmt_struct *)phdplayback->media.temp_buf;
    //buflen = MAX_FRAME_SIZE;//防止缓冲区溢出

    //MSHMPOOL_LOCAL_USR
    ret = connect_media_read(&phdplayback->media ,get_video_enc_key(encodeno)/*0x30000+5*/, "playback", MSHMPOOL_NET_USR);
    if(ret < 0)
    {
        printf("error in connect media read and exit\n");
        return PLAYBACK_ERR_CONNECT;
    }

    pmedia = (media_source_t *)&(phdplayback->media);
    attrib = pmedia->attrib;
    if(attrib != NULL)
    {
         
        if(attrib->stat != ENC_STAT_OK)
        {
            printf("error in connect stat error\n");
            return PLAYBACK_ERR_CONNECT;
        }
        video = &attrib->fmt.v_fmt;
        frate = video->v_frate;
        ret = moveto_media_packet(&phdplayback->media,-(frate*pre_sec));
    }
    else
    {
        printf("error in connect media read and exit\n");
        return PLAYBACK_ERR_CONNECT;
    }

    printf("connect success...\n");
    return PLAYBACK_SUCCESS;
  
}


int playbackreadpoolframe(struct hd_playback_struct *phdplayback, void *buf, int buf_len, int *seq, int *flag)
{ 
   
    struct stream_fmt_struct *frame = NULL;
    int video_flag = 1;
    int ret;
    int new_seq = -1;
   
    ret = read_media_resource(&phdplayback->media,frame,buf_len,&new_seq,&video_flag);
    if(ret<0)
    {
        printf(" record_file_threadread_video_frame failed ret=%d\n",ret);
        return PLAYBACK_ERR_POLL_NODATA;
    }

   *seq = new_seq;
   *flag = video_flag;
    //printf("video_frame,read:%d,framelen:%d seq:%x,video_flag:%d!\n",ret,frame->len,new_seq,video_flag);

    ret=read_media_remain(&phdplayback->media);
    if((ret>=100)&&((ret%10) == 0))//每10条记录一次
    {
        printf("warn:record_file_thread 有%d帧视频没有读取!\n",ret);
    }

     return PLAYBACK_SUCCESS;
    
}

int playbackclosepool(struct hd_playback_struct *phdplayback)
{
    int ret;
    
    ret = disconnect_media_read(&phdplayback->media);
    if(ret != 0)
    {
        return PLAYBACK_ERR_CONNECT;
    }
    return PLAYBACK_SUCCESS;

}


