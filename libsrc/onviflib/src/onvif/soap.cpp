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

#include "soap.h"
#include "soap_parser.h"


BOOL onvif_GetCapabilities_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node;
	GetCapabilities_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetCapabilitiesResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

	p_res = (GetCapabilities_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memcpy(&p_res->capablity, &p_dev->capablity, sizeof(ONVIF_CAP));

	return parse_GetCapabilities(p_node, p_res);
}

BOOL onvif_GetServices_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node;
	XMLN * p_Service;
	GetServices_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetServicesResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetServices_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memcpy(&p_res->capablity, &p_dev->capablity, sizeof(ONVIF_CAP));

	p_Service = xml_node_soap_get(p_node, "Service");
	while (p_Service)
	{
	    XMLN * p_Namespace = xml_node_soap_get(p_Service, "Namespace");
    	if (p_Namespace && p_Namespace->data)
    	{
    	    if (strstr(p_Namespace->data, "device") == 0)
    	    {
    	        parse_DeviceService(p_Service, &p_res->capablity.device);
    	    }
    	    else if (strcasecmp(p_Namespace->data, "media") == 0)
    	    {
    	        p_res->capablity.media.support = parse_MediaService(p_Service, &p_res->capablity.media);
    	    }
    	    else if (strcasecmp(p_Namespace->data, "events") == 0)
    	    {
    	        p_res->capablity.events.support = parse_EventsService(p_Service, &p_res->capablity.events);
    	    }
    	    else if (strcasecmp(p_Namespace->data, "ptz") == 0)
    	    {
    	        p_res->capablity.ptz.support = parse_PTZService(p_Service, &p_res->capablity.ptz);
    	    }
    	    else if (strcasecmp(p_Namespace->data, "imaging") == 0)
    	    {
    	        p_res->capablity.image.support = parse_ImageingService(p_Service, &p_res->capablity.image);
    	    }
    	    else if (strcasecmp(p_Namespace->data, "recording") == 0)
    	    {
    	        p_res->capablity.recording.support = parse_RecordingService(p_Service, &p_res->capablity.recording);
    	    }
    	    else if (strcasecmp(p_Namespace->data, "search") == 0)
    	    {
    	        p_res->capablity.search.support = parse_SearchService(p_Service, &p_res->capablity.search);
    	    }
    	    else if (strcasecmp(p_Namespace->data, "replay") == 0)
    	    {
    	        p_res->capablity.replay.support = parse_ReplayService(p_Service, &p_res->capablity.replay);
    	    }
    	}

    	p_Service = p_Service->next;
	}

	return TRUE;
}

BOOL onvif_GetDeviceInformation_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetDeviceInformation_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetDeviceInformationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

	p_res = (GetDeviceInformation_RES *) argv;
	if (NULL == p_res)
	{
		return TRUE;
	}

	memset(p_res, 0, sizeof(GetDeviceInformation_RES));
    
	return parse_GetDeviceInformation(p_node, p_res);
}


BOOL onvif_GetNetworkInterfaces_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node;
	XMLN * p_NetworkInterfaces;
	GetNetworkInterfaces_RES * p_res;
	
	p_node = xml_node_soap_get(p_xml, "GetNetworkInterfacesResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}

	p_res = (GetNetworkInterfaces_RES *) argv;
	if (NULL == p_res)
	{
		return TRUE;
	}

	memset(p_res, 0, sizeof(GetNetworkInterfaces_RES));
	
	p_NetworkInterfaces = xml_node_soap_get(p_node, "NetworkInterfaces");
	while (p_NetworkInterfaces)
	{
		ONVIF_NET_INF * p_net_inf = onvif_add_network_interface(&p_res->p_net_inf);
		if (NULL != p_net_inf)
		{
			const char * p_token = xml_attr_get(p_NetworkInterfaces, "token");
			if (p_token)
			{
				strncpy(p_net_inf->token, p_token, sizeof(p_net_inf->token)-1);
			}
			
			if (parse_NetworkInterface(p_NetworkInterfaces, p_net_inf) == FALSE)
	        {
	            onvif_free_network_interface(&p_res->p_net_inf);
	            return FALSE;
	        }	
		}

		p_NetworkInterfaces = p_NetworkInterfaces->next;
	}
        
	return TRUE;
}

BOOL onvif_SetNetworkInterfaces_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node;
	SetNetworkInterfaces_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "SetNetworkInterfacesResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}

	p_res = (SetNetworkInterfaces_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }

    memset(p_res, 0, sizeof(SetNetworkInterfaces_RES));
    
	return parse_SetNetworkInterfaces(p_node, p_res);
}

BOOL onvif_GetNTP_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node;
	GetNTP_RES * p_res;
	
	p_node = xml_node_soap_get(p_xml, "GetNTPResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

	p_res = (GetNTP_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetNTP_RES));

	return parse_GetNTP(p_node, p_res);
}

BOOL onvif_SetNTP_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetNTPResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL onvif_GetHostname_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetHostname_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetHostnameResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

	p_res = (GetHostname_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetHostname_RES));

	return parse_GetHostname(p_node, p_res);
}

BOOL onvif_SetHostname_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetHostnameResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL onvif_SetHostnameFromDHCP_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	SetHostnameFromDHCP_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "SetHostnameFromDHCPResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}

	p_res = (SetHostnameFromDHCP_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }

    memset(p_res, 0, sizeof(SetHostnameFromDHCP_RES));
    
	return parse_SetHostnameFromDHCP(p_node, p_res);
}
	
BOOL onvif_GetDNS_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{	
	GetDNS_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetDNSResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

	p_res = (GetDNS_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetDNS_RES));

	return parse_GetDNS(p_node, p_res);
}
	
BOOL onvif_SetDNS_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetDNSResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}
	
	return TRUE;
}
	
BOOL onvif_GetDynamicDNS_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetDynamicDNS_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetDynamicDNSResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

	p_res = (GetDynamicDNS_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetDynamicDNS_RES));

	return parse_GetDynamicDNS(p_node, p_res);
}
	
