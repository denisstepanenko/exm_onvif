/** @file	       net_aplay.c
 *   @brief 	接收并处理客户端发来的音频下行命令及数据流
 *   @date 	2007.04
 */
#include "rtimage2.h"

#include <commonlib.h>
#include <gtthread.h>
#include <netinfo.h>    
#include <gt_com_api.h>
#include <audiofmt.h>
#include "rtnet_cmd.h"
#include <gt_errlist.h>
#include <gtsocket.h>
#include <fcntl.h>
//#include "common/sample_comm.h" //zsk add

#define AUDIO_PLAY_PKT_SIZE     320    //2048 zsk mod
#define AAC_HEAD_SIZE           4
#define MAX_AAC_PKT_SIZE		4096
#define TW2865_FILE     "/dev/tw2865dev" 
#define AUDIO_ADPCM_TYPE ADPCM_TYPE_DVI4/* ADPCM_TYPE_IMA, ADPCM_TYPE_DVI4*/
#define AUDIO_AAC_TYPE AAC_TYPE_AACLC   /* AAC_TYPE_AACLC, AAC_TYPE_EAAC, AAC_TYPE_EAACPLUS*/
#define G726_BPS MEDIA_G726_16K         /* MEDIA_G726_16K, MEDIA_G726_24K ... */
#define AMR_FORMAT AMR_FORMAT_MMS       /* AMR_FORMAT_MMS, AMR_FORMAT_IF1, AMR_FORMAT_IF2*/
#define AMR_MODE AMR_MODE_MR74         /* AMR_MODE_MR122, AMR_MODE_MR102 ... */
#define AMR_DTX 0
extern unsigned long GetTickCount();
extern int write_rawaudio2file_send();
/*
typedef struct tagSAMPLE_AIAENC_SINGLE_S
{
    HI_S32 s32AencChn; 
    FILE *pfd;
    HI_S32 u32GetStrmCnt;
} SAMPLE_AIAENC_SINGLE_S;

typedef struct tagSAMPLE_AIAENC_S
{
    HI_BOOL bStart;
    pthread_t stAencPid;
    HI_U32 u32AeChnCnt;
    HI_S32 s32BindAeChn;    
    SAMPLE_AIAENC_SINGLE_S astAencChn[AENC_MAX_CHN_NUM];
} SAMPLE_AIAENC_S;

typedef struct tagSAMPLE_AOADEC_S
{
    HI_BOOL bStart;
    HI_S32 AdChn; 
    FILE *pfd;
    pthread_t stAdPid;
} SAMPLE_AOADEC_S;


static SAMPLE_AIAENC_S s_stSampleAiAenc;
static SAMPLE_AOADEC_S s_stSampleAoAdec;
static PAYLOAD_TYPE_E s_enPayloadType = PT_G711U;

static HI_BOOL s_bAioReSample = HI_FALSE;
static AUDIO_RESAMPLE_ATTR_S s_stAiReSmpAttr;
static AUDIO_RESAMPLE_ATTR_S s_stAoReSmpAttr;
*/

/**   
	*	计算两个时间的间隔，得到时间差	 
	*	@param	 struct   timeval*	 resule   返回计算出来的时间   
	*	@param	 struct   timeval*	 x			   需要计算的前一个时间   
	*	@param	 struct   timeval*	 y			   需要计算的后一个时间   
	*	return	 -1   failure	,0	 success   
**/   
int   timeval_subtract(struct	timeval*   result,	 struct   timeval*	 x,   struct   timeval*   y)   
{	

	  if(x->tv_sec > y->tv_sec)   
		  return   -1;	 
  
	  if((x->tv_sec == y->tv_sec) && (x->tv_usec > y->tv_usec))   
		  return   -1;	 
  
	  result->tv_sec = (y->tv_sec-x->tv_sec);	
	  result->tv_usec = (y->tv_usec-x->tv_usec);   
  
	  if(result->tv_usec < 0)	
	  {   
		  result->tv_sec--;   
		  result->tv_usec += 1000000;	
	  }   
  
	  return   0;	
}	

