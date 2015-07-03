//GT1000系统内部通讯的命令字
#ifndef MOD_CMD_H
#define MOD_CMD_H
#include <typedefine.h>
#include "time.h"
#include "gate_cmd.h"
#include <stdio.h>



#ifndef DWORD
#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#endif
/******************************************
	各进程ID定义
******************************************/

#define		ALL_PROCESS		0           ///<广播地址
#define		MAIN_PROCESS_ID		1
//#define 	UPDATE_PROCESS_ID		2				//已经并入主进程
#define		RTIMAGE_PROCESS_ID	3		
#define		PLAYBACK_PROCESS_ID	    4				//回放模块ID
#define		HQSAVE_PROCESS_ID		5	
#define		PPPOE_WATCH_ID		6				//监视adsl拨号的进程
#define		DISKMAN_ID				7				//磁盘管理进程
#define		KEYBOARD_ID			8				//外部键盘处理
#define		ALARM_MOD_ID			9				//报警控制模块
#define		VIDEOENC_MOD_ID		10				//视频数据采集模块
#define          HW_DIAG_MOD_ID            11                        //硬件故障侦测模块
#define		UPNPD_MOD_ID				12				//upnp端口映射模块

#define 		HDBUF_PROCESS_ID		13				//硬盘缓冲区模块 zw-add 2011-11-23
#define		CGI_MAIN_ID					14             //cgimain处理模块
//


/************************************************************************************
	主进程发送给其它进程的命令
************************************************************************************/
//主进程查询其它进程的状态,这个命令什么参数也没有
#define	MAIN_QUERY_STATE			0x1001



/*主进程将报警触发状态发送给高清晰录像模块，高清晰录像模块
 *应当按照预先设置的参数，对之进行处理(录像)
 *格式:len(2)+SEND_TRIG_STATE(2)+state(4)
 *state的具体含义由主进程定义
*/
#define	SEND_TRIG_STATE			0x1002

/*
	主进程让其他进程刷新参数的命令
	这个命令没有参数
*/
#define	MAIN_REFRESH_PARA		0x1003

//zw-add -2011-08-05
//报警回放使用给tcprtimage
#define  MAIN_PLAYBACK_IMG_CMD			0x1004

//zw-add 2011-08-05
//发送给videoenc,准备停止更新共享缓冲池中的视频帧数据
//在报警发生后延时30秒，然后停止更新，直到录像回放完毕后接着更新缓冲池。
#define MAIN_PLAYBACK_ENC_CMD				0x1005

typedef struct
{
	int offset;		//帧偏移量
}hddbuf_offset_t;

//zw-add 2011-11-30
//主要给gtvs1000使用，从tcprtimg2发送获取当前读取硬盘缓冲区为位置发送给vsmain,在转发给hdrecd
#define MAIN_PLAYBACK_GETPOC_CMD				(0x1006)
#define MAIN_PLAYBACK_GETPOC_ANSWER_CMD	(0x1007) //需要跟着一个int型数

#define MAIN_REQUEST_APLAY (0x1008)

//zw-add 2011-08-17
//发送给hdmodule用的命令
//使用MAIN_PALYBACK_ENC_CMD
/**********************************************************************************
	其他进程发送给主进程的命令
***********************************************************************************/
//这个是tcprtimg发给vsmain的命令，需要vsmain再转发给videoenc，告诉它录像回放结束了
//可以继续更新共享内存了
#define RTIMG_PLAYBACK_STOP_CMD					0x2a01

//rtimage给ipmain发的命令，告知下行通道号
#define RTIMG_AUDIODOWN_CMD                       0x2a05




/*
	rtimage向videoenc请求i帧
*/
//IPDEVICE 相关信令
#define REQUIRE_IFRAME                                               0x2a02
#define PROBE_IP_DEVICE												 0x2a06
#define PROBE_IP_ACK											 0x2b06

#define SET_DEVICE_IP                                                0x2a11
#define SET_DEVICE_IP_ACK                                            0x2b11


#define SET_DEVICE_MASK                                              0x2a12
#define SET_DEVICE_MASK_ACK                                          0x2b12

#define SET_RATE                                                     0x2a13
#define SET_RATE_ACK												 0x2b13

#define SET_PRICSION                                                 0x2a14
#define SET_PRICSION_ACK                                             0x2b14

#define SET_CHANNEL                                                  0x2a15
#define SET_CHANNEL_ACK                                              0x2b15

#define SET_ENV                                                      0x2a16
#define SET_ENV_ACK													 0x2b16

