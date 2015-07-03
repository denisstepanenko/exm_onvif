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

#ifndef	__SYS_INC_H__
#define	__SYS_INC_H__

#ifdef WIN32
#define __WIN32_OS__	1
#define __VXWORKS_OS__	0
#define __LINUX_OS__	0
#else
#define __WIN32_OS__	0
#define __VXWORKS_OS__	0
#define __LINUX_OS__	1
#endif

/***************************************************************************************/
typedef int 			int32;
typedef unsigned int 	uint32;
typedef unsigned short 	uint16;
typedef unsigned char 	uint8;

typedef unsigned int	__u32;
typedef unsigned short	__u16;
typedef int				__s32;
typedef unsigned char 	__u8;


/***************************************************************************************/
#if	__WIN32_OS__

#include "stdafx.h"

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <process.h>    /* _beginthread, _endthread */
#include <iphlpapi.h>
#include <assert.h>

#define sleep(x) 		Sleep(x * 1000)
#define usleep(x) 		Sleep(x/1000)

#define strcasecmp 		stricmp
#define snprintf 		_snprintf
#define strncasecmp 	_memicmp

#define pthread_t 		DWORD

#elif __LINUX_OS__

#include <sys/types.h>
#include <sys/ipc.h>

#ifndef ANDROID
#include <sys/sem.h>
#endif

#include <semaphore.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>
#include <netdb.h>

#include <net/if.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>

#include <dlfcn.h>
#include <dirent.h>

#ifndef BOOL_DEF
typedef int				BOOL;
#define BOOL_DEF
#endif

typedef unsigned int	UINT;
typedef int				SOCKET;
typedef uint8 *			LPBYTE;

#define TRUE			1
#define	FALSE			0

#define	closesocket 	close

#ifndef ANDROID
#define HANDLE			sem_t*
#endif

#endif

/*************************************************************************/
#include "sys_log.h"
#include "ppstack.h"
#include "word_analyse.h"
#include "sys_buf.h"
#include "util.h"


/*************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void  * sys_os_create_mutex();
void  * sys_os_create_sig();

void 	sys_os_destroy_sig_mutx(void * ptr);

int 	sys_os_mutex_enter(void * p_sem);
void 	sys_os_mutex_leave(void * p_sem);

int 	sys_os_sig_wait(void * p_sig);
int 	sys_os_sig_wait_timeout(void * p_sig, unsigned int ms);
void 	sys_os_sig_sign(void * p_sig);

pthread_t sys_os_create_thread(void * thread_func, void * argv);

uint32 	sys_os_get_ms();
char  * sys_os_get_socket_error();

/*************************************************************************/
void  * xmalloc(size_t size, const char * pFileName, int line);
void 	xfree(void * ptr, const char * pFileName, int line);

#define XMALLOC(x) 	xmalloc(x, __FILE__, __LINE__)
#define XFREE(x) 	xfree(x, __FILE__, __LINE__)

int 	mem_log_print(char * fmt,...);
void 	mem_log_quit();
/*************************************************************************/

#ifdef __cplusplus
}
#endif

#endif	//	__SYS_INC_H__



