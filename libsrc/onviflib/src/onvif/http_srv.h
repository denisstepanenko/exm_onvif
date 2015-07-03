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

#ifndef HTTP_SRV_H
#define HTTP_SRV_H

#include "http.h"


#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************/
int 	    http_srv_net_init(HTTPSRV * p_srv);
int 	    http_srv_init(HTTPSRV * p_srv, unsigned int saddr, unsigned short sport, int cln_num);
void 	    http_srv_deinit(HTTPSRV * p_srv);

/***************************************************************************************/
unsigned long http_cln_index(HTTPSRV * p_srv, HTTPCLN * p_cln);
HTTPCLN   * http_get_cln_by_index(HTTPSRV * p_srv, unsigned long index);
HTTPCLN   * http_get_idle_cln(HTTPSRV * p_srv);
void 	    http_free_used_cln(HTTPSRV * p_srv, HTTPCLN * p_cln);
HTTPCLN   * rpc_find_cln(HTTPSRV * p_srv, unsigned int rip, unsigned short rport);

#ifdef __cplusplus
}
#endif

#endif


