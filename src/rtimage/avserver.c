/** @file	       avserver.c
 *   @brief 	提供音视频服务的相关函数实现
 *   @date 	2007.03
 */
#include "rtimage2.h"
#include <media_api.h>
#include <venc_read.h>
#include <gtthread.h>
#include <ime6410api.h>     ///FIXME stream_fmt_struct结构定义放到其它地方!
#include <AVIEncoder.h>
#include <commonlib.h>
#include <gt_errlist.h>
#include "rtnet_cmd.h"
#include "net_avstream.h"
#include <soundapi.h>
#include <audiofmt.h>
#include "audio_pool.h"
#include "maincmdproc.h"
#include <gtsocket.h>
#include <devinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<dirent.h>
#include <time.h>
#include "avserver.h"
#include <hddbuf.h>
#include <gtsf.h>
//zsk add for getticketcount
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include <nvp1918.h>
#include "play_back.h"
#include "avilib.h"
#include <onviflib.h>
//#define DEBUG
#define TCP_SEND_RESERVE_AUDIO      32*1024                                 ///<为发送音频数据保留的缓冲区
#define TCP_SEND_ADJUST_STEP         128*1024                                ///<每次调节tcp发送缓冲区大小的步长
#define TCP_SEND_BUF_MAX                1024*1024                               ///<tcp发送缓冲区的最大值
static unsigned char    avi_head_buf[MAX_VIDEO_ENCODER][1024*3];   ///<存放avi头的缓冲区
static  int             avi_head_len[MAX_VIDEO_ENCODER];                ///<avi_head_buf中每个元素的有效字节数
static  unsigned char frame_head_buf[MAX_VIDEO_ENCODER][28];
extern unsigned long  net_pkts_sta;
struct timeval  venc_last_tv[MAX_VIDEO_ENCODER];       ///<视频编码器0最后一帧的时间戳
struct timeval  venc_pb_tv[MAX_AUDIO_ENCODER];
 ///由于8180的视频格式和以前不太一样，传输时需转换一下
static __inline__ void convert_intra_frame(unsigned long *flag)
{
    #define IFRAME_HEAD         0xb0010000
    *flag=IFRAME_HEAD;
}
 
/** 
 *   @brief     初始化音视频服务用到的相关变量
 *   @return   0表示成功,负值表示失败
 */ 
static int init_avserver(void)
{
    init_video_enc();                   ///<初始化视频编码器缓冲池
    //init_audio_enc_pool();          ///<初始化音频编码器缓冲池//zsk del
    init_audio_enc();
//#ifdef USE_ADEC_POOL
	init_audio_dec_pool();
//#endif
    return 0;
}
/** 
 *   @brief     生成指定编号编码器的avi头信息
 *   @param  no 视频编码器编号
 *   @param  attrib 视频编码器属性结果指针
 *   @return   0表示成功负值表示失败
 *                 avi 头信息填充在avi_head_buf数组中
 */
static int gen_venc_avi_head(IN int no)
{
    int     ret;
    struct defAVIVal davi;
    video_format_t	*vfmt=NULL;
	audio_format_t  *afmt=NULL;
	media_attrib_t * a_attrib=NULL;
	media_attrib_t * v_attrib=NULL;

    if(no>=MAX_VIDEO_ENCODER)
        return -EINVAL;

    memset((void*)&davi,0,sizeof(struct defAVIVal));

	v_attrib=get_venc_attrib(no);
	if(v_attrib!=NULL)
	{
		vfmt=&v_attrib->fmt.v_fmt;
		switch(vfmt->format)
		{
			case VIDEO_H264:
				sprintf(davi.v_avitag,"H264");
			break;
			case VIDEO_MJPEG:
				sprintf(davi.v_avitag,"MJPG");
			break;
			case VIDEO_MPEG4:
				sprintf(davi.v_avitag,"divx");
			break;
			default:
				sprintf(davi.v_avitag,"H264");
			break;

		}
		davi.ispal=vfmt->ispal;				    //是否是pal制视频
		davi.nr_stream=2;					    //有几个媒体流 1表示仅视频 2表示音频视频都有
		davi.v_width=vfmt->v_width;		        //图像宽度
		davi.v_height=vfmt->v_height;		    //图像高度
		davi.v_frate=vfmt->v_frate;			    //帧率
		davi.v_buffsize=vfmt->v_buffsize;	    //建议缓冲区大小


	}

	a_attrib=get_aenc_attrib(no);
	if(a_attrib!=NULL)
	{
		afmt=&a_attrib->fmt.a_fmt;
		davi.a_sampling=afmt->a_sampling;
		davi.a_channel=afmt->a_channel;
		davi.a_bitrate=afmt->a_bitrate;
		davi.a_wformat=afmt->a_wformat;	         //音频格式
		davi.a_nr_frame=afmt->a_nr_frame;
		
	}

    ret=FormatAviHeadBufDef(&davi,avi_head_buf[no], sizeof(avi_head_buf[no]));
    avi_head_len[no]=ret;

    return 0;
}
int preflag[MAX_VIDEO_ENCODER]={0};
int sendflag=0;
int  write_rawaudio2file_send(void *buf,int len)
 {
	 static FILE *afp=NULL; 	 ///<文件指针
	 char filename[50];
	 int ret=0;
		
	 if(afp==NULL&&sendflag==0)
	 {
	 		sendflag=1;
		 sprintf(filename,"/mnt/zsk/send.g711u");
		 afp=fopen(filename,"wb");
		 if(afp==NULL)
		 {
			 printf("can't create file:%s errno=%d:%s!!\n",filename,errno,strerror(errno));
			 return -errno;
		 }
	 }
	 if(afp!=NULL)
	 {
		 ret=fwrite(buf,1,len,afp);
		 fflush(afp);
	 }
	 return ret;
	 
 }
 static int  write_rawaudio2file_pre(void *buf,int len,avi_t **AVI)
 {
	 //static FILE *afp=NULL; 	 ///<文件指针
	 char filename[50];
	 int ret=0;
	 if(*AVI==NULL||preflag[0]==0)
	 {
	      preflag[0]=1;
		 sprintf(filename,"/mnt/zsk/pre.g711u");
		 *AVI=AVI_open_output_file(filename);
		 if(*AVI==NULL)
		 {
			printf("AVI_OUTPUT_FILE open failed!! name[%s]\n",filename);
			close((*AVI)->fdes);
		 
			return -1; 
		 }
		 AVI_set_audio(*AVI,2,16000,16,WAVE_FORMAT_MULAW,0);
		 (*AVI)->mode=AVI_MODE_WRITE;
		 /*afp=fopen(filename,"wb");
		 if(afp==NULL)
		 {
			 printf("can't create file:%s errno=%d:%s!!\n",filename,errno,strerror(errno));
			 return -errno;
		 }
		 */
		 
	 }
	 if(*AVI!=NULL)
	 {
		 ret=AVI_write_audio(*AVI,buf,len);
		 //ret=fwrite(buf,1,len,afp);
		 //fflush(afp);
	 }
	 return ret;
	 
 }

 /** 
 *   @brief     发送媒体数据结构到网络描述符
 *   @param  fd 以及打开的网络描述符
 *   @param  frame 指向要发送的数据结构指针
 *   @param  v_fmt 视频编码格式VIDEO_MPEG4, VIDEO_H264 ,VIDEO_MJPEG
 *   @return  正值表示发送出去的字节数,负值表示出错
 */ 
