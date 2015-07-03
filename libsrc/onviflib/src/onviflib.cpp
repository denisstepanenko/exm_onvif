#include <stdio.h>
#include "onviflib.h"
#include "sys_inc.h"
#include "http.h"
#include "onvif.h"
#include "onvif_probe.h"
#include "onvif_event.h"
#include "onvif_api.h"
#include "rtsp.h"
#include <algorithm>


#define MAX_DEVICES_NUM 100


void* g_probe_usr_data = NULL;
probe_callback g_probe_usr_cb = NULL;

PPSN_CTX * m_dev_fl;    // device free list
PPSN_CTX * m_dev_ul;    // device used list

/////////////////---------private funs-------///////////////////////////

bool parseUrl(const char * url,char* ip,int* pport)
{
	const char *psz_mrl;
	char *mrl_ptr;
	char *slash, *colon;
	unsigned int hostend, pathbegin;
	
	psz_mrl = url;

	if (!strncmp( psz_mrl, "rtsp://", 7))
	{
		psz_mrl += 7;
	}
	mrl_ptr = strdup(psz_mrl);

	slash = strchr( mrl_ptr, '/' );
	colon = strchr( mrl_ptr, ':' );

	if (slash==0 && colon == 0)
	{
		strcpy(ip, mrl_ptr);
		goto end;
	}

	if ( !slash ) slash = mrl_ptr + strlen(mrl_ptr) + 1;
	if ( !colon ) colon = slash;
	if ( colon > slash ) colon = slash;

	pathbegin = slash - mrl_ptr;
	hostend = colon - mrl_ptr;

	strncpy(ip, mrl_ptr, hostend);

	if ( colon != slash )
	{
 		char buffer[64];
 
 		strncpy( buffer, mrl_ptr+hostend+1, pathbegin-hostend-1 );
 		buffer[pathbegin-hostend-1] = 0;
 		*pport = atoi(buffer);
 		
 		if( *pport < 0 || *pport > 65535 ) *pport = 554;
	}

end:
	if (mrl_ptr)
	{
		free(mrl_ptr);
		mrl_ptr = NULL;
	}

	return true;
}

bool parseAudiosdp(char* audioinfobuf,audio_mediainfo_t* a_info)
{
    char* pfirstblank=NULL,*pfirsttag=NULL,*psectag=NULL;
    char format[10]={0};
    char samplerate[10]={0};
    int srstrlen = 0,i;

    pfirstblank=strchr(audioinfobuf,' ');
    pfirsttag = strchr(audioinfobuf,'/');
    psectag = strchr(pfirsttag+1,'/');
    if(psectag)
        srstrlen = psectag-pfirsttag-1;
    else
        srstrlen = audioinfobuf+strlen(audioinfobuf)-pfirsttag-1; 

    memcpy(format,pfirstblank+1,pfirsttag-pfirstblank-1);
    memcpy(samplerate,pfirsttag+1,srstrlen);

    
    for(i=0;i<strlen(format);++i)
    {
        format[i] = toupper(format[i]);
    }

    printf("audio format from sdp is %s\n",format);
    printf("audio samplerate from sdp is %s\n",samplerate);
    
    if(strstr(format,"PCMA")!=NULL)
    {
        a_info->a_wformat = 6;
    }
    else if(strstr(format,"PCMU")!=NULL)
    {
        a_info->a_wformat = 7;

    }
    else if(strstr(format,"G726")!=NULL)
    {
        a_info->a_wformat = 2;
    }

    a_info->a_sampling = atoi(samplerate);

    return true;
}

int findDeviceSession(unsigned int targetip, stream_type_t type , ONVIF_DEVICE ** pfound,CRtsp **psessionfound)
{
    ONVIF_DEVICE * p_dev = (ONVIF_DEVICE *) pps_lookup_start(m_dev_ul);
    while (p_dev)
    {
        if (p_dev->binfo.ip == targetip)
        {
            if(type==STREAM_MAIN)
                *psessionfound = p_dev->rtsp_main_session;
            else
                *psessionfound = p_dev->rtsp_sec_session;
            
            break;
        }
        
        p_dev = (ONVIF_DEVICE *) pps_lookup_next(m_dev_ul, p_dev);
    }

    pps_lookup_end(m_dev_ul);

    *pfound = p_dev;  

    return 0;
}

