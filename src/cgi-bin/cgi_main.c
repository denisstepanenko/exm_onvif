#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iniparser.h>
#include <sys/file.h>
#include "version.h"
#include "cgic.h"
#include "cgi_main.h"
#include "iconv.h"
#include "lock.c"
#include "mod_cmd.h"
#include "gtlog.h"
//#include "mod_socket_cmd.h"
#include "mod_socket.h"
#include <sys/socket.h>
#define MAX_IP_DEVICE 12
#define GET_START_TIME 1
#define GET_END_TIME 2

typedef int index_type;
//快速设置的内容

char * _AUDIO_PARM[]=
{
	"encoding_type",
	"sampling_rate",
	"sampling_precision",
	"sampling_channels"
};
char * _FAST_CONF[]=

{
	"eth0:ipaddr",
	"eth0:netmask",
	"net_config:route_default",
	"net_config:internet_mode",
	"net_config:adsl_user",
	"net_config:adsl_passwd",
	"port:cmd_port",
	"port:image_port",
	"port:audio_port",
	"port:web_port",
	"port:ftp_port",
	"port:pasv_min_port",
	"port:pasv_max_port",
	"port:com0_port",
	"port:com1_port",
	"port:telnet_port",
	"remote_gate:rmt_gate1",
	"remote_gate:rmt_gate2",
	"remote_gate:rmt_gate3",
	"remote_gate:rmt_gate4",
	"install:inst_place",
	"install:inst_place_prov",
	"install:inst_place_city",
	"install:inst_place_dist",
	"install:inst_place_vill",
	"remote_gate:alarm_gate",
	"serial:serial_mode",
	"serial:serial_interval",
	"alarm0:alarmtype",
	"alarm0:enable",
	"alarm1:alarmtype",
	"alarm1:enable",
	"alarm2:alarmtype",
	"alarm2:enable",
	"alarm3:alarmtype",
	"alarm3:enable",
	"alarm4:alarmtype",
	"alarm4:enable",
	"alarm5:alarmtype",
	"alarm5:enable",
	"alarm6:alarmtype",
	"alarm6:enable",
	"alarm7:alarmtype",
	"alarm7:enable",
	"alarm8:alarmtype",
	"alarm8:enable",
	"alarm9:alarmtype",
	"alarm9:enable",
	"alarm10:alarmtype",
	"alarm10:enable",
	"alarm11:alarmtype",
	"alarm11:enable",
	"multichannel:enable",
	"netencoder:playback_enable",
	"buzalarm:enable",
	"power_mon:enable"
};

