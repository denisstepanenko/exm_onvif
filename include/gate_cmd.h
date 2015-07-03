/*
	定义GT1000系统与远程网关交互的命令字
*/

#ifndef GATE_CMD_H
#define GATE_CMD_H
//#include "dev_svr.h"
#include <status.h>
#ifndef _WIN32  
#include <pthread.h>
#include <typedefine.h>
////zw
#include<sys/time.h>
#include<unistd.h>
#include<stdio.h>
//#include <gt_netsdk_ip.h>
////
#else
#include <windows.h>
#endif
//GT1000与网关通讯时用到的关于时间的结构
struct gt_time_struct{
	WORD		year;		//年
	BYTE		month;		//月
	BYTE		day;		//日
	BYTE		hour;		//时
	BYTE		minute;		//分
	BYTE		second;		//秒
	BYTE		reserve;		 
};

//PC机发出的命令
#define USR_REQUIRE_RT_IMAGE	0x0100				 //网关发送设定允许访问实时图像端口的客户ip
#define USR_STOP_RT_IMAGE       0x0102
#define USR_REQUIRE_RT_IMAGE_ANSWER 0x8100
#define USR_REQUIRE_SPEAK		0x0107				//网关发送允许访问音频下行端口的客户的ip
#define USR_REQUIRE_RECORD_PLAYBACK 0x0117        //网关发送请求录像点播命令

//control&set
#define  USR_SET_AUTH_IP   				0x0101		 //网关服务器地址设定
#define  USR_CLOCK_SETING				0x0200 		 //时钟设定
#define  USR_SCREEN_SETING				0x0204 		 //画面分割控制
#define  USR_NET_STREAM_SETING 		0x0205		 //设定网络传输图像质量
#define  USR_LOCAL_STREAM_SETING 		0x0206		//设定高清晰路像质量
#define  USR_SET_SENSER_ARRAY			0x0208		//设定运动检测
#define  USR_SET_VIDEO_AD_PARAM		0x0209		//视频转换参数设置
#define  USR_SET_SWITCH_IN				0x0213		//开关量输入属性设置
#define  USR_SET_SWITCH_OUT			0x0214		//开关量输出属性设置
#define  USR_SWITCH_OUT				0x0215		//开关量输出控制


#define  USR_LOCAL_RECORDER_CONTROL 	0x0216		//高清晰路像控制
#define  USR_LOCAL_RECORDER_SETING	0x0217		//高清晰路像属性设置
#define  USR_SEIRAL_PORT_CONTROL		0x0218		//设备串行口控制
#define  USR_CANCEL_ALARM				0x0219		//解除报警状态

#define  USR_QUERY_INDEX				0x0105		//查询设备高清晰索引文件
#define  USR_QUERY_FTP					0x0106		//查询ftp服务帐号密码
#define  USR_RW_DEV_PARA				0x0108		//访问设备配置文件
#define  USR_QUERY_TRIGSTATE			0x0109		//设备报警状态查询	add-20071113		

#define 	USR_QUERY_STATE				0x0103		//设备状态查询
#define 	USR_CMD_ACK					0x0201		//邋邋USER响应设备发出的命令
#define 	USER_UPDATE                			0x0220 		//升级软件
#define 	USR_TAKE_HQ_PIC				0x0221 		//抓图			
#define	START_ALARM_ACTIONS_YES		0x0222 		//确认报警
#define 	START_ALARM_ACTIONS			0x0223		//控制执行报警联动
#define 	USR_LOCK_FILE_TIME			0x0224		//将指定时间段的高清晰文件加锁或解锁
#define	USR_REBOOT_DEVICE				0x0225		//设备复位
#define	USR_SET_ALARM_SCHEDULE		0x0226		//布撤防及设定报警有效时间段
#define	UPDATE_SOFTWARE_DIRECT		0x0227		///直接给设备升级(不采用ftp方式)
#define 	USR_QUERY_HD_STATUS			0x0228		//硬盘状态参数查询	//add-20071113
#define	USR_QUERY_REGIST				0x0229		//注册查询				//add-20071113
#define	USR_QUERY_VIDEO_AD_PARAM	0x0230		//图像A/D参数查询		//add-20071112
#define   USR_RUN_HD_TEST 				0x0231     	//触发硬盘状态测试
#define	USR_FORMAT_HARDDISK			0x0232		//远程格式化硬盘

//// 2010-9-16 by lsk
#define	USR_TAKE_HQ_PIC_DIR			0x0233		//高清晰录像控制(直连模式)
#define	USR_SEIRAL_PORT_INFO 			0x0234		//透明串口控制命令 (直连模式)
#define   USR_QUERY_INDEX_DIR			0x0235		//查询设备高清晰索引文件(直连模式)

#define 	USR_RW_DEV_PARA				0x0108		//访问设备配置文件

#define	USR_LOGIN_DEVICE				0x0110		///<登录设备
#define   USR_REQUIRE_SELF_IP			0x0111		///<查询自己在设备端看到的ip地址
#define	USR_QUERY_DEVICE_POSITION	0x0113		//控制设备发送自己的位置信息			
//设备唤醒和心跳2010 -02- 01	//add by lsk
#define	USR_WAKE_UP_DEVICE			0x0114		//唤醒
//#define	USR_QUERY_HEART_BEAT			0x0115		//心跳

//// 2010-9-16 by lsk
#define	USR_QUERY_LOG					0x0116		//查询设备日志(直连模式)


//// 2013-12-24 lc 定义录像点播相关信令与结构
//////////////////////////////////////////////////////////////////////////////////////////
typedef struct 
{
	unsigned short year;
	unsigned char  month;
	unsigned char  day;
	unsigned char  hour;
	unsigned char  minute;
	unsigned char  second;
	unsigned char  reserved;
}timepoint_struct;

typedef enum
{
	QSPEED,
	HSPEED,
	NSPEED,
	TSPEED,
	FSPEED,
	ISPEED
}transpeed_t;

typedef enum
{
	PAUSE,
	RESUME,
	SPEED,
	SEEK
}recordctl_t;

#define  USR_QUERY_TIMESECTION			0x0120		//查询设备录像时间段

typedef struct
{
	unsigned char  channel;            ////对应视频通道
	unsigned char reserved[3];
	
	timepoint_struct starttime;
	timepoint_struct endtime;
}usr_query_timesection_struct;  

#define  DEV_TIMESECTION_RETURN		0x8220
typedef struct
{
	unsigned char  dev_id[8];           ///dev guid
	unsigned short result;              ////操作成功0，查不到返回ERR_DVC_NO_RECORD_INDEX,ERR_ENC_NOT_ALLOW设备不允许本操作
	unsigned short format;              ////索引格式 0：原文 1：gzip压缩 目前只支持原文方式

	unsigned char  index_file[48];      ////索引文件名，格式为: 路径 + 文件名
}dev_timesection_ret_struct;