static inline int write_frame_data2trans
	(IN int fd,IN char * head_buf,IN struct stream_fmt_struct *frame,int v_fmt)
{
    //static  unsigned long  iframe_head=0xb0010000; 
	unsigned char *p;
	int len;
	int ret=-1;
	//char	head_buf[4+sizeof(frame->tv)+4+8];
	memset(head_buf,0,GTSF_HEAD_SIZE);
	struct NCHUNK_HDR *chunk=&frame->chunk;
    if(fd<0)
            return -EINVAL;
       
	if(frame->media==MEDIA_VIDEO)
    {   
		chunk->chk_id=IDX1_VID;         
		/*
        if((v_fmt!=VIDEO_MJPEG)&&( frame->type==FRAMETYPE_I))
        {
	    //v2.25 由于控件还不支持00000100的关键字,故这段转换还得加上if(v_fmt!=VIDEO_MPEG4)				///<MPEG4编码时也不对I帧头进行标记了,因为解码器不能解经过这样转换的GTVS3000的图像,新版本转发也支持00000100的关键帧标志了
                	convert_intra_frame((void*)frame->data);       ///传输时进行数据转换
        }
		*/
	}
	else if(frame->media==MEDIA_AUDIO)
	{
		chunk->chk_id=IDX1_AID;
		//write_rawaudio2file_send(&frame->data,frame->len); 
	}	
	frame->chunk.chk_siz=frame->len;	
	/*
	if((frame->media==MEDIA_VIDEO)&&(v_fmt==VIDEO_MJPEG))
	{ 
		frame->chunk.chk_siz+=4;    
	}
	*/
    do
	{


	      ///发送数据头
		memcpy(head_buf,&frame->chunk.chk_id,4);
		memcpy(&head_buf[4],&frame->tv,sizeof(frame->tv));
		memcpy(&head_buf[4+sizeof(frame->tv)],&frame->chunk.chk_siz,4);
		ret=fd_write_buf(fd,head_buf,4+sizeof(frame->tv)+4);
		if(ret!=(4+sizeof(frame->tv)+4))
		{			
			break;
		}
		/*
		if((frame->media==MEDIA_VIDEO)&&(v_fmt==VIDEO_MJPEG))
		{
	    	ret=fd_write_buf(fd,&iframe_head,sizeof(iframe_head));
	    	if(ret!=4)
	        	break;
		}
		*/


	  	///发送数据体
		p=(unsigned char*)frame->data;
		//MAMAYATOTO
		len=(int)frame->len;
		if(frame->type==0)
		{
			//printf("frame I ------------timestamp[%d.%d]\n",frame->tv.tv_sec,frame->tv.tv_usec);
		}


		
		ret=fd_write_buf(fd,p,len);	
		//printf("发送数据体 fd =%d,ret=%d,len=%d chk_size=%ld\n",fd,ret,len,frame->chunk.chk_siz);
		if(ret>0)
		{
			ret+=(4+sizeof(frame->tv)+4);
			if((frame->media==MEDIA_VIDEO)&&(v_fmt==VIDEO_MJPEG))
			{
				ret+=4;
			}
	    }
	}while(0);
	return ret;
}

static inline int write_frame_data2sdk
	(IN int fd,IN char * head_buf,IN struct stream_fmt_struct*frame,IN media_format_t * fmt)
{
	//int len;
	int ret=0;
	//unsigned short media;
	//unsigned short frame_type;
	//unsigned long  * head_tag=NULL;
	gtsf_stream_fmt	*pstream=NULL;;
	stream_video_format_t  *pV_fmt=NULL;
    stream_audio_format_t  *pA_fmt=NULL;
    stream_format_t  *pmedia_format=NULL;
	pstream = (gtsf_stream_fmt*)head_buf;
	pmedia_format = &pstream->media_format;
	//copy head
	
	//len=frame->len;
	//media=frame->media;
	//frame_type=frame->type;
	memset(pstream,0,GTSF_HEAD_SIZE);
	pstream->mark = GTSF_MARK;
	pstream->encrypt_type = 0;
	pstream->len = frame->len;
	if(frame->media==MEDIA_VIDEO)
   	{

		pstream->type=MEDIA_VIDEO;
		pV_fmt = (stream_video_format_t  *)&pmedia_format->v_fmt;
		pV_fmt->format = VIDEO_H264;
		pV_fmt->type=frame->type;
		if(( fmt->v_fmt.v_width==720)&&(fmt->v_fmt.v_height== 576))
		{
		pV_fmt->ratio = RATIO_D1_PAL;
		}
		else if(( fmt->v_fmt.v_width == 704)&&(fmt->v_fmt.v_height== 576))
		{
		pV_fmt->ratio = RATIO_D1_NTSC;
		}
		else if(( fmt->v_fmt.v_width == 352)&&(fmt->v_fmt.v_height== 288))
		{
		pV_fmt->ratio = RATIO_CIF_PAL;
		}
		else if(( fmt->v_fmt.v_width == 320)&&(fmt->v_fmt.v_height== 240))
		{
		pV_fmt->ratio = RATIO_CIF_NTSC;
		}    

   }//video
   else
   {
		pstream->type=MEDIA_AUDIO;
        pA_fmt = (stream_audio_format_t  *)&pmedia_format->a_fmt;
        pA_fmt->a_channel = 1;
        pA_fmt ->a_wformat = 7;
        pA_fmt->a_sampling = 8;
        pA_fmt->a_bits = 8;
        pA_fmt->a_bitrate = 64;

   }
	//audio
	//printf("mark[%#x]\ntype[%#x]\nencrypt_type[%#x]\nchannel[%#x]\nversion[%#x]\nlen[%d]\n",pstream->mark,pstream->type,pstream->encrypt_type,\
	pstream->channel,pstream->version,pstream->len);
	do
	{
		ret=fd_write_buf(fd,(char*)pstream,GTSF_HEAD_SIZE);
		if(ret!=GTSF_HEAD_SIZE)
		{			
			break;
		}
		if(frame->type==FRAMETYPE_I)
		{
			  
			convert_intra_frame((void*)frame->data); 	  ///传输时进行数据转换
		}

		//head_tag=frame->data;
		//*head_tag=0x01000000;
		ret=fd_write_buf(fd,(char*)frame->data,frame->len);
		if(ret>0)
		{
			ret+=GTSF_HEAD_SIZE;
		
		}
	}while(0);
	return ret;

}

////////////////////////////map相关////////////////////////////////////////

/** 
 *   @brief     向发送媒体信息映像中增加一个数据元素信息
 *   @param  map 指向发送媒体信息映像结构的指针
 *   @param  flag 元素的类型
 *   @param  bytes 新加元素的字节数
 *   @return   0表示成功 负值表示出错
 */ 