#define GET_DEVICE_IP                                                0x3a11
#define GET_DEVICE_IP_ACK                                            0x3b11

#define GET_DEVICE_MASK                                              0x3a12
#define GET_DEVICE_MASK_ACK                                          0x3b12

#define GET_RATE                                                     0x3a13
#define GET_RATE_ACK												 0x3b13

#define GET_PRICSION                                                 0x3a14
#define GET_PRICSION_ACK                                             0x3b14

#define GET_CHANNEL                                                  0x3a15
#define GET_CHANNEL_ACK                                              0x3b15

#define GET_ENV                                                      0x3a16
#define GET_ENV_ACK													 0x3b16

#define REQUIRE_UP_AUDIO											 0x3a17	
#define REQUIRE_UP_AUDIO_ACK										 0x3b17

#define STOP_UP_AUDIO												 0x3a18	
#define STOP_UP_ADUIO_ACK											 0x3b18

#define SEND_DOWN_AUDIO												 0x3a19	
#define SEND_DOWN_AUDIO_ACK											 0x3b19

#define STOP_DOWN_AUDIO												 0x3a20	
#define STOP_DOWN_AUDIO_ACK											 0x3b20

#define QUERY_STATUS												 0x3a21
#define QUERY_STATUS_ACK											 0x3b21	


//ONVIFDEVICE 相关信令
#define PROBE_ONVIF_DEV                                              0x4001
#define PROBE_ONVIF_DEV_ACK                                          0x4101







/*
     键盘处理模块发送给主进程的状态信息
*/
#define KEYBOARD_STAT_RETURN		0x6001	//键盘状态返回
struct keyboard_state_struct{
	unsigned		mod_state		:1;	//模块状态0表示正常 1表示异常,此位由主模块设置
	unsigned		connect_fail		:1;	//连接外部键盘错误0表示正常 1表示错误
	unsigned		reserve			:30; //保留
};
#define	KEYBOARD_ALARM_SET			0x6002	//键盘模块发送的外触发防区布防信息
struct keyboard_alarm_set_struct{
	int mode;		//0 表示外部触发  1表示移动触发
	int channel;		// 外触发或移动触发通道号0,1,2...  -1表示所有外触发通道或移动触发通道
	int flag;			// 1表示布防 0表示撤防
};










/*下行声音处理模块向主进程返回自己的状态，标志位的意义由声音模块确定,0表示正常，1表示异常
 *当下行声音模块自己发现状态变化时发送此命令，或者当接收到主进程发来的MAIN_QUERY_STATE命令时发送此命令 
 *格式:len(2)+AUDIO_STATE_RETURN(2)+pid(pid_t)+state(4)
 *pid:下行音频处理模块的进程id，是pid_t类型的数据，长度为sizeof(pid_t)
 *state:下行音频处理模块的状态，如果全0表示一切正常，具体位的意义由声音模块确定
 */
#define AUDIO_STATE_RETURN	0x0201

/*实时音视频传送模块向主进程返回自己的状态,状态标志位的意义由实时图像模块确定,0表示正常，1表示异常
 *当实时图像模块自己发现状态变化时发送此命令，或者当接收到主进程发来的MAIN_QUERY_STATE命令时发送此命令 
 *格式:len(2)+RTSTREAM_STATE_RETURN(2)+pid(pid_t)+state(4)
 *pid:实时图像模块的进程id，是pid_t类型的数据，长度为sizeof(pid_t)
 *state:实时图像模块的状态，如果全0表示一切正常，具体位的意义由实时图像模块确定
 */
#define RTSTREAM_STATE_RETURN		0x0301
struct rtimage_state_struct{
	unsigned		mod_state		:1;	//模块状态0表示正常 1表示异常,此位由主模块设置
	unsigned		net_enc_err		:1;	//网络的压缩芯片故障
	unsigned 		net_enc_busy	:1;  //网络编码芯片忙标志
	unsigned 		test_d1_flag		:1; //测试d1通道标志
	unsigned		reserve			:28;
};


/*
	主进程通知实时图像进程切换视频通道的命令
	格式:len(2)+SET_SCREEN_DISPLAY(2)+display_type(2)+channel(2)
*/

typedef struct{
	WORD	channel;		//表示全屏时的视频通道号，取值0,1,2...
	BYTE    peeruser[20];
	BYTE    peeraddr[16];
}set_scr_display_struct;


#define SET_SCREEN_DISPLAY	0x1301