/** 
 *   @brief     初始化网络音视频下行服务的全部用户变量
 *   @return   0表示成功,负值表示失败
 */ 
static int init_net_aplay(void)
{
    tcprtimg_svr_t  *p=get_rtimg_para();
    aplay_server_t *svr=&p->aplay_server;
    aplay_usr_t      *usr=NULL;
    int                   i;
    pthread_mutex_init(&svr->l_mutex,NULL);
    pthread_mutex_init(&svr->s_mutex,NULL);         
    svr->listen_fd=-1;
    
    ///用户信息初始化
    for(i=0;i<(TCPRTIMG_MAX_APLAY_NO+1);i++)
    {
        usr=&svr->usr_list[i];
        memset((void*)usr,0,sizeof(aplay_usr_t));
        pthread_mutex_init(&usr->u_mutex,NULL);
        usr->fd=-1;
        usr->no=i;
        usr->pid=-1;
        usr->thread_id=-1;
        usr->magic=0x55aa;                                  ///<已初始化标志
        
    }
   
   	return 0;
}

/** 
 *   @brief     初始化音视频用户数据结构(用于有新连接到来时)
 *   @param  usr 用户信息结构指针
 *   @return   无
 */     
static void init_aplay_usr_info(IO aplay_usr_t *usr)
{
    tcprtimg_svr_t *p=get_rtimg_para();

    pthread_mutex_lock(&usr->u_mutex);
    
    memset(&usr->addr,0,sizeof(struct sockaddr_in));
    usr->fd=-1;
    usr->th_timeout=p->th_timeout;
    usr->timeout_cnt=0;
    gettimeofday(&usr->start_time,NULL);
    memset(&usr->last_cmd_time,0,sizeof(struct timeval));
    sprintf(usr->name,"%s","newuser");
    usr->serv_stat=0;
    usr->aenc_no=-1;
    memset(&usr->sock_attr,0,sizeof(socket_attrib_t));
    memset(&usr->recv_info,0,sizeof(stream_recv_info_t));
    pthread_mutex_unlock(&usr->u_mutex);
}

/** 
 *   @brief     将一个用户地址信息加入用户列表
 *   @param  usr 存放用户信息的结构指针
 *   @param  guest 新用户的ip地址和端口号信息
 *   @return   0表示成功 1表示忙 负值表示出错
 *   @warnning 调用此函数前调用者应该已经取得了媒体服务的s_mutex
 */ 
static int add_aplay_usr_list(aplay_usr_t *usr,struct sockaddr_in *guest)
{
    tcprtimg_svr_t      *p=get_rtimg_para();
    aplay_server_t     *svr=&p->aplay_server;
    if((usr==NULL)||(guest==NULL))
        return -EINVAL;       
    svr->usrs++;
    if(svr->usrs>p->max_aplay_usrs)
    {
        svr->usrs--;
        return 1;
    }
    memcpy(&usr->addr,guest,sizeof(struct sockaddr_in));    ///<复制用户信息
    return 0;
    
}
/** 
 *   @brief     从音频下行服务中删除指定的用户
 *   @param  usr 存放用户信息的结构指针
 *   @return   0表示成功  负值表示出错
 *   @warnning 调用此函数前调用者应该已经取得了媒体服务的s_mutex
 */ 
static int del_aplay_usr_list(aplay_usr_t *usr)
{
    tcprtimg_svr_t      *p=get_rtimg_para();
    aplay_server_t     *svr=&p->aplay_server;
    if(usr==NULL)
        return -EINVAL;
    svr->usrs--;
    if(svr->usrs<0)
    {
        showbug();
        logbug();
    }
    

    ///删除用户信息
    pthread_mutex_lock(&usr->u_mutex);
    usr->fd=-1;
    usr->serv_stat=-1;
    usr->aenc_no=-1;
    pthread_mutex_unlock(&usr->u_mutex);
    return 0;
}