static inline int add_ele2map(IO stream_send_map_t *map,IN int flag,IN int size)
{
    map_frame_t *t_frame=&map->frame_map[map->tail];             ///<队尾指针

    t_frame->flag=flag;
    t_frame->size=size;
    if(++map->tail>=MAX_MAP_BUF_FRAMES)
        map->tail=0;
    if(map->tail==map->head)
    {///队列满了,应该不会发生这种情况，如果发生说明需要增大MAX_MAP_BUF_FRAMES
        printf("tail=head=%d ",map->tail);
        showbug();
    }
    
    if(flag==FRAMETYPE_PCM)
        map->a_frames++;                ///<音频
    else
        map->v_frames++;                ///<视频
    
    return 0;
}





#if 0

/** 
 *   @brief     计算map中的有效字节数
 *   @param  map 指向发送媒体信息映像结构的指针
 *   @return   map中的有效字节数
 *   测试用
 */
static inline int calc_map(IO stream_send_map_t *map)
{///测试用,
    map_frame_t *frame;
    int     total=0;
    int     i;
    int     head=map->head;
    int     tail=map->tail;
    while(head!=tail)
    for(i=0;i<MAX_MAP_BUF_FRAMES;i++)
    {
        if(head!=tail)
        {
            frame=&map->frame_map[head];
            total+=frame->size;
            if(++head>=MAX_MAP_BUF_FRAMES)
                head=0;
        }
        else
        {
            break;
        }
    }
    return total;
}
/** 
 *   @brief     打印map中的元素
 *   @param  map 指向发送媒体信息映像结构的指针
 *   @return   map中的有效元素数
 *   测试用
 */
static inline int print_map(IN stream_send_map_t *map)
{///测试用,打印map中的元素
    map_frame_t *frame;
//    int     i;
    int     total=0;
    int     head=map->head;
    int     tail=map->tail;
    printf("map:");
    while(head!=tail)
    {
        frame=&map->frame_map[head];
        printf("%d,",frame->size);
        if(++head>=MAX_MAP_BUF_FRAMES)
            head=0;
        total++;
    }
    printf("(%d)\n",total);
    return total;
}
#endif
 /** 
 *   @brief     从发送媒体信息映像中删除指定字节数的数据
 *   @param  map 指向发送媒体信息映像结构的指针
 *   @param  bytes 要删除的字节数
 *   @return   正值表示删除的元素数
 */ 
static inline int del_ele_from_map(stream_send_map_t *map,int bytes)
{
    int i;
    int remain=bytes;                 ///<还需要删除的字节数
    int dels=0;                             ///<删除的元素数
    map_frame_t *frame;             ///<指向map中元素的指针
    if(map->head==map->tail)
        return 0;                           ///<队列空
    if(bytes<=0)
        return 0;
    for(i=0;i<MAX_MAP_BUF_FRAMES;i++)
    {
        frame=&map->frame_map[map->head];
        if(frame->size<=remain)
        {
            remain-=frame->size;
            if(++map->head>=MAX_MAP_BUF_FRAMES)
                map->head=0;
            dels++;
            if(frame->flag==FRAMETYPE_PCM)
                map->a_frames--;
            else
                map->v_frames--;
            if(map->head==map->tail)
            {///队列已经空了
                //printf("map is empty!! head=tail=%d! remain=%d vframe=%d aframe=%d\n",map->head,remain,map->v_frames,map->a_frames);
                break;
            }
        }
        else
        {
            frame->size-=remain;
            break;
        }
    }
    return dels;
}
///////////////////////////////////////////////////////////////////////////////////

#if 0
 /** 
 *   @brief     调节用户的发送socket缓冲区大小
 *   @param  usr 用户指针
 *   @param  size 要调节的大小 正值表示需要增大 负值表示需要减小
 *   @return   0表示成功 负值表示失败
 */ 
static int adjust_usr_sock_buf(av_usr_t *usr,int size)
{
    int ret;
    int send_len=net_get_tcp_sendbuf_len(usr->fd);
    tcprtimg_svr_t    *p=get_rtimg_para();
    
    send_len+=size;

    #if 0
    //if(send_len>TCP_SEND_BUF_MAX)
    if(send_len>200*1024)
    {
        return -ENOMEM;
    }
    #endif
    //将缓冲区长度设为1
    net_set_tcp_sendbuf_len(usr->fd,1);
    ////send_len=150*1024;

    //将缓冲区设置为最大
    send_len=p->tcp_max_buff*1024;
    ret=net_set_tcp_sendbuf_len(usr->fd,send_len);
    if(ret<0)
    {
        return -errno;
    }
    send_len=net_get_tcp_sendbuf_len(usr->fd);
    //printf("adjuest_buf to %dk!!\n",send_len/1024);
    //告诉应用程序只有(max*1024)*80%的空间可用，但是实际设置的是max*1024
    usr->sock_attr.send_buf_len=send_len*0.8;               ///<只能有80%的空间可以使用,超过的话会引起阻塞
    //zw-add
    // 看看当前没有发送出去的数据包有没有达到整个缓冲区的80%
    if(usr->sock_attr.send_buf_remain>usr->sock_attr.send_buf_len)
    {
        printf("原来的send_buf_remain[%d]太大\n",usr->sock_attr.send_buf_remain);
        //如果达到了的话，就临时将未发送的数据包个数表面上减少到send_buff_len那么长,主要目的是进入后面的扔包阶段
        //减少send_buf_remain,骗一下应用程序
        usr->sock_attr.send_buf_remain=usr->sock_attr.send_buf_len;
       //// usr->sock_attr.send_buf_remain=usr->sock_attr.send_buf_len*2/5;
        printf("调整send_buf_remain=[%d]\n",usr->sock_attr.send_buf_len);
    }

    //zw-add

    return 0;
}
#endif
static int adjust_usr_sock_buf(av_usr_t *usr,int size)
{
    int ret;
    int send_len=net_get_tcp_sendbuf_len(usr->fd);
    tcprtimg_svr_t    *p=get_rtimg_para();
    
    send_len+=size;

    #if 0
    //if(send_len>TCP_SEND_BUF_MAX)
    if(send_len>200*1024)
    {
        return -ENOMEM;
    }
    #endif
    //将缓冲区长度设为1
    net_set_tcp_sendbuf_len(usr->fd,1);
    ////send_len=150*1024;

    //将缓冲区设置为最大
    send_len=p->tcp_max_buff*1024;
    ret=net_set_tcp_sendbuf_len(usr->fd,send_len);
    if(ret<0)
    {
        return -errno;
    }
    send_len=net_get_tcp_sendbuf_len(usr->fd);
    //printf("adjuest_buf to %dk!!\n",send_len/1024);
    //告诉应用程序只有(max*1024)*80%的空间可用，但是实际设置的是max*1024
    usr->sock_attr.send_buf_len=send_len*0.8;               ///<只能有80%的空间可以使用,超过的话会引起阻塞
    //zw-add
    // 看看当前没有发送出去的数据包有没有达到整个缓冲区的80%
    if(usr->sock_attr.send_buf_remain>usr->sock_attr.send_buf_len)
    {
        printf("原来的send_buf_remain[%d]太大\n",usr->sock_attr.send_buf_remain);
        //如果达到了的话，就临时将未发送的数据包个数表面上减少到send_buff_len那么长,主要目的是进入后面的扔包阶段
        //减少send_buf_remain,骗一下应用程序
        usr->sock_attr.send_buf_remain=usr->sock_attr.send_buf_len;
       //// usr->sock_attr.send_buf_remain=usr->sock_attr.send_buf_len*2/5;
        printf("调整send_buf_remain=[%d]\n",usr->sock_attr.send_buf_len);
    }

    //zw-add

    return 0;
}


 /** 
 *   @brief     发送指定媒体服务器的数据给相应的用户
 *   @param  enc 指向媒体服务器的指针
 *   @param  frame 指向要发送的数据结构指针
 *   @param  usr    被发送的用户对象
 *   @param  seq    媒体数据的序列号
 *   @param  flag   媒体数据的标记
 *   @return   正值表示成功负值表示出错,0表示什么也没有发送
 */
 unsigned long switch_cnt_2=0;
 unsigned long switch_cnt_1=0;
  int send_i_flag=1;
  int send_i_interval=0;
  int old_i_interval=0;
  int drop_p_flag=0;

