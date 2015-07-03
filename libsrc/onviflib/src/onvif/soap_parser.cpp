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
#include "soap_parser.h"
#include "onvif_utils.h"


/***************************************************************************************/

BOOL parse_Bool(const char * pdata)
{    
    if (strcasecmp(pdata, "true") == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 * dn:NetworkVideoTransmitter tds:Device
 */
int parse_DeviceType(const char * pdata)
{
    const char *p1, *p2;
    char type[256];
    
    p2 = pdata;
    p1 = strchr(pdata, ' ');
    while (p1)
    {
        strncpy(type, p2, p1-p2);

        if (soap_strcmp(type, "dn:NetworkVideoTransmitter") == 0 || 
		    soap_strcmp(type, "tds:Device") == 0)
        {
            return ODT_NVT;
        }
        else if (soap_strcmp(type, "dn:NetworkVideoDisplay") == 0)
        {
            return ODT_NVD;
        }
        else if (soap_strcmp(type, "dn:NetworkVideoStorage") == 0)
        {
            return ODT_NVS;
        }
        else if (soap_strcmp(type, "dn:NetworkVideoAnalytics") == 0)
        {
            return ODT_NVA;
        }

        p2 = p1+1;
        p1 = strchr(p2, ' ');
    }

	if (p2)
	{
		if (soap_strcmp(p2, "dn:NetworkVideoTransmitter") == 0 || 
		    soap_strcmp(p2, "tds:Device") == 0)
        {
            return ODT_NVT;
        }
        else if (soap_strcmp(p2, "dn:NetworkVideoDisplay") == 0)
        {
            return ODT_NVD;
        }
        else if (soap_strcmp(p2, "dn:NetworkVideoStorage") == 0)
        {
            return ODT_NVS;
        }
        else if (soap_strcmp(p2, "dn:NetworkVideoAnalytics") == 0)
        {
            return ODT_NVA;
        }
	}

    return ODT_NVT;
}


/**
 * http://192.168.5.235/onvif/device_service
 */
int parse_XAddr_(const char * pdata, ONVIF_XADDR * p_xaddr)
{
    const char *p1, *p2;
    int len = strlen(pdata);

    p_xaddr->port = 80;
    
    if (len > 7) // skip "http://"
    {
        p1 = strchr(pdata+7, ':');
        if (p1)
        {
			char buff[100];

            strncpy(p_xaddr->host, pdata+7, p1-pdata-7);            
            memset(buff, 0, 100);
            
            p2 = strchr(p1, '/');
            if (p2)
            {
                strncpy(p_xaddr->url, p2, len - (p2 - pdata));
                
                len = p2 - p1 - 1;
                strncpy(buff, p1+1, len);                
            }
            else
            {
                len = len - (p1 - pdata);
                strncpy(buff, p1+1, len);
            }  

            p_xaddr->port = atoi(buff);
        }
        else
        {
            p2 = strchr(pdata+7, '/');
            if (p2)
            {
                strncpy(p_xaddr->url, p2, len - (p2 - pdata));
                
                len = p2 - pdata - 7;
                strncpy(p_xaddr->host, pdata+7, len);
            }
            else
            {
                len = len - 7;
                strncpy(p_xaddr->host, pdata+7, len);
            }
        }
    }

    return 1;
}

/**
 * http://192.168.5.235/onvif/device_service http://[fe80::c256:e3ff:fea2:e019]/onvif/device_service 
 */
int parse_XAddr(const char * pdata, ONVIF_XADDR * p_xaddr)
{
    const char *p1, *p2;
    char xaddr[256];

    //lc fix xaddr to 0
    memset(xaddr,0,sizeof(xaddr));

    p2 = pdata;
    p1 = strchr(pdata, ' ');
    while (p1)
    {
        strncpy(xaddr, p2, p1-p2);

        parse_XAddr_(xaddr, p_xaddr);
        if (is_ip_address(p_xaddr->host)) // if ipv4 address
        {
            return 1;
        }

        p2 = p1+1;
        p1 = strchr(p2, ' ');
    }

	if (p2)
	{
		parse_XAddr_(p2, p_xaddr);	
	}

    return 1;
}

int parse_Time(const char * pdata)
{
    return atoi(pdata+2);
}

BOOL parse_XSDDatetime(const char * s, time_t * p)
{
	if (s)
	{ 
		char zone[32];
		struct tm T;
		const char *t;
		
		*zone = '\0';
		memset(&T, 0, sizeof(T));
		
		if (strchr(s, '-'))
		{
			t = "%d-%d-%dT%d:%d:%d%31s";
		}	
		else if (strchr(s, ':'))
		{
			t = "%4d%2d%2dT%d:%d:%d%31s";
		}	
		else /* parse non-XSD-standard alternative ISO 8601 format */
		{
			t = "%4d%2d%2dT%2d%2d%2d%31s";
		}
		
		if (sscanf(s, t, &T.tm_year, &T.tm_mon, &T.tm_mday, &T.tm_hour, &T.tm_min, &T.tm_sec, zone) < 6)
		{
			return FALSE;
		}
		
		if (T.tm_year == 1)
		{
			T.tm_year = 70;
		}	
		else
		{
			T.tm_year -= 1900;
		}
		
		T.tm_mon--;
		
		if (*zone == '.')
		{ 
			for (s = zone + 1; *s; s++)
			{
				if (*s < '0' || *s > '9')
				{
					break;
				}	
			}	
		}
    	else
    	{
      		s = zone;
      	}
      	
		if (*s)
		{
			if (*s == '+' || *s == '-')
			{ 
				int h = 0, m = 0;
				if (s[3] == ':')
				{ 
					/* +hh:mm */
					sscanf(s, "%d:%d", &h, &m);
					if (h < 0)
						m = -m;
				}
				else /* +hhmm */
				{
					m = (int)strtol(s, NULL, 10);
					h = m / 100;
					m = m % 100;
				}
				
				T.tm_min -= m;
				T.tm_hour -= h;
				/* put hour and min in range */
				T.tm_hour += T.tm_min / 60;
				T.tm_min %= 60;
				
				if (T.tm_min < 0)
				{ 
					T.tm_min += 60;
					T.tm_hour--;
				}
				
				T.tm_mday += T.tm_hour / 24;
				T.tm_hour %= 24;
				
				if (T.tm_hour < 0)
				{
					T.tm_hour += 24;
					T.tm_mday--;
				}
				/* note: day of the month may be out of range, timegm() handles it */
			}

			*p = onvif_timegm(&T);
		}
		else /* no UTC or timezone, so assume we got a localtime */
		{ 
			T.tm_isdst = -1;
			*p = mktime(&T);
		}
	}
	
	return TRUE;
}

BOOL parse_XSDDuration(const char *s, int *a)
{ 
	int sign = 1, Y = 0, M = 0, D = 0, H = 0, N = 0, S = 0;
	float f = 0;
	*a = 0;
	if (s)
	{ 
		if (*s == '-')
		{ 
			sign = -1;
			s++;
		}
		if (*s++ != 'P')
			return FALSE;
			
		/* date part */
		while (s && *s)
		{ 
			int n;
			char k;
			if (*s == 'T')
			{ 
				s++;
				break;
			}
			
			if (sscanf(s, "%d%c", &n, &k) != 2)
				return FALSE;
				
			s = strchr(s, k);
			if (!s)
				return FALSE;
				
			switch (k)
			{ 
			case 'Y':
				Y = n;
				break;
				
			case 'M':
				M = n;
				break;
				
			case 'D':
				D = n;
				break;
				
			default:
				return FALSE;
			}
			
			s++;
		}
		
	    /* time part */
	    while (s && *s)
		{ 
			int n;
			char k;
			if (sscanf(s, "%d%c", &n, &k) != 2)
				return FALSE;
				
			s = strchr(s, k);
			if (!s)
				return FALSE;
				
			switch (k)
			{ 
			case 'H':
				H = n;
				break;
				
			case 'M':
				N = n;
				break;
				
			case '.':
				S = n;
				if (sscanf(s, "%g", &f) != 1)
					return FALSE;
				s = NULL;
				continue;
				
			case 'S':
				S = n;
				break;
				
			default:
				return FALSE;
			}
			
			s++;
		}
	    /* convert Y-M-D H:N:S.f to signed long long int */
	    *a = sign * (((((((((((Y * 12) + M) * 30) + D) * 24) + H) * 60) + N) * 60) + S) * 1000);
	}

	return TRUE;
}



/* format : "1.0 2.0" */
BOOL parse_FloatRangeList(const char * p_buff, FLOAT_RANGE * p_range)
{
    int i = 0;
    float min = 0.0, max = 0.0;
    const char * p_buf = p_buff;
    char p_min[32];
    
    // remove space
    while (p_buf) 
    {
        if (*p_buf == ' ') p_buf++;
        else break;
    }
    
    while (p_buf) 
    {
        if (*p_buf == ' ')
        {
            p_buf++;
            break;
        }
        else if (i < 31)
        {
            p_min[i++] = *p_buf;
            p_buf++;
        }
    }

    p_min[i] = '\0';
    min = (float)atof(p_min);

    while (p_buf) 
    {
        if (*p_buf == ' ') p_buf++;
        else break;
    }

    if (p_buf)
    {
        max = (float)atof(p_buf);
    }

    if (min < max)
    {
        p_range->min = min;
        p_range->max = max;

        return TRUE;
    }
    
    return FALSE;
}

BOOL parse_FloatRange(XMLN * p_node, FLOAT_RANGE * p_xrange, FLOAT_RANGE * p_yrange)
{
	XMLN * p_XRange;
	XMLN * p_YRange;

	p_XRange = xml_node_soap_get(p_node, "XRange");
	if (p_XRange)
	{
		XMLN * p_Min;
		XMLN * p_Max;

		p_Min = xml_node_soap_get(p_XRange, "Min");
		if (p_Min && p_Min->data)
		{
			p_xrange->min = (float)atof(p_Min->data);
		}
		
		p_Max = xml_node_soap_get(p_XRange, "Max");
		if (p_Max && p_Max->data)
		{
			p_xrange->max = (float)atof(p_Max->data);
		}
	}

	if (NULL == p_yrange)
	{
		return TRUE;
	}
	
	p_YRange = xml_node_soap_get(p_node, "YRange");
	if (p_YRange)
	{
		XMLN * p_Min;
		XMLN * p_Max;
		
		p_Min = xml_node_soap_get(p_YRange, "Min");
		if (p_Min && p_Min->data)
		{
			p_yrange->min = (float)atof(p_Min->data);
		}
		
		p_Max = xml_node_soap_get(p_YRange, "Max");
		if (p_Max && p_Max->data)
		{
			p_yrange->max = (float)atof(p_Max->data);
		}
	}

	return TRUE;
}

/*format : "JPEG MPEG4 H264 G711 G726 AAC" */
BOOL parse_EncodingList(const char * p_encoding, RECORDING_CAP * p_cap)
{
    int i = 0;
    const char * p_buf = p_encoding;
    char p_buff[32];

    while (p_buf)
    {
        // remove space
        while (p_buf) 
        {
            if (*p_buf == ' ') p_buf++;
            else break;
        }

        i = 0;
        while (p_buf) 
        {
            if (*p_buf == ' ')
            {
                p_buf++;
                break;
            }
            else if (i < 31)
            {
                p_buff[i++] = *p_buf;
                p_buf++;
            }
        }

        p_buff[i] = '\0';

        if (strcasecmp(p_buff, "JPEG") == 0)
        {
            p_cap->JPEG = 1;
        }
        else if (strcasecmp(p_buff, "MPEG4") == 0)
        {
            p_cap->MPEG4 = 1;
        }
        else if (strcasecmp(p_buff, "H264") == 0)
        {
            p_cap->H264 = 1;
        }
        else if (strcasecmp(p_buff, "G711") == 0)
        {
            p_cap->G711 = 1;
        }
        else if (strcasecmp(p_buff, "G726") == 0)
        {
            p_cap->G726 = 1;
        }
        else if (strcasecmp(p_buff, "AAC") == 0)
        {
            p_cap->AAC = 1;
        }
    }
    
    return TRUE;
}

E_VIDEO_ENCODING parse_VideoEncoding(const char * pdata)
{
    if (strcasecmp(pdata, "H264") == 0)
    {
        return VIDEO_ENCODING_H264;
    }
    else if (strcasecmp(pdata, "JPEG") == 0)
    {
        return VIDEO_ENCODING_JPEG;
    }
    else if (strcasecmp(pdata, "MPEG4") == 0)
    {
        return VIDEO_ENCODING_MPEG4;
    }
    
    return VIDEO_ENCODING_H264;
}

E_AUDIO_ENCODING parse_AudioEncoding(const char * pdata)
{
	if (strcasecmp(pdata, "G711") == 0)
    {
        return AUDIO_ENCODING_G711;
    }
    else if (strcasecmp(pdata, "G726") == 0)
    {
        return AUDIO_ENCODING_G726;
    }
    else if (strcasecmp(pdata, "AAC") == 0)
    {
        return AUDIO_ENCODING_AAC;
    }
    
    return AUDIO_ENCODING_G711;
}

E_H264_PROFILE parse_H264Profile(const char * pdata)
{
    if (strcasecmp(pdata, "Baseline") == 0)
    {
        return H264_PROFILE_Baseline;
    }
    else if (strcasecmp(pdata, "High") == 0)
    {
        return H264_PROFILE_High;
    }
    else if (strcasecmp(pdata, "Main") == 0)
    {
        return H264_PROFILE_Main;
    }
    else if (strcasecmp(pdata, "Extended") == 0)
    {
        return H264_PROFILE_Extended;
    }

    return H264_PROFILE_Baseline;
}

E_MPEG4_PROFILE parse_Mpeg4Profile(const char * pdata)
{
    if (strcasecmp(pdata, "SP") == 0)
    {
        return MPEG4_PROFILE_SP;
    }
    else if (strcasecmp(pdata, "ASP") == 0)
    {
        return MPEG4_PROFILE_ASP;
    }

    return MPEG4_PROFILE_SP;
}


/***************************************************************************************/

BOOL parse_DeviceCapabilities(XMLN * p_node, DEVICE_CAP * p_cap)
{
	XMLN * p_XAddr;
	XMLN * p_Network;
	XMLN * p_System;

	p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

	p_Network = xml_node_soap_get(p_node, "Network");
	if (p_Network)
	{
		XMLN * p_IPFilter;
		XMLN * p_ZeroConfiguration;
		XMLN * p_IPVersion6;
		XMLN * p_DynDNS;

		p_IPFilter = xml_node_soap_get(p_Network, "IPFilter");
		if (p_IPFilter && p_IPFilter->data)
		{
			p_cap->IPFilter = parse_Bool(p_IPFilter->data);
		}

		p_ZeroConfiguration = xml_node_soap_get(p_Network, "ZeroConfiguration");
		if (p_ZeroConfiguration && p_ZeroConfiguration->data)
		{
			p_cap->ZeroConfiguration = parse_Bool(p_ZeroConfiguration->data);
		}

		p_IPVersion6 = xml_node_soap_get(p_Network, "IPVersion6");
		if (p_IPVersion6 && p_IPVersion6->data)
		{
			p_cap->IPVersion6 = parse_Bool(p_IPVersion6->data);
		}

		p_DynDNS = xml_node_soap_get(p_Network, "DynDNS");
		if (p_DynDNS && p_DynDNS->data)
		{
			p_cap->DynDNS = parse_Bool(p_DynDNS->data);
		}
	}

    p_System = xml_node_soap_get(p_node, "System");
	if (p_System)
	{
		XMLN * p_DiscoveryResolve;
		XMLN * p_DiscoveryBye;
		XMLN * p_RemoteDiscovery;
		XMLN * p_SystemBackup;
		XMLN * p_SystemLogging;
		XMLN * p_FirmwareUpgrade;

		p_DiscoveryResolve = xml_node_soap_get(p_System, "DiscoveryResolve");
		if (p_DiscoveryResolve && p_DiscoveryResolve->data)
		{
			p_cap->DiscoveryResolve = parse_Bool(p_DiscoveryResolve->data);
		}

		p_DiscoveryBye = xml_node_soap_get(p_System, "DiscoveryBye");
		if (p_DiscoveryBye && p_DiscoveryBye->data)
		{
			p_cap->DiscoveryBye = parse_Bool(p_DiscoveryBye->data);
		}

		p_RemoteDiscovery = xml_node_soap_get(p_System, "RemoteDiscovery");
		if (p_RemoteDiscovery && p_RemoteDiscovery->data)
		{
			p_cap->RemoteDiscovery = parse_Bool(p_RemoteDiscovery->data);
		}

		p_SystemBackup = xml_node_soap_get(p_System, "SystemBackup");
		if (p_SystemBackup && p_SystemBackup->data)
		{
			p_cap->SystemBackup = parse_Bool(p_SystemBackup->data);
		}

		p_SystemLogging = xml_node_soap_get(p_System, "SystemLogging");
		if (p_SystemLogging && p_SystemLogging->data)
		{
			p_cap->SystemLogging = parse_Bool(p_SystemLogging->data);
		}

		p_FirmwareUpgrade = xml_node_soap_get(p_System, "FirmwareUpgrade");
		if (p_FirmwareUpgrade && p_FirmwareUpgrade->data)
		{
			p_cap->FirmwareUpgrade = parse_Bool(p_FirmwareUpgrade->data);
		}
	}

	return TRUE;
}

BOOL parse_EventsCapabilities(XMLN * p_node, EVENT_CAP * p_cap)
{
	XMLN * p_XAddr;
	XMLN * p_WSSubscriptionPolicySupport;
	XMLN * p_WSPullPointSupport;
	XMLN * p_WSPausableSubscriptionManagerInterfaceSupport;

	p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

    p_WSSubscriptionPolicySupport = xml_node_soap_get(p_node, "WSSubscriptionPolicySupport");
    if (p_WSSubscriptionPolicySupport && p_WSSubscriptionPolicySupport->data)
	{
		p_cap->WSSubscriptionPolicySupport = parse_Bool(p_WSSubscriptionPolicySupport->data);
    }

    p_WSPullPointSupport = xml_node_soap_get(p_node, "WSPullPointSupport");
    if (p_WSPullPointSupport && p_WSPullPointSupport->data)
	{
		p_cap->WSPullPointSupport = parse_Bool(p_WSPullPointSupport->data);
    }

    p_WSPausableSubscriptionManagerInterfaceSupport = xml_node_soap_get(p_node, "WSPausableSubscriptionManagerInterfaceSupport");
    if (p_WSPausableSubscriptionManagerInterfaceSupport && p_WSPausableSubscriptionManagerInterfaceSupport->data)
	{
		p_cap->WSPausableSubscriptionManagerInterfaceSupport = parse_Bool(p_WSPausableSubscriptionManagerInterfaceSupport->data);
    }

    return TRUE;
}

BOOL parse_ImageCapabilities(XMLN * p_node, IMAGE_CAP * p_cap)
{
	XMLN * p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

BOOL parse_MediaCapabilities(XMLN * p_node, MEDIA_CAP * p_cap)
{
    XMLN * p_XAddr;
	XMLN * p_StreamingCapabilities;

	p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

    p_StreamingCapabilities = xml_node_soap_get(p_node, "StreamingCapabilities");
    if (p_StreamingCapabilities)
    {
        XMLN * p_RTPMulticast;
		XMLN * p_RTP_TCP;
		XMLN * p_RTP_RTSP_TCP;

		p_RTPMulticast = xml_node_soap_get(p_StreamingCapabilities, "RTPMulticast");
	    if (p_RTPMulticast && p_RTPMulticast->data)
	    {
	        p_cap->RTPMulticast = parse_Bool(p_RTPMulticast->data);
	    }

		p_RTP_TCP = xml_node_soap_get(p_StreamingCapabilities, "RTP_TCP");
	    if (p_RTP_TCP && p_RTP_TCP->data)
	    {
	        p_cap->RTP_TCP = parse_Bool(p_RTP_TCP->data);
	    }

	    p_RTP_RTSP_TCP = xml_node_soap_get(p_StreamingCapabilities, "RTP_RTSP_TCP");
	    if (p_RTP_RTSP_TCP && p_RTP_RTSP_TCP->data)
	    {
	        p_cap->RTP_RTSP_TCP = parse_Bool(p_RTP_RTSP_TCP->data);
	    } 
    }
					
    return TRUE;
}

BOOL parse_PTZCapabilities(XMLN * p_node, PTZ_CAP * p_cap)
{
    XMLN * p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL parse_RecordingCapabilities(XMLN * p_node, RECORDING_CAP * p_cap)
{
    XMLN * p_XAddr;
	XMLN * p_ReceiverSource;
	XMLN * p_MediaProfileSource;
	XMLN * p_DynamicRecordings;
	XMLN * p_DynamicTracks;
	XMLN * p_MaxStringLength;
	
	p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

    p_ReceiverSource = xml_node_soap_get(p_node, "ReceiverSource");
    if (p_ReceiverSource && p_ReceiverSource->data)
    {
        p_cap->ReceiverSource = parse_Bool(p_ReceiverSource->data);
    }

    p_MediaProfileSource = xml_node_soap_get(p_node, "MediaProfileSource");
    if (p_MediaProfileSource && p_MediaProfileSource->data)
    {
        p_cap->MediaProfileSource = parse_Bool(p_MediaProfileSource->data);
    }

    p_DynamicRecordings = xml_node_soap_get(p_node, "DynamicRecordings");
    if (p_DynamicRecordings && p_DynamicRecordings->data)
    {
        p_cap->DynamicRecordings = parse_Bool(p_DynamicRecordings->data);
    }

    p_DynamicTracks = xml_node_soap_get(p_node, "DynamicTracks");
    if (p_DynamicTracks && p_DynamicTracks->data)
    {
        p_cap->DynamicTracks = parse_Bool(p_DynamicTracks->data);
    }

    p_MaxStringLength = xml_node_soap_get(p_node, "MaxStringLength");
    if (p_MaxStringLength && p_MaxStringLength->data)
    {
        p_cap->MaxStringLength = atoi(p_MaxStringLength->data);
    }
    
    return TRUE;
}

BOOL parse_SearchCapabilities(XMLN * p_node, SEARCH_CAP * p_cap)
{
    XMLN * p_XAddr;
	XMLN * p_MetadataSearch;

	p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

    p_MetadataSearch = xml_node_soap_get(p_node, "MetadataSearch");
    if (p_MetadataSearch && p_MetadataSearch->data)
    {
        p_cap->MetadataSearch = parse_Bool(p_MetadataSearch->data);
    }

    return TRUE;
}

BOOL parse_ReplayCapabilities(XMLN * p_node, REPLAY_CAP * p_cap)
{
    XMLN * p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

BOOL parse_VideoSourceConfiguration(XMLN * p_node, VIDEO_SRC_CFG * p_v_src_cfg)
{
    XMLN * p_Name;
	XMLN * p_UseCount;
	XMLN * p_SourceToken;
	XMLN * p_Bounds;

	p_Name = xml_node_soap_get(p_node, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_v_src_cfg->name, p_Name->data, sizeof(p_v_src_cfg->name)-1);
    }
    else
    {
        return FALSE;
    }

    p_UseCount = xml_node_soap_get(p_node, "UseCount");
    if (p_UseCount && p_UseCount->data)
    {
        p_v_src_cfg->use_count = atoi(p_UseCount->data);
    }

    p_SourceToken = xml_node_soap_get(p_node, "SourceToken");
    if (p_SourceToken && p_SourceToken->data)
    {
        strncpy(p_v_src_cfg->source_token, p_SourceToken->data, sizeof(p_v_src_cfg->source_token)-1);
    }
    else
    {
        return FALSE;
    }

    p_Bounds = xml_node_soap_get(p_node, "Bounds");
    if (p_Bounds)
    {
    	const char * p_height;
		const char * p_width;
		const char * p_x;
		const char * p_y;

		p_height = xml_attr_get(p_Bounds, "height");
    	if (p_height)
    	{
        	p_v_src_cfg->height = atoi(p_height);
        }

        p_width = xml_attr_get(p_Bounds, "width");
        if (p_width)
        {
        	p_v_src_cfg->width = atoi(p_width);
        }

        p_x = xml_attr_get(p_Bounds, "x");
        if (p_x)
        {
        	p_v_src_cfg->x = atoi(p_x);
        }

        p_y = xml_attr_get(p_Bounds, "y");
        if (p_y)
        {
        	p_v_src_cfg->y = atoi(p_y);
        }	
    }
    else
    {
        return FALSE;
    }
    
    return TRUE;
}

BOOL parse_AudioSourceConfiguration(XMLN * p_node, AUDIO_SRC_CFG * p_a_src_cfg)
{
    XMLN * p_Name;
	XMLN * p_UseCount;
	XMLN * p_SourceToken;

	p_Name = xml_node_soap_get(p_node, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_a_src_cfg->name, p_Name->data, sizeof(p_a_src_cfg->name)-1);
    }
    else
    {
        return FALSE;
    }

    p_UseCount = xml_node_soap_get(p_node, "UseCount");
    if (p_UseCount && p_UseCount->data)
    {
        p_a_src_cfg->use_count = atoi(p_UseCount->data);
    }

    p_SourceToken = xml_node_soap_get(p_node, "SourceToken");
    if (p_SourceToken && p_SourceToken->data)
    {
        strncpy(p_a_src_cfg->source_token, p_SourceToken->data, sizeof(p_a_src_cfg->source_token)-1);
    }
    else
    {
        return FALSE;
    }
    
    return TRUE;
}


BOOL parse_VideoEncoder(XMLN * p_node, VIDEO_ENCODER * p_v_enc)
{
    XMLN * p_Name;
	XMLN * p_UseCount;
	XMLN * p_Encoding;
	XMLN * p_Resolution;
	XMLN * p_Quality;
	XMLN * p_RateControl;
	XMLN * p_SessionTimeout;
	
	p_Name = xml_node_soap_get(p_node, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_v_enc->name, p_Name->data, sizeof(p_v_enc->name));
    }
    else
    {
        return FALSE;
    }

    p_UseCount = xml_node_soap_get(p_node, "UseCount");
    if (p_UseCount && p_UseCount->data)
    {
        p_v_enc->use_count = atoi(p_UseCount->data);
    }

    p_Encoding = xml_node_soap_get(p_node, "Encoding");
    if (p_Encoding && p_Encoding->data)
    {
        p_v_enc->encoding = parse_VideoEncoding(p_Encoding->data);
    }
    else
    {
        return FALSE;
    }

    p_Resolution = xml_node_soap_get(p_node, "Resolution");
    if (p_Resolution)
    {
        XMLN * p_Width;
		XMLN * p_Height;

		p_Width = xml_node_soap_get(p_Resolution, "Width");
        if (p_Width && p_Width->data)
        {
            p_v_enc->width = atoi(p_Width->data);
        }

        p_Height = xml_node_soap_get(p_Resolution, "Height");
        if (p_Height && p_Height->data)
        {
            p_v_enc->height = atoi(p_Height->data);
        }
    }

    p_Quality = xml_node_soap_get(p_node, "Quality");
    if (p_Quality && p_Quality->data)
    {
        p_v_enc->quality = atoi(p_Quality->data);
    }

    p_RateControl = xml_node_soap_get(p_node, "RateControl");
    if (p_RateControl)
    {
        XMLN * p_FrameRateLimit;
		XMLN * p_EncodingInterval;
		XMLN * p_BitrateLimit;

		p_FrameRateLimit = xml_node_soap_get(p_RateControl, "FrameRateLimit");
        if (p_FrameRateLimit && p_FrameRateLimit->data)
        {
            p_v_enc->framerate_limit = atoi(p_FrameRateLimit->data);
        }

        p_EncodingInterval = xml_node_soap_get(p_RateControl, "EncodingInterval");
        if (p_EncodingInterval && p_EncodingInterval->data)
        {
            p_v_enc->encoding_interval = atoi(p_EncodingInterval->data);
        }

        p_BitrateLimit = xml_node_soap_get(p_RateControl, "BitrateLimit");
        if (p_BitrateLimit && p_BitrateLimit->data)
        {
            p_v_enc->bitrate_limit = atoi(p_BitrateLimit->data);
        }
    }

    if (p_v_enc->encoding == VIDEO_ENCODING_H264)
    {
        XMLN * p_H264 = xml_node_soap_get(p_node, "H264");
        if (p_H264)
        {
            XMLN * p_GovLength;
			XMLN * p_H264Profile;

			p_GovLength = xml_node_soap_get(p_H264, "GovLength");
            if (p_GovLength && p_GovLength->data)
            {
                p_v_enc->gov_len = atoi(p_GovLength->data);
            }

            p_H264Profile = xml_node_soap_get(p_H264, "H264Profile");
            if (p_H264Profile && p_H264Profile->data)
            {
                p_v_enc->profile = parse_H264Profile(p_H264Profile->data);
            }
        }    
    }
    else if (p_v_enc->encoding == VIDEO_ENCODING_MPEG4)
    {
    	XMLN * p_MPEG4 = xml_node_soap_get(p_node, "MPEG4");
        if (p_MPEG4)
        {
            XMLN * p_GovLength;
			XMLN * p_Mpeg4Profile;

			p_GovLength = xml_node_soap_get(p_MPEG4, "GovLength");
            if (p_GovLength && p_GovLength->data)
            {
                p_v_enc->gov_len = atoi(p_GovLength->data);
            }

            p_Mpeg4Profile = xml_node_soap_get(p_MPEG4, "Mpeg4Profile");
            if (p_Mpeg4Profile && p_Mpeg4Profile->data)
            {
                p_v_enc->profile = parse_Mpeg4Profile(p_Mpeg4Profile->data);
            }
        }
    }
    
	p_SessionTimeout = xml_node_soap_get(p_node, "SessionTimeout");
	if (p_SessionTimeout && p_SessionTimeout->data)
	{
	    p_v_enc->session_timeout = parse_Time(p_SessionTimeout->data);
	}
					
    return TRUE;
}

BOOL parse_AudioEncoder(XMLN * p_node, AUDIO_ENCODER * p_a_enc)
{
    XMLN * p_Name;
	XMLN * p_UseCount;
	XMLN * p_Encoding;
	XMLN * p_Bitrate;
	XMLN * p_SampleRate;
	XMLN * p_SessionTimeout;

	p_Name = xml_node_soap_get(p_node, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_a_enc->name, p_Name->data, sizeof(p_a_enc->name));
    }
    else
    {
        return FALSE;
    }

    p_UseCount = xml_node_soap_get(p_node, "UseCount");
    if (p_UseCount && p_UseCount->data)
    {
        p_a_enc->use_count = atoi(p_UseCount->data);
    }

    p_Encoding = xml_node_soap_get(p_node, "Encoding");
    if (p_Encoding && p_Encoding->data)
    {
        p_a_enc->encoding = parse_AudioEncoding(p_Encoding->data);
    }
    else
    {
        return FALSE;
    }

   	p_Bitrate = xml_node_soap_get(p_node, "Bitrate");
    if (p_Bitrate && p_Bitrate->data)
    {
        p_a_enc->bitrate = atoi(p_Bitrate->data);
    }

    p_SampleRate = xml_node_soap_get(p_node, "SampleRate");
    if (p_SampleRate && p_SampleRate->data)
    {
        p_a_enc->sample_rate = atoi(p_SampleRate->data);
    }
    
	p_SessionTimeout = xml_node_soap_get(p_node, "SessionTimeout");
	if (p_SessionTimeout && p_SessionTimeout->data)
	{
	    p_a_enc->session_timeout = parse_Time(p_SessionTimeout->data);
	}
					
    return TRUE;
}


BOOL parse_PTZConfiguration(XMLN * p_node, PTZ_CFG * p_ptz_cfg)
{
	XMLN * p_Name;
	XMLN * p_UseCount;
	XMLN * p_NodeToken;
	XMLN * p_DefaultPTZSpeed;
	XMLN * p_DefaultPTZTimeout;
	XMLN * p_PanTiltLimits;
	XMLN * p_ZoomLimits;

	p_Name = xml_node_soap_get(p_node, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_ptz_cfg->name, p_Name->data, sizeof(p_ptz_cfg->name)-1);
    }
    else
    {
        return FALSE;
    }

    p_UseCount = xml_node_soap_get(p_node, "UseCount");
    if (p_UseCount && p_UseCount->data)
    {
        p_ptz_cfg->use_count = atoi(p_UseCount->data);
    }

    p_NodeToken = xml_node_soap_get(p_node, "NodeToken");
    if (p_NodeToken && p_NodeToken->data)
    {
        strncpy(p_ptz_cfg->node_token, p_NodeToken->data, sizeof(p_ptz_cfg->node_token)-1);
    }

    p_DefaultPTZSpeed = xml_node_soap_get(p_node, "DefaultPTZSpeed");
    if (p_DefaultPTZSpeed)
    {
        XMLN * p_PanTilt;
		XMLN * p_Zoom;
		
		p_PanTilt = xml_node_soap_get(p_DefaultPTZSpeed, "PanTilt");
        if (p_PanTilt)
        {
        	const char * p_x;
			const char * p_y;

			p_x = xml_attr_get(p_PanTilt, "x");
        	if (p_x)
        	{
            	p_ptz_cfg->def_pantilt_speed_x = (float) atof(p_x);
            }

            p_y = xml_attr_get(p_PanTilt, "y");
            if (p_y)
            {
            	p_ptz_cfg->def_pantilt_speed_y = (float) atof(p_y);
            }
        }

        p_Zoom = xml_node_soap_get(p_DefaultPTZSpeed, "Zoom");
        if (p_Zoom)
        {
        	const char * p_x = xml_attr_get(p_Zoom, "x");
        	if (p_x)
        	{
            	p_ptz_cfg->def_zoom_speed = atoi(p_x);
            }	
        }
    }

    p_DefaultPTZTimeout = xml_node_soap_get(p_node, "DefaultPTZTimeout");
	if (p_DefaultPTZTimeout && p_DefaultPTZTimeout->data)
	{
	    p_ptz_cfg->def_timeout = parse_Time(p_DefaultPTZTimeout->data);
	}

	p_PanTiltLimits = xml_node_soap_get(p_node, "PanTiltLimits");
	if (p_PanTiltLimits)
	{
		XMLN * p_Range = xml_node_soap_get(p_PanTiltLimits, "Range");
		if (p_Range)
		{
			parse_FloatRange(p_Range, &p_ptz_cfg->pantilt_limits_x, &p_ptz_cfg->pantilt_limits_y);
		}
	}
	
	p_ZoomLimits = xml_node_soap_get(p_node, "ZoomLimits");
	if (p_ZoomLimits)
	{
		XMLN * p_Range = xml_node_soap_get(p_ZoomLimits, "Range");
		if (p_Range)
		{
			parse_FloatRange(p_Range, &p_ptz_cfg->zoom_limits, NULL);
		}		
	}
				
    return TRUE;
}

