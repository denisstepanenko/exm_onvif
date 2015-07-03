
/*
	ipmain.h 	created by lc
	
*/
//启动日志守护进程syslogd -m 0 -O /conf/log/gtlog.txt -s 512 -b 10 -S
#ifndef IPMAIN_H
#define IPMAIN_H
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/time.h>
#include <string.h>
#include <syslog.h>
#include <typedefine.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mod_socket.h"

//#include "hi3515/wtdg/watchdog.h"

//#include "exdrv_3520Ademo/hi_wtdg/watchdog.h"
//lc change from 30 to 150
#define WTDG_TIMEOUT  150

//ifconfig eth0 hw ether 00:0F:EA:05:B2:77
//note 修改/proc/sys/kernel/msgmni  文件可以增加系统可以创建的最大消息队列数
#define EMBEDED	 1	//嵌入式系统:1 PC系统 0

#define  VERSION		"0.43"



/*
 *0.43  15-3-24   recv iptalk alarm msg as 8-11 chn alarm
 *0.42  15-1-30   send alarm chn when trig audio output
 *0.41  14-10-20  when gate down,set relay output due to config
 *0.40  14-9-25   when alarm happen, net led flash or/and buz
 *0.39  14-9-19   fix send ini to gate bug lockfile
 *0.38  14-9-12   disable/enable wdt with serial mode
 *0.37  14-8-18   fix set_time bug
 *0.36  14-8-4    fix multichannel audio down and usr_lock_time cmd bug(also mutlichannel) fix notice.wav
 *0.35  14-7-29   fix single channel cancle alarm playback bug
 *0.34  14-7-28   fix multichannel cancel alarm clear trig hdmod bug and update failed reboot
 *0.33  14-7-21   add power monitor
 *0.32  14-7-9    对于3520D主板，音频选择与看门狗机制改变
 *0.31  14-3-25   原来的报警与ack日志机制存在问题，解决该bug
 *0.30  14-2-25   对rtimage发送playback命令将取决于netencoder:playback_enable选项
 *0.29  14-2-21   当升级，收到完整文件后，开始停止心跳，这样当升级出现问题，至少设备肯定可以重启
 *0.28  14-2-11   支持录像多通道的相关功能，但其中部分信令需平台提供相应修改才可支持
 *0.27  14-2-7    修正多通道中一个bug，去掉0.26版本做出的修改
 *0.26  14-1-22   加入从devinfo获取视频通道名称，并在设备注册时添加该部分字段
 *0.25  14-1-10   加入对多通道支持，通过配置文件中multichannel，目前只支持报警时，对应一路视频，同时加入对hdplayback的信令转发
 *0.24  13-12-27  加入对encbox检查，如果大量出现coaxial err则重启设备
 *0.23  13-12-24  对时命令之前一直默认返回操作正确，现在将返回实际值
 *0.22  13-12-20  对报警后处理扩大加锁范围，同时将报警复位的锁提前，保证不会在报警后续动作执行中插入报警复位
 *0.21  13-11-07  在killencbox后，先sleep5秒保证ipcrm成功后，进行tar
 *0.20  13-11-05  修改升级中，tar命令的优先级为较低，避免出现tar解压时，设备卡住情况
 *0.19  13-11-05  支持防区一对多，并且加入对防区单独核警支持
 *0.18  13-10-25  接收encbox，关于视频源故障的信令，并打日志
 *0.17  13-9-22   改动升级方式，先删除ip1004下内容，再判断容量，同时对重启命令做备份处理，保证可以重启成功
 *0.16  13-9-17   去掉multichannel功能，同时解决gpio对常闭处理错误
 *0.15  13-9-13   增加新功能，对串口操作，加入文件锁，禁止其他进程使用串口，并重复占用串口
 *0.14  13-9-9    增加新功能，支持multi-channel
 *0.13  13-9-3    解决移动侦测只报1路的bug
 *0.12  13-8-31   在调用aplay之前，先杀掉一次aplay
 *0.11  13-8-29   加入对内部串口的侦听和解析机制
 *0.10  13-8-15   修正对于err灯不起作用的问题
 *0.09  13-8-2    使用三型机openssl源码，no-asm选项编译的libssl libcrypto库，做解密测试，同时修改对格式化时间的估算功能
 *0.08  13-7-24   修改升级时判断空间的bug
 *0.07  13-7-04   核警期间双向语音与核警通道保持一致
 *0.06  13-7-04   增加vda相关功能
 *0.05  13-6-25   修改报警记录中，时间为当前时间而非报警时间的bug，修正aplay相关功能
 *0.04  13-6-21   加入批量升级功能
 *0.03  13-6-18   测试第一轮结束后统一修改，解决bug
 *0.02  13-6-14   交给测试部门测试，修正切4分屏时。逻辑错误问题
 *0.01  12-11-15  建立ipmain项目，实现设备配置，初始化，网络信令部分功能
 */


//设备制造商标识定义
#define	VENDOR_GTALARM		0x41435447		//国通创安

#define 	DEVICE_VENDOR 			VENDOR_GTALARM


//网关和设备通讯协议的内部版本号
#define	INTERNAL_PROTOCAL		2

//指示灯GPG_12

/*
	如果使用则定义
*/


#define		QUAD_CTRL				//使用画面分割控制模块
#define     USE_IO
#define     DUMP_ALARM
#define     USE_SSL
//#define     USE_VDA
#define     USE_WTDG
#define     USE_LED
#define     HQMODULE_USE
#define     HDPLAYBACK_USE
#define     AUDIO_OUTPUT
//#define     RECV_MCU

