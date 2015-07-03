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
#include "onvif.h"


/***************************************************************************************/
OVFACTS g_onvif_acts[] = 
{
	// device
	{eGetCapabilities, "http://www.onvif.org/ver10/device/wsdl/GetCapabilities"},
	{eGetServices, "http://www.onvif.org/ver10/device/wsdl/GetServices"},
	{eGetDeviceInformation, "http://www.onvif.org/ver10/device/wsdl/GetDeviceInformation"},
	{eGetNetworkInterfaces, "http://www.onvif.org/ver10/device/wsdl/GetNetworkInterfaces"},	
	{eSetNetworkInterfaces, "http://www.onvif.org/ver10/device/wsdl/SetNetworkInterfaces"},
	{eGetNTP, "http://www.onvif.org/ver10/device/wsdl/GetNTP"},
	{eSetNTP, "http://www.onvif.org/ver10/device/wsdl/SetNTP"},
	{eGetHostname, "http://www.onvif.org/ver10/device/wsdl/GetHostname"},
	{eSetHostname, "http://www.onvif.org/ver10/device/wsdl/SetHostname"},
	{eSetHostnameFromDHCP, "http://www.onvif.org/ver10/device/wsdl/SetHostnameFromDHCP"},
	{eGetDNS, "http://www.onvif.org/ver10/device/wsdl/GetDNS"},
	{eSetDNS, "http://www.onvif.org/ver10/device/wsdl/SetDNS"},
	{eGetDynamicDNS, "http://www.onvif.org/ver10/device/wsdl/GetDynamicDNS"},
	{eSetDynamicDNS, "http://www.onvif.org/ver10/device/wsdl/SetDynamicDNS"},
	{eGetNetworkProtocols, "http://www.onvif.org/ver10/device/wsdl/GetNetworkProtocols"},
	{eSetNetworkProtocols, "http://www.onvif.org/ver10/device/wsdl/SetNetworkProtocols"},
	{eGetDiscoveryMode, "http://www.onvif.org/ver10/device/wsdl/GetDiscoveryMode"},
	{eSetDiscoveryMode, "http://www.onvif.org/ver10/device/wsdl/SetDiscoveryMode"},
	{eGetNetworkDefaultGateway, "http://www.onvif.org/ver10/device/wsdl/GetNetworkDefaultGateway"},
	{eSetNetworkDefaultGateway, "http://www.onvif.org/ver10/device/wsdl/SetNetworkDefaultGateway"},
	{eGetSystemDateAndTime, "http://www.onvif.org/ver10/device/wsdl/GetSystemDateAndTime"},
	{eSetSystemDateAndTime, "http://www.onvif.org/ver10/device/wsdl/SetSystemDateAndTime"},
	{eSystemReboot, "http://www.onvif.org/ver10/device/wsdl/SystemReboot"},
	{eSetSystemFactoryDefault, "http://www.onvif.org/ver10/device/wsdl/SetSystemFactoryDefault"},
	{eGetSystemLog, "http://www.onvif.org/ver10/device/wsdl/GetSystemLog"},
	{eGetScopes, "http://www.onvif.org/ver10/device/wsdl/GetScopes"},
	{eSetScopes, "http://www.onvif.org/ver10/device/wsdl/SetScopes"},
	{eAddScopes, "http://www.onvif.org/ver10/device/wsdl/AddScopes"},
	{eRemoveScopes, "http://www.onvif.org/ver10/device/wsdl/RemoveScopes"},
	// end of device

	// media
	{eGetVideoSources, "http://www.onvif.org/ver10/media/wsdl/GetVideoSources"},
	{eGetAudioSources, "http://www.onvif.org/ver10/media/wsdl/GetAudioSources"},
	{eCreateProfile, "http://www.onvif.org/ver10/media/wsdl/CreateProfile"},
	{eGetProfile, "http://www.onvif.org/ver10/media/wsdl/GetProfile"},
	{eGetProfiles, "http://www.onvif.org/ver10/media/wsdl/GetProfiles"},
	{eAddVideoEncoderConfiguration, "http://www.onvif.org/ver10/media/wsdl/AddVideoEncoderConfiguration"},
	{eAddVideoSourceConfiguration, "http://www.onvif.org/ver10/media/wsdl/AddVideoSourceConfiguration"},
	{eAddAudioEncoderConfiguration, "http://www.onvif.org/ver10/media/wsdl/AddAudioEncoderConfiguration"},
	{eAddAudioSourceConfiguration, "http://www.onvif.org/ver10/media/wsdl/AddAudioSourceConfiguration"},
	{eAddPTZConfiguration, "http://www.onvif.org/ver10/media/wsdl/AddPTZConfiguration"},
	{eRemoveVideoEncoderConfiguration, "http://www.onvif.org/ver10/media/wsdl/RemoveVideoEncoderConfiguration"},
	{eRemoveVideoSourceConfiguration, "http://www.onvif.org/ver10/media/wsdl/RemoveVideoSourceConfiguration"},
	{eRemoveAudioEncoderConfiguration, "http://www.onvif.org/ver10/media/wsdl/RemoveAudioEncoderConfiguration"},
	{eRemoveAudioSourceConfiguration, "http://www.onvif.org/ver10/media/wsdl/RemoveAudioSourceConfiguration"},
	{eRemovePTZConfiguration, "http://www.onvif.org/ver10/media/wsdl/RemovePTZConfiguration"},
	{eDeleteProfile, "http://www.onvif.org/ver10/media/wsdl/DeleteProfile"},
	{eGetVideoSourceConfigurations, "http://www.onvif.org/ver10/media/wsdl/GetVideoSourceConfigurations"},
	{eGetVideoEncoderConfigurations, "http://www.onvif.org/ver10/media/wsdl/GetVideoEncoderConfigurations"},
	{eGetAudioSourceConfigurations, "http://www.onvif.org/ver10/media/wsdl/GetAudioSourceConfigurations"},
	{eGetAudioEncoderConfigurations, "http://www.onvif.org/ver10/media/wsdl/GetAudioEncoderConfigurations"},
	{eGetVideoSourceConfiguration, "http://www.onvif.org/ver10/media/wsdl/GetVideoSourceConfiguration"},
	{eGetVideoEncoderConfiguration, "http://www.onvif.org/ver10/media/wsdl/GetVideoEncoderConfiguration"},
	{eGetAudioSourceConfiguration, "http://www.onvif.org/ver10/media/wsdl/GetAudioSourceConfiguration"},
	{eGetAudioEncoderConfiguration, "http://www.onvif.org/ver10/media/wsdl/GetAudioEncoderConfiguration"},
	{eSetVideoSourceConfiguration, "http://www.onvif.org/ver10/media/wsdl/SetVideoSourceConfiguration"},
	{eSetVideoEncoderConfiguration, "http://www.onvif.org/ver10/media/wsdl/SetVideoEncoderConfiguration"},
	{eSetAudioSourceConfiguration, "http://www.onvif.org/ver10/media/wsdl/SetAudioSourceConfiguration"},
	{eSetAudioEncoderConfiguration, "http://www.onvif.org/ver10/media/wsdl/SetAudioEncoderConfiguration"},
	{eGetVideoSourceConfigurationOptions, "http://www.onvif.org/ver10/media/wsdl/GetVideoSourceConfigurationOptions"},
	{eGetVideoEncoderConfigurationOptions, "http://www.onvif.org/ver10/media/wsdl/GetVideoEncoderConfigurationOptions"},
	{eGetAudioSourceConfigurationOptions, "http://www.onvif.org/ver10/media/wsdl/GetAudioSourceConfigurationOptions"},
	{eGetAudioEncoderConfigurationOptions, "http://www.onvif.org/ver10/media/wsdl/GetAudioEncoderConfigurationOptions"},	
	{eGetStreamUri,	"http://www.onvif.org/ver10/media/wsdl/GetStreamUri"},	
	{eSetSynchronizationPoint, "http://www.onvif.org/ver10/media/wsdl/SetSynchronizationPoint"},
	{eGetSnapshotUri, "http://www.onvif.org/ver10/media/wsdl/GetSnapshotUri"},
	// end of media
	
	// PTZ
	{eGetNodes, "http://www.onvif.org/ver20/ptz/wsdl/GetNodes"},
	{eGetNode, "http://www.onvif.org/ver20/ptz/wsdl/GetNode"},
	{eGetPresets, "http://www.onvif.org/ver20/ptz/wsdl/GetPresets"},
	{eSetPreset, "http://www.onvif.org/ver20/ptz/wsdl/SetPreset"},
	{eRemovePreset, "http://www.onvif.org/ver20/ptz/wsdl/RemovePreset"},
	{eGotoPreset, "http://www.onvif.org/ver20/ptz/wsdl/GotoPreset"},
	{eGotoHomePosition, "http://www.onvif.org/ver20/ptz/wsdl/GotoHomePosition"},
	{eSetHomePosition, "http://www.onvif.org/ver20/ptz/wsdl/SetHomePosition"},
	{eGetStatus, "http://www.onvif.org/ver20/ptz/wsdl/GetStatus"},
	{eContinuousMove, "http://www.onvif.org/ver20/ptz/wsdl/ContinuousMove"},	
	{eRelativeMove, "http://www.onvif.org/ver20/ptz/wsdl/RelativeMove"},
	{eAbsoluteMove, "http://www.onvif.org/ver20/ptz/wsdl/AbsoluteMove"},
	{ePTZStop, "http://www.onvif.org/ver20/ptz/wsdl/Stop"},
	{eGetConfigurations, "http://www.onvif.org/ver20/ptz/wsdl/GetConfigurations"},
	{eGetConfiguration, "http://www.onvif.org/ver20/ptz/wsdl/GetConfiguration"},
	{eSetConfiguration, "http://www.onvif.org/ver20/ptz/wsdl/SetConfiguration"},
	{eGetConfigurationOptions, "http://www.onvif.org/ver20/ptz/wsdl/GetConfigurationOptions"},
	// end of PTZ

	// event
	{eGetEventProperties, "http://www.onvif.org/ver10/events/wsdl/EventPortType/GetEventPropertiesRequest"},
	{eRenew, "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/RenewRequest"},
	{eUnsubscribe, "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/UnsubscribeRequest"},
	{eSubscribe, "http://docs.oasis-open.org/wsn/bw-2/NotificationProducer/SubscribeRequest"},
	// end of event

	// image
	{eGetImagingSettings, "http://www.onvif.org/ver20/imaging/wsdl/GetImagingSettings"},
	{eSetImagingSettings, "http://www.onvif.org/ver20/imaging/wsdl/SetImagingSettings"},
	{eGetOptions, "http://www.onvif.org/ver20/imaging/wsdl/GetOptions"},	
	// end of image

	{eGetReplayUri, "http://www.onvif.org/ver10/replay/wsdl/GetReplayUri"},

	{eGetRecordingSummary, "http://www.onvif.org/ver10/search/wsdl/GetRecordingSummary"},
	{eGetRecordingInformation, "http://www.onvif.org/ver10/search/wsdl/GetRecordingInformation"},
	{eGetMediaAttributes, "http://www.onvif.org/ver10/search/wsdl/GetMediaAttributes"},
	{eFindRecordings, "http://www.onvif.org/ver10/search/wsdl/FindRecordings"},
	{eGetRecordingSearchResults, "http://www.onvif.org/ver10/search/wsdl/GetRecordingSearchResults"},
	{eFindEvents, "http://www.onvif.org/ver10/search/wsdl/FindEvents"},
	{eGetEventSearchResults, "http://www.onvif.org/ver10/search/wsdl/GetEventSearchResults"},
	{eGetSearchState, "http://www.onvif.org/ver10/search/wsdl/GetSearchState"},
	{eEndSearch, "http://www.onvif.org/ver10/search/wsdl/EndSearch"},
};