//设备信息内容 
char * _DEV_INFO[]=
{
	"devinfo:devguid",
	"devinfo:firmware",
	"devinfo:devtypestring",
	"devinfo:batchseq",
	"devinfo:cur_time",
	"leave_fac:date",
	"install:inst_place",
	"resource:trignum",
	"resource:com",
	"resource:quad",
	"resource:videonum",
	"resource:videoencnum",
	"resource:ide",
	"resource:eth_port",
	"resource:outnum",
	"resource:audionum",
	"resource:disk_capacity"
};
//音视频设置
char * _VIDEO_AUDIO_PARA[]=
{
	"netencoder:audio_vol",
	"video0:bright",
	"video0:hue",
	"video0:contrast",
	"video0:saturation",
	"video0:mic_vol",
	"video1:bright",
	"video1:hue",
	"video1:contrast",
	"video1:saturation",
	"video1:mic_vol",
	"video2:bright",
	"video2:hue",
	"video2:contrast",
	"video2:saturation",
	"video2:mic_vol",
	"video3:bright",
	"video3:hue",
	"video3:contrast",
	"video3:saturation",
	"video3:mic_vol",
	"netencoder:framerate",
	"netencoder:vidpicsize",
	"netencoder:bitratecon",
	"netencoder:targetbitrate",
	"netencoder:maxbitrate",
	"hqenc0:framerate",
	"hqenc0:vidpicsize",
	"hqenc0:bitratecon",
	"hqenc0:targetbitrate",
	"hqenc0:maxbitrate",
	"hqpara:rec_type",
	"hqpara:a_channel",
	"hqpara:pre_rec",
	"hqpara:dly_rec",
	"hqpara:cut_len",
	"audio0:internal",
	"audio0:guid",
	"audio0:mac",
	"audio0:ip_addr",
	"audio0:mask",
	"audio0:cmd_port",
	"audio0:audio_down_port",
	"audio1:internal",
	"audio1:guid",
	"audio1:mac",
	"audio1:ip_addr",
	"audio1:mask",
	"audio1:cmd_port",
	"audio1:audio_down_port",
	"audio2:internal",
	"audio2:guid",
	"audio2:mac",
	"audio2:ip_addr",
	"audio2:mask",
	"audio2:cmd_port",
	"audio2:audio_down_port",
	"audio3:internal",
	"audio3:guid",
	"audio3:mac",
	"audio3:ip_addr",
	"audio3:mask",
	"audio3:cmd_port",
	"audio3:audio_down_port",
	"onvif0:ip",
	"onvif1:ip",
	"onvif2:ip",
	"onvif3:ip"



};
char * _IP_DEVICE[]=
{
	"audio0:guid",
	"audio0:mac",
	"audio0:ip_addr",
	"audio0:mask",
	"audio0:cmd_port",
	"audio0:audio_down_port",
	"audio1:guid",
	"audio1:mac",
	"audio1:ip_addr",
	"audio1:mask",
	"audio1:cmd_port",
	"audio1:audio_down_port",
	"audio2:guid",
	"audio2:mac",
	"audio2:ip_addr",
	"audio2:mask",
	"audio2:cmd_port",
	"audio2:audio_down_port",
	"audio3:guid",
	"audio3:mac",
	"audio3:ip_addr"
	"audio3:mask"
	"audio3:cmd_port",
	"audio3:audio_down_port"
};
//报警设置
char * _ALARM_PARA[]=
{
	"alarm0:alarmtype",
	"alarm0:enable",
	"alarm0:playback_ch",
	"alarm1:alarmtype",
	"alarm1:enable",
	"alarm1:playback_ch",
	"alarm2:alarmtype",
	"alarm2:enable",
	"alarm2:playback_ch",
	"alarm3:alarmtype",
	"alarm3:enable",
	"alarm3:playback_ch",
	"alarm4:alarmtype",
	"alarm4:enable",
	"alarm4:playback_ch",
	"alarm5:alarmtype",
	"alarm5:enable",
	"alarm5:playback_ch",
	"alarm6:alarmtype",
	"alarm6:enable",
	"alarm6:playback_ch",
	"alarm7:alarmtype",
	"alarm7:enable",
	"alarm7:playback_ch",
	"alarm8:alarmtype",
	"alarm8:enable",
	"alarm8:playback_ch",
	"alarm9:alarmtype",
	"alarm9:enable",
	"alarm9:playback_ch",
	"alarm10:alarmtype",
	"alarm10:enable",
	"alarm10:playback_ch",
	"alarm11:alarmtype",
	"alarm11:enable",
	"alarm11:playback_ch",
	"alarm0:imact",
	"alarm0:ackact",
	"alarm0:selectact",
	"alarm0:rstact",
	"alarm1:imact",
	"alarm1:ackact",
	"alarm1:selectact",
	"alarm1:rstact",
	"alarm2:imact",
	"alarm2:ackact",
	"alarm2:selectact",
	"alarm2:rstact",
	"alarm3:imact",
	"alarm3:ackact",
	"alarm3:selectact",
	"alarm3:rstact",
	"alarm4:imact",
	"alarm4:ackact",
	"alarm4:selectact",
	"alarm4:rstact",
	"alarm5:imact",
	"alarm5:ackact",
	"alarm5:selectact",
	"alarm5:rstact",
	"alarm6:imact",
	"alarm6:ackact",
	"alarm6:selectact",
	"alarm6:rstact",
	"alarm7:imact",
	"alarm7:ackact",
	"alarm7:selectact",
	"alarm7:rstact",
	"alarm8:imact",
	"alarm8:ackact",
	"alarm8:selectact",
	"alarm8:rstact",
	"alarm9:imact",
	"alarm9:ackact",
	"alarm9:selectact",
	"alarm9:rstact",
	"alarm10:imact",
	"alarm10:ackact",
	"alarm10:selectact",
	"alarm10:rstact",
	"alarm11:imact",
	"alarm11:ackact",
	"alarm11:selectact",
	"alarm11:rstact",
	"alarm0:starthour",
	"alarm0:startmin",
	"alarm0:endhour",
	"alarm0:endmin",
	"alarm1:starthour",
	"alarm1:startmin",
	"alarm1:endhour",
	"alarm1:endmin",
	"alarm2:starthour",
	"alarm2:startmin",
	"alarm2:endhour",
	"alarm2:endmin",
	"alarm3:starthour",
	"alarm3:startmin",
	"alarm3:endhour",
	"alarm3:endmin",
	"alarm4:starthour",
	"alarm4:startmin",
	"alarm4:endhour",
	"alarm4:endmin",
	"alarm5:starthour",
	"alarm5:startmin",
	"alarm5:endhour",
	"alarm5:endmin",
	"alarm6:starthour",
	"alarm6:startmin",
	"alarm6:endhour",
	"alarm6:endmin",
	"alarm7:starthour",
	"alarm7:startmin",
	"alarm7:endhour",
	"alarm7:endmin",
	"alarm8:starthour",
	"alarm8:startmin",
	"alarm8:endhour",
	"alarm8:endmin",
	"alarm9:starthour",
	"alarm9:startmin",
	"alarm9:endhour",
	"alarm9:endmin",
	"alarm10:starthour",
	"alarm10:startmin",
	"alarm10:endhour",
	"alarm10:endmin",
	"alarm11:starthour",
	"alarm11:startmin",
	"alarm11:endhour",
	"alarm11:endmin",
	"actiondefine:1",
	"actiondefine:51",
	"actiondefine:2",
	"actiondefine:52",
	"actiondefine:3",
	"actiondefine:53",
	"actiondefine:4",
	"actiondefine:54",
	"actiondefine:5",
	"actiondefine:55",
	"actiondefine:6",
	"actiondefine:56",
	"actiondefine:7",
	"actiondefine:57",
	"actiondefine:8",
	"actiondefine:58",
	"multichannel:enable"
	};