BOOL onvif_SetDynamicDNS_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetDynamicDNSResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL onvif_GetNetworkProtocols_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetNetworkProtocols_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetNetworkProtocolsResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

	p_res = (GetNetworkProtocols_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetNetworkProtocols_RES));

	return parse_GetNetworkProtocols(p_node, p_res);
}

BOOL onvif_SetNetworkProtocols_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetNetworkProtocolsResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}
	
	return TRUE;
}
	
BOOL onvif_GetDiscoveryMode_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetDiscoveryMode_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetDiscoveryModeResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

	p_res = (GetDiscoveryMode_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetDiscoveryMode_RES));

	return parse_GetDiscoveryMode(p_node, p_res);
}

BOOL onvif_SetDiscoveryMode_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetDiscoveryModeResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}
	
	return TRUE;
}
	
BOOL onvif_GetNetworkDefaultGateway_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetNetworkDefaultGateway_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetNetworkDefaultGatewayResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

	p_res = (GetNetworkDefaultGateway_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetNetworkDefaultGateway_RES));

	return parse_GetNetworkDefaultGateway(p_node, p_res);
}

BOOL onvif_SetNetworkDefaultGateway_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetNetworkDefaultGatewayResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL onvif_GetSystemDateAndTime_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetSystemDateAndTime_RES * p_res;
    XMLN * p_node = xml_node_soap_get(p_xml, "GetSystemDateAndTimeResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetSystemDateAndTime_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetSystemDateAndTime_RES));
	
	return parseGetSystemDateAndTime(p_node, p_res);
}

BOOL onvif_SetSystemDateAndTime_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetSystemDateAndTimeResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	return TRUE;
}

BOOL onvif_SystemReboot_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SystemRebootResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	return TRUE;
}

BOOL onvif_SetSystemFactoryDefault_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetSystemFactoryDefaultResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	return TRUE;
}


BOOL onvif_GetSystemLog_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	return TRUE;
}
		
BOOL onvif_GetScopes_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	return TRUE;
}
		
BOOL onvif_SetScopes_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	return TRUE;
}
		
BOOL onvif_AddScopes_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	return TRUE;
}
		
BOOL onvif_RemoveScopes_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	return TRUE;
}
	
BOOL onvif_GetVideoSources_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node;
	XMLN * p_VideoSources;
	GetVideoSources_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetVideoSourcesResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetVideoSources_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetVideoSources_RES));
	
    p_VideoSources = xml_node_soap_get(p_node, "VideoSources");
    while (p_VideoSources)
    {
		VIDEO_SRC * p_v_src = onvif_add_video_source(&p_res->p_v_src);
    	if (p_v_src)
    	{
    		const char * p_token = xml_attr_get(p_VideoSources, "token");
    		if (p_token)
    		{
	        	strncpy(p_v_src->token, p_token, sizeof(p_v_src->token)-1);
			}
			
	        if (parse_VideoSource(p_VideoSources, p_v_src) == FALSE)
	        {
	            onvif_free_video_source(&p_res->p_v_src);
	            return FALSE;
	        }
        }

        p_VideoSources = p_VideoSources->next;
    }
    
	return TRUE;
}
		
BOOL onvif_GetAudioSources_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node;
	XMLN * p_AudioSources;
	GetAudioSources_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetAudioSourcesResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetAudioSources_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetAudioSources_RES));
	
    p_AudioSources = xml_node_soap_get(p_node, "AudioSources");
    while (p_AudioSources)
    {
		AUDIO_SRC * p_a_src = onvif_add_audio_source(&p_res->p_a_src);
    	if (p_a_src)
    	{
    		const char * p_token = xml_attr_get(p_AudioSources, "token");
    		if (p_token)
    		{
	        	strncpy(p_a_src->token, p_token, sizeof(p_a_src->token)-1);
			}
			
	        if (parse_AudioSource(p_AudioSources, p_a_src) == FALSE)
	        {
	            onvif_free_audio_source(&p_res->p_a_src);
	            return FALSE;
	        }
        }

        p_AudioSources = p_AudioSources->next;
    }
    
	return TRUE;
}
		
BOOL onvif_CreateProfile_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node;
	XMLN * p_Profile;
	CreateProfile_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "CreateProfileResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (CreateProfile_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(CreateProfile_RES));
	
    p_Profile = xml_node_soap_get(p_node, "Profile");
    if (p_Profile)
    {
    	const char * p_fixed;
		const char * p_token;

		p_fixed = xml_attr_get(p_Profile, "fixed");
    	if (p_fixed)
    	{
        	p_res->profile.fixed = parse_Bool(p_fixed);
        }

        p_token = xml_attr_get(p_Profile, "token");
        if (p_token)
        {
        	strncpy(p_res->profile.token, p_token, sizeof(p_res->profile.token)-1);
		}
		
        if (parse_Profile(p_Profile, &p_res->profile) == FALSE)
        {
            return FALSE;
        }
    }
    
    return TRUE;
}
		
BOOL onvif_GetProfile_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node;
	XMLN * p_Profile;
	GetProfile_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetProfileResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetProfile_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetProfile_RES));
	
    p_Profile = xml_node_soap_get(p_node, "Profile");
    if (p_Profile)
    {
    	const char * p_fixed;
		const char * p_token;

		p_fixed = xml_attr_get(p_Profile, "fixed");
    	if (p_fixed)
    	{
        	p_res->profile.fixed = parse_Bool(p_fixed);
        }

        p_token = xml_attr_get(p_Profile, "token");
        if (p_token)
        {
        	strncpy(p_res->profile.token, p_token, sizeof(p_res->profile.token)-1);
		}
		
        if (parse_Profile(p_Profile, &p_res->profile) == FALSE)
        {
            return FALSE;
        }
    }
    
    return TRUE;
}