#define  VIEWER_SUBSCRIBE_RECORD		0x0607

typedef struct
{
/*
	unsigned short len;
	unsigned short cmd;                 ////VIEWER_SUBSCRIBE_RECORD
	unsigned char  en_ack; 
	unsigned char  reserved[3];
*/
	unsigned char  dev_ip[32];           ////设备ip
	unsigned char  reserved1[12];        //// for ipv6
	unsigned char  dev_id[8];           ////dev guid
	unsigned char  gateway_ip[32];          ////gate ip
	unsigned char  reserved2[12];        //// for ipv6
	unsigned char  token[20];            ////　　访问令牌(字符串)

	unsigned short user_level;          ////　　用户访问级别
	unsigned char  user_account;        ////    用户帐号
	unsigned char  reserved3;
	
	unsigned short trans_protocal;      ////    传输协议
	unsigned short time_out;             ////	如果控件连接被断，转发服务器需要等待的时间，以秒为单位（如果设置为0，则使用转发服务器自定义的超时时间）

	unsigned short band_width;          ////  用户网络带宽，以K为单位(本字段预留，如果不设定可将其清为0)
	unsigned char  reserved4[2];

	unsigned int   dev_port;             ////  设备录像服务端口
	unsigned int   stream_idx;           ////  录像回放事物号，由监听端产生

	unsigned char  peer_ip[32];            ////请求端ip地址
	unsigned short peer_port;            ////请求端监听port端口
	unsigned short channel;              //// 0-31
	
	transpeed_t    speed;                ////  点播速率 
	timepoint_struct starttime;          ////  起始订阅时间（有效时间段内）
	timepoint_struct endtime;            ////  终止订阅时间（有效时间段内）
}viewer_subscribe_record_struct;

#define  VIEWER_SUBSCRIBE_ANSWER_RECORD	0x0608

typedef struct
{
	unsigned char  dev_id[8];           ////dev guid
	unsigned short status;              ////返回操作状态（0表示成功，非0表示错误号)
	unsigned char  reseved[2];
	
	unsigned int   video_trans_id;     ////由转发服务器动态生成的传输事务ID号（如不成功为0）
	unsigned int   query_usr_id;       //// 　　设备返回给sdk端的订阅用户标识号
}viewer_subsrcibe_answer_record_struct;

#define  VIEWER_SUBSCRIBE_RECORD_CONTROL 	0x0609

typedef struct
{
	unsigned char  dev_id[8];           ////dev guid
	unsigned int   query_usr_id;       //// 　　设备返回给sdk端的订阅用户标识号

	recordctl_t    ctl_cmd;             //// 回放控制命令
	transpeed_t    speed;               //// 速度值

	timepoint_struct starttime;        ////  改变订阅时间
	timepoint_struct endtime;              ////  改变订阅时间
}viewer_subscribe_record_ctl_struct;

#define  VIEWER_SUBSCRIBE_RECORD_CONTROL_RESULT	 0x061a

typedef struct
{
	unsigned char  dev_id[8];           ////dev guid
	unsigned short status;              ////返回操作状态（0表示成功，非0表示错误号)
	unsigned char  reserved2[2];

	unsigned char  msg[16];              //// 英文描述错误原因
}viewer_subscribe_record_ctl_ret_struct;

#define  VIEWER_UNSUBSCRIBE_RECORD	0x061b

typedef struct
{
	unsigned char  dev_id[8];           //// dev guid
	unsigned int   query_usr_id;       //// 　　设备返回给sdk端的订阅用户标识号
}viewer_unsubscribe_record_struct;



////////////////////////////////////////////////////////////////////////////////////////////
//临时调试用，杀vsmain
#define USR_KILL                        			0x0250


//GT1000发出的命令
#define DEV_REGISTER					0x8101		//设备注册
#define DEV_STATE_RETURN				0x8102		//设备状态报告
#define DEV_ALARM_RETURN				0x8103		//发案报警
#define DEV_COM_PORT_RET				0x8104		//串口对应的tcp端口返回
#define USR_QUERY_FTP_ANSWER			0x8105		//ftp帐号返回
#define DEV_RECORD_RETURN				0x8106		//返回高清晰录像索引文件名

#define UPDATE_ANSWER                  		0x8107		//升级响应    
#define HQ_PIC_ANSWER              			0x8108		//抓图响应
#define DEV_PARA_RETURN 				0x8109		//返回配置文件
#define DEV_REQ_SYNC_TIME				0x810a		//请求时间同步
#define DEV_QUERY_ADPARAM_RETURN 	0x810b		//返回图像A/D参数	//add-20071112
#define DEV_QUERY_HDSTATUS_RETURN	0x810c		//返回硬盘状态参数查询	//add-20071113
#define DEV_QUERY_TRIG_RETURN			0x810d 		//返回设备报警状态	//add-20071113
#define DEV_LOGIN_RETURN				0x8110		///设备对登录命令的响应		
#define DEV_IP_RETURN					0x8111		///<返回远程客户端及设备自己的ip地址
#define FORMAT_DISK_ANSWER			0x8112		///<格式化硬盘响应
#define DEV_POSITION_RETURN			0x8113		//设备发送位置信息,收到USR_QUERY_DEVICE_POSITION命令后返回
#define DEV_SLEEP_REPORT				0x8114		//设备休眠前的消息通知给邋邋USER	// add 2010-2-1 lsk
#define DEV_HEART_BEAT					0x8115		//心跳

//// 2010-9-16	by lsk
#define DEV_COM_INFO_RET				0x8116		//透明串口控制和数据发送(直连模式)
#define HQ_PIC_ANSWER_DIR         		0x8117		//抓图响应(直连模式)
#define DEV_QUERY_LOG_RETURN         		0x8118		//日志文件返回(直连模式)
#define DEV_QUERY_NDEX_DIR_RETURN      	0x8119		//录像索引文件返回(直连模式)

#define DEV_CMD_ACK					0x8200		//设备发出响应邋邋USER的命令

//#define DEV_UNSUBSCRIBE_RECORD_ANSWER  0x8201   ///设备发送停止录像订阅响应