BOOL parse_Profile(XMLN * p_node, ONVIF_PROFILE * p_profile)
{
    XMLN * p_Name;
	XMLN * p_VideoSourceConfiguration;
	XMLN * p_AudioSourceConfiguration;
	XMLN * p_VideoEncoderConfiguration;
	XMLN * p_AudioEncoderConfiguration;
	XMLN * p_PTZConfiguration;

	p_Name = xml_node_soap_get(p_node, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_profile->name, p_Name->data, sizeof(p_profile->name)-1);
    }
    else
    {
        return FALSE;
    }    

    p_VideoSourceConfiguration = xml_node_soap_get(p_node, "VideoSourceConfiguration");
    if (p_VideoSourceConfiguration)
    {
        p_profile->video_src_cfg = (VIDEO_SRC_CFG *) malloc(sizeof(VIDEO_SRC_CFG));
        if (p_profile->video_src_cfg)
        {
        	const char * p_token;
			
            memset(p_profile->video_src_cfg, 0, sizeof(VIDEO_SRC_CFG));

            p_token = xml_attr_get(p_VideoSourceConfiguration, "token");
            if (p_token)
            {
            	strncpy(p_profile->video_src_cfg->token, p_token, sizeof(p_profile->video_src_cfg->token)-1);
            }	

            if (!parse_VideoSourceConfiguration(p_VideoSourceConfiguration, p_profile->video_src_cfg))
            {
            	return FALSE;
            }
        }
    }

	p_AudioSourceConfiguration = xml_node_soap_get(p_node, "AudioSourceConfiguration");
    if (p_AudioSourceConfiguration)
    {
        p_profile->audio_src_cfg = (AUDIO_SRC_CFG *) malloc(sizeof(AUDIO_SRC_CFG));
        if (p_profile->audio_src_cfg)
        {
        	const char * p_token;
			
            memset(p_profile->audio_src_cfg, 0, sizeof(AUDIO_SRC_CFG));

            p_token = xml_attr_get(p_AudioSourceConfiguration, "token");  
            if (p_token)
            {
            	strncpy(p_profile->audio_src_cfg->token, p_token, sizeof(p_profile->audio_src_cfg->token)-1);
            }	

            if (!parse_AudioSourceConfiguration(p_AudioSourceConfiguration, p_profile->audio_src_cfg))
            {
            	return FALSE;
            }
        }    
    }
    
    p_VideoEncoderConfiguration = xml_node_soap_get(p_node, "VideoEncoderConfiguration");
    if (p_VideoEncoderConfiguration)
    {
        p_profile->video_enc = (VIDEO_ENCODER *) malloc(sizeof(VIDEO_ENCODER));
        if (p_profile->video_enc)
        { 
        	const char * p_token;
			
            memset(p_profile->video_enc, 0, sizeof(VIDEO_ENCODER));

            p_token = xml_attr_get(p_VideoEncoderConfiguration, "token");
            if (p_token)
            {
            	strncpy(p_profile->video_enc->token, p_token, sizeof(p_profile->video_enc->token)-1);
            }	

            if (!parse_VideoEncoder(p_VideoEncoderConfiguration, p_profile->video_enc))
            {
                return FALSE;           
            }
        }    
    }

	p_AudioEncoderConfiguration = xml_node_soap_get(p_node, "AudioEncoderConfiguration");
    if (p_AudioEncoderConfiguration)
    {
        p_profile->audio_enc = (AUDIO_ENCODER *) malloc(sizeof(AUDIO_ENCODER));
        if (p_profile->audio_enc)
        { 
        	const char * p_token;
			
            memset(p_profile->audio_enc, 0, sizeof(AUDIO_ENCODER));

            p_token = xml_attr_get(p_AudioEncoderConfiguration, "token");
            if (p_token)
            {
            	strncpy(p_profile->audio_enc->token, p_token, sizeof(p_profile->audio_enc->token)-1);
            }	

            if (!parse_AudioEncoder(p_AudioEncoderConfiguration, p_profile->audio_enc))
            {
                return FALSE;           
            }
        }    
    }

    p_PTZConfiguration = xml_node_soap_get(p_node, "PTZConfiguration");
    if (p_PTZConfiguration)
    {
        p_profile->ptz_cfg = (PTZ_CFG *) malloc(sizeof(PTZ_CFG));
        if (p_profile->ptz_cfg)
        { 
        	const char * p_token;
			
            memset(p_profile->ptz_cfg, 0, sizeof(PTZ_CFG));

            p_token = xml_attr_get(p_PTZConfiguration, "token");	
            if (p_token)
            {
            	strncpy(p_profile->ptz_cfg->token, p_token, sizeof(p_profile->ptz_cfg->token)-1);
            }	

            if (!parse_PTZConfiguration(p_PTZConfiguration, p_profile->ptz_cfg))
            {
                return FALSE;           
            }
        }    
    }   

    return TRUE;
}