int findDevice(unsigned int targetip, ONVIF_DEVICE ** pfound)
{
    ONVIF_DEVICE * p_dev = (ONVIF_DEVICE *) pps_lookup_start(m_dev_ul);
    while (p_dev)
    {
        if (p_dev->binfo.ip == targetip)
        {
            break;
        }
        
        p_dev = (ONVIF_DEVICE *) pps_lookup_next(m_dev_ul, p_dev);
    }

    pps_lookup_end(m_dev_ul);

    *pfound = p_dev;  

    return 0;
}

CRtsp* addDeviceSession(unsigned int targetip,stream_type_t type,ONVIF_DEVICE **pfound)
{
    ONVIF_DEVICE * p_dev = (ONVIF_DEVICE *) pps_lookup_start(m_dev_ul);
    CRtsp* res_session = NULL;
    while (p_dev)
    {
        if (p_dev->binfo.ip == targetip)
        {
            if(type==STREAM_MAIN)
            {
                if(p_dev->rtsp_main_session==NULL)
                    res_session = p_dev->rtsp_main_session = new CRtsp();
                else 
                    res_session = p_dev->rtsp_main_session;
            }
            else
            {
                if(p_dev->rtsp_sec_session==NULL)
                    res_session = p_dev->rtsp_sec_session = new CRtsp();
                else
                    res_session = p_dev->rtsp_sec_session;
            }
            break;
        }
        
        p_dev = (ONVIF_DEVICE *) pps_lookup_next(m_dev_ul, p_dev);
    }

    pps_lookup_end(m_dev_ul);

    *pfound = p_dev;

    return res_session;
}

void delDeviceSession(unsigned int targetip,stream_type_t type)
{
    ONVIF_DEVICE * p_dev = (ONVIF_DEVICE *) pps_lookup_start(m_dev_ul);
    CRtsp * targetsession = NULL;
    while (p_dev)
    {
        if (p_dev->binfo.ip == targetip)
        {
            if(type==STREAM_MAIN)
            {
                targetsession = p_dev->rtsp_main_session;
                p_dev->rtsp_main_session = NULL;
            }
            else
            {
                targetsession = p_dev->rtsp_sec_session;
                p_dev->rtsp_sec_session = NULL;
            }
            break;
        }
        
        p_dev = (ONVIF_DEVICE *) pps_lookup_next(m_dev_ul, p_dev);
    }

    pps_lookup_end(m_dev_ul);
    
    if(targetsession)
    {
        targetsession->rtsp_stop();
        delete targetsession;
    }
}

ONVIF_DEVICE * addDevice(ONVIF_DEVICE * pdevice)
{
    ONVIF_DEVICE * p_dev = (ONVIF_DEVICE *) ppstack_fl_pop(m_dev_fl);
    if (p_dev)
    {
        memcpy(p_dev, pdevice, sizeof(ONVIF_DEVICE));
        p_dev->p_user = 0;
        p_dev->events.init_term_time = 60;
        
        pps_ctx_ul_add(m_dev_ul, p_dev);
    }

    return p_dev;
}

void freeDevice(ONVIF_DEVICE * p_dev)
{
    if (NULL == p_dev)
    {
        return;
    }
    
    onvif_free_profile(&p_dev->profiles);
    onvif_free_video_source(&p_dev->video_src);
    onvif_free_audio_source(&p_dev->audio_src);
    onvif_free_video_source_cfg(&p_dev->video_src_cfg);
    onvif_free_audio_source_cfg(&p_dev->audio_src_cfg);
    onvif_free_video_encoder(&p_dev->video_enc);
    onvif_free_audio_encoder(&p_dev->audio_enc);
    onvif_free_ptz_node(&p_dev->ptznodes);
    onvif_free_ptz_cfg(&p_dev->ptz_cfg);

    if (p_dev->events.subscribe)
    {
        Unsubscribe(p_dev);
    }

    onvif_free_notify(&p_dev->events.notify);
}