//#define	DEAMON_MODE			//以守护进程方式运行程序
//#define 		MULTICAST_USE		//使用组播方式查找设备
//#define  		USE_LEDS			//使用面板LED驱动
//#define ARCH_3520A
#define    ARCH_3520D

#define    GATE_PERIOD_TIME_DEF			0		//网关有效期默认值，如果是GTIP1004发起的连接，如果空闲超过这个时间则断开连接
#define    REMOTE_GATE_CMD_PORT		 	8090	    //远程网关开放的命令端口
#define    DEV_MAIN_CMD_PORT	 			8095    //0x4791
#define    DEV_MAIN_IMAGE_PORT   			8096
#define    DEV_MAIN_AUDIO_PORT   			8097
#define    DEV_DEFAULT_WEB_PORT			8094
#define	   DEV_DEFAULT_FTP_PORT			  21
#define	   DEV_DEFAULT_TELNET_PORT		  23

#define    COM1_TCP_PORT		8098//串口1建立tcp连接的端口
#define    COM2_TCP_PORT		8099//串口2建立tcp连接的端口

#define TRANS_COM_NOT_CHECK_REMOTEIP

#define    WATCH_SERIAL_BAUD	600

#define    GTIP1004_TOTAL_COM		2	//GTIP1004上，可供控制的串口总数
#ifdef ARCH_3520A
	#define    GTIP1004_COM1			"/dev/ttyAMA3"
	#define    GTIP1004_COM2			"/dev/ttyAMA1"
	//lc do 2013-5-30
	#define    GTIP1004_COM_INTERNAL  "/dev/ttyAMA0"
#endif
#ifdef ARCH_3520D
	#define    GTIP1004_COM1			"/dev/ttyAMA1"
	#define    GTIP1004_COM2			"/dev/ttyAMA2"
	//lc do 2013-5-30
	#define    GTIP1004_COM0           "/dev/ttyAMA0"
#endif


#define    GTIP_IO_VALID_DELAY    0   //正常->触发延迟
#define    GTIP_IO_INVALID_DELAY  3   //触发->正常延迟



//#define ALARM_SNAP_INTERVAL 500 //报警抓图缺省间隔
//#define ALARM_SNAP_NUM		5   //报警抓图缺省张数



//#define		RMT_PERIOD				10	//远程连接(网关)空闲指定时间后自动断开连接


//test function
#define		DISPLAY_REC_NETCMD	//显示从网络收到的命令信息
#define		DISPLAY_THREAD_INFO	//现程启动后打印线程信息
#define		SHOW_WORK_INFO		//显示一些工作流程信息
#define		SHOW_GATE_CMD_REC	//显示接收到的网关命令
//#define		NOT_INCLUDE_WATCH_BD	//不加入51监控板的内容
//#define		SHOW_ALL_WATCHBD_INFO	//显示所有的51监控板通讯信息
//#define		TEST_FROM_TERM			//从终端模拟输入信息
//#define		TEST_REGIST				//连续测试注册
//#define 		NOT_CHECK_REMOTEIP			//不检查远程连接ip地址是否合法 
#define		REMOTE_NOT_TIMEOUT		//不进行超时检测否则如果远程计算机进行连接后到达指定时间仍没有发送命令则断开连接
//#define     DONT_CONNECT_GATE   		//不连接网关
//#define		TEST_SEND_STATE			//测试不断发送状态给网关

#if EMBEDED==0
	#define FOR_PC_MUTI_TEST		//支持在同一台pc机上启动多个程序
#endif


//#define TRANS_COM_NOT_CHECK_REMOTEIP		//zw-add -2010-12-03-18:31 //zw-del 2010-12-15



#ifndef HQMODULE_USE
	#define HDTYPE	0	//存储介质类型  0:没有 1:CF卡 2:硬盘
#else
	#define HDTYPE	1	//存储介质类型  0:没有 1:CF卡 2:硬盘
#endif



#define printdbg	printf
#if 0
//电路板型号定义

#define INTIME_RDK5		0
#define GT1000_1_0		1
#define BOARD			INTIME_RDK5
	
#endif







#include <file_def.h>
#include <mod_com.h>
#include <gtsocket.h>


#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long

#ifndef OUT
#define OUT
#define INOUT
#define IN
#endif



#if BOARD==INTIME_RDK5

#elif BOARD==GT1000_1_0

#else
	#error "Rtimage software can't support this board"
#endif


#include "netinfo.h"

//function define

#if 0
static __inline__ int show_time(void)
{
	struct timeval tv;
	struct tm *ptime;
	char pbuf[60];
	time_t ctime;
	if(gettimeofday(&tv,NULL)<0)
	{
		return -1;
	}
	ctime=tv.tv_sec;
	ptime=localtime(&ctime);
	if(ptime!=NULL)
	{
		sprintf(pbuf,"%d-%d-%d %d:%d:%d.%03d\n",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min,ptime->tm_sec,(int)tv.tv_usec/1000);	
		printf("%s",pbuf);
	}
	return 0;
	
}
#endif







DWORD atohex(char *buffer);

int reinstall_localimedrv(int ch);

//获取一个默认属性结构
//用完了应该释放
int get_gtthread_attr(pthread_attr_t *attr);
int posix_memalign(void **memptr, size_t alignment, size_t size); 

int init_wtdg_dev(int timeout);
int feed_watch_dog();

#endif



















//串口1,2对应于协议中的串口0,1
