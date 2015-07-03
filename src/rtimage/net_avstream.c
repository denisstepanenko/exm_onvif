/** @file	       net_avstream.c
 *   @brief 	接收并处理客户端发来的音视频上行命令
 *   @date 	2007.03
 */
#include "rtimage2.h"

#include <commonlib.h>
#include <gtthread.h>
#include <netinfo.h>    
#include <gt_com_api.h>
#include <venc_read.h>
#include "rtnet_cmd.h"
#include <gt_errlist.h>
#include <gtsocket.h>
#include <errno.h>
#include "play_back.h"
#include "gtsf.h"

/** 
 *   @brief     初始化网络音视频上行服务的全部用户变量
 *   @return   0表示成功,负值表示失败
 */ 
int init_net_avstream(void)
{
    tcprtimg_svr_t *p=get_rtimg_para();
    av_server_t     *svr=&p->av_server;
    av_usr_t          *usr=NULL;
    int                   i;
    p->mic_gain           = def_mic_gain;                     ///<mic增益
    p->audio_gain        = def_audio_gain;	           ///<音频输出增益
	p->virdev_num = virdev_get_virdev_number();
	p->max_wan_usrs = p->virdev_num;
	p->max_lan_usrs    = TCPRTIMG_MAX_AVUSR_NO*p->virdev_num - p->max_wan_usrs;	    ///<最大局域网用户数
    p->max_aplay_usrs =TCPRTIMG_MAX_APLAY_NO; ///<最大音频下行服务用户数
    p->av_svr_port       = def_rtstream_port;	        ///<音视频上行服务端口
    p->audio_play_port = def_rtsnd_port;               ///<音频下行服务端口
    p->th_timeout         = def_svr_timeout;            ///<长时间没有数据交互时判断超时的时间
    p->audio_pkt_size   = def_audio_pkt_size;       ///<默认音频上行数据包大小
    p->th_drop_p          = def_th_drop_p;		        ///<缓冲池数据多后开始丢视频p帧的阈值
    p->th_drop_v          =  p->th_drop_p + 10;          ///<缓冲池数据多后开始丢弃所有视频的阈值
    p->th_drop_a          = MAX_MAP_BUF_FRAMES -p->th_drop_v - 5;    ///<开始丢弃所有数据的阈值
    p->eth0_addr          = get_net_dev_ip("eth0");     
    p->eth0_mask         = get_net_dev_mask("eth0");
    p->eth1_addr          = get_net_dev_ip("eth1");     
    p->eth1_mask         = get_net_dev_mask("eth1");
    pthread_mutex_init(&svr->l_mutex,NULL);
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&svr->s_mutex,&attr);         
    
    svr->listen_fd=-1;
    
    ///用户信息初始化
  	for(i=0;i<(TCPRTIMG_MAX_AVUSR_NO*p->virdev_num + 1);i++)
// #else
	//for(i=0;i<(TCPRTIMG_MAX_AVUSR_NO+1);i++)
      {
        usr=&svr->av_usr_list[i];
        memset((void*)usr,0,sizeof(av_usr_t));
        pthread_mutex_init(&usr->u_mutex,NULL);
        usr->fd=-1;
        usr->no=i;
        usr->pid=-1;
        usr->thread_id=-1;
        usr->magic=0x55aa;
        
    }


    return 0;
}

/** 
 *   @brief     判断一个地址是否为同一个局域网中的
 *   @param  需要判断的ip地址
 *   @return   1表示是同一个局域网的,0表示不是
 */ 
int is_lan_user(in_addr_t addr)
{
    tcprtimg_svr_t *p=get_rtimg_para();
    in_addr_t          subnet;
    subnet=p->eth0_addr&p->eth0_mask;   ///<计算子网
    if(subnet==(p->eth0_mask&addr))
        return 1;                                         ///<是同一个子网的
    else
    {
        if(get_eth_num()>1)
        {
            subnet=p->eth1_addr&p->eth1_mask;
            if(subnet==(p->eth1_mask&addr))
                return 1;                                         ///<是同一个子网的            
        }
        return 0;                                                ///<不是同一个子网的
    }
}
int is_usr_valid(av_usr_t *usr)
{
	
	if(usr->used>0)
		return 1;
	else
		return 0;



}
av_usr_t * get_free_usr(int * usr_index)
{
	int i;
	tcprtimg_svr_t *p=get_rtimg_para();
	int total=p->max_lan_usrs+p->max_wan_usrs+1;
	av_usr_t *usr=NULL;
	
    av_server_t          *svr=&p->av_server;
    for(i=0;i<total;i++)  //多一个用于处理忙
    {
        usr=&svr->av_usr_list[i];
		if(usr->used==0)
		{
			*usr_index=i;
			break;
		}
    }
	if(i==total)
	{
		usr=NULL;
	}
	return usr;

}

