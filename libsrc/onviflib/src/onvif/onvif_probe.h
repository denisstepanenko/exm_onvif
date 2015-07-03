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

#ifndef	__H_ONVIF_PROBE_H__
#define	__H_ONVIF_PROBE_H__

#include "onvif.h"


typedef void (* onvif_probe_cb)(DEVICE_BINFO * p_res, void * pdata);

#ifdef __cplusplus
extern "C" {
#endif

void set_probe_cb(onvif_probe_cb cb, void * pdata);
int  start_probe(int interval,bool once=false);
void stop_probe();
void send_probe_req();


#ifdef __cplusplus
}
#endif

#endif	//	__H_ONVIF_PROBE_H__