/** 
 *   @brief     返回音视频下行服务的响应信息
 *   @param  fd 目的描述符
 *   @param  result 返回的错误代码
 *   @param  需要挂在返回信息answer_data后面的信息
 *   @param  datalen buf中有效数据的个数
 *   @return   负值表示出错,非负表示成功
 */ 
int send_aplay_ack_pkt(int fd,WORD result,char* buf,int datalen)
{
	int rc;
	DWORD	sendbuf[100];
	struct gt_usr_cmd_struct *cmd;
	struct gt_pkt_struct *send;
	struct viewer_subscribe_answer_audio_struct *answer;
	int answerlen;
	
	if(fd<0)
		return -1;
	send=(struct gt_pkt_struct *)sendbuf;
	cmd=(struct gt_usr_cmd_struct*)send->msg;
	cmd->cmd=VIEWER_SUBSCRIBE_ANSWER_AUDIO;
	cmd->en_ack=0;
	cmd->reserve0=0;
	cmd->reserve1=0;
	answer=(struct viewer_subscribe_answer_audio_struct  *)cmd->para;
	rc=get_devid(answer->dev_id);
	answer->status=result;
	rc=1;
	memcpy((BYTE*)&answer->audio_trans_id,(BYTE*)&rc,4);
	if(buf!=NULL)
	{
			answerlen=datalen;
			memcpy(answer->answer_data,buf,answerlen);
	}
	else
	{
		switch(result)
		{
			case 0:
				
			break;
			case ERR_DVC_BUSY:	
				answerlen=0;
			break;
			default:
				answerlen=0;
			break;
		}
	}
	answer->answer_data_len=answerlen;
	cmd->len=sizeof(struct viewer_subscribe_answer_audio_struct)-sizeof(answer->answer_data)+answerlen+6;	
	rc=gt_cmd_send_noencrypt(fd,send,cmd->len+2,NULL,0);

	return rc;
}



/** 
 *   @brief     处理网络用户发来的音频下行订阅命令
 *   @param  usr 收到命令的用户结构指针
 *   @param  cmd 从网络收到的命令结构指针
 *   @return   0表示返回后继续执行,负值表示返回后需要断开连接
 */ 
