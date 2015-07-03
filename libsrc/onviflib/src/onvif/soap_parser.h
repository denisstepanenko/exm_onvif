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

#ifndef SOAP_PARSER_H
#define SOAP_PARSER_H

#include "sys_inc.h"
#include "onvif.h"
#include "xml_node.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL parse_Bool(const char * pdata);
int  parse_XAddr(const char * pdata, ONVIF_XADDR * p_xaddr);
int  parse_DeviceType(const char * pdata);

BOOL parse_Profile(XMLN * p_node, ONVIF_PROFILE * p_profile);
BOOL parse_VideoSourceConfiguration(XMLN * p_node, VIDEO_SRC_CFG * p_v_src_cfg);
BOOL parse_AudioSourceConfiguration(XMLN * p_node, AUDIO_SRC_CFG * p_a_src_cfg);
BOOL parse_VideoEncoder(XMLN * p_node, VIDEO_ENCODER * p_v_enc);
BOOL parse_AudioEncoder(XMLN * p_node, AUDIO_ENCODER * p_a_enc);
BOOL parse_PTZConfiguration(XMLN * p_node, PTZ_CFG * p_ptz_cfg);
BOOL parse_PTZNode(XMLN * p_node, PTZ_NODE * p_ptz_node);
BOOL parse_Preset(XMLN * p_node, PTZ_PRESET * p_preset);
BOOL parse_NetworkInterface(XMLN * p_node, ONVIF_NET_INF * p_net_inf);
BOOL parse_ImagingSettings(XMLN * p_node, IMAGE_CFG * p_img_cfg);
BOOL parse_VideoSource(XMLN * p_node, VIDEO_SRC * p_v_src);
BOOL parse_AudioSource(XMLN * p_node, AUDIO_SRC * p_a_src);

BOOL parse_SetNetworkInterfaces(XMLN * p_node, SetNetworkInterfaces_RES * p_res);
BOOL parse_GetCapabilities(XMLN * p_node, GetCapabilities_RES * p_res);
BOOL parse_GetDeviceInformation(XMLN * p_node, GetDeviceInformation_RES * p_res);
BOOL parse_GetStreamUri(XMLN * p_node, GetStreamUri_RES * p_res);

BOOL parse_GetConfigurationOptions(XMLN * p_node, GetConfigurationOptions_RES * p_res);
BOOL parse_SetHostnameFromDHCP(XMLN * p_node, SetHostnameFromDHCP_RES * p_res);
BOOL parse_GetNTP(XMLN * p_node, GetNTP_RES * p_res);
BOOL parse_GetHostname(XMLN * p_node, GetHostname_RES * p_res);
BOOL parse_GetDNS(XMLN * p_node, GetDNS_RES * p_res);
BOOL parse_GetDynamicDNS(XMLN * p_node, GetDynamicDNS_RES * p_res);
BOOL parse_GetNetworkProtocols(XMLN * p_node, GetNetworkProtocols_RES * p_res);
BOOL parse_GetDiscoveryMode(XMLN * p_node, GetDiscoveryMode_RES * p_res);
BOOL parse_GetNetworkDefaultGateway(XMLN * p_node, GetNetworkDefaultGateway_RES * p_res);
BOOL parse_GetSnapshotUri(XMLN * p_node, GetSnapshotUri_RES * p_res);
BOOL parse_GetVideoEncoderConfigurationOptions(XMLN * p_node, GetVideoEncoderConfigurationOptions_RES * p_res);
BOOL parse_SetPreset(XMLN * p_node, SetPreset_RES * p_res);
BOOL parse_GetStatus(XMLN * p_node, GetStatus_RES * p_res);
BOOL parseGetSystemDateAndTime(XMLN * p_node, GetSystemDateAndTime_RES * p_res);
BOOL parse_GetOptions(XMLN * p_node, GetOptions_RES * p_res);
BOOL parse_Subscribe(XMLN * p_node, Subscribe_RES * p_res);
BOOL parse_Notify(XMLN * p_node, ONVIF_NOTIFY * p_notify);

BOOL parse_DeviceService(XMLN * p_node, DEVICE_CAP * p_cap);
BOOL parse_MediaService(XMLN * p_node, MEDIA_CAP * p_cap);
BOOL parse_EventsService(XMLN * p_node, EVENT_CAP * p_cap);
BOOL parse_PTZService(XMLN * p_node, PTZ_CAP * p_cap);
BOOL parse_ImageingService(XMLN * p_node, IMAGE_CAP * p_cap);
BOOL parse_RecordingService(XMLN * p_node, RECORDING_CAP * p_cap);
BOOL parse_SearchService(XMLN * p_node, SEARCH_CAP * p_cap);
BOOL parse_ReplayService(XMLN * p_node, REPLAY_CAP * p_cap);

BOOL parse_GetReplayUri(XMLN * p_node, GetReplayUri_RES * p_res);
BOOL parse_GetRecordingSummary(XMLN * p_node, GetRecordingSummary_RES * p_res);
BOOL parse_GetRecordingInformation(XMLN * p_node, GetRecordingInformation_RES * p_res);
BOOL parse_GetMediaAttributes(XMLN * p_node, GetMediaAttributes_RES * p_res);
BOOL parse_FindRecordings(XMLN * p_node, FindRecordings_RES * p_res);
BOOL parse_GetRecordingSearchResults(XMLN * p_node, GetRecordingSearchResults_RES * p_res);
BOOL parse_FindEvents(XMLN * p_node, FindEvents_RES * p_res);
BOOL parse_GetSearchState(XMLN * p_node, GetSearchState_RES * p_res);
BOOL parse_EndSearch(XMLN * p_node, EndSearch_RES * p_res);

#ifdef __cplusplus
}
#endif

#endif


