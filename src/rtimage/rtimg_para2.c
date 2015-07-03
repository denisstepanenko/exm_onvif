/** @file	rtimg_para2.c
 *   @brief 	操作模块参数的函数定义
 *   @date 	2007.03
 */
#include "rtimage2.h"
#include <devinfo.h>

#include "serv_info.h"
#include "audio_pool.h"
#include "net_avstream.h"
#include <mshm.h>
#include <iniparser.h>
#include "debug.h"

static tcprtimg_svr_t     *p_tcprtimg=NULL;      ///<指向参数结构的指针
/** 
 *   @brief     分配tcprtimage2模块状态变量的内存
 *   @return   0表示成功,负值表示失败
 */ 
static int malloc_rtimage_para(void)
{
    int             size=(((sizeof(tcprtimg_svr_t))/512)+1)*512;
    MEM_CORE	*mc=NULL;           ///<因为之后不需要释放、加锁等操作所以使用局部变量就可以了
    printf("malloc size=%d need=%d!!\n",size,sizeof(tcprtimg_svr_t));
    mc=MShmCoreCreate("tcprtimage",get_tcprtimg_stat_key(),size,(void**)&p_tcprtimg);   ///<多分配200byte是为了扩展方便
	
    if(mc==NULL)
    {
	return -ENOMEM;
    } 
    if(p_tcprtimg==NULL)
    {
        printf("p_tcprtimg=NULL!!!\n");
        return -ENOMEM;
    }

    return 0;
}
/** 
 *   @brief     将tcprtimage2模块的所有参数和状态设置为初始值
 *   @return   0表示成功,负值表示失败
 */ 
int init_server_para(void)
{
    int ret;
    ret=malloc_rtimage_para(); //分配watch_tcprt连接的池子
    if(ret<0)
        return ret;
	
    memset((void*)p_tcprtimg,0,sizeof(tcprtimg_svr_t));
    //init_audio_enc_pool();        ///<初始化音频缓冲池变量     //zsk del
    init_net_avstream();           ///<初始化所有网络音视频上行用户的变量
	printf("init finished\n");

    return 0;
}


//zw-add 2011-08-22
static int get_frame_rate(int rate)       //2010-03-21 zsk 修改了之前的case值和return值
{
    switch(rate)
    {
         case 0:
            return 25;
        break;
        case 1:
            return 22;
        break;
        case 2:
            return 20;
        break;
        case 3:
            return 18;
		break;
		case 4:
		        return 16;
		 break;
		    case 5:
		        return 14 ;
		 break;
		 case 6:
		 	return 12;
		  break;
		 case 7:
			return 10;
		  break;
		case 8:
			return 8;
		 break;
		 case 9:
			return 6;
		 break;
		 case 10:
			return 4;
		 break;
        default:
            return 1;
    }
    return 1;
	
}

int check_enctype(dictionary *ini,char *section)
{
	char parastr[100];
	char *cat;
	char *pstr;
	int section_len;
	int enc_type;
	
    section_len=strlen(section);
	memset(parastr,0,sizeof(parastr));
    memcpy(parastr,section,section_len);
    parastr[section_len]=':';
    section_len++;
	parastr[section_len]='\0';
	cat=strncat(parastr,"VideoStand",strlen("VideoStand"));	
	
	pstr=iniparser_getstring(ini,parastr,"MPEG4");
       if((strcasecmp(pstr,"H264")==0)||(strcasecmp(pstr,"h264")==0))
       {
            enc_type=2;
       }
       else if((strcasecmp(pstr,"MJPEG")==0)||(strcasecmp(pstr,"mjpeg")==0))
        {
            enc_type=0;
        }
       else
       {///默认是mpeg4
           enc_type=1;      
        }
	return enc_type;
}
//<----zw-add 20110822


/** 
 *   @brief     从配置文件中读出tcprtimage2模块服务需要的参数
 *   @return   0表示成功,负值表示失败
 */ 