//视频报警设置
char * _VIDEO_ALARM_PARA[]=
{
	"video0:name",
	"video0:enable",
	"video1:name",
	"video1:enable",
	"video2:name",
	"video2:enable",
	"video3:name",
	"video3:enable",
	"video0:inst_place",
	"video1:inst_place",
	"video2:inst_place",
	"video3:inst_place",
	"motion0:imact",
	"motion0:ackact",
	"motion0:selectact",
	"motion0:rstact",
	"motion1:imact",
	"motion1:ackact",
	"motion1:selectact",
	"motion1:rstact",
	"motion2:imact",
	"motion2:ackact",
	"motion2:selectact",
	"motion2:rstact",
	"motion3:imact",
	"motion3:ackact",
	"motion3:selectact",
	"motion3:rstact",
	"video0:motion_alarm",
	"video0:blind_sen",
	"video0:motion_sen",
	"video0:starthour",
	"video0:startmin",
	"video0:endhour",
	"video0:endmin",
	"video1:motion_alarm",
	"video1:blind_sen",
	"video1:motion_sen",
	"video1:starthour",
	"video1:startmin",
	"video1:endhour",
	"video1:endmin",
	"video2:motion_alarm",
	"video2:blind_sen",
	"video2:motion_sen",
	"video2:starthour",
	"video2:startmin",
	"video2:endhour",
	"video2:endmin",
	"video3:motion_alarm",
	"video3:blind_sen",
	"video3:motion_sen",
	"video3:starthour",
	"video3:startmin",
	"video3:endhour",
	"video3:endmin",
	"actiondefine:1",
	"actiondefine:51",
	"actiondefine:2",
	"actiondefine:52",
	"actiondefine:3",
	"actiondefine:53",
	"actiondefine:4",
	"actiondefine:54",
	"actiondefine:5",
	"actiondefine:55",
	"actiondefine:6",
	"actiondefine:56",
	"actiondefine:7",
	"actiondefine:57",
	"actiondefine:8",
	"actiondefine:58"
};
char * _PLAYBACK_CONF[]=
{
	"playback_config:video0",
	"playback_config:video1",
	"playback_config:video2",
	"playback_config:video3",
	"playback_config:video4"
};

char out_str[2000];

int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
        iconv_t cd;
        char **pin = &inbuf;
        char **pout = &outbuf;

        cd = iconv_open(to_charset,from_charset);
        if (cd==0) return -1;
        memset(outbuf,0,outlen);
        if (iconv(cd,pin,&inlen,pout,&outlen)==-1) return -1;
        iconv_close(cd);
        return 0;
}
//GB2312......UNICODE..
int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
        return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
}
int  u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
 		return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}


//替换字符函数，可用于将键值对转换为名称，没用了。。。
char * replaceAll(char * src,char oldChar,char newChar)
{
	char * head=src;
	while(*src!='\0')
	{
		if(*src==oldChar) 
			*src=newChar;
		src++;
	}
	return head;
}
//建立一个json字符串
char * make_json_str(char * in_str,dictionary * ini)
{
	char *gb_str = iniparser_getstring(ini,in_str,"");
	char utf_str[MAX_STR_LENTH]={0};
	g2u(gb_str,strlen(gb_str),utf_str,MAX_STR_LENTH);
	sprintf(out_str,"\"%s\":\"%s\",",in_str,utf_str);
	//gtloginfo("%s",out_str);
	
	return out_str;
}
void make_fast_conf_json()
{

	int i;
	FILE *fp=NULL;
	char *pstr=NULL;	
	char str[5000];
	dictionary		  *ini=NULL;
	ini = iniparser_load_lockfile(RTIMAGE_PARA_FILE,1,&fp);
	
	strcpy(str,"{");
	for(i=0;i<sizeof(_FAST_CONF)/sizeof(int);i++)
	{
	
		pstr=make_json_str(_FAST_CONF[i],ini);

		strcat(str,pstr);

	}	
	str[strlen(str)-1]='}';
	fprintf(cgiOut,"%s",str);
	save_inidict_file(RTIMAGE_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);

}
void make_dev_info_json()
{

	int i;
	FILE *fp=NULL;
	char *pstr=NULL;	
	char str[1000];

	dictionary		  *ini=NULL;
	ini = iniparser_load_lockfile(RTIMAGE_PARA_FILE,1,&fp);




	strcpy(str,"{");
	for(i=0;i<sizeof(_DEV_INFO)/sizeof(int);i++)
	{
	
		pstr=make_json_str(_DEV_INFO[i],ini);

		strcat(str,pstr);

	}
	str[strlen(str)-1]='}';
	fprintf(cgiOut,"%s",str);


	save_inidict_file(RTIMAGE_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);



}
void make_video_audio_para_json()
{
	int i;
	FILE *fp=NULL;
	char *pstr=NULL;	
	char str[2000];

	dictionary		  *ini=NULL;
	ini = iniparser_load_lockfile(RTIMAGE_PARA_FILE,1,&fp);




	strcpy(str,"{");
	for(i=0;i<sizeof(_VIDEO_AUDIO_PARA)/sizeof(int);i++)
	{
	
		pstr=make_json_str(_VIDEO_AUDIO_PARA[i],ini);

		strcat(str,pstr);

	}
	str[strlen(str)-1]='}';
	fprintf(cgiOut,"%s",str);


	save_inidict_file(RTIMAGE_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);


}