/***************************************************************************************/
OVFACTS * onvif_find_action_by_type(eOnvifAction type)
{
	int i;
	
	if (type < eActionNull || type >= eActionMax)
	{
		return NULL;
	}
	
	for (i=0; i<(sizeof(g_onvif_acts)/sizeof(OVFACTS)); i++)
	{
		if (g_onvif_acts[i].type == type)
		{
			return &g_onvif_acts[i];
		}	
	}

	return NULL;
}


ONVIF_NET_INF * onvif_add_network_interface(ONVIF_NET_INF ** p_net_inf)
{
	ONVIF_NET_INF * p_tmp;
	ONVIF_NET_INF * p_new_net_inf = (ONVIF_NET_INF *) malloc(sizeof(ONVIF_NET_INF));
	if (NULL == p_new_net_inf)
	{
		return NULL;
	}

	memset(p_new_net_inf, 0, sizeof(ONVIF_NET_INF));

	p_tmp = *p_net_inf;
	if (NULL == p_tmp)
	{
		*p_net_inf = p_new_net_inf;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_net_inf;
	}
	
	return p_new_net_inf;
}

void onvif_free_network_interface(ONVIF_NET_INF ** p_net_inf)
{
	ONVIF_NET_INF * p_next;
	ONVIF_NET_INF * p_tmp = *p_net_inf;

	while (p_tmp)
	{
		p_next = p_tmp->next;

		free(p_tmp);
		p_tmp = p_next;
	}

	*p_net_inf = NULL;
}