BOOL parse_NetworkInterface(XMLN * p_node, ONVIF_NET_INF * p_net_inf)
{
	XMLN * p_Enabled;
	XMLN * p_Info;
	XMLN * p_IPv4;

	p_Enabled = xml_node_soap_get(p_node, "Enabled");
    if (p_Enabled && p_Enabled->data)
    {   
    	p_net_inf->enabled = parse_Bool(p_Enabled->data);
    }

    p_Info = xml_node_soap_get(p_node, "Info");
    if (p_Info)
    {
    	XMLN * p_Name;
		XMLN * p_HwAddress;
		XMLN * p_MTU;

		p_Name = xml_node_soap_get(p_Info, "Name");
	    if (p_Name && p_Name->data)
	    {
	    	strncpy(p_net_inf->name, p_Name->data, sizeof(p_net_inf->name)-1);
	    }

	    p_HwAddress = xml_node_soap_get(p_Info, "HwAddress");
	    if (p_HwAddress && p_HwAddress->data)
	    {
	    	strncpy(p_net_inf->hwaddr, p_HwAddress->data, sizeof(p_net_inf->hwaddr)-1);
	    }

	    p_MTU = xml_node_soap_get(p_Info, "MTU");
	    if (p_MTU && p_MTU->data)
	    {
	    	p_net_inf->mtu = atoi(p_MTU->data);
	    }
    }

    p_IPv4 = xml_node_soap_get(p_node, "IPv4");
    if (p_IPv4)
    {
    	XMLN * p_Enabled;
		XMLN * p_Config;

		p_Enabled = xml_node_soap_get(p_IPv4, "Enabled");
	    if (p_Enabled && p_Enabled->data)
	    {   
	    	p_net_inf->ipv4_enabled = parse_Bool(p_Enabled->data);
	    }

	    p_Config = xml_node_soap_get(p_IPv4, "Config");
	    if (p_Config)
	    {   
	    	XMLN * p_Manual;
			XMLN * p_FromDHCP;
			XMLN * p_DHCP;
			
			p_Manual = xml_node_soap_get(p_Config, "Manual");
		    if (p_Manual)
		    {   
		    	XMLN * p_Address;
				XMLN * p_PrefixLength;
				
		    	p_Address = xml_node_soap_get(p_Manual, "Address");
			    if (p_Address && p_Address->data)
			    {   
			    	strncpy(p_net_inf->ipv4_addr, p_Address->data, sizeof(p_net_inf->ipv4_addr)-1);
			    }

			    p_PrefixLength = xml_node_soap_get(p_Manual, "PrefixLength");
			    if (p_PrefixLength && p_PrefixLength->data)
			    {   
			    	p_net_inf->prefix_len = atoi(p_PrefixLength->data);
			    }
		    }

		    p_FromDHCP = xml_node_soap_get(p_Config, "FromDHCP");
		    if (p_FromDHCP)
		    {   
		    	XMLN * p_Address;
				XMLN * p_PrefixLength;

				p_Address = xml_node_soap_get(p_FromDHCP, "Address");
			    if (p_Address && p_Address->data)
			    {   
			    	strncpy(p_net_inf->ipv4_addr, p_Address->data, sizeof(p_net_inf->ipv4_addr)-1);
			    }

			    p_PrefixLength = xml_node_soap_get(p_FromDHCP, "PrefixLength");
			    if (p_PrefixLength && p_PrefixLength->data)
			    {   
			    	p_net_inf->prefix_len = atoi(p_PrefixLength->data);
			    }
		    }

		    p_DHCP = xml_node_soap_get(p_Config, "DHCP");
		    if (p_DHCP && p_DHCP->data)
		    {
		    	p_net_inf->fromdhcp = parse_Bool(p_DHCP->data);
		    }
	    }
    }

    return TRUE;
}


/***************************************************************************************/
BOOL parse_GetCapabilities(XMLN * p_node, GetCapabilities_RES * p_res)
{
    XMLN * p_Capabilities;
	XMLN * p_Device;
	XMLN * p_Events;
	XMLN * p_Imaging;
	XMLN * p_Media;
	XMLN * p_PTZ;
	XMLN * p_Extension;

	p_Capabilities = xml_node_soap_get(p_node, "Capabilities");
    if (NULL == p_Capabilities)
    {
        return FALSE;
    }

    p_Device = xml_node_soap_get(p_Capabilities, "Device");
    if (p_Device)
    {
        parse_DeviceCapabilities(p_Device, &p_res->capablity.device);
    }

	p_Events = xml_node_soap_get(p_Capabilities, "Events");
    if (p_Events)
    {
        if (parse_EventsCapabilities(p_Events, &p_res->capablity.events))
        {
        	p_res->capablity.events.support = 1;
        }
        else
        {
        	p_res->capablity.events.support = 0;
        }
    }

    p_Imaging = xml_node_soap_get(p_Capabilities, "Imaging");
    if (p_Imaging)
    {
        if (parse_ImageCapabilities(p_Imaging, &p_res->capablity.image))
        {
        	p_res->capablity.image.support = 1;
        }
        else
        {
        	p_res->capablity.image.support = 0;
        }
    }
    
    p_Media = xml_node_soap_get(p_Capabilities, "Media");
    if (p_Media)
    {
        if (parse_MediaCapabilities(p_Media, &p_res->capablity.media))
        {
        	p_res->capablity.media.support = 1;
        }
        else
        {
        	p_res->capablity.media.support = 0;
        }
    }

    p_PTZ = xml_node_soap_get(p_Capabilities, "PTZ");
    if (p_PTZ)
    {
        if (parse_PTZCapabilities(p_PTZ, &p_res->capablity.ptz))
        {
        	p_res->capablity.ptz.support = 1;
        }
        else
        {
        	p_res->capablity.ptz.support = 0;
        }
    }

	p_Extension = xml_node_soap_get(p_Capabilities, "Extension");
    if (p_Extension)
    {
        XMLN * p_Recording;
		XMLN * p_Search;
		XMLN * p_Replay;

		p_Recording = xml_node_soap_get(p_Extension, "Recording");
        if (p_Recording)
        {
            if (parse_RecordingCapabilities(p_Recording, &p_res->capablity.recording))
            {
            	p_res->capablity.recording.support = 1;
            }
            else
            {
            	p_res->capablity.recording.support = 0;
            }
        }

        p_Search = xml_node_soap_get(p_Extension, "Search");
        if (p_Search)
        {
            if (parse_SearchCapabilities(p_Search, &p_res->capablity.search))
            {
            	p_res->capablity.search.support = 1;
            }
            else
            {
            	p_res->capablity.search.support = 0;
            }
        }

        p_Replay = xml_node_soap_get(p_Extension, "Replay");
        if (p_Replay)
        {
            if (parse_ReplayCapabilities(p_Replay, &p_res->capablity.replay))
            {
            	p_res->capablity.replay.support = 1;
            }
            else
            {
            	p_res->capablity.replay.support = 0;
            }
        }
    }
    
    return TRUE;
}

BOOL parse_GetDeviceInformation(XMLN * p_node, GetDeviceInformation_RES * p_res)
{
    XMLN * p_Manufacturer;
	XMLN * p_Model;
	XMLN * p_FirmwareVersion;
	XMLN * p_SerialNumber;
	XMLN * p_HardwareId;

	p_Manufacturer = xml_node_soap_get(p_node, "Manufacturer");
    if (p_Manufacturer && p_Manufacturer->data)
    {   
        strncpy(p_res->dev_info.Manufacturer, p_Manufacturer->data, sizeof(p_res->dev_info.Manufacturer)-1);
    }

    p_Model = xml_node_soap_get(p_node, "Model");
    if (p_Model && p_Model->data)
    {   
        strncpy(p_res->dev_info.Model, p_Model->data, sizeof(p_res->dev_info.Model)-1);
    }

    p_FirmwareVersion = xml_node_soap_get(p_node, "FirmwareVersion");
    if (p_FirmwareVersion && p_FirmwareVersion->data)
    {   
        strncpy(p_res->dev_info.FirmwareVersion, p_FirmwareVersion->data, sizeof(p_res->dev_info.FirmwareVersion)-1);
    }

    p_SerialNumber = xml_node_soap_get(p_node, "SerialNumber");
    if (p_SerialNumber && p_SerialNumber->data)
    {   
        strncpy(p_res->dev_info.SerialNumber, p_SerialNumber->data, sizeof(p_res->dev_info.SerialNumber)-1);
    }

    p_HardwareId = xml_node_soap_get(p_node, "HardwareId");
    if (p_HardwareId && p_HardwareId->data)
    {   
        strncpy(p_res->dev_info.HardwareId, p_HardwareId->data, sizeof(p_res->dev_info.HardwareId)-1);
    }	

    return TRUE;
}


BOOL parse_GetStreamUri(XMLN * p_node, GetStreamUri_RES * p_res)
{
    XMLN * p_MediaUri;
	XMLN * p_Uri;
	XMLN * p_InvalidAfterConnect;
	XMLN * p_InvalidAfterReboot;
	XMLN * p_Timeout;

	p_MediaUri = xml_node_soap_get(p_node, "MediaUri");
    if (NULL == p_MediaUri)
    {
    	return FALSE;
    }
    
    p_Uri = xml_node_soap_get(p_MediaUri, "Uri");
    if (p_Uri && p_Uri->data)
    {
    	char * p3;
		
        // rtsp://ip:port1:port2/url, remove "port1:", result rtsp://ip:port2/url
        const char * p = p_Uri->data + 7;
		const char * p1 = strchr(p, ':');
		if (p1)
		{
			const char * p2 = strchr(p1+1, ':');
			if (p2)
			{
				strncpy(p_res->Uri, p_Uri->data, p1 - p_Uri->data);					
				strcat(p_res->Uri, p2);
			}
			else
			{
				strncpy(p_res->Uri, p_Uri->data, sizeof(p_res->Uri)-1);
			}
		}
		else
		{
			strncpy(p_res->Uri, p_Uri->data, sizeof(p_res->Uri)-1);
		}

        // replace "&amp;" with "&"
		p3 = strstr(p_res->Uri, "&amp;");
		while (p3)
		{
			memmove(p3+1, p3+5, strlen(p3+5));
			p3 = strstr(p3+5, "&amp;");
			p_res->Uri[strlen(p_res->Uri)-4] = '\0';
		}
    }
    else
    {
        return FALSE;
    }

	p_InvalidAfterConnect = xml_node_soap_get(p_MediaUri, "InvalidAfterConnect");
    if (p_InvalidAfterConnect && p_InvalidAfterConnect->data)
    {
    	p_res->InvalidAfterConnect = parse_Bool(p_InvalidAfterConnect->data);
    }

    p_InvalidAfterReboot = xml_node_soap_get(p_MediaUri, "InvalidAfterReboot");
    if (p_InvalidAfterReboot && p_InvalidAfterReboot->data)
    {
    	p_res->InvalidAfterReboot = parse_Bool(p_InvalidAfterReboot->data);
    }

	p_Timeout = xml_node_soap_get(p_MediaUri, "Timeout");
    if (p_Timeout && p_Timeout->data)
    {
    	p_res->Timeout = parse_Time(p_Timeout->data);
    }
    
    return TRUE;
}

BOOL parse_GetConfigurationOptions(XMLN * p_node, GetConfigurationOptions_RES * p_res)
{
	XMLN * p_PTZConfigurationOptions;
	XMLN * p_Spaces;
	XMLN * p_AbsolutePanTiltPositionSpace;
	XMLN * p_AbsoluteZoomPositionSpace;
	XMLN * p_RelativePanTiltTranslationSpace;
	XMLN * p_RelativeZoomTranslationSpace;
	XMLN * p_ContinuousPanTiltVelocitySpace;
	XMLN * p_ContinuousZoomVelocitySpace;
	XMLN * p_PanTiltSpeedSpace;
	XMLN * p_ZoomSpeedSpace;
	XMLN * p_PTZTimeout;

	p_PTZConfigurationOptions = xml_node_soap_get(p_node, "PTZConfigurationOptions");
	if (NULL == p_PTZConfigurationOptions)
	{
		return FALSE;
	}

	p_Spaces = xml_node_soap_get(p_PTZConfigurationOptions, "Spaces");
	if (NULL == p_Spaces)
	{
		return FALSE;
	}

	p_AbsolutePanTiltPositionSpace = xml_node_soap_get(p_Spaces, "AbsolutePanTiltPositionSpace");
	if (p_AbsolutePanTiltPositionSpace)
	{
		parse_FloatRange(p_AbsolutePanTiltPositionSpace, &p_res->ptz_cfg_opt.abs_pantilt_x, &p_res->ptz_cfg_opt.abs_pantilt_y);
	}

	p_AbsoluteZoomPositionSpace = xml_node_soap_get(p_Spaces, "AbsoluteZoomPositionSpace");
	if (p_AbsoluteZoomPositionSpace)
	{
		parse_FloatRange(p_AbsoluteZoomPositionSpace, &p_res->ptz_cfg_opt.abs_zoom, NULL);
	}

	p_RelativePanTiltTranslationSpace = xml_node_soap_get(p_Spaces, "RelativePanTiltTranslationSpace");
	if (p_RelativePanTiltTranslationSpace)
	{
		parse_FloatRange(p_RelativePanTiltTranslationSpace, &p_res->ptz_cfg_opt.rel_pantilt_x, &p_res->ptz_cfg_opt.rel_pantilt_y);		
	}

	p_RelativeZoomTranslationSpace = xml_node_soap_get(p_Spaces, "RelativeZoomTranslationSpace");
	if (p_RelativeZoomTranslationSpace)
	{
		parse_FloatRange(p_RelativeZoomTranslationSpace, &p_res->ptz_cfg_opt.rel_zoom, NULL);
	}

	p_ContinuousPanTiltVelocitySpace = xml_node_soap_get(p_Spaces, "ContinuousPanTiltVelocitySpace");
	if (p_ContinuousPanTiltVelocitySpace)
	{
		parse_FloatRange(p_ContinuousPanTiltVelocitySpace, &p_res->ptz_cfg_opt.con_pantilt_x, &p_res->ptz_cfg_opt.con_pantilt_y);	
	}

	p_ContinuousZoomVelocitySpace = xml_node_soap_get(p_Spaces, "ContinuousZoomVelocitySpace");
	if (p_ContinuousZoomVelocitySpace)
	{
		parse_FloatRange(p_ContinuousZoomVelocitySpace, &p_res->ptz_cfg_opt.con_zoom, NULL);
	}

	p_PanTiltSpeedSpace = xml_node_soap_get(p_Spaces, "PanTiltSpeedSpace");
	if (p_PanTiltSpeedSpace)
	{
		parse_FloatRange(p_PanTiltSpeedSpace, &p_res->ptz_cfg_opt.pantile_speed, NULL);
	}

	p_ZoomSpeedSpace = xml_node_soap_get(p_Spaces, "ZoomSpeedSpace");
	if (p_ZoomSpeedSpace)
	{
		parse_FloatRange(p_ZoomSpeedSpace, &p_res->ptz_cfg_opt.zoom_speed, NULL);
	}

	p_PTZTimeout = xml_node_soap_get(p_PTZConfigurationOptions, "PTZTimeout");
	if (p_PTZTimeout)
	{
		XMLN * p_Min;
		XMLN * p_Max;

		p_Min = xml_node_soap_get(p_PTZTimeout, "Min");
		if (p_Min && p_Min->data)
		{
			p_res->ptz_cfg_opt.timeout_range.min = parse_Time(p_Min->data);
		}

		p_Max = xml_node_soap_get(p_PTZTimeout, "Max");
		if (p_Max && p_Max->data)
		{
			p_res->ptz_cfg_opt.timeout_range.max = parse_Time(p_Max->data);
		}
	}
    
	return TRUE;
}

BOOL parse_SetNetworkInterfaces(XMLN * p_node, SetNetworkInterfaces_RES * p_res)
{
	XMLN * p_RebootNeeded = xml_node_soap_get(p_node, "RebootNeeded");
	if (p_RebootNeeded && p_RebootNeeded->data)
	{
		p_res->need_reboot = parse_Bool(p_RebootNeeded->data);
	}

	return TRUE;
}

BOOL parse_SetHostnameFromDHCP(XMLN * p_node, SetHostnameFromDHCP_RES * p_res)
{
	XMLN * p_RebootNeeded = xml_node_soap_get(p_node, "RebootNeeded");
	if (p_RebootNeeded && p_RebootNeeded->data)
	{
		p_res->need_reboot = parse_Bool(p_RebootNeeded->data);
	}

	return TRUE;
}

