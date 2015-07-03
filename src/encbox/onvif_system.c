#include "onviflib.h"
#include "onvif_system.h"
#include "iniparser.h"
#include "vs3_videoenc.h"
#include "videoencoder.h"
#include <stdio.h>
#include "gtthread.h"
#include <math.h>

#define THE_ONE		1000000      //usec per sec
#define MAC_LEN_IN_BYTE 6
onvif_device_t onvif_device[MAX_ONVIF_DEVICE];
onvif_device_t temp_onvif_device[MAX_ONVIF_DEVICE*2];
extern video_enc_t video_enc[MAX_ONVIF_DEVICE*2];
extern audio_enc_t audio_enc[MAX_ONVIF_DEVICE*2];

FILE * pfd_main,*pfd_slave;

int temp_onvif_count=0;
unsigned char a2x(const char c)  
{  
      switch(c) {  
         case '0'...'9':  
            return (unsigned char)atoi(&c);  
         case 'a'...'f':  
            return 0xa + (c-'a');  
         case 'A'...'F':  
             return 0xa + (c-'A');  
         default:  
           goto error;  
      }  
      error:  
      exit(0);  
}

void COPY_STR2MAC(char *mac,char *str)  
{
	int i; 
	do { 
		for(i = 0; i < MAC_LEN_IN_BYTE; i++) 
		{
		  mac[i] = (a2x(str[i*3]) << 4) + a2x(str[i*3 + 1]);
		}
	}while(0);
}

onvif_device_t * get_onvif_para(int no)
{

	return &onvif_device[no];

}