ONVIF_NET_INF * onvif_find_network_interface(ONVIF_DEVICE * p_dev, const char * token)
{
    ONVIF_NET_INF * p_net_inf = p_dev->network.interfaces;
    
    while (p_net_inf)
	{
	    if (strcmp(p_net_inf->token, token) == 0)
	    {
	        break;
	    }
	    
		p_net_inf = p_net_inf->next;
	}

	return p_net_inf;
}

VIDEO_SRC * onvif_add_video_source(VIDEO_SRC ** p_v_src)
{
	VIDEO_SRC * p_tmp;
    VIDEO_SRC * p_new_v_src = (VIDEO_SRC *) malloc(sizeof(VIDEO_SRC));
	if (NULL == p_new_v_src)
	{
		return NULL;
	}

	memset(p_new_v_src, 0, sizeof(VIDEO_SRC));

	p_tmp = *p_v_src;
	if (NULL == p_tmp)
	{
		*p_v_src = p_new_v_src;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_v_src;
	}
	
	return p_new_v_src;
}

void onvif_free_video_source(VIDEO_SRC ** p_v_src)
{
    VIDEO_SRC * p_next;
	VIDEO_SRC * p_tmp = *p_v_src;

	while (p_tmp)
	{
		p_next = p_tmp->next;

		free(p_tmp);
		p_tmp = p_next;
	}

	*p_v_src = NULL;
}