av_usr_t * find_usr(int index)
{
	tcprtimg_svr_t *p=get_rtimg_para();
	int total=p->max_lan_usrs+p->max_wan_usrs+1;
	av_usr_t *usr=NULL;
	if(index>=total)
	{
		printf("WRONG USR INDEX\n");
		return NULL;
	}
	av_server_t 		 *svr=&p->av_server;
	usr=&svr->av_usr_list[index];
	return usr;


}
/** 
 *   @brief     初始化音视频用户数据结构(用于有新连接到来时)
 *   @param  usr 用户信息结构指针
 *   @return   无
 */     
static void init_av_usr_info(av_usr_t *usr)
{
    tcprtimg_svr_t *p=get_rtimg_para();
	pthread_mutex_init(&usr->u_mutex,NULL);
    pthread_mutex_lock(&usr->u_mutex);
    
    memset(&usr->addr,0,sizeof(struct sockaddr_in));
    usr->fd=-1;
    usr->th_timeout=p->th_timeout;
    usr->timeout_cnt=0;
    usr->th_drop_p=p->th_drop_p;
    usr->th_drop_v=p->th_drop_v;
    usr->th_drop_a=p->th_drop_a;
    gettimeofday(&usr->start_time,NULL);
    memset(&usr->last_cmd_time,0,sizeof(struct timeval));
    sprintf(usr->name,"%s","newuser");
    usr->serv_stat=0;
    usr->venc_no=-1;
    usr->aenc_no=-1;
	//zsk add
	usr->stream_idx=-1;
	usr->used=0;
    memset(&usr->sock_attr,0,sizeof(socket_attrib_t));
    memset(&usr->send_info,0,sizeof(stream_send_info_t));
    usr->send_info.send_ack_flag=0;
    pthread_mutex_unlock(&usr->u_mutex);
}

/** 
 *   @brief     将一个用户地址信息加入用户列表
 *   @param  usr 存放用户信息的结构指针
 *   @param  guest 新用户的ip地址和端口号信息
 *   @return   0表示成功 1表示忙 负值表示出错
 *   @warnning 调用此函数前调用者应该已经取得了媒体服务的s_mutex
 */ 
static int add_av_usr_list(av_usr_t *usr,struct sockaddr_in *guest)
{

    tcprtimg_svr_t *p=get_rtimg_para();
    av_server_t     *svr=&p->av_server;
    if((usr==NULL)||(guest==NULL))
        return -EINVAL;       

    if(is_lan_user(guest->sin_addr.s_addr))
    {
        svr->lan_usrs++;                                                         ///<局域网用户数加1
        if(svr->lan_usrs>p->max_lan_usrs)
        {
            svr->lan_usrs--;
            return 1;
        }
    }
    else
    {
        svr->wan_usrs++;                                                        ///<广域网用户数加1
        if(svr->wan_usrs>p->max_wan_usrs)
        {
            svr->wan_usrs--;
            return 1;
        }
    }
    memcpy(&usr->addr,guest,sizeof(struct sockaddr_in));    ///<复制用户信息
    return 0;
    
}
/** 
 *   @brief     从音视频上行服务中删除指定的用户
 *   @param  usr 存放用户信息的结构指针
 *   @return   0表示成功  负值表示出错
 *   @warnning 调用此函数前调用者应该已经取得了媒体服务的s_mutex
 */ 
static int del_av_usr_list(av_usr_t *usr)
{
    tcprtimg_svr_t *p=get_rtimg_para();
    av_server_t     *svr=&p->av_server;
    if(usr==NULL)
        return -EINVAL;
    if(is_lan_user(usr->addr.sin_addr.s_addr))
    {
        svr->lan_usrs--;
        if(svr->lan_usrs<0)
        {
            showbug();
            logbug();
        }
    }
    else
    {
        svr->wan_usrs--;
        if(svr->wan_usrs<0)
        {
            showbug();
            logbug();
        }
    }
    ///删除用户信息
    pthread_mutex_lock(&usr->u_mutex);
    usr->serv_stat=0;
	usr->fd=-1;
    usr->venc_no=-1;
    usr->aenc_no=-1;
	usr->stream_idx=-1;
	usr->used=0;
	memset(&usr->addr,0,sizeof(struct sockaddr_in));
    pthread_mutex_unlock(&usr->u_mutex);
    return 0;
}
static int netsend_streamid(int socket, int streamid)
{

    int  result;
    char *pdata;
    int  getbufflen = 4;
    gtsf_stream_fmt  *pStream;
    char  buf[200];


    pStream = (gtsf_stream_fmt *)buf;
        /*数据打包*/
    pStream->mark = GTSF_MARK;
    pStream->encrypt_type = 0;
    pStream->len = getbufflen;
    pStream->type = MEDIA_STREAMID;

    pdata = buf + GTSF_HEAD_SIZE;
    memcpy(pdata,&streamid,4);
    result = net_write_buf(socket ,(char*)pStream,getbufflen+GTSF_HEAD_SIZE);
    if(result != GTSF_HEAD_SIZE+4)
    {
        printf("tcp send errot %d,   result %d\n",getbufflen,result);
        return ERR_RT_SEND_ERR; 
    }
    else
    {
        
        return RESULT_SUCCESS;
    }        
}