int read_server_para_file(void)
{
	int tmp;
	FILE *fp=NULL;
	
    tcprtimg_svr_t  *p=get_rtimg_para();
    dictionary        *ini=NULL;
    //ini=iniparser_load(RTIMAGE_PARA_FILE);
	ini=iniparser_load_lockfile(RTIMAGE_PARA_FILE,1,&fp);
	if(ini==NULL)
    {
        printf("rtimage cann't parse ini file file [%s]", RTIMAGE_PARA_FILE);
        gtlogerr("rtimage cann't parse ini file file [%s]", RTIMAGE_PARA_FILE);
        return -ENOENT;
    }
	//gtloginfo("__________________________d->n[%d]_____d->size[%d]___________________\n",ini->n,ini->size);
    p->th_timeout=iniparser_getint(ini,"netencoder:timeout",def_svr_timeout);   ///<长时间没有有效数据交换的超时时间

    ///读取丢帧阈值
    p->th_drop_p=iniparser_getint(ini,"netencoder:th_drop",def_th_drop_p);      ///<开始丢弃p帧的阈值
    if((p->th_drop_p+30)>MAX_MAP_BUF_FRAMES)
    	p->th_drop_p=MAX_MAP_BUF_FRAMES-30;
    if(p->th_drop_p<2)
        p->th_drop_p=2;
    p->th_drop_v+=10;
    
    p->audio_pkt_size=iniparser_getint(ini,"netencoder:audio_pkt_size",def_audio_pkt_size);
    p->audio_pkt_size>>=9;
    p->audio_pkt_size<<=9;
    if(p->audio_pkt_size<512)
        p->audio_pkt_size=512;    
    else if(p->audio_pkt_size>8192)
        p->audio_pkt_size=8192;
    
    //force_send_audio 取消了,因为转发服务器现在命令发送正常
    //AudEncMode 取消了,只使用u-pcm,因为音频组件已经可以正常播放u-pcm的数据
    
    p->av_svr_port       = iniparser_getint(ini,"port:image_port",def_rtstream_port);
    p->audio_play_port = iniparser_getint(ini,"port:audio_port",def_rtsnd_port);

	p->ain0_gain = iniparser_getint(ini,"video0:mic_vol",def_mic_gain);
	if(p->ain0_gain>0xf)
		p->ain0_gain=def_mic_gain;
	p->ain1_gain = iniparser_getint(ini,"video1:mic_vol",def_mic_gain);
	if(p->ain1_gain>0xf)
		p->ain1_gain=def_mic_gain;
	p->ain2_gain = iniparser_getint(ini,"video2:mic_vol",def_mic_gain);
	if(p->ain2_gain>0xf)
		p->ain2_gain=def_mic_gain;
	p->ain3_gain = iniparser_getint(ini,"video3:mic_vol",def_mic_gain);
	if(p->ain3_gain>0xf)
		p->ain3_gain=def_mic_gain;
    p->audio_gain = iniparser_getint(ini,"netencoder:audio_vol",def_audio_gain);
	if(p->audio_gain>0xf)
		p->audio_gain=def_audio_gain;

	p->virdev_num = virdev_get_virdev_number();
	    ///读取用户容量
    p->max_wan_usrs=iniparser_getint(ini,"netencoder:max_wan_usr",1);
	p->bitratecon = iniparser_getint(ini,"netencoder:bitratecon",1);
	p->targetbitrate= iniparser_getint(ini,"netencoder:targetbitrate",2000);
	p->maxbitrate= iniparser_getint(ini,"netencoder:maxbitrate",2000);


#if 1
        //zw-add 2011-06-30
        p->pkts_limit=iniparser_getint(ini,"netencoder:pkts_limit",1);
        if(p->pkts_limit==1)
        {
               p->pkts_limit=7;
               iniparser_setint(ini,"netencoder:pkts_limit",p->pkts_limit);
        }
        //默认是200*1024
        p->tcp_max_buff=iniparser_getint(ini,"netencoder:tcp_max_buff",1);
        if(p->tcp_max_buff==1)
        {
            p->tcp_max_buff=200;
            iniparser_setint(ini,"netencoder:tcp_max_buff",p->tcp_max_buff);
        }
        printf("======================================the pkts_limit = %d\n",p->pkts_limit);

	if(p->bitratecon==1)
	{
		if(p->targetbitrate<=2000)
		{
			p->playback_pre=30;
		}
		else
			p->playback_pre=15;
	}
	else if(p->bitratecon==2)
	{
		if(p->maxbitrate<=2000)
			p->playback_pre=30;
		else
			p->playback_pre=15;

	}
	p->playback_dly=iniparser_getint(ini,"playback_config:playback_dly",-1);
	if(p->playback_dly==-1)
	{
		//默认值是30s
		p->playback_dly=30;
		iniparser_setint(ini,"playback_config:playback_dly",p->playback_dly);
	}
	
	if(p->playback_dly>30)
	{
		//最大值是30s
		p->playback_dly=30;		//默认值设为60s，即报警发生时前向回放60s时长录像
		iniparser_setint(ini,"playback_config:playback_dly",p->playback_dly);
	}
	tmp=iniparser_getint(ini,"netencoder:framerate",-1);
	if(tmp>=0)
	{	
		p->frame_rate=get_frame_rate(tmp);
		gtloginfo("读到的帧率为[%d]\n",p->frame_rate);
	}

#endif 

    if(p->max_wan_usrs<0)
        p->max_wan_usrs=0;
    if(p->max_wan_usrs>TCPRTIMG_MAX_AVUSR_NO)
        p->max_wan_usrs=TCPRTIMG_MAX_AVUSR_NO;
    p->max_lan_usrs=iniparser_getint(ini,"netencoder:max_lan_usr",(TCPRTIMG_MAX_AVUSR_NO-p->max_wan_usrs));

    if(p->max_lan_usrs<0)
        p->max_lan_usrs=0;
    if((p->max_lan_usrs+p->max_wan_usrs)>TCPRTIMG_MAX_AVUSR_NO)
        p->max_lan_usrs=TCPRTIMG_MAX_AVUSR_NO-p->max_wan_usrs;
	p->max_wan_usrs *= p->virdev_num;
	p->max_lan_usrs *= p->virdev_num;
    #ifdef USE_ALSA
         p->max_aplay_usrs=(p->max_lan_usrs+p->max_wan_usrs)/p->virdev_num;
         if(p->max_aplay_usrs<3)
         {
            if(TCPRTIMG_MAX_APLAY_NO>=3)
                p->max_aplay_usrs=3;
         }
    #else
        p->max_aplay_usrs=1;
    #endif
	save_inidict_file(RTIMAGE_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);

    return 0;
}

/** 
 *   @brief     返回tcprtimage2模块参数结构指针
 *   @return   参数结构指针
 */ 
tcprtimg_svr_t *get_rtimg_para(void)
{
    return p_tcprtimg;
}

