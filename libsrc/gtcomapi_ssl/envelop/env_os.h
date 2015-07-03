/*************************************************************
File:   env_os.h
Author: Zhangtao 
Date: 	2005-2-23
Description:平台相关的定义
*************************************************************/

#ifndef _GT_ALARM_TAO_ENV_OS_20050225_
#define _GT_ALARM_TAO_ENV_OS_20050225_

#ifdef WIN32
#include <windows.h>
#include <string.h>
#else
#include <unistd.h>
#include <strings.h>
#include <pthread.h>
#endif

#ifdef WIN32 
#define SLEEP(x) Sleep(x * 1000)
#else
#define SLEEP(x) sleep(x)
#endif


#endif//_GT_ALARM_TAO_ENV_OS_20050225_