BOOL onvif_GetProfiles_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node;
	XMLN * p_Profiles;
	GetProfiles_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetProfilesResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetProfiles_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetProfiles_RES));
	
    p_Profiles = xml_node_soap_get(p_node, "Profiles");
    while (p_Profiles)
    {
		ONVIF_PROFILE * p_profile = onvif_add_profile(&p_res->p_profile);
    	if (p_profile)
    	{
    		const char * p_fixed;
			const char * p_token;

			p_fixed = xml_attr_get(p_Profiles, "fixed");
    		if (p_fixed)
    		{
	        	p_profile->fixed = parse_Bool(p_fixed);
	        }

	        p_token = xml_attr_get(p_Profiles, "token");
	        if (p_token)
	        {
	        	strncpy(p_profile->token, p_token, sizeof(p_profile->token)-1);
			}
			
	        if (parse_Profile(p_Profiles, p_profile) == FALSE)
	        {
	            onvif_free_profile(&p_res->p_profile);
	            return FALSE;
	        }
        }

        p_Profiles = p_Profiles->next;
    }
    
    return TRUE;
}		


BOOL onvif_AddVideoEncoderConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "AddVideoEncoderConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_AddVideoSourceConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node = xml_node_soap_get(p_xml, "AddVideoSourceConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_AddAudioEncoderConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node = xml_node_soap_get(p_xml, "AddAudioEncoderConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_AddAudioSourceConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "AddAudioSourceConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_AddPTZConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "AddPTZConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_RemoveVideoEncoderConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "RemoveVideoEncoderConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_RemoveVideoSourceConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node = xml_node_soap_get(p_xml, "RemoveVideoSourceConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_RemoveAudioEncoderConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node = xml_node_soap_get(p_xml, "RemoveAudioEncoderConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_RemoveAudioSourceConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node = xml_node_soap_get(p_xml, "RemoveAudioSourceConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_RemovePTZConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node = xml_node_soap_get(p_xml, "RemovePTZConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_DeleteProfile_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node = xml_node_soap_get(p_xml, "DeleteProfileResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_GetVideoSourceConfigurations_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node;
	XMLN * p_Configurations;
	GetVideoSourceConfigurations_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetVideoSourceConfigurationsResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetVideoSourceConfigurations_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetVideoSourceConfigurations_RES));
	
    p_Configurations = xml_node_soap_get(p_node, "Configurations");
    while (p_Configurations)
    {
		VIDEO_SRC_CFG * p_v_src_cfg = onvif_add_video_source_cfg(&p_res->p_v_src_cfg);
    	if (p_v_src_cfg)
    	{
    		const char * p_token = xml_attr_get(p_Configurations, "token");
    		if (p_token)
    		{
	        	strncpy(p_v_src_cfg->token, p_token, sizeof(p_v_src_cfg->token)-1);
			}
			
	        if (parse_VideoSourceConfiguration(p_Configurations, p_v_src_cfg) == FALSE)
	        {
	            onvif_free_video_source_cfg(&p_res->p_v_src_cfg);
	            return FALSE;
	        }
        }

        p_Configurations = p_Configurations->next;
    }
    
    return TRUE;
}


BOOL onvif_GetVideoEncoderConfigurations_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node;
	XMLN * p_Configurations;
	GetVideoEncoderConfigurations_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetVideoEncoderConfigurationsResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetVideoEncoderConfigurations_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetVideoEncoderConfigurations_RES));
	
    p_Configurations = xml_node_soap_get(p_node, "Configurations");
    while (p_Configurations)
    {
		VIDEO_ENCODER * p_v_enc = onvif_add_video_encoder(&p_res->p_v_enc);
    	if (p_v_enc)
    	{
    		const char * p_token = xml_attr_get(p_Configurations, "token");
    		if (p_token)
    		{
	        	strncpy(p_v_enc->token, p_token, sizeof(p_v_enc->token)-1);
			}
			
	        if (parse_VideoEncoder(p_Configurations, p_v_enc) == FALSE)
	        {
	            onvif_free_video_encoder(&p_res->p_v_enc);
	            return FALSE;
	        }
        }

        p_Configurations = p_Configurations->next;
    }
    
    return TRUE;
}

BOOL onvif_GetAudioSourceConfigurations_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node;
	XMLN * p_Configurations;
	GetAudioSourceConfigurations_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetAudioSourceConfigurationsResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetAudioSourceConfigurations_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetAudioSourceConfigurations_RES));
	
    p_Configurations = xml_node_soap_get(p_node, "Configurations");
    while (p_Configurations)
    {
		AUDIO_SRC_CFG * p_a_src_cfg = onvif_add_audio_source_cfg(&p_res->p_a_src_cfg);
    	if (p_a_src_cfg)
    	{
    		const char * p_token = xml_attr_get(p_Configurations, "token");
    		if (p_token)
    		{
	        	strncpy(p_a_src_cfg->token, p_token, sizeof(p_a_src_cfg->token)-1);
	        }	

	        if (parse_AudioSourceConfiguration(p_Configurations, p_a_src_cfg) == FALSE)
	        {
	            onvif_free_audio_source_cfg(&p_res->p_a_src_cfg);
	            return FALSE;
	        }
        }

        p_Configurations = p_Configurations->next;
    }
    
    return TRUE;
}
		
BOOL onvif_GetAudioEncoderConfigurations_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node;
	XMLN * p_Configurations;
	GetAudioEncoderConfigurations_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetAudioEncoderConfigurationsResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetAudioEncoderConfigurations_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetAudioEncoderConfigurations_RES));
	
    p_Configurations = xml_node_soap_get(p_node, "Configurations");
    while (p_Configurations)
    {
		AUDIO_ENCODER * p_a_enc = onvif_add_audio_encoder(&p_res->p_a_enc);
    	if (p_a_enc)
    	{
    		const char * p_token = xml_attr_get(p_Configurations, "token");
    		if (p_token)
    		{
	        	strncpy(p_a_enc->token, p_token, sizeof(p_a_enc->token)-1);
	        }

	        if (parse_AudioEncoder(p_Configurations, p_a_enc) == FALSE)
	        {
	            onvif_free_audio_encoder(&p_res->p_a_enc);
	            return FALSE;
	        }
        }

        p_Configurations = p_Configurations->next;
    }
    
    return TRUE;
}
		
