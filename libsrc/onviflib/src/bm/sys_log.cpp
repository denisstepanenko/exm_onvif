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
#include "sys_log.h"

/***************************************************************************************/
static FILE * g_pLogFile = NULL;
static void * g_pLogMutex = NULL;


/***************************************************************************************/
int log_init(const char * log_fname)
{
	log_close();
	
	g_pLogFile = fopen(log_fname,"w+");
	if (g_pLogFile == NULL)
	{
		printf("log init fopen[%s] failed[%s]\r\n", log_fname, strerror(errno));
		return -1;
	}

	g_pLogMutex = sys_os_create_mutex();
	if (g_pLogMutex == NULL)
	{
		printf("log init sem_init failed[%s]\r\n", strerror(errno));
		return -1;
	}
	
	return 0;
}

int log_time_init(const char * fname_prev)
{
	char fpath[256];
	time_t time_now = time(NULL);
	struct tm * st = localtime(&(time_now));
	
	sprintf(fpath, "%s-%04d%02d%02d_%02d%02d%02d.txt", fname_prev, 
		st->tm_year+1900, st->tm_mon+1, st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec);
	
	return log_init(fpath);
}

void log_close()
{
	if (g_pLogFile)
	{
		fclose(g_pLogFile);
		g_pLogFile = NULL;
	}

	if (g_pLogMutex)
	{
		sys_os_destroy_sig_mutx(g_pLogMutex);
		g_pLogMutex = NULL;
	}
}

int _log_print(const char *fmt, va_list argptr)
{
	int slen = 0;

	if (g_pLogFile == NULL || g_pLogMutex == NULL)
		return 0;

	sys_os_mutex_enter(g_pLogMutex);

	slen = vfprintf(g_pLogFile,fmt,argptr);
	fflush(g_pLogFile);

	sys_os_mutex_leave(g_pLogMutex);

	return slen;
}

int log_print(const char * fmt,...)
{
	int slen;	
	va_list argptr;

	va_start(argptr,fmt);

	slen = _log_print(fmt,argptr);

	va_end(argptr);

	return slen;
}

static int _log_lock_print(const char *fmt, va_list argptr)
{
	int slen;

	if (g_pLogFile == NULL || g_pLogMutex == NULL)
		return 0;

	slen = vfprintf(g_pLogFile, fmt, argptr);
	fflush(g_pLogFile);
	return slen;
}

int log_lock_start(const char * fmt,...)
{
	int slen = 0;

	va_list argptr;
	va_start(argptr,fmt);	

	if (g_pLogFile == NULL || g_pLogMutex == NULL)
		return 0;

	sys_os_mutex_enter(g_pLogMutex);
	
	slen = _log_lock_print(fmt,argptr);
	va_end(argptr);

	return slen;
}

int log_lock_print(const char * fmt,...)
{
	int slen;

	va_list argptr;
	va_start(argptr,fmt);

	slen = _log_lock_print(fmt, argptr);

	va_end(argptr);
	
	return slen;

}

int log_lock_end(const char * fmt,...)
{
	int slen;

	va_list argptr;
	va_start(argptr,fmt);

	slen = _log_lock_print(fmt, argptr);

	va_end(argptr);

	sys_os_mutex_leave(g_pLogMutex);

	return slen;
}