/** 
 *   @brief     返回音视频上行服务的响应信息
 *   @param  fd 目的描述符
 *   @param  result 返回的错误代码
 *   @param  需要挂在返回信息answer_data后面的信息
 *   @param  datalen buf中有效数据的个数
 *   @return   负值表示出错,非负表示成功
 */ 
int send_rtstream_ack_pkt(int fd,WORD result,char* buf,int datalen)
{
	int ret;
	DWORD	                            sendbuf[1024];                                              ///<存放要发送数据的缓冲区
	struct gt_pkt_struct            *send=(struct gt_pkt_struct *)sendbuf;           ///<要发送的数据的结构指针
	struct gt_usr_cmd_struct    *cmd=(struct gt_usr_cmd_struct*)send->msg;///<命令结构指针
	struct viewer_subscribe_cmd_answer_struct *answer=(struct viewer_subscribe_cmd_answer_struct  *)cmd->para;///<响应指针
	int answerlen;
	
	if(fd<0)
		return -EINVAL;
	cmd->cmd=VIEWER_SUBSCRIBE_VIDEO_ANSWER;
	cmd->en_ack=0;
	cmd->reserve0=0;
	cmd->reserve1=0;
	answer=(struct viewer_subscribe_cmd_answer_struct  *)cmd->para;
	ret=get_devid(answer->dev_id);
	answer->result=result;
       ret=1;           //除了0写几都行
	memcpy((BYTE*)&answer->video_trans_id,(BYTE*)&ret,4);
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
	cmd->len=sizeof(struct viewer_subscribe_cmd_answer_struct)-sizeof(answer->answer_data)+answerlen+6;	
	ret=gt_cmd_send_noencrypt(fd,send,cmd->len+2,NULL,0);
	return ret;
}

/**
  *   @brief	判断请求音视频的guid是否合法
  *   @param	cmd_dev_id申请音视频命令里的guid
  *	@param	cmd_addr请求音视频的设备ip的地址
  *	@return	成功返回虚拟设备号，失败返回负值
*/
int is_guid_valid(unsigned char *cmd_dev_id, struct sockaddr_in *cmd_addr)
{
	int virdev_num;
	unsigned char virdev_guid[10][20];
	int guid_len;
	int i;


	virdev_num = virdev_get_virdev_number();		
	for(i=0; i<virdev_num; i++)
	{
		guid_len = virdev_get_devid(i, (unsigned char *)virdev_guid[i]);
		///guid匹配的话，返回此设备号
		if(memcmp(virdev_guid[i], cmd_dev_id, guid_len) == 0)
		{
			return i;
		}
		else
		{
			///如果最后一个设备仍不匹配，则说明guid不合法，打印退出
			if(i == (virdev_num-1))
			{
				printf("收到%s的订阅命令,GUID不匹配->\n",inet_ntoa(cmd_addr->sin_addr));
				gtloginfo("收到%s的订阅命令,GUID不匹配->\n",inet_ntoa(cmd_addr->sin_addr));
				for(i=0; i<virdev_num; i++)
				{
					printf("dev_guid=%02x%02x%02x%02x%02x%02x%02x%02x\n",virdev_guid[i][7],virdev_guid[i][6],virdev_guid[i][5],virdev_guid[i][4],virdev_guid[i][3],virdev_guid[i][2],virdev_guid[i][1],virdev_guid[i][0]);
					gtloginfo("dev_guid=%02x%02x%02x%02x%02x%02x%02x%02x\n",virdev_guid[i][7],virdev_guid[i][6],virdev_guid[i][5],virdev_guid[i][4],virdev_guid[i][3],virdev_guid[i][2],virdev_guid[i][1],virdev_guid[i][0]);
				}
      		  		
		      		printf("reg_guid=%02x%02x%02x%02x%02x%02x%02x%02x\n",*(cmd_dev_id+7),*(cmd_dev_id+6),*(cmd_dev_id+5),*(cmd_dev_id+4),*(cmd_dev_id+3),*(cmd_dev_id+2),*(cmd_dev_id+1),*(cmd_dev_id+0));
				gtloginfo("reg_guid=%02x%02x%02x%02x%02x%02x%02x%02x\n",*(cmd_dev_id+7),*(cmd_dev_id+6),*(cmd_dev_id+5),*(cmd_dev_id+4),*(cmd_dev_id+3),*(cmd_dev_id+2),*(cmd_dev_id+1),*(cmd_dev_id+0));
				return -2;
			}
		}
	}
	
	return 0;
}