void make_alarm_para_json()
{

	int i;
	FILE *fp=NULL;
	char *pstr=NULL;	
	char str[5000];

	dictionary		  *ini=NULL;
	ini = iniparser_load_lockfile(RTIMAGE_PARA_FILE,1,&fp);




	strcpy(str,"{");
	for(i=0;i<sizeof(_ALARM_PARA)/sizeof(int);i++)
	{
	
		pstr=make_json_str(_ALARM_PARA[i],ini);

		strcat(str,pstr);

	}

	str[strlen(str)-1]='}';
	fprintf(cgiOut,"%s",str);


	save_inidict_file(RTIMAGE_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);



}
void make_video_alarm_para_json()
{

	int i;
	FILE *fp=NULL;
	char *pstr=NULL;	
	char str[5000];

	dictionary		  *ini=NULL;
	ini = iniparser_load_lockfile(RTIMAGE_PARA_FILE,1,&fp);




	strcpy(str,"{");
	for(i=0;i<sizeof(_VIDEO_ALARM_PARA)/sizeof(int);i++)
	{
	
		pstr=make_json_str(_VIDEO_ALARM_PARA[i],ini);

		strcat(str,pstr);

	}

	str[strlen(str)-1]='}';
	fprintf(cgiOut,"%s",str);


	save_inidict_file(RTIMAGE_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);



}
void make_playback_config_json()
{

	int i;
	FILE *fp=NULL;
	char *pstr=NULL;	
	char str[5000];

	dictionary		  *ini=NULL;
	ini = iniparser_load_lockfile(RTIMAGE_PARA_FILE,1,&fp);




	strcpy(str,"{");
	for(i=0;i<sizeof(_PLAYBACK_CONF)/sizeof(int);i++)
	{
	
		pstr=make_json_str(_PLAYBACK_CONF[i],ini);

		strcat(str,pstr);

	}

	str[strlen(str)-1]='}';
	fprintf(cgiOut,"%s",str);


	save_inidict_file(RTIMAGE_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);



}


void save_fast_conf_string()
{

	int i,j;
	FILE *fp=NULL;	
	char temp[MAX_STR_LENTH];
	char gb_str[MAX_STR_LENTH];
	dictionary		  *ini=NULL;
	ini = iniparser_load_lockfile(RTIMAGE_PARA_FILE,1,&fp);
	for(i=0;i<sizeof(_FAST_CONF)/sizeof(int);i++)
	{
	   cgiFormString(_FAST_CONF[i], temp, MAX_STR_LENTH);
	   u2g(temp,strlen(temp),gb_str,MAX_STR_LENTH);
	   iniparser_setstr(ini,_FAST_CONF[i],gb_str);
	   if(strncmp(_FAST_CONF[i],"multichannel:enable",19)==0&&gb_str[0]=='1')
	   {
			for(j=0;j<sizeof(_ALARM_PARA)/sizeof(int);j++)
			{
				if(strstr(_ALARM_PARA[j],"playback_ch")!=NULL\
					&&iniparser_getint(ini,_ALARM_PARA[j],4)==4)
				{
					iniparser_setint(ini,_ALARM_PARA[j],-1);
				}
			}


	   }

	}
	save_inidict_file(RTIMAGE_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);
	system(PROGRAME_PATH);

}
void save_dev_info_string()
{

	int i;
	FILE *fp=NULL;	
	char temp[MAX_STR_LENTH];
	dictionary		  *ini=NULL;
	ini = iniparser_load_lockfile(RTIMAGE_PARA_FILE,1,&fp);
	for(i=0;i<sizeof(_DEV_INFO)/sizeof(int);i++)
	{
	   cgiFormString(_DEV_INFO[i], temp, MAX_STR_LENTH);
	   iniparser_setstr(ini,_DEV_INFO[i],temp);

	}
	save_inidict_file(RTIMAGE_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);


}
void save_video_audio_para_string()
{

	int i,j;
	FILE *fp=NULL;	
	char temp[MAX_STR_LENTH];
	char temp_buf[50];
	dictionary		  *ini=NULL;
	ini = iniparser_load_lockfile(RTIMAGE_PARA_FILE,1,&fp);
	for(i=0;i<sizeof(_VIDEO_AUDIO_PARA)/sizeof(int);i++)
	{
	   cgiFormString(_VIDEO_AUDIO_PARA[i], temp, MAX_STR_LENTH);
	   iniparser_setstr(ini,_VIDEO_AUDIO_PARA[i],temp);
	   j=atoi(_VIDEO_AUDIO_PARA[i]+5);


		if(strstr(_VIDEO_AUDIO_PARA[i],"internal")!=NULL){
			if(temp[0]=='1'){
				sprintf(temp_buf,"audio%d:%s",j,_AUDIO_PARM[0]);
				iniparser_setstr(ini,temp_buf,"pcmu");
				sprintf(temp_buf,"audio%d:%s",j,_AUDIO_PARM[1]);
				iniparser_setint(ini,temp_buf,8000);
				sprintf(temp_buf,"audio%d:%s",j,_AUDIO_PARM[2]);
				iniparser_setint(ini,temp_buf,16);
				sprintf(temp_buf,"audio%d:%s",j,_AUDIO_PARM[3]);
				iniparser_setint(ini,temp_buf,1);
					
						
						
			}else{
				sprintf(temp_buf,"audio%d:%s",j,_AUDIO_PARM[0]);
				iniparser_setstr(ini,temp_buf,"aac");
				sprintf(temp_buf,"audio%d:%s",j,_AUDIO_PARM[1]);
				iniparser_setint(ini,temp_buf,16000);
				sprintf(temp_buf,"audio%d:%s",j,_AUDIO_PARM[2]);
				iniparser_setint(ini,temp_buf,16);
				sprintf(temp_buf,"audio%d:%s",j,_AUDIO_PARM[3]);
				iniparser_setint(ini,temp_buf,2);
					
			
					
					
			}
		}
			
	}
	save_inidict_file(RTIMAGE_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);
	system(PROGRAME_PATH);

}

