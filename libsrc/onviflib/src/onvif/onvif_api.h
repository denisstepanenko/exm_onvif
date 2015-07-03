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
#ifndef ONVIF_API_H
#define ONVIF_API_H

#include "sys_inc.h"
#include "onvif.h"


#ifdef __cplusplus
extern "C" {
#endif

BOOL GetCapabilities(ONVIF_DEVICE * p_dev);
BOOL GetServices(ONVIF_DEVICE * p_dev);
BOOL GetDeviceInformation(ONVIF_DEVICE * p_dev);
BOOL GetNetworkInterfaces(ONVIF_DEVICE * p_dev);
BOOL GetNTP(ONVIF_DEVICE * p_dev);
BOOL GetHostname(ONVIF_DEVICE * p_dev);
BOOL GetDNS(ONVIF_DEVICE * p_dev);
BOOL GetDynamicDNS(ONVIF_DEVICE * p_dev);
BOOL GetNetworkProtocols(ONVIF_DEVICE * p_dev);
BOOL GetNetworkDefaultGateway(ONVIF_DEVICE * p_dev);
BOOL GetDiscoveryMode(ONVIF_DEVICE * p_dev);
BOOL GetProfiles(ONVIF_DEVICE * p_dev);
BOOL GetStreamUris(ONVIF_DEVICE * p_dev);
BOOL GetVideoSourceConfigurations(ONVIF_DEVICE * p_dev);
BOOL GetAudioSourceConfigurations(ONVIF_DEVICE * p_dev);
BOOL GetVideoEncoderConfigurations(ONVIF_DEVICE * p_dev);
BOOL GetAudioEncoderConfigurations(ONVIF_DEVICE * p_dev);
BOOL GetVideoEncoderConfigurationOptions(ONVIF_DEVICE * p_dev);
BOOL GetNodes(ONVIF_DEVICE * p_dev);
BOOL GetPresets(ONVIF_DEVICE * p_dev);
BOOL GetConfigurations(ONVIF_DEVICE * p_dev);
BOOL GetVideoSources(ONVIF_DEVICE * p_dev);
BOOL GetAudioSources(ONVIF_DEVICE * p_dev);
BOOL GetImagingSettings(ONVIF_DEVICE * p_dev);
BOOL Subscribe(ONVIF_DEVICE * p_dev, int index);
BOOL Unsubscribe(ONVIF_DEVICE * p_dev);

#ifdef __cplusplus
}
#endif

#endif



