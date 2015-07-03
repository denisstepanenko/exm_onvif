/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2010-2014, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#include "sys_inc.h"
#include "http.h"
#include "onvif.h"
#include "onvif_probe.h"
#include "onvif_event.h"
#include "onvif_api.h"

/***************************************************************************************/
PPSN_CTX * m_dev_fl;    // device free list
PPSN_CTX * m_dev_ul;    // device used list

/***************************************************************************************/
#ifdef WIN32
void initWinSock()
{
    WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}
#endif

void findDevice(ONVIF_DEVICE * pdevice, ONVIF_DEVICE ** pfound)
{
    ONVIF_DEVICE * p_dev = (ONVIF_DEVICE *) pps_lookup_start(m_dev_ul);
    while (p_dev)
    {
        if (p_dev->binfo.ip == pdevice->binfo.ip /* && strcmp(p_dev->binfo.EndpointReference, pdevice->binfo.EndpointReference) == 0*/)
        {
            break;
        }
        
        p_dev = (ONVIF_DEVICE *) pps_lookup_next(m_dev_ul, p_dev);
    }

    pps_lookup_end(m_dev_ul);

    *pfound = p_dev;  
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

void * getDevInfoThread(void * argv)
{
	char profileToken[ONVIF_TOKEN_LEN];
	ONVIF_PROFILE * p_profile = NULL;
	ONVIF_DEVICE * p_device = (ONVIF_DEVICE *) argv;
    
    GetCapabilities(p_device);

	GetServices(p_device);

    GetDeviceInformation(p_device);

    GetVideoSources(p_device);

    GetAudioSources(p_device);

    GetImagingSettings(p_device);
    
	GetVideoSourceConfigurations(p_device);

	GetAudioSourceConfigurations(p_device);

	GetVideoEncoderConfigurations(p_device);
	
	GetAudioEncoderConfigurations(p_device);

	GetVideoEncoderConfigurationOptions(p_device);

	/* save currrent profile token */
	
	if (p_device->curProfile)
	{
		strcpy(profileToken, p_device->curProfile->token);
	}
	else
	{
		memset(profileToken, 0, sizeof(profileToken));
	}
	
    GetProfiles(p_device);

    GetStreamUris(p_device);

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

	p_device->curProfile = p_profile;
	
	GetNodes(p_device);

	GetPresets(p_device);

	GetConfigurations(p_device);
	
	return NULL;
}

ONVIF_DEVICE * findDeviceByNotify(ONVIF_NOTIFY * p_notify)
{
    ONVIF_DEVICE * dev = (ONVIF_DEVICE *) pps_lookup_start(m_dev_ul);
    while (dev)
    {
        if (strcmp(dev->events.reference_addr, p_notify->reference_addr) == 0)
        {
            break;
        }
        
        dev = (ONVIF_DEVICE *) pps_lookup_next(m_dev_ul, dev);
    }

    pps_lookup_end(m_dev_ul);	

    return dev;
}

/* onvif device probed callback */
void probecb(DEVICE_BINFO * p_res, void * p_data)
{
	ONVIF_DEVICE * p_dev = NULL;	
	ONVIF_DEVICE device;
    memset(&device, 0, sizeof(ONVIF_DEVICE));

    memcpy(&device.binfo, p_res, sizeof(DEVICE_BINFO));
	device.state = 1;

	findDevice(&device, &p_dev);
    if (p_dev == NULL)
    {
		printf("Found device. ip : %s, port : %d\n", p_res->xaddr.host, p_res->xaddr.port);

    	p_dev = addDevice(&device);
    	if (p_dev)
    	{
    		sys_os_create_thread((void *)getDevInfoThread, p_dev);
    	}
    }
    else 
    {
    	if (p_dev->no_res_nums >= 2)
    	{
    		sys_os_create_thread((void *)getDevInfoThread, p_dev);
    	}
    	
    	p_dev->no_res_nums = 0; 
    	p_dev->state = 1;
    }
}

/* onvif event notify callback */
void eventnotifycb(Notify_REQ * p_req, void * p_data)
{
	ONVIF_DEVICE * p_dev;
	ONVIF_NOTIFY * p_notify = p_req->notify;    
         
    p_dev = findDeviceByNotify(p_notify);
    if (p_dev)
    {
        onvif_device_add_notify(p_dev, p_notify);

        p_dev->events.notify_nums += onvif_get_notify_nums(p_notify);
        // max save 100 event notify
        if (p_dev->events.notify_nums > 100)
        {
            p_dev->events.notify_nums -= onvif_device_free_notify(p_dev, p_dev->events.notify_nums - 100);
        }
    }
}

int main(int argc, char* argv[])
{
#ifdef WIN32
    initWinSock();
#endif
	
    sys_buf_init();
    http_msg_buf_fl_init(100);

	// max support 100 devices
    m_dev_fl = pps_ctx_fl_init(100, sizeof(ONVIF_DEVICE), TRUE);
    m_dev_ul = pps_ctx_ul_init(m_dev_fl, TRUE);

	onvif_set_event_notify_cb(eventnotifycb, 0);

	set_probe_cb(probecb, 0);
    start_probe(30);

	for (;;)
		sleep(1);

	return 0;
}

