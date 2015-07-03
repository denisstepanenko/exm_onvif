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
#include "onvif_event.h"
#include "soap_parser.h"
#include "onvif_cln.h"


/***************************************************************************************/
onvif_event_notify_cb g_notify_cb = 0;
void * g_notify_cb_data = 0;

/***************************************************************************************/
void onvif_set_event_notify_cb(onvif_event_notify_cb cb, void * pdata)
{
    g_notify_cb = cb;
    g_notify_cb_data = pdata;
}

void onvif_event_notify(XMLN * p_xml)
{
	time_t arrival_time;
    Notify_REQ req;
	XMLN * p_NotificationMessage;
	
    memset(&req, 0, sizeof(req));

    arrival_time = time(NULL);
    p_NotificationMessage = xml_node_soap_get(p_xml, "NotificationMessage");
    while (p_NotificationMessage && soap_strcmp(p_NotificationMessage->name, "NotificationMessage") == 0)
    {
        ONVIF_NOTIFY * p_notify = onvif_add_notify(&req.notify);
        if (p_notify)
        {
            if (parse_Notify(p_NotificationMessage, p_notify) == FALSE)
            {
                return;
            }
            p_notify->arrival_time = arrival_time;

            if (p_notify->operation[0] == '\0')
            {
            	strcpy(p_notify->operation, "Initialized");
            }
        }

        p_NotificationMessage = p_NotificationMessage->next;
    }

    if (g_notify_cb)    
    {
        g_notify_cb(&req, g_notify_cb_data);
    }
}


void onvif_event_timer(ONVIF_DEVICE * p_dev)
{
    Renew_REQ req;
    req.term_time = p_dev->events.init_term_time;
    
	onvif_Renew(p_dev, &req, NULL);
}

#if __WIN32_OS__

void CALLBACK onvif_event_timer_win(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
    ONVIF_DEVICE * p_dev = (ONVIF_DEVICE *) dwUser;

    onvif_event_timer(p_dev);
}

void onvif_event_timer_init(ONVIF_DEVICE * p_dev)
{
	p_dev->events.timer_id = timeSetEvent(p_dev->events.init_term_time * 1000,0,onvif_event_timer_win,(DWORD_PTR)p_dev,TIME_PERIODIC);
}

void onvif_event_timer_deinit(ONVIF_DEVICE * p_dev)
{
    if (p_dev->events.timer_id != 0)
    {
    	timeKillEvent(p_dev->events.timer_id);
    	p_dev->events.timer_id = 0;
	}
}

#elif __LINUX_OS__

void * onvif_event_timer_task(void * argv)
{
	struct timeval tv;	
	ONVIF_DEVICE * p_dev = (ONVIF_DEVICE *) argv;
	
	while (p_dev->events.event_timer_run)
	{		
		tv.tv_sec = p_dev->events.init_term_time;
		tv.tv_usec = 0;
		
		select(1,NULL,NULL,NULL,&tv);
		
		onvif_event_timer(p_dev);
	}

	p_dev->events.timer_id = 0;
}

void onvif_event_timer_init(ONVIF_DEVICE * p_dev)
{
	p_dev->events.event_timer_run = TRUE;

	pthread_t tid = sys_os_create_thread((void *)onvif_event_timer_task, p_dev);
	if (tid == 0)
	{
		log_print("onvif_timer_init::pthread_create pp_timer_task\r\n");
		return;
	}

    p_dev->events.timer_id = (unsigned int)tid;
}

void onvif_event_timer_deinit(ONVIF_DEVICE * p_dev)
{
	p_dev->events.event_timer_run = FALSE;
	
	while (p_dev->events.timer_id != 0)
	{
		usleep(1000);
	}
}

#endif