/**********************************************************************************
 *	高清晰录像进程相关命令
***********************************************************************************/

/*
	控制录像模块进行录像(只是控制录像模式而不管锁定文件等)
	格式len(2)+CTRL_HDRECORD(2)+channel(4)+mode(4)
	channel:需要控制的通道号0,1,2,3
	mode:	0: 表示停止录像线程
			1: 表示启动录像线程
			2: 表示重新启动录像线程
*/
struct hdrecord_ctrl_struct{
	DWORD channel;
	DWORD mode;
};
#define HDRECORD_CTRL	0x1401

/*
	触发一次高清晰录像(本地报警等事件触发)
	格式 len(2)+TRIG_RECORD_EVENT(2)+channel(4)+trig_flag(4)+reclen(4)
	channel:需要触发录像的压缩通道号
	trig_flag:触发标志(需要存入文件名)
	reclen:需要触发录像的时间
*/
struct trig_record_event_struct{
	DWORD	channel;
	DWORD	trig_flag;
	DWORD	reclen;
};
#define TRIG_RECORD_EVENT 	0x1402

/*
	远程计算机发来的触发录像命令
	格式 len(2)+REMOTE_TRIG_RECORD(2)+channel(4)+mode(4)+reclen(4)
	channel:需要触发录像的压缩通道号
	mode: 0表示停止录像
		   1表示触发录像
	reclen:需要录像的长度(对于停止命令无意义)	
	
*/
struct remote_trig_record_struct{
	DWORD	channel;//录像通道
	DWORD	mode;//1为录像，0为不录
	DWORD	reclen;
};
#define REMOTE_TRIG_RECORD	0x1403

/*
	解除对文件的锁定
	无参数
*/
#define UNLOCK_FILE		0x1404

/*
	清除触发标志(退出报警录像状态)
	DWORD	channel;//录像通道
*/
#define CLEAR_TRIG_FLAG	0x1405

/*
	按时间加解锁
	格式 len(2)+USR_LOCK_FILE_TIME(2)+lockfile(20)
	lockfile: struct usr_lock_file_time_struct
*/

#define LOCK_FILE_TIME	0x1406

/*报警后发给hdmodule模块进行抓图的命令
 *格式:len(2)+ALARM_SNAPSHOT(2)+takepic(20)
 *takepic:是takepic_struct结构的数据，包括时间戳，间隔，张数，通道号
*/
#define ALARM_SNAPSHOT     0x1407
	
/*发给hdmodule模块进行查询索引的命令
 *格式:QUERY_INDEX(2)+len(2)+channel(4)+query_index(20)
 *queryindex:是query_index_struct结构的数据(在gate_cmd.h中定义)，包括起始和结束时间
*/
#define QUERY_INDEX			0x1408

struct query_index_with_channel{
DWORD						channel;	//要查询的录像通道号，-1表示全部通道
struct 	query_index_struct	queryindex;
};

	
	
/*高清晰图像记录模块向主进程返回自己的状态，状态标志位的意义由高清晰存储模块确定
 * 当高清晰图像记录模块发现自己状态改变或者主进程发送了查询状态命令后发送此命令
 * 格式:len(2)+HDMODE_STATE_RETURN(2)+pid(pid_t)+state(4)
 * pid:高清晰图像记录进程的id，是pid_t类型的数据，长度为sizeof(pid_t)
 * state:高清晰图像模块的状态，如果全0表示一切正常，具体的意义由高清晰录像模块确定
*/
#define HDMOD_STATE_RETURN		0x0401
struct hdmod_state_struct{
	unsigned reserve1		:7;
	unsigned cf_err         :1; //磁盘故障
	/*unsigned video_enc1_err	:1;	//第0路高清晰录像通道故障
	unsigned video_enc2_err	:1;	//第1路高清晰录像通道故障
	unsigned video_enc3_err	:1;	//第2路高清晰录像通道故障
	unsigned video_enc4_err	:1;	//第3路高清晰录像通道故障
	unsigned reserve2		:20;
	*/
	unsigned reserve2		:24; //wsy,不再处理和报告编码器故障
};




/**********************************************************************************
 *	磁盘管理录像进程相关命令
***********************************************************************************/
#define DISKMAN_STATE_RETURN		0x0701
struct diskman_state_struct{
	unsigned cf_err			:1;  //磁盘故障
	unsigned disk_full			:1;	//磁盘满标志
	unsigned reserve			:30;
};