void save_alarm_para_string()
{

	int i;
	FILE *fp=NULL;	
	char gb_str[MAX_STR_LENTH];
	char temp[MAX_STR_LENTH];
	dictionary		  *ini=NULL;
	ini = iniparser_load_lockfile(RTIMAGE_PARA_FILE,1,&fp);
	for(i=0;i<sizeof(_ALARM_PARA)/sizeof(int)-1;i++) //multichannel:enable这一节不保存，因为提交时候是空的
	{
	   cgiFormString(_ALARM_PARA[i], temp, MAX_STR_LENTH);

	   u2g(temp,strlen(temp),gb_str,MAX_STR_LENTH);

	   iniparser_setstr(ini,_ALARM_PARA[i],gb_str);
	   if(strncmp(_ALARM_PARA[i],"actiondefine",12)==0)
	   {
			
			iniparser_setstr(ini,_ALARM_PARA[++i],gb_str);


	   }

	}
	save_inidict_file(RTIMAGE_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);
	system(PROGRAME_PATH);

}
void save_video_alarm_para_string()
{

	int i;
	FILE *fp=NULL;	
	char gb_str[MAX_STR_LENTH];
	char temp[MAX_STR_LENTH];
	dictionary		  *ini=NULL;
	ini = iniparser_load_lockfile(RTIMAGE_PARA_FILE,1,&fp);
	for(i=0;i<sizeof(_VIDEO_ALARM_PARA)/sizeof(int);i++)
	{
	   cgiFormString(_VIDEO_ALARM_PARA[i], temp, MAX_STR_LENTH);

	   u2g(temp,strlen(temp),gb_str,MAX_STR_LENTH);

	   iniparser_setstr(ini,_VIDEO_ALARM_PARA[i],gb_str);
	   if(strncmp(_VIDEO_ALARM_PARA[i],"actiondefine",12)==0)
	   {
			
			iniparser_setstr(ini,_VIDEO_ALARM_PARA[++i],gb_str);


	   }

	}
	save_inidict_file(RTIMAGE_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);
	system(PROGRAME_PATH);

}
void save_playback_config_string()
{

	int i;
	FILE *fp=NULL;	
	char gb_str[MAX_STR_LENTH];
	char temp[MAX_STR_LENTH];
	dictionary		  *ini=NULL;
	ini = iniparser_load_lockfile(RTIMAGE_PARA_FILE,1,&fp);
	for(i=0;i<sizeof(_PLAYBACK_CONF)/sizeof(int);i++)
	{
	   cgiFormString(_PLAYBACK_CONF[i], temp, MAX_STR_LENTH);

	   u2g(temp,strlen(temp),gb_str,MAX_STR_LENTH);

	   iniparser_setstr(ini,_PLAYBACK_CONF[i],gb_str);


	}
	save_inidict_file(RTIMAGE_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);
	system(PROGRAME_PATH);

}


