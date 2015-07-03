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

#ifndef ONVIF_CLIENT_H
#define ONVIF_CLIENT_H

#include "onvif.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL onvif_GetCapabilities(ONVIF_DEVICE * p_dev, GetCapabilities_REQ * p_req, GetCapabilities_RES * p_res);
BOOL onvif_GetServices(ONVIF_DEVICE * p_dev, GetServices_REQ * p_req, GetServices_RES * p_res);
BOOL onvif_GetDeviceInformation(ONVIF_DEVICE * p_dev, GetDeviceInformation_REQ * p_req, GetDeviceInformation_RES * p_res);
BOOL onvif_GetNetworkInterfaces(ONVIF_DEVICE * p_dev, GetNetworkInterfaces_REQ * p_req, GetNetworkInterfaces_RES * p_res);
BOOL onvif_SetNetworkInterfaces(ONVIF_DEVICE * p_dev, SetNetworkInterfaces_REQ * p_req, SetNetworkInterfaces_RES * p_res);
BOOL onvif_GetNTP(ONVIF_DEVICE * p_dev, GetNTP_REQ * p_req, GetNTP_RES * p_res);
BOOL onvif_SetNTP(ONVIF_DEVICE * p_dev, SetNTP_REQ * p_req, SetNTP_RES * p_res);
BOOL onvif_GetHostname(ONVIF_DEVICE * p_dev, GetHostname_REQ * p_req, GetHostname_RES * p_res);
BOOL onvif_SetHostname(ONVIF_DEVICE * p_dev, SetHostname_REQ * p_req, SetHostname_RES * p_res);
BOOL onvif_SetHostnameFromDHCP(ONVIF_DEVICE * p_dev, SetHostnameFromDHCP_REQ * p_req, SetHostnameFromDHCP_RES * p_res);
BOOL onvif_GetDNS(ONVIF_DEVICE * p_dev, GetDNS_REQ * p_req, GetDNS_RES * p_res);
BOOL onvif_SetDNS(ONVIF_DEVICE * p_dev, SetDNS_REQ * p_req, SetDNS_RES * p_res);
BOOL onvif_GetDynamicDNS(ONVIF_DEVICE * p_dev, GetDynamicDNS_REQ * p_req, GetDynamicDNS_RES * p_res);
BOOL onvif_SetDynamicDNS(ONVIF_DEVICE * p_dev, SetDynamicDNS_REQ * p_req, SetDynamicDNS_RES * p_res);
BOOL onvif_GetNetworkProtocols(ONVIF_DEVICE * p_dev, GetNetworkProtocols_REQ * p_req, GetNetworkProtocols_RES * p_res);
BOOL onvif_SetNetworkProtocols(ONVIF_DEVICE * p_dev, SetNetworkProtocols_REQ * p_req, SetNetworkProtocols_RES * p_res);
BOOL onvif_GetDiscoveryMode(ONVIF_DEVICE * p_dev, GetDiscoveryMode_REQ * p_req, GetDiscoveryMode_RES * p_res);
BOOL onvif_SetDiscoveryMode(ONVIF_DEVICE * p_dev, SetDiscoveryMode_REQ * p_req, SetDiscoveryMode_RES * p_res);
BOOL onvif_GetNetworkDefaultGateway(ONVIF_DEVICE * p_dev, GetNetworkDefaultGateway_REQ * p_req, GetNetworkDefaultGateway_RES * p_res);
BOOL onvif_SetNetworkDefaultGateway(ONVIF_DEVICE * p_dev, SetNetworkDefaultGateway_REQ * p_req, SetNetworkDefaultGateway_RES * p_res);
BOOL onvif_GetSystemDateAndTime(ONVIF_DEVICE * p_dev, GetSystemDateAndTime_REQ * p_req, GetSystemDateAndTime_RES * p_res);
BOOL onvif_SetSystemDateAndTime(ONVIF_DEVICE * p_dev, SetSystemDateAndTime_REQ * p_req, SetSystemDateAndTime_RES * p_res);
BOOL onvif_SystemReboot(ONVIF_DEVICE * p_dev, SystemReboot_REQ * p_req, SystemReboot_RES * p_res);
BOOL onvif_SetSystemFactoryDefault(ONVIF_DEVICE * p_dev, SetSystemFactoryDefault_REQ * p_req, SetSystemFactoryDefault_RES * p_res);
BOOL onvif_GetSystemLog(ONVIF_DEVICE * p_dev, GetSystemLog_REQ * p_req, GetSystemLog_RES * p_res);
BOOL onvif_GetScopes(ONVIF_DEVICE * p_dev, GetScopes_REQ * p_req, GetScopes_RES * p_res);
BOOL onvif_SetScopes(ONVIF_DEVICE * p_dev, SetScopes_REQ * p_req, SetScopes_RES * p_res);
BOOL onvif_AddScopes(ONVIF_DEVICE * p_dev, AddScopes_REQ * p_req, AddScopes_RES * p_res);
BOOL onvif_RemoveScopes(ONVIF_DEVICE * p_dev, RemoveScopes_REQ * p_req, RemoveScopes_RES * p_res);
BOOL onvif_GetVideoSources(ONVIF_DEVICE * p_dev, GetVideoSources_REQ * p_req, GetVideoSources_RES * p_res);
BOOL onvif_GetAudioSources(ONVIF_DEVICE * p_dev, GetAudioSources_REQ * p_req, GetAudioSources_RES * p_res);
BOOL onvif_CreateProfile(ONVIF_DEVICE * p_dev, CreateProfile_REQ * p_req, CreateProfile_RES * p_res);
BOOL onvif_GetProfile(ONVIF_DEVICE * p_dev, GetProfile_REQ * p_req, GetProfile_RES * p_res);
BOOL onvif_GetProfiles(ONVIF_DEVICE * p_dev, GetProfiles_REQ * p_req, GetProfiles_RES * p_res);
BOOL onvif_AddVideoEncoderConfiguration(ONVIF_DEVICE * p_dev, AddVideoEncoderConfiguration_REQ * p_req, AddVideoEncoderConfiguration_RES * p_res);
BOOL onvif_AddVideoSourceConfiguration(ONVIF_DEVICE * p_dev, AddVideoSourceConfiguration_REQ * p_req, AddVideoSourceConfiguration_RES * p_res);
BOOL onvif_AddAudioEncoderConfiguration(ONVIF_DEVICE * p_dev, AddAudioEncoderConfiguration_REQ * p_req, AddAudioEncoderConfiguration_RES * p_res);
BOOL onvif_AddAudioSourceConfiguration(ONVIF_DEVICE * p_dev, AddAudioSourceConfiguration_REQ * p_req, AddAudioSourceConfiguration_RES * p_res);
BOOL onvif_AddPTZConfiguration(ONVIF_DEVICE * p_dev, AddPTZConfiguration_REQ * p_req, AddPTZConfiguration_RES * p_res);
BOOL onvif_RemoveVideoEncoderConfiguration(ONVIF_DEVICE * p_dev, RemoveVideoEncoderConfiguration_REQ * p_req, RemoveVideoEncoderConfiguration_RES * p_res);
BOOL onvif_RemoveVideoSourceConfiguration(ONVIF_DEVICE * p_dev, RemoveVideoSourceConfiguration_REQ * p_req, RemoveVideoSourceConfiguration_RES * p_res);
BOOL onvif_RemoveAudioEncoderConfiguration(ONVIF_DEVICE * p_dev, RemoveAudioEncoderConfiguration_REQ * p_req, RemoveAudioEncoderConfiguration_RES * p_res);
BOOL onvif_RemoveAudioSourceConfiguration(ONVIF_DEVICE * p_dev, RemoveAudioSourceConfiguration_REQ * p_req, RemoveAudioSourceConfiguration_RES * p_res);
BOOL onvif_RemovePTZConfiguration(ONVIF_DEVICE * p_dev, RemovePTZConfiguration_REQ * p_req, RemovePTZConfiguration_RES * p_res);
BOOL onvif_DeleteProfile(ONVIF_DEVICE * p_dev, DeleteProfile_REQ * p_req, DeleteProfile_RES * p_res);
BOOL onvif_GetVideoSourceConfigurations(ONVIF_DEVICE * p_dev, GetVideoSourceConfigurations_REQ * p_req, GetVideoSourceConfigurations_RES * p_res);
BOOL onvif_GetVideoEncoderConfigurations(ONVIF_DEVICE * p_dev, GetVideoEncoderConfigurations_REQ * p_req, GetVideoEncoderConfigurations_RES * p_res);
BOOL onvif_GetAudioSourceConfigurations(ONVIF_DEVICE * p_dev, GetAudioSourceConfigurations_REQ * p_req, GetAudioSourceConfigurations_RES * p_res);
BOOL onvif_GetAudioEncoderConfigurations(ONVIF_DEVICE * p_dev, GetAudioEncoderConfigurations_REQ * p_req, GetAudioEncoderConfigurations_RES * p_res);
BOOL onvif_GetVideoSourceConfiguration(ONVIF_DEVICE * p_dev, GetVideoSourceConfiguration_REQ * p_req, GetVideoSourceConfiguration_RES * p_res);
BOOL onvif_GetVideoEncoderConfiguration(ONVIF_DEVICE * p_dev, GetVideoEncoderConfiguration_REQ * p_req, GetVideoEncoderConfiguration_RES * p_res);
BOOL onvif_GetAudioSourceConfiguration(ONVIF_DEVICE * p_dev, GetAudioSourceConfiguration_REQ * p_req, GetAudioSourceConfiguration_RES * p_res);
BOOL onvif_GetAudioEncoderConfiguration(ONVIF_DEVICE * p_dev, GetAudioEncoderConfiguration_REQ * p_req, GetAudioEncoderConfiguration_RES * p_res);
BOOL onvif_SetVideoSourceConfiguration(ONVIF_DEVICE * p_dev, SetVideoSourceConfiguration_REQ * p_req, SetVideoSourceConfiguration_RES * p_res);
BOOL onvif_SetVideoEncoderConfiguration(ONVIF_DEVICE * p_dev, SetVideoEncoderConfiguration_REQ * p_req, SetVideoEncoderConfiguration_RES * p_res);
BOOL onvif_SetAudioSourceConfiguration(ONVIF_DEVICE * p_dev, SetAudioSourceConfiguration_REQ * p_req, SetAudioSourceConfiguration_RES * p_res);
BOOL onvif_SetAudioEncoderConfiguration(ONVIF_DEVICE * p_dev, SetAudioEncoderConfiguration_REQ * p_req, SetAudioEncoderConfiguration_RES * p_res);
BOOL onvif_GetVideoSourceConfigurationOptions(ONVIF_DEVICE * p_dev, GetVideoSourceConfigurationOptions_REQ * p_req, GetVideoSourceConfigurationOptions_RES * p_res);
BOOL onvif_GetVideoEncoderConfigurationOptions(ONVIF_DEVICE * p_dev, GetVideoEncoderConfigurationOptions_REQ * p_req, GetVideoEncoderConfigurationOptions_RES * p_res);
BOOL onvif_GetAudioSourceConfigurationOptions(ONVIF_DEVICE * p_dev, GetAudioSourceConfigurationOptions_REQ * p_req, GetAudioSourceConfigurationOptions_RES * p_res);
BOOL onvif_GetAudioEncoderConfigurationOptions(ONVIF_DEVICE * p_dev, GetAudioEncoderConfigurationOptions_REQ * p_req, GetAudioEncoderConfigurationOptions_RES * p_res);
BOOL onvif_GetStreamUri(ONVIF_DEVICE * p_dev, GetStreamUri_REQ * p_req, GetStreamUri_RES * p_res);
BOOL onvif_SetSynchronizationPoint(ONVIF_DEVICE * p_dev, SetSynchronizationPoint_REQ * p_req, SetSynchronizationPoint_RES * p_res);
BOOL onvif_GetSnapshotUri(ONVIF_DEVICE * p_dev, GetSnapshotUri_REQ * p_req, GetSnapshotUri_RES * p_res);
BOOL onvif_GetNodes(ONVIF_DEVICE * p_dev, GetNodes_REQ * p_req, GetNodes_RES * p_res);
BOOL onvif_GetNode(ONVIF_DEVICE * p_dev, GetNode_REQ * p_req, GetNode_RES * p_res);
BOOL onvif_GetPresets(ONVIF_DEVICE * p_dev, GetPresets_REQ * p_req, GetPresets_RES * p_res);
BOOL onvif_SetPreset(ONVIF_DEVICE * p_dev, SetPreset_REQ * p_req, SetPreset_RES * p_res);
BOOL onvif_RemovePreset(ONVIF_DEVICE * p_dev, RemovePreset_REQ * p_req, RemovePreset_RES * p_res);
BOOL onvif_GotoPreset(ONVIF_DEVICE * p_dev, GotoPreset_REQ * p_req, GotoPreset_RES * p_res);
BOOL onvif_GotoHomePosition(ONVIF_DEVICE * p_dev, GotoHomePosition_REQ * p_req, GotoHomePosition_RES * p_res);
BOOL onvif_SetHomePosition(ONVIF_DEVICE * p_dev, SetHomePosition_REQ * p_req, SetHomePosition_RES * p_res);
BOOL onvif_GetStatus(ONVIF_DEVICE * p_dev, GetStatus_REQ * p_req, GetStatus_RES * p_res);
BOOL onvif_ContinuousMove(ONVIF_DEVICE * p_dev, ContinuousMove_REQ * p_req, ContinuousMove_RES * p_res);
BOOL onvif_RelativeMove(ONVIF_DEVICE * p_dev, RelativeMove_REQ * p_req, RelativeMove_RES * p_res);
BOOL onvif_AbsoluteMove(ONVIF_DEVICE * p_dev, AbsoluteMove_REQ * p_req, AbsoluteMove_RES * p_res);
BOOL onvif_PTZStop(ONVIF_DEVICE * p_dev, PTZStop_REQ * p_req, PTZStop_RES * p_res);
BOOL onvif_GetConfigurations(ONVIF_DEVICE * p_dev, GetConfigurations_REQ * p_req, GetConfigurations_RES * p_res);
BOOL onvif_GetConfiguration(ONVIF_DEVICE * p_dev, GetConfiguration_REQ * p_req, GetConfiguration_RES * p_res);
BOOL onvif_SetConfiguration(ONVIF_DEVICE * p_dev, SetConfiguration_REQ * p_req, SetConfiguration_RES * p_res);
BOOL onvif_GetConfigurationOptions(ONVIF_DEVICE * p_dev, GetConfigurationOptions_REQ * p_req, GetConfigurationOptions_RES * p_res);
BOOL onvif_GetEventProperties(ONVIF_DEVICE * p_dev, GetEventProperties_REQ * p_req, GetEventProperties_RES * p_res);
BOOL onvif_Renew(ONVIF_DEVICE * p_dev, Renew_REQ * p_req, Renew_RES * p_res);
BOOL onvif_Unsubscribe(ONVIF_DEVICE * p_dev, Unsubscribe_REQ * p_req, Unsubscribe_RES * p_res);
BOOL onvif_Subscribe(ONVIF_DEVICE * p_dev, Subscribe_REQ * p_req, Subscribe_RES * p_res);
BOOL onvif_GetImagingSettings(ONVIF_DEVICE * p_dev, GetImagingSettings_REQ * p_req, GetImagingSettings_RES * p_res);
BOOL onvif_SetImagingSettings(ONVIF_DEVICE * p_dev, SetImagingSettings_REQ * p_req, SetImagingSettings_RES * p_res);
BOOL onvif_GetOptions(ONVIF_DEVICE * p_dev, GetOptions_REQ * p_req, GetOptions_RES * p_res);