//网关发来的设置配置文件参数命令
struct usr_rwdevpara_struct{
  	WORD	type;	       // 类型（2:ip1004.ini 3:alarm.ini其他值保留具体见REMOTE_FILE_TYPE的订以）
	WORD	mode;		//操作模式:0：读 1：写
	DWORD 	filelen;		//文件长度(在写模式下有效,长度不能超过60k)
	BYTE	file[4];		//	文件内容(仅在写模式下有效)
};
//网关发来的请求实时图像命令
//这个结构会被原样转发到实时图像模块
//转发时在结构前应该有一个fd字段表示返回信息的描述符
struct usr_req_rt_img_struct{
//	WORD	save_flag;		//应该一直为0
//	WORD	mode;			//访问模式 1表示请求连接，0表示强行断开任何已有的实时图像连接，其他值待定
	WORD 	trans_protocal;    //协议类型
	WORD    channel;        //通道号
	DWORD 	remoteip;		//要访问GT1000实时图像的计算机ip地址
	BYTE  	reserved[12];   //ipv6
	DWORD   remoteport;     //对端端口
	DWORD   stream_idx;     //事务号
	DWORD   audio_flag;     //是否需要音频
	
};


//设备响应网关请求实时图像命令
struct usr_req_rt_img_answer_struct{
	BYTE  	dev_id[8];           ////dev guid
	WORD  	status;              ////返回操作状态（0表示成功，非0表示错误号)
	BYTE  	reseved[2];	
	DWORD	video_trans_id;     ////由转发服务器动态生成的传输事务ID号（如不成功为0）
	DWORD   query_usr_id;       //// 　　设备返回给sdk端的订阅用户标识号
};
struct usr_stop_rt_img_struct{
	unsigned char  dev_id[8];           //// dev guid
	unsigned int   query_usr_id;       //// 　　设备返回给sdk端的订阅用户标识号
};

//网关发来的设置网关服务器命令
struct usr_set_auth_ip_struct{
	WORD save_flag;			//存入前端嵌入式视频服务器（DEVICE） 硬件标志，0-不存，1-存入
	WORD reserve;			//保留
							//允许访问的服务器信息
	BYTE  server_sn;			//服务器存储序号(查找优先级序号)
	BYTE  reserve1[3];		//保留
    	DWORD ip;				//服务器ip地址（ip3,ip2,ip1,ip0）
	DWORD reserve2[12];		//为ipv6地址扩展保留

};
//网关发来设置系统时间的命令
//#define SIZEOF_USR_SET_TIME_STRUCT	(9)
#define SIZEOF_USR_SET_TIME_STRUCT	(sizeof(struct usr_set_time_struct))
struct usr_set_time_struct{
	WORD	save_flag;		//存入前端嵌入式视频服务器（DEVICE） 硬件标志，0-不存，1-存入
	WORD	year;			//年
	BYTE	month;			//月
	BYTE	day;			//日
	BYTE	hour; 			//时
	BYTE	minute; 			//分
	BYTE	second;			//秒

};




//触发输入属性设置
struct trig_in_attrib_struct{
	WORD	save_flag;		//是否存储标志
	WORD	ch_mode;		//低字节表示通道号，高字节表示常开或常闭
	DWORD	action0;			//对应端口绑定的动作
	DWORD	action1;
	DWORD	action2;
	DWORD	action3;
};

//设备串行口控制命令的数据结构 USR_SEIRAL_PORT_CONTROL
#define SIZEOF_USR_SERIAL_CTL_STRUCT 28
struct usr_serial_ctl_struct{
	WORD	save_flag;		//是否存储标志
	BYTE	ch;				//GT1000上的串口号
	BYTE	mode;			//建立还是断开
	DWORD	remoteip;		//要访问串口的计算机ip地址
	BYTE	reserve[12];		//保留for ipv6
//串口参数设置
	DWORD	baud;			//波特率
	BYTE	databit;			//数据位，一般为8
	BYTE	parity;			//是否要奇偶校验位,一般为不需要'N'	
	BYTE	stopbit;			//停止位，一般为1
	BYTE	flow;			//流量控制，一般不需要
};
//设备返回串口对应端口的数据结构 DEV_COM_PORT_RET
#define SIZEOF_DEV_COM_RET_STRUCT	6
struct dev_com_ret_struct{
	WORD	ch;				//GT1000上的串口通道号
	WORD	result;			//操作结果
	WORD	port;			//如果成功表示串口对应的tcp端口
};



//zw-add-20100916 --------------------->直连模式下的串口数据结构
//PC端发送给设备的结构
struct usr_serial_info_struct
{
	WORD	save_flag;		//是否存储标志
	BYTE	ch;				//GT1000上的串口号
	BYTE	mode;			//发送的是串口配置还是串口数据mode=0取消连接，mode=1配置，mode=2 串口数据发送
	DWORD	remoteip;		//要访问串口的计算机ip地址
	BYTE	reserve[12];		//保留for ipv6
//串口参数设置
	DWORD	baud;			//波特率
	BYTE	databit;			//数据位，一般为8
	BYTE	parity;			//是否要奇偶校验位,一般为不需要'N'	
	BYTE	stopbit;			//停止位，一般为1
	BYTE	flow;			//流量控制，一般不需要
	WORD 		data_len;				//用户发送的数据长度
//zw-del	BYTE		Data[data_len];			//存放数据的缓冲区
	BYTE		data[2];
};
#define SIZEOF_USR_SERIAL_INFO_STRUCT (sizeof(struct usr_serial_info_struct))

//设备返回给PC的结构
struct dev_com_info_ret_struct
{
	WORD		ch;				//GT1000上的端口号
	WORD		result;			//设备操作串口的结果
	WORD		data_len;		//设备接收到的数据字节数
};
#define	SIZEOF_DEV_COM_INFO_RET_STRUCT		(sizeof(struct dev_com_info_ret_struct))

//<-----------------------------------zw-add 直连模式下串口数据结构


//----------------------------------------->zw-add 直连模式下抓取高清图像
//PC端发给设备的结构
struct usr_take_hq_pic_dir_struct
{
	WORD	save_flag;			//保存标志
	WORD	rec_ch; 			//按位表示的需要进行抓图的通道，对于2路压缩的前端嵌入式视频服务
								//器(DEVICE)此参数无效，对于5路压缩的系统此参数有效，1表示需要
								//录像，0表示不需要录像，如0x0003表示通道0，1需要抓图，其他通
								//道不需要抓图
	BYTE	time[8];				//时间戳
	WORD	getpic;				//连续抓图的张数
	WORD	interval;			//抓拍的时间间隔(单位:毫秒)(近似值)
};
#define 	SIZEOF_USR_TAKE_HQ_PIC_DIR_STRUCT		(sizeof(struct usr_take_hq_pic_dir_struct))