// 读取配置文件对onvif_device实例赋值
int read_config_file(char * filename)
{



	int i,total;
	int ret;
	dictionary      *ini;	
	char name[50];
	char *pstr=NULL;
	char buf_tmp[50];
	ini=iniparser_load(filename);
	if (ini==NULL) 
	{
		printf("parse ini file file [%s]\n", filename);
		return -1;
	}
	
	onvif_device_t * device = NULL;
	total=get_onvif_device_num();
	if(total>MAX_ONVIF_DEVICE)
		total = MAX_ONVIF_DEVICE;

	for(i=0;i<total;i++)
	{
		device = &onvif_device[i];
		sprintf(name,"onvif%d:ip",i);

		pstr = iniparser_getstring(ini,name,"0.0.0.0");
		memcpy(device->ip,pstr,strlen(pstr));

		sprintf(name,"onvif%d:username",i);
		pstr = iniparser_getstring(ini,name,"admin");
		memcpy(device->username,pstr,strlen(pstr));
		
		
		sprintf(name,"onvif%d:password",i);
		pstr = iniparser_getstring(ini,name,"admin");
		memcpy(device->password,pstr,strlen(pstr));

		//init audio_server_t
		memset(&device->audio,0,sizeof(device->audio));
		memset(buf_tmp, 0, sizeof(buf_tmp));
		sprintf(buf_tmp, "%s%d%s","audio",i,":internal");
		ret=iniparser_getint(ini, buf_tmp, 1);
		if(ret==1)
		{
			printf("get audio[%d] ONVIF dev\n",i);	
			device->audio.enc_no=-1;	
		
		}
		else
		{
			printf("get audio[%d] IP dev\n",i);	
			memset(buf_tmp, 0, sizeof(buf_tmp));
			sprintf(buf_tmp, "%s%d%s","audio",i,":up_port");
			ret=iniparser_getint(ini,buf_tmp,5556+i*1000);
			device->audio.up_port=ret;

			memset(buf_tmp, 0, sizeof(buf_tmp));
			sprintf(buf_tmp, "%s%d%s","audio",i,":cmd_port");
			ret=iniparser_getint(ini,buf_tmp,5554+i*1000);
			device->audio.cmd_port=ret;



			memset(buf_tmp, 0, sizeof(buf_tmp));
			sprintf(buf_tmp, "%s%d%s","audio",i,":audio_down_port");
			ret=iniparser_getint(ini,buf_tmp,5555+i*1000);
			device->audio.audio_down_port=ret; 


			memset(buf_tmp, 0, sizeof(buf_tmp));
			sprintf(buf_tmp, "%s%d%s","audio",i,":ip_addr");
			pstr=iniparser_getstring(ini,buf_tmp,"192.168.1.21");
			inet_aton(pstr,(struct in_addr *)&device->audio.ip_addr);


			memset(buf_tmp, 0, sizeof(buf_tmp));
			sprintf(buf_tmp, "%s%d%s","audio",i,":mask");
			pstr=iniparser_getstring(ini,buf_tmp,"255.255.255.0");
			inet_aton(pstr,(struct in_addr *)&device->audio.mask);


			memset(buf_tmp, 0, sizeof(buf_tmp));
			sprintf(buf_tmp, "%s%d%s","audio",i,":guid");
			pstr=iniparser_getstring(ini,buf_tmp,"0000000000000000");
			memcpy(&device->audio.guid,pstr,16);


			memset(buf_tmp, 0, sizeof(buf_tmp));
			sprintf(buf_tmp, "%s%d%s","audio",i,":mac");
			pstr=iniparser_getstring(ini,buf_tmp,"00:00:00:00:00:00");

			COPY_STR2MAC((char*)&device->audio.mac,pstr);
			device->audio.enc_no=i;

		
	
		}
		
		
	
	}
	

	save_inidict_file(filename,ini,NULL);
	iniparser_freedict(ini);	    
	return 0;
}
// 初始化 onvif_device实例
int init_onvif_devices(void)
{

	int i,total;
	onvif_device_t * device= NULL; 
	total=get_onvif_device_num();
	for(i=0;i<total;i++)
	{
		device = &onvif_device[i];
		memset(device,0,sizeof(onvif_device_t));
		pthread_mutex_init(&device->mutex,NULL);
		device->token = 0;
		device->is_running = 0;
		device->ready_status = STATUS_INIT;
		device->channel=i;


	}
	return 0;



}
int init_onvif_devices_list()
{

	init_onvif_devices();

	read_config_file(PARA_FILE);
	return 0;

}
//encbox注册的probe callback，在设备列表找到在线设备并设置token
int  self_probe_cb(dev_handler_t token,device_info_t * pinfo)
{
	int i;
	onvif_device_t * device= onvif_device;
	printf("get onvif device ip:%s  port:%d token:%u\n",pinfo->host,pinfo->port,token);
		
	for(i=0;i<MAX_ONVIF_DEVICE;i++)
	{
		if(0==strcmp(pinfo->host,device->ip)&&device->token==0)
		{

			pthread_mutex_lock(&device->mutex);
			device->token=token;
			pthread_mutex_unlock(&device->mutex);
			printf("find onvif device %s in para file\n ",device->ip);
		
		}	
		device++;	
	}
	device=NULL;
	return 0;
	
}