int old_v_frames=0;
struct timeval last_tv;
int debug=0;

void begin_debug(int signo)
{
	gtloginfo("receive sigusr1,begin_debug\n");
	debug=1;

}

void end_debug(int signo)
{
	gtloginfo("receive sigusr2,end_debug\n");
	debug=0;

}
static inline int send_media_frame2usr
	(av_usr_t *usr,media_source_t *enc,struct stream_fmt_struct *frame,int seq,int flag)
{
    int ret;
    int send_bytes;
    stream_send_info_t  *send_info=&usr->send_info;         ///<发送信息
    socket_attrib_t         *sock_attr=&usr->sock_attr;         ///<socket信息
    int len;
	int no;
    tcprtimg_svr_t    *p=get_rtimg_para();
	
	no=enc->no;
    pthread_mutex_lock(&usr->u_mutex);
    do
    {
    	//netcmdavihead
        if(!send_info->send_ack_flag)
        {///<还没有发送响应的avi头

            if(enc->media_type==MEDIA_TYPE_VIDEO)
            {
				gen_venc_avi_head(enc->no);
                ret=send_rtstream_ack_pkt(usr->fd,RESULT_SUCCESS,(char*)avi_head_buf[enc->no],avi_head_len[enc->no]);
                if(ret>=0)
                {   ///发送响应包成功
                    sock_attr->send_buffers=get_fd_out_buffer_num(usr->fd);
                    add_ele2map(&send_info->map,FRAMETYPE_P,sock_attr->send_buffers);   ///<向map中放一个假数据
                    send_info->total_put=sock_attr->send_buffers;               ///<初始化发送字节数
                    send_info->total_out=0;                                                 
                    send_info->send_ack_flag=1;                                         ///<已经发送过文件头
                    send_info->first_flag=1;
					send_info->require_i_flag=1;
                    ret=1;                                                                            ///<从下一帧数据开始发送
                    break;
                }
            }
            else
            {
                ret=0;
                break;
            }
        }
        else
        {            
        		if(debug==1)
        		{
        			printf("pool_read_flag______[%d]\n",flag);
        			gtloginfo("pool_read_flag______[%d]\n",flag);
        		}
                if(send_info->first_flag)
                {
                    if(flag==FRAMETYPE_P)
                    {
	                    	if(get_playback_stat(no)<0&&send_info->require_i_flag==1)
	                    	{
	                    		require_videoenc_iframe(enc->no);
								send_info->require_i_flag=0;
	                    	}
                    
                        ret=0;
                        break;
                   
                    }
                    else if(flag==FRAMETYPE_I)
                    {
                    
                        send_info->first_flag=0;						
                    }
                }
            
        }

        //send_buf_remain和send_buffers两个变量的含义有些近似
        //send_buf_remain是socket缓冲区实时读取出来的当前剩余的字节数
        //因为socket发送是异步的，send_buffers记录的是上次发送完之后的缓冲区的数据字节数，在
        //本次发送数据前用send_buffers减去 send_buf_remain则表示这两次之间发送了多少字节数


        ///socket发送缓冲区中的数据
        sock_attr->send_buf_remain=get_fd_out_buffer_num(usr->fd);                    ///<socket发送缓冲区内的字节数
        send_bytes=sock_attr->send_buffers-sock_attr->send_buf_remain;              ///<刚发送出去的字节数
        sock_attr->send_buffers=sock_attr->send_buf_remain;
        ret=del_ele_from_map(&send_info->map,send_bytes);
        send_info->total_out+=send_bytes;                                                             ///<总共发送出去的字节数
        if(send_bytes<0)
        {
        	gtloginfo("send_bytes=%d\n",send_bytes);
            printf("send=%d sock_attr->send_buf_remain=%d!!!xxx!!!!!!!!!!!send_bytes=为负了!!!!!!!!!!!!!!\n",send_bytes,sock_attr->send_buf_remain);
        }
        else
       {	
  			//printf("=============================================ramin=%d============当前缓冲区中的帧数=%d\n",sock_attr->send_buf_remain,send_info->map.v_frames);
		}

	
        //在每次发送视频帧的时候都会实时的修改用于记录统计数据的变量
        //同时，每次发送的时候也会通过上次的统计数据来判断当前的缓冲区大小情况。
        if(enc->media_type==MEDIA_TYPE_VIDEO)
        {///处理发送视频数据
            //如果当前的缓冲区内音频帧加上视频帧再加保留的10帧大于MAX_MAP_BUF_FRAMES的话
            //本次就不继续发送数据了，直接丢掉本次的视频帧，等socket缓冲区中的数据发送完毕
            //然后使用drop_v_flag,send_i_flag和drop_v_frames记录当前的丢帧计数状态
            //跟下面的条件对比，这个丢帧机制是根据tcprtimg的缓冲区判断的，tcprtimg拥有自己的缓冲区
            //这个缓冲区负责从videoenc的缓冲区中读取数据，如果这个读取速度比较快的话，导致读
            //出来的数据超过了tcprtimg自己的最大缓冲区的上限的话，tcprtimg就不再将读取到的音视频数
            //据帧发到socket的发送缓冲区去发送了，而是采用丢掉当前的多出来的帧，继续发送着缓
            //冲区中原有的帧，直到tcprtimg读到的数据帧小于MAX_MAP_BUF_FRAMES为止，然后再将读到的数据
            //帧接着发送到socket的发送缓冲区，关于根据socket的发送缓冲区情况决定是否需要丢帧的机
            //制详见下面的分析
            if((send_info->map.v_frames+send_info->map.a_frames+10)>MAX_MAP_BUF_FRAMES)
            {///为音频数据预留10帧空间
                    printf("line:[%d]usr:%d not enough map drop video:%d!!break\n",__LINE__,usr->no,(int)frame->len);
                    		gtloginfo("line:[%d]usr:%d not enough map drop video:%d!!break\n",__LINE__,usr->no,(int)frame->len);
                    send_info->drop_v_flag=1;
                    send_info->send_i_flag=0;
                    send_info->drop_v_frames++;
                    ret=0;
                    break;                     
            }

            //从socket缓冲区中的可用空间那里判断是否需要调整缓冲区大小，调整大小时已将当前的这
            //一帧给丢了
            //if((sock_attr->send_buf_len-sock_attr->send_buf_remain-TCP_SEND_RESERVE_AUDIO)<(frame->len+sizeof(struct stream_fmt_struct)*2))
            ////printf("line[%d] [%d]--[%d].....drop_len=[%d].............switch_cnt=%ld\n",__LINE__,sock_attr->send_buf_len,sock_attr->send_buf_remain,(sock_attr->send_buf_len)*3/5,switch_cnt);                

			   if(net_pkts_sta<p->pkts_limit)
                {              
 
                    //当空余空间小于80*1024时进入此处，调整缓冲区为200*1024，并且丢帧，不再向下运行
                    //等待网卡那边把目前剩余的帧给发送出去。当检测到剩余空间大于80*1024时则转到
                    //增大缓冲区设置，将缓冲区设置为800*1024,然后继续向它发送数据
                    ret=adjust_usr_sock_buf(usr,TCP_SEND_ADJUST_STEP);  ///<socket发送缓冲区不足,调节发送缓冲区
                    //if((sock_attr->send_buf_len-sock_attr->send_buf_remain-TCP_SEND_RESERVE_AUDIO)<(frame->len+sizeof(struct stream_fmt_struct)*2))

                    //下面这个接口需要结合adjust_usr_sock_buf()来判断。adjust_usr_sock_buf()会判断remain是否达到
                    //整个缓冲区的80%,如果达到了，则肯定会进入下面的接口，因为已经把remain重新赋值了,remain和buf_len相等，如果没有达到80%，那么就先计算一下，已经发送出去的数据包量是否达到缓冲区总量的3/5,如果没有达到，那也需要扔掉新的数据包,不再接客了，先把剩下的未发送出去的包包们伺候完了再说。
                     if((sock_attr->send_buf_len-sock_attr->send_buf_remain< ((sock_attr->send_buf_len) *3/5)))
                    { ///还是没有分配到足够的缓冲区
                        //printf("line[%d]usr:%d not enough buf!%d<%d drop video frame !!\n",__LINE__,usr->no,(sock_attr->send_buf_len-sock_attr->send_buf_remain-TCP_SEND_RESERVE_AUDIO),(int)(frame->len+sizeof(struct stream_fmt_struct)*2));
                        printf("都扔了line[%d]usr:%d not enough buf!%d<%d drop video frame !!\n",__LINE__,usr->no,(sock_attr->send_buf_len-sock_attr->send_buf_remain),80*1024);
                        send_info->drop_v_flag=1;
                        send_info->send_i_flag=0;
                        send_info->drop_v_frames++;
                        ret=0;
                        break;                    
                    }               
                }
                else 
                {   

                    //重新设置回800k
                    len=net_get_tcp_sendbuf_len(usr->fd);
                    if(len!=512*1024)
                    {
                        len=512*1024;
                        net_set_tcp_sendbuf_len(usr->fd,len);
                        sock_attr->send_buf_len=len*0.8;
                        //remain应该不需要设置了
                       // printf("我又回到800*1024了\n");
                    }
                }
				

            ///判断是否到丢弃阈值
            if(send_info->drop_v_flag)
            {///仍在丢弃视频状态
                if(send_info->drop_p_flag)
                {///还在丢弃帧状态

			//恢复map.v_frames有两个途径，一个是v_frames小于10，一个是下面的(v_frames+10)<th_drop_v
                    if(send_info->map.v_frames<10)
                    {///可以恢复视频传输
                        send_info->drop_v_flag=0;
                    }
                    else
                    {
                        send_info->send_i_flag=0;           ///<需要遇到I帧才可以恢复
                        send_info->drop_v_frames++;     ///<又丢失了一帧视频
                        ret=0;
                        printf("line:[%d] drop frame:%d\n",__LINE__,send_info->drop_v_frames);
                        break;
                    }
                }

                //如果drop_v_flag在被丢弃状态，则需要给它一个机会，用来申诉重新获取重生
                if((send_info->map.v_frames+10)<usr->th_drop_v)
                {///可以恢复视频传输
                    //取消了丢弃drop_v_flag,意味着I帧和P帧都有机会发送了，只是有机会，不是一定啊
                    send_info->drop_v_flag=0;
                }
                else
                {
                    //什么都不要想了，还是继续自生自灭吧
                    printf("line:[%d]usr:%d %d frame > th_drop_v(%d) drop frame:%d!\n",__LINE__,usr->no,send_info->map.v_frames,usr->th_drop_v,(int)frame->len);
                    send_info->drop_v_frames++;
                    ret=0;
                    break;
                }
            }
 
            //事到如今，如果当前帧是I帧，那就发送了吧
            if(flag==FRAMETYPE_I)
            {
                send_info->send_i_flag=1;
                send_info->last_v_seq=seq;
			//printf("=======我发送了一个I帧\n");
            }
            else
            {///p帧
                if(send_info->drop_p_flag)
                {
                    if((send_info->map.v_frames+10)<usr->th_drop_p)
                    {//恢复传送p帧
                        send_info->drop_p_flag=0;
                    }
                    else
                    {
                        printf("line[%d]usr:%d drop p frame:%d!\n",__LINE__,usr->no,(int)frame->len);
                        send_info->send_i_flag=0;
                        send_info->drop_v_frames++;
                        ret=0;
                        break;
                    }                    
                }
                else
                {
        
                    if((send_info->map.v_frames)>=usr->th_drop_p)
                    {
                        printf("line:[%d]usr:%d drop p frame:%d!\n",__LINE__,usr->no,(int)frame->len);
                        send_info->send_i_flag=0;
                        send_info->drop_v_frames++;
                        send_info->drop_p_flag=1;
                        send_info->drop_v_frames++;
                        ret=0;   
                        break;
                    }

                }
                if(send_info->send_i_flag)
                {
                    

                }
                else
                {///还没有发送i帧
                     printf("line[%d],usr:%d drop p frame:%d!\n",__LINE__,usr->no,(int)frame->len);      ///<必须先发送i帧后才能发送p帧
                    send_info->send_i_flag=0;
                    send_info->drop_v_frames++;
                    ret=0;                  
                    break;                                   
                }                
            }


      }//end if(enc->media_type==MEDIA_TYPE_VIDEO)
      else
        {///音频数据

            if(send_info->map.a_frames>40)
            {///最多缓冲40包音频
                    printf("usr:%d too many audio pkt, drop audio:%d!!\n",usr->no,(int)frame->len);
                    send_info->drop_a_flag=1;
                    send_info->drop_a_frames++;
                    ret=0;
                    break;                       
            }
            if((send_info->map.v_frames+send_info->map.a_frames+2)>MAX_MAP_BUF_FRAMES)
            {///预留2帧空间
                    printf("usr:%d not enough map drop audio:%d!!\n",usr->no,(int)frame->len);
                    send_info->drop_a_flag=1;
                    send_info->drop_a_frames++;
                    ret=0;
                    break;                     
            }
            if((sock_attr->send_buf_len-sock_attr->send_buf_remain-1024)<(frame->len+sizeof(struct stream_fmt_struct)*2))
            {

                ret=adjust_usr_sock_buf(usr,TCP_SEND_ADJUST_STEP);      ///<socket缓冲区不足,调节缓冲区
                if((sock_attr->send_buf_len-sock_attr->send_buf_remain-1024)<(frame->len+sizeof(struct stream_fmt_struct)*2))
                { ///还是没有分配到足够的缓冲区
                    printf("usr:%d not enough buf!%d<%d drop audio frame !!\n",usr->no,(sock_attr->send_buf_len-sock_attr->send_buf_remain-1024),(int)(frame->len+sizeof(struct stream_fmt_struct)*2));
                    send_info->drop_a_flag=1;
                    send_info->drop_a_frames++;
                    ret=0;
                    break;                    
                }
               
            }
            if(send_info->drop_a_flag)
            {
                if(send_info->map.a_frames<10)
                {///恢复音频发送
                    send_info->drop_a_flag=0;
                }
                else
                {
                    printf("usr:%d drop a frame:%d!\n",usr->no,(int)frame->len);
                    ret=0;
                    break;
                }
            }
           
        
        }
    	if(1)
    	{
    		//printf("send_frame_type_____[%d]_____length[%d]\n",frame->type,frame->len);
    		//gtloginfo("send_frame_type_____[%d]_____length[%d]\n",frame->type,frame->len);
    	}
		if(usr->stream_idx!=-1)//sdk usr
		{
			ret=write_frame_data2sdk(usr->fd,(char*)frame_head_buf[no],frame,&(enc->attrib->fmt));
			if(ret>0)
			{
				sock_attr->send_buffers+=ret;									///<已放入缓冲区的字节数
				add_ele2map(&send_info->map,frame->type,ret);
				sock_attr->send_buf_remain+=ret;							///<缓冲区生育字节数
				send_info->total_put+=ret;			   
			}
       	 	else
       		{
	            printf("DBG write_frame_data2net ret=%d:%d:%s\n",ret,errno,strerror(errno));
				printf(":%d(%s),ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
	            gtloginfo(":%d(%s),ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
				pthread_mutex_unlock(&usr->u_mutex);
				rtnet_av_close_connect(usr->no);
				return ret;
        	}


			
		}
		else//trans usr
        {
        	ret=write_frame_data2trans(usr->fd,(char*)frame_head_buf[no],frame,enc->attrib->fmt.v_fmt.format);                ///<将媒体数据包发送到网络
        	if(ret>0)
        	{
	            sock_attr->send_buffers+=ret;                                   ///<已放入缓冲区的字节数
	            add_ele2map(&send_info->map,frame->type,ret);
	            sock_attr->send_buf_remain+=ret;                            ///<缓冲区生育字节数
	            send_info->total_put+=ret;             
        	}
       	 	else
       		{
	            printf("DBG write_frame_data2net ret=%d:%d:%s\n",ret,errno,strerror(errno));
				printf(":%d(%s),ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
	            gtloginfo(":%d(%s),ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
        	}
		}
    }while(0);
    pthread_mutex_unlock(&usr->u_mutex);
	
    return ret;

}
 /** 
 *   @brief     发送指定媒体服务器的数据给相应的用户
 *   @param  enc 指向媒体服务器的指针
 *   @param  frame 指向要发送的数据结构指针
 *   @param  seq  媒体数据的序列号
 *   @param  flag   媒体数据的标记
 *   @return   正值表示用户该媒体发送的用户数,负值表示出错
 */ 
static int send_media_frames
	(media_source_t *enc,struct stream_fmt_struct *frame,int seq,int flag)
{

    tcprtimg_svr_t    *p=get_rtimg_para();
    av_usr_t            *usr=NULL;
    int                     total=sizeof(p->av_server.av_usr_list)/sizeof(av_usr_t);        ///<总用户数
    int                     nums=0;                 ///<发送的用户计数
    int                     i;
    int                     ret;
	int 					no;
	stream_send_info_t  *send_info;         ///<发送信息
	no=enc->no;

    for(i=0;i<total;i++)
    {
        usr=&p->av_server.av_usr_list[i];
		send_info=&usr->send_info;
        if(usr->serv_stat<=0)
        {
            continue;                                                         ///<还没有收到订阅命令
        }
        else
        {

            if(enc->media_type==MEDIA_TYPE_VIDEO)       ///<视频信息是最基本的，必须发送
            {


            	if(enc->no==usr->venc_no)       
                {               
                    ///<视频编码器编号与用户订阅的匹配


					
                    ret=send_media_frame2usr(usr,enc,frame,seq,flag);
					//printf("send_media_frame2usr ret =%d\n",ret);
                    if(ret>0)
                        nums++;
                }
            }
            else
            {
            	
            	if(enc->no==usr->venc_no)       
                {       
                    if(usr->serv_stat==3)                               ///<订阅了音频服务
                    {///<目前只有一个音频编码器 所以不用进行特殊判断
                        ret=send_media_frame2usr(usr,enc,frame,seq,flag);
                        if(ret>0)
                            nums++;
                    }
            	}
               
            }
        }
        
    }
    return nums;
}





 unsigned long GetTickCount()
 {
		 static unsigned long s_mode = 0;
		 static unsigned long s_tt = 0;
		 if (s_mode == 0)
		 {
				 unsigned long tps = (unsigned long)sysconf(_SC_CLK_TCK);
				 printf("tps = %lu\r\n", tps);
				 if (1000 % tps == 0)
				 {
						 s_tt = 1000 / tps;
						 s_mode = 1;
				 }
				 else
				 {
						 s_tt = tps;
						 s_mode = 2;
				 }
		 }
		 struct tms t;
		 const unsigned long dw = (unsigned long)times(&t);
		 return (s_mode == 1 ? (dw * s_tt) : (unsigned long)(dw * 1000LL / s_tt));
 }

 /** 
 *   @brief     视频服务线程
 *   @param  para 指向描述视频编码器的指针
 *   @return   永不返回
 */ 
static void *venc_server_thread(void *para)
{
	media_source_t *enc=(media_source_t*)para;  
	int                     ret,ret2;
	struct stream_fmt_struct      *frame_buf=NULL;                    ///<存放视频帧的缓冲区
	struct stream_fmt_struct      *frame_buf_playback=NULL;
	int                     buflen;                                      ///<frame_buf的长度
	int 					buflen_playback;
	int                     no;                                            ///<编码器序号enc->no
	int                     old_seq=-1,seq=-1;                 ///<媒体数据序号
	int                     old_seq_playback=-1,seq_playback=-1;
	int                     flag;                                          ///<媒体数据的标记
	int						flag_playback;
	int play_back_no;
	tcprtimg_svr_t    *p=get_rtimg_para();
	playback_t * pb=get_playback_parm();
	play_back_no=enc->no;
	if(enc==NULL)
	{
		printf     ("venc_server_thread para=NULL exit thread!!\n");
		gtloginfo("venc_server_thread para=NULL exit thread!!\n");
		pthread_exit(NULL);
	}
	no=enc->no;
	media_source_t * enc_playback = get_video_enc_playback(no);
	printf     (" start venc_server_thread (%d)...\n",no);
	gtloginfo(" start venc_server_thread (%d)...\n",no);
	frame_buf=(struct stream_fmt_struct *)enc->temp_buf;                                            ///<临时缓冲区
	frame_buf_playback=(struct stream_fmt_struct *)enc_playback->temp_buf;
	buflen=enc->buflen;                                                       //临时缓冲区长度
	buflen_playback=enc_playback->buflen;

	ret=connect_video_enc_succ(no,(int)STREAM_SEC ,"rt_usr",0);            ///<连接视频编码器缓冲池
	//gen_venc_avi_head(no,get_venc_attrib(no));          

	pthread_mutex_lock(&pb->mutex);

	if(get_frate_from_venc(no)>0);
	{
		pb->pb_vct[no]=p->playback_pre*get_frate_from_venc(no); //这个只是最大等待时间
	}
		
	pthread_mutex_unlock(&pb->mutex);
	while(1)
	{ 
			if(pb->pb_vct[no] == 0)
			{
						
					if(pb->pb_venc[no]==0) //连接一次性的flag
					{
						connect_video_enc_playback(no,(int)STREAM_SEC,"pb_usr",p->playback_pre);
						pthread_mutex_lock(&pb->mutex);
						pb->pb_venc[no]=-1;
						pthread_mutex_unlock(&pb->mutex);
					}
					ret=read_video_playback(no,(void*)frame_buf_playback,buflen_playback,&seq_playback,&flag_playback);
					old_seq_playback++;
					if(old_seq_playback!=seq_playback)
					{
						printf("[%s]read_playback_frame old_seq+1=%d newseq=%d!!\n",__FUNCTION__,old_seq_playback,seq_playback);
						old_seq_playback=seq_playback;
					}

			}
				ret=read_video_frame(no,(void*)frame_buf,buflen,&seq,&flag);
				//printf("video_frame flag=%#x\n",flag);
				venc_last_tv[no].tv_sec=frame_buf->tv.tv_sec;
				venc_last_tv[no].tv_usec=frame_buf->tv.tv_usec;
				old_seq++;
				if(old_seq!=seq)
				{
					printf("[%s]read_video_frame old_seq+1=%d newseq=%d!!\n",__FUNCTION__,old_seq,seq);
					old_seq=seq;
					continue;
				}

				if(pb->pb_vct[no]>0)
				{
						pthread_mutex_lock(&pb->mutex);
						pb->pb_vct[no]--;
						//printf("pb_vct[%d]=%d\n",no,pb->pb_vct[no]);
						pthread_mutex_unlock(&pb->mutex);
				}
				
				
				
				//ret=get_video_enc_remain(no);
				//printf("实时通道读写指针差[%d]\n",ret);


#ifdef DEBUG
		if(tmp_cnt++>750&&no==0&&playback==1)
	
		{
			set_playback_en(play_back_no);
			playback=0;

		}
		else
		{
			if(tmp_cnt>1500&&get_playback_stat(play_back_no)==1)
				set_playback_cancel(play_back_no);
		}

#endif

		
		if(ret>=0)
		{

			if(get_playback_stat(play_back_no)<0)
			{
				//读取实时视频
				send_media_frames(enc,frame_buf,seq,flag);    



			}	
			else if(get_playback_stat(play_back_no)==no)//已成功连接回放池
			{
			
				if(get_playback_frame_adjust(no)==0)
				{	
					ret2=find_playback_iframe(no);
					if(ret2<0)
						gtlogerr("find_playback_iframe err\n");
					set_playback_frame_adjust(no,ret2);
				}
				//读取回放视频
					
				send_media_frames(enc_playback,frame_buf_playback,seq_playback,flag_playback);
				
			}
			//printf("编码器[%d]发送给用户数量[%d]\n",enc->no,ret);
		}
		else
		{
			///读数据出错
			printf("read_video_frame%d ret=%d!!\n",no,ret);
			if(ret!=-EINTR)
				sleep(1);
		}
	}  
	return NULL;
}

int posix_memalign(void **memptr, size_t alignment, size_t size);

/** 
 *   @brief     写一包音频数据到指定编号的音频编码器缓冲池
 *   @param  aenc 音频编码器缓冲池结构指针
 *   @param  frame 指向音频帧的指针
 *   @return   0表示成功,负值表示失败
 */ 
static inline int write_adec_frame_pool(media_source_t *adec, struct stream_fmt_struct *frame)
{
	int s_len=frame->len+sizeof(struct stream_fmt_struct)-sizeof(frame->len);
	adec->attrib->stat=ENC_STAT_OK;
	return write_media_resource(adec,frame,s_len,frame->type);
}
static inline int read_audio_frame_pool(IN media_source_t *aenc,OUT void *buf,IN int buf_len,OUT int *eleseq,OUT int *flag)

{
	return read_media_resource(aenc,buf,buf_len,eleseq,flag);
}
//#ifdef SAVE_RAW_AUDIO
///将原始音频数据存入磁盘文件,用于测试

//#endif



#if 0
/** 
 *   @brief     计算map中的有效字节数
 *   @param  map 指向发送媒体信息映像结构的指针
 *   @return   map中的有效字节数
 *   测试用
 */
static inline int calc_map(IO stream_send_map_t *map)
{///测试用,
    map_frame_t *frame;
    int     total=0;
    int     i;
    int     head=map->head;
    int     tail=map->tail;
    while(head!=tail)
    for(i=0;i<MAX_MAP_BUF_FRAMES;i++)
    {
        if(head!=tail)
        {
            frame=&map->frame_map[head];
            total+=frame->size;
            if(++head>=MAX_MAP_BUF_FRAMES)
                head=0;
        }
        else
        {
            break;
        }
    }
    return total;
}


static inline int print_map(IN stream_send_map_t *map)
{///测试用,打印map中的元素
    map_frame_t *frame;
//    int     i;
    int     total=0;
    int     head=map->head;
    int     tail=map->tail;
    printf("map:");
    while(head!=tail)
    {
        frame=&map->frame_map[head];
        printf("%d,",frame->size);
        if(++head>=MAX_MAP_BUF_FRAMES)
            head=0;
        total++;
    }
    printf("(%d)\n",total);
    return total;
}
#endif



/** 
 *   @brief     音视频上行服务的秒处理程序
 */
void avserver_second_proc(void)
{
    int                         i;
    int                         total_video=get_videoenc_num();
	int							total_audio=get_audio_num();
    tcprtimg_svr_t        *p=get_rtimg_para();
    av_server_t           *av_svr=&p->av_server;
    int                         av_usr_num=av_svr->wan_usrs+av_svr->lan_usrs;
    static int                 old_av_usrs=0;               ///<上一次的用户数
    static int                 av_freecnt=0;                ///<音视频上行服务的空闲时间

    if(old_av_usrs!=av_usr_num)
    {
        if(old_av_usrs==0)
        {
            set_net_enc_busy(1);
        }
        old_av_usrs=av_usr_num;
        av_freecnt=0;
    }
    else
    {
        if(old_av_usrs==0)
        {
            if(++av_freecnt==30)//30秒后报告空闲状态10)
            {
                set_net_enc_busy(0);
            }
        }
        else
            av_freecnt=0;
    }
        


    for(i=0;i<total_video;i++)
    {
		if(get_venc_stat(i)>=0)
		{
				
			if(reactive_video_enc(i)==1||reactive_video_enc_playback(i)==1)
			{
				gtloginfo("%d!\n",i);
				printf("%s %d!\n",__FUNCTION__,i);
				exit(0);

			}
		}

		

    }
	for(i=0;i<total_audio;i++)
	{
		if(get_aenc_stat(i)>=0)
		{

			if(reactive_audio_enc(i)==1||reactive_audio_enc_playback(i)==1)
			{
				gtloginfo("%d!\n",i);
				printf("%d!\n",i);
				exit(0);

			}
		}

	}
        
}

/** 
 *   @brief     音频服务线程
 *   @param  para 指向描述音频编码器的指针
 *   @return   0表示成功,负值表示失败
 */ 

static void *aenc_server_thread(void *para)
{
    tcprtimg_svr_t    *p=get_rtimg_para();
    struct stream_fmt_struct  *frame_buf=NULL;
	struct stream_fmt_struct  *frame_buf_playback=NULL;
    media_source_t *enc=(media_source_t*)para;      ///<编码器指针
    playback_t * pb=get_playback_parm();
    int                     ret;
    int                     buflen;                                       ///<frame_buf的长度
    int 					buflen_playback;
    int                      no=enc->no;                                           ///<编码器序号enc->no
    int                     seq=-1;                                     ///<媒体数据序号
    int eleseq=-1,old_seq=-1,flag;
	int old_seq_playback=-1,seq_playback=-1,flag_playback;
	avi_t * AVI=NULL;
    if(enc==NULL)
    {
        printf     ("aenc_server_thread para=NULL exit thread!!\n");
        gtlogerr ("aenc_server_thread para=NULL exit thread!!\n");
        pthread_exit(NULL);
    }


   
    printf     (" start aenc_server_thread (%d)...\n",no);
    gtloginfo(" start aenc_server_thread (%d)...\n",no);

	media_source_t * enc_playback = (media_source_t*)get_audio_enc_playback(no);
	frame_buf=(struct stream_fmt_struct  *)enc->temp_buf;
	frame_buf_playback=(struct stream_fmt_struct *)enc_playback->temp_buf;
	buflen=enc->buflen; 			
	buflen_playback=enc_playback->buflen;
	ret=connect_audio_enc_succ(no,(int)STREAM_SEC,"rt_usr");			
	//pb->pb_act[no]=p->playback_pre*30; //暂定30*30帧后连接

	if(get_frate_from_aenc(no)>0);
	{
		pb->pb_act[no]=p->playback_pre*get_frate_from_aenc(no); //这个只是最大等待时间
	}

    while(1)
    {
    		if(pb->pb_act[no] == 0)
			{
					if(pb->pb_aenc[no]==0)
						
					{	
						connect_audio_enc_playback(no,(int)STREAM_SEC,"pb_usr",p->playback_pre);
						pthread_mutex_lock(&pb->mutex);
						pb->pb_aenc[no]=-1;
						pthread_mutex_unlock(&pb->mutex);
					}
					ret=read_audio_playback(no,(void*)frame_buf_playback,buflen_playback,&seq_playback,&flag_playback);
					old_seq_playback++;
					if(old_seq_playback!=seq_playback)
					{
						printf("[%s]read_playback_frame old_seq+1=%d newseq=%d!!\n",__FUNCTION__,old_seq_playback,seq_playback);
						old_seq_playback=seq_playback;
						//continue;
					}
			}
			


		    ret = read_audio_frame(no,(void *)frame_buf,buflen,&eleseq,&flag);
		//	printf("audio_frame flag=%#x\n",flag);
		//	printf("__________buflen[%d]_____frame_len[%d]\n",buflen,frame_buf->len);
			
			frame_buf->tv.tv_sec=venc_last_tv[no].tv_sec;
			frame_buf->tv.tv_usec=venc_last_tv[no].tv_usec;
			old_seq++;
			if(old_seq!=eleseq)
			{
				printf("[%s]read_audio_frame old_seq+1=%d newseq=%d!!\n",__FUNCTION__,old_seq,eleseq);
				old_seq=eleseq;
				continue;
			}

			if(pb->pb_act[no]>0)
			{
					pthread_mutex_lock(&pb->mutex);
					pb->pb_act[no]--;
					//printf("pb_act[%d]=%d\n",no,pb->pb_act[no]);
					pthread_mutex_unlock(&pb->mutex);
			}
		
			if(ret>=0)
			{
	
				if(get_playback_stat(no)<0)
				{
					//读取实时视频
					ret=send_media_frames(enc,frame_buf,seq,flag);	  
				}	
				else
				{
					//读取回放视频
					ret=send_media_frames(enc_playback,frame_buf_playback,seq_playback,flag_playback);
				}

			}
			else
			{
				///读数据出错
				printf("read_audio_frame%d ret=%d!!\n",no,ret);
				if(ret!=-EINTR)
					sleep(1);
			}

		/*    if(ret<=0)
		    {
		        printf("read_audio_data ret=%d!\n",ret);
				gtlogerr("read_audio_data ret=%d!\n",ret);
				gtlogerr("get_audio_enc_remain=%d,no=%d\n",get_audio_enc_remain(no),no);
		        sleep(1);
		        continue;
		    }
		    */
	//printf("read adio data ret=%d\tlen=%d\t type=%d\t chkid=%x\n",ret,frame->len,frame->type,frame->chunk.chk_id);
//#define SAVE_RAW_AUDIO
#ifdef SAVE_RAW_AUDIO
			if(enc->no==0)
				write_rawaudio2file_pre(&frame_buf->data,frame_buf->len,&AVI);      ///<将读到的原始数据存入磁盘
#endif
		


   //     seq++;
  	//  	ret=send_media_frames(enc,frame,eleseq,FRAMETYPE_PCM);                     ///<发送给用户


    
       //zsk del ret=write_audio_frame_pool(enc,frame);                                               ///<写入缓冲池
    }
	AVI_close_output_file(AVI);
    return NULL;    
}



 /** 
 *   @brief     创建音视频服务线程
 *   @return   0表示成功,负值表示失败
 */ 
int create_av_server(void)
{
  tcprtimg_svr_t      *p=get_rtimg_para();
 // av_server_t          *svr=&p->av_server;
  media_source_t     *venc=NULL;                  ///<视频编码器
  media_source_t     *aenc=NULL;
  int                         ret=0;
  int                         total;
  int                         i;
  init_avserver();                    ///<初始化音视频上行服务用到的变量

  init_playback_parm();
  //音频编码部分放在videoenc里面做，rtimage只取
  ///创建音频服务线程
  total = get_audio_num();
  for(i=0;i<total;i++)//目前只启一个编码器连接线3
  {
	  	aenc=get_audio_enc(i);
	  	ret=gt_create_thread(&aenc->thread_id,aenc_server_thread,(void*)aenc);
  }

  ///创建视频服务线程
	total=get_video_num();
	for(i=0;i<total;i++)
	{

	//设置音频上行属性

		venc=get_video_enc(i);
		ret=gt_create_thread(&venc->thread_id, venc_server_thread, (void*)venc);

	}

    return 0;
}




