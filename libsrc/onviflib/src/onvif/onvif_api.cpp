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

#include "onvif_api.h"
#include "onvif_cln.h"
#include "http_srv.h"
#include "onvif_event.h"


BOOL GetCapabilities(ONVIF_DEVICE * p_dev)
{
	GetCapabilities_RES res;
	
    if (!onvif_GetCapabilities(p_dev, NULL, &res))
    {
    	return FALSE;
    }
    else
    {
    	memcpy(&p_dev->capablity, &res.capablity, sizeof(ONVIF_CAP));
    }

    return TRUE;
}

BOOL GetServices(ONVIF_DEVICE * p_dev)
{
	GetServices_REQ req;
	GetServices_RES res;

	req.IncludeCapability = TRUE;
	
    if (!onvif_GetServices(p_dev, &req, &res))
    {
    	return FALSE;
    }
    else
    {
    	memcpy(&p_dev->capablity, &res.capablity, sizeof(ONVIF_CAP));
    }

    return TRUE;
}

BOOL GetDeviceInformation(ONVIF_DEVICE * p_dev)
{
	GetDeviceInformation_RES res;
	
    if (!onvif_GetDeviceInformation(p_dev, NULL, &res))
    {
    	return FALSE;
    }
    else
    {
    	memcpy(&p_dev->dev_info, &res.dev_info, sizeof(DEVICE_BINFO));
    }

	return TRUE;
}

BOOL GetNetworkInterfaces(ONVIF_DEVICE * p_dev)
{
	GetNetworkInterfaces_RES res;
	
    if (!onvif_GetNetworkInterfaces(p_dev, NULL, &res))
    {
    	return FALSE;
    }

    onvif_free_network_interface(&p_dev->network.interfaces);

    p_dev->network.interfaces = res.p_net_inf;

	return TRUE;
}

BOOL GetNTP(ONVIF_DEVICE * p_dev)
{
	GetNTP_RES res;
	
    if (!onvif_GetNTP(p_dev, NULL, &res))
    {
    	return FALSE;
    }

    memcpy(&p_dev->network.ntp, &res.ntp, sizeof(NTP_INFO));

	return TRUE;
}

BOOL GetHostname(ONVIF_DEVICE * p_dev)
{
	GetHostname_RES res;
	
    if (!onvif_GetHostname(p_dev, NULL, &res))
    {
    	return FALSE;
    }

    memcpy(&p_dev->network.hostname, &res.hostname, sizeof(HOSTNAME));

	return TRUE;
}

BOOL GetDNS(ONVIF_DEVICE * p_dev)
{
	GetDNS_RES res;
	
    if (!onvif_GetDNS(p_dev, NULL, &res))
    {
    	return FALSE;
    }
   
    memcpy(&p_dev->network.dns, &res.dns, sizeof(DNS_INFO));

	return TRUE;
}

BOOL GetDynamicDNS(ONVIF_DEVICE * p_dev)
{
	GetDynamicDNS_RES res;
	
	if (p_dev->capablity.device.DynDNS == 0)
	{
		return FALSE;
	}
	
    if (!onvif_GetDynamicDNS(p_dev, NULL, &res))
    {
    	return FALSE;
    }
   
    memcpy(&p_dev->network.ddns, &res.ddns, sizeof(DDNS_INFO));

	return TRUE;
}

BOOL GetNetworkProtocols(ONVIF_DEVICE * p_dev)
{
	GetNetworkProtocols_RES res;
	
    if (!onvif_GetNetworkProtocols(p_dev, NULL, &res))
    {
    	return FALSE;
    }
   
    memcpy(&p_dev->network.protocols, &res.protocols, sizeof(NETPROTOCOL));

	return TRUE;
}

BOOL GetNetworkDefaultGateway(ONVIF_DEVICE * p_dev)
{
	GetNetworkDefaultGateway_RES res;
	
    if (!onvif_GetNetworkDefaultGateway(p_dev, NULL, &res))
    {
    	return FALSE;
    }
   
    memcpy(&p_dev->network.gateway, &res.gateway, sizeof(p_dev->network.gateway[0]) * MAX_GATEWAY);

	return TRUE;
}