BOOL parse_GetNTP(XMLN * p_node, GetNTP_RES * p_res)
{
	int i = 0;	
	char node[32];
	XMLN * p_NTPInformation;
	XMLN * p_FromDHCP;
	XMLN * p_NTP;

	p_NTPInformation = xml_node_soap_get(p_node, "NTPInformation");
	if (NULL == p_NTPInformation)
	{
		return FALSE;
	}
	
	p_FromDHCP = xml_node_soap_get(p_NTPInformation, "FromDHCP");
	if (p_FromDHCP && p_FromDHCP->data)
	{
		p_res->ntp.fromdhcp = parse_Bool(p_FromDHCP->data);
	}	

	if (p_res->ntp.fromdhcp)
	{
		strcpy(node, "NTPFromDHCP");
	}
	else
	{
		strcpy(node, "NTPManual");
	}
	
	p_NTP = xml_node_soap_get(p_NTPInformation, node);
	while (p_NTP && soap_strcmp(p_NTP->name, node) == 0)
	{
		XMLN * p_Type;
		XMLN * p_IPv4Address;
		XMLN * p_DNSname;

		p_Type = xml_node_soap_get(p_NTP, "Type");
		if (p_Type && p_Type->data)
		{
			if (strcasecmp(p_Type->data, "IPv4") != 0 && strcasecmp(p_Type->data, "DNS") != 0) // only support ipv4
			{
			    p_NTP = p_NTP->next;
				continue;
			}
		}

		p_IPv4Address = xml_node_soap_get(p_NTP, "IPv4Address");
		if (p_IPv4Address && p_IPv4Address->data)
		{
			if (is_ip_address(p_IPv4Address->data) && i < MAX_NTP_SERVER)
			{
				strncpy(p_res->ntp.server[i], p_IPv4Address->data, sizeof(p_res->ntp.server[i])-1);
				++i;
			}
		}

		p_DNSname = xml_node_soap_get(p_NTP, "DNSname");
		if (p_DNSname && p_DNSname->data)
		{
			if (i < MAX_NTP_SERVER)
			{
				strncpy(p_res->ntp.server[i], p_DNSname->data, sizeof(p_res->ntp.server[i])-1);
				++i;
			}
		}
		
		p_NTP = p_NTP->next;
	}

	return TRUE;
}

BOOL parse_GetHostname(XMLN * p_node, GetHostname_RES * p_res)
{
	XMLN * p_HostnameInformation;
	XMLN * p_FromDHCP;
	XMLN * p_Name;

	p_HostnameInformation = xml_node_soap_get(p_node, "HostnameInformation");
	if (NULL == p_HostnameInformation)
	{
		return FALSE;
	}	
	
	p_FromDHCP = xml_node_soap_get(p_HostnameInformation, "FromDHCP");
	if (p_FromDHCP && p_FromDHCP->data)
	{
		p_res->hostname.fromdhcp = parse_Bool(p_FromDHCP->data);
	}
	
	p_Name = xml_node_soap_get(p_HostnameInformation, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_res->hostname.name, p_Name->data, sizeof(p_res->hostname.name)-1);
	}

	return TRUE;
}

BOOL parse_GetDNS(XMLN * p_node, GetDNS_RES * p_res)
{
	int i = 0;
	char node[32];
	XMLN * p_DNSInformation;
	XMLN * p_FromDHCP;
	XMLN * p_SearchDomain;
	XMLN * p_DNS;

	p_DNSInformation = xml_node_soap_get(p_node, "DNSInformation");
	if (NULL == p_DNSInformation)
	{
		return FALSE;
	}
	
	p_FromDHCP = xml_node_soap_get(p_DNSInformation, "FromDHCP");
	if (p_FromDHCP && p_FromDHCP->data)
	{
		p_res->dns.fromdhcp = parse_Bool(p_FromDHCP->data);
	}
	
	p_SearchDomain = xml_node_soap_get(p_DNSInformation, "SearchDomain");
	while (p_SearchDomain && soap_strcmp(p_SearchDomain->name, "SearchDomain") == 0)
	{
		if (i < MAX_SEARCHDOMAIN)
		{
			strncpy(p_res->dns.searchdomain[i], p_SearchDomain->data, sizeof(p_res->dns.searchdomain[i])-1);
			++i;
		}

		p_SearchDomain = p_SearchDomain->next;
	}
	
	i = 0;

	if (p_res->dns.fromdhcp)
	{
		strcpy(node, "DNSFromDHCP");
	}
	else
	{
		strcpy(node, "DNSManual");
	}
	
	p_DNS = xml_node_soap_get(p_DNSInformation, node);
	while (p_DNS && soap_strcmp(p_DNS->name, node) == 0)
	{
		XMLN * p_Type;
		XMLN * p_IPv4Address;

		p_Type = xml_node_soap_get(p_DNS, "Type");
		if (p_Type && p_Type->data)
		{
			if (strcasecmp(p_Type->data, "IPv4") != 0) // only support ipv4
			{
			    p_DNS = p_DNS->next;
				continue;
			}
		}

		p_IPv4Address = xml_node_soap_get(p_DNS, "IPv4Address");
		if (p_IPv4Address && p_IPv4Address->data)
		{
			if (is_ip_address(p_IPv4Address->data) && i < MAX_NTP_SERVER)
			{
				strncpy(p_res->dns.server[i], p_IPv4Address->data, sizeof(p_res->dns.server[i])-1);
				++i;
			}
		}
		
		p_DNS = p_DNS->next;
	}

	return TRUE;
}

BOOL parse_GetDynamicDNS(XMLN * p_node, GetDynamicDNS_RES * p_res)
{
	XMLN * p_DynamicDNSInformation;
	XMLN * p_Type;
	XMLN * p_Name;
	XMLN * p_TTL;

	p_DynamicDNSInformation = xml_node_soap_get(p_node, "DynamicDNSInformation");
	if (NULL == p_DynamicDNSInformation)
	{
		return FALSE;
	}

	p_Type = xml_node_soap_get(p_DynamicDNSInformation, "Type");
	if (p_Type && p_Type->data)
	{
		p_res->ddns.type = onvif_get_ddns_type(p_Type->data);
	}

	p_Name = xml_node_soap_get(p_DynamicDNSInformation, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_res->ddns.name, p_Name->data, sizeof(p_res->ddns.name)-1);
	}

	p_TTL = xml_node_soap_get(p_DynamicDNSInformation, "TTL");
	if (p_TTL && p_TTL->data)
	{
		p_res->ddns.ttl = atoi(p_TTL->data);
	}

	return TRUE;
}


BOOL parse_GetNetworkProtocols(XMLN * p_node, GetNetworkProtocols_RES * p_res)
{
	char name[32];
	BOOL enable;
	int  port[MAX_SERVER_PORT];
	
	XMLN * p_NetworkProtocols = xml_node_soap_get(p_node, "NetworkProtocols");
	while (p_NetworkProtocols && soap_strcmp(p_NetworkProtocols->name, "NetworkProtocols") == 0)
	{
		int i = 0;
		XMLN * p_Name;
		XMLN * p_Enabled;
		XMLN * p_Port;
		
		enable = FALSE;
		memset(name, 0, sizeof(name));
		memset(port, 0, sizeof(int)*MAX_SERVER_PORT);
		
		p_Name = xml_node_soap_get(p_NetworkProtocols, "Name");
		if (p_Name && p_Name->data)
		{
			strncpy(name, p_Name->data, sizeof(name)-1);
		}

		p_Enabled = xml_node_soap_get(p_NetworkProtocols, "Enabled");
		if (p_Enabled && p_Enabled->data)
		{
			enable = parse_Bool(p_Enabled->data);
		}
		
		p_Port = xml_node_soap_get(p_NetworkProtocols, "Port");
		while (p_Port && p_Port->data && soap_strcmp(p_Port->name, "Port") == 0)
		{
			if (i < MAX_SERVER_PORT)
			{
				port[i++] = atoi(p_Port->data);
			}
			
			p_Port = p_Port->next;
		}

		if (strcasecmp(name, "HTTP") == 0)
		{
			p_res->protocols.http_support = 1;
			p_res->protocols.http_enable = enable;
			memcpy(p_res->protocols.http_port, port, sizeof(int)*MAX_SERVER_PORT);
		}
		else if (strcasecmp(name, "HTTPS") == 0)
		{
			p_res->protocols.https_support = 1;
			p_res->protocols.https_enable = enable;
			memcpy(p_res->protocols.https_port, port, sizeof(int)*MAX_SERVER_PORT);
		}
		else if (strcasecmp(name, "RTSP") == 0)
		{
			p_res->protocols.rtsp_support = 1;
			p_res->protocols.rtsp_enable = enable;
			memcpy(p_res->protocols.rtsp_port, port, sizeof(int)*MAX_SERVER_PORT);
		}
		
		p_NetworkProtocols = p_NetworkProtocols->next;
	}

	return TRUE;
}

BOOL parse_GetDiscoveryMode(XMLN * p_node, GetDiscoveryMode_RES * p_res)
{
	XMLN * p_DiscoveryMode = xml_node_soap_get(p_node, "DiscoveryMode");
	if (p_DiscoveryMode && p_DiscoveryMode->data)
	{
		if (strcasecmp(p_DiscoveryMode->data, "Discoverable") == 0)
		{
			p_res->discoverable = TRUE;
		}
	}

	return TRUE;
}

BOOL parse_GetNetworkDefaultGateway(XMLN * p_node, GetNetworkDefaultGateway_RES * p_res)
{
	int i = 0;
	XMLN * p_NetworkGateway;
	XMLN * p_IPv4Address;

	p_NetworkGateway = xml_node_soap_get(p_node, "NetworkGateway");
	if (NULL == p_NetworkGateway)
	{
		return FALSE;
	}

	p_IPv4Address = xml_node_soap_get(p_NetworkGateway, "IPv4Address");
	while (p_IPv4Address && p_IPv4Address->data && soap_strcmp(p_IPv4Address->name, "IPv4Address") == 0)
	{
		if (is_ip_address(p_IPv4Address->data) && i < MAX_GATEWAY)
		{
			strncpy(p_res->gateway[i++], p_IPv4Address->data, sizeof(p_res->gateway[0])-1);
		}
		
		p_IPv4Address = p_IPv4Address->next;
	}

	return TRUE;
}

BOOL parse_GetSnapshotUri(XMLN * p_node, GetSnapshotUri_RES * p_res)
{
    XMLN * p_MediaUri;
	XMLN * p_Uri;
	XMLN * p_InvalidAfterConnect;
	XMLN * p_InvalidAfterReboot;
	XMLN * p_Timeout;

	p_MediaUri = xml_node_soap_get(p_node, "MediaUri");
    if (NULL == p_MediaUri)
    {
    	return FALSE;
    }
    
    p_Uri = xml_node_soap_get(p_MediaUri, "Uri");
    if (p_Uri && p_Uri->data)
    {
        strncpy(p_res->Uri, p_Uri->data, sizeof(p_res->Uri));
    }
    else
    {
        return FALSE;
    }

	p_InvalidAfterConnect = xml_node_soap_get(p_MediaUri, "InvalidAfterConnect");
    if (p_InvalidAfterConnect && p_InvalidAfterConnect->data)
    {
    	p_res->InvalidAfterConnect = parse_Bool(p_InvalidAfterConnect->data);
    }

    p_InvalidAfterReboot = xml_node_soap_get(p_MediaUri, "InvalidAfterReboot");
    if (p_InvalidAfterReboot && p_InvalidAfterReboot->data)
    {
    	p_res->InvalidAfterReboot = parse_Bool(p_InvalidAfterReboot->data);
    }

	p_Timeout = xml_node_soap_get(p_MediaUri, "Timeout");
    if (p_Timeout && p_Timeout->data)
    {
    	p_res->Timeout = parse_Time(p_Timeout->data);
    }
    
    return TRUE;
}

BOOL parse_GetVideoEncoderConfigurationOptions(XMLN * p_node, GetVideoEncoderConfigurationOptions_RES * p_res)
{
	XMLN * p_Options;
	XMLN * p_QualityRange;
	XMLN * p_JPEG;
	XMLN * p_MPEG4;
	XMLN * p_H264;

	p_Options = xml_node_soap_get(p_node, "Options");
	if (NULL == p_Options)
	{
		return FALSE;
	}

	p_QualityRange = xml_node_soap_get(p_Options, "QualityRange");
	if (p_QualityRange)
	{
		XMLN * p_Min;
		XMLN * p_Max;

		p_Min = xml_node_soap_get(p_QualityRange, "Min");
		if (p_Min && p_Min->data)
		{
			p_res->video_enc_cfg.quality_min = atoi(p_Min->data);
		}

		p_Max = xml_node_soap_get(p_QualityRange, "Max");
		if (p_Max && p_Max->data)
		{
			p_res->video_enc_cfg.quality_max = atoi(p_Max->data);
		}
	}

	p_JPEG = xml_node_soap_get(p_Options, "JPEG");
	if (p_JPEG)
	{
		int	i = 0;		
		XMLN * p_ResolutionsAvailable;
		XMLN * p_FrameRateRange;
		XMLN * p_EncodingIntervalRange;

		p_ResolutionsAvailable = xml_node_soap_get(p_JPEG, "ResolutionsAvailable");
		while (p_ResolutionsAvailable && soap_strcmp(p_ResolutionsAvailable->name, "ResolutionsAvailable") == 0)
		{
			XMLN * p_Width;
			XMLN * p_Height;

			p_Width = xml_node_soap_get(p_ResolutionsAvailable, "Width");
			if (p_Width && p_Width->data)
			{
				p_res->video_enc_cfg.jpeg_opt.resolution[i].w = atoi(p_Width->data);
			}

			p_Height = xml_node_soap_get(p_ResolutionsAvailable, "Height");
			if (p_Height && p_Height->data)
			{
				p_res->video_enc_cfg.jpeg_opt.resolution[i].h = atoi(p_Height->data);
			}

			i++;			
			p_ResolutionsAvailable = p_ResolutionsAvailable->next;
		}

		p_FrameRateRange = xml_node_soap_get(p_JPEG, "FrameRateRange");
		if (p_FrameRateRange)
		{
			XMLN * p_Min;
			XMLN * p_Max;

			p_Min = xml_node_soap_get(p_FrameRateRange, "Min");
			if (p_Min && p_Min->data)
			{
				p_res->video_enc_cfg.jpeg_opt.frame_rate_min = atoi(p_Min->data);
			}

			p_Max = xml_node_soap_get(p_FrameRateRange, "Max");
			if (p_Max && p_Max->data)
			{
				p_res->video_enc_cfg.jpeg_opt.frame_rate_max = atoi(p_Max->data);
			}
		}

		p_EncodingIntervalRange = xml_node_soap_get(p_JPEG, "EncodingIntervalRange");
		if (p_EncodingIntervalRange)
		{
			XMLN * p_Min;
			XMLN * p_Max;

			p_Min = xml_node_soap_get(p_EncodingIntervalRange, "Min");
			if (p_Min && p_Min->data)
			{
				p_res->video_enc_cfg.jpeg_opt.encoding_interval_min = atoi(p_Min->data);
			}

			p_Max = xml_node_soap_get(p_EncodingIntervalRange, "Max");
			if (p_Max && p_Max->data)
			{
				p_res->video_enc_cfg.jpeg_opt.encoding_interval_max = atoi(p_Max->data);
			}
		}
	}

	p_MPEG4 = xml_node_soap_get(p_Options, "MPEG4");
	if (p_MPEG4)
	{
		int	i = 0;		
		XMLN * p_ResolutionsAvailable;
		XMLN * p_GovLengthRange;
		XMLN * p_FrameRateRange;
		XMLN * p_EncodingIntervalRange;
		XMLN * p_Mpeg4ProfilesSupported;

		p_ResolutionsAvailable = xml_node_soap_get(p_MPEG4, "ResolutionsAvailable");
		while (p_ResolutionsAvailable && soap_strcmp(p_ResolutionsAvailable->name, "ResolutionsAvailable") == 0)
		{
			XMLN * p_Width;
			XMLN * p_Height;

			p_Width = xml_node_soap_get(p_ResolutionsAvailable, "Width");
			if (p_Width && p_Width->data)
			{
				p_res->video_enc_cfg.mpeg4_opt.resolution[i].w = atoi(p_Width->data);
			}

			p_Height = xml_node_soap_get(p_ResolutionsAvailable, "Height");
			if (p_Height && p_Height->data)
			{
				p_res->video_enc_cfg.mpeg4_opt.resolution[i].h = atoi(p_Height->data);
			}

			i++;			
			p_ResolutionsAvailable = p_ResolutionsAvailable->next;
		}

		p_GovLengthRange = xml_node_soap_get(p_MPEG4, "GovLengthRange");
		if (p_GovLengthRange)
		{
			XMLN * p_Min;
			XMLN * p_Max;

			p_Min = xml_node_soap_get(p_GovLengthRange, "Min");
			if (p_Min && p_Min->data)
			{
				p_res->video_enc_cfg.mpeg4_opt.gov_length_min = atoi(p_Min->data);
			}

			p_Max = xml_node_soap_get(p_GovLengthRange, "Max");
			if (p_Max && p_Max->data)
			{
				p_res->video_enc_cfg.mpeg4_opt.gov_length_max = atoi(p_Max->data);
			}
		}
		
		p_FrameRateRange = xml_node_soap_get(p_MPEG4, "FrameRateRange");
		if (p_FrameRateRange)
		{
			XMLN * p_Min;
			XMLN * p_Max;
			
			p_Min = xml_node_soap_get(p_FrameRateRange, "Min");
			if (p_Min && p_Min->data)
			{
				p_res->video_enc_cfg.mpeg4_opt.frame_rate_min = atoi(p_Min->data);
			}

			p_Max = xml_node_soap_get(p_FrameRateRange, "Max");
			if (p_Max && p_Max->data)
			{
				p_res->video_enc_cfg.mpeg4_opt.frame_rate_max = atoi(p_Max->data);
			}
		}

		p_EncodingIntervalRange = xml_node_soap_get(p_MPEG4, "EncodingIntervalRange");
		if (p_EncodingIntervalRange)
		{
			XMLN * p_Min;
			XMLN * p_Max;
			
			p_Min = xml_node_soap_get(p_EncodingIntervalRange, "Min");
			if (p_Min && p_Min->data)
			{
				p_res->video_enc_cfg.mpeg4_opt.encoding_interval_min = atoi(p_Min->data);
			}

			p_Max = xml_node_soap_get(p_EncodingIntervalRange, "Max");
			if (p_Max && p_Max->data)
			{
				p_res->video_enc_cfg.mpeg4_opt.encoding_interval_max = atoi(p_Max->data);
			}
		}

		p_Mpeg4ProfilesSupported = xml_node_soap_get(p_MPEG4, "Mpeg4ProfilesSupported");
		while (p_Mpeg4ProfilesSupported && soap_strcmp(p_Mpeg4ProfilesSupported->name, "Mpeg4ProfilesSupported") == 0)
		{
			if (strcasecmp(p_Mpeg4ProfilesSupported->data, "SP") == 0)
			{
				p_res->video_enc_cfg.mpeg4_opt.sp_profile = 1;
			}
			else if (strcasecmp(p_Mpeg4ProfilesSupported->data, "ASP") == 0)
			{
				p_res->video_enc_cfg.mpeg4_opt.asp_profile = 1;
			}
			
			p_Mpeg4ProfilesSupported = p_Mpeg4ProfilesSupported->next;
		}
	}	

	p_H264 = xml_node_soap_get(p_Options, "H264");
	if (p_H264)
	{
		int	i = 0;		
		XMLN * p_ResolutionsAvailable;
		XMLN * p_GovLengthRange;
		XMLN * p_FrameRateRange;
		XMLN * p_EncodingIntervalRange;
		XMLN * p_H264ProfilesSupported;

		p_ResolutionsAvailable = xml_node_soap_get(p_H264, "ResolutionsAvailable");
		while (p_ResolutionsAvailable && soap_strcmp(p_ResolutionsAvailable->name, "ResolutionsAvailable") == 0)
		{
			XMLN * p_Width;
			XMLN * p_Height;

			p_Width = xml_node_soap_get(p_ResolutionsAvailable, "Width");
			if (p_Width && p_Width->data)
			{
				p_res->video_enc_cfg.h264_opt.resolution[i].w = atoi(p_Width->data);
			}

			p_Height = xml_node_soap_get(p_ResolutionsAvailable, "Height");
			if (p_Height && p_Height->data)
			{
				p_res->video_enc_cfg.h264_opt.resolution[i].h = atoi(p_Height->data);
			}

			i++;			
			p_ResolutionsAvailable = p_ResolutionsAvailable->next;
		}

		p_GovLengthRange = xml_node_soap_get(p_H264, "GovLengthRange");
		if (p_GovLengthRange)
		{
			XMLN * p_Min;
			XMLN * p_Max;
			
			p_Min = xml_node_soap_get(p_GovLengthRange, "Min");
			if (p_Min && p_Min->data)
			{
				p_res->video_enc_cfg.h264_opt.gov_length_min = atoi(p_Min->data);
			}

			p_Max = xml_node_soap_get(p_GovLengthRange, "Max");
			if (p_Max && p_Max->data)
			{
				p_res->video_enc_cfg.h264_opt.gov_length_max = atoi(p_Max->data);
			}
		}
		
		p_FrameRateRange = xml_node_soap_get(p_H264, "FrameRateRange");
		if (p_FrameRateRange)
		{
			XMLN * p_Min;
			XMLN * p_Max;
			
			p_Min = xml_node_soap_get(p_FrameRateRange, "Min");
			if (p_Min && p_Min->data)
			{
				p_res->video_enc_cfg.h264_opt.frame_rate_min = atoi(p_Min->data);
			}

			p_Max = xml_node_soap_get(p_FrameRateRange, "Max");
			if (p_Max && p_Max->data)
			{
				p_res->video_enc_cfg.h264_opt.frame_rate_max = atoi(p_Max->data);
			}
		}

		p_EncodingIntervalRange = xml_node_soap_get(p_H264, "EncodingIntervalRange");
		if (p_EncodingIntervalRange)
		{
			XMLN * p_Min;
			XMLN * p_Max;
			
			p_Min = xml_node_soap_get(p_EncodingIntervalRange, "Min");
			if (p_Min && p_Min->data)
			{
				p_res->video_enc_cfg.h264_opt.encoding_interval_min = atoi(p_Min->data);
			}

			p_Max = xml_node_soap_get(p_EncodingIntervalRange, "Max");
			if (p_Max && p_Max->data)
			{
				p_res->video_enc_cfg.h264_opt.encoding_interval_max = atoi(p_Max->data);
			}
		}

		p_H264ProfilesSupported = xml_node_soap_get(p_H264, "H264ProfilesSupported");
		while (p_H264ProfilesSupported && soap_strcmp(p_H264ProfilesSupported->name, "H264ProfilesSupported") == 0)
		{
			if (strcasecmp(p_H264ProfilesSupported->data, "Baseline") == 0)
			{
				p_res->video_enc_cfg.h264_opt.baseline_profile = 1;
			}
			else if (strcasecmp(p_H264ProfilesSupported->data, "Main") == 0)
			{
				p_res->video_enc_cfg.h264_opt.main_profile = 1;
			}
			else if (strcasecmp(p_H264ProfilesSupported->data, "High") == 0)
			{
				p_res->video_enc_cfg.h264_opt.high_profile = 1;
			}
			else if (strcasecmp(p_H264ProfilesSupported->data, "Extended") == 0)
			{
				p_res->video_enc_cfg.h264_opt.extended_profile = 1;
			}
			
			p_H264ProfilesSupported = p_H264ProfilesSupported->next;
		}
	}	

	return TRUE;
}