void probe_cb_wrapper(DEVICE_BINFO * p_res, void * pdata)
{
    //orgnize DEVICE_BINFO in onviflib
    //use user cb to notify user!
    ONVIF_DEVICE * p_dev = NULL;	
	ONVIF_DEVICE device;

    memset(&device, 0, sizeof(ONVIF_DEVICE));

    memcpy(&device.binfo, p_res, sizeof(DEVICE_BINFO));
	device.state = 1;

	findDevice(device.binfo.ip, &p_dev);
    if (p_dev == NULL)
    {
		printf("Found device. ip : %s, port : %d\n", p_res->xaddr.host, p_res->xaddr.port);

    	p_dev = addDevice(&device);
        
        g_probe_usr_cb(p_res->ip,(device_info_t*)p_res);
    }
    else 
    {
        /*
    	if (p_dev->no_res_nums >= 2)
    	{
    		sys_os_create_thread((void *)getDevInfoThread, p_dev);
    	}
    	
    	p_dev->no_res_nums = 0; 
    	p_dev->state = 1;
        */

        printf("probe find a already exsit dev %s:port:%d\n",p_res->xaddr.host, p_res->xaddr.port);
        //lc still callback to user!
        g_probe_usr_cb(p_res->ip,(device_info_t*)p_res);
    }

}




#ifdef WIN32
void initWinSock()
{
    WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}
#endif

/////////////////////////////////////////////////////////////////


//only should call once!
int onvif_lib_init()
{
    sys_buf_init();
    rtsp_parse_buf_init();
    http_msg_buf_fl_init(100);

    m_dev_fl = pps_ctx_fl_init(MAX_DEVICES_NUM, sizeof(ONVIF_DEVICE), TRUE);
    m_dev_ul = pps_ctx_ul_init(m_dev_fl, TRUE);
    
    //if embeded ,change it
    log_init("onviflib_log.txt");

    return 0;
}

/**
 * @brief    去初始化sdk
 * @param    none
 * @return   0:去初始化成功，<0出错
 */
int onvif_lib_uninit(void)
{
    ONVIF_DEVICE * next_dev;
    ONVIF_DEVICE * dev = (ONVIF_DEVICE *) pps_lookup_start(m_dev_ul);
    while (dev)
    {
        next_dev = (ONVIF_DEVICE *) pps_lookup_next(m_dev_ul, dev);

		// clear auto discovery devices
        // if (!(dev->flags & FLAG_MANUAL)) 
        {
        	pps_ctx_ul_del_unlock(m_dev_ul, dev);
        	ppstack_fl_push(m_dev_fl, dev);
        }

        freeDevice(dev);
        
        dev = next_dev;
    }

    pps_lookup_end(m_dev_ul);	
    return 0;
}

/**
 *   @brief     对ipc进行探测,异步进行
 *   @param     probe_cb 探测结果回调函数
 *   @param     usr_data 用户自定义数据
 *   @return    0表示成功，负值表示失败
 */ 
int onvif_lib_start_probe( probe_callback probe_cb, void* usr_data)
{
    if(probe_cb == NULL)
        return -ERR_ONVIF_SDK_INVALID_ARGS;

    g_probe_usr_data = usr_data;
    g_probe_usr_cb = probe_cb;

    set_probe_cb(probe_cb_wrapper,g_probe_usr_data);
   
    //start probe for once
    return start_probe(10,true);
}



/**
 *   @brief     设置任一设备的用户名，密码,内部会获取设备信息，同步执行
 *   @param     dev_index 设备序列号
 *   @param     usr_name/password 该设备的用户名密码
 *   @return    0表示成功，负值表示失败
 */ 
int onvif_lib_set_auth( dev_handler_t dev_index, const char* usr_name, const char* password)
{
    if((int)dev_index == 0 || usr_name == NULL || password == NULL)
        return -ERR_ONVIF_SDK_INVALID_ARGS;

    ONVIF_DEVICE * p_dev = NULL;	
    findDevice(dev_index,&p_dev);
    if(p_dev == NULL)
        return -ERR_ONVIF_SDK_NO_SUCH_DEV;

    //set password to p_dev
    strncpy(p_dev->username,usr_name,strlen(usr_name));
    strncpy(p_dev->password,password,strlen(password));


    return 0;
}

/**
 *   @brief     获取设备音视频码流格式信息同步执行
 *   @param     dev_index 设备序列号
 *   @param     type 码流类型 主/副
    @param     v_info , a_info ,音视频码流格式信息输出
 *   @return    0表示成功，负值表示失败
 */ 