BOOL onvif_GetVideoSourceConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node;
	XMLN * p_Configuration;
	GetVideoSourceConfiguration_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetVideoSourceConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetVideoSourceConfiguration_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetVideoSourceConfiguration_RES));
	
    p_Configuration = xml_node_soap_get(p_node, "Configuration");
    if (p_Configuration)
    {
    	const char * p_token = xml_attr_get(p_Configuration, "token");
    	if (p_token)
    	{
        	strncpy(p_res->video_src_cfg.token, p_token, sizeof(p_res->video_src_cfg.token)-1);
        }	

        if (parse_VideoSourceConfiguration(p_Configuration, &p_res->video_src_cfg) == FALSE)
        {
            return FALSE;
        }
    }
    else
    {
    	return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_GetVideoEncoderConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node;
	XMLN * p_Configuration;
	GetVideoEncoderConfiguration_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetVideoEncoderConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetVideoEncoderConfiguration_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetVideoEncoderConfiguration_RES));
	
    p_Configuration = xml_node_soap_get(p_node, "Configuration");
    if (p_Configuration)
    {
    	const char * p_token = xml_attr_get(p_Configuration, "token");
    	if (p_token)
    	{
        	strncpy(p_res->video_enc.token, p_token, sizeof(p_res->video_enc.token)-1);
        }	

        if (parse_VideoEncoder(p_Configuration, &p_res->video_enc) == FALSE)
        {
            return FALSE;
        }
    }
    else
    {
    	return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_GetAudioSourceConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node;
	XMLN * p_Configuration;
	GetAudioSourceConfiguration_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetAudioSourceConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetAudioSourceConfiguration_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetAudioSourceConfiguration_RES));
	
    p_Configuration = xml_node_soap_get(p_node, "Configuration");
    if (p_Configuration)
    {
    	const char * p_token = xml_attr_get(p_Configuration, "token");
    	if (p_token)
    	{
        	strncpy(p_res->audio_src_cfg.token, p_token, sizeof(p_res->audio_src_cfg.token)-1);
        }	

        if (parse_AudioSourceConfiguration(p_Configuration, &p_res->audio_src_cfg) == FALSE)
        {
            return FALSE;
        }
    }
    else
    {
    	return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_GetAudioEncoderConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node;
	XMLN * p_Configuration;
	GetAudioEncoderConfiguration_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetAudioEncoderConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetAudioEncoderConfiguration_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetAudioEncoderConfiguration_RES));
	
    p_Configuration = xml_node_soap_get(p_node, "Configuration");
    if (p_Configuration)
    {
    	const char * p_token = xml_attr_get(p_Configuration, "token");
    	if (p_token)
    	{
        	strncpy(p_res->audio_enc.token, p_token, sizeof(p_res->audio_enc.token)-1);
        }	

        if (parse_AudioEncoder(p_Configuration, &p_res->audio_enc) == FALSE)
        {
            return FALSE;
        }
    }
    else
    {
    	return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_SetVideoSourceConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "GetAudioEncoderConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_SetVideoEncoderConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetVideoEncoderConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_SetAudioSourceConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetAudioSourceConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_SetAudioEncoderConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetAudioEncoderConfigurationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_GetVideoSourceConfigurationOptions_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "GetVideoSourceConfigurationOptionsResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_GetVideoEncoderConfigurationOptions_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetVideoEncoderConfigurationOptions_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetVideoEncoderConfigurationOptionsResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetVideoEncoderConfigurationOptions_RES *) argv;
	if (NULL == p_res)
	{
		return TRUE;
	}

    memset(p_res, 0, sizeof(GetVideoEncoderConfigurationOptions_RES));

	return parse_GetVideoEncoderConfigurationOptions(p_node, p_res);
}
		
BOOL onvif_GetAudioSourceConfigurationOptions_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "GetAudioSourceConfigurationOptionsResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}
		
BOOL onvif_GetAudioEncoderConfigurationOptions_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "GetAudioEncoderConfigurationOptionsResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL onvif_GetStreamUri_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetStreamUri_RES * p_res;
    XMLN * p_node = xml_node_soap_get(p_xml, "GetStreamUriResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	p_res = (GetStreamUri_RES *)argv;
	if (NULL == p_res)
	{
	    return FALSE;
	}
	
	memset(p_res, 0, sizeof(GetStreamUri_RES));
	
    return parse_GetStreamUri(p_node, p_res);
}

BOOL onvif_SetSynchronizationPoint_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetSynchronizationPointResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	return TRUE;
}
		
BOOL onvif_GetSnapshotUri_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetSnapshotUri_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetSnapshotUriResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	p_res = (GetSnapshotUri_RES *)argv;
	if (NULL == p_res)
	{
	    return FALSE;
	}
	
	memset(p_res, 0, sizeof(GetSnapshotUri_RES));
	
    return parse_GetSnapshotUri(p_node, p_res);
}
		
BOOL onvif_GetNodes_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node;
	XMLN * p_PTZNode;
	GetNodes_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetNodesResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetNodes_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetNodes_RES));
	
    p_PTZNode = xml_node_soap_get(p_node, "PTZNode");
    while (p_PTZNode)
    {
		PTZ_NODE * p_ptz_node = onvif_add_ptz_node(&p_res->p_ptz_node);
    	if (p_ptz_node)
    	{
    		const char * p_FixedHomePosition;
			const char * p_token;

			p_FixedHomePosition = xml_attr_get(p_PTZNode, "FixedHomePosition");
    		if (p_FixedHomePosition)
    		{
    			p_res->p_ptz_node->fixed_home_pos =  parse_Bool(p_FixedHomePosition);
    		}

    		p_token = xml_attr_get(p_PTZNode, "token");
    		if (p_token)
    		{
	        	strncpy(p_ptz_node->token, p_token, sizeof(p_ptz_node->token)-1);
			}
			
	        if (parse_PTZNode(p_PTZNode, p_ptz_node) == FALSE)
	        {
	            onvif_free_ptz_node(&p_res->p_ptz_node);
	            return FALSE;
	        }
        }

        p_PTZNode = p_PTZNode->next;
    }
    
    return TRUE;
}
    	