/** 
 *   @brief     处理网络用户发来的音视频上传订阅命令
 *   @param  usr 收到命令的用户结构指针
 *   @param  cmd 从网络收到的命令结构指针
 *   @param  virdev_ret新增用户号	///<for 3022
 *   @return   0表示返回后继续执行,负值表示返回后需要断开连接
 */ 
static int proc_video_subscrib_cmd(IO av_usr_t *usr,IN struct gt_usr_cmd_struct* cmd, OUT int *virdev_ret )
{
    char                 *ptemp=NULL;
    int                    ret;
//    int	             guid_len;
//    unsigned char   dev_guid[20];

    int			virdev_num=virdev_get_virdev_number();		///<虚拟设备个数
    tcprtimg_svr_t *p=get_rtimg_para();
	playback_t     *pb=get_playback_parm();
    int		      ret_addusr=0;


    struct viewer_subscribe_cmd_struct *subscribe=(struct viewer_subscribe_cmd_struct*)cmd->para;
    if((cmd->cmd!=VIEWER_SUBSCRIBE_D1_VIDEO)&&(cmd->cmd!=VIEWER_SUBSCRIBE_VIDEO))
    {
        return -EINVAL;
    }

	//ret=is_guid_valid(subscribe->dev_id, &usr->addr);
	ret=0;
	switch(ret)
	{
		case 0:
			///for 3021、3024
			if(virdev_num==1)
			{

				if(subscribe->channel<0||subscribe->channel>=MAX_VIDEO_ENCODER)
					usr->venc_no=0;
				else
					usr->venc_no=subscribe->channel;
				printf("\n\nsubscribe->channel=%ld\n",subscribe->channel);
					
			}
			break;


		default:
			///guid不匹配
			send_rtstream_ack_pkt(usr->fd,ERR_DVC_INVALID_REQ,"invalid req guid\n",strlen("invalid req guid\n")+1);
        		usleep(500000);
        		return -2;
	}

	///判断虚拟设备的用户数是否达到上限
	if(virdev_num==2)
	{
		if(ret_addusr!=0)
  	 	{
       		if(ret_addusr<0)
        	{///出错
            		send_rtstream_ack_pkt(usr->fd,ERR_DVC_INTERNAL,"device internal  error\n",strlen("device internal  error\n")+1);
            		sleep(1);
            		close(usr->fd);
            		gtloginfo("内部错误,无法完成图像服务!\n");
	  				printf("内部错误,无法完成图像服务!\n");
            		return -2;
       		 }
        	else if(ret_addusr==1)
        	{ ///virdev-0 忙
        		send_rtstream_ack_pkt(usr->fd,ERR_DVC_BUSY,"device busy\n",strlen("device busy\n")+1);
        		sleep(1);
        		close(usr->fd);
		  		printf("设备忙,无法完成%s的图像连接服务!\n",inet_ntoa(usr->addr.sin_addr));
				gtloginfo("virdev-0设备忙,无法完成%s的图像连接服务!\n",inet_ntoa(usr->addr.sin_addr));
			 	return -2;
        	}
     		else if(ret_addusr==2)
        	{ ///virdev-1 忙
				send_rtstream_ack_pkt(usr->fd,ERR_DVC_BUSY,"device busy\n",strlen("device busy\n")+1);
				sleep(1);
				close(usr->fd);
				printf("设备忙,无法完成%s的图像连接服务!\n",inet_ntoa(usr->addr.sin_addr));
				gtloginfo("virdev-1设备忙,无法完成%s的图像连接服务!\n",inet_ntoa(usr->addr.sin_addr));
				return -2;
        	}
        
    	}
	}

	printf     ("收到 %s 的订阅媒体流命令(venc=%d)account=%s audioflag=0x%x \n",inet_ntoa(usr->addr.sin_addr),usr->venc_no,subscribe->account,(int)subscribe->audio_flag);
	gtloginfo("收到 %s 的订阅媒体流命令(venc=%d)account=%s audioflag=0x%x \n",inet_ntoa(usr->addr.sin_addr),usr->venc_no,subscribe->account,(int)subscribe->audio_flag);


    if((usr->serv_stat!=1)&&(usr->serv_stat!=3))
    {///之前还没有开始服务
        ret=get_venc_stat(usr->venc_no);
      	if(ret!=ENC_STAT_OK)
        {
            printf("venc%d state err:%d\n",usr->venc_no,ret);
            gtlogerr("venc%d state err:%d\n",usr->venc_no,ret);
            send_rtstream_ack_pkt(usr->fd,ERR_DVC_FAILURE,"device video encoder  error\n",strlen("device video encoder  error\n")+1);
            return -1;
        }
        
        usr->send_info.send_ack_flag=0;
    }
	//加入同一通道连接的判断


    pthread_mutex_lock(&usr->u_mutex);
	
    ret=get_audio_num();
    if((subscribe->audio_flag==0x55aa)||(ret<=0))
    {
        usr->aenc_no=-1;                                     ///<不需要声音
        usr->serv_stat=1;                           
    }
    else
    {
	///for 3022 audio 

		if(virdev_num==2)
		{
			if(usr->aenc_no==0)				///<第一路虚拟3021
			{
				usr->serv_stat=3;
			}
			else if(usr->aenc_no==1)			///<第二路虚拟3021
			{
				usr->serv_stat=1;
				printf("Donnot support audio\n");
			}
		}
		///for 3021/3024
		else
		{
			//zsk mod 下行音频不改变上行音频通道号
			usr->aenc_no=subscribe->channel;
		   	usr->serv_stat=3;
	//		//发给encbox上行音频的请求,是否采用外部音频设备由encbox判断
			//require_up_audio2enc(subscribe->channel);
			if(usr->aenc_no==4){	
				printf("require up audio no %d\n",usr->aenc_no);
		
			}else{
				require_up_audio2enc(usr->aenc_no);
				printf("require up audio no %d\n",usr->aenc_no);
	
			}
			
		}
    }
    strncpy(usr->name,(char *)subscribe->account,20);   ///<用户名最长为19
    usr->name[19]='\0';
    ptemp=strchr(usr->name,'\n');
    if(ptemp!=NULL)
    {///去掉回车符
        *ptemp='\0';
    }
    pthread_mutex_unlock(&usr->u_mutex);
        
    return 0;
}
/** 
 *   @brief     处理网络用户发来的音视频命令
 *   @param  usr 收到命令的用户结构指针
 *   @param  cmd 从网络收到的命令结构指针
 *   @param  virdev_ret新增用户号	///<for 3022
 *   @return   0表示返回后继续执行,负值表示返回后需要断开连接
 */ 
