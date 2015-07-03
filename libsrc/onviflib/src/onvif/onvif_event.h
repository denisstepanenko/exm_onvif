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

#ifndef ONVIF_EVENT_H
#define ONVIF_EVENT_H

#include "sys_inc.h"
#include "onvif.h"
#include "xml_node.h"


typedef void (* onvif_event_notify_cb)(Notify_REQ * p_req, void * pdata);

#ifdef __cplusplus
extern "C" {
#endif

void onvif_set_event_notify_cb(onvif_event_notify_cb cb, void * pdata);
void onvif_event_notify(XMLN * p_xml);
void onvif_event_timer_init(ONVIF_DEVICE * p_dev);
void onvif_event_timer_deinit(ONVIF_DEVICE * p_dev);

#ifdef __cplusplus
}
#endif


#endif