VIDEO_SRC * onvif_find_video_source(ONVIF_DEVICE * p_dev, const char * token)
{
    VIDEO_SRC * p_v_src = p_dev->video_src;
    
    while (p_v_src)
	{
	    if (strcmp(p_v_src->token, token) == 0)
	    {
	        break;
	    }
	    
		p_v_src = p_v_src->next;
	}

	return p_v_src;
}

AUDIO_SRC * onvif_add_audio_source(AUDIO_SRC ** p_a_src)
{
	AUDIO_SRC * p_tmp;
    AUDIO_SRC * p_new_a_src = (AUDIO_SRC *) malloc(sizeof(AUDIO_SRC));
	if (NULL == p_new_a_src)
	{
		return NULL;
	}

	memset(p_new_a_src, 0, sizeof(AUDIO_SRC));

	p_tmp = *p_a_src;
	if (NULL == p_tmp)
	{
		*p_a_src = p_new_a_src;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_a_src;
	}
	
	return p_new_a_src;
}

void onvif_free_audio_source(AUDIO_SRC ** p_a_src)
{
    AUDIO_SRC * p_next;
	AUDIO_SRC * p_tmp = *p_a_src;

	while (p_tmp)
	{
		p_next = p_tmp->next;

		free(p_tmp);
		p_tmp = p_next;
	}

	*p_a_src = NULL;
}

AUDIO_SRC * onvif_find_audio_source(ONVIF_DEVICE * p_dev, const char * token)
{
    AUDIO_SRC * p_a_src = p_dev->audio_src;
    
    while (p_a_src)
	{
	    if (strcmp(p_a_src->token, token) == 0)
	    {
	        break;
	    }
	    
		p_a_src = p_a_src->next;
	}

	return p_a_src;
}

VIDEO_SRC_CFG * onvif_add_video_source_cfg(VIDEO_SRC_CFG ** p_v_src_cfg)
{
	VIDEO_SRC_CFG * p_tmp;
	VIDEO_SRC_CFG * p_new_v_src_cfg = (VIDEO_SRC_CFG *) malloc(sizeof(VIDEO_SRC_CFG));
	if (NULL == p_new_v_src_cfg)
	{
		return NULL;
	}

	memset(p_new_v_src_cfg, 0, sizeof(VIDEO_SRC_CFG));

	p_tmp = *p_v_src_cfg;
	if (NULL == p_tmp)
	{
		*p_v_src_cfg = p_new_v_src_cfg;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_v_src_cfg;
	}
	
	return p_new_v_src_cfg;
}

void onvif_free_video_source_cfg(VIDEO_SRC_CFG ** p_v_src_cfg)
{
	VIDEO_SRC_CFG * p_next;
	VIDEO_SRC_CFG * p_tmp = *p_v_src_cfg;

	while (p_tmp)
	{
		p_next = p_tmp->next;

		free(p_tmp);
		p_tmp = p_next;
	}

	*p_v_src_cfg = NULL;
}