BOOL onvif_GetNode_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node;
	XMLN * p_PTZNode;
	GetNode_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetNodeResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetNode_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetNode_RES));
	
    p_PTZNode = xml_node_soap_get(p_node, "PTZNode");
    if (p_PTZNode)
    {
    	const char * p_FixedHomePosition;
		const char * p_token;

		p_FixedHomePosition = xml_attr_get(p_PTZNode, "FixedHomePosition");
    	if (p_FixedHomePosition)
    	{
			p_res->ptz_node.fixed_home_pos =  parse_Bool(p_FixedHomePosition);
		}

		p_token = xml_attr_get(p_PTZNode, "token");
		{
        	strncpy(p_res->ptz_node.token, p_token, sizeof(p_res->ptz_node.token)-1);
        }	

        if (parse_PTZNode(p_PTZNode, &p_res->ptz_node) == FALSE)
        {
            return FALSE;
        }
    }
    
    return TRUE;
}		

BOOL onvif_GetPresets_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	int i = 0;
	XMLN * p_node;
	XMLN * p_Preset;
	GetPresets_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetPresetsResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

    p_res = (GetPresets_RES *) argv;
    if (NULL == p_res)
    {
    	return TRUE;
    }
    
	memset(p_res, 0, sizeof(GetPresets_RES));

    p_Preset = xml_node_soap_get(p_node, "Preset");
    while (p_Preset)
    {		
    	const char * p_token;
		
    	p_res->presets[i].used_flag = TRUE;

    	p_token = xml_attr_get(p_Preset, "token");
    	if (p_token)
    	{
        	strncpy(p_res->presets[i].token, p_token, sizeof(p_res->presets[i].token)-1);
        }	

        if (parse_Preset(p_Preset, &p_res->presets[i]) == FALSE)
        {
            return FALSE;
        }

        if (++i >= MAX_PTZ_PRESETS)
        {
        	break;
        }

        p_Preset = p_Preset->next;
    }
    
    return TRUE;
}

BOOL onvif_SetPreset_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	SetPreset_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "SetPresetResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	p_res = (SetPreset_RES *)argv;
	if (NULL == p_res)
	{
	    return FALSE;
	}
	
	memset(p_res, 0, sizeof(SetPreset_RES));
	
    return parse_SetPreset(p_node, p_res);
}

BOOL onvif_RemovePreset_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "RemovePresetResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	return TRUE;
}

BOOL onvif_GotoPreset_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{	
	XMLN * p_node = xml_node_soap_get(p_xml, "GotoPresetResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	return TRUE;
}

BOOL onvif_GotoHomePosition_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "GotoHomePositionResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	return TRUE;
}

BOOL onvif_SetHomePosition_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetHomePositionResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	return TRUE;
}

BOOL onvif_GetStatus_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{	
	GetStatus_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetStatusResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }

	p_res = (GetStatus_RES *)argv;
	if (NULL == p_res)
	{
	    return FALSE;
	}
	
	memset(p_res, 0, sizeof(GetStatus_RES));
	
    return parse_GetStatus(p_node, p_res);
}

BOOL onvif_ContinuousMove_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "ContinuousMoveResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	return TRUE;
}

BOOL onvif_RelativeMove_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "RelativeMoveResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	return TRUE;
}
		
BOOL onvif_AbsoluteMove_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "AbsoluteMoveResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	return TRUE;
}

BOOL onvif_PTZ_Stop_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "StopResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	return TRUE;
}

BOOL onvif_GetConfigurations_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{	
	XMLN * p_node;
	XMLN * p_PTZConfiguration;
	GetConfigurations_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetConfigurationsResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}

	p_res = (GetConfigurations_RES *) argv;
	if (NULL == p_res)
	{
	    return TRUE;
	}

	memset(p_res, 0, sizeof(GetConfigurations_RES));
	
	p_PTZConfiguration = xml_node_soap_get(p_node, "PTZConfiguration");
	while (p_PTZConfiguration)
	{
		PTZ_CFG * p_ptz_cfg = onvif_add_ptz_cfg(&p_res->p_ptz_cfg);
		if (p_ptz_cfg)
		{
			const char * p_token = xml_attr_get(p_PTZConfiguration, "token");
			if (p_token)
			{
    			strncpy(p_ptz_cfg->token, p_token, sizeof(p_ptz_cfg->token)-1);
    		}

    		if (parse_PTZConfiguration(p_PTZConfiguration, p_ptz_cfg) == FALSE)
            {
                onvif_free_ptz_cfg(&p_res->p_ptz_cfg);
                return FALSE;
            }		
        }

        p_PTZConfiguration = p_PTZConfiguration->next;
	}
        
	return TRUE;
} 

BOOL onvif_GetConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node;
	XMLN * p_PTZConfiguration;
	GetConfiguration_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetConfigurationResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}

	p_res = (GetConfiguration_RES *) argv;
	if (NULL == p_res) 
	{
	    return TRUE;
	}

	memset(p_res, 0, sizeof(GetConfiguration_RES));
	
	p_PTZConfiguration = xml_node_soap_get(p_node, "PTZConfiguration");
	if (p_PTZConfiguration)
	{
		const char * p_token = xml_attr_get(p_PTZConfiguration, "token");
		if (p_token)
		{
			strncpy(p_res->ptz_cfg.token, p_token, sizeof(p_res->ptz_cfg.token)-1);
		}

		if (parse_PTZConfiguration(p_PTZConfiguration, &p_res->ptz_cfg) == FALSE)
        {
            return FALSE;
        }		
	}
	
	return TRUE;
}
		
BOOL onvif_SetConfiguration_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "SetConfigurationResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL onvif_GetConfigurationOptions_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetConfigurationOptions_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetConfigurationOptionsResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}
	
	p_res = (GetConfigurationOptions_RES *) argv;
	if (NULL == p_res)
	{
		return TRUE;
	}

	memset(p_res, 0, sizeof(GetConfigurationOptions_RES));
	
	return parse_GetConfigurationOptions(p_xml, p_res);
}