//设备返回给PC的结构
struct dev_hq_pic_answer_dir_struct
{
	BYTE		dev_id[8];		//设备id
	WORD 		state;			//操作结果，0成功
								//其他值为错误，错误码见<<错误号定义>>中的[前端设备定义的错误]
								//ERR_DVC_FAILURE	ERR_DVC_NOT_SUPPORT	ERR_NO_SPACE
	WORD		reserve2;		//保留
	BYTE		timestamp[8];	//时间戳(state为0有效，该值是由USR_TAKE_HQ_PIC发送的)
	BYTE		data_send;		//数据发送标志，1表示数据发送未完成，0所有数据发送完成(包含多个文件)
	BYTE		file_send;		//文件发送标志，1发送未完成，0发送完成
	WORD		reserve3;		//保留
	BYTE		filename[48];	//图片的文件名
	DWORD		pic_seq;			// 图片序号1.....10
	DWORD		pack_seq;		//包序列号1......100
	DWORD		pack_len;		//包长度 最长为0--4096BYTEs
	BYTE		pack[2];			//用来补全字节对齐的缓冲区
};
#define SIZEOF_DEV_HQ_PIC_ANSWER_DIR_STRUCT		(sizeof(struct dev_hq_pic_answer_dir_struct))

//放在ipmain_para使用的保存
typedef struct 
{
	int		take_pic_dr_f;					//直连模式下高清抓图标志，如果为1，表示直连模式在抓图
	int 		netfd;									//连接网关用的描述符
	int		send_file;								//有文件需要发送标志
	int		send_data;								//有数据要发送标志
	int		pic_fd;									//图片文件索引
	int		pic_seq;									//图片序列号
	int		pack_seq;								//数据包序列号
	int 		dev_no;
	int 		result;
	unsigned short enc;
	unsigned short env;
	FILE		*index_fp;								//高清抓图索引文件描述符
	BYTE	filename[48];							//高清图片的名字
	BYTE	timestamp[8];							//高清图片的时间戳
	BYTE	devid[8];								//DEVID
}TAKE_HQ_STRUCT_DIR_FOR_VSMAIN;
//<-----------------------------------------zw-add直连模式下抓取高清图像


//------------------------------------------>zw-add设备返回的直连模式下的查询设备高清晰索引文件
struct dev_query_index_dir_return
{
	BYTE		dev_id[8];		//设备ID
	WORD		state;			//操作结果
	WORD		reserve2;
	BYTE			data_send;
	BYTE			reserve3;	
	BYTE			filename[48];
	DWORD		pack_seq;
	DWORD		pack_len;
	BYTE			pack[2];
};
#define 	SIZEOF_DEV_QUERY_INDEX_DIR_STRUCT		(sizeof(struct dev_query_index_dir_return))

//<-----------------------------------------zw-add 





//网络传输的图像显示方式控制结构
#define SIZEOF_SCR_CTRL_STRUCT			6
struct scr_ctrl_struct{
	WORD	save_flag;		//存入前端嵌入式视频服务器（DEVICE） 硬件标志，0-不存，1-存入
	BYTE	disp_type;		//显示方式 	0：单画面显示
							//		  		1：4画面分割显示
	BYTE	ch;				//如果是单画面方式，这个字段表示显示的通道号，4画面方式本字段无效
	WORD	reserve; 
};

//设置视频转换参数的命令结?
#define SIZEOF_USR_VIDEO_PARA_STRUCT 12
struct usr_video_para_struct{
	WORD	save_flag;		//存入前端嵌入式视频服务器（DEVICE） 硬件标志，0-不存，1-存入
	WORD	channel;		//要设置的图像通道，0，1，2，3…以下参数 -1000表示不设定、无效，
	short	bright;			//亮度	：0～255
	short	hue;			//色度	：－127～127
	short	contrast;		//对比度	：－127～127
	short	saturation;		//饱和度  ：0～255
		
};

//设置网络传输音视频码流的参数
#define SIZEOF_USR_NET_AVSTREAM_SET_STRUCT 14
struct usr_net_avstream_set{
		WORD save_flag;		//存入前端嵌入式视频服务器（DEVICE） 硬件标志，0-不存，1-存入
		WORD picsize;		//图像规格：有以下四种
							//0x0000:  无效，即本参数不需要改变
							//0x0001:	 D1------720*576（PAL）
							//0x0002:  CIF-----352*288
							//0x0003:  QCIF----176*144
							//0x0004:  HD1-----720*288
		WORD vbitrate;		//图像码流，单位kbit/s，0表示本参数不需要改变
		WORD frame;		//图像帧率 最大25
		BYTE   aud_mode;	//	音频编码方式	0:不需要音频 1:adpcm  2:pcm 3:raw-pcm
		BYTE   aud_samrate;	//音频采样率，单位kbit/s
		BYTE   aud_sambit;	//音频转换精度 24或其他
		BYTE   reserve[3];
	
};
#define SIZEOF_USR_LOCAL_AVSTREAM_SET_STRUCT 16
struct usr_local_avstream_set{
	
		WORD	save_flag;	//		存入前端嵌入式视频服务器（DEVICE） 硬件标志，0-不存，1-存入
		WORD	channel;	//	 	图像通道0,1,2,3(如果是两块卡结构，即视频服务器高清晰录像是4画面形式，则1，2，3无效)
		WORD	picsize	;	// 			图像规格：有以下四种
							//0x0000:  无效，即本参数不需要改变
							//0x0001:	 D1------720*576（PAL）
							//0x0002:  CIF-----352*288
							//0x0003:  QCIF----176*144
							//0x0004: halfd1
		WORD	vbitrate;	//图像码流，单位kbit/s，0表示本参数保持不变
		WORD	frame;		//	图像帧率 0：25 1：25/2  2： 25/4  3：25/8  4：25/16 5：25/32 其他值无效
		BYTE   	aud_mode;	//音频编码方式	0:不需要音频 1:adpcm 2：Pcm
		BYTE	aud_samrate; // 音频采样率，单位kbit/s  //不用
		BYTE	aud_sambit;//音频转换精度 24或其他  //不用
		BYTE   reserve[3];

};

typedef struct {
		WORD	save_flag; 		//存入前端嵌入式视频服务器（DEVICE） 硬件标志，应为0-不存
		WORD	diskno;	 		//要格式的硬盘编号
								//0表示格式化所有硬盘，1，2。。等表示格式化第1，2..块硬盘（多硬盘时）。
		DWORD	reserved1;		//保留	
		char	usr_info[256];	//进行格式化操作的用户信息
}usr_format_hd_struct;