int read_server_para_file(index_type index)
{
	switch((enum PAGE_ENUM)index)
	{
		case FAST_CONF:
			make_fast_conf_json();
		break;
		case DEV_INFO:
			make_dev_info_json();
			break;
		case VIDEO_AUDIO_PARA:
			make_video_audio_para_json();
			break;
		case ALARM_PARA:
			make_alarm_para_json();
			break;
		case VIDEO_ALARM_PARA:
			make_video_alarm_para_json();
			break;
		case PLAYBACK_CONF:
			make_playback_config_json();
			break;
		default:
			break;

	}
	return 0;
}
int set_server_para_file(index_type index)
{

        
 	switch((enum PAGE_ENUM)index)
	{
		case FAST_CONF:
			save_fast_conf_string();
		break;
		case DEV_INFO:
			save_dev_info_string();
			break;
		case VIDEO_AUDIO_PARA:
			save_video_audio_para_string();
			break;
		case ALARM_PARA:
			save_alarm_para_string();
			break;
		case VIDEO_ALARM_PARA:
			save_video_alarm_para_string();
			break;
		case PLAYBACK_CONF:
			save_playback_config_string();
			break;
			
		default:
			break;

	}
	return 0;
}
/*int process_probe_ack(void *data,int len)
{
	FILE *fp=NULL;
	char str[1000];
	strcpy(str,"{");
	strcat(str,data);
	str[strlen(str)-1]='}';
	fprintf(cgiOut,"%s",str);

}
*/
int process_modsocket(mod_socket_cmd_type *modsocket)
{
	int rc=0;
	switch(modsocket->cmd){
		case PROBE_ONVIF_DEV_ACK:

		case PROBE_IP_ACK:
			fprintf(cgiOut,"%s",modsocket->para);
			fflush(cgiOut);
			break;
		default:
			fprintf(cgiOut,"UNKNOW CMD???\n");
			break;
	}
	return rc;



}

int	probe_device(void)
{
	int comfd=-1;
	int len=0;
	int ret=-1;
	int myid=CGI_MAIN_ID;
	int peerid=VIDEOENC_MOD_ID;
	mod_socket_cmd_type cmd;
	char buf[MAX_MOD_SOCKET_CMD_LEN]={0};
	char sourceaddr[100];
	struct timeval tv;
	tv.tv_sec=3;
	tv.tv_usec=0;
	
	comfd=mod_socket_init(0,0);
	
	memset(&cmd,0,sizeof(mod_socket_cmd_type));
	cmd.cmd=PROBE_IP_DEVICE;
	ret=mod_socket_send(comfd,peerid,myid,&cmd,sizeof(mod_socket_cmd_type));
	//set timeout
	if(setsockopt(comfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))<0)
	{
		printf("sokcet option SO_RCVTIMEO not support\n");
		close(comfd);
		return 0;
	
	
	}
	len=mod_socket_recv(comfd,myid,&peerid,&buf,MAX_MOD_SOCKET_CMD_LEN,sourceaddr);
	if(len>0)
	{
		if(myid!=CGI_MAIN_ID)
		{
			printf("It is not my cmd,from id %d\n",peerid);
			return -1;
		
		}
		process_modsocket((mod_socket_cmd_type *)buf);
	
	
	}
	close(comfd);
	return 0;


}
int	probe_onvif_device(void)
{
	int comfd=-1;
	int len=0;
	int ret=-1;
	int myid=CGI_MAIN_ID;
	int peerid=VIDEOENC_MOD_ID;
	mod_socket_cmd_type cmd;
	char buf[MAX_MOD_SOCKET_CMD_LEN]={0};
	char sourceaddr[100];
	struct timeval tv;
	tv.tv_sec=3;
	tv.tv_usec=0;
	comfd=mod_socket_init(0,0);
	
	memset(&cmd,0,sizeof(mod_socket_cmd_type));
	cmd.cmd=PROBE_ONVIF_DEV;
	ret=mod_socket_send(comfd,peerid,myid,&cmd,sizeof(mod_socket_cmd_type));
	//set timeout
	if(setsockopt(comfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))<0)
	{
		printf("sokcet option SO_RCVTIMEO not support\n");
		close(comfd);
		return 0;
	
	
	}
	len=mod_socket_recv(comfd,myid,&peerid,&buf,MAX_MOD_SOCKET_CMD_LEN,sourceaddr);
	if(len>0)
	{
		if(myid!=CGI_MAIN_ID)
		{
			printf("It is not my cmd,from id %d\n",peerid);
			return -1;
		
		}
		process_modsocket((mod_socket_cmd_type *)buf);
	
	
	}
	close(comfd);
	return 0;


}