BOOL onvif_GetEventProperties_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	return TRUE;
}
    
BOOL onvif_Renew_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node = xml_node_soap_get(p_xml, "RenewResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}

	return TRUE;
}
    
BOOL onvif_Unsubscribe_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node = xml_node_soap_get(p_xml, "UnsubscribeResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}
	
	return TRUE;
}
    
BOOL onvif_Subscribe_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	Subscribe_RES * p_res;
    XMLN * p_node = xml_node_soap_get(p_xml, "SubscribeResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}

	p_res = (Subscribe_RES *) argv;
	if (NULL == p_res)
	{
		return TRUE;
	}

	memset(p_res, 0, sizeof(Subscribe_RES));
	
	return parse_Subscribe(p_node, p_res);
}
    	
BOOL onvif_GetImagingSettings_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	XMLN * p_node;
	XMLN * p_ImagingSettings;
	GetImagingSettings_RES * p_res;

	p_node = xml_node_soap_get(p_xml, "GetImagingSettingsResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}
	
	p_res = (GetImagingSettings_RES *) argv;
	if (NULL == p_res)
	{
		return TRUE;
	}

	memset(p_res, 0, sizeof(GetImagingSettings_RES));

	p_ImagingSettings = xml_node_soap_get(p_node, "ImagingSettings");
	if (NULL == p_ImagingSettings)
	{
		return FALSE;
	}
	
	return parse_ImagingSettings(p_ImagingSettings, &p_res->img_cfg);
}
    
BOOL onvif_SetImagingSettings_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
    XMLN * p_node = xml_node_soap_get(p_xml, "SetImagingSettingsResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL onvif_GetOptions_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetOptions_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetOptionsResponse");
	if (NULL == p_node)
	{
		return FALSE;
	}
	
	p_res = (GetOptions_RES *) argv;
	if (NULL == p_res)
	{
		return TRUE;
	}

	memset(p_res, 0, sizeof(GetOptions_RES));
	
	return parse_GetOptions(p_node, p_res);
}

BOOL onvif_GetReplayUri_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetReplayUri_RES * p_res;
    XMLN * p_node = xml_node_soap_get(p_xml, "GetReplayUriResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	p_res = (GetReplayUri_RES *)argv;
	if (NULL == p_res)
	{
	    return FALSE;
	}
	
	memset(p_res, 0, sizeof(GetReplayUri_RES));
	
    return parse_GetReplayUri(p_node, p_res);
}

BOOL onvif_GetRecordingSummary_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetRecordingSummary_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetRecordingSummaryResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	p_res = (GetRecordingSummary_RES *)argv;
	if (NULL == p_res)
	{
	    return FALSE;
	}
	
	memset(p_res, 0, sizeof(GetRecordingSummary_RES));
	
    return parse_GetRecordingSummary(p_node, p_res);
}

BOOL onvif_GetRecordingInformation_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetRecordingInformation_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetRecordingInformationResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	p_res = (GetRecordingInformation_RES *)argv;
	if (NULL == p_res)
	{
	    return FALSE;
	}
	
	memset(p_res, 0, sizeof(GetRecordingInformation_RES));
	
    return parse_GetRecordingInformation(p_node, p_res);
}

BOOL onvif_GetMediaAttributes_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetMediaAttributes_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetMediaAttributesResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	p_res = (GetMediaAttributes_RES *)argv;
	if (NULL == p_res)
	{
	    return FALSE;
	}
	
	memset(p_res, 0, sizeof(GetMediaAttributes_RES));
	
    return parse_GetMediaAttributes(p_node, p_res);
}

BOOL onvif_FindRecordings_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	FindRecordings_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "FindRecordingsResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	p_res = (FindRecordings_RES *)argv;
	if (NULL == p_res)
	{
	    return FALSE;
	}
	
	memset(p_res, 0, sizeof(FindRecordings_RES));
	
    return parse_FindRecordings(p_node, p_res);
}

BOOL onvif_GetRecordingSearchResults_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetRecordingSearchResults_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetRecordingSearchResultsResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	p_res = (GetRecordingSearchResults_RES *)argv;
	if (NULL == p_res)
	{
	    return FALSE;
	}
	
	memset(p_res, 0, sizeof(GetRecordingSearchResults_RES));
	
    return parse_GetRecordingSearchResults(p_node, p_res);
}

BOOL onvif_FindEvents_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	FindEvents_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "FindEventsResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	p_res = (FindEvents_RES *)argv;
	if (NULL == p_res)
	{
	    return FALSE;
	}
	
	memset(p_res, 0, sizeof(FindEvents_RES));
	
    return parse_FindEvents(p_node, p_res);
}

BOOL onvif_GetEventSearchResults_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	return TRUE;
}

BOOL onvif_GetSearchState_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	GetSearchState_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "GetSearchStateResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	p_res = (GetSearchState_RES *)argv;
	if (NULL == p_res)
	{
	    return FALSE;
	}
	
	memset(p_res, 0, sizeof(GetSearchState_RES));
	
    return parse_GetSearchState(p_node, p_res);
}

BOOL onvif_EndSearch_rly(XMLN * p_xml, ONVIF_DEVICE * p_dev, void * argv)
{
	EndSearch_RES * p_res;
	XMLN * p_node = xml_node_soap_get(p_xml, "EndSearchResponse");
    if (NULL == p_node)
    {
        return FALSE;
    }
    
	p_res = (EndSearch_RES *)argv;
	if (NULL == p_res)
	{
	    return FALSE;
	}
	
	memset(p_res, 0, sizeof(EndSearch_RES));
	
    return parse_EndSearch(p_node, p_res);
}