void  mod_probe_cb(dev_handler_t token,device_info_t * pinfo)
{

	if(temp_onvif_count==MAX_ONVIF_DEVICE*2)
		printf("find %d+ onvif device,next device will not show\n",MAX_ONVIF_DEVICE*2);

	onvif_device_t * device= &temp_onvif_device[temp_onvif_count];
	printf("mod_probe_cb get onvif device ip:%s  port:%d token:%u\n",pinfo->host,pinfo->port,token);
	memcpy(&device->ip,&pinfo->host,24);
	temp_onvif_count++;
	
}
void get_video_info(video_mediainfo_t * v_info,int *width,int * height,int *format)
{
	switch(v_info->ratio){
	
		case RATIO_D1_PAL:
			*width=704;
			*height=576;
			break;
		case RATIO_D1_NTSC:
			*width=704;
			*height=480;
			break;
		case RATIO_CIF_PAL:
			*width=352;
			*height=288;
			break;
		case RATIO_CIF_NTSC:
			*width=352;
			*height=240;
			break;
		case RATIO_720P:
			*width=1280;
			*height=720;
			break;
		case RATIO_1080P:
			*width=1920;
			*height=1080;
			break;
		case RATIO_ERR:
			*width=704;
			*height=576;
			printf("RATIO ERR! USE DEFAULT RATIO\n");
			break;
		default:
			*width=704;
			*height=576;
			printf("RATIO ERR! USE DEFAULT RATIO\n");
			break;
	}
	switch(v_info->format){
	
		case VT_MPEG4:
			*format=VIDEO_MPEG4;
			break;
		case VT_H264:
			*format=VIDEO_H264;
			break;
		default:
			printf("FORMAT ERR! USE DEFAULT FORMAT %d\n",VIDEO_H264);
			*format=VIDEO_H264;
			break;
	}
}
int get_media_pool(stream_type_t  type)
{
	if (type==STREAM_MAIN)
	  return 0x30000;
	else if (type==STREAM_SEC)
	  return 0x31000;
	else 
	  return 0;

}
// 设置videoenc属性
void  init_video_system(onvif_device_t * device)
{
	int width,height,ratio;
	video_enc_t *enc=NULL;
	media_attrib_t *attrib=NULL;
	

	// main_stream
	
	get_video_info(&device->main_stream.v_info,&width,&height,&ratio);	
	enc=&video_enc[device->channel*2+STREAM_MAIN];
	attrib=enc->media.attrib;
	attrib->stat=ENC_STAT_OK;
	enc->media.buflen=FRAME_BUFFER_SIZE ;
	attrib->fmt.v_fmt.v_width=width;
	attrib->fmt.v_fmt.v_height=height;
	attrib->fmt.v_fmt.format=ratio;
	attrib->fmt.v_fmt.ispal=1;
	attrib->fmt.v_fmt.v_frate=device->main_stream.v_info.framerate;
	attrib->fmt.v_fmt.v_buffsize=FRAME_BUFFER_SIZE ;


	//sec_stream
	
	get_video_info(&device->sec_stream.v_info,&width,&height,&ratio);	
	enc=&video_enc[device->channel*2+STREAM_SEC];
	attrib=enc->media.attrib;
	attrib->stat=ENC_STAT_OK;
	enc->media.buflen=FRAME_BUFFER_SIZE;
	attrib->fmt.v_fmt.v_width=width;
	attrib->fmt.v_fmt.v_height=height;
	attrib->fmt.v_fmt.format=ratio;
	attrib->fmt.v_fmt.ispal=1;
	attrib->fmt.v_fmt.v_frate=device->sec_stream.v_info.framerate;
	attrib->fmt.v_fmt.v_buffsize=FRAME_BUFFER_SIZE;












}
void  init_audio_system(onvif_device_t * device)
{
	audio_enc_t *enc=NULL;
	media_attrib_t *attrib=NULL;
	

	audio_mediainfo_t * a_info=&device->main_stream.a_info;

//  main_stream
	enc=&audio_enc[device->channel*2+STREAM_MAIN];
	attrib=enc->media.attrib;
	attrib->stat=ENC_STAT_OK;
	enc->media.buflen=AUDIO_FRAME_SIZE ;
	attrib->fmt.a_fmt.a_wformat=a_info->a_wformat;
	attrib->fmt.a_fmt.a_sampling=a_info->a_sampling;
	attrib->fmt.a_fmt.a_channel=1;
	
	attrib->fmt.a_fmt.a_nr_frame=1;//a_info->a_nr_frame;
	attrib->fmt.a_fmt.a_bitrate=8000;//a_info->a_bitrate;
	attrib->fmt.a_fmt.a_bits=8;//attrib->fmt.a_fmt.a_bitrate;
	attrib->fmt.a_fmt.a_frate=25;//a_info->a_frate;
	
	printf("wformat %d\n",	attrib->fmt.a_fmt.a_wformat);
	printf("sampling %d\n",attrib->fmt.a_fmt.a_sampling);
	printf("channel %d\n",attrib->fmt.a_fmt.a_channel);
	printf("bitrate %d\n",attrib->fmt.a_fmt.a_bitrate);
	printf("bits %d\n",attrib->fmt.a_fmt.a_bits);









	a_info=&device->sec_stream.a_info;


// sec_stream
	enc=&audio_enc[device->channel*2+STREAM_SEC];
	attrib=enc->media.attrib;
	attrib->stat=ENC_STAT_OK;
	enc->media.buflen=AUDIO_FRAME_SIZE ;
	attrib->fmt.a_fmt.a_wformat=a_info->a_wformat;
	attrib->fmt.a_fmt.a_sampling=a_info->a_sampling;
	attrib->fmt.a_fmt.a_channel=1;
	attrib->fmt.a_fmt.a_nr_frame=1;
	attrib->fmt.a_fmt.a_bitrate=8000;//a_info->a_bitrate;
	attrib->fmt.a_fmt.a_bits=8;//attrib->fmt.a_fmt.a_bitrate;
	attrib->fmt.a_fmt.a_frate=25;//a_info->a_frate;
	
	printf("wformat %d\n",	attrib->fmt.a_fmt.a_wformat);
	printf("sampling %d\n",attrib->fmt.a_fmt.a_sampling);
	printf("channel %d\n",attrib->fmt.a_fmt.a_channel);
	printf("bitrate %d\n",attrib->fmt.a_fmt.a_bitrate);
	printf("bits %d\n",attrib->fmt.a_fmt.a_bits);


	
	
}
video_Callback onvif_video_cb(unsigned char * frame_data,int len,unsigned int ts, unsigned short seq,void * usr_data)
{

	static struct timeval tv1[2],tv2;
	int frate;
	int ret;
	static int frame_count[2];
	static int only_do_once[2]={1,1};
	static int set_attrib_flag[2]={1,1};
	if(usr_data==NULL)
	{
		printf("usr_data is NULL!\n");
	    return (video_Callback)-1;
	}
	callback_parm_t *parm=(callback_parm_t *)usr_data;
	enc_frame_t *buf=NULL;
	video_enc_t *enc=NULL; 
	stream_type_t type=parm->type;
	if(type!=STREAM_MAIN && type!=STREAM_SEC)
	{
	  printf("frame type err type is %d\n",type);
	  return (video_Callback)-1;
	}
	if(only_do_once[type])
	{
		gettimeofday(&tv1[type],NULL);
		only_do_once[type]=0;
	
	}

	enc=&video_enc[parm->channel*2+type];
	buf=(enc_frame_t *)enc->media.temp_buf;

	memcpy(buf->frame_buf,frame_data,len);
	

	
	

	buf->len=len;
	buf->media=MEDIA_VIDEO;

	buf->type=1; //is p frame
	if((frame_data[4]&31)==7)
	{
		buf->type=0; //is i frame
	}

	buf->tv.tv_sec=ts/SECOND;
	buf->tv.tv_usec=ts%SECOND;
	buf->chunk.chk_id=IDX1_VID;
	buf->chunk.chk_siz=len;




	if(frame_data!=NULL)
	{
		ret=write_media_resource(&enc->media,buf,len+sizeof(enc_frame_t)-4,buf->type);
		if(buf->type==0)
		 
			printf("Got a video data length[%d] stream_type:%d  channel:%d iflag=%d ts=%u \n",len,type,parm->channel,buf->type,ts);
		
		/*
		if(type==STREAM_MAIN)

		  fwrite(buf->frame_buf,1,len,pfd_main);

		else

		  fwrite(buf->frame_buf,1,len,pfd_slave);
		*/
	


		  
	

	}
	else 
	{
		printf("Video frame_data is NULL\n");
	}
	gettimeofday(&tv2,NULL);
	if(set_attrib_flag[type])
	{	
		if(tv2.tv_sec*THE_ONE+tv2.tv_usec-tv1[type].tv_sec*THE_ONE+tv1[type].tv_usec>10*THE_ONE)
		{
			frate=frame_count[type]/10+1;
			patch_video_attrib(enc,frate);
			set_attrib_flag[type]=0;
		}
	}
	frame_count[type]++;


	return 0;


}
void patch_audio_attrib(audio_enc_t *enc,int frate)
{
	media_attrib_t *attrib=NULL;
	attrib=enc->media.attrib;
	attrib->fmt.a_fmt.a_frate=frate;

}
void patch_video_attrib(video_enc_t *enc,int frate)
{
	media_attrib_t *attrib=NULL;
	attrib=enc->media.attrib;
	attrib->fmt.v_fmt.v_frate=frate;

}
audio_Callback onvif_audio_cb(unsigned char * frame_data,int len,void * usr_data)
{

	static struct timeval tv1[2],tv2;
	int frate;
	static int frame_count[2];
	static int only_do_once[2]={1,1};
	static int set_attrib_flag[2]={1,1};
	if(usr_data==NULL)
	{
		printf(" usr_data is NULL!\n");
	    return (audio_Callback)-1;
	}
	
	callback_parm_t *parm=(callback_parm_t *)usr_data;

	enc_frame_t *buf=NULL;
	audio_enc_t *enc=NULL; 

	int ret;
	stream_type_t type=parm->type;

	
	if(type!=STREAM_MAIN && type!=STREAM_SEC)
	{
	  printf("frame type err type is %d\n",type);
	  return (audio_Callback)-1;
	}


	if(only_do_once[type])
	{
		gettimeofday(&tv1[type],NULL);
		only_do_once[type]=0;
	}

	enc=(audio_enc_t*)&audio_enc[parm->channel*2+parm->type];  
	buf=(enc_frame_t *)enc->media.temp_buf;

	memcpy(buf->frame_buf,frame_data,len);
	

	
	

	buf->len=len;
	buf->media=MEDIA_AUDIO;

	buf->tv.tv_sec=0;//ts/SECOND;
	buf->tv.tv_usec=0;//ts%SECOND;
	buf->chunk.chk_id=IDX1_AID;
	buf->chunk.chk_siz=len;
	buf->type=FRAMETYPE_A;




	if(frame_data!=NULL)
	{
		ret=write_media_resource(&enc->media,buf,len+sizeof(enc_frame_t)-4,buf->type);
		//printf("Got a audio data length[%d]  stream_type[%d] channel:%d iflag=%d  \n",len,type,parm->channel,buf->type);
	    //fwrite(buf->frame_buf,1,len,pfd_main);
	}

	else 
	{
		printf("Audio frame_data is NULL\n");
	}
	gettimeofday(&tv2,NULL);
	if(set_attrib_flag[type])
	{	
		if(tv2.tv_sec*THE_ONE+tv2.tv_usec-tv1[type].tv_sec*THE_ONE+tv1[type].tv_usec>10*THE_ONE)
		{
			frate=frame_count[type]/10+1;
			patch_audio_attrib(enc,frate);
			set_attrib_flag[type]=0;
		}
	}
	frame_count[type]++;
	return 0;


}


