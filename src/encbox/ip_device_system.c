#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "onvif_system.h"
#include "fenc_api.h"
#include "videoencoder.h"
#include "ip_device.h"
#include "gtlog.h"
#include "gtthread.h"
#include "string.h"
#include "process_modcmd.h"
#include "vs3_videoenc.h"
#include "AVIEncoder.h"
int AencFd[MAX_ONVIF_DEVICE]={-1,-1,-1,-1};
extern audio_enc_t audio_enc[MAX_ONVIF_DEVICE*2];
void init_ip_device_server(void * parm)
{

	int recvfd;
	struct sockaddr_in server;
	audio_server_t *server_parm=(audio_server_t *)parm;
	if((recvfd=socket(AF_INET,SOCK_DGRAM,0))==-1)
	{
		perror("Creating recvfd failed.");
		exit(1);
	
	}
	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(server_parm->up_port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(recvfd,(struct sockaddr *)&server,sizeof(struct sockaddr))==-1)
	{
		perror("Bind()error.");
		exit(1);
	
	}
	AencFd[server_parm->enc_no]=recvfd;
	require_up_audio(server_parm->enc_no);//请求一次
	
}

void *ip_dev_check_thread(void * no)
{

	int channel=*(int*)no;
	while(1)
	{
		//todo check status
		sleep(1);
	}

}
void *ip_read_frame_thread(void  *parm)
{
    int ret=0;
	int i;
	int maxfds=0;
	char frame_buf[4096];
	enc_frame_t *buf=NULL;
	audio_enc_t *enc=NULL; 
	audio_t * pbuf=NULL;
	fd_set read_fds;
    struct timeval TimeoutVal;
	struct sockaddr_in client;
	onvif_device_t * device=(onvif_device_t*)parm;
	socklen_t addrlen=sizeof(client);
	int once[MAX_ONVIF_DEVICE]={1,1,1,1};

    for (i=0; i<MAX_ONVIF_DEVICE; i++)
    {
		if(maxfds <= AencFd[i])    
		maxfds = AencFd[i];
		
    }
	while (1)
    {
		TimeoutVal.tv_sec = 1;
 		TimeoutVal.tv_usec = 0;

        FD_ZERO(&read_fds);
		for (i=0; i<MAX_ONVIF_DEVICE; i++)
		{
			if(AencFd[i]>0)
			{
				FD_SET(AencFd[i], &read_fds);
			}
		}
		ret = select(maxfds+1, &read_fds, NULL, NULL, &TimeoutVal);
 		if (ret < 0) {
			gtloginfo("select <0 exit\n");
            //break;
			continue;
 		}
 		else if (ret == 0) {
            continue;
 		}

        for (i=0; i<MAX_ONVIF_DEVICE; i++)
        {
            if (FD_ISSET(AencFd[i], &read_fds))
            {
				enc=(audio_enc_t*)&audio_enc[i*2+1]; //rtimage 连接副码流 
				buf=(enc_frame_t *)enc->media.temp_buf;
				if(once[i]==1)
				{
					init_ip_audio_attrib(device+i);
					once[i]=0;
				}


				ret=recvfrom(AencFd[i],(void *)frame_buf,4096,0,(struct sockaddr*)&client,&addrlen);
				
				if(ret<0)
				{
					  perror("recvform() error\n");
		  
				}
				pbuf=(audio_t *)&frame_buf;

				if(pbuf->audio_head!=0x77061600||pbuf->audio_length!=ret-8)
				{
					gtloginfo("packet recv error head=%#X  needhead=%#X\n length=%#X recvlen=%#X\n",pbuf->audio_head,0x77061600,pbuf->audio_length,ret-8);
				}
				memcpy(buf->frame_buf,pbuf,ret);
				

				printf("Got  data form ip:%s,port %d\n",inet_ntoa(client.sin_addr),htons(client.sin_port));

				buf->len=ret ;
				buf->media=MEDIA_AUDIO;
				buf->tv.tv_sec=0;
				buf->tv.tv_usec=0;
				buf->chunk.chk_id=IDX1_AID;
				buf->chunk.chk_siz=ret;
				buf->type=FRAMETYPE_AAC;
				write_media_resource(&enc->media,(void *)buf,ret+sizeof(enc_frame_t)-4,FRAMETYPE_AAC);
			

			
				

				

			}

		}
    }

}