BOOL onvif_rly_handler(XMLN * p_xml, eOnvifAction act, ONVIF_DEVICE * p_dev, void * argv)
{
    BOOL ret = FALSE;
    
    switch(act)
	{
	case eGetCapabilities:
        ret = onvif_GetCapabilities_rly(p_xml, p_dev, argv);
        break;

    case eGetServices:
        ret = onvif_GetServices_rly(p_xml, p_dev, argv);
        break;
        
	case eGetDeviceInformation:
		ret = onvif_GetDeviceInformation_rly(p_xml, p_dev, argv);
		break;

	case eGetNetworkInterfaces:
        ret = onvif_GetNetworkInterfaces_rly(p_xml, p_dev, argv);
        break;

	case eSetNetworkInterfaces:
        ret = onvif_SetNetworkInterfaces_rly(p_xml, p_dev, argv);
        break;

	case eGetNTP:
		ret = onvif_GetNTP_rly(p_xml, p_dev, argv);
		break;

	case eSetNTP:
		ret = onvif_SetNTP_rly(p_xml, p_dev, argv);
		break;

	case eGetHostname:
		ret = onvif_GetHostname_rly(p_xml, p_dev, argv);
		break;

	case eSetHostname:
		ret = onvif_SetHostname_rly(p_xml, p_dev, argv);
		break;

	case eSetHostnameFromDHCP:
		ret = onvif_SetHostnameFromDHCP_rly(p_xml, p_dev, argv);
		break;
		
	case eGetDNS:
		ret = onvif_GetDNS_rly(p_xml, p_dev, argv);
		break;
		
	case eSetDNS:
		ret = onvif_SetDNS_rly(p_xml, p_dev, argv);
		break;	
		
	case eGetDynamicDNS:
		ret = onvif_GetDynamicDNS_rly(p_xml, p_dev, argv);
		break;
		
	case eSetDynamicDNS:
		ret = onvif_SetDynamicDNS_rly(p_xml, p_dev, argv);
		break;

	case eGetNetworkProtocols:
		ret = onvif_GetNetworkProtocols_rly(p_xml, p_dev, argv);
		break;

	case eSetNetworkProtocols:
		ret = onvif_SetNetworkProtocols_rly(p_xml, p_dev, argv);
		break;
		
	case eGetDiscoveryMode:
		ret = onvif_GetDiscoveryMode_rly(p_xml, p_dev, argv);
		break;

	case eSetDiscoveryMode:
		ret = onvif_SetDiscoveryMode_rly(p_xml, p_dev, argv);
		break;
		
	case eGetNetworkDefaultGateway:
		ret = onvif_GetNetworkDefaultGateway_rly(p_xml, p_dev, argv);
		break;

	case eSetNetworkDefaultGateway:
		ret = onvif_SetNetworkDefaultGateway_rly(p_xml, p_dev, argv);
		break;
		
	case eGetSystemDateAndTime:
		ret = onvif_GetSystemDateAndTime_rly(p_xml, p_dev, argv);
		break;
		
	case eSetSystemDateAndTime:
		ret = onvif_SetSystemDateAndTime_rly(p_xml, p_dev, argv);
		break;
		
	case eSystemReboot:
		ret = onvif_SystemReboot_rly(p_xml, p_dev, argv);
		break;

	case eSetSystemFactoryDefault:
		ret = onvif_SetSystemFactoryDefault_rly(p_xml, p_dev, argv);
		break;
		
	case eGetSystemLog:
		ret = onvif_GetSystemLog_rly(p_xml, p_dev, argv);
		break;
		
	case eGetScopes:
		ret = onvif_GetScopes_rly(p_xml, p_dev, argv);
		break;
		
	case eSetScopes:
		ret = onvif_SetScopes_rly(p_xml, p_dev, argv);
		break;
		
	case eAddScopes:
		ret = onvif_AddScopes_rly(p_xml, p_dev, argv);
		break;
		
	case eRemoveScopes:
		ret = onvif_RemoveScopes_rly(p_xml, p_dev, argv);
		break;
	
	case eGetVideoSources:
		ret = onvif_GetVideoSources_rly(p_xml, p_dev, argv);
		break;
		
	case eGetAudioSources:
		ret = onvif_GetAudioSources_rly(p_xml, p_dev, argv);
		break;
		
	case eCreateProfile:
		ret = onvif_CreateProfile_rly(p_xml, p_dev, argv);
		break;
		
	case eGetProfile:
		ret = onvif_GetProfile_rly(p_xml, p_dev, argv);
		break;
		
	case eGetProfiles:
		ret = onvif_GetProfiles_rly(p_xml, p_dev, argv);
		break;
		
	case eAddVideoEncoderConfiguration:
		ret = onvif_AddVideoEncoderConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eAddVideoSourceConfiguration:
		ret = onvif_AddVideoSourceConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eAddAudioEncoderConfiguration:
		ret = onvif_AddAudioEncoderConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eAddAudioSourceConfiguration:
		ret = onvif_AddAudioSourceConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eAddPTZConfiguration:
		ret = onvif_AddPTZConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eRemoveVideoEncoderConfiguration:
		ret = onvif_RemoveVideoEncoderConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eRemoveVideoSourceConfiguration:
		ret = onvif_RemoveVideoSourceConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eRemoveAudioEncoderConfiguration:
		ret = onvif_RemoveAudioEncoderConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eRemoveAudioSourceConfiguration:
		ret = onvif_RemoveAudioSourceConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eRemovePTZConfiguration:
		ret = onvif_RemovePTZConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eDeleteProfile:
		ret = onvif_DeleteProfile_rly(p_xml, p_dev, argv);
		break;
		
	case eGetVideoSourceConfigurations:
		ret = onvif_GetVideoSourceConfigurations_rly(p_xml, p_dev, argv);
		break;
		
	case eGetVideoEncoderConfigurations:
		ret = onvif_GetVideoEncoderConfigurations_rly(p_xml, p_dev, argv);
		break;
		
	case eGetAudioSourceConfigurations:
		ret = onvif_GetAudioSourceConfigurations_rly(p_xml, p_dev, argv);
		break;
		
	case eGetAudioEncoderConfigurations:
		ret = onvif_GetAudioEncoderConfigurations_rly(p_xml, p_dev, argv);
		break;
		
	case eGetVideoSourceConfiguration:
		ret = onvif_GetVideoSourceConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eGetVideoEncoderConfiguration:
		ret = onvif_GetVideoEncoderConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eGetAudioSourceConfiguration:
		ret = onvif_GetAudioSourceConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eGetAudioEncoderConfiguration:
		ret = onvif_GetAudioEncoderConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eSetVideoSourceConfiguration:
		ret = onvif_SetVideoSourceConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eSetVideoEncoderConfiguration:
		ret = onvif_SetVideoEncoderConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eSetAudioSourceConfiguration:
		ret = onvif_SetAudioSourceConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eSetAudioEncoderConfiguration:
		ret = onvif_SetAudioEncoderConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eGetVideoSourceConfigurationOptions:
		ret = onvif_GetVideoSourceConfigurationOptions_rly(p_xml, p_dev, argv);
		break;
		
	case eGetVideoEncoderConfigurationOptions:
		ret = onvif_GetVideoEncoderConfigurationOptions_rly(p_xml, p_dev, argv);
		break;
		
	case eGetAudioSourceConfigurationOptions:
		ret = onvif_GetAudioSourceConfigurationOptions_rly(p_xml, p_dev, argv);
		break;
		
	case eGetAudioEncoderConfigurationOptions:
		ret = onvif_GetAudioEncoderConfigurationOptions_rly(p_xml, p_dev, argv);
		break;
		
	case eGetStreamUri:
		ret = onvif_GetStreamUri_rly(p_xml, p_dev, argv);
		break;
		
	case eSetSynchronizationPoint:
		ret = onvif_SetSynchronizationPoint_rly(p_xml, p_dev, argv);
		break;
		
	case eGetSnapshotUri:
		ret = onvif_GetSnapshotUri_rly(p_xml, p_dev, argv);
		break;
		
    case eGetNodes:
    	ret = onvif_GetNodes_rly(p_xml, p_dev, argv);
    	break;
    	
	case eGetNode:
		ret = onvif_GetNode_rly(p_xml, p_dev, argv);
		break;
		
	case eGetPresets:
		ret = onvif_GetPresets_rly(p_xml, p_dev, argv);
		break;
		
	case eSetPreset:
		ret = onvif_SetPreset_rly(p_xml, p_dev, argv);
		break;
		
	case eRemovePreset:
		ret = onvif_RemovePreset_rly(p_xml, p_dev, argv);
		break;
		
	case eGotoPreset:
		ret = onvif_GotoPreset_rly(p_xml, p_dev, argv);
		break;
		
	case eGotoHomePosition:
		ret = onvif_GotoHomePosition_rly(p_xml, p_dev, argv);
		break;
		
	case eSetHomePosition:
		ret = onvif_SetHomePosition_rly(p_xml, p_dev, argv);
		break;
		
	case eGetStatus:
		ret = onvif_GetStatus_rly(p_xml, p_dev, argv);
		break;
		
	case eContinuousMove:
        ret = onvif_ContinuousMove_rly(p_xml, p_dev, argv);
        break;
        
	case eRelativeMove:
		ret = onvif_RelativeMove_rly(p_xml, p_dev, argv);
		break;
		
	case eAbsoluteMove:
		ret = onvif_AbsoluteMove_rly(p_xml, p_dev, argv);
		break;
		
	case ePTZStop:
        ret = onvif_PTZ_Stop_rly(p_xml, p_dev, argv);
        break;
        
	case eGetConfigurations:
        ret = onvif_GetConfigurations_rly(p_xml, p_dev, argv);
        break;
        
	case eGetConfiguration:
		ret = onvif_GetConfiguration_rly(p_xml, p_dev, argv);
		break;
		
	case eSetConfiguration:
		ret = onvif_SetConfiguration_rly(p_xml, p_dev, argv);
		break;
			
	case eGetConfigurationOptions:
        ret = onvif_GetConfigurationOptions_rly(p_xml, p_dev, argv);
        break;
    
	case eGetEventProperties:
        ret = onvif_GetEventProperties_rly(p_xml, p_dev, argv);
        break;
    
	case eRenew:
        ret = onvif_Renew_rly(p_xml, p_dev, argv);
        break;
    
	case eUnsubscribe:
        ret = onvif_Unsubscribe_rly(p_xml, p_dev, argv);
        break;
    
	case eSubscribe:
        ret = onvif_Subscribe_rly(p_xml, p_dev, argv);
        break;
    	
	case eGetImagingSettings:
        ret = onvif_GetImagingSettings_rly(p_xml, p_dev, argv);
        break;
    
	case eSetImagingSettings:
        ret = onvif_SetImagingSettings_rly(p_xml, p_dev, argv);
        break; 

	case eGetOptions:
		ret = onvif_GetOptions_rly(p_xml, p_dev, argv);
        break;

    case eGetReplayUri:
		ret = onvif_GetReplayUri_rly(p_xml, p_dev, argv);
        break;

    case eGetRecordingSummary:
		ret = onvif_GetRecordingSummary_rly(p_xml, p_dev, argv);
        break;

	case eGetRecordingInformation:
    	ret = onvif_GetRecordingInformation_rly(p_xml, p_dev, argv);
        break;
        
	case eGetMediaAttributes:
    	ret = onvif_GetMediaAttributes_rly(p_xml, p_dev, argv);
        break;
        
	case eFindRecordings:
    	ret = onvif_FindRecordings_rly(p_xml, p_dev, argv);
        break;
        
	case eGetRecordingSearchResults:
    	ret = onvif_GetRecordingSearchResults_rly(p_xml, p_dev, argv);
        break;
        
	case eFindEvents:
    	ret = onvif_FindEvents_rly(p_xml, p_dev, argv);
        break;
        
	case eGetEventSearchResults:
    	ret = onvif_GetEventSearchResults_rly(p_xml, p_dev, argv);
        break;
        
	case eGetSearchState:
    	ret = onvif_GetSearchState_rly(p_xml, p_dev, argv);
        break;
        
	case eEndSearch:
    	ret = onvif_EndSearch_rly(p_xml, p_dev, argv);
        break;   
        
	default:
	    break;
	}

	return ret;
}