static int proc_audio_subscrib_cmd(IO aplay_usr_t *usr,IN struct gt_usr_cmd_struct* cmd)
{
    char                 *ptemp=NULL;
    int                    ret;
    int                   *sample_rate=NULL;
	tcprtimg_svr_t *p=get_rtimg_para();
    struct viewer_subscribe_audio_cmd_struct *subscribe=(struct viewer_subscribe_audio_cmd_struct*)cmd->para;
    if(cmd->cmd!=VIEWER_SUBSCRIBE_AUDIO)
        return -EINVAL;

    sample_rate=(int*)subscribe->audioheader_data;
    printf     ("收到 %s 的音频下行订阅命令account=%s type=%d(%s) sample_rate=%d\n",inet_ntoa(usr->addr.sin_addr),subscribe->account,subscribe->audiotype,get_audio_fmt_str(subscribe->audiotype),*sample_rate);
    gtloginfo("收到 %s 的音频下行订阅命令account=%s type=%d(%s) sample_rate=%d\n",inet_ntoa(usr->addr.sin_addr),subscribe->account,subscribe->audiotype,get_audio_fmt_str(subscribe->audiotype),*sample_rate);




    pthread_mutex_lock(&usr->u_mutex);
	usr->serv_stat=1;
    usr->a_sam_rate=*sample_rate;                           ///<音频采样率
    switch(subscribe->audiotype) 
    {
    	case AUDIO_PLAY_TYPE_PCM:
    	case AUDIO_PLAY_TYPE_UPCM:
		case AUDIO_PLAY_TYPE_AAC:
			usr->a_fmt=subscribe->audiotype;
			break;
    	default:
    		usr->a_fmt=AUDIO_PLAY_TYPE_UNKNOW;
			break;
    }
    if((usr->a_sam_rate<7500)||(usr->a_sam_rate>8500))
        usr->a_sam_rate=8000;                                   ///<参数超出范围
    strncpy(usr->name,(char *)subscribe->account,20);   ///<用户名最长为19
    usr->name[19]='\0';
    ptemp=strchr(usr->name,'\n');
    if(ptemp!=NULL)
    {///去掉回车符
        *ptemp='\0';
    }
    pthread_mutex_unlock(&usr->u_mutex);

	if(usr->aenc_no!=-1)//用户已经在播放,请求失败
		goto NOT_SUPPORT;
	//if(获取其他用户是否在播放subscribe->audio_channel)
	//	goto FAIL;
	if(get_playback_stat(subscribe->audio_channel)!=-1)
	{
		printf("get_playback_stat   %d\n",get_playback_stat(subscribe->audio_channel));
	
		mutichannel_set_playback_to_live(subscribe->audio_channel);
		gtloginfo("通道[%d]正在录像回放状态，收到音频下行请求，准备结束录像回放状态\n",subscribe->audio_channel);
	}
	//ret=send_audio_channel2main(subscribe->audio_channel);

	
	ret=send_down_audio2enc(subscribe->audio_channel,0);

	if(ret==3)	
		goto BUSY;
	else if(ret==2)
		goto NOT_SUPPORT;
	else if(ret==4)
		goto SUBSCRIBE_FAIL;

	send_aplay_ack_pkt(usr->fd,RESULT_SUCCESS,"success",strlen("success")+1);
	usr->aenc_no=subscribe->audio_channel;
	return 0;
BUSY:
	send_aplay_ack_pkt(usr->fd,ERR_DVC_BUSY,"fail already play",strlen("fail already play\n")+1);
	return -1;
NOT_SUPPORT:
	send_aplay_ack_pkt(usr->fd,ERR_DVC_NO_AUDIO,"fail no audio",strlen("fail no audio\n")+1);
	printf(" NOT SUPPORT!\n");
	return -2;
SUBSCRIBE_FAIL:
	send_aplay_ack_pkt(usr->fd,ERR_DVC_INTERNAL,"fail subscribe",strlen("fail subscribe\n")+1);
	return -3;




	

}
/** 
 *   @brief     处理网络用户发来的音频下行命令
 *   @param  usr 收到命令的用户结构指针
 *   @param  cmd 从网络收到的命令结构指针
 *   @return   0表示返回后继续执行,负值表示返回后需要断开连接
 */ 
static int process_net_aplay_cmd(aplay_usr_t *usr,struct gt_usr_cmd_struct* cmd)
{
    int                    ret=0;

    if((usr==NULL)||(cmd==NULL))
        return -EINVAL;

    gettimeofday(&usr->last_cmd_time,NULL);                                 ///<接收到命令的时间
    switch(cmd->cmd)
    {
        case VIEWER_SUBSCRIBE_AUDIO:
            ret=proc_audio_subscrib_cmd(usr,cmd);
        break;
        case VIEWER_SUBSCRIBE_AUDIO_START:
            ///现在不处理了,直接扔掉(在外面)
        break;
        default:
            printf("rtimage recv unknow net snd cmd:0x%04x\n",cmd->cmd);
        break;        
    }
    
    return ret;
}




/** 
 *   @brief       接收网络发来的音频数据,存入缓冲区
 *   @param    fd 已经连接的网络描述符
 *   @buf         buf 准备存放数据的缓冲区
 *   @read_len 准备读取的数据长度
 *   @return     正值表示接收到的字节数,负值表示出错
 */
static inline int read_net_audio_data(int fd,void *buf,int read_len)
{
    return fd_read_buf(fd,buf,read_len);
}




