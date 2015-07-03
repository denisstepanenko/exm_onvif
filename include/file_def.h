#ifndef FILE_DEF_H
#define FILE_DEF_H
#include <typedefine.h>
#include <stdio.h>
#include <string.h>
#include <gtlog.h>
#include "mod_socket.h"

//电路板型号定义

#define INTIME_RDK5				0
#define GT1000_1_0				1
#define BOARD					GT1000_1_0

#define		ALARMIN_MASK			0xffff	// from 0xff lc //from 0x3f,wsy		//报警输入淹码
#define		TOTAL_ALARMOUT_PORT  		4			//最大报警输出通道数
#define		ALARMOUT_MASK			0xff 		//fixme,输出端子改变时

#define		MAX_HQCHANNEL			5			//最大高清晰录像通道数.这个数只作分配空间用
#define		MAX_VIDEO_IN			10			//最大视频输入数,这个数只做分配空间用,需要设备的具体值时调用get_video_num()
#define		TOTAL_TRIG_IN			6			//触发输入数
#define 	MAX_TRIG_IN				20 		////最大触发输入数目,这个数只做分配空间用,需要设备的具体值时调用get_trigin_num()

#define 		MAX_TRIG_EVENT		10			//单路触发最大触发事件个数

#define 		MAX_FILE_LEN		30*1024			//配置文件最大长度 远程设定参数时用到

//视频丢失，遮挡，移动的取消时间定义,wsy
#define VLOSS_CANCEL_TIME	8	//视频丢失的取消时间
#define VMOTION_CANCEL_TIME	8	//视频移动的取消时间
#if 0
#ifndef _WIN32
#include <syslog.h>

//打开日志记录功能
//name表示日志信息中的名字
#define gtopenlog(name) openlog(name,LOG_CONS|LOG_NDELAY|LOG_PID,LOG_LOCAL0 );//LOG_USER);

//#define gtlog  syslog		//系统日志信息记录
#define gtlog syslog
//一般性信息
#define gtloginfo(args...) syslog(LOG_INFO,##args)	//记录一般信息
//严重的错误信息
#define gtlogfault(args...) syslog(LOG_CRIT,##args)	//
//错误信息
#define gtlogerr(args...) syslog(LOG_ERR,##args)	//
//警告信息
#define gtlogwarn(args...) syslog(LOG_WARNING,##args)

#define gtlogdebug(args...) syslog(LOG_DEBUG,##args)
#endif //_WIN32
#endif


/*
	vsmain define
*/
#define		DEV_CERT_FILE		"/conf/cert/dev-peer.crt"			//设备证书文件,通讯时不使用
#define		CERT_FILE			"/conf/cert/dev-gateway.crt"		//证书文件
#define		CERT_BAK_FILE		"/conf/cert/dev-gateway-bak.crt"	//证书文件备份

#define		KEY_FILE			"/conf/cert/dev-peer.key"			//私钥文件
#define		KEY_BAK_FILE		"/conf/cert/dev-peer-bak.key"		//私钥文件备份

#define    	TMP_AVI_LENGTH_FILE     	"/tmp/avi/avi_time.txt"

#define		IPMAIN_TMP_DIR			"/tmp"							//ipmain用于存放临时文件的地方
#ifndef FOR_PC_MUTI_TEST
#define	 	DEVINFO_PARA_FILE		"/conf/devinfo.ini"					//存放设备固定信息的文件，这些信息一般不会修改
#define	 	IPMAIN_PARA_FILE		"/conf/ip1004.ini"	
#define	 	IPMAIN_PARA_GATE_BAK		"/conf/ip1004_gate.ini" 			//用于指示变化的配置文件备份
#define     	MOTION_ALARM_PARA_FILE 		"/conf/ip1004.ini"
#define		MOTION_ALARM_GATE_BAK		"/conf/ip1004_gate.ini"				//用于指示变化的配置文件备份
#define	 	DISK_INI_FILE           "/conf/diskinfo.ini"