VIDEO_SRC_CFG * onvif_find_video_source_cfg(ONVIF_DEVICE * p_dev, const char * token)
{
    VIDEO_SRC_CFG * p_v_src_cfg = p_dev->video_src_cfg;
    
    while (p_v_src_cfg)
	{
	    if (strcmp(p_v_src_cfg->token, token) == 0)
	    {
	        break;
	    }
	    
		p_v_src_cfg = p_v_src_cfg->next;
	}

	return p_v_src_cfg;
}

AUDIO_SRC_CFG * onvif_add_audio_source_cfg(AUDIO_SRC_CFG ** p_a_src_cfg)
{
	AUDIO_SRC_CFG * p_tmp;
	AUDIO_SRC_CFG * p_new_a_src_cfg = (AUDIO_SRC_CFG *) malloc(sizeof(AUDIO_SRC_CFG));
	if (NULL == p_new_a_src_cfg)
	{
		return NULL;
	}

	memset(p_new_a_src_cfg, 0, sizeof(AUDIO_SRC_CFG));

	p_tmp = *p_a_src_cfg;
	if (NULL == p_tmp)
	{
		*p_a_src_cfg = p_new_a_src_cfg;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_a_src_cfg;
	}
	
	return p_new_a_src_cfg;
}

void onvif_free_audio_source_cfg(AUDIO_SRC_CFG ** p_a_src_cfg)
{
	AUDIO_SRC_CFG * p_next;
	AUDIO_SRC_CFG * p_tmp = *p_a_src_cfg;

	while (p_tmp)
	{
		p_next = p_tmp->next;

		free(p_tmp);
		p_tmp = p_next;
	}

	*p_a_src_cfg = NULL;
}

AUDIO_SRC_CFG * onvif_find_audio_source_cfg(ONVIF_DEVICE * p_dev, const char * token)
{
    AUDIO_SRC_CFG * p_a_src_cfg = p_dev->audio_src_cfg;
    
    while (p_a_src_cfg)
	{
	    if (strcmp(p_a_src_cfg->token, token) == 0)
	    {
	        break;
	    }
	    
		p_a_src_cfg = p_a_src_cfg->next;
	}

	return p_a_src_cfg;
}

VIDEO_ENCODER * onvif_add_video_encoder(VIDEO_ENCODER ** p_v_enc)
{
	VIDEO_ENCODER * p_tmp;
	VIDEO_ENCODER * p_new_v_enc = (VIDEO_ENCODER *) malloc(sizeof(VIDEO_ENCODER));
	if (NULL == p_new_v_enc)
	{
		return NULL;
	}

	memset(p_new_v_enc, 0, sizeof(VIDEO_ENCODER));

	p_tmp = *p_v_enc;
	if (NULL == p_tmp)
	{
		*p_v_enc = p_new_v_enc;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_v_enc;
	}
	
	return p_new_v_enc;
}

void onvif_free_video_encoder(VIDEO_ENCODER ** p_v_enc)
{
	VIDEO_ENCODER * p_next;
	VIDEO_ENCODER * p_tmp = *p_v_enc;

	while (p_tmp)
	{
		p_next = p_tmp->next;

		free(p_tmp);
		p_tmp = p_next;
	}

	*p_v_enc = NULL;
}

VIDEO_ENCODER * onvif_find_video_encoder(ONVIF_DEVICE * p_dev, const char * token)
{
    VIDEO_ENCODER * p_v_enc = p_dev->video_enc;
    
    while (p_v_enc)
	{
	    if (strcmp(p_v_enc->token, token) == 0)
	    {
	        break;
	    }
	    
		p_v_enc = p_v_enc->next;
	}

	return p_v_enc;
}

AUDIO_ENCODER * onvif_add_audio_encoder(AUDIO_ENCODER ** p_a_enc)
{
	AUDIO_ENCODER * p_tmp;
	AUDIO_ENCODER * p_new_a_enc = (AUDIO_ENCODER *) malloc(sizeof(AUDIO_ENCODER));
	if (NULL == p_new_a_enc)
	{
		return NULL;
	}

	memset(p_new_a_enc, 0, sizeof(AUDIO_ENCODER));

	p_tmp = *p_a_enc;
	if (NULL == p_tmp)
	{
		*p_a_enc = p_new_a_enc;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_a_enc;
	}
	
	return p_new_a_enc;
}

void onvif_free_audio_encoder(AUDIO_ENCODER ** p_a_enc)
{
	AUDIO_ENCODER * p_next;
	AUDIO_ENCODER * p_tmp = *p_a_enc;

	while (p_tmp)
	{
		p_next = p_tmp->next;

		free(p_tmp);
		p_tmp = p_next;
	}

	*p_a_enc = NULL;
}