/*按时间段加解锁文件
 *参数格式:
 *lockfile:是usr_lock_file_time_struct结构的数据，
 *         包括时间戳，间隔，张数，通道号
*/
#define LOCK_FILE_BY_TIME	     0x1701



/**********************************************************************************
*	视频数据采集模块相关命令
***********************************************************************************/
#define VIDEOENC_STATE_RETURN		0x0901
struct videoenc_state_struct{
	unsigned video_enc0_err :1; //第0路视频编码器故障
	unsigned video_enc1_err	:1;	//第1路视频编码器故障
	unsigned video_enc2_err	:1;	//第2路视频编码器故障
	unsigned video_enc3_err	:1;	//第3路视频编码器故障
	unsigned video_enc4_err	:1;	//第4路视频编码器故障
	//unsigned video_enc5
	unsigned reserve			:3;		//保留
	unsigned video_motion0	:1;	//第0路移动侦测
	unsigned video_motion1	:1;
	unsigned video_motion2	:1;
	unsigned video_motion3	:1;
//	unsigned reserve1		:4;//11
	unsigned audio_enc0_err :1; //第0路视频编码器故障
	unsigned audio_enc1_err	:1;	//第1路视频编码器故障
	unsigned audio_enc2_err	:1;	//第2路视频编码器故障
	unsigned audio_enc3_err	:1;	//第3路视频编码器故障
	
	unsigned video_loss0		:1;	//第0路视频丢失
	unsigned video_loss1		:1;	//第1路视频丢失
	unsigned video_loss2		:1;	//第2路视频丢失
	unsigned video_loss3		:1;	//第3路视频丢失
	unsigned reserve2		:4; 
	unsigned video_blind0		:1;	//第0路视频遮挡
	unsigned video_blind1		:1;	//第1路视频遮挡
	unsigned video_blind2		:1;	//第2路视频遮挡
	unsigned video_blind3		:1;	//第3路视频遮挡
	unsigned reserve3		:4; 
	
};

//lc 视频VDA模块相关状态
#define VIDEOENC_VDA_STATE		0x0902
struct videoenc_vda_state_struct{
	unsigned video0_loss   :1;  //第0路通道丢失
	unsigned video1_loss   :1;  //第1路通道丢失
	unsigned video2_loss   :1;  //第2路通道丢失
	unsigned video3_loss   :1;  //第3路通道丢失
	unsigned video4_loss   :1;  //第4路通道丢失
	unsigned video5_loss   :1;  //第5路通道丢失
	unsigned video6_loss   :1;  //第6路通道丢失
	unsigned video7_loss   :1;  //第7路通道丢失

	unsigned video0_blind  :1;  //第1路通道遮挡
	unsigned video1_blind  :1;  //第2路通道遮挡
	unsigned video2_blind  :1;  //第3路通道遮挡
	unsigned video3_blind  :1;  //第4路通道遮挡
	unsigned video4_blind  :1;  //第5路通道遮挡
	unsigned video5_blind  :1;  //第6路通道遮挡
	unsigned video6_blind  :1;  //第7路通道遮挡
	unsigned video7_blind  :1;  //第8路通道遮挡

	unsigned video0_motion  :1;  //第1路通道移动
	unsigned video1_motion  :1;  //第2路通道motion
	unsigned video2_motion  :1;  //第3路通道motion
	unsigned video3_motion  :1;  //第4路通道motion
	unsigned video4_motion  :1;  //第5路通道motion
	unsigned video5_motion  :1;  //第6路通道motion
	unsigned video6_motion  :1;  //第7路通道motion
	unsigned video7_motion  :1;  //第8路通道motion
};

//lc 视频各路错误状态
#define VIDEOENC_COAXIAL_ERR		0x0903
struct enc_coaxial_state
{
	unsigned video0_err      :1;   //第1路视频源出错
	unsigned video1_err      :1;
	unsigned video2_err      :1;
	unsigned video3_err      :1;
	unsigned video4_err      :1;
	unsigned video5_err      :1;
	unsigned video6_err      :1;
	unsigned video7_err      :1;
};

//返回结果ACK的结构
struct result_return_struct 
{
	int fd;
	int command;
	int result;
};



/**********************************************************************************
 *	网络监控进程相关命令
***********************************************************************************/