BOOL parse_PTZNode(XMLN * p_node, PTZ_NODE * p_ptz_node)
{
	XMLN * p_Name;
	XMLN * p_SupportedPTZSpaces;
	XMLN * p_HomeSupported;

	p_Name = xml_node_soap_get(p_node, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_ptz_node->name, p_Name->data, sizeof(p_ptz_node->name)-1);
	}

	p_SupportedPTZSpaces = xml_node_soap_get(p_node, "SupportedPTZSpaces");
	if (p_SupportedPTZSpaces)
	{
		XMLN * p_AbsolutePanTiltPositionSpace;
		XMLN * p_AbsoluteZoomPositionSpace;
		XMLN * p_RelativePanTiltTranslationSpace;
		XMLN * p_RelativeZoomTranslationSpace;
		XMLN * p_ContinuousPanTiltVelocitySpace;
		XMLN * p_ContinuousZoomVelocitySpace;
		XMLN * p_PanTiltSpeedSpace;
		XMLN * p_ZoomSpeedSpace;

		p_AbsolutePanTiltPositionSpace = xml_node_soap_get(p_SupportedPTZSpaces, "AbsolutePanTiltPositionSpace");
		if (p_AbsolutePanTiltPositionSpace)
		{
			p_ptz_node->abs_pantilt_space = 1;
			
			parse_FloatRange(p_AbsolutePanTiltPositionSpace, &p_ptz_node->abs_pantilt_x, &p_ptz_node->abs_pantilt_y);
		}

		p_AbsoluteZoomPositionSpace = xml_node_soap_get(p_SupportedPTZSpaces, "AbsoluteZoomPositionSpace");
		if (p_AbsoluteZoomPositionSpace)
		{
			p_ptz_node->abs_zoom_space = 1;
			
			parse_FloatRange(p_AbsoluteZoomPositionSpace, &p_ptz_node->abs_zoom, NULL);
		}

		p_RelativePanTiltTranslationSpace = xml_node_soap_get(p_SupportedPTZSpaces, "RelativePanTiltTranslationSpace");
		if (p_RelativePanTiltTranslationSpace)
		{
			p_ptz_node->rel_pantilt_space = 1;
			
			parse_FloatRange(p_RelativePanTiltTranslationSpace, &p_ptz_node->rel_pantilt_x, &p_ptz_node->rel_pantilt_y);
		}

		p_RelativeZoomTranslationSpace = xml_node_soap_get(p_SupportedPTZSpaces, "RelativeZoomTranslationSpace");
		if (p_RelativeZoomTranslationSpace)
		{
			p_ptz_node->rel_zoom_space = 1;
			
			parse_FloatRange(p_RelativeZoomTranslationSpace, &p_ptz_node->rel_zoom, NULL);
		}

		p_ContinuousPanTiltVelocitySpace = xml_node_soap_get(p_SupportedPTZSpaces, "ContinuousPanTiltVelocitySpace");
		if (p_ContinuousPanTiltVelocitySpace)
		{
			p_ptz_node->con_pantilt_space = 1;
			
			parse_FloatRange(p_ContinuousPanTiltVelocitySpace, &p_ptz_node->con_pantilt_x, &p_ptz_node->con_pantilt_y);
		}

		p_ContinuousZoomVelocitySpace = xml_node_soap_get(p_SupportedPTZSpaces, "ContinuousZoomVelocitySpace");
		if (p_ContinuousZoomVelocitySpace)
		{
			p_ptz_node->con_zoom_space = 1;
			
			parse_FloatRange(p_ContinuousZoomVelocitySpace, &p_ptz_node->con_zoom, NULL);
		}

		p_PanTiltSpeedSpace = xml_node_soap_get(p_SupportedPTZSpaces, "PanTiltSpeedSpace");
		if (p_PanTiltSpeedSpace)
		{
			p_ptz_node->pantile_speed_space = 1;
			
			parse_FloatRange(p_PanTiltSpeedSpace, &p_ptz_node->pantile_speed, NULL);
		}

		p_ZoomSpeedSpace = xml_node_soap_get(p_SupportedPTZSpaces, "ZoomSpeedSpace");
		if (p_ZoomSpeedSpace)
		{
			p_ptz_node->zoom_speed_space = 1;
			
			parse_FloatRange(p_ZoomSpeedSpace, &p_ptz_node->zoom_speed, NULL);
		}
	}

	p_HomeSupported = xml_node_soap_get(p_node, "HomeSupported");
	if (p_HomeSupported && p_HomeSupported->data)
	{
		p_ptz_node->home_support = parse_Bool(p_HomeSupported->data);
	}
	
	return TRUE;
}

BOOL parse_Preset(XMLN * p_node, PTZ_PRESET * p_preset)
{
	XMLN * p_Name;
	XMLN * p_PTZPosition;

	p_Name = xml_node_soap_get(p_node, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_preset->name, p_Name->data, sizeof(p_preset->name)-1);
	}

	p_PTZPosition = xml_node_soap_get(p_node, "PTZPosition");
	if (p_PTZPosition)
	{
		XMLN * p_PanTilt;
		XMLN * p_Zoom;

		p_PanTilt = xml_node_soap_get(p_PTZPosition, "PanTilt");
		if (p_PanTilt)
		{
			const char * p_x;
			const char * p_y;

			p_x = xml_attr_get(p_PanTilt, "x");
			if (p_x)
			{
				p_preset->pantilt_pos_x = (float)atof(p_x);
			}

			p_y = xml_attr_get(p_PanTilt, "y");
			if (p_y)
			{
				p_preset->pantilt_pos_y = (float)atof(p_y);
			}
		}

		p_Zoom = xml_node_soap_get(p_PTZPosition, "Zoom");
		if (p_Zoom)
		{
			const char * p_x = xml_attr_get(p_Zoom, "x");
			if (p_x)
			{
				p_preset->zoom_pos = (float)atof(p_x);
			}
		}
	}

	return TRUE;
}

BOOL parse_SetPreset(XMLN * p_node, SetPreset_RES * p_res)
{
	XMLN * p_PresetToken = xml_node_soap_get(p_node, "PresetToken");
	if (p_PresetToken && p_PresetToken->data)
	{
		strncpy(p_res->preset_token, p_PresetToken->data, sizeof(p_res->preset_token)-1);
	}	

	return TRUE;
}

BOOL parse_GetStatus(XMLN * p_node, GetStatus_RES * p_res)
{	
	XMLN * p_PTZStatus;
	XMLN * p_Position;
	XMLN * p_MoveStatus;
	XMLN * p_Error;

	p_PTZStatus = xml_node_soap_get(p_node, "PTZStatus");
	if (NULL == p_PTZStatus)
	{
		return FALSE;
	}

	p_Position = xml_node_soap_get(p_PTZStatus, "Position");
	if (p_Position)
	{
		XMLN * p_PanTilt;
		XMLN * p_Zoom;

		p_PanTilt = xml_node_soap_get(p_Position, "PanTilt");
		if (p_PanTilt)
		{
			const char * p_x;
			const char * p_y;

			p_x = xml_attr_get(p_PanTilt, "x");
			if (p_x)
			{
				p_res->status.pantilt_pos_x = (float)atof(p_x);
			}

			p_y = xml_attr_get(p_PanTilt, "y");
			if (p_y)
			{
				p_res->status.pantilt_pos_y = (float)atof(p_y);
			}
		}

		p_Zoom = xml_node_soap_get(p_Position, "Zoom");
		if (p_Zoom)
		{
			const char * p_x = xml_attr_get(p_Zoom, "x");
			if (p_x)
			{
				p_res->status.zoom_pos = (float)atof(p_x);
			}
		}
	}

	p_MoveStatus = xml_node_soap_get(p_PTZStatus, "MoveStatus");
	if (p_MoveStatus)
	{
		XMLN * p_PanTilt;
		XMLN * p_Zoom;

		p_PanTilt = xml_node_soap_get(p_MoveStatus, "PanTilt");
		if (p_PanTilt && p_PanTilt->data)
		{
			p_res->status.move_sta = onvif_get_ptz_status(p_PanTilt->data);
		}

		p_Zoom = xml_node_soap_get(p_MoveStatus, "Zoom");
		if (p_Zoom && p_Zoom->data)
		{
			p_res->status.zoom_sta = onvif_get_ptz_status(p_Zoom->data);
		}
	}

	p_Error = xml_node_soap_get(p_PTZStatus, "Error");
	if (p_Error && p_Error->data)
	{
		strncpy(p_res->status.error, p_Error->data, sizeof(p_res->status.error)-1);
	}

	return TRUE;
}

BOOL parse_ImagingSettings(XMLN * p_node, IMAGE_CFG * p_img_cfg)
{
    XMLN * p_BacklightCompensation;
	XMLN * p_Brightness;
	XMLN * p_ColorSaturation;
	XMLN * p_Contrast;
	XMLN * p_Exposure;
	XMLN * p_IrCutFilter;
	XMLN * p_Sharpness;
	XMLN * p_WideDynamicRange;
	XMLN * p_WhiteBalance;

	p_BacklightCompensation = xml_node_soap_get(p_node, "BacklightCompensation");
    if (p_BacklightCompensation)
    {
    	XMLN * p_Mode = xml_node_soap_get(p_BacklightCompensation, "Mode");
    	if (p_Mode && p_Mode->data)
    	{
    		if (strcasecmp(p_Mode->data, "OFF") == 0)
    		{
    			p_img_cfg->BacklightCompensation_Mode = 0;
    		}
    		else if (strcasecmp(p_Mode->data, "ON") == 0)
    		{
    			p_img_cfg->BacklightCompensation_Mode = 1;
    		}
    	}
    }

    p_Brightness = xml_node_soap_get(p_node, "Brightness");
    if (p_Brightness && p_Brightness->data)
    {
    	p_img_cfg->Brightness = atoi(p_Brightness->data);
    }

    p_ColorSaturation = xml_node_soap_get(p_node, "ColorSaturation");
    if (p_ColorSaturation && p_ColorSaturation->data)
    {
    	p_img_cfg->ColorSaturation = atoi(p_ColorSaturation->data);
    }

    p_Contrast = xml_node_soap_get(p_node, "Contrast");
    if (p_Contrast && p_Contrast->data)
    {
    	p_img_cfg->Contrast= atoi(p_Contrast->data);
    }

    p_Exposure = xml_node_soap_get(p_node, "Exposure");
    if (p_Exposure)
    {
    	XMLN * p_Mode;
		XMLN * p_MinExposureTime;
		XMLN * p_MaxExposureTime;
		XMLN * p_MinGain;
		XMLN * p_MaxGain;

		p_Mode = xml_node_soap_get(p_Exposure, "Mode");
    	if (p_Mode && p_Mode->data)
    	{
    		if (strcasecmp(p_Mode->data, "AUTO") == 0)
    		{
    			p_img_cfg->Exposure_Mode = 0;
    		}
    		else if (strcasecmp(p_Mode->data, "Manual") == 0)
    		{
    			p_img_cfg->Exposure_Mode = 1;
    		}
    	}

    	p_MinExposureTime = xml_node_soap_get(p_Exposure, "MinExposureTime");
    	if (p_MinExposureTime && p_MinExposureTime->data)
    	{
    		p_img_cfg->MinExposureTime = atoi(p_MinExposureTime->data);
    	}

    	p_MaxExposureTime = xml_node_soap_get(p_Exposure, "MaxExposureTime");
    	if (p_MaxExposureTime && p_MaxExposureTime->data)
    	{
    		p_img_cfg->MaxExposureTime= atoi(p_MaxExposureTime->data);
    	}

    	p_MinGain = xml_node_soap_get(p_Exposure, "MinGain");
    	if (p_MinGain && p_MinGain->data)
    	{
    		p_img_cfg->MinGain = atoi(p_MinGain->data);
    	}

    	p_MaxGain = xml_node_soap_get(p_Exposure, "MaxGain");
    	if (p_MaxGain && p_MaxGain->data)
    	{
    		p_img_cfg->MaxGain = atoi(p_MaxGain->data);
    	}
    }

    p_IrCutFilter = xml_node_soap_get(p_node, "IrCutFilter");
    if (p_IrCutFilter && p_IrCutFilter->data)
    {
		if (strcasecmp(p_IrCutFilter->data, "OFF") == 0)
		{
			p_img_cfg->IrCutFilter_Mode = 0;
		}
		else if (strcasecmp(p_IrCutFilter->data, "ON") == 0)
		{
			p_img_cfg->IrCutFilter_Mode = 1;
		}
		else if (strcasecmp(p_IrCutFilter->data, "AUTO") == 0)
		{
			p_img_cfg->IrCutFilter_Mode = 2;
		}
    }

    p_Sharpness = xml_node_soap_get(p_node, "Sharpness");
    if (p_Sharpness && p_Sharpness->data)
    {
    	p_img_cfg->Sharpness = atoi(p_Sharpness->data);
    }

    p_WideDynamicRange = xml_node_soap_get(p_node, "WideDynamicRange");
    if (p_WideDynamicRange)
    {
    	XMLN * p_Mode;
		XMLN * p_Level;

		p_Mode = xml_node_soap_get(p_WideDynamicRange, "Mode");
    	if (p_Mode && p_Mode->data)
    	{
    		if (strcasecmp(p_Mode->data, "OFF") == 0)
    		{
    			p_img_cfg->WideDynamicRange_Mode = 0;
    		}
    		else if (strcasecmp(p_Mode->data, "ON") == 0)
    		{
    			p_img_cfg->WideDynamicRange_Mode = 1;
    		}
    	}

    	p_Level = xml_node_soap_get(p_WideDynamicRange, "Level");
    	if (p_Level && p_Level->data)
    	{
    		p_img_cfg->WideDynamicRange_Level = atoi(p_Level->data);
    	}
    }

    p_WhiteBalance = xml_node_soap_get(p_node, "WhiteBalance");
    if (p_WhiteBalance)
    {
    	XMLN * p_Mode = xml_node_soap_get(p_WhiteBalance, "Mode");
    	if (p_Mode && p_Mode->data)
    	{
    		if (strcasecmp(p_Mode->data, "AUTO") == 0)
    		{
    			p_img_cfg->WhiteBalance_Mode = 0;
    		}
    		else if (strcasecmp(p_Mode->data, "Manual") == 0)
    		{
    			p_img_cfg->WhiteBalance_Mode = 1;
    		}
    	}
    }
    
    return TRUE;
}

BOOL parse_VideoSource(XMLN * p_node, VIDEO_SRC * p_v_src)
{
    XMLN * p_Framerate;
	XMLN * p_Resolution;
	XMLN * p_Imaging;

	p_Framerate = xml_node_soap_get(p_node, "Framerate");
	if (p_Framerate && p_Framerate->data)
	{
	    p_v_src->frame_rate = atoi(p_Framerate->data);
	}

	p_Resolution = xml_node_soap_get(p_node, "Resolution");
	if (p_Resolution)
	{
	    XMLN * p_Width;
		XMLN * p_Height;

		p_Width = xml_node_soap_get(p_Resolution, "Width");
    	if (p_Width && p_Width->data)
    	{
    	    p_v_src->width = atoi(p_Width->data);
    	}

    	p_Height = xml_node_soap_get(p_Resolution, "Height");
    	if (p_Height && p_Height->data)
    	{
    	    p_v_src->height = atoi(p_Height->data);
    	}
	}

	p_Imaging = xml_node_soap_get(p_node, "Imaging");
	if (p_Imaging)
	{
	    parse_ImagingSettings(p_Imaging, &p_v_src->img_cfg);
	}
	
    return TRUE;
}