static int process_net_av_cmd(av_usr_t *usr,struct gt_usr_cmd_struct* cmd, OUT int *virdev_ret)
{
    int                    ret;

    if((usr==NULL)||(cmd==NULL))
        return -EINVAL;

    gettimeofday(&usr->last_cmd_time,NULL);                                 ///<接收到命令的时间
    switch(cmd->cmd)
    {
        case VIEWER_SUBSCRIBE_D1_VIDEO:
        case VIEWER_SUBSCRIBE_VIDEO:
            ret=proc_video_subscrib_cmd(usr,cmd,virdev_ret); 
        break;
        case VIEWER_UNSUBSCRIBE_VIDEO:
                printf("usr:%d receive VIEWER_UNSUBSCRIBE_VIDEO cmd from %s\n",usr->no,inet_ntoa(usr->addr.sin_addr));	
                gtloginfo("usr:%d 收到 %s 的退订媒体流命令 \n",usr->no,inet_ntoa(usr->addr.sin_addr));
                usr->serv_stat=-2;
        break;
        default:
            printf("rtimage recv unknow net cmd:0x%04x\n",cmd->cmd);
            ret=0;
        break;
    }
    
    return ret;
}
int rtnet_av_connect2gate(IN struct usr_req_rt_img_struct* require_parm,OUT int * usr_index)
{
	int ret;
	int fd;
	tcprtimg_svr_t *p=get_rtimg_para();
	av_usr_t * usr=NULL;
	av_server_t          *svr=&p->av_server;

	
	struct sockaddr_in remote_addr;
	bzero(&remote_addr,sizeof(remote_addr));
	remote_addr.sin_family=AF_INET;
	remote_addr.sin_port=htons(require_parm->remoteport);
	remote_addr.sin_addr.s_addr=(in_addr_t)require_parm->remoteip;
	usr=get_free_usr(usr_index);
	if(usr==NULL)
	{
		printf("USR FULL!!!!\n");
		return ERR_DVC_BUSY;
	}
	init_av_usr_info(usr);
	pthread_mutex_lock(&usr->u_mutex); 
	usr->used=1;
	usr->send_info.first_flag=1;
	usr->send_info.require_i_flag=1;
	pthread_mutex_unlock(&usr->u_mutex);
	
	
	pthread_mutex_lock(&svr->s_mutex); 
	ret=add_av_usr_list(usr,&remote_addr);
	pthread_mutex_unlock(&svr->s_mutex);
	if(ret!=0)
	{
	    if(ret<0)
	    {///出错  
	        sleep(1);
	        gtloginfo("内部错误,无法完成图像服务!\n");
	        return ERR_DVC_INTERNAL;
	    }
	    else
	    { ///忙
	        sleep(1);
	        gtloginfo("设备忙,无法完成图像连接服务!\n");
	        return ERR_DVC_BUSY;
	    }
	    
	}


   	printf("connect %s port:%ld ....\n",(char*)inet_ntoa(remote_addr.sin_addr),require_parm->remoteport);
   /*建立连接*/
    fd = tcp_connect_block((char *)inet_ntoa(remote_addr.sin_addr),require_parm->remoteport,10);

    if((int)fd < 0)
    {
    	printf("tcp connect error\n");
        return ERR_RT_NO_CONN;
       
    }   
	net_set_linger(fd,0);
	pthread_mutex_lock(&usr->u_mutex); 

	usr->fd=fd; 
	usr->sock_attr.send_buf_len=200*1024;///>初始值为128k,发送数据过程中发现不足还可以增加
	ret=net_set_tcp_sendbuf_len(usr->fd,usr->sock_attr.send_buf_len);
	if(ret<0)
	{
		gtlogerr("net_set_tcp_sendbuf_len ret=%d!\n",ret);
		printf    ("net_set_tcp_sendbuf_len ret=%d!\n",ret);
	}

	usr->sock_attr.send_buf_len=net_get_tcp_sendbuf_len(usr->fd);
	pthread_mutex_unlock(&usr->u_mutex); 
	ret=net_activate_keepalive(usr->fd);
	ret=net_set_recv_timeout(usr->fd,3);
	ret=net_set_nodelay(usr->fd);        
	ret=net_set_linger(usr->fd,0);

	printf("tcprtimage 成功接受了一个音视频上行服务连接(u:%d,w:%d,l:%d):%s\n",
	usr->no,svr->wan_usrs,svr->lan_usrs,inet_ntoa(usr->addr.sin_addr));
	gtloginfo("tcprtimage 成功接受了一个音视频上行服务连接(u:%d,w:%d,l:%d):%s\n",
	usr->no,svr->wan_usrs,svr->lan_usrs,inet_ntoa(usr->addr.sin_addr));
    printf("send  streamid,socket:%d streamid:%ld ....\n",fd,require_parm->stream_idx);
    ret = netsend_streamid(fd,require_parm->stream_idx);
   	if(ret != 0)
    {
        printf("send stream id error,%x\n",ret);
        pthread_mutex_lock(&svr->s_mutex); 
		ret=del_av_usr_list(usr);
		pthread_mutex_unlock(&svr->s_mutex);
        return ret;
    }
	pthread_mutex_lock(&usr->u_mutex);
	usr->venc_no=require_parm->channel;
	usr->stream_idx=require_parm->stream_idx;
	//zsk add 修改send_ack_flag 确保不发送avihead
	
	printf("\n\require_parm->channel=%d\n",require_parm->channel);

	if((usr->serv_stat!=1)&&(usr->serv_stat!=3))
	{///之前还没有开始服务
		ret=get_venc_stat(usr->venc_no);
		if(ret!=ENC_STAT_OK)
		{
			printf("venc%d state err:%d\n",usr->venc_no,ret);
			gtlogerr("venc%d state err:%d\n",usr->venc_no,ret);
			return ERR_RT_ENC_NOT_OK;
		}
		
		//usr->send_info.send_ack_flag=0;
	}
	
	//pthread_mutex_lock(&usr->u_mutex);
	
	ret=get_audio_num();
	if((require_parm->audio_flag==0x55aa)||(ret<=0))
	{
		usr->aenc_no=-1;									 ///<不需要声音
		usr->serv_stat=1;							
	}
	else
	{

		usr->aenc_no=require_parm->channel;
		usr->serv_stat=3;
	}
	usr->send_info.send_ack_flag=1;
	
	strncpy(usr->name,"netsdk_usr",20);	///<用户名最长为19
	usr->name[19]='\0';	
	pthread_mutex_unlock(&usr->u_mutex);
    return 0;
}
void rtnet_av_close_connect(int index)
{
	tcprtimg_svr_t *p=get_rtimg_para();
	av_server_t          *svr=&p->av_server;
	av_usr_t * usr=find_usr(index);
	int fd=usr->fd;
	if(usr==NULL)
	{
		printf("DO NOT FIND USR\n");
		gtloginfo("DO NOT FIND USR\n");
		
	}
	
	pthread_mutex_lock(&svr->s_mutex);
	del_av_usr_list(usr);
	pthread_mutex_unlock(&svr->s_mutex);
	close(fd);

}