/*
 *	PPPOE监控进程发送给主进程状态
 *    格式:len(2)+HQSAVE_STATE_RETURN(2)+pid(pid_t)+state(4)
 *    pid:PPPOE监控进程的id，是pid_t类型的数据，长度为sizeof(pid_t)
 *    state:PPPOE监控模块的状态，

*/
#define PPPOE_SUCCESS				0	//adsl正常连接
#define PPPOE_NO_MODEM			1	//找不到adsl modem
#define PPPOE_PASSWD_ERR			2	//adsl帐号密码错误
#define PPPOE_USR_TWICE			3	//帐号重复登入
#define PPPOE_USR_INVALID			4	//帐号无效
#define PPPOE_PAP_FAILED			5	//帐号无效

#define PPPOE_STATE_RETURN		0x0501


/**********************************************************************************
 *	硬件检测进程的相关命令
***********************************************************************************/

#define HW_DIAG_STATE_RETURN        0x0b01  //状态返回
typedef struct {
    unsigned    ide_err                 :1;                    ///磁盘错误
    unsigned    reserve                :31;

}hw_diag_state_t;
/**********************************************************************************
 *	报警模块相关命令
 **********************************************************************************/

/*报警状态返回(主要指防区报警)
   当防区报警状态改变或故障状态改变时,报警模块主动发送此命令
   或者当收到MAIN_QUERY_STATE命令时返回此命令
*/
#define	ALARM_STATE_RETURN			0x0801	

#ifndef pid_t
#define	pid_t	int	
#endif

typedef struct {
//报警返回的结构
	pid_t	pid;			//进程id
	time_t 	time;		//报警时间
	DWORD	state;		//报警模块控制的设备相关状态如串口等
						//按位表示 1表示有故障 0表示正常
	DWORD	alarm;		//报警的防区状态最多表示32个防区 1表示有报警 0表示没有报警
}ALARM_STATE;


//设置报警输出
#define SET_ALARM_OUT	 0x1802
typedef struct{
	int	ch;			//输出端口号0，1，2...
	int	val;			//值 1表示输出 0表示不输出
	int	timelong;	//表示需要输出多少秒,0表示一直输出(暂时没有用)
}ALARM_OUT;


/**********************************************************************************
 *	upnpd端口映射进程相关命令
***********************************************************************************/

/*
 *	upnpd端口映射进程发送给主进程状态
 *    格式:len(2)+UPNPD_STATE_RETURN(2)+state(4)
 *    state:UPNPD模块的状态，

*/
#define UPNPD_SUCCESS				0	//upnpd正常映射
#define UPNPD_FAILURE				1   //upnpd映射失败
#define UPNPD_STATE_RETURN		0x0c01



/**********************************************************************************
 *	hdplayback进程相关命令
***********************************************************************************/
#define HDPLAYBACK_STATE_RETURN		0x2301
struct hdplayback_state_struct{
	unsigned int    err;         //// 0 正常 非0异常
};









/*
	转发命令，此类命令用于主模块将远程网关发来的相关命令直接转发到相应模块
	以及将相应模块需要返回给网关的命令发送出去
*/
//实际转发时，第一个字段应该是4字节的文件描述符，表示收到命令的连接
//主模块发送的命令
/*主模块将网关发来的信息原样包含在模块内部通讯的消息体里发送给相应模块*/
/*格式
	len(2)+GATE_BYPASSTO_MOD_CMD(2)+gate_fd(4)+cmd_pkt(len-2-4)
	gate_fd:主进程用于标识网关的文件描述符，其它进程只要把这个描述符原样发回就可以了
*/
#define  GATE_BYPASSTO_MOD_CMD		0x1000//网关发来的命令如果主模块自己处理不了则转发给相应的模块处理

//模块发送的命令
/*模块将需要发送给网关的返回消息包含在消息体内，发给主进程，由主进程转发给网关*/
/*格式
	len(2)+MOD_BYPASSTO_GATE_CMD(2)+gate_fd(4)+cmd_pkt(len-2-4)
	gate_fd:主进程发送GATE_BYPASSTO_MOD_CMD命令时带的gate_fd参数，其它模块需要在返回信息的时候原样填充这个字段
*/
#define  MOD_BYPASSTO_GATE_CMD		0x2000//模块反馈一些信息给网关	

//模块发送的命令
/*模块将需要发送给网关的返回结果包含在消息体内，发给主进程，由主进程转发给网关*/
/*格式
	len(2)+MOD_BYPASSTO_GATE_ACK(2)+gate_fd(4)+command(4)+result(4)
	gate_fd:主进程发送GATE_BYPASSTO_MOD_CMD命令时带的gate_fd参数，其它模块需要在返回信息的时候原样填充这个字段
*/	
#define  MOD_BYPASSTO_GATE_ACK		0x2001//模块反馈result给网关
#endif