//ip1004系统信息
#define SIZEOF_IP1004_INFO_STRUCT	sizeof(struct ip1004_info_struct)//
struct ip1004_info_struct
{							//注册时发送的设备信息结构
	DWORD vendor;			//设备制造商标识(4) +设备型号标识(4)
	DWORD device_type;		//设备型号
	DWORD protocal_version;	// 网关和前端设备通讯的协议内部版本号(无符号整数)
	BYTE site_name[40];		//安装地点名称
	DWORD dev_ip;			//设备的动态ip地址
	BYTE reserve[12];		//为ipv6地址扩展保留
 	WORD boot_type;		//设备的注册原因,16位，按位表示意义
  	WORD video_num;		//设备视频输入通道数。
  	WORD com_num;			// 串口数
	WORD storage_type;		//设备的存储介质类型 0：没有 1：cf卡 2：硬盘
	DWORD storage_room;	//设备的存储介质容量 单位Mbyte
	WORD compress_num;	//压缩通道数	目前为1，2或5
	WORD compress_type;	//压缩数据类型，(压缩芯片，压缩格式)
	WORD audio_compress;	//声音输入压缩类型
	WORD audio_in;			//声音输入通道数，目前为1
       WORD audio_in_act;		//声音输入通道有效位，从右向左，1-8，0-通道无效，1-有效
	WORD switch_in_num;		//开关量输入通道数
	WORD switch_in_act;		//开关量输入通道有效位，从右向左，1-8，0-通道无效，1-有效
	WORD switch_out_num;	//开关量输出通道数
	WORD switch_out;			//开关量输出通道有效位，从右向左，1-8，0-通道无效，1-有效
	WORD max_pre_rec;		//设备最大预录时间，以秒为单位
	WORD max_dly_rec;		//设备最大延时录像时间
	//设备出厂时间
	WORD	year;			//年
	BYTE	month;			//月
	BYTE 	day; 			//日
	BYTE	hour;			//时
	BYTE	minute;			//分
	BYTE	second;			//秒
	BYTE	reserve1; 
	BYTE	reserve2;		
	BYTE	sleep;			// 支持休眠功能标志  0 不支持； 1支持。
	DWORD    cmd_port;    		//命令服务端口
	DWORD    image_port;		//图像服务端口	
	DWORD    audio_port;		//音频服务端口
	BYTE	firmware[20];		//固件版本号，暂时不用
	BYTE	dev_info[40];		//设备的一些相关信息

	BYTE	ex_info[160];		//外接dvs(如果有的话)的相关信息，包括品牌，端口，用户名，密码
	
};

#if 0
struct per_state_struct{			//GT1000外设连接状态
	unsigned 		video_loss0		:1;	//bit0视频输入丢失
	unsigned 		video_loss1		:1;
	unsigned		video_loss2		:1;
	unsigned 		video_loss3		:1;
	unsigned 		reserve			:8;
	unsigned		video_blind0	:1;//bit12 第1路视频遮挡
	unsigned		video_blind1	:1;
	unsigned 		video_blind2	:1;
	unsigned 		video_blind3	:1;
	unsigned		audio_loss0		:1;	//bit16 音频输入丢失
	unsigned		audio_loss1		:1;
	unsigned		audio_loss2		:1;
	unsigned		audio_loss3		:1;
	unsigned		disk_full		:1;	//磁盘满
	unsigned 		keyboard_err	:1;	 //键盘连接故障
	unsigned		reserve1		:2;
	unsigned 		pwr_loss		:1;	//外接触发用电源故障
	unsigned		audio_out_err	:1;	//音频输出故障
	unsigned		reserve2		:6;
};
struct dev_state_struct{		//GT1000系统内部状态
	unsigned		link_err			:1;	//断线
	unsigned		mem_err		:1;	//内存故障
	unsigned		flash_err		:1;	//flash故障
	unsigned		hd_err			:1;	//硬盘故障	//这位不用了
	unsigned		cf_err			:1;	//存储卡故障
	unsigned		audio_dec_err	:1;	//音频解码设备故障
	unsigned		reserve			:2;
	unsigned		video_enc0_err	:1;	//netenc
	unsigned		video_enc1_err	:1;	//hq0
	unsigned		video_enc2_err	:1;	//hq1
	unsigned		video_enc3_err	:1;	//hq2
	unsigned		video_enc4_err	:1;	//hq3
	//06.09.06	新加故障
	unsigned		quad_dev_err	:1;	//画面分割器故障
	unsigned 		watch_51_err		:1;	//51模块故障
	unsigned        vda_mod_err     :1;//vda模块故障
	unsigned        reserve1        :17;
};

struct	trig_state_struct{	//GT1000系统报警状态
	unsigned		trig0			:1;//外触发
	unsigned		trig1			:1;
	unsigned		trig2			:1;
	unsigned		trig3			:1;
	unsigned		trig4			:1;
	unsigned		trig5			:1;
	unsigned		trig6			:1;//震动触发//trig_vib
	unsigned		trig7			:1;//外部触发电源//trig_pwr
	unsigned		reserve			:2;
	unsigned		motion0			:1;//移动触发
	unsigned		motion1			:1;
	unsigned		motion2			:1;
	unsigned		motion3			:1;
	unsigned		motion4			:1;
	unsigned		motion5			:1;
	unsigned		motion6			:1;
	unsigned		motion7			:1;
	unsigned		motion8			:1;
	unsigned		reserve1		:13;

	
};
#endif
//查询索引时用到的结构
struct query_index_struct{
	struct gt_time_struct start;
	struct gt_time_struct stop;
};

//查询FTP帐号命令的数据结构
struct query_ftp_struct{
	  WORD		type;		//类型（0x1: 高清晰度录像；0x2：日志; 0x3: 配置文件）
	  WORD		reserve;	//保留
  	  DWORD 	client_ip;	//客户机的IP地址
	  BYTE		reserve2[12];	//为ipv6保留的字段
};