BOOL GetDiscoveryMode(ONVIF_DEVICE * p_dev)
{
    GetDiscoveryMode_RES res;
    
    if (!onvif_GetDiscoveryMode(p_dev, NULL, &res))
    {
        return FALSE;
    }

    p_dev->network.discoverable = res.discoverable;

    return TRUE;
}

BOOL GetProfiles(ONVIF_DEVICE * p_dev)
{
    GetProfiles_RES res;
	
    if (!onvif_GetProfiles(p_dev, NULL, &res))
    {
    	return FALSE;
    }
   
    onvif_free_profile(&p_dev->profiles);

    p_dev->profiles = res.p_profile;

	return TRUE;
}

BOOL GetStreamUris(ONVIF_DEVICE * p_dev)
{
    ONVIF_PROFILE * p_profile = p_dev->profiles;

    while (p_profile)
    {
        GetStreamUri_REQ req;
        GetStreamUri_RES res;

        req.protocol = 2;		// rtsp
        req.stream_type = 0;	// rtp-unicast
        strcpy(req.profile_token, p_profile->token);

        if (onvif_GetStreamUri(p_dev, &req, &res))
        {
            strcpy(p_profile->stream_uri, res.Uri);
        }

        p_profile = p_profile->next;
    }

    return TRUE;
}

BOOL GetVideoSourceConfigurations(ONVIF_DEVICE * p_dev)
{
	GetVideoSourceConfigurations_RES res;
	
    if (!onvif_GetVideoSourceConfigurations(p_dev, NULL, &res))
    {
    	return FALSE;
    }
   
    onvif_free_video_source_cfg(&p_dev->video_src_cfg);

    p_dev->video_src_cfg = res.p_v_src_cfg;

	return TRUE;
}

BOOL GetAudioSourceConfigurations(ONVIF_DEVICE * p_dev)
{
	GetAudioSourceConfigurations_RES res;
	
    if (!onvif_GetAudioSourceConfigurations(p_dev, NULL, &res))
    {
    	return FALSE;
    }
   
    onvif_free_audio_source_cfg(&p_dev->audio_src_cfg);

    p_dev->audio_src_cfg = res.p_a_src_cfg;

	return TRUE;
}

BOOL GetVideoEncoderConfigurations(ONVIF_DEVICE * p_dev)
{
	GetVideoEncoderConfigurations_RES res;
	
    if (!onvif_GetVideoEncoderConfigurations(p_dev, NULL, &res))
    {
    	return FALSE;
    }
   
    onvif_free_video_encoder(&p_dev->video_enc);

    p_dev->video_enc = res.p_v_enc;

	return TRUE;
}

BOOL GetAudioEncoderConfigurations(ONVIF_DEVICE * p_dev)
{
	GetAudioEncoderConfigurations_RES res;
	
    if (!onvif_GetAudioEncoderConfigurations(p_dev, NULL, &res))
    {
    	return FALSE;
    }
   
    onvif_free_audio_encoder(&p_dev->audio_enc);

    p_dev->audio_enc = res.p_a_enc;

	return TRUE;
}

BOOL GetVideoEncoderConfigurationOptions(ONVIF_DEVICE * p_dev)
{
	GetVideoEncoderConfigurationOptions_RES res;
	
    if (!onvif_GetVideoEncoderConfigurationOptions(p_dev, NULL, &res))
    {
    	return FALSE;
    }

    memcpy(&p_dev->video_enc_cfg, &res.video_enc_cfg, sizeof(VIDEO_ENC_CFG));

	return TRUE;
}

BOOL GetNodes(ONVIF_DEVICE * p_dev)
{
	GetNodes_RES res;
	
	if (p_dev->capablity.ptz.support == 0)
	{
		return FALSE;
	}
	
    if (!onvif_GetNodes(p_dev, NULL, &res))
    {
    	return FALSE;
    }

    onvif_free_ptz_node(&p_dev->ptznodes);

    p_dev->ptznodes = res.p_ptz_node;

	return TRUE;
}

