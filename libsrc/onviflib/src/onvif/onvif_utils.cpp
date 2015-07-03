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
#include "onvif_utils.h"
#include "onvif.h"


/***************************************************************************************/

const char * onvif_get_video_encoding_str(E_VIDEO_ENCODING encoding)
{
	switch (encoding)
    {
    case VIDEO_ENCODING_JPEG:
        return "JPEG";

    case VIDEO_ENCODING_MPEG4:
        return "MPEG4";

    case VIDEO_ENCODING_H264:
        return "H264";
    }

    return "H264";
}


const char * onvif_get_audio_encoding_str(E_AUDIO_ENCODING encoding)
{
    switch (encoding)
    {
    case AUDIO_ENCODING_G711:
        return "G711";

    case AUDIO_ENCODING_G726:
    	return "G726";

    case AUDIO_ENCODING_AAC:
    	return "AAC";
    }

    return "G711";
}


const char * onvif_get_h264_profile_str(E_H264_PROFILE profile)
{
    switch (profile)
    {
    case H264_PROFILE_Baseline:
        return "Baseline";

    case H264_PROFILE_Extended:
        return "Extended";

    case H264_PROFILE_High:
        return "High";

    case H264_PROFILE_Main:
        return "Main";
    }

    return "Baseline";
}

const char * onvif_get_mpeg4_profile_str(E_MPEG4_PROFILE profile)
{
    switch (profile)
    {
    case MPEG4_PROFILE_SP:
        return "SP";

    case MPEG4_PROFILE_ASP:
        return "ASP";    
    }

    return "SP";
}


const char * onvif_get_cap_str(E_CAP_CATEGORY cap)
{
    switch (cap)
    {
    case CAP_CATEGORY_ALL:
        return "All";

    case CAP_CATEGORY_ANALYTICS:
        return "Analytics";

    case CAP_CATEGORY_DEVICE:
        return "Device";

    case CAP_CATEGORY_EVENTS:
        return "Events";

    case CAP_CATEGORY_IMAGE:
        return "Imaging";

    case CAP_CATEGORY_MEDIA:
        return "Media";

    case CAP_CATEGORY_PTZ:
        return "PTZ";
    }  
    	    
    return NULL;
}

const char * onvif_get_ddns_type_str(int type)
{
    if (type == 1)
    {
        return "ClientUpdates";
    }
    else if (type == 2)
    {
        return "ServerUpdates";
    }

    return "NoUpdate";
}

int	onvif_get_ddns_type(const char * data)
{
	if (strcasecmp(data, "ClientUpdates") == 0)
	{
		return 1;
	}
	else if (strcasecmp(data, "ServerUpdates") == 0)
	{
		return 2;
	}

	return 0;
}

E_PTZ_STATUS onvif_get_ptz_status(const char * data)
{
	if (strcasecmp(data, "IDLE") == 0)
	{
		return PTZ_STA_IDLE;
	}
	else if (strcasecmp(data, "MOVING") == 0)
	{
		return PTZ_STA_MOVING;
	}
	else if (strcasecmp(data, "UNKNOWN") == 0)
	{
		return PTZ_STA_UNKNOWN;
	}
	
	return PTZ_STA_UNKNOWN;
}

time_t onvif_datetime_to_time_t(DATETIME * p_datetime)
{
    struct tm t1;

    t1.tm_year = p_datetime->year - 1900;
    t1.tm_mon = p_datetime->month - 1;
    t1.tm_mday = p_datetime->day;
    t1.tm_hour = p_datetime->hour;
    t1.tm_min = p_datetime->minute;
    t1.tm_sec = p_datetime->second;

    return mktime(&t1);
}

void onvif_time_t_to_datetime(time_t n, DATETIME * p_datetime)
{
    struct tm *t1 = gmtime(&n);

    p_datetime->year = t1->tm_year + 1900;
    p_datetime->month = t1->tm_mon + 1;
    p_datetime->day = t1->tm_mday;
    p_datetime->hour = t1->tm_hour;
    p_datetime->minute = t1->tm_min;
    p_datetime->second = t1->tm_sec;
}

const char * onvif_format_datetime_str(time_t n, int flag /*0 -- local, 1 -- utc*/, const char * format)
{
    struct tm *t1;
  	static char buff[100];

	memset(buff, 0, sizeof(buff));

    if (flag == 1)
    {
  	    t1 = gmtime(&n);
  	}
  	else
  	{
  	    t1 = localtime(&n);
  	}
  	
	strftime(buff, sizeof(buff), format, t1);

	return buff;
}

const char * onvif_uuid_create()
{
	static char uuid[100];

	srand((unsigned int)time(NULL));
	sprintf(uuid, "%04x%04x-%04x-%04x-%04x-%04x%04x%04x", rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand());

	return uuid;
}

time_t onvif_timegm(struct tm *T)
{
	time_t t, g, z;
	struct tm tm;
	
	t = mktime(T);	
	if (t == (time_t)-1)
	{
		return (time_t)-1;
	}
	
	tm = *gmtime(&t);

	tm.tm_isdst = 0;
	g = mktime(&tm);
	if (g == (time_t)-1)
	{
		return (time_t)-1;
	}
	
	z = g - t;
	return t - z;
}


RECORDINGSTATUS onvif_get_recording_status(const char * data)
{
	if (strcasecmp(data, "Initiated") == 0)
	{
		return RecordingStatus__Initiated;
	}
	else if (strcasecmp(data, "Recording") == 0)
	{
		return RecordingStatus__Recording;
	}
	else if (strcasecmp(data, "Stopped") == 0)
	{
		return RecordingStatus__Stopped;
	}
	else if (strcasecmp(data, "Removing") == 0)
	{
		return RecordingStatus__Removing;
	}
	else if (strcasecmp(data, "Removed") == 0)
	{
		return RecordingStatus__Removed;
	}

	return RecordingStatus__Unknown;
}

TRACKTYPE onvif_get_track_type(const char * data)
{	
	if (strcasecmp(data, "Video") == 0)
	{
		return TrackType__Video;
	}
	else if (strcasecmp(data, "Audio") == 0)
	{
		return TrackType__Audio;
	}
	else if (strcasecmp(data, "Metadata") == 0)
	{
		return TrackType__Metadata;
	}

	return TrackType__Extended;
}

SEARCHSTATUS onvif_get_search_status(const char * data)
{	
	if (strcasecmp(data, "Queued") == 0)
	{
		return SearchState__Queued;
	}
	else if (strcasecmp(data, "Searching") == 0)
	{
		return SearchState__Searching;
	}
	else if (strcasecmp(data, "Completed") == 0)
	{
		return SearchState__Completed;
	}

	return SearchState__Unknown;
}