/** 
 *   @brief     接收并处理网络用户的音频下行服务的线程
 *   @return   0表示成功,负值表示失败
 */ 
static void *rtnet_aplay_thread(void *para)
{
    static  const char        proto_head[4]={0x40,0x55,0xaa,0x55};   ///<老的协议里面要VIEWER_SUBSCRIBE_AUDIO_START命令
    static  const char        proto_start_len = 36;                             ///<VIEWER_SUBSCRIBE_AUDIO_START命令包的整个长度
    char                      *pbuf=NULL;                                            ///<播放指针
    tcprtimg_svr_t           *p=get_rtimg_para();
    aplay_server_t          *svr=&p->aplay_server;
    aplay_usr_t               *usr=(aplay_usr_t*)para;
	media_source_t 			* adec= (media_source_t*)get_audio_dec(0);
	int i;
	int *i_pbuf;
	for(i=0;i<MAX_AUDIO_DECODER;i++)

	{
		adec->attrib->stat=ENC_STAT_OK;
		adec++;
	}

    struct sockaddr_in     guest_addr;
    fd_set                        read_fds;
    int                             accept_fd=-1;
    unsigned int                             addrlen;
    int                             ret,ret2;
    int                             sel;
	int u1=0;
    struct timeval	         timeout;
    char             net_rec_buf[4096+36];		                ///<接收网络发来的数据的缓冲区
    int                             read_size=0;                                  ///<一次从网络读取的字节数
    int                             first_audio_flag=1;                        ///<第一次播放音频标志

	struct   timeval   start,stop,diff;
    struct gt_pkt_struct* recv=(struct gt_pkt_struct*)net_rec_buf;
    if(usr==NULL)
    {
        printf     ("rtnet_aplay_thread para=NULL exit thread!!\n");
        gtloginfo("rtnet_aplay_thread para=NULL exit thread!!\n");
        pthread_exit(NULL);
    }
    printf     (" start rtnet_aplay_thread user:%d...\n",usr->no);
    gtloginfo(" start rtnet_aplay_thread user:%d...\n",usr->no);
    while(1)
    {
        ///侦听连接
        FD_ZERO(&read_fds);
        addrlen=sizeof(struct sockaddr_in);

        if(pthread_mutex_lock(&svr->l_mutex)==0)
        {
        	accept_fd = accept(svr->listen_fd, (void*)&guest_addr, &addrlen);     ///<侦听新连接    		
        }
        else
            continue;
        if((accept_fd<0)&&(errno==EINTR)) 
        {
            pthread_mutex_unlock(&svr->l_mutex);
            continue;
        }
         else if(accept_fd<0) 
        { 
        	printf("rtnet_aplay_thread accept errno=%d:%s!!\n",errno,strerror(errno));
        	gtlogerr("rtnet_aplay_thread accept errno=%d:%s!!\n",errno,strerror(errno));
        	pthread_mutex_unlock(&svr->l_mutex);
         	continue; 
        } 
		 
	  ///针对无音频设备作出相应处理
        ret = get_audio_num();
		if(ret <= 0)
		{
			send_aplay_ack_pkt(accept_fd,ERR_EVC_NOT_SUPPORT,"device not support audio\n",strlen("device not support audio\n")+1);
			continue;
		}
	 
        printf     ("come a new net_aplay guest:%s \n",inet_ntoa(guest_addr.sin_addr));
        gtloginfo("come a new net_aplay guest:%s \n",inet_ntoa(guest_addr.sin_addr));
        init_aplay_usr_info(usr);
        #if 0
            TODO:check valid guest
        #endif
         pthread_mutex_lock(&svr->s_mutex);
         ret=add_aplay_usr_list(usr,&guest_addr);
         pthread_mutex_unlock(&svr->s_mutex);
         
         pthread_mutex_unlock(&svr->l_mutex);
         
         net_set_linger(accept_fd,0);
        if(ret!=0)
        {
            { ///忙
                send_aplay_ack_pkt(accept_fd,ERR_DVC_BUSY,"device busy\n",strlen("device busy\n")+1);
                sleep(1);
                close(accept_fd);
                printf     ("设备忙,无法完成%s的音频下行服务ret=%d!\n",inet_ntoa(guest_addr.sin_addr),ret);
                gtloginfo("设备忙,无法完成%s的音频下行服务ret=%d!\n",inet_ntoa(guest_addr.sin_addr),ret);
                
                continue;
            }
            
        }
		
       ///设置socket属性
        usr->fd=accept_fd; 
        usr->sock_attr.send_buf_len=net_get_tcp_sendbuf_len(usr->fd);
        usr->sock_attr.recv_buf_len=net_get_tcp_recvbuf_len(usr->fd);
        
        ret=net_activate_keepalive(usr->fd);
        ret=net_set_recv_timeout(usr->fd,3);
        ret=net_set_nodelay(usr->fd);        
        ret=net_set_linger(usr->fd,0);

        printf     ("tcprtimage 成功接受了一个音频下行服务连接(uno:%d,total:%d,max:%d):%s\n",
            usr->no,svr->usrs,p->max_aplay_usrs,inet_ntoa(usr->addr.sin_addr));
        gtloginfo ("tcprtimage 成功接受了一个音频下行服务连接(uno:%d,total:%d,max:%d):%s\n",
            usr->no,svr->usrs,p->max_aplay_usrs,inet_ntoa(usr->addr.sin_addr));


        while(1)
        {
            if((usr->fd>0)&&(accept_fd>0))
            {
                FD_SET(accept_fd,&read_fds);
            }
            else
            {
                break;//已经被断开
            }
            if(usr->serv_stat<0)
            {
                break;
            }
            timeout.tv_sec=5;
            timeout.tv_usec=0;
            sel=select(accept_fd+1,&read_fds,NULL,NULL,&timeout);
         	if(sel==0)
            {

                //continue;
    	 	 	 printf("select timeout \n");
    	 	 	 break;
            }
            else if(sel<0)
            {
                if(errno!=EINTR)
                    sleep(1);
                continue;
            }
            if(usr->fd<0)
                break;
            if(FD_ISSET(accept_fd,&read_fds))
            {
            	
                if(usr->serv_stat<=0)
                {///接收命令状态
                    ret=gt_cmd_pkt_recv(accept_fd,recv,sizeof(net_rec_buf),NULL,0);		
					if(ret>=0)
                    {		
                        pthread_mutex_lock(&usr->u_mutex);
                        usr->timeout_cnt=0;     ///<清除超时计数器
                        pthread_mutex_unlock(&usr->u_mutex);
						//这里处理后就只能接受ip对讲设备了
                        ret=process_net_aplay_cmd(usr,(struct gt_usr_cmd_struct*)( recv->msg));
                        if(ret<0)
						{
                        	    break;
							}
                         if(usr->a_fmt==AUDIO_PLAY_TYPE_UPCM)
                            read_size=   AUDIO_PLAY_PKT_SIZE/2;

						 else if(usr->a_fmt==AUDIO_PLAY_TYPE_AAC)
						    read_size = AAC_HEAD_SIZE;
							
						 else
                            read_size=AUDIO_PLAY_PKT_SIZE;

                        first_audio_flag=1;
                        ret=1;
						gettimeofday(&start,0);
                    }	
                }
                else
                {///接收数据状态	
					
                    ret=read_net_audio_data(accept_fd,(char*)net_rec_buf,read_size); 
					//printf("ret=%d\n",ret);
					if(u1>=10)
					{
						u1=0;
						gettimeofday(&stop,0);
						timeval_subtract(&diff,&start,&stop);
						printf("用时:%d秒%d 微秒\n",(int)diff.tv_sec, (int)diff.tv_usec); 
						gettimeofday(&start,0); 

					}
					else
						u1++;
					

		     	 	if(ret>0)
                    {
                        usr->sock_attr.recv_buf_remain=get_fd_in_buffer_num(accept_fd);
                        usr->recv_info.total_recv+=ret;

						if(first_audio_flag)
                   
                        {///<以前的协议要求有VIEWER_SUBSCRIBE_AUDIO_START命令
                            first_audio_flag=0;
                            if(memcmp(proto_head,net_rec_buf,sizeof(proto_head))==0)
                            {
                                printf("receive a VIEWER_SUBSCRIBE_AUDIO_START cmd!\n");
                                gtloginfo("receive a VIEWER_SUBSCRIBE_AUDIO_START cmd!\n");
								if(usr->a_fmt==AUDIO_PLAY_TYPE_UPCM) 
								{
									pbuf=(char*)net_rec_buf+ret;
									ret2=read_net_audio_data(accept_fd,pbuf,proto_start_len);///<补充start包长度的音频
									if(ret2<0)
									{
										ret=ret2;
									}
									else
									{
										pbuf=(char*)net_rec_buf+proto_start_len;
									}
								}
								else if(usr->a_fmt==AUDIO_PLAY_TYPE_AAC) //aac 再读36-8字节,后面就是数据
								{
									pbuf=(char*)net_rec_buf;
									ret2=read_net_audio_data(accept_fd,pbuf+AAC_HEAD_SIZE,proto_start_len-AAC_HEAD_SIZE);///<补充start包长度的音频,并扔掉不发送
									if(ret2<0)
									{
										ret=ret2;
									}		
									continue;
									
								
								}
                            }
                            else
                            {
								pbuf=(char*)net_rec_buf;            ///<没有收到VIEWER_SUBSCRIBE_AUDIO_START
                            }
                        }
                        else
						{
							pbuf=(char*)net_rec_buf;
							if(usr->a_fmt==AUDIO_PLAY_TYPE_AAC){
										 
								i_pbuf=(int*)net_rec_buf;
								if(*i_pbuf!=0x77061600){
									printf("AAC read head ERR! %d\n",*i_pbuf);
									gtloginfo("AAC read head ERR! %d\n",*i_pbuf);
									continue;
								}

								ret2=read_net_audio_data(accept_fd,net_rec_buf+AAC_HEAD_SIZE,AAC_HEAD_SIZE);
								if(ret2<0){
									ret=ret2;
									goto HANDLE;
								}
								i_pbuf=(int *)(net_rec_buf+AAC_HEAD_SIZE);
								printf("will read len=%d\n",*i_pbuf);

								if(*i_pbuf>=MAX_AAC_PKT_SIZE||*i_pbuf<=0){
									printf("AAC read length  ERR [%d]!\n",*i_pbuf);
									gtloginfo("AAC read frame length ERR [%d]!\n",*i_pbuf);
									continue;
								}
								ret2=read_net_audio_data(accept_fd,net_rec_buf+AAC_HEAD_SIZE*2,*i_pbuf);
								if(ret2<0){
								  ret=ret2;
								}
							}
							
		

                       }
HANDLE:
					if(ret>0)
					{


#define DEBUG
#ifdef DEBUG
						//write_rawaudio2file_send(pbuf,read_size);
						//write_rawaudio2file_send(pbuf,4096);
					
						//gettimeofday(&start,0);
#endif
#ifdef DEBUG
						//gettimeofday(&stop,0);
						//timeval_subtract(&diff,&start,&stop);
						//printf("总计用时:%d秒%d 微秒\n",(int)diff.tv_sec, (int)diff.tv_usec); 
#endif							
						//目前都向0x51000这一个buffer发送数据
						/*
						if(usr->a_fmt==AUDIO_PLAY_TYPE_UPCM) 
						{
							//ret2=write_media_resource(get_audio_dec(0),pbuf,read_size,0);
							memcpy(head_pbuf+4,pbuf,read_size);
							play_audio_buf(NULL,0,0,head_pbuf,read_size,0,0);
							
						}
						*/
						if(usr->a_fmt==AUDIO_PLAY_TYPE_AAC)
						{
							if(	write_media_resource((media_source_t *)get_audio_dec(usr->aenc_no),(void*)net_rec_buf ,ret2+AAC_HEAD_SIZE*2,0)==0)
							
								printf("write_media_resource ret=%d\n",ret2+AAC_HEAD_SIZE);
						}
						usr->recv_info.total_play+=ret2;
                        usr->recv_info.total_recv+=ret2;


					}
                   
                    else if (ret==0)
                    {
                       ret=-140;   ///<连接被主动关闭
                    }
                    
                }
                if(ret<=0)
                {
                    if(ret==-EAGAIN)
                    {///接收命令超时
                        printf     ("rtnet_aplay_thread 收到一个不完整的命令\n");
                        gtloginfo("rtnet_aplay_thread 收到一个不完整的命令\n");
                        continue;
                    }
                    else if(ret==-EINTR)
                    {
                        continue;
                    }
                    else if(ret==-ETIMEDOUT)
                    {
                        printf     ("ETIMEDOUT 错误:客户:%s的音频下行连接超时\n",inet_ntoa(usr->addr.sin_addr));
                        gtloginfo("ETIMEDOUT 错误:客户:%s的音频下行连接超时\n",inet_ntoa(usr->addr.sin_addr));
                        break;
                    }
                    else
                    {
                        if(ret==-140)
                        {
                            printf     ("音频下行连接:%d(%s)被关闭,ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
                            gtloginfo("音频下行连接:%d(%s)被关闭,ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
                        }
                        else
                        {
                            printf     ("音频下行连接:%d(%s)断开,ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
                            gtloginfo("音频下行连接:%d(%s)断开,ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
							
                        }
                        break;
                    }
				

                }
				}
			}
            else
            {
                printf     ("rtnet_aplay_thread FD_ISSET(accept_fd,&read_fds)!=1!!!\n");
                gtloginfo("rtnet_aplay_thread FD_ISSET(accept_fd,&read_fds)!=1!!!\n");
                break;               
            }
			
        }
		//断开连接

		stop_down_audio2enc(usr->aenc_no);
        pthread_mutex_lock(&svr->s_mutex);
		



        del_aplay_usr_list(usr);   //删除用户
        pthread_mutex_unlock(&svr->s_mutex);
		FD_CLR(accept_fd,&read_fds);
        close(accept_fd);          //关闭网络连接
        accept_fd=-1;    
    }
    return NULL;
}

/** 
 *   @brief     创建侦听网络音频下行服务的socket以及线程池
 *   @return   0表示成功,负值表示失败
 */ 
int create_rtnet_aplay_servers(void)
{
    tcprtimg_svr_t *p=get_rtimg_para();
    aplay_server_t     *svr=&p->aplay_server;
    aplay_usr_t          *usr=NULL;
    int                   fd;
    int                   i;

    init_net_aplay();
    fd = create_tcp_listen_port(INADDR_ANY,p->audio_play_port); ///<创建网络音视频服务socket
    if(fd<0)
    {
        printf("can't create socket port:%d:%s exit !\n",p->av_svr_port,strerror(-fd));
        printf("can't create socket port:%d:%s exit !\n",p->av_svr_port,strerror(-fd));
        exit(1);
    }
    svr->listen_fd=fd;
    net_activate_keepalive(svr->listen_fd);                             ///<探测对方断开
    listen(svr->listen_fd,p->max_aplay_usrs);             ///<设置侦听连接数
    for(i=0;i<(p->max_aplay_usrs+1);i++)
    {
        usr=&svr->usr_list[i];
        gt_create_thread(&usr->thread_id, rtnet_aplay_thread, (void*)usr);
    }
    return 0;
}