BOOL parse_AudioSource(XMLN * p_node, AUDIO_SRC * p_a_src)
{
    XMLN * p_Channels = xml_node_soap_get(p_node, "Channels");
	if (p_Channels && p_Channels->data)
	{
	    p_a_src->channels = atoi(p_Channels->data);
	}
	
    return TRUE;
}

BOOL parse_Datetime(XMLN * p_node, DATETIME * p_datetime)
{
    XMLN * p_Time;
	XMLN * p_Hour;
	XMLN * p_Minute;
	XMLN * p_Second;
	XMLN * p_Date;
	XMLN * p_Year;
	XMLN * p_Month;
	XMLN * p_Day;

	p_Time = xml_node_soap_get(p_node, "Time");
	if (NULL == p_Time)
	{
	    return FALSE;
	}

	p_Hour = xml_node_soap_get(p_Time, "Hour");
	if (p_Hour && p_Hour->data)
	{
	    p_datetime->hour = atoi(p_Hour->data);
	}

	p_Minute = xml_node_soap_get(p_Time, "Minute");
	if (p_Minute && p_Minute->data)
	{
	    p_datetime->minute = atoi(p_Minute->data);
	}

	p_Second = xml_node_soap_get(p_Time, "Second");
	if (p_Second && p_Second->data)
	{
	    p_datetime->second = atoi(p_Second->data);
	}

	p_Date = xml_node_soap_get(p_node, "Date");
	if (NULL == p_Date)
	{
	    return FALSE;
	}

	p_Year = xml_node_soap_get(p_Date, "Year");
	if (p_Year && p_Year->data)
	{
	    p_datetime->year = atoi(p_Year->data);
	}

	p_Month = xml_node_soap_get(p_Date, "Month");
	if (p_Month && p_Month->data)
	{
	    p_datetime->month = atoi(p_Month->data);
	}

	p_Day = xml_node_soap_get(p_Date, "Day");
	if (p_Day && p_Day->data)
	{
	    p_datetime->day = atoi(p_Day->data);
	}

	return TRUE;
}

BOOL parseGetSystemDateAndTime(XMLN * p_node, GetSystemDateAndTime_RES * p_res)
{
    XMLN * p_SystemDateAndTime;
	XMLN * p_DateTimeType;
	XMLN * p_DaylightSavings;
	XMLN * p_TimeZone;
	XMLN * p_UTCDateTime;
	XMLN * p_LocalDateTime;

	p_SystemDateAndTime = xml_node_soap_get(p_node, "SystemDateAndTime");
	if (NULL == p_SystemDateAndTime)
	{
	    return FALSE;
	}

	p_DateTimeType = xml_node_soap_get(p_SystemDateAndTime, "DateTimeType");
	if (p_DateTimeType && p_DateTimeType->data)
	{
	    if (strcasecmp(p_DateTimeType->data, "NTP") == 0)
	    {
	        p_res->type = 1;
	    }
	    else if (strcasecmp(p_DateTimeType->data, "Manual") == 0)
	    {
	        p_res->type = 0;
	    }
	    else
	    {
	        return FALSE;
	    }
	}

	p_DaylightSavings = xml_node_soap_get(p_SystemDateAndTime, "DaylightSavings");
	if (p_DaylightSavings && p_DaylightSavings->data)
	{
	    p_res->DaylightSavings = parse_Bool(p_DaylightSavings->data);
	}

	p_TimeZone = xml_node_soap_get(p_SystemDateAndTime, "TimeZone");
	if (p_TimeZone)
	{
	    XMLN * p_TZ = xml_node_soap_get(p_TimeZone, "TZ");
    	if (p_TZ && p_TZ->data)
    	{
    	    strncpy(p_res->TZ, p_TZ->data, sizeof(p_res->TZ)-1);
    	}
	}

	p_UTCDateTime = xml_node_soap_get(p_SystemDateAndTime, "UTCDateTime");
	if (p_UTCDateTime)
	{
	    p_res->utc_valid = parse_Datetime(p_UTCDateTime, &p_res->utc_datetime);
	}

	p_LocalDateTime = xml_node_soap_get(p_SystemDateAndTime, "LocalDateTime");
	if (p_LocalDateTime)
	{
	    p_res->local_valid = parse_Datetime(p_LocalDateTime, &p_res->local_datetime);
	}

	return TRUE;
}

BOOL parse_GetOptions(XMLN * p_node, GetOptions_RES * p_res)
{
	XMLN * p_ImagingOptions;
	XMLN * p_BacklightCompensation;
	XMLN * p_Brightness;
	XMLN * p_ColorSaturation;
	XMLN * p_Contrast;
	XMLN * p_Exposure;
	XMLN * p_IrCutFilterModes;
	XMLN * p_Sharpness;
	XMLN * p_WideDynamicRange;
	XMLN * p_WhiteBalance;

	p_ImagingOptions = xml_node_soap_get(p_node, "ImagingOptions");
	if (NULL == p_ImagingOptions)
	{
	    return FALSE;
	}
	
	p_BacklightCompensation = xml_node_soap_get(p_ImagingOptions, "BacklightCompensation");
	if (p_BacklightCompensation)
	{
		XMLN * p_Mode = xml_node_soap_get(p_BacklightCompensation, "Mode");
		while (p_Mode && soap_strcmp(p_Mode->name, "Mode") == 0)
		{
			if (p_Mode->data)
			{
				if (strcasecmp(p_Mode->data, "OFF") == 0)
				{
					p_res->img_opt.BacklightCompensation_OFF = 1;
				}
				else if (strcasecmp(p_Mode->data, "ON") == 0)
				{
					p_res->img_opt.BacklightCompensation_ON = 1;
				}
			}

			p_Mode = p_Mode->next;
		}
	}

	p_Brightness = xml_node_soap_get(p_ImagingOptions, "Brightness");
	if (p_Brightness)
	{
		XMLN * p_Min;
		XMLN * p_Max;

		p_Min = xml_node_soap_get(p_Brightness, "Min");
		if (p_Min && p_Min->data)
		{
			p_res->img_opt.Brightness_min = atoi(p_Min->data);
		}

		p_Max = xml_node_soap_get(p_Brightness, "Max");
		if (p_Max && p_Max->data)
		{
			p_res->img_opt.Brightness_max = atoi(p_Max->data);
		}
	}

	p_ColorSaturation = xml_node_soap_get(p_ImagingOptions, "ColorSaturation");
	if (p_ColorSaturation)
	{
		XMLN * p_Min;
		XMLN * p_Max;
		
		p_Min = xml_node_soap_get(p_ColorSaturation, "Min");
		if (p_Min && p_Min->data)
		{
			p_res->img_opt.ColorSaturation_min = atoi(p_Min->data);
		}

		p_Max = xml_node_soap_get(p_ColorSaturation, "Max");
		if (p_Max && p_Max->data)
		{
			p_res->img_opt.ColorSaturation_max = atoi(p_Max->data);
		}
	}

	p_Contrast = xml_node_soap_get(p_ImagingOptions, "Contrast");
	if (p_Contrast)
	{
		XMLN * p_Min;
		XMLN * p_Max;
		
		p_Min = xml_node_soap_get(p_Contrast, "Min");
		if (p_Min && p_Min->data)
		{
			p_res->img_opt.Contrast_min = atoi(p_Min->data);
		}

		p_Max = xml_node_soap_get(p_Contrast, "Max");
		if (p_Max && p_Max->data)
		{
			p_res->img_opt.Contrast_max = atoi(p_Max->data);
		}
	}

	p_Exposure = xml_node_soap_get(p_ImagingOptions, "Exposure");
	if (p_Exposure)
	{
		XMLN * p_Mode;
		XMLN * p_MinExposureTime;
		XMLN * p_MaxExposureTime;
		XMLN * p_MinGain;
		XMLN * p_MaxGain;

		p_Mode = xml_node_soap_get(p_Exposure, "Mode");
		while (p_Mode && soap_strcmp(p_Mode->name, "Mode") == 0)
		{
			if (p_Mode->data)
			{
				if (strcasecmp(p_Mode->data, "Auto") == 0)
				{
					p_res->img_opt.Exposure_AUTO = 1;
				}
				else if (strcasecmp(p_Mode->data, "Manual") == 0)
				{
					p_res->img_opt.Exposure_MANUAL = 1;
				}
			}

			p_Mode = p_Mode->next;
		}
		
		p_MinExposureTime = xml_node_soap_get(p_Exposure, "MinExposureTime");
		if (p_MinExposureTime)
		{
			XMLN * p_Min;
			XMLN * p_Max;
			
			p_Min = xml_node_soap_get(p_MinExposureTime, "Min");
			if (p_Min && p_Min->data)
			{
				p_res->img_opt.MinExposureTime_min = atoi(p_Min->data);
			}

			p_Max = xml_node_soap_get(p_MinExposureTime, "Max");
			if (p_Max && p_Max->data)
			{
				p_res->img_opt.MinExposureTime_max = atoi(p_Max->data);
			}
		}

		p_MaxExposureTime = xml_node_soap_get(p_Exposure, "MaxExposureTime");
		if (p_MaxExposureTime)
		{
			XMLN * p_Min;
			XMLN * p_Max;
			
			p_Min = xml_node_soap_get(p_MaxExposureTime, "Min");
			if (p_Min && p_Min->data)
			{
				p_res->img_opt.MaxExposureTime_min = atoi(p_Min->data);
			}

			p_Max = xml_node_soap_get(p_MaxExposureTime, "Max");
			if (p_Max && p_Max->data)
			{
				p_res->img_opt.MaxExposureTime_max = atoi(p_Max->data);
			}
		}

		p_MinGain = xml_node_soap_get(p_Exposure, "MinGain");
		if (p_MinGain)
		{
			XMLN * p_Min;
			XMLN * p_Max;
			
			p_Min = xml_node_soap_get(p_MinGain, "Min");
			if (p_Min && p_Min->data)
			{
				p_res->img_opt.MinGain_min = atoi(p_Min->data);
			}

			p_Max = xml_node_soap_get(p_MinGain, "Max");
			if (p_Max && p_Max->data)
			{
				p_res->img_opt.MinGain_max = atoi(p_Max->data);
			}
		}

		p_MaxGain = xml_node_soap_get(p_Exposure, "MaxGain");
		if (p_MaxGain)
		{
			XMLN * p_Min;
			XMLN * p_Max;
			
			p_Min = xml_node_soap_get(p_MaxGain, "Min");
			if (p_Min && p_Min->data)
			{
				p_res->img_opt.MaxGain_min = atoi(p_Min->data);
			}

			p_Max = xml_node_soap_get(p_MaxGain, "Max");
			if (p_Max && p_Max->data)
			{
				p_res->img_opt.MaxGain_max = atoi(p_Max->data);
			}
		}
	}

	p_IrCutFilterModes = xml_node_soap_get(p_ImagingOptions, "IrCutFilterModes");
	while (p_IrCutFilterModes && soap_strcmp(p_IrCutFilterModes->name, "IrCutFilterModes") == 0)
	{
		if (p_IrCutFilterModes->data)
		{
			if (strcasecmp(p_IrCutFilterModes->data, "ON") == 0)
			{
				p_res->img_opt.IrCutFilter_ON = 1;
			}
			else if (strcasecmp(p_IrCutFilterModes->data, "OFF") == 0)
			{
				p_res->img_opt.IrCutFilter_OFF = 1;
			}
			else if (strcasecmp(p_IrCutFilterModes->data, "Auto") == 0)
			{
				p_res->img_opt.IrCutFilter_AUTO = 1;
			}
		}

		p_IrCutFilterModes = p_IrCutFilterModes->next;
	}

	p_Sharpness = xml_node_soap_get(p_ImagingOptions, "Sharpness");
	if (p_Sharpness)
	{
		XMLN * p_Min;
		XMLN * p_Max;
			
		p_Min = xml_node_soap_get(p_Sharpness, "Min");
		if (p_Min && p_Min->data)
		{
			p_res->img_opt.Sharpness_min = atoi(p_Min->data);
		}

		p_Max = xml_node_soap_get(p_Sharpness, "Max");
		if (p_Max && p_Max->data)
		{
			p_res->img_opt.Sharpness_max = atoi(p_Max->data);
		}
	}

	p_WideDynamicRange = xml_node_soap_get(p_ImagingOptions, "WideDynamicRange");
	if (p_WideDynamicRange)
	{
		XMLN * p_Mode;
		XMLN * p_Level;

		p_Mode = xml_node_soap_get(p_WideDynamicRange, "Mode");
		while (p_Mode && soap_strcmp(p_Mode->name, "Mode") == 0)
		{
			if (p_Mode->data)
			{
				if (strcasecmp(p_Mode->data, "ON") == 0)
				{
					p_res->img_opt.WideDynamicRange_ON = 1;
				}
				else if (strcasecmp(p_Mode->data, "OFF") == 0)
				{
					p_res->img_opt.WideDynamicRange_OFF = 1;
				}
			}

			p_Mode = p_Mode->next;
		}

		p_Level = xml_node_soap_get(p_WideDynamicRange, "Level");
		if (p_Level)
		{
			XMLN * p_Min;
			XMLN * p_Max;
			
			p_Min = xml_node_soap_get(p_Level, "Min");
			if (p_Min && p_Min->data)
			{
				p_res->img_opt.WideDynamicRange_Level_min = atoi(p_Min->data);
			}

			p_Max = xml_node_soap_get(p_Level, "Max");
			if (p_Max && p_Max->data)
			{
				p_res->img_opt.WideDynamicRange_Level_max = atoi(p_Max->data);
			}
		}
	}

	p_WhiteBalance = xml_node_soap_get(p_ImagingOptions, "WhiteBalance");
	if (p_WhiteBalance)
	{
		XMLN * p_Mode = xml_node_soap_get(p_WhiteBalance, "Mode");
		while (p_Mode && soap_strcmp(p_Mode->name, "Mode") == 0)
		{
			if (p_Mode->data)
			{
				if (strcasecmp(p_Mode->data, "Auto") == 0)
				{
					p_res->img_opt.WhiteBalance_AUTO = 1;
				}
				else if (strcasecmp(p_Mode->data, "Manual") == 0)
				{
					p_res->img_opt.WhiteBalance_MANUAL = 1;
				}
			}

			p_Mode = p_Mode->next;
		}
	}
	
	return TRUE;
}


