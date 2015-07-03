#ifndef CGI_TEST_H
#define CGI_TEST_H
#define RTIMAGE_PARA_FILE "/conf/ip1004.ini"
#define CGI_LOCK_FILE		"/lock/ipserver/cgi"
#define VSMAIN_LOCK_FILE	"/lock/ipserver/ipmain"
#define RTIAMGE_LOCK_FILE	"/lock/ipserver/rtimage"
#define ENCBOX_LOCK_FILE    "/lock/ipserver/encbox" 
#define PROGRAME_PATH       "/ip1004/para_conv -s"

#define MAX_STR_LENTH 		100
/*
#define FAST_CONF    		0
#define DEV_INFO     		1
#define VIDEO_AUDIO_PARA   	2
#define ALARM_PARA         	3
#define VIDEO_ALARM_PARA	4
#define PLAYBACK_CONF       5
*/
enum PAGE_ENUM
{
	FAST_CONF=0,
	DEV_INFO,
	VIDEO_AUDIO_PARA,
	ALARM_PARA,
	VIDEO_ALARM_PARA,
	PLAYBACK_CONF
};
#define PARTITION_FORMAT 0
enum {
	PARTITION1 = 1,
	PARTITION2 = 2,
	PARTITION3 = 3,
	PARTITION4 = 4
};
#define PARTITION_GET 5

#endif