int onvif_lib_get_av_mediainfo( dev_handler_t dev_index, stream_type_t type, video_mediainfo_t* v_info, audio_mediainfo_t* a_info)
{
    char profileToken[ONVIF_TOKEN_LEN];
	ONVIF_PROFILE * p_profile = NULL;
    ONVIF_PROFILE * p_p=NULL;
    ONVIF_DEVICE * p_device = NULL;	
    CRtsp * p_sdpsession = NULL;
    int succ=-1;
    int audioprofiles = 0;
    char*          target_url = NULL;
    char           ip[32]={0};
    int            port = 554;
    char audioinfobuf[100]={0};

    if(v_info == NULL || a_info == NULL || dev_index == 0)
        return -ERR_ONVIF_SDK_INVALID_ARGS;
   
    findDevice(dev_index,&p_device);
    if(p_device == NULL)
        return -ERR_ONVIF_SDK_NO_SUCH_DEV;
/*
    if(p_device->password == NULL || p_device->username == NULL)
        return -ERR_ONVIF_SDK_NO_AUTH;
    
        */
    //get a/v info from p_dev due to stream_type
    do{
        if(!GetCapabilities(p_device))
        {
            printf("get capacity failed!\n");
            succ -= 1;
        }

        if(!GetServices(p_device))
        {
            printf("get services failed!\n");
            succ -= 1;
        }

        if(!GetDeviceInformation(p_device))
        {
            printf("get dev information failed!\n");
            succ -= 1;
        }

        if(!GetVideoSources(p_device))
        {
            printf("get videosources failed!\n");
            succ -= 1;
        }

        if(!GetAudioSources(p_device))
        {
            printf("get audiosources failed!\n");
            succ -= 1;
        }

        if(!GetImagingSettings(p_device))
        {
            printf("get image setting failed!\n");
            succ -= 1;
        }

        if(!GetVideoSourceConfigurations(p_device))
        {
            printf("get video source conf failed!\n");
            succ -= 1;
        }

        if(!GetAudioSourceConfigurations(p_device))
        {
            succ -= 1;
            printf("get audio source conf failed!\n");
        }

        if(!GetVideoEncoderConfigurations(p_device))
        {
            printf("get video enc conf failed!\n");
            succ -= 1;
            break;
        }

        if(!GetAudioEncoderConfigurations(p_device))
        {
            succ -= 1;
            printf("get audio enc conf failed!\n");
        }

        if(!GetVideoEncoderConfigurationOptions(p_device))
        {
            printf("get video enc conf option failed!\n");
            succ -= 1;
            break;
        }

        /* save currrent profile token */

        if (p_device->curProfile)
        {
            strcpy(profileToken, p_device->curProfile->token);
        }
        else
        {
            memset(profileToken, 0, sizeof(profileToken));
        }

        if(!GetProfiles(p_device))
        {
            printf("get profiles failed!\n");
            succ -= 1;
            break;
        }

        if(!GetStreamUris(p_device))
        {
            printf("get stream uris failed!\n");
            succ -= 1;
            break;
        }

        /* resume current profile */	

        if (profileToken[0] != '\0')
        {
            p_profile = onvif_find_profile(p_device, profileToken);
        }

        if (NULL == p_profile)
        {
            ONVIF_PROFILE * p_tmp = p_device->profiles;
            while (p_tmp)
            {
                // todo: now support H264, will be add MPEG4 and MJPEG
                if (p_tmp->video_enc && p_tmp->video_enc->encoding == VIDEO_ENCODING_H264)
                {
                    p_profile = p_tmp;
                    break;
                }

                p_tmp = p_tmp->next;
            }
        }

        ONVIF_PROFILE* p_tmp1 = p_device->profiles;
        while(p_tmp1)
        {
            if(p_tmp1->audio_enc)
                audioprofiles+=1;
            p_tmp1 = p_tmp1->next;
        }
        printf("got %d audio profiles!\n",audioprofiles);

        p_device->curProfile = p_profile;

        //lc if we want MAIN stream profile ,use Profile0 , else use next profile
        if(type == STREAM_MAIN)
            p_p = p_device->profiles;
        else
            p_p = p_device->profiles->next;

        if(p_p->audio_enc)
        {
            if(p_p->audio_enc->encoding==AUDIO_ENCODING_G711)
                a_info->a_wformat=7;
            else if(p_p->audio_enc->encoding==AUDIO_ENCODING_AAC)
                a_info->a_wformat = 255;

            if(p_p->audio_enc->bitrate>1000)
                a_info->a_bitrate = p_p->audio_enc->bitrate/1000;
            else
                a_info->a_bitrate = p_p->audio_enc->bitrate;
            a_info->a_bits = 0;
            a_info->a_channel = 0;
			a_info->a_sampling = p_p->audio_enc->sample_rate;
            a_info->a_frate = 0;
            a_info->a_nr_frame = 0;
        }
        
        if(p_p->video_enc)
        {
            v_info->format = p_p->video_enc->encoding;
            switch(p_p->video_enc->height)
            {
                case 288:
                    v_info->ratio = RATIO_CIF_PAL;
                    break;
                case 576:
                    v_info->ratio = RATIO_D1_PAL;
                    break;
                case 720:
                    v_info->ratio = RATIO_720P;
                    break;
                case 1080:
                    v_info->ratio = RATIO_1080P;
                    break;
                default:
                    v_info->ratio = RATIO_ERR;
                    printf("err video enc height %d\n",p_device->curProfile->video_enc->height);
            }
            v_info->framerate = p_p->video_enc->framerate_limit;
            v_info->en_interval = p_p->video_enc->encoding_interval;
            v_info->bitrate = p_p->video_enc->bitrate_limit;

        }
            
        if(!GetNodes(p_device))
        {
            printf("get nodes failed!\n");
            succ -= 1;
        }

        if(!GetPresets(p_device))
        {
            printf("get presets failed!\n");
            succ -= 1;
        }

        if(!GetConfigurations(p_device))
        {
            printf("get configurations failed!\n");
            succ -= 1;
        }
/*
        target_url = p_p->stream_uri;
        parseUrl(target_url,ip,&port);

        p_sdpsession = new CRtsp();
        p_sdpsession->rtsp_getsdp_start(target_url,ip,port,audioinfobuf,(const char*)p_device->username,(const char*)p_device->password);

        printf("get audio sdp info:%s\n",audioinfobuf);
        
        if(audioinfobuf[0] != '\0')
            parseAudiosdp(audioinfobuf,a_info);


        delete p_sdpsession;
*/
        succ = 0;
    }while(false);

    //printf("stream uri is %s\n",p_device->profiles->stream_uri);

    return succ;
}