#define     	CONFIG_FILE			"/conf/config"
#define  	IPMAIN_LOCK_FILE		"/lock/ipserver/ipmain"
#define         VMMAIN_LOCK_FILE		"/lock/ipserver/vmmain"
#define		QUAD_DEV			"/dev/quadev"
#define     	SIMCOM_DEV             		"/dev/simcom" 
#define     	LEDS_DEV			"/dev/leddrv"

#define		WATCH_BD_LOCK_FILE		"/lock/ipserver/watch_board"	//报警51的版本号
#else
extern char	DEVINFO_PARA_FILE[];
extern char	VSMAIN_PARA_FILE[];
extern char VSMAIN_PARA_GATE_BAK[];
extern char MOTION_ALARM_PARA_FILE[];
extern char	MOTION_ALARM_GATE_BAK[];
extern char 	CONFIG_FILE[];
extern char	IPMAIN_LOCK_FILE[];
//#define		QUAD_DEV				"/dev/quadev"
//#define     	SIMCOM_DEV              	"/dev/simcom" 
//#define     	LEDS_DEV				"/dev/leds"
//#define		LOCAL6410_DRV			"/gt1000/drivers/ime6410_d1.o"
//#define		LOCAL6410_NAME		"ime6410_d1"
extern char WATCH_BD_LOCK_FILE[];

#endif

#define		MAIN_PARA_FILE_DEV0		"/conf/virdev/0/gt1000.ini"
#define		MAIN_PARA_FILE_DEV1		"/conf/virdev/1/gt1000.ini"
#define		MAIN_PARA_DEV0_GATE_BAK	"/conf/virdev/0/gt1000_bak.ini"
#define		MAIN_PARA_DEV1_GATE_BAK	"/conf/virdev/1/gt1000_bak.ini"
#define		ALARM_PARA_FILE		MOTION_ALARM_PARA_FILE
#define		LOCAL6410_DRV		"/gt1000/drivers/ime6410_d1.o"
#define		LOCAL6410_NAME		"ime6410_d1"
/*
	rtimage define
*/


#if EMBEDED
	#define  RT_6410_DEV			"/dev/IME6410"
#else
	#define	RT_6410_DEV			"/vserver/raw6410_cif.dat"
#endif

#define		NET6410_DRV			"/gt1000/drivers/ime6410_pcm.o"
#define		NET6410_NAME			"ime6410_pcm"
#define  		RTIMAGE_EXEC			"/gt1000/tcprtimg"

#ifndef FOR_PC_MUTI_TEST
#define	 	RTIMAGE_PARA_FILE		"/conf/ip1004.ini"//"/conf/rtimage.ini"
#define  		RT_LOCK_FILE			"/lock/ipserver/rtimage"
#else
extern char	RT_LOCK_FILE[];
extern char 	RTIMAGE_PARA_FILE[];
#endif

#ifndef FOR_PC_MUTI_TEST
#define	 	ENCBOX_PARA_FILE		"/conf/ip1004.ini"//"/conf/rtimage.ini"
#define  		ENC_LOCK_FILE			"/lock/ipserver/encbox"
#else
extern char	RT_LOCK_FILE[];
extern char 	RTIMAGE_PARA_FILE[];
#endif
/*
	hdsave define
*/

#define 	PARTITION_NODE1		"/hqdata/hda1"

#define     ALARM_SNAPSHOT_PATH    "/picindex/alarmpic.txt"  //报警抓图存放路径

#define		IMG_FILE_EXT			".AVI"	  	 //文件扩展名
#define		LOCK_FILE_FLAG			'@'      		 //上锁标记
#define 		REMOTE_TRIG_FLAG		'#'		 	 //远程触发标记
#define     	RECORDING_FILE_EXT      ".ING"    		//正在录的
#define     	OLD_FILE_EXT            	"_OLD.AVI"	//上次被打断的
#define 		FIX_FILE_EXT			"_FIX.AVI" //正在修理的avi文件
#if EMBEDED
	#define		HD_MINVAL				250//wsy change from 50 //shixintest 50//50*1024		//停止创建录像的空间
	#define		HD_RMVAL				280//wsy change from 80  100*1024	//开始删除文件的空间,报警空间
       #define  HD_RM_STOP             450   //wsy change from  		150  //删除文件的停止点
	#define		CF_MINVAL				50
	#define		CF_RMVAL				80
	#define		CF_RM_STOP				150		//values for CF card