BOOL GetPresets(ONVIF_DEVICE * p_dev)
{
	ONVIF_PROFILE * p_profile;
	
	if (p_dev->capablity.ptz.support == 0)
	{
		return FALSE;
	}

	p_profile = p_dev->profiles;

    while (p_profile)
    {
        GetPresets_REQ req;
		GetPresets_RES res;

		strcpy(req.profile_token, p_profile->token);
		
	    if (!onvif_GetPresets(p_dev, &req, &res))
	    {
	    	memcpy(p_profile->presets, res.presets, sizeof(PTZ_PRESET) * MAX_PTZ_PRESETS);
	    }

        p_profile = p_profile->next;
    }	

	return TRUE;
}

BOOL GetConfigurations(ONVIF_DEVICE * p_dev)
{
	GetConfigurations_RES res;
	
    if (p_dev->capablity.ptz.support == 0)
	{
		return FALSE;
	}
	
    if (!onvif_GetConfigurations(p_dev, NULL, &res))
    {
    	return FALSE;
    }

    onvif_free_ptz_cfg(&p_dev->ptz_cfg);

    p_dev->ptz_cfg = res.p_ptz_cfg;

	return TRUE;
}

BOOL GetVideoSources(ONVIF_DEVICE * p_dev)
{
    GetVideoSources_RES res;
	
    if (!onvif_GetVideoSources(p_dev, NULL, &res))
    {
    	return FALSE;
    }

    onvif_free_video_source(&p_dev->video_src);

    p_dev->video_src = res.p_v_src;

	return TRUE;
}

BOOL GetAudioSources(ONVIF_DEVICE * p_dev)
{
    GetAudioSources_RES res;
	
    if (!onvif_GetAudioSources(p_dev, NULL, &res))
    {
    	return FALSE;
    }

    onvif_free_audio_source(&p_dev->audio_src);

    p_dev->audio_src = res.p_a_src;

	return TRUE;
}

BOOL GetImagingSettings(ONVIF_DEVICE * p_dev)
{
	VIDEO_SRC * p_v_src;
	
    if (p_dev->capablity.image.support == 0)
	{
		return FALSE;
	}

	p_v_src = p_dev->video_src;

    while (p_v_src)
    {
        GetImagingSettings_REQ req;
		GetImagingSettings_RES res;

		strcpy(req.source_token, p_v_src->token);
		
	    if (!onvif_GetImagingSettings(p_dev, &req, &res))
	    {
	    	memcpy(&p_v_src->img_cfg, &res.img_cfg, sizeof(IMAGE_CFG));
	    }

        p_v_src = p_v_src->next;
    }	

	return TRUE;
}

BOOL Subscribe(ONVIF_DEVICE * p_dev, int index)
{
	int port;
	struct in_addr addr;
	Subscribe_REQ req;
	Subscribe_RES res;
	
    if (p_dev->local_ip == 0)
    {
        p_dev->local_ip = get_default_if_ip();
    }

    port = 30100+index;

    addr.s_addr = p_dev->local_ip;
    
    sprintf(p_dev->events.reference_addr, "http://%s:%d/subscription-%d", inet_ntoa(addr), port, index);
    
    req.init_term_time = p_dev->events.init_term_time;
    strcpy(req.reference_addr, p_dev->events.reference_addr);

    memset(&res, 0, sizeof(res));
    
    if (onvif_Subscribe(p_dev, &req, &res) == FALSE)
    {
        return FALSE;
    }

    strcpy(p_dev->events.producter_addr, res.producter_addr);
    
    if (http_srv_init(&p_dev->events.http_srv, p_dev->local_ip, port, 2) < 0)
    {
        Unsubscribe(p_dev);        
        return FALSE;
    }

    p_dev->events.subscribe = TRUE;
    
    onvif_event_timer_init(p_dev);
    
    return TRUE;
}

BOOL Unsubscribe(ONVIF_DEVICE * p_dev)
{
    onvif_event_timer_deinit(p_dev);
    
    http_srv_deinit(&p_dev->events.http_srv);
    
    onvif_Unsubscribe(p_dev, NULL, NULL);

    p_dev->events.subscribe = FALSE;
    
    return TRUE;
}