/** 
 *   @brief     接收并处理网络用户的音视频服务的线程
 *   @return   0表示成功,负值表示失败
 */ 
static void *rtnet_av_cmd_thread(void *para)
{
    tcprtimg_svr_t      *p=get_rtimg_para();
    av_server_t          *svr=&p->av_server;
    av_usr_t               *usr=(av_usr_t*)para;
    struct sockaddr_in guest_addr;
    fd_set                   read_fds;
    int                        accept_fd=-1;
    unsigned int                        addrlen;
    int                        ret;
    int                        sel;
    struct timeval	    timeout;
    unsigned long        net_rec_buf[512];		///<接收网络发来的数据的缓冲区
     int				virdev_no=-1;			///<虚拟设备号
    struct gt_pkt_struct* recv=(struct gt_pkt_struct*)net_rec_buf;
	
    if(usr==NULL)
    {
        printf     ("rtnet_av_cmd_thread para=NULL exit thread!!\n");
        gtloginfo("rtnet_av_cmd_thread para=NULL exit thread!!\n");
        pthread_exit(NULL);
    }
	
    printf     (" start rtnet_av_cmd_thread user:%d...\n",usr->no);
    gtloginfo(" start rtnet_av_cmd_thread user:%d...\n",usr->no);
	//init_av_usr_info(usr);
	pthread_mutex_init(&usr->u_mutex,NULL);
    while(1)
    {
    	FD_ZERO(&read_fds);
        ///侦听连接
        addrlen=sizeof(struct sockaddr_in);

		do
		{
			//printf("waitting for usr free usrno[%d]\n",usr->no);
			sleep(1);


		}while(is_usr_valid(usr));
        if(pthread_mutex_trylock(&svr->l_mutex)==0)
        {
        	printf("lockin usr[%d]\n",usr->no);
			pthread_mutex_lock(&usr->u_mutex);
       		usr->used=1;
       		pthread_mutex_unlock(&usr->u_mutex);
        	accept_fd = accept(svr->listen_fd, (void*)&guest_addr, &addrlen);     ///<侦听新连接    		
        }
        else
        {
        	//printf("conntinue\n");
        	//pthread_mutex_unlock(&svr->l_mutex);
            continue;
        }
        if((accept_fd<0)&&(errno==EINTR)) 
        {
        	printf("accept_fd<0\n");
            pthread_mutex_unlock(&svr->l_mutex);
            continue;
        }
         else if(accept_fd<0) 
        { 
        	printf("rtnet_av_cmd_thread accept errno=%d:%s!!\n",errno,strerror(errno));
        	gtlogerr("rtnet_av_cmd_thread accept errno=%d:%s!!\n",errno,strerror(errno));
        	pthread_mutex_unlock(&svr->l_mutex);
         	continue; 
        } 

        printf     ("come a new net_av guest:%s \n",inet_ntoa(guest_addr.sin_addr));
        gtloginfo("come a new net_av guest:%s \n",inet_ntoa(guest_addr.sin_addr));
		init_av_usr_info(usr);
		pthread_mutex_lock(&svr->s_mutex);

		ret=add_av_usr_list(usr,&guest_addr);
		pthread_mutex_unlock(&svr->s_mutex);


		pthread_mutex_unlock(&svr->l_mutex);
		net_set_linger(accept_fd,0);
		 

        if(ret!=0)
        {
            if(ret<0)
            {///出错
                send_rtstream_ack_pkt(accept_fd,ERR_DVC_INTERNAL,"device internal  error\n",strlen("device internal  error\n")+1);
                sleep(1);
                close(accept_fd);
                gtloginfo("内部错误,无法完成图像服务!\n");
                continue;
            }
            else
            { ///忙
                send_rtstream_ack_pkt(accept_fd,ERR_DVC_BUSY,"device busy\n",strlen("device busy\n")+1);
                sleep(1);
                close(accept_fd);
                gtloginfo("设备忙,无法完成%s的图像连接服务!\n",inet_ntoa(guest_addr.sin_addr));
                continue;
            }
            
        }

        ///set socket
        usr->fd=accept_fd; 
        usr->sock_attr.send_buf_len=200*1024;///>初始值为128k,发送数据过程中发现不足还可以增加
        ret=net_set_tcp_sendbuf_len(usr->fd,usr->sock_attr.send_buf_len);
        if(ret<0)
        {
            gtlogerr("net_set_tcp_sendbuf_len ret=%d!\n",ret);
            printf    ("net_set_tcp_sendbuf_len ret=%d!\n",ret);
            
            showbug();
            logbug();
            //TODO 错误处理
        }

        usr->sock_attr.send_buf_len=net_get_tcp_sendbuf_len(usr->fd);
        //usr->sock_attr.send_buf_len*=0.8;       ///>有20%的socket缓冲区不能正常使用,目前还没有发现原因
                                                                  ///>在非阻塞方式下会阻塞socket,等
       // usr->sock_attr.recv_buf_len=net_get_tcp_recvbuf_len(usr->fd);

		//	 gtloginfo("此时socket的send_buff =%d,usr=%d\n",usr->sock_attr.send_buf_len,usr->no);

        ret=net_activate_keepalive(usr->fd);
        ret=net_set_recv_timeout(usr->fd,3);
        ret=net_set_nodelay(usr->fd);        
        ret=net_set_linger(usr->fd,0);

        	printf("tcprtimage 成功接受了一个音视频上行服务连接(u:%d,w:%d,l:%d):%s\n",
            usr->no,svr->wan_usrs,svr->lan_usrs,inet_ntoa(usr->addr.sin_addr));
        	gtloginfo("tcprtimage 成功接受了一个音视频上行服务连接(u:%d,w:%d,l:%d):%s\n",
            usr->no,svr->wan_usrs,svr->lan_usrs,inet_ntoa(usr->addr.sin_addr));
        
        while(1)
        {
            if((usr->fd>0)&&(accept_fd>0))
            {
                FD_SET(accept_fd,&read_fds);
            }
            else
            {
            	//printf("here1\n");
                break;//已经被断开
            }
            if(usr->serv_stat<0)
            {
            	//printf("here2\n");
                break;
            }
            timeout.tv_sec=1;
            timeout.tv_usec=0;
            sel=select(accept_fd+1,&read_fds,NULL,NULL,&timeout);
            if(sel==0)
            {
            	printf("select accept_fd 没有变化\n");
                continue;
            }
            else if(sel<0)
            {
            	//printf("here4\n");
                if(errno!=EINTR)
                    sleep(1);
                continue;
            }
            if(usr->fd<0)
            {
            	//printf("here5\n");
                break;
            }
				errno=0;///aahtest
				
            if(FD_ISSET(accept_fd,&read_fds))
            {
            	
                net_set_noblock(accept_fd,0);
                ret=gt_cmd_pkt_recv(accept_fd,recv,sizeof(net_rec_buf),NULL,0);		
                net_set_noblock(accept_fd,1);
                if(ret>=0)
                {		
                    pthread_mutex_lock(&usr->u_mutex);
                    usr->timeout_cnt=0;     ///<清除超时计数器
                    pthread_mutex_unlock(&usr->u_mutex);
                    ret=process_net_av_cmd(usr,(struct gt_usr_cmd_struct*)( recv->msg), &virdev_no);
                    if(ret<0)
                    	    break;
                }	
                else
                {
                    if(ret==-EAGAIN)
                    {///接收命令超时
                        printf     ("process_net_av_cmd 收到一个不完整的命令\n");
                        gtloginfo("process_net_av_cmd 收到一个不完整的命令\n");
                        continue;
                    }
                    else if(ret==-EINTR)
                    {
						continue;
                    }
                    else if(ret==-ETIMEDOUT)
                    {
                        printf     ("ETIMEDOUT 错误:客户:%s的图像连接超时\n",inet_ntoa(usr->addr.sin_addr));
                        gtloginfo("ETIMEDOUT 错误:客户:%s的图像连接超时\n",inet_ntoa(usr->addr.sin_addr));						
                        break;
                    }
                    else
                    {
                        if(ret==-140)
                        {
                            printf     ("图像连接:%d(%s)被关闭,ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
                            gtloginfo("图像连接:%d(%s)被关闭,ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
                        }
                        else
                        {
                            printf     ("图像连接:%d(%s)断开,ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
                            gtloginfo("图像连接:%d(%s)断开,ret=%d\n",usr->fd,inet_ntoa(usr->addr.sin_addr),ret);
                        }
						//断开后发送停止命令给encbox
						//stop_up_audio2enc(usr->aenc_no);
                        break;
                    }
                }
            }
            else
            {
                printf     ("FD_ISSET(accept_fd,&read_fds)!=1!!!\n");
                gtloginfo("FD_ISSET(accept_fd,&read_fds)!=1!!!\n");
                break;               
            }
        }
        ///断开连接
        pthread_mutex_lock(&svr->s_mutex);
        del_av_usr_list(usr);                               ///<删除用户
        pthread_mutex_unlock(&svr->s_mutex);
		FD_CLR(accept_fd,&read_fds);
        close(accept_fd);                                       ///<关闭网络连接
        accept_fd=-1; 
    }
    return NULL;
}

/** 
 *   @brief     创建侦听网络音视频上行服务的socket以及线程池
 *   @return   0表示成功,负值表示失败
 */ 
int create_rtnet_av_servers(void)
{
    tcprtimg_svr_t *p=get_rtimg_para();
    av_server_t     *svr=&p->av_server;
    av_usr_t          *usr=NULL;
    int                   fd;
    int                   i;

    fd = create_tcp_listen_port(INADDR_ANY,p->av_svr_port); ///<创建网络音视频服务socket
    if(fd<0)
    {
        printf("can't create socket port:%d:%s exit !\n",p->av_svr_port,strerror(-fd));
        printf("can't create socket port:%d:%s exit !\n",p->av_svr_port,strerror(-fd));
        exit(1);
    }
    svr->listen_fd=fd;
    net_activate_keepalive(svr->listen_fd);          ///<探测对方断开
    listen(svr->listen_fd,TCPRTIMG_MAX_AVUSR_NO*p->virdev_num);            ///<设置侦听连接数

    for(i=0;i<(p->max_lan_usrs+p->max_wan_usrs+1);i++)  //多一个用于处理忙
    {
        usr=&svr->av_usr_list[i];
        gt_create_thread(&usr->thread_id, rtnet_av_cmd_thread, (void*)usr);
    }
    return 0;
}