#define	SIZEOF_FTP_INFO_STRUCT	sizeof(struct ftp_info_struct)
struct ftp_info_struct{		//返回ftp帐号用的结构
	WORD		result;		//操作结果 0:成功 
							//其他值表示错误，错误码见《错误号定义》中的[前端设备定义的错误]
							//ERR_ENC_NOT_ALLOW：设备不允许本操作
	WORD		reserve;		//保留
  	BYTE		user[12];		//FTP用户名
  	BYTE		password[12];//FTP用户密码
	DWORD		expired_time;	//  用户名密码过期时间（以秒为单位）
	DWORD		ftp_port;		//ftp服务端口号
};
//返回索引检索结果的结构
struct index_return_struct{
  	WORD		result;		//操作结果 0:成功  
							//查不到时返回ERR_DVC_NO_RECORD_INDEX
							//其他值表示错误，错误码见《错误号定义》中的[前端设备定义的错误]
							//ERR_ENC_NOT_ALLOW：设备不允许本操作
	 WORD		format;		//索引格式 0：原文 1：gzip压缩 目前只支持原文方式
	 WORD		reserve;	//保留
	 char		indexname[2]; //	索引文件名

};
struct local_record_set_struct{
		WORD save_flag;		//	是否存入前端嵌入式视频服务器（DEVICE） 硬件标志，0-不存，1-存入
		WORD pre_rec;		// 	预录时间，以秒为单位,如果前端设备的空间达不到设定的值，则前端自动设成最大预录时间。
		WORD dly_rec;		// 	延时录像时间，以秒为单位，如果前端设备的空间达不到设定的值，则前端自动设定成最大延时录像时间。
		WORD file_len;		// 	设定高清晰度录像文件最大长度，以秒为单位，如果高清晰录像的时候文件到达了此长度，则自动进行切割。
};
//远程计算机控制高清晰录像的结构
struct local_record_ctl_struct{
	WORD	save_flag;	//	本命令的这个参数无意义
	WORD	mode;		//	0停止高清晰录像
						// 1启动高清晰录像
	WORD	rec_ch;		//按位表示的需要进行高清晰录像的通道，对于2路压缩的前端嵌入式视频服务器（DEVICE） 此参数无效，对于5路压缩的系统此参数有效，1表示需要录像，0表示不需要录像，如0x0003表示通道0，1需要高清晰录像，其他通道不需要高清晰录像
	WORD	rec_time;	//高清晰录像的时间，以秒为单位，当mode=1时此字段有效。rec_time=0表示录像65535秒，或直到收到停止以后（实际上由于视频服务器存储空间有限，不可能无限制的录像，所以rec_time=0要慎用，当空间不足时，前端设备会自动覆盖最早的记录）
};

struct update_software_struct{
   							//user发来的升级命令结构
    WORD    	type; 			//类型,0为应用程序,1为内核映像，2为ramdisk	//已经没用了
    WORD    	reset_flag;		//(0为重启进程，1为重启设备)				//已经没用了
    BYTE    	userid[12];		//用户名
    BYTE   	 password[12];	//密码
    DWORD   	FTPip;			//FTP服务器地址
    BYTE    	reserve[12];		//为ipv6保留的字段
    WORD    	ftpport;			//FTP服务器端口号
    WORD	  	reserve1;
    DWORD   	filesize;			 //升级文件大小
    BYTE    	filepath[50]; 		//文件路径
};

struct update_direct_struct{		//UPDATE_SOFTWARE_DIRECT
					///user发来的直接升级的命令结构
	DWORD	type;			///升级文件的类型，保留，写0
	DWORD	filesize;		//升级文件大小（后面跟着的文件数据大小）
};


struct usr_run_hd_test{	//触发硬盘测试
	WORD	save_flag; 		//存入前端嵌入式视频服务器（DEVICE） 硬件标志，应为0-不存
	WORD	diskno; 		//要测试的硬盘编号，0，1，2。。等（多硬盘时）。
	WORD	testtype;		//测试类型，0-短测试，1-长测试，2-短测试结束后，如果通过，进行长测试	
	WORD	reserved1;		//保留	
};


//user发来的命令响应结构
struct usr_cmd_ack_struct{
	WORD	result;			//执行结果
	WORD	rec_cmd;		//响应的命令
};

struct user_upd_ack_struct{
    BYTE    dev_id[8]; //设备id
    WORD    state;    //处理结果
    WORD    reserve;  
    BYTE    info[16];   //信息
};

typedef struct {
	BYTE	dev_id[8]; 		//设备id
  	WORD	result;			//操作结果 0:成功 其他值为错误，
  							//错误码见《错误号定义》中的[前端设备定义的错误],如ERR_DVC_NO_DISK
	WORD	approxtime;		//预计需要的时间，以秒为单位
	DWORD  	reserve1;		//保留
}format_disk_answer_struct;	  

struct usr_lock_file_time_struct 
{
	WORD	mode;// 1表示锁文件 0表示解锁文件
	WORD	lock_ch;			
	//按位表示的需要进行高清晰录像的通道，对于2路压缩的前端嵌入式视频服务器（DEVICE） 此参数无效，对于5路压缩的系统此参数有效，1表示需要上锁，0表示不需要上锁或解锁；
	//bit0:表示网络编码通道，此位无效，仅是为了统一编号保留的占位
	//bit1:高清晰录像通道1(高清晰录像通道按照1,2,3,4进行编号)
	//bit2:高清晰录像通道2
	//bit3:高清晰录像通道3
	//bit4:高清晰录像通道4
	//如0x0006表示通道1，2需要高清晰录像，其他通道保持原来状态不变，0xfe表示将所有通道的文件都上锁或解锁
	struct  gt_time_struct start;//起始时间
	struct  gt_time_struct stop; //结束时间
};


struct user_get_hq_pic_struct{
	WORD  	save_flag; 	//0
	WORD    rec_ch; 
	BYTE    time[8]; 		//时间戳
	WORD    getpic; 		//连续抓的照片数
	WORD    interval; 	//时间间隔，毫秒
};

struct hqpic_answer_struct{
	BYTE    dev_id[8]; 	//设备id
	WORD    state; 		//操作结果，0为成功
	WORD    reserve2;
	BYTE    timeprint[8]; 	//时间戳
	BYTE    index_file[128];
};

struct takepic_struct{
	int  takepic;
	int  interval;
	int  channel; 			//抓图通道号	
//	int  fd;				//remed by shixin
	BYTE time[8];

};

struct return_para_struct{
	BYTE    dev_id[8]; 	//设备id
  	WORD type;			//    类型（1：表示有错误发生，以下字段无效2:gt1000.ini 3:alarm.ini其他值保留）
	WORD reserve;		//	保留
	DWORD  filelen;		//	文件长度
	BYTE  file[4];			//	文件内容
};
struct send_dev_trig_state_struct {//包括报警状态和时间，用于发送报警需要
	DWORD       alarmstate; 	//设备的报警状态
	WORD		year;		//报警时间，年
	BYTE		month;
	BYTE		day;
	BYTE	 	hour;
	BYTE		minute;
	BYTE		second;
	BYTE 		reserve;
};	 

#ifndef _WIN32	
struct thread_struct{
     pthread_t thread_tid;      /* thread ID */
     long    thread_count;      /* # connections handled */
 };
#endif //_WIN32

struct usr_start_alarm_actions_yes_struct //用户确认报警结构
{
	WORD save_flag;
	WORD reserve1;
	DWORD trig;
};

struct usr_start_alarm_actions_struct //用户控制报警联动结构
{
	WORD save_flag;
	WORD event;
};