notify_Callback notify_cb(int event, void * usr_data)
{

	callback_parm_t *parm=(callback_parm_t *)usr_data;
	onvif_device_t * device=&onvif_device[0];



	switch (event){

	case RTSP_STOPPED: //0

	case RTSP_CONNFAIL: //2

		pthread_mutex_lock(&device->mutex);
		(device+parm->channel)->ready_status=STATUS_MEDIA_DISCON;
		pthread_mutex_unlock(&device->mutex);
		break;
	case RTSP_CONNECTING: //1

		//noting todo
		break;

	case RTSP_CONNSUCC: //3

		pthread_mutex_lock(&device->mutex);
		(device+parm->channel)->ready_status=STATUS_MEDIA_CONNECED;
		pthread_mutex_unlock(&device->mutex);
		break;

	default:
		break;
		//noting todo

	
	}


	printf("notify_cb EVENT %d stream_type:%d channel:%d\n",event,parm->type,parm->channel);
	


	return 0;


}


void  * onvif_device_thread(void * arg)
{

	onvif_device_t * device=(onvif_device_t  * )arg;
	audio_Callback acb=NULL;


	callback_parm_t parm[2];
	int ret;
	while(1) 
	{
		//printf("onvif_device_thread start! no:%d ip;%s\n",device->channel,device->ip);
		//printf("token=%u online=%d\n",device->token,device->online);
		if(device->token!=0 && device->ready_status==STATUS_INIT) //has probed & has not start stream
		{
			while(1) //login success
			{
				onvif_lib_set_auth(device->token,device->username,device->password);
			
				//get stream_main info
				ret=onvif_lib_get_av_mediainfo(device->token,STREAM_MAIN,&device->main_stream.v_info,&device->main_stream.a_info);

				if(ret!=GT_SUCCESS)
				{
					printf("onvif_lib_get_av_mediainfo STREAM_MAIN failed! ret=%d\n",ret);
					sleep(1);
					continue;
				
				}
				//get stream_sec info
				ret=onvif_lib_get_av_mediainfo(device->token,STREAM_SEC,&device->sec_stream.v_info,&device->sec_stream.a_info);

				if(ret!=GT_SUCCESS)
				{
					printf("onvif_lib_get_av_mediainfo STREAM_SEC failed! ret=%d\n",ret);
					sleep(1);
					continue;
				
				}
				if(device->is_media_init==0)
				{
					printf("set v attrib\n");
					init_video_system(device); //create video_pool and set media_attrib 
					if(device->audio.enc_no==-1)//onvif talkback
					{
						acb=(audio_Callback)onvif_audio_cb;
						init_audio_system(device); //create audio_pool and set media_attrib 
						printf("set a attrib\n");
		

					}
					device->is_media_init=1;
				}
					
				pthread_mutex_lock(&device->mutex);
				device->ready_status=STATUS_LOGIN;
				pthread_mutex_unlock(&device->mutex);
				break;
					

			}
			parm[0].channel=device->channel;
			parm[0].type=STREAM_MAIN;
			parm[1].channel=device->channel;
			parm[1].type=STREAM_SEC;
	
			onvif_lib_rtsp_start(device->token,STREAM_MAIN,(video_Callback)onvif_video_cb,(audio_Callback)acb,(notify_Callback)notify_cb,(void *)&parm[0]);

			onvif_lib_rtsp_start(device->token,STREAM_SEC,(video_Callback)onvif_video_cb,(audio_Callback)acb,(notify_Callback)notify_cb,(void *)&parm[1]);

			pthread_mutex_lock(&device->mutex);

			device->is_running=1;
			device->ready_status=STATUS_MEDIA_CONNECTING;
	
			pthread_mutex_unlock(&device->mutex);


			
		}
		else if(device->token!=0 && device->ready_status==STATUS_MEDIA_DISCON) //had probed & disconnected
		{

			onvif_lib_rtsp_stop(device->token,STREAM_MAIN); //先停止
			onvif_lib_rtsp_stop(device->token,STREAM_SEC);

			onvif_lib_rtsp_start(device->token,STREAM_MAIN,(video_Callback)onvif_video_cb,(audio_Callback)acb,(notify_Callback)notify_cb,(void *)&parm[0]);
			onvif_lib_rtsp_start(device->token,STREAM_SEC,(video_Callback)onvif_video_cb,(audio_Callback)acb,(notify_Callback)notify_cb,(void *)&parm[1]);

			pthread_mutex_lock(&device->mutex);
			device->ready_status=STATUS_MEDIA_CONNECTING;
			pthread_mutex_unlock(&device->mutex);


			printf("\t\tRECONNECTING\n");
		
		}
		sleep(1);
	}

}