void  init_ip_audio_attrib(onvif_device_t * device)
{
	audio_enc_t *enc=NULL;
	media_attrib_t *attrib=NULL;
//main_stream
/*
	enc=&audio_enc[device->channel*2+STREAM_MAIN];
	attrib=enc->media.attrib;
	attrib->stat=ENC_STAT_OK;
	enc->media.buflen=IP_AUDIO_FRAME_SIZE;
	attrib->fmt.a_fmt.a_wformat=WFORMAT_AAC;
	attrib->fmt.a_fmt.a_sampling=16000;
	attrib->fmt.a_fmt.a_channel=2;
	attrib->fmt.a_fmt.a_nr_frame=1;
	attrib->fmt.a_fmt.a_bitrate=32000;
	attrib->fmt.a_fmt.a_bits=16;
	attrib->fmt.a_fmt.a_frate=8;
	*/
	
// only sec_stream
	enc=&audio_enc[device->channel*2+STREAM_SEC];
	attrib=enc->media.attrib;
	attrib->stat=ENC_STAT_OK;
	enc->media.buflen=IP_AUDIO_FRAME_SIZE;
	attrib->fmt.a_fmt.a_wformat=WFORMAT_AAC;
	attrib->fmt.a_fmt.a_sampling=16000;
	attrib->fmt.a_fmt.a_channel=2;
	attrib->fmt.a_fmt.a_nr_frame=1;
	attrib->fmt.a_fmt.a_bitrate=32000;
	attrib->fmt.a_fmt.a_bits=16;
	attrib->fmt.a_fmt.a_frate=8;
	
	
}
void * ip_play_frame_thread(void * parm)
{
	audio_server_t *audio=(audio_server_t *)parm;
	int no=audio->enc_no;
	int	seq=-1;                 ///<媒体数据序号
	int flag;
	int ret=-1;
	int sendfd;
	int num;
	char audio_buf[IP_AUDIO_FRAME_SIZE]={0};
	struct sockaddr_in client;
	socklen_t addrlen=sizeof(client);
	if((sendfd = socket(AF_INET,SOCK_DGRAM,0))==-1)
	{
		perror("Creating sendfd failed");
		exit(1);
	}
	bzero(&client,sizeof(client));
	client.sin_family=AF_INET;
	client.sin_port=htons(audio->audio_down_port);
	memcpy(&client.sin_addr.s_addr,&audio->ip_addr,sizeof(in_addr_t));
	init_audio_dec_usr(no);
	connect_audio_dec_succ(no,"dec_usr");
    while(1)
    {
		ret=read_adec_frame(no,(void *)audio_buf,IP_AUDIO_FRAME_SIZE ,&seq,&flag);
		if(ret<0)
		{
			printf("error in read media resource\n");
		}
		else 
		{
			num=sendto(sendfd,audio_buf,ret,0,(struct sockaddr *)&client,addrlen);
		}	

		if (num < 0)
		{
			printf("send to ip device  error\n");
			gtlogerr("send to ip device  error\n");
		}
	}
}
	
void create_ip_device_thread(void)
{

	int i;
	int have_ip_device=0;
	onvif_device_t *device=NULL;
	static pthread_t status_tid[MAX_ONVIF_DEVICE];
	static pthread_t audio_server_tid;
	static pthread_t audio_dec_tid[MAX_ONVIF_DEVICE];

	for(i=0;i<MAX_ONVIF_DEVICE;i++)
	{
		device=get_onvif_para(i);
		if(device->audio.enc_no!=-1)
		{
		
			init_ip_device_server((void *)&device->audio);
			gt_create_thread(&audio_dec_tid[i],ip_play_frame_thread,(void *)&device->audio);
			have_ip_device++;
		}

	}
	if(have_ip_device>0)
	{
		gt_create_thread(&audio_server_tid,ip_read_frame_thread,(void *)device);
		gt_create_thread(&status_tid[i],ip_dev_check_thread,(void *)&device->audio.enc_no);
			
	}

	
	


}