BOOL parse_Subscribe(XMLN * p_node, Subscribe_RES * p_res)
{
    XMLN * p_SubscriptionReference;
	XMLN * p_Address;

	p_SubscriptionReference = xml_node_soap_get(p_node, "SubscriptionReference");
    if (NULL == p_SubscriptionReference)
    {
        return FALSE;
    }

    p_Address = xml_node_soap_get(p_SubscriptionReference, "Address");
    if (p_Address && p_Address->data)
    {
        strncpy(p_res->producter_addr, p_Address->data, sizeof(p_res->producter_addr)-1);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

BOOL parse_SimpleItem(XMLN * p_node, SIMPLE_ITEM * p_item)
{
    const char * p_Name;
	const char * p_Value;

	p_Name = xml_attr_get(p_node, "Name");
    if (p_Name)
    {
        strncpy(p_item->name, p_Name, sizeof(p_item->name)-1);
    }

    p_Value = xml_attr_get(p_node, "Value");
    if (p_Value)
    {
        strncpy(p_item->value, p_Value, sizeof(p_item->value)-1);
    }

    return TRUE;
}

BOOL parse_Notify(XMLN * p_node, ONVIF_NOTIFY * p_notify)
{
    XMLN * p_SubscriptionReference;
	XMLN * p_Topic;
	XMLN * p_ProducerReference;
	XMLN * p_Message;

	p_SubscriptionReference = xml_node_soap_get(p_node, "SubscriptionReference");
    if (p_SubscriptionReference)
    {
        XMLN * p_Address = xml_node_soap_get(p_SubscriptionReference, "Address");
        if (p_Address && p_Address->data)
        {
            strncpy(p_notify->reference_addr, p_Address->data, sizeof(p_notify->reference_addr)-1);
        }
    }

    p_Topic = xml_node_soap_get(p_node, "Topic");
    if (p_Topic && p_Topic->data)
    {
        strncpy(p_notify->topic, p_Topic->data, sizeof(p_notify->topic)-1);
    }

    p_ProducerReference = xml_node_soap_get(p_node, "ProducerReference");
    if (p_ProducerReference)
    {
        XMLN * p_Address = xml_node_soap_get(p_ProducerReference, "Address");
        if (p_Address && p_Address->data)
        {
            strncpy(p_notify->producter_addr, p_Address->data, sizeof(p_notify->producter_addr)-1);
        }
    }

    p_Message = xml_node_soap_get(p_node, "Message");
    if (p_Message)
    {
        XMLN * p_tt_Message = xml_node_soap_get(p_Message, "tt:Message");
        if (p_tt_Message)
        {
        	XMLN * p_Source;
			XMLN * p_Key;
			XMLN * p_Data;
            const char * p_UtcTime;
			const char * p_PropertyOperation;

			p_UtcTime = xml_attr_get(p_tt_Message, "UtcTime");
            if (p_UtcTime)
            {
                strncpy(p_notify->utctime, p_UtcTime, sizeof(p_notify->utctime)-1);
            }

            p_PropertyOperation = xml_attr_get(p_tt_Message, "PropertyOperation");
            if (p_PropertyOperation)
            {
                strncpy(p_notify->operation, p_PropertyOperation, sizeof(p_notify->operation)-1);
            }

            p_Source = xml_node_soap_get(p_tt_Message, "Source");
            if (p_Source)
            {
                XMLN * p_SimpleItem = xml_node_soap_get(p_Source, "SimpleItem");
                while (p_SimpleItem && soap_strcmp(p_SimpleItem->name, "SimpleItem") == 0)
                {
                    SIMPLE_ITEM * p_item = onvif_add_simple_item(&p_notify->source);
                    if (p_item)
                    {
                        parse_SimpleItem(p_SimpleItem, p_item);
                    }

                    p_SimpleItem = p_SimpleItem->next;
                }
            }

            p_Key = xml_node_soap_get(p_tt_Message, "Key");
            if (p_Key)
            {
                XMLN * p_SimpleItem = xml_node_soap_get(p_Key, "SimpleItem");
                while (p_SimpleItem && soap_strcmp(p_SimpleItem->name, "SimpleItem") == 0)
                {
                    SIMPLE_ITEM * p_item = onvif_add_simple_item(&p_notify->key);
                    if (p_item)
                    {
                        parse_SimpleItem(p_SimpleItem, p_item);
                    }

                    p_SimpleItem = p_SimpleItem->next;
                }
            }

            p_Data = xml_node_soap_get(p_tt_Message, "Data");
            if (p_Data)
            {
                XMLN * p_SimpleItem = xml_node_soap_get(p_Data, "SimpleItem");
                while (p_SimpleItem && soap_strcmp(p_SimpleItem->name, "SimpleItem") == 0)
                {
                    SIMPLE_ITEM * p_item = onvif_add_simple_item(&p_notify->data);
                    if (p_item)
                    {
                        parse_SimpleItem(p_SimpleItem, p_item);
                    }

                    p_SimpleItem = p_SimpleItem->next;
                }
            }
        }
    }
    
    return TRUE;
}

BOOL parse_DeviceService(XMLN * p_node, DEVICE_CAP * p_cap)
{
    XMLN * p_XAddr;
	XMLN * p_tds_Capabilities;

	p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

    p_tds_Capabilities = xml_node_soap_get(p_node, "Capabilities");
    if (p_tds_Capabilities)
    {
        XMLN * p_Capabilities = xml_node_soap_get(p_tds_Capabilities, "Capabilities");
        if (p_Capabilities)
        {
            XMLN * p_Network;
			XMLN * p_Security;
			XMLN * p_System;
			XMLN * p_Misc;

			p_Network = xml_node_soap_get(p_Capabilities, "Network");
            if (p_Network)
            {
                const char * p_IPFilter;
				const char * p_ZeroConfiguration;
				const char * p_IPVersion6;
				const char * p_DynDNS;
				const char * p_Dot11Configuration;
				const char * p_HostnameFromDHCP;
				const char * p_DHCPv6;
				const char * p_Dot1XConfigurations;
				const char * p_NTP;

				p_IPFilter = xml_attr_get(p_Network, "IPFilter");
                if (p_IPFilter)
                {
                    p_cap->IPFilter = parse_Bool(p_IPFilter);
                }

                p_ZeroConfiguration = xml_attr_get(p_Network, "ZeroConfiguration");
                if (p_ZeroConfiguration)
                {
                    p_cap->ZeroConfiguration = parse_Bool(p_ZeroConfiguration);
                }

                p_IPVersion6 = xml_attr_get(p_Network, "IPVersion6");
                if (p_IPVersion6)
                {
                    p_cap->IPVersion6 = parse_Bool(p_IPVersion6);
                }

                p_DynDNS = xml_attr_get(p_Network, "DynDNS");
                if (p_DynDNS)
                {
                    p_cap->DynDNS = parse_Bool(p_DynDNS);
                }

                p_Dot11Configuration = xml_attr_get(p_Network, "Dot11Configuration");
                if (p_Dot11Configuration)
                {
                    p_cap->Dot11Configuration = parse_Bool(p_Dot11Configuration);
                }

                p_HostnameFromDHCP = xml_attr_get(p_Network, "HostnameFromDHCP");
                if (p_HostnameFromDHCP)
                {
                    p_cap->HostnameFromDHCP = parse_Bool(p_HostnameFromDHCP);
                }

                p_DHCPv6 = xml_attr_get(p_Network, "DHCPv6");
                if (p_DHCPv6)
                {
                    p_cap->DHCPv6 = parse_Bool(p_DHCPv6);
                }

                p_Dot1XConfigurations = xml_attr_get(p_Network, "Dot1XConfigurations");
                if (p_Dot1XConfigurations)
                {
                    p_cap->Dot1XConfigurations = atoi(p_Dot1XConfigurations);
                }

                p_NTP = xml_attr_get(p_Network, "NTP");
                if (p_NTP)
                {
                    p_cap->NTP = atoi(p_NTP);
                }
            }

            p_Security = xml_node_soap_get(p_Capabilities, "Security");
            if (p_Security)
            {
                const char * p_TLS10;
				const char * p_TLS11;
				const char * p_TLS12;
				const char * p_OnboardKeyGeneration;
				const char * p_AccessPolicyConfig;
				const char * p_DefaultAccessPolicy;
				const char * p_Dot1X;
				const char * p_RemoteUserHandling;
				const char * p_X509Token;
				const char * p_SAMLToken;
				const char * p_KerberosToken;
				const char * p_UsernameToken;
				const char * p_HttpDigest;
				const char * p_RELToken;
				const char * p_SupportedEAPMethods;
				const char * p_MaxUsers;

				p_TLS10 = xml_attr_get(p_Network, "TLS1.0");
                if (p_TLS10)
                {
                    p_cap->TLS10 = parse_Bool(p_TLS10);
                }

                p_TLS11 = xml_attr_get(p_Network, "TLS1.1");
                if (p_TLS11)
                {
                    p_cap->TLS11 = parse_Bool(p_TLS11);
                }

                p_TLS12 = xml_attr_get(p_Network, "TLS1.2");
                if (p_TLS12)
                {
                    p_cap->TLS12 = parse_Bool(p_TLS12);
                }

                p_OnboardKeyGeneration = xml_attr_get(p_Network, "OnboardKeyGeneration");
                if (p_OnboardKeyGeneration)
                {
                    p_cap->OnboardKeyGeneration = parse_Bool(p_OnboardKeyGeneration);
                }

                p_AccessPolicyConfig = xml_attr_get(p_Network, "AccessPolicyConfig");
                if (p_AccessPolicyConfig)
                {
                    p_cap->AccessPolicyConfig = parse_Bool(p_AccessPolicyConfig);
                }

                p_DefaultAccessPolicy = xml_attr_get(p_Network, "DefaultAccessPolicy");
                if (p_DefaultAccessPolicy)
                {
                    p_cap->DefaultAccessPolicy = parse_Bool(p_DefaultAccessPolicy);
                }

                p_Dot1X = xml_attr_get(p_Network, "Dot1X");
                if (p_Dot1X)
                {
                    p_cap->Dot1X = parse_Bool(p_Dot1X);
                }

                p_RemoteUserHandling = xml_attr_get(p_Network, "RemoteUserHandling");
                if (p_RemoteUserHandling)
                {
                    p_cap->RemoteUserHandling = parse_Bool(p_RemoteUserHandling);
                }

                p_X509Token = xml_attr_get(p_Network, "X.509Token");
                if (p_X509Token)
                {
                    p_cap->X509Token = parse_Bool(p_X509Token);
                }

                p_SAMLToken = xml_attr_get(p_Network, "SAMLToken");
                if (p_SAMLToken)
                {
                    p_cap->SAMLToken = parse_Bool(p_SAMLToken);
                }

                p_KerberosToken = xml_attr_get(p_Network, "KerberosToken");
                if (p_KerberosToken)
                {
                    p_cap->KerberosToken = parse_Bool(p_KerberosToken);
                }

                p_UsernameToken = xml_attr_get(p_Network, "UsernameToken");
                if (p_UsernameToken)
                {
                    p_cap->UsernameToken = parse_Bool(p_UsernameToken);
                }

                p_HttpDigest = xml_attr_get(p_Network, "HttpDigest");
                if (p_HttpDigest)
                {
                    p_cap->HttpDigest = parse_Bool(p_HttpDigest);
                }

                p_RELToken = xml_attr_get(p_Network, "RELToken");
                if (p_RELToken)
                {
                    p_cap->RELToken = parse_Bool(p_RELToken);
                }

                p_SupportedEAPMethods = xml_attr_get(p_Network, "SupportedEAPMethods");
                if (p_SupportedEAPMethods)
                {
                    p_cap->SupportedEAPMethods = atoi(p_SupportedEAPMethods);
                }

                p_MaxUsers = xml_attr_get(p_Network, "MaxUsers");
                if (p_MaxUsers)
                {
                    p_cap->MaxUsers = atoi(p_MaxUsers);
                }
            }

            p_System = xml_node_soap_get(p_Capabilities, "System");
            if (p_System)
            {
                const char * p_DiscoveryResolve;
				const char * p_DiscoveryBye;
				const char * p_RemoteDiscovery;
				const char * p_SystemBackup;
				const char * p_SystemLogging;
				const char * p_FirmwareUpgrade;
				const char * p_HttpFirmwareUpgrade;
				const char * p_HttpSystemBackup;
				const char * p_HttpSystemLogging;
				const char * p_HttpSupportInformation;

				p_DiscoveryResolve = xml_attr_get(p_Network, "DiscoveryResolve");
                if (p_DiscoveryResolve)
                {
                    p_cap->DiscoveryResolve = parse_Bool(p_DiscoveryResolve);
                }

                p_DiscoveryBye = xml_attr_get(p_Network, "DiscoveryBye");
                if (p_DiscoveryBye)
                {
                    p_cap->DiscoveryBye = parse_Bool(p_DiscoveryBye);
                }

                p_RemoteDiscovery = xml_attr_get(p_Network, "RemoteDiscovery");
                if (p_RemoteDiscovery)
                {
                    p_cap->RemoteDiscovery = parse_Bool(p_RemoteDiscovery);
                }

                p_SystemBackup = xml_attr_get(p_Network, "SystemBackup");
                if (p_SystemBackup)
                {
                    p_cap->SystemBackup = parse_Bool(p_SystemBackup);
                }

                p_SystemLogging = xml_attr_get(p_Network, "SystemLogging");
                if (p_SystemLogging)
                {
                    p_cap->SystemLogging = parse_Bool(p_SystemLogging);
                }

                p_FirmwareUpgrade = xml_attr_get(p_Network, "FirmwareUpgrade");
                if (p_FirmwareUpgrade)
                {
                    p_cap->FirmwareUpgrade = parse_Bool(p_FirmwareUpgrade);
                }

                p_HttpFirmwareUpgrade = xml_attr_get(p_Network, "HttpFirmwareUpgrade");
                if (p_HttpFirmwareUpgrade)
                {
                    p_cap->HttpFirmwareUpgrade = parse_Bool(p_HttpFirmwareUpgrade);
                }

                p_HttpSystemBackup = xml_attr_get(p_Network, "HttpSystemBackup");
                if (p_HttpSystemBackup)
                {
                    p_cap->HttpSystemBackup = parse_Bool(p_HttpSystemBackup);
                }

                p_HttpSystemLogging = xml_attr_get(p_Network, "HttpSystemLogging");
                if (p_HttpSystemLogging)
                {
                    p_cap->HttpSystemLogging = parse_Bool(p_HttpSystemLogging);
                }

                p_HttpSupportInformation = xml_attr_get(p_Network, "HttpSupportInformation");
                if (p_HttpSupportInformation)
                {
                    p_cap->HttpSupportInformation = parse_Bool(p_HttpSupportInformation);
                }
            }

            p_Misc = xml_node_soap_get(p_Capabilities, "Misc");
            if (p_Misc)
            {
                const char * p_AuxiliaryCommands = xml_attr_get(p_Network, "AuxiliaryCommands");
                if (p_AuxiliaryCommands)
                {
                    strncpy(p_cap->AuxiliaryCommands, p_AuxiliaryCommands, sizeof(p_cap->AuxiliaryCommands)-1);
                }
            }
        }
    }

    return TRUE;
}

BOOL parse_MediaService(XMLN * p_node, MEDIA_CAP * p_cap)
{
    XMLN * p_XAddr;
	XMLN * p_tds_Capabilities;

	p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

    p_tds_Capabilities = xml_node_soap_get(p_node, "Capabilities");
    if (p_tds_Capabilities)
    {
        XMLN * p_Capabilities = xml_node_soap_get(p_tds_Capabilities, "Capabilities");
        if (p_Capabilities)
        {
        	XMLN * p_ProfileCapabilities;
			XMLN * p_StreamingCapabilities;
            const char * p_SnapshotUri;
			const char * p_Rotation;
			const char * p_VideoSourceMode;
			const char * p_OSD;

			p_SnapshotUri = xml_attr_get(p_Capabilities, "SnapshotUri");
            if (p_SnapshotUri)
            {
                p_cap->SnapshotUri = parse_Bool(p_SnapshotUri);
            }

            p_Rotation = xml_attr_get(p_Capabilities, "Rotation");
            if (p_Rotation)
            {
                p_cap->Rotation = parse_Bool(p_Rotation);
            }

            p_VideoSourceMode = xml_attr_get(p_Capabilities, "VideoSourceMode");
            if (p_VideoSourceMode)
            {
                p_cap->VideoSourceMode = parse_Bool(p_VideoSourceMode);
            }

            p_OSD = xml_attr_get(p_Capabilities, "OSD");
            if (p_OSD)
            {
                p_cap->OSD = parse_Bool(p_OSD);
            }

            p_ProfileCapabilities = xml_node_soap_get(p_Capabilities, "ProfileCapabilities");
            if (p_ProfileCapabilities)
            {
                const char * p_MaximumNumberOfProfiles = xml_attr_get(p_ProfileCapabilities, "MaximumNumberOfProfiles");
                if (p_MaximumNumberOfProfiles)
                {
                    p_cap->MaximumNumberOfProfiles = atoi(p_MaximumNumberOfProfiles);
                }
            }

            p_StreamingCapabilities = xml_node_soap_get(p_Capabilities, "StreamingCapabilities");
            if (p_StreamingCapabilities)
            {
                const char * p_RTPMulticast;
				const char * p_RTP_TCP;
				const char * p_RTP_RTSP_TCP;
				const char * p_NonAggregateControl;
				const char * p_NoRTSPStreaming;

				p_RTPMulticast = xml_attr_get(p_StreamingCapabilities, "RTPMulticast");
                if (p_RTPMulticast)
                {
                    p_cap->RTPMulticast = parse_Bool(p_RTPMulticast);
                }
                
                p_RTP_TCP = xml_attr_get(p_StreamingCapabilities, "RTP_TCP");
                if (p_RTP_TCP)
                {
                    p_cap->RTP_TCP = parse_Bool(p_RTP_TCP);
                }

                p_RTP_RTSP_TCP = xml_attr_get(p_StreamingCapabilities, "RTP_RTSP_TCP");
                if (p_RTP_RTSP_TCP)
                {
                    p_cap->RTP_RTSP_TCP = parse_Bool(p_RTP_RTSP_TCP);
                }

                p_NonAggregateControl = xml_attr_get(p_StreamingCapabilities, "NonAggregateControl");
                if (p_NonAggregateControl)
                {
                    p_cap->NonAggregateControl = parse_Bool(p_NonAggregateControl);
                }

                p_NoRTSPStreaming = xml_attr_get(p_StreamingCapabilities, "NoRTSPStreaming");
                if (p_NoRTSPStreaming)
                {
                    p_cap->NoRTSPStreaming = parse_Bool(p_NoRTSPStreaming);
                }
            }
        }
    }   

    return TRUE;
}

BOOL parse_EventsService(XMLN * p_node, EVENT_CAP * p_cap)
{
    XMLN * p_XAddr;
	XMLN * p_tds_Capabilities;

	p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

    p_tds_Capabilities = xml_node_soap_get(p_node, "Capabilities");
    if (p_tds_Capabilities)
    {
        XMLN * p_Capabilities = xml_node_soap_get(p_tds_Capabilities, "Capabilities");
        if (p_Capabilities)
        {
            const char * p_WSSubscriptionPolicySupport;
			const char * p_WSPullPointSupport;
			const char * p_WSPausableSubscriptionManagerInterfaceSupport;
			const char * p_MaxNotificationProducers;
			const char * p_MaxPullPoints;
			const char * p_PersistentNotificationStorage;

			p_WSSubscriptionPolicySupport = xml_attr_get(p_Capabilities, "WSSubscriptionPolicySupport");
            if (p_WSSubscriptionPolicySupport)
            {
                p_cap->WSSubscriptionPolicySupport = parse_Bool(p_WSSubscriptionPolicySupport);
            }

            p_WSPullPointSupport = xml_attr_get(p_Capabilities, "WSPullPointSupport");
            if (p_WSPullPointSupport)
            {
                p_cap->WSPullPointSupport = parse_Bool(p_WSPullPointSupport);
            }

            p_WSPausableSubscriptionManagerInterfaceSupport = xml_attr_get(p_Capabilities, "WSPausableSubscriptionManagerInterfaceSupport");
            if (p_WSPausableSubscriptionManagerInterfaceSupport)
            {
                p_cap->WSPausableSubscriptionManagerInterfaceSupport = parse_Bool(p_WSPausableSubscriptionManagerInterfaceSupport);
            }

            p_MaxNotificationProducers = xml_attr_get(p_Capabilities, "MaxNotificationProducers");
            if (p_MaxNotificationProducers)
            {
                p_cap->MaxNotificationProducers = atoi(p_MaxNotificationProducers);
            }

            p_MaxPullPoints = xml_attr_get(p_Capabilities, "MaxPullPoints");
            if (p_MaxPullPoints)
            {
                p_cap->MaxPullPoints = atoi(p_MaxPullPoints);
            }

            p_PersistentNotificationStorage = xml_attr_get(p_Capabilities, "PersistentNotificationStorage");
            if (p_PersistentNotificationStorage)
            {
                p_cap->PersistentNotificationStorage = parse_Bool(p_PersistentNotificationStorage);
            }
        }
    } 

    return TRUE;
}

BOOL parse_PTZService(XMLN * p_node, PTZ_CAP * p_cap)
{
    XMLN * p_XAddr;
	XMLN * p_tds_Capabilities;

	p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

    p_tds_Capabilities = xml_node_soap_get(p_node, "Capabilities");
    if (p_tds_Capabilities)
    {
        XMLN * p_Capabilities = xml_node_soap_get(p_tds_Capabilities, "Capabilities");
        if (p_Capabilities)
        {
            const char * p_EFlip;
			const char * p_Reverse;
			const char * p_GetCompatibleConfigurations;

			p_EFlip = xml_attr_get(p_Capabilities, "EFlip");
            if (p_EFlip)
            {
                p_cap->EFlip = parse_Bool(p_EFlip);
            }

            p_Reverse = xml_attr_get(p_Capabilities, "Reverse");
            if (p_Reverse)
            {
                p_cap->Reverse = parse_Bool(p_Reverse);
            }

            p_GetCompatibleConfigurations = xml_attr_get(p_Capabilities, "GetCompatibleConfigurations");
            if (p_GetCompatibleConfigurations)
            {
                p_cap->GetCompatibleConfigurations = parse_Bool(p_GetCompatibleConfigurations);
            }
        }
    } 

    return TRUE;
}

BOOL parse_ImageingService(XMLN * p_node, IMAGE_CAP * p_cap)
{
    XMLN * p_XAddr;
	XMLN * p_tds_Capabilities;

	p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

    p_tds_Capabilities = xml_node_soap_get(p_node, "Capabilities");
    if (p_tds_Capabilities)
    {
        XMLN * p_Capabilities = xml_node_soap_get(p_tds_Capabilities, "Capabilities");
        if (p_Capabilities)
        {
            const char * p_ImageStabilization = xml_attr_get(p_Capabilities, "ImageStabilization");
            if (p_ImageStabilization)
            {
                p_cap->ImageStabilization = parse_Bool(p_ImageStabilization);
            }
        }
    } 

    return TRUE;
}

BOOL parse_RecordingService(XMLN * p_node, RECORDING_CAP * p_cap)
{
	XMLN * p_XAddr;
	XMLN * p_tds_Capabilities;
	
    p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

    p_tds_Capabilities = xml_node_soap_get(p_node, "Capabilities");
    if (p_tds_Capabilities)
    {
        XMLN * p_Capabilities = xml_node_soap_get(p_tds_Capabilities, "Capabilities");
        if (p_Capabilities)
        {
            const char * p_DynamicRecordings;
			const char * p_DynamicTracks;
			const char * p_Encoding;
			const char * p_MaxRate;
			const char * p_MaxTotalRate;
			const char * p_MaxRecordings;
			const char * p_MaxRecordingJobs;
			const char * p_Options;
			const char * p_MetadataRecording;

			p_DynamicRecordings = xml_attr_get(p_Capabilities, "DynamicRecordings");
            if (p_DynamicRecordings)
            {
                p_cap->DynamicRecordings = parse_Bool(p_DynamicRecordings);
            }

            p_DynamicTracks = xml_attr_get(p_Capabilities, "DynamicTracks");
            if (p_DynamicTracks)
            {
                p_cap->DynamicTracks = parse_Bool(p_DynamicTracks);
            }

            p_Encoding = xml_attr_get(p_Capabilities, "Encoding");
            if (p_Encoding)
            {
                parse_EncodingList(p_Encoding, p_cap);
            }

            p_MaxRate = xml_attr_get(p_Capabilities, "MaxRate");
            if (p_MaxRate)
            {
                p_cap->MaxRate = (float)atof(p_MaxRate);
            }

            p_MaxTotalRate = xml_attr_get(p_Capabilities, "MaxTotalRate");
            if (p_MaxTotalRate)
            {
                p_cap->MaxTotalRate = (float)atof(p_MaxTotalRate);
            }

            p_MaxRecordings = xml_attr_get(p_Capabilities, "MaxRecordings");
            if (p_MaxRecordings)
            {
                p_cap->MaxRecordings = atoi(p_MaxRecordings);
            }

            p_MaxRecordingJobs = xml_attr_get(p_Capabilities, "MaxRecordingJobs");
            if (p_MaxRecordingJobs)
            {
                p_cap->MaxRecordingJobs = atoi(p_MaxRecordingJobs);
            }

            p_Options = xml_attr_get(p_Capabilities, "Options");
            if (p_Options)
            {
                p_cap->Options = parse_Bool(p_Options);
            }

            p_MetadataRecording = xml_attr_get(p_Capabilities, "MetadataRecording");
            if (p_MetadataRecording)
            {
                p_cap->MetadataRecording = parse_Bool(p_MetadataRecording);
            }
        }
    }

    return TRUE;
}

BOOL parse_SearchService(XMLN * p_node, SEARCH_CAP * p_cap)
{	
	XMLN * p_XAddr;
	XMLN * p_tds_Capabilities;
	
    p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

    p_tds_Capabilities = xml_node_soap_get(p_node, "Capabilities");
    if (p_tds_Capabilities)
    {
        XMLN * p_Capabilities = xml_node_soap_get(p_tds_Capabilities, "Capabilities");
        if (p_Capabilities)
        {
            const char * p_MetadataSearch;
			const char * p_GeneralStartEvents;

			p_MetadataSearch = xml_attr_get(p_Capabilities, "MetadataSearch");
            if (p_MetadataSearch)
            {
                p_cap->MetadataSearch = parse_Bool(p_MetadataSearch);
            }

            p_GeneralStartEvents = xml_attr_get(p_Capabilities, "GeneralStartEvents");
            if (p_GeneralStartEvents)
            {
                p_cap->GeneralStartEvents = parse_Bool(p_GeneralStartEvents);
            }
        }
    }   
   
    return TRUE;
}

BOOL parse_ReplayService(XMLN * p_node, REPLAY_CAP * p_cap)
{
	XMLN * p_XAddr;
	XMLN * p_tds_Capabilities;
	
    p_XAddr = xml_node_soap_get(p_node, "XAddr");
    if (p_XAddr && p_XAddr->data)
    {
        parse_XAddr(p_XAddr->data, &p_cap->xaddr);
    }
    else
    {
        return FALSE;
    }

    p_tds_Capabilities = xml_node_soap_get(p_node, "Capabilities");
    if (p_tds_Capabilities)
    {
        XMLN * p_Capabilities = xml_node_soap_get(p_tds_Capabilities, "Capabilities");
        if (p_Capabilities)
        {
            const char * p_ReversePlayback;
			const char * p_SessionTimeoutRange;
			const char * p_RTP_RTSP_TCP;

			p_ReversePlayback = xml_attr_get(p_Capabilities, "ReversePlayback");
            if (p_ReversePlayback)
            {
                p_cap->ReversePlayback = parse_Bool(p_ReversePlayback);
            }

            p_SessionTimeoutRange = xml_attr_get(p_Capabilities, "SessionTimeoutRange");
            if (p_SessionTimeoutRange)
            {
                parse_FloatRangeList(p_SessionTimeoutRange, &p_cap->SessionTimeoutRange);
            }

            p_RTP_RTSP_TCP = xml_attr_get(p_Capabilities, "RTP_RTSP_TCP");
            if (p_RTP_RTSP_TCP)
            {
                p_cap->RTP_RTSP_TCP = parse_Bool(p_RTP_RTSP_TCP);
            }
        }
    }   
    
    return TRUE;
}

BOOL parse_GetReplayUri(XMLN * p_node, GetReplayUri_RES * p_res)
{
    XMLN * p_Uri = xml_node_soap_get(p_node, "Uri");
    if (p_Uri && p_Uri->data)
    {
        strncpy(p_res->Uri, p_Uri->data, sizeof(p_res->Uri)-1);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

BOOL parse_GetRecordingSummary(XMLN * p_node, GetRecordingSummary_RES * p_res)
{
	XMLN * p_Summary = xml_node_soap_get(p_node, "Summary");
	if (p_Summary)
	{
		XMLN * p_DataFrom;
		XMLN * p_DataUntil;
		XMLN * p_NumberRecordings;

		p_DataFrom = xml_node_soap_get(p_node, "DataFrom");
		if (p_DataFrom && p_DataFrom->data)
		{
			parse_XSDDatetime(p_DataFrom->data, &p_res->DataFrom);
		}

		p_DataUntil = xml_node_soap_get(p_node, "DataUntil");
		if (p_DataUntil && p_DataUntil->data)
		{
			parse_XSDDatetime(p_DataUntil->data, &p_res->DataUntil);
		}

		p_NumberRecordings = xml_node_soap_get(p_node, "NumberRecordings");
		if (p_NumberRecordings && p_NumberRecordings->data)
		{
			p_res->NumberRecordings = atoi(p_NumberRecordings->data);
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL parse_Track(XMLN * p_node, TRACK * p_track)
{
	XMLN * p_TrackToken;
	XMLN * p_TrackType;
	XMLN * p_Description;
	XMLN * p_DataFrom;
	XMLN * p_DataTo;

	p_TrackToken = xml_node_soap_get(p_node, "TrackToken");
	if (p_TrackToken && p_TrackToken->data)
	{
		strncpy(p_track->TrackToken, p_TrackToken->data, sizeof(p_track->TrackToken)-1);
	}

	p_TrackType = xml_node_soap_get(p_node, "TrackType");
	if (p_TrackType && p_TrackType->data)
	{
		p_track->TrackType = onvif_get_track_type(p_TrackType->data);
	}

	p_Description = xml_node_soap_get(p_node, "Description");
	if (p_Description && p_Description->data)
	{
		strncpy(p_track->Description, p_Description->data, sizeof(p_track->Description)-1);
	}

	p_DataFrom = xml_node_soap_get(p_node, "DataFrom");
	if (p_DataFrom && p_DataFrom->data)
	{
		parse_XSDDatetime(p_DataFrom->data, &p_track->DataFrom);
	}

	p_DataTo = xml_node_soap_get(p_node, "DataTo");
	if (p_DataTo && p_DataTo->data)
	{
		parse_XSDDatetime(p_DataTo->data, &p_track->DataTo);
	}

	return TRUE;
}

BOOL parse_RecordingInformation(XMLN * p_node, RECORDING * p_recording)
{
	XMLN * p_RecordingToken;
	XMLN * p_Source;
	XMLN * p_EarliestRecording;
	XMLN * p_LatestRecording;
	XMLN * p_Content;
	XMLN * p_Track;
	XMLN * p_RecordingStatus;

	p_RecordingToken = xml_node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_recording->RecordingToken, p_RecordingToken->data, sizeof(p_recording->RecordingToken)-1);
	}

	p_Source = xml_node_soap_get(p_node, "Source");
	if (p_Source)
	{
		XMLN * p_SourceId;
		XMLN * p_Name;
		XMLN * p_Location;
		XMLN * p_Description;
		XMLN * p_Address;

		p_SourceId = xml_node_soap_get(p_Source, "SourceId");
		if (p_SourceId && p_SourceId->data)
		{
			strncpy(p_recording->SourceId, p_SourceId->data, sizeof(p_recording->SourceId)-1);
		}

		p_Name = xml_node_soap_get(p_Source, "Name");
		if (p_Name && p_Name->data)
		{
			strncpy(p_recording->Name, p_Name->data, sizeof(p_recording->Name)-1);
		}

		p_Location = xml_node_soap_get(p_Source, "Location");
		if (p_Location && p_Location->data)
		{
			strncpy(p_recording->Location, p_Location->data, sizeof(p_recording->Location)-1);
		}

		p_Description = xml_node_soap_get(p_Source, "Description");
		if (p_Description && p_Description->data)
		{
			strncpy(p_recording->Description, p_Description->data, sizeof(p_recording->Description)-1);
		}

		p_Address = xml_node_soap_get(p_Source, "Address");
		if (p_Address && p_Address->data)
		{
			strncpy(p_recording->Address, p_Address->data, sizeof(p_recording->Address)-1);
		}
	}

	p_EarliestRecording = xml_node_soap_get(p_node, "EarliestRecording");
	if (p_EarliestRecording && p_EarliestRecording->data)
	{
		parse_XSDDatetime(p_EarliestRecording->data, &p_recording->EarliestRecording);
	}

	p_LatestRecording = xml_node_soap_get(p_node, "LatestRecording");
	if (p_LatestRecording && p_LatestRecording->data)
	{
		parse_XSDDatetime(p_LatestRecording->data, &p_recording->LatestRecording);
	}

	p_Content = xml_node_soap_get(p_node, "Content");
	if (p_Content && p_Content->data)
	{
		strncpy(p_recording->Content, p_Content->data, sizeof(p_recording->Content)-1);
	}

	p_Track = xml_node_soap_get(p_node, "Track");
	while (p_Track && soap_strcmp(p_Track->name, "Track") == 0)
	{
		TRACK_LIST * p_track = onvif_add_recording_track(&p_recording->Track);
		if (p_track)
		{
			parse_Track(p_Track, &p_track->track);
		}	

		p_Track = p_Track->next;
	}

	p_RecordingStatus = xml_node_soap_get(p_node, "RecordingStatus");
	if (p_RecordingStatus && p_RecordingStatus->data)
	{
		p_recording->RecordingStatus = onvif_get_recording_status(p_RecordingStatus->data);
	}

	return TRUE;
}

BOOL parse_GetRecordingInformation(XMLN * p_node, GetRecordingInformation_RES * p_res)
{
	XMLN * p_RecordingInformation = xml_node_soap_get(p_node, "RecordingInformation");
	if (NULL == p_RecordingInformation)
	{
		return FALSE;
	}

	return parse_RecordingInformation(p_RecordingInformation, &p_res->RecordingInformation);
}

BOOL parse_TrackAttributes(XMLN * p_node, TRACK_ATTR * p_track_attr)
{
	XMLN * p_TrackInformation;
	XMLN * p_VideoAttributes;
	XMLN * p_AudioAttributes;
	XMLN * p_MetadataAttributes;

	p_TrackInformation = xml_node_soap_get(p_node, "TrackInformation");
	if (p_TrackInformation)
	{
		parse_Track(p_TrackInformation, &p_track_attr->TrackInformation);
	}

	p_VideoAttributes = xml_node_soap_get(p_node, "VideoAttributes");
	if (p_VideoAttributes)
	{
		XMLN * p_Bitrate;
		XMLN * p_Width;
		XMLN * p_Height;
		XMLN * p_Encoding;
		XMLN * p_Framerate;

		p_Bitrate = xml_node_soap_get(p_VideoAttributes, "Bitrate");
		if (p_Bitrate && p_Bitrate->data)
		{
			p_track_attr->VideoAttributes.Bitrate = atoi(p_Bitrate->data);
		}

		p_Width = xml_node_soap_get(p_VideoAttributes, "Width");
		if (p_Width && p_Width->data)
		{
			p_track_attr->VideoAttributes.Width = atoi(p_Width->data);
		}

		p_Height = xml_node_soap_get(p_VideoAttributes, "Height");
		if (p_Height && p_Height->data)
		{
			p_track_attr->VideoAttributes.Height = atoi(p_Height->data);
		}

		p_Encoding = xml_node_soap_get(p_VideoAttributes, "Encoding");
		if (p_Encoding && p_Encoding->data)
		{
			p_track_attr->VideoAttributes.Encoding = parse_VideoEncoding(p_Encoding->data);
		}

		p_Framerate = xml_node_soap_get(p_VideoAttributes, "Framerate");
		if (p_Framerate && p_Framerate->data)
		{
			p_track_attr->VideoAttributes.Framerate = (float)atof(p_Framerate->data);
		}
	}

	p_AudioAttributes = xml_node_soap_get(p_node, "AudioAttributes");
	if (p_AudioAttributes)
	{
		XMLN * p_Bitrate;
		XMLN * p_Encoding;
		XMLN * p_Samplerate;

		p_Bitrate = xml_node_soap_get(p_AudioAttributes, "Bitrate");
		if (p_Bitrate && p_Bitrate->data)
		{
			p_track_attr->AudioAttributes.Bitrate = atoi(p_Bitrate->data);
		}
		
		p_Encoding = xml_node_soap_get(p_AudioAttributes, "Encoding");
		if (p_Encoding && p_Encoding->data)
		{
			p_track_attr->AudioAttributes.Encoding = parse_AudioEncoding(p_Encoding->data);
		}

		p_Samplerate = xml_node_soap_get(p_AudioAttributes, "Samplerate");
		if (p_Samplerate && p_Samplerate->data)
		{
			p_track_attr->AudioAttributes.Samplerate = atoi(p_Samplerate->data);
		}
	}

	p_MetadataAttributes = xml_node_soap_get(p_node, "MetadataAttributes");
	if (p_MetadataAttributes)
	{
		XMLN * p_CanContainPTZ;
		XMLN * p_CanContainAnalytics;
		XMLN * p_CanContainNotifications;

		p_CanContainPTZ = xml_node_soap_get(p_MetadataAttributes, "CanContainPTZ");
		if (p_CanContainPTZ && p_CanContainPTZ->data)
		{
			p_track_attr->MetadataAttributes.CanContainPTZ = parse_Bool(p_CanContainPTZ->data);
		}
		
		p_CanContainAnalytics = xml_node_soap_get(p_MetadataAttributes, "CanContainAnalytics");
		if (p_CanContainAnalytics && p_CanContainAnalytics->data)
		{
			p_track_attr->MetadataAttributes.CanContainAnalytics = parse_Bool(p_CanContainAnalytics->data);
		}

		p_CanContainNotifications = xml_node_soap_get(p_MetadataAttributes, "CanContainNotifications");
		if (p_CanContainNotifications && p_CanContainNotifications->data)
		{
			p_track_attr->MetadataAttributes.CanContainNotifications = parse_Bool(p_CanContainNotifications->data);
		}
	}

	return TRUE;
}

BOOL parse_GetMediaAttributes(XMLN * p_node, GetMediaAttributes_RES * p_res)
{
	XMLN * p_MediaAttributes;
	XMLN * p_RecordingToken;
	XMLN * p_TrackAttributes;
	XMLN * p_From;
	XMLN * p_Until;

	p_MediaAttributes = xml_node_soap_get(p_node, "MediaAttributes");
	if (NULL == p_MediaAttributes)
	{
		return FALSE;
	}

	p_RecordingToken = xml_node_soap_get(p_MediaAttributes, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_res->MediaAttributes.RecordingToken, p_RecordingToken->data, sizeof(p_res->MediaAttributes.RecordingToken)-1);
	}

	p_TrackAttributes = xml_node_soap_get(p_MediaAttributes, "TrackAttributes");
	while (p_TrackAttributes && soap_strcmp(p_TrackAttributes->name, "TrackAttributes") == 0)
	{
		TRACK_ATTR_LIST * p_track_attr = onvif_add_track_attr(&p_res->MediaAttributes.TrackAttributes);
		if (p_track_attr)
		{
			parse_TrackAttributes(p_TrackAttributes, &p_track_attr->attr);
		}

		p_TrackAttributes = p_TrackAttributes->next;
	}

	p_From = xml_node_soap_get(p_MediaAttributes, "From");
	if (p_From && p_From->data)
	{
		parse_XSDDatetime(p_From->data, &p_res->MediaAttributes.From);
	}

	p_Until = xml_node_soap_get(p_MediaAttributes, "Until");
	if (p_Until && p_Until->data)
	{
		parse_XSDDatetime(p_Until->data, &p_res->MediaAttributes.Until);
	}

	return TRUE;
}

BOOL parse_FindRecordings(XMLN * p_node, FindRecordings_RES * p_res)
{
	XMLN * p_SearchToken = xml_node_soap_get(p_node, "SearchToken");
	if (p_SearchToken && p_SearchToken->data)
	{
		strncpy(p_res->SearchToken, p_SearchToken->data, sizeof(p_res->SearchToken)-1);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL parse_GetRecordingSearchResults(XMLN * p_node, GetRecordingSearchResults_RES * p_res)
{
	XMLN * p_ResultList;
	XMLN * p_SearchState;
	XMLN * p_RecordingInformation;

	p_ResultList = xml_node_soap_get(p_node, "ResultList");
	if (NULL == p_ResultList)
	{
		return FALSE;
	}

	p_SearchState = xml_node_soap_get(p_ResultList, "SearchState");
	if (p_SearchState && p_SearchState->data)
	{
		p_res->ResultList.SearchState = onvif_get_search_status(p_SearchState->data);
	}
	
	p_RecordingInformation = xml_node_soap_get(p_ResultList, "RecordingInformation");
	while (p_RecordingInformation && soap_strcmp(p_RecordingInformation->name, "RecordingInformation") == 0)
	{
		RECORDING_LIST * p_recording = onvif_add_recording(&p_res->ResultList.RecordingInformation);
		if (p_recording)
		{
			parse_RecordingInformation(p_RecordingInformation, &p_recording->RecordingInformation);
		}
		
		p_RecordingInformation = p_RecordingInformation->next;
	}

	return TRUE;
}

BOOL parse_FindEvents(XMLN * p_node, FindEvents_RES * p_res)
{
	XMLN * p_SearchToken = xml_node_soap_get(p_node, "SearchToken");
	if (p_SearchToken && p_SearchToken->data)
	{
		strncpy(p_res->SearchToken, p_SearchToken->data, sizeof(p_res->SearchToken)-1);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL parse_GetSearchState(XMLN * p_node, GetSearchState_RES * p_res)
{
	XMLN * p_State = xml_node_soap_get(p_node, "State");
	if (p_State && p_State->data)
	{
		p_res->State = onvif_get_search_status(p_State->data);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL parse_EndSearch(XMLN * p_node, EndSearch_RES * p_res)
{
	XMLN * p_Endpoint = xml_node_soap_get(p_node, "Endpoint");
	if (p_Endpoint && p_Endpoint->data)
	{
		parse_XSDDatetime(p_Endpoint->data, &p_res->Endpoint);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}