/**
 *   @brief     请求设备实时音视频数据,rtsp过程同步，rtp过程异步
 *   @param     dev_index,设备序列号
 *   @param     type 设备码流类型 主/副
 *   @param     vcb,acb 音视频数据，回调函数
 *   @param     usr_data 用户自定义数据
 *   @return    0表示成功，负值表示失败
 */ 
int onvif_lib_rtsp_start( dev_handler_t dev_index, stream_type_t type, video_Callback vcb, audio_Callback acb, notify_Callback ncb, void* usr_data)
{
    ONVIF_DEVICE * p_device = NULL;	
    char*          target_url = NULL;
    char           ip[32]={0};
    int            port = 554;

    if(dev_index == 0 || ncb == NULL)
        return -ERR_ONVIF_SDK_INVALID_ARGS;
        
    //1.set usr_data to found dev
    //2.set vcb and acb to found dev
    CRtsp* session = addDeviceSession(dev_index,type,&p_device);
    if(session==NULL || p_device==NULL)
    {
        if(p_device==NULL)
            return -ERR_ONVIF_SDK_NO_SUCH_DEV;
        else
            return -ERR_ONVIF_SDK_SESSION_EXIST;
    }
    
    session->set_notify_cb(ncb,usr_data);
    session->set_video_cb(vcb);
    session->set_audio_cb(acb);

    if(type==STREAM_MAIN)
        target_url = p_device->profiles->stream_uri;
    else
    {
        if(p_device->profiles->next)
            target_url = p_device->profiles->next->stream_uri;
        else
        {
            target_url = NULL;
            return -1;
        }
    }

    parseUrl(target_url,ip,&port);
   
    if(session->rtsp_start(target_url,ip,port,(const char*)p_device->username,(const char*)p_device->password))
        return 0;
    
    return -1;
}


/**
 * @brief     停止实时音视频,rtsp过程同步
 * @param     dev_index,设备序列号
 * @param     type 设备码流类型 主/副
 * @return    是否成功停止请求，0正常 <0 异常
 */
int onvif_lib_rtsp_stop( dev_handler_t dev_index, stream_type_t type)
{
    ONVIF_DEVICE * p_device = NULL;	
    CRtsp*         p_session = NULL;
    
    if(dev_index == 0)
        return -ERR_ONVIF_SDK_INVALID_ARGS;

    delDeviceSession(dev_index,type);

    return 0; 
}