int disk_operate(index_type index)
{
	//检测值的范围
	if(index < 0 ||index > 5)
	{
		fprintf(cgiOut,"参数错误");
		return -1;
	}
	//全局分区并格式化
	if(PARTITION_FORMAT == index )
	{
		//cgiHeaderContentType("text/html");
		//硬盘重新分区并格式化
		printf("Context-type:text/html; charset=gb2312\r\n\r\n");
		system("killall -9 playback>/dev/null");
		system("/ip1004/initdisk >/dev/null ");
		fprintf(cgiOut,"format finished\r\n");
		fflush(cgiOut);
	
		return 0;
	}
	else if(PARTITION_GET==index)
	{
		//显示分区
		system("/ip1004/www/cgi-bin/show_partition.sh");
		//fprintf(cgiOut,"22222获取分区参数%d",index);
		return 0;
	}else
	{



	//cgiHeaderContentType("text/html");
	
	//单独对一个分区格式化

	printf("Context-type:text/html; charset=gb2312\r\n\r\n");
	char cmd[100]={0};
	system("killall -9 watch_proc>/dev/null");
	fprintf(cgiOut,"watch_proc\r\n\r\n");
	fflush(cgiOut);
	system("killall -15 hdmodule>/dev/null");
	fprintf(cgiOut,"hdmodule\r\n\r\n");
	fflush(cgiOut);
	system("killall -9 playback>/dev/null");
	fprintf(cgiOut,"playback\r\n\r\n");
	fflush(cgiOut);
	system("killall -9 diskman>/dev/null");
	system("killall -9 ftpd>/dev/null");
	system("killall -9 e2fsck>/dev/null");
	system("killall -9 encbox>/dev/null");
	system("killall -9 rtimage>/dev/null");
	system("umount /dev/sd*");
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "/sbin/mke2fs -T ext3 /dev/sda%d -b 4096 -j -L hqdata$QUERY_STRING -m 1 -i 1048576 1>/dev/null 2>/dev/null" ,\
			index);

	system(cmd);
	fprintf(cgiOut,"format finished\r\n");
	fflush(cgiOut);
	system("/ip1004/hwrbt>/dev/null");
	
	}
	return 0;
}


/*获取log 日志的开始和结束时间，以第一行和最后一行的正常时间为准*/
int get_log_time(char *filename,int get_time,char *buf,int buf_size)
{
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	int lines=0,lines1;//文件的总行数
	fp = fopen(filename, "r");
	if (fp == NULL)
		return -1;
	//获取文件总行数
	while ((read = getline(&line, &len, fp)) != -1)
	{
		lines++;
	}
	if (line)
		free(line);
	//printf("lines:%d\n",lines);
	if(GET_START_TIME == get_time)
	{
		fseek(fp, 0, SEEK_SET);
		while((read = getline(&line, &len, fp)) != -1)
		{
			//如果以不是以<开头，则这行就是开始行
			//line[0]!=10防止空行出现
			if(line[0] != '<' && line[0] != 10)
			{
				if(len<buf_size)
					memcpy(buf,line,len);
				else
					memcpy(buf,line,buf_size);
				goto out2;
			}

		}
		if (line)
		{
			free(line);
		}
	}else if(GET_END_TIME == get_time)
	{
		//printf("debug2\n");

		while(lines>0)
		{
			lines1=lines--;
			fseek(fp, 0, SEEK_SET);
			while((read = getline(&line, &len, fp)) != -1)
			{
				lines1--;
				if(lines1==0 && line[0] != '<' && line[0] != 10)
				{
					if(len<buf_size)
						memcpy(buf,line,len);
					else
						memcpy(buf,line,buf_size);
					goto out2;
				}
			}
		}

		return 0;

	}
	else return -3;
out2:
	//fclose(filename);
	return 0;
}
void get_log()
{
#include <unistd.h>
#include <fcntl.h>
//只存第一行和最后一行的前的多少个字符
#define WIDTH  16
	//查找/log/下面gtlog.txt开头的文件数
	int tmp;
	char filename[100];
	char cmd[100];
	char time_start[WIDTH];
	char time_end[WIDTH];
	int now=rand();
	//删除log/下面的符号连接
	system("rm /ip1004/www/log/*");
	//gtlog.txt.*
	for(tmp = 9; tmp >=0; tmp--)
	{
		memset(filename, 0, sizeof(filename));
		sprintf(filename, "/log/gtlog.txt.%d",tmp);
		//printf("1filename:%s\n",filename);
		if(0 == access(filename, F_OK))
		{
			//取第一行和最后一行
			memset(time_start, 0, sizeof(time_start));
			if(get_log_time(filename,GET_START_TIME,time_start, sizeof(time_start) )<0)
			{
				printf("get start time:");
				return ;
			}
			memset(time_end, 0, sizeof(time_end));
			if(get_log_time(filename,GET_END_TIME,time_end, sizeof(time_end) )<0)
			{
				printf("get end time:");
				return ;
			}
			//取字符
			time_start[WIDTH-1]='\0';
			time_end[WIDTH-1]='\0';
			//系统调用创建符号连接
			memset(cmd , 0, sizeof(cmd));
			sprintf(cmd,"ln -s /log/gtlog.txt.%d /ip1004/www/log/%d.txt",tmp,tmp+now);
			//printf(cmd);
			system(cmd);
			//cgi输出
			fprintf(cgiOut,"<a href =\"/log/%d.txt\" target =\"showframe\">%s->%s</a><br />",tmp+now,time_start,time_end);
			//fflush(cgiOut);
		}
	}
	//输出gtlog.txt
	{
		memset(filename, 0, sizeof(filename));
		strcpy(filename, "/log/gtlog.txt");
		if(0 == access(filename, F_OK))
		{
			//取第一行和最后一行
			memset(time_start, 0, sizeof(time_start));
			if(get_log_time(filename,GET_START_TIME,time_start, sizeof(time_start) )<0)
			{
				printf("get start time:");
				return ;
			}

			memset(time_end, 0, sizeof(time_end));
			if(get_log_time(filename,GET_END_TIME,time_end, sizeof(time_end) )<0)
			{
				printf("get end time:");
				return ;
			}
			//取字符
			system("ln -s /log/gtlog.txt /ip1004/www/log/gtlog.txt");
			time_start[WIDTH-1]='\0';
			time_end[WIDTH-1]='\0';
			fprintf(cgiOut,"<a href =\"/log/gtlog.txt\" target =\"showframe\">%s->%s</a><br />",time_start,time_end);
		}

	}
	fflush(cgiOut);
	return ;
}
int set_login_flag(int flag)
{
		int lf;
		char fbuf[11];
		int ret;
		lf=open("/tmp/cgi-login",O_WRONLY|O_CREAT|O_TRUNC,0640);

		if(lf<0)
		{
			printf("open cgi-login error\n");
			return -2;
		}

		else
		{
			sprintf(fbuf,"is_login %d\n",flag);
			ret=write(lf,fbuf,strlen(fbuf));
			if(ret < 0)
			{
				printf("write error!\n");
			}
			close(lf);
		}
		
		return 0;
}
int get_login_flag()
{
		int lf;
		char fbuf[100];
		int ret;
		lf=open("/tmp/cgi-login",O_RDONLY,0640);
		
		if(lf<0)
		{
			printf("open cgi-login error\n");
			return -2;
		}	
		else
		{
			read(lf,fbuf,sizeof(fbuf));
			sscanf(fbuf,"is_login %d\n",&ret);
			close(lf);
		}
		
		return ret;
}

