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

#ifndef _ONVIF_UTILS_H_
#define _ONVIF_UTILS_H_

#include "sys_inc.h"
#include "onvif.h"

#ifdef __cplusplus
extern "C" {
#endif

const char * onvif_get_video_encoding_str(E_VIDEO_ENCODING encoding);
const char * onvif_get_audio_encoding_str(E_AUDIO_ENCODING encoding);
const char * onvif_get_h264_profile_str(E_H264_PROFILE profile);
const char * onvif_get_mpeg4_profile_str(E_MPEG4_PROFILE profile);
const char * onvif_get_cap_str(E_CAP_CATEGORY cap);
const char * onvif_get_ddns_type_str(int type);

int			 onvif_get_ddns_type(const char * data);
E_PTZ_STATUS onvif_get_ptz_status(const char * data);
time_t       onvif_datetime_to_time_t(DATETIME * p_datetime);
void         onvif_time_t_to_datetime(time_t n, DATETIME * p_datetime);
const char * onvif_format_datetime_str(time_t n, int flag, const char * format);
const char * onvif_uuid_create();
time_t		 onvif_timegm(struct tm *T);

RECORDINGSTATUS onvif_get_recording_status(const char * data);
TRACKTYPE 	 onvif_get_track_type(const char * data);
SEARCHSTATUS onvif_get_search_status(const char * data);

#ifdef __cplusplus
}
#endif


#endif


