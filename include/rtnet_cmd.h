#ifndef RTNET_CMD_H
#define RTNET_CMD_H

#define	VIEWER_SUBSCRIBE_D1_VIDEO		0x0600	//订阅D1通道的视频(测试用)
#define	VIEWER_SUBSCRIBE_VIDEO			0x0601	//订阅视频
#define   VIEWER_SUBSCRIBE_VIDEO_ANSWER   0x0602	//设备对订阅视频的返回
#define   VIEWER_UNSUBSCRIBE_VIDEO			0x0603	//退定视频
#define 	VIEWER_SUBSCRIBE_AUDIO			0x0604	//请求音频下行通道
#define	VIEWER_SUBSCRIBE_ANSWER_AUDIO	0x0605	//音频下行订阅命令返回
#define   VIEWER_SUBSCRIBE_AUDIO_START	0x0606	//开始音频下行传送


//请求订阅上传音视频的命令结构
#define AV_TCP		1
#define AV_UDP    	2
#define AV_RTP_UDP	3
#define AV_QOS_UDP	4

struct viewer_subscribe_cmd_struct{//音视频传送订阅
	DWORD	devip;			//设备ip地址
	BYTE 	reserve[12];		//保留for ipv6
	BYTE	dev_id[8];		//前端设备id
	DWORD	gateip;			//网关ip
	BYTE	reserve1[12];	//保留
	BYTE	token[20];		//访问令牌，字符串
	WORD	user_level;		//用户访问级别
	BYTE	account[20];		//用户帐号
	WORD	protocal;		//音视频流的传送协议
	WORD	timeout;			//如果控件连接被断，转发服务器需要等待的时间，以秒为单位（如果设置为0，则使用转发服务器自定义的超时时间）
	WORD	bandwidth;		//用户网络带宽，以K为单位(本字段预留，如果不设定可将其清为0)
	DWORD   device_port;		//设备视频服务端口
	DWORD   audio_flag;		//是否订阅音频标志0x55aa表示不要音频,1表示要音频	
	DWORD    channel;          //用户连接编码器序号
	
};
//GT1000响应音视频传送命令的结构
struct viewer_subscribe_cmd_answer_struct{
	BYTE	dev_id[8];			//前端设备的id
	WORD	result;				//VIEWER_SUBSCRIBE_VIDEO命令的执行结果，0表示成功，其他值表示错误
	BYTE	video_trans_id[4];	//由转发服务器动态生成的传输事务ID号（如不成功为0）
	WORD	answer_data_len;	//返回信息answer_data的长度（成功时返回的时AVI文件头，失败返回错误信息）
	BYTE	answer_data[4];		//返回信息
};


#define AUDIO_PLAY_TYPE_MP3			0
#define AUDIO_PLAY_TYPE_PCM			1
#define AUDIO_PLAY_TYPE_ADPCM			2
#define AUDIO_PLAY_TYPE_MP2			4
#define AUDIO_PLAY_TYPE_UPCM			5
#define AUDIO_PLAY_TYPE_AAC				6 //for ip device
#define AUDIO_PLAY_TYPE_UNKNOW			255
static  char *get_audio_fmt_str(int type)
{
	switch(type)
	{
		case AUDIO_PLAY_TYPE_UPCM:
			return "u-pcm";
		break;
		case AUDIO_PLAY_TYPE_PCM:
			return "raw-pcm";
		break;
	}
	return "unknow";
}
//lc 2014-1-2
struct viewer_subscribe_audio_cmd_struct
{//音频下行订阅
	DWORD		device_ip;			//	前端设备ip地址
	BYTE		reserve2[12];		//	为ipv6保留的字段
	DWORD		device_port;			// 设备音频服务端口
	BYTE		dev_id[8];		       //	前端设备GUID
	DWORD		gateway_ip;			//	网关IP地址
	BYTE		reserve3[12];		//	为ipv6保留的字段
	BYTE		token[20];			//	访问令牌（字符串）
	WORD		user_level;			//	用户访问级别
	BYTE		account[20];	              //	用户帐号
	WORD		trans_protocal;   	//	传输协议（参见下文"传输协议定义"）
	WORD		timeout;				//	如果控件连接被断，转发服务器需要等待的时间，以秒为单位（如果设置为0，则使用转发服务器自定义的超时时间）
	WORD		bandwidth;			//	用户网络带宽，以K为单位(本字段预留，如果不设定可将其清为0)
	WORD		audiotype;			//	0-mp3; 1-pcm; 2-adpcm; 4-mp2 ,5-ulaw-pcm,255-未知
    WORD		audioheader_datalen;//音频文件头的长度；
	BYTE		audioheader_data[4];//audioheader_datalen）	音频文件头信息；
	WORD        audio_channel;		//音频下行通道号，新指令中增加
};



struct viewer_subscribe_answer_audio_struct
{//下行声音请求命令返回
	//应该加guid
	BYTE	dev_id[8];
	WORD	status;				//	返回操作状态（0表示成功，非0表示错误号）	
	BYTE	audio_trans_id[4];	//	由转发服务器动态生成的传输事务ID号（如不成功为0）
	WORD	answer_data_len;	//	返回信息answer_data的长度（成功时返回"success"，失败返回错误信息）
	BYTE	answer_data[2];			//(answer_data_len)	返回信息
};




/*

//实时图像传送命令的错误码

#define RT_RESULT_SUCCESS		0		//成功
#define ERR_DVC_INTERNAL		0x1001	//设备忙
#define ERR_DVC_INVALID_REQ	0x1002	//客户请求数据格式错
#define ERR_DVC_BUSY			0X1003	//设备故障

*/


#endif