AUDIO_ENCODER * onvif_find_audio_encoder(ONVIF_DEVICE * p_dev, const char * token)
{
    AUDIO_ENCODER * p_a_enc = p_dev->audio_enc;
    
    while (p_a_enc)
	{
	    if (strcmp(p_a_enc->token, token) == 0)
	    {
	        break;
	    }
	    
		p_a_enc = p_a_enc->next;
	}

	return p_a_enc;
}

ONVIF_PROFILE * onvif_add_profile(ONVIF_PROFILE ** p_profile)
{
	ONVIF_PROFILE * p_tmp;
    ONVIF_PROFILE * p_new_profile = (ONVIF_PROFILE *) malloc(sizeof(ONVIF_PROFILE));
	if (NULL == p_new_profile)
	{
		return NULL;
	}

	memset(p_new_profile, 0, sizeof(ONVIF_PROFILE));

	p_tmp = *p_profile;
	if (NULL == p_tmp)
	{
		*p_profile = p_new_profile;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_profile;
	}
	
	return p_new_profile;
}

void onvif_free_profile(ONVIF_PROFILE ** p_profile)
{
	ONVIF_PROFILE * p_next;
	ONVIF_PROFILE * p_tmp = *p_profile;

	while (p_tmp)
	{
		p_next = p_tmp->next;

        if (p_tmp->video_src_cfg)
        {
            free(p_tmp->video_src_cfg);
        }

        if (p_tmp->video_enc)
        {
            free(p_tmp->video_enc);
        }

        if (p_tmp->audio_src_cfg)
        {
            free(p_tmp->audio_src_cfg);
        }

        if (p_tmp->audio_enc)
        {
            free(p_tmp->audio_enc);
        }

        if (p_tmp->ptz_cfg)
        {
            free(p_tmp->ptz_cfg);
        }
        
		free(p_tmp);
		p_tmp = p_next;
	}

	*p_profile = NULL;
}

ONVIF_PROFILE * onvif_find_profile(ONVIF_DEVICE * p_dev, const char * token)
{
    ONVIF_PROFILE * p_profile = p_dev->profiles;
    
    while (p_profile)
	{
	    if (strcmp(p_profile->token, token) == 0)
	    {
	        break;
	    }
	    
		p_profile = p_profile->next;
	}

	return p_profile;
}

PTZ_NODE * onvif_add_ptz_node(PTZ_NODE ** p_ptz_node)
{
	PTZ_NODE * p_tmp;
    PTZ_NODE * p_new_ptz_node = (PTZ_NODE *) malloc(sizeof(PTZ_NODE));
	if (NULL == p_new_ptz_node)
	{
		return NULL;
	}

	memset(p_new_ptz_node, 0, sizeof(PTZ_NODE));

	p_tmp = *p_ptz_node;
	if (NULL == p_tmp)
	{
		*p_ptz_node = p_new_ptz_node;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_ptz_node;
	}
	
	return p_new_ptz_node;
}

void onvif_free_ptz_node(PTZ_NODE ** p_ptz_node)
{
	PTZ_NODE * p_next;
	PTZ_NODE * p_tmp = *p_ptz_node;

	while (p_tmp)
	{
		p_next = p_tmp->next;
        
		free(p_tmp);
		p_tmp = p_next;
	}

	*p_ptz_node = NULL;
}

PTZ_NODE * onvif_find_ptz_node(ONVIF_DEVICE * p_dev, const char * token)
{
    PTZ_NODE * p_ptz_node = p_dev->ptznodes;
    
    while (p_ptz_node)
	{
	    if (strcmp(p_ptz_node->token, token) == 0)
	    {
	        break;
	    }
	    
		p_ptz_node = p_ptz_node->next;
	}

	return p_ptz_node;
}

PTZ_CFG * onvif_add_ptz_cfg(PTZ_CFG ** p_ptz_cfg)
{
	PTZ_CFG * p_tmp;
    PTZ_CFG * p_new_ptz_cfg = (PTZ_CFG *) malloc(sizeof(PTZ_CFG));
	if (NULL == p_new_ptz_cfg)
	{
		return NULL;
	}

	memset(p_new_ptz_cfg, 0, sizeof(PTZ_CFG));

	p_tmp = *p_ptz_cfg;
	if (NULL == p_tmp)
	{
		*p_ptz_cfg = p_new_ptz_cfg;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_ptz_cfg;
	}
	
	return p_new_ptz_cfg;
}

