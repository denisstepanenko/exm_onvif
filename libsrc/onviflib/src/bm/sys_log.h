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

#ifndef	__H_SYS_LOG_H__
#define	__H_SYS_LOG_H__


#ifdef __cplusplus
extern "C" {
#endif

int 	log_init(const char * log_fname);
int 	log_time_init(const char * fname_prev);
void 	log_close();
int 	log_print(const char * fmt,...);

int 	log_lock_start(const char * fmt,...);
int 	log_lock_print(const char * fmt,...);
int 	log_lock_end(const char * fmt,...);

#ifdef __cplusplus
}
#endif

#endif	//	__H_SYS_LOG_H__



