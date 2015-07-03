#ifndef VS3_VIDEOENC_H
#define VS3_VIDEOENC_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "gtlog.h"


#define     VERSION     "0.01"
#define     PARA_FILE       "/conf/ip1004.ini"

#define     FRAME_BUFFER_SIZE      (200*1024)      //200k
#define     AUDIO_FRAME_SIZE       (4*1024)        //4k
#define     STREAM_POOL_SIZE_MASTER  (1*1024*1024) //1M
#define		STREAM_POOL_SIZE_SLAVE	(8*1024*1024)    //8M 
#define		AUDIO_POOL_SIZE_MASTER	(100*1024)    // 100k
#define		AUDIO_POOL_SIZE_SLAVE	(1*1024*1024)    //1M 
#define     IP_AUDIO_FRAME_SIZE      (4*1024)       //4k
 
 
                                                                          










#endif