void onvif_free_ptz_cfg(PTZ_CFG ** p_ptz_cfg)
{
    PTZ_CFG * p_next;
	PTZ_CFG * p_tmp = *p_ptz_cfg;

	while (p_tmp)
	{
		p_next = p_tmp->next;
        
		free(p_tmp);
		p_tmp = p_next;
	}

	*p_ptz_cfg = NULL;
}

PTZ_CFG * onvif_find_ptz_cfg(ONVIF_DEVICE * p_dev, const char * token)
{
    PTZ_CFG * p_ptz_cfg = p_dev->ptz_cfg;
    
    while (p_ptz_cfg)
	{
	    if (strcmp(p_ptz_cfg->token, token) == 0)
	    {
	        break;
	    }
	    
		p_ptz_cfg = p_ptz_cfg->next;
	}

	return p_ptz_cfg;
}


VIDEO_SRC * onvif_get_cur_video_src(ONVIF_DEVICE * p_dev)
{
	VIDEO_SRC * p_v_src;
    ONVIF_PROFILE * p_profile = p_dev->curProfile;	
	if (NULL == p_profile)
	{
		p_profile = p_dev->profiles;
	}
	
	if (NULL == p_profile || NULL == p_profile->video_src_cfg)
	{
		return NULL;
	}

	p_v_src = onvif_find_video_source(p_dev, p_profile->video_src_cfg->source_token);
	if (NULL == p_v_src)
	{
		return NULL;
	}

	return p_v_src;
}

ONVIF_NOTIFY * onvif_add_notify(ONVIF_NOTIFY ** p_notify)
{
	ONVIF_NOTIFY * p_tmp;
    ONVIF_NOTIFY * p_new_notify = (ONVIF_NOTIFY *) malloc(sizeof(ONVIF_NOTIFY));
	if (NULL == p_new_notify)
	{
		return NULL;
	}

	memset(p_new_notify, 0, sizeof(ONVIF_NOTIFY));

	p_tmp = *p_notify;
	if (NULL == p_tmp)
	{
		*p_notify = p_new_notify;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_notify;
	}
	
	return p_new_notify;
}

void onvif_free_notify(ONVIF_NOTIFY ** p_notify)
{
    ONVIF_NOTIFY * p_next;
	ONVIF_NOTIFY * p_tmp = *p_notify;

	while (p_tmp)
	{
		p_next = p_tmp->next;

        if (p_tmp->source)
        {
            onvif_free_simple_item(&p_tmp->source);
        }

        if (p_tmp->key)
        {
            onvif_free_simple_item(&p_tmp->key);
        }

        if (p_tmp->data)
        {
            onvif_free_simple_item(&p_tmp->data);
        }
        
		free(p_tmp);
		p_tmp = p_next;
	}

	*p_notify = NULL;
}

SIMPLE_ITEM * onvif_add_simple_item(SIMPLE_ITEM ** p_item)
{
	SIMPLE_ITEM * p_tmp;
    SIMPLE_ITEM * p_new_item = (SIMPLE_ITEM *) malloc(sizeof(SIMPLE_ITEM));
	if (NULL == p_new_item)
	{
		return NULL;
	}

	memset(p_new_item, 0, sizeof(SIMPLE_ITEM));

	p_tmp = *p_item;
	if (NULL == p_tmp)
	{
		*p_item = p_new_item;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_item;
	}
	
	return p_new_item;
}

void onvif_free_simple_item(SIMPLE_ITEM ** p_item)
{
    SIMPLE_ITEM * p_next;
	SIMPLE_ITEM * p_tmp = *p_item;

	while (p_tmp)
	{
		p_next = p_tmp->next;
        
		free(p_tmp);
		p_tmp = p_next;
	}

	*p_item = NULL;
}

/***
 * get notify list item numbers
 */
int onvif_get_notify_nums(ONVIF_NOTIFY * p_notify)
{
    int nums = 0;
    ONVIF_NOTIFY * p_tmp = p_notify;

    while (p_tmp)
    {
        ++nums;
        p_tmp = p_tmp->next;
    }

    return nums;
}

/***
 * add notify list to onvif device event notify list at last
 */
void onvif_device_add_notify(ONVIF_DEVICE * p_dev, ONVIF_NOTIFY * p_notify)
{
    if (NULL == p_dev->events.notify)
    {        
        p_dev->events.notify = p_notify;
    }
    else
    {
        ONVIF_NOTIFY * p_tmp = p_dev->events.notify;

        while (p_tmp && p_tmp->next)
        {
            p_tmp = p_tmp->next;
        }

        p_tmp->next = p_notify;
    }    
}

