#include <stdio.h>                /* perror() */
#include <stdlib.h>               /* atoi() */
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <unistd.h>               /* read() */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>

#include "onviflib.h"

#define DEV_IP		"192.168.10.191"
#define DEV_PORT 	554
#define USERNAME        "admin"
#define PASSWORD        "admin"

dev_handler_t g_devs[100];
int           g_devs_index;
int           g_rtpdatafile;
int           g_rtpadatafile;

typedef struct
{
    int a;
    int b;
}userdata_t;

userdata_t g_ud;

void probecb(dev_handler_t dev_index,device_info_t* pinfo)
{
    printf("probe dev handler %u:info:\n",dev_index);
    printf("type:%d\n",pinfo->type);
    printf("reference:%s\n",pinfo->EndpointReference);
    printf("host:%s,port:%d\n",pinfo->host,pinfo->port);
    printf("ip:%u\n",pinfo->ip);
    printf("url:%s\n",pinfo->url);

    if(dev_index > 0)
    {
        g_devs[g_devs_index] = dev_index;
        ++g_devs_index;
    }
}

int videocb(unsigned char* data,int len,unsigned int ts,unsigned short seq,void* usr_data)
{
    printf("recv video data len %d,ts is %u\n",len,ts);
    //write(g_rtpdatafile,data,len);
}

int audiocb(unsigned char* data,int len,void* usr_data)
{
    printf("recv audio data len %d\n",len);
    write(g_rtpadatafile,data,len);
}

int notifycb(int event, void * usr_data)
{
    printf("got stream event %d\n",event);
}

int main(int argc,char* argv[])
{
    int ret;
    video_mediainfo_t vinfo;
    audio_mediainfo_t ainfo;
    ret = onvif_lib_init();
    if(ret < 0)
    {
        printf("onviflib init err!%d\n",ret);
        return ret;
    }
    
    ret = onvif_lib_start_probe(probecb,NULL);
    if(ret < 0)
    {
        printf("onviflib start probe err!%d\n",ret);
        return ret;
    }
    
    sleep(5);

    ret = onvif_lib_set_auth(g_devs[0],USERNAME,PASSWORD);
    if(ret < 0)
    {
        printf("onviflib set auth to dev %u err:%d\n",g_devs[0],ret);
        return ret;
    }
    
    ret = onvif_lib_get_av_mediainfo(g_devs[0],STREAM_MAIN,&vinfo,&ainfo);
    if(ret < 0)
    {
        printf("onviflib get stream main av info err:%d\n",ret);
        return ret;
    }
    printf("dev %u,stream %d:\n",g_devs[0],STREAM_MAIN);
    printf("video info:\n");
    printf("format %d,ratio %d\n",vinfo.format,vinfo.ratio);
    printf("framerate %d\n",vinfo.framerate);
    printf("audio info:\n");
    printf("sampling %d\n,format %d\n,channel %d\n,bitrate %d\n,bits %d\n,frate %d\n",ainfo.a_sampling,ainfo.a_wformat,ainfo.a_channel,ainfo.a_bitrate,ainfo.a_bits,ainfo.a_frate);

    ret = onvif_lib_get_av_mediainfo(g_devs[0],STREAM_SEC,&vinfo,&ainfo);
    if(ret < 0)
    {
        printf("onviflib get stream sec av info err:%d\n",ret);
        return ret;
    }
    printf("dev %u,stream %d:\n",g_devs[0],STREAM_SEC);
    printf("video info:\n");
    printf("format %d,ratio %d\n",vinfo.format,vinfo.ratio);
    printf("framerate %d\n",vinfo.framerate);
    printf("audio info:\n");
    printf("sampling %d\n,format %d\n,channel %d\n,bitrate %d\n,bits %d\n,frate %d\n",ainfo.a_sampling,ainfo.a_wformat,ainfo.a_channel,ainfo.a_bitrate,ainfo.a_bits,ainfo.a_frate);

    g_rtpdatafile = open("rtpdata.h264",O_RDWR|O_CREAT);
    if(g_rtpdatafile<0)
    {
        printf("open data file error!\n");
        return -1;
    }

    g_rtpadatafile = open("rtpaudio.raw",O_RDWR|O_CREAT);
    if(g_rtpadatafile<0)
    {
        printf("open audio data file error!\n");
        return -1;
    }

    ret = onvif_lib_rtsp_start(g_devs[0],STREAM_MAIN,videocb,audiocb,notifycb,&g_ud);
    if(ret < 0)
    {
        printf("onviflib start rtsp at dev %u err%d\n",g_devs[0],ret);
        return ret;
    }
	

    while(1)
    {
        sleep(1);
    }

    return 0;
}













