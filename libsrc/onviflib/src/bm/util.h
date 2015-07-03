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

#ifndef	__H_UTIL_H__
#define	__H_UTIL_H__


/*************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************/
#if __LINUX_OS__
#define stricmp(ds,ss) 		strcasecmp(ds,ss)
#define strnicmp(ds,ss,len) strnicmp(ds,ss,len)
#endif

/*************************************************************************/
int 			get_if_nums();
unsigned int 	get_if_ip(int index);
unsigned int 	get_route_if_ip(unsigned int dst_ip);
unsigned int 	get_default_if_ip();
int 			get_default_if_mac(unsigned char * mac);
unsigned int 	get_address_by_name(char *host_name);
const char    * get_default_gateway();
const char    * get_dns_server();
const char    * get_mask_by_prefix_len(int len);
int 			get_prefix_len_by_mask(const char *mask);


/*************************************************************************/
char          * lowercase(char *str);
char          * uppercase(char *str);
int 			unicode(char **dst, char *src);

char          * printmem(char *src, size_t len, int bitwidth);
char 		  * scanmem(char *src, int bitwidth);

/*************************************************************************/
int 			base64_encode(unsigned char *source, unsigned int sourcelen, char *target, unsigned int targetlen);
void 			str_b64decode(char* str);
int 			base64_decode(const char *source, unsigned char *target, unsigned int targetlen);


/*************************************************************************/
time_t 			get_time_by_string(char * p_time_str);
int 			tcp_connect_timeout(unsigned int rip, int port, int timeout);

#ifdef __cplusplus
}
#endif

#endif	//	__H_UTIL_H__