int login_verify(void)
{
	int result_verify;
	char username[MAX_STR_LENTH];
	char passwd[MAX_STR_LENTH];
	cgiFormString("username", username, MAX_STR_LENTH);
	cgiFormString("passwd", passwd, MAX_STR_LENTH);
	result_verify=1;
	if(strcmp(username,"root")==0)
	{
		if(strcmp(passwd,"gtalarm")==0)
		{
			result_verify=0;
			fprintf(cgiOut,"Set-Cookie:login_status=%d; path=/; \r\n ",result_verify);
			
		}
	}
	
	
	cgiHeaderContentType("text/html");
	sprintf(out_str,"\"result\":\"%d\"",result_verify);
	fprintf(cgiOut,"{%s}",out_str);
	return 0;
}

int process_argument(int argc,char **argv)
{
	int oc;
	if(argc<2)
	{
		return 0;
	}
	while((oc=getopt(argc,argv,"hv"))>=0)
    {
    	
    	switch(oc)
        {

			case 'v':
			 
				create_and_lockfile_cgi(CGI_LOCK_FILE); 
				printf("************************version %s*************************\n",VERSION);
			    exit(0);
				break;
			default:
				break;
            }
    }

	printf("*************************************************\n\n\n");
	return 0;
}

int cgiMain(int argc,char *argv[])
{
	process_argument(argc,argv);  
	int status=1;
	char * cookies=NULL;
    char method[MAX_STR_LENTH];
	char page[MAX_STR_LENTH];
	
	bzero(method,0);
	bzero(page,0);
    //cgiHeaderContentType("text/html");
    cgiFormString("method",method,MAX_STR_LENTH);
	cgiFormString("page",page,MAX_STR_LENTH);                         
                                              
	//在此组织json 们
	//todo 区分页面类型 
	cookies=getenv("HTTP_COOKIE");
	if(cookies!=NULL){
		  sscanf(cookies,"login_status=%d",&status);
	}
	if(strcmp(method,"GET_PARA")==0&&status==0){

		
		// 判断是哪个页面发送的
		cgiHeaderContentType("text/html");
		read_server_para_file(atoi(page));


			
	}else if(strcmp(method,"SAVE_PARA")==0&&status==0){	
		// 判断是哪个页面发送的
		cgiHeaderContentType("text/html");
    	set_server_para_file(atoi(page));

	}else if(strcmp(method,"DISK_OPERATE")==0&&status==0){
		// 判断是哪个页面发送的
		//cgiHeaderContentType("text/html");
		disk_operate(atoi(page));

	}else if(strcmp(method,"PROBE_DEVICE")==0&&status==0){
		
		cgiHeaderContentType("text/html");
		probe_device();
	}else if(strcmp(method,"PROBE_ONVIF_DEVICE")==0&&status==0){
		
		cgiHeaderContentType("text/html");
		probe_onvif_device();

	}else if(strcmp(method,"GET_LOG")==0&&status==0){
		// 判断是哪个页面发送的
		cgiHeaderContentType("text/html");
		get_log();

	}else if(strcmp(method,"REBOOT")==0&&status==0){
		//todo para_conv 一转多
		
		system("/ip1004/hwrbt");

	}else if(strcmp(method,"LOGIN")==0){
		
		login_verify();
	}
    return 0;
}
