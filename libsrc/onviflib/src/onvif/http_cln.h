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

#ifndef HTTP_CLN_H
#define HTTP_CLN_H

#include "onvif.h"


#ifdef __cplusplus
extern "C" {
#endif

BOOL http_onvif_trans(HTTPREQ * p_http, int timeout, eOnvifAction act, ONVIF_DEVICE * p_dev, void * p_req, void * p_res);

#ifdef __cplusplus
}
#endif

#endif