/***
 * free nums event notify item from onvif device event notify list at first
 *
 * return the freed notify item numbers
 */
int onvif_device_free_notify(ONVIF_DEVICE * p_dev, int nums)
{
    int freed_nums = 0;

    ONVIF_NOTIFY * p_next;
    ONVIF_NOTIFY * p_notify = p_dev->events.notify;
    while (p_notify && freed_nums < nums)
    {
        p_next = p_notify->next;
        
        if (p_notify->source)
        {
            onvif_free_simple_item(&p_notify->source);
        }

        if (p_notify->key)
        {
            onvif_free_simple_item(&p_notify->key);
        }

        if (p_notify->data)
        {
            onvif_free_simple_item(&p_notify->data);
        }

        free(p_notify);

        ++freed_nums;
        p_notify = p_next;
    }

    p_dev->events.notify = p_notify;

    return freed_nums;
}

const char * onvif_format_simple_item(SIMPLE_ITEM * p_item)
{
    int offset = 0;
    int mlen = 1024;
    static char str[1024];
	SIMPLE_ITEM * p_tmp;

    memset(str, 0, sizeof(str));
    
    p_tmp = p_item;
    while (p_tmp)
    {
        offset += snprintf(str+offset, mlen-offset, "%s:%s\r\n", p_tmp->name, p_tmp->value);
        
        p_tmp = p_tmp->next;
    }

    return str;
}

TRACK_LIST * onvif_add_recording_track(TRACK_LIST ** p_track)
{
	TRACK_LIST * p_tmp;
	TRACK_LIST * p_new_track = (TRACK_LIST *) malloc(sizeof(TRACK_LIST));
	if (NULL == p_new_track)
	{
		return NULL;
	}

	memset(p_new_track, 0, sizeof(TRACK_LIST));

	p_tmp = *p_track;
	if (NULL == p_tmp)
	{
		*p_track = p_new_track;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_track;
	}
	
	return p_new_track;
}

void onvif_free_recording_track(TRACK_LIST ** p_track)
{
	TRACK_LIST * p_next;
	TRACK_LIST * p_tmp = *p_track;

	while (p_tmp)
	{
		p_next = p_tmp->next;
        
		free(p_tmp);
		p_tmp = p_next;
	}

	*p_track = NULL;
}

TRACK_ATTR_LIST* onvif_add_track_attr(TRACK_ATTR_LIST ** p_track_attr)
{
	TRACK_ATTR_LIST * p_tmp;
	TRACK_ATTR_LIST * p_new_track_attr = (TRACK_ATTR_LIST *) malloc(sizeof(TRACK_ATTR_LIST));
	if (NULL == p_new_track_attr)
	{
		return NULL;
	}

	memset(p_new_track_attr, 0, sizeof(TRACK_ATTR_LIST));

	p_tmp = *p_track_attr;
	if (NULL == p_tmp)
	{
		*p_track_attr = p_new_track_attr;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_track_attr;
	}
	
	return p_new_track_attr;
}

void onvif_free_track_attr(TRACK_ATTR_LIST ** p_track_attr)
{
	TRACK_ATTR_LIST * p_next;
	TRACK_ATTR_LIST * p_tmp = *p_track_attr;

	while (p_tmp)
	{
		p_next = p_tmp->next;
        
		free(p_tmp);
		p_tmp = p_next;
	}

	*p_track_attr = NULL;
}

RECORDING_LIST* onvif_add_recording(RECORDING_LIST ** p_recording)
{
	RECORDING_LIST * p_tmp;
	RECORDING_LIST * p_new_recording = (RECORDING_LIST *) malloc(sizeof(RECORDING_LIST));
	if (NULL == p_new_recording)
	{
		return NULL;
	}

	memset(p_new_recording, 0, sizeof(RECORDING_LIST));

	p_tmp = *p_recording;
	if (NULL == p_tmp)
	{
		*p_recording = p_new_recording;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_recording;
	}
	
	return p_new_recording;
}

void onvif_free_recording(RECORDING_LIST ** p_recording)
{
	RECORDING_LIST * p_next;
	RECORDING_LIST * p_tmp = *p_recording;

	while (p_tmp)
	{
		p_next = p_tmp->next;

        if (p_tmp->RecordingInformation.Track)
        {
        	onvif_free_recording_track(&p_tmp->RecordingInformation.Track);
        }
        
		free(p_tmp);
		p_tmp = p_next;
	}

	*p_recording = NULL;
}