struct usr_set_alarm_schedule_struct	//布撤防及设定报警有效时间段结构
{
	WORD save_flag;
	BYTE alarm_type;	//表示需要设置的报警类型，0为端子报警，1为移动侦测报警
	BYTE setalarm;		//表示布防或撤防，1为布防，0为撤防
	WORD channel;		//当alarm_type为0时，表示输入端子0,1,2,3,4,5..当alarm_type为1时，表示视频通道0,1,2,3..
	WORD reserved;
	WORD begin_hour;	//起始的时  如18
	WORD begin_min;		//起始的分  如30   表示18:30
	WORD end_hour;		//结束的时
	WORD end_min;		//结束的分
};
struct usr_set_motion_para_struct   //用于设置运动侦测参数的结构
{ 
	WORD save_flag;
	WORD channel;		//通道
	WORD sen;	  		//灵敏度
	WORD alarm;   		//是否报警，1为报警
	WORD beginhour;
	WORD beginmin;
	WORD endhour;
	WORD endmin;  		//时间段
	WORD area[12];		//运动侦测区域
};

#define SIZEOF_DEV_QUERY_ADPARAM_RETURN		(sizeof(struct dev_query_adparam_ret))
//用于返回用户图像A/D参数查询的结构	//add-20071112
struct dev_query_adparam_ret
{

	BYTE	dev_id[8];	//设备id			//zw-del-20071113	//zw-add-20071116
	WORD	result;		//操作结果 
						//0:		   成功
						//其他值表示出错，如：  
						//ERR_ENC_NOT_ALLOW: 设备不允许本操作
						//其他错误码见《错误号定义》中的[前端设备定义的错误]

	WORD	channel;		//图像通道号,0,1,2,3..	
	WORD	bright;			//亮度	：范围0～100，下同，超出此范围为出错 
	WORD	hue;			//色度	：
	WORD	contrast;		//对比度	： 
	WORD	saturation;		//饱和度  ： 
};


//user发来的查询A/D参数的数据结构	//add-20071112
struct usr_query_adparam
{
	WORD	save_flag;		//存入前端嵌入式视频服务器（DEVICE） 硬件标志，应为0-不存
	WORD	channel;		//要查询的图像通道，0，1，2，3…
};

struct usr_req_position_struct
{
	//请求获得设备gps位置信息的命令结构
	WORD	enable;				//开始发送还是停止发送1:开始发送 0:停止发送
	WORD	target;				//让设备把定位信息发送到哪里,0:已连接的网关 其它，保留,enable=1时有效
	WORD	interval;			//发送位置信息的间隔，单位（秒），enable=1时有效
	WORD	format;				//位置信息格式,0,其它值保留
        //2011-06-16 zw-m 由16位改成32位
       DWORD	send_seconds;		//发送位置信息的持续时间，单位（秒），到时间如果没有收到停止命令则不再发送位置信息；enable=1时有效；
};
#define SIZEOF_DEV_QUERY_TRIG_RETURN		(sizeof(struct dev_query_trig_return))
struct dev_query_trig_return		//返回设备报警状态及变化add-20071113
{
	BYTE	dev_id[8];			//设备id		//zw-del-20071115 //zw-add-20071116
	WORD	result;				//操作结果 
				   				//0：成功
				   				//其他值表示出错，错误码见《错误号定义》中的[前端设备定义的错误]
	WORD	reserve1;			//保留
	DWORD	alarmin;			//当前的端子输入状态，按位表示，目前0~15位有效，1表示有触发、0表示没有触发；
	DWORD 	changed_info_len;	//端口变化信息的长度
	BYTE 	changed_info[4];    //端口变化信息。该信息以XML形式出现, 格式为：
								/*
								<changed_trigs>
								<trig>
								<id> 端口号 </id>
								<state>当前状态，1或0</state>
								<time>发生变化的时间，格式为yyyy-MM-dd hh:mm:ss</time>
								</trig>
								</changed_trigs>
								其中changed_trigs可以包含多个trig
								*/
};

//-->zwadd-20090714
#define SIZEOF_DEV_QUERY_STA_RETURN		(sizeof(struct dev_query_sta_return))
struct dev_query_sta_return
{
        BYTE devid[8];				//设备id
        DWORD p_sta;				//设备外设连接状态
        DWORD d_sta;				//设备其他状态
};
//<--



struct 	usr_query_hd_status		//user发来的查询硬盘状态	//add-20071112
{
	WORD	save_flag;		//存入前端嵌入式视频服务器（DEVICE） 硬件标志，应为0-不存
	WORD	diskno;			//要查询的硬盘编号，0，1，2。。等（多硬盘时）。	
};


#define	SIZEOF_DEV_QUERY_HDSTATUS_RETURN	(sizeof(struct dev_query_hdstatus_return ))
struct  dev_query_hdstatus_return		//返回硬盘状态参数查询	add-20071113
{
	BYTE	dev_id[8];			//设备id		//zw-del-20071115	//zw-add-20071116
	WORD	result;				//操作结果 
				   				//0:		  成功
				   				//其他值表示出错，如：  
				   				//ERR_EVC_NOT_SUPPORT: 设备无指定硬盘
				   				//其他错误码见《错误号定义》中的[前端设备定义的错误]
	WORD	diskno;			// 硬盘编号,0，1,2。。等（多硬盘时）
	BYTE	model[16];		//硬盘型号，字符串    
	BYTE	serialno[16];		//硬盘序列号，字符串
	BYTE	firmware[8];		//固件版本号，字符串
	WORD	volume;			//容量(G为单位，如250G，320G)
	WORD	temprature;		//当前温度(摄氏)
	WORD	maxtemprature;	//历史最高温度(摄氏),值在100以内有效
	WORD	reserve1;		//保留
	DWORD	age;			//工作小时数
	DWORD	relocate;		//重分配扇区数
	DWORD	pending;		//当前挂起扇区数
	DWORD	error_no;		//错误日志数
	WORD	shorttest;		//上一次短测试结果，0通过，1失败,2读不到或未测试过,3进行中
	WORD	shortstatus;	//短测试若在进行中，完成的百分比，0-100的整数	
	WORD	longtest;		//上一次长测试结果，0通过，1失败,2读不到或未测试过,3进行中
	WORD	longstatus;		//长测试若在进行中，完成的百分比，0-100的整数	
};


struct usr_query_regist		//usr  发来的注册查询		//add-20071113
{
	WORD	save_flag;		//存入前端嵌入式视频服务器（DEVICE） 硬件标志，应为0-不存	
	WORD	reserve1;		//保留
};


struct usr_login_device
{//用户登录设备的结构
	BYTE	username[200];		///用户名,登录设备的用户名或用户信息字符串
	BYTE	passwd[32];		///密码,登录设备的密码(暂时无用)
};