#else
	#define		HD_MINVAL				10240//50*1024		//报警空间
	#define		HD_RMVAL				24000//100*1024	//开始删除文件的空间
	#define     	RM_STOP                		23000
#endif
#define 		MAX_HQFILE_LEN		180	//最大的录像切分长度	

struct dir_info_struct   //目录结构
{
	int year;                  
	int month;
	int date;
	int hour;
	int min;
	int sec;
};


#ifndef FOR_PC_MUTI_TEST
#define     HDMOD_PARA_FILE         "/conf/ip1004.ini"//"/conf/rtimage.ini"
#define     HDMOD_LOCK_FILE         "/lock/ipserver/hdmodule"
#define     PLAYBACK_LOCK_FILE    "/lock/ipserver/playback"
#define     HDSAVE_PATH                "/hqdata"   //高清晰存储文件存放路径
#else
extern char HDMOD_PARA_FILE[];
extern char HDMOD_LOCK_FILE[];
extern char HDSAVE_PATH[];			//高清晰存储文件存放路径

#endif
#define  	HDMOD_EXEC			"/ip1004/hdmodule"
#if EMBEDED==1
#define		HDMOUNT_PATH			HDSAVE_PATH			//文件系统挂接路径
#define		HQDEV0		"/dev/sda1"
#define     HQDEV1		"/dev/sda2"   
#define     HQDEV2		"/dev/sda3"
#define     HQDEV3		"/dev/sda4"

#else
#define		HDMOUNT_PATH			HDSAVE_PATH			//文件系统挂接路径
#define	HQDEV0		"/vserver/noaudio.dat"
#define	HQDEV1		"/vserver/noaudio.dat"
#define	HQDEV2		"/vserver/noaudio.dat"
#define	HQDEV3		"/vserver/noaudio.dat"

#endif


/*
	diskman define
*/

#define  		DISKMAN_EXEC			"/ip1004/diskman"
#ifndef FOR_PC_MUTI_TEST
#define  		DISKMAN_LOCK_FILE		"/lock/ipserver/diskman"
#else
extern char DISKMAN_LOCK_FILE[];
#endif
#define RESULT_TXT        "/var/tmp/result.txt"  //lost+found目录清理时的结果

#define HD_DEVICE_NODE		"/dev/hda"		//第一个硬盘设备节点
#define HD_PART_NODE		"/dev/hda1"		//硬盘1分区节点1

#define CFERR_PERCENT		80		//能正常工作的硬盘容量百分比如果低于此值，则报硬盘故障		

#define FIXDISK_LOG_FILE   "/var/tmp/fixdisk.txt"	 //修磁盘的日志
#define FIXDISK_LOG_FILE_0 "/var/tmp/fixdisk.txt.0"

#define REBOOT_FILE    "/var/tmp/rbt" //如无该文件，说明刚硬重起
/*
	rtaudio define
*/
#define		RTAUDIO_EXEC			"/vserver/rtaudio"			
#define		RTAUDIO_LOCK_FILE		"/lock/ipserver/rtaudio"


//
/*
	alarm_mod define
*/
#define 		ALARM_LOCK_FILE		"/lock/ipserver/alarm_mod"



#define 		TCPSNDPLAY_LOCK_FILE	"/lock/ipserver/tcpsndplay"

#define 		AUDIO_CDC_LOCK_FILE	"/lock/ipserver/audiocdc"



//
/*
     hdplayback  define
*/
#define          HDPLAYBACK_LOCK_FILE  "/lock/ipserver/playback"


#define  showbug()		printf("maybe a bug at %s:%d!!!!!!!!!!!!!\n",__FILE__,__LINE__)
#define	logbug()			gtlogerr("maybe a bug at %s:%d\n",__FILE__,__LINE__)
#endif