void  * onvif_probe_device_thread(void * arg)
{
	onvif_device_t *device=NULL;
	device=onvif_device;
	int i;
		
	while(1)
	{
		onvif_lib_start_probe((probe_callback)arg,NULL);       
		for(i=0;i<MAX_ONVIF_DEVICE;i++)
		{
			if(device->token==0) //until all the onvif found
			{
				break;
			}
			device++;
		}
		sleep(60);
	
	}

}
// 初始化onvif_system 
int init_onvif_system(void)
{

	static pthread_t id;
	init_onvif_devices_list(); 

	if(GT_SUCCESS!= onvif_lib_init())
		return GT_FAIL;

	gt_create_thread(&id,onvif_probe_device_thread,(void *)self_probe_cb);

	//if(GT_SUCCESS!= onvif_lib_start_probe((probe_callback)self_probe_cb,NULL))        
	//	return GT_FAIL;

	return GT_SUCCESS; 
}


void create_onvif_device_thread(void)
{

	int i;
	onvif_device_t *device=NULL;

	for(i=0;i<MAX_ONVIF_DEVICE;i++)
	{
		device=&onvif_device[i];
		gt_create_thread(&device->thread_id,onvif_device_thread,(void *)device);


	
	}
	pfd_main=fopen("/mnt/zsk/onvif_main.h264", "w");
	pfd_slave=fopen("/mnt/zsk/onvif_slave.h264", "w");


}