BOOL onvif_GetReplayUri(ONVIF_DEVICE * p_dev, GetReplayUri_REQ * p_req, GetReplayUri_RES * p_res);
BOOL onvif_GetRecordingSummary(ONVIF_DEVICE * p_dev, GetRecordingSummary_REQ * p_req, GetRecordingSummary_RES * p_res);
BOOL onvif_GetRecordingInformation(ONVIF_DEVICE * p_dev, GetRecordingInformation_REQ * p_req, GetRecordingInformation_RES * p_res);
BOOL onvif_GetMediaAttributes(ONVIF_DEVICE * p_dev, GetMediaAttributes_REQ * p_req, GetMediaAttributes_RES * p_res);
BOOL onvif_FindRecordings(ONVIF_DEVICE * p_dev, FindRecordings_REQ * p_req, FindRecordings_RES * p_res);
BOOL onvif_GetRecordingSearchResults(ONVIF_DEVICE * p_dev, GetRecordingSearchResults_REQ * p_req, GetRecordingSearchResults_RES * p_res);
BOOL onvif_FindEvents(ONVIF_DEVICE * p_dev, FindEvents_REQ * p_req, FindEvents_RES * p_res);
BOOL onvif_GetEventSearchResults(ONVIF_DEVICE * p_dev, GetEventSearchResults_REQ * p_req, GetEventSearchResults_RES * p_res);
BOOL onvif_GetSearchState(ONVIF_DEVICE * p_dev, GetSearchState_REQ * p_req, GetSearchState_RES * p_res);
BOOL onvif_EndSearch(ONVIF_DEVICE * p_dev, EndSearch_REQ * p_req, EndSearch_RES * p_res);


#ifdef __cplusplus
}
#endif

#endif