struct dev_login_return
{
	BYTE	dev_id[8];		//设备id
	WORD	result;			//操作结果 
	WORD	reserved;		//保留
	DWORD	login_magic;		//设备分配给此次登录的随机数
};

struct dev_ip_return
{
	BYTE	dev_id[8];		//设备id
	WORD	result;			//操作结果 
	WORD	reserved1;		//保留
	DWORD	client_ip;		//客户端ip
	BYTE	reserved2[12];		//保留
	DWORD	device_ip;		//前端设备ip地址
	BYTE	reserved3[12];		//为ipv6保留的字段
	
};

struct dev_position_return_struct{
	//返回设备的gps位置信息
	BYTE	dev_id[8];			//设备id
	WORD	state;				//状态；0：正常 1：无gps信号 2：正在定位 3:故障
	BYTE	reserved[6];			//保留
	double	lon;				//经度
	double  lat;				//纬度
	double  direction;			//方位角  
	double	altitude;			//海拔高度
	double	speed;				//速度(km/h)   
};
/**********************************************************************************************
 * 函数名	:get_gtcmd_name()
 * 功能	:将与网关通讯的命令字转化为字符串返回
 * 输入	:cmd:要转换的命令字
 * 返回值	:命令字的描述字符串
 **********************************************************************************************/
#ifdef _WIN32
static  char * get_gtcmd_name(WORD cmd)
#else
static __inline__ char * get_gtcmd_name(WORD cmd)
#endif
{
	switch(cmd)
	{
//PC机发出的命令
		case USR_REQUIRE_RT_IMAGE:
			return "USR_REQUIRE_RT_IMAGE";
		break;
		case USR_REQUIRE_SPEAK:
			return "USR_REQUIRE_SPEAK";
		break;
		case USR_SET_AUTH_IP:
			return "USR_SET_AUTH_IP";
		break;
		case USR_CLOCK_SETING:
			return "USR_CLOCK_SETING";
		break;
		case USR_SCREEN_SETING:
			return "USR_SCREEN_SETING";
		break;
		case USR_NET_STREAM_SETING:
			return "USR_NET_STREAM_SETING";
		break;
		case USR_LOCAL_STREAM_SETING:
			return "USR_LOCAL_STREAM_SETING";
		break;
		case USR_SET_SENSER_ARRAY:
			return "USR_SET_SENSER_ARRAY";
		break;
		case USR_SET_VIDEO_AD_PARAM:
			return "USR_SET_VIDEO_AD_PARAM";
		break;
		case USR_SET_SWITCH_IN:
			return "USR_SET_SWITCH_IN";
		break;
		case USR_SET_SWITCH_OUT:
			return "USR_SET_SWITCH_OUT";
		break;
		case USR_SWITCH_OUT:
			return "USR_SWITCH_OUT";
		break;
		case USR_LOCAL_RECORDER_CONTROL:
			return "USR_LOCAL_RECORDER_CONTROL";
		break;
		case USR_LOCAL_RECORDER_SETING:
			return "USR_LOCAL_RECORDER_SETING";
		break;
		case USR_SEIRAL_PORT_CONTROL:
			return "USR_SEIRAL_PORT_CONTROL";
		break;
		case USR_CANCEL_ALARM:
			return "USR_CANCEL_ALARM";
		break;
		case USR_QUERY_INDEX:
			return "USR_QUERY_INDEX";
		break;
		case USR_QUERY_FTP:
			return "USR_QUERY_FTP";
		break;

		case USR_QUERY_STATE:
			return "USR_QUERY_STATE";
		break;
		case USR_CMD_ACK:
			return "USR_CMD_ACK";
		break;
		case USER_UPDATE:
			return "USER_UPDATE";
		break;
		case UPDATE_SOFTWARE_DIRECT:
			return "UPDATE_SOFTWARE_DIRECT";
		break;
		case USR_TAKE_HQ_PIC:
			return "USR_TAKE_HQ_PIC";
		break;
		case USR_RW_DEV_PARA:
			return "USR_RW_DEV_PARA";
		break;
		case USR_LOCK_FILE_TIME:
			return "USR_LOCK_FILE_TIME";
		break;
		case USR_QUERY_VIDEO_AD_PARAM:			//add-20071112
			return "USR_QUERY_VIDEO_AD_PARAM";
		break;
		case USR_QUERY_TRIGSTATE:					//add-20071113
			return "USR_QUERY_TRIGSTATE";
		break;
		case USR_QUERY_HD_STATUS:					//add-20071113
			return "USR_QUERY_HD_STATUS";
		break;
		case USR_QUERY_REGIST:						//add-20071113
			return "USR_QUERY_REGIST";
		break;
		case START_ALARM_ACTIONS:
			return "START_ALARM_ACTIONS";
		break;
		case START_ALARM_ACTIONS_YES:
			return "START_ALARM_ACTIONS_YES";
		break;
		

//设备发送的命令		
		case DEV_REGISTER:
			return "DEV_REGISTER";
		break;
		case DEV_STATE_RETURN:
			return "DEV_STATE_RETURN";
		break;
		case DEV_ALARM_RETURN:
			return "DEV_ALARM_RETURN";
		break;
		case DEV_COM_PORT_RET:
			return "DEV_COM_PORT_RET";
		break;
		case DEV_CMD_ACK:
			return "DEV_CMD_ACK";
		break;
		case USR_QUERY_FTP_ANSWER:
			return "USR_QUERY_FTP_ANSWER";
		break;
		case DEV_RECORD_RETURN:
			return "DEV_RECORD_RETURN";
		break;
		case UPDATE_ANSWER:
			return "UPDATE_ANSWER";
		break;
		case HQ_PIC_ANSWER:
			return "HQ_PIC_ANSWER";
		break;
		case DEV_PARA_RETURN:
			return "DEV_PARA_RETURN";
		break;
		case DEV_REQ_SYNC_TIME:
			return "DEV_REQ_SYNC_TIME";
		break;
		case DEV_QUERY_ADPARAM_RETURN:			//add-20071113
			return "DEV_QUERY_ADPARAM_RETURN";
		break;
		case DEV_QUERY_TRIG_RETURN:				//add-20071113
			return "DEV_QUERY_TRIG_RETURN";
		break;
		case DEV_QUERY_HDSTATUS_RETURN :			//add-20071113
			return "DEV_QUERY_HDSTATUS_RETURN";
		break;
		case DEV_POSITION_RETURN :			//add-20100128
			return "DEV_POSITION_RETURN";
		break;
		
		case USR_RUN_HD_TEST:
			return "USR_RUN_HD_TEST";
		break;
			
		
	}
	return "UNKNOW_CMD";
}
		




#endif




