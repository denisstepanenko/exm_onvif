/** @file	serv_info.h
 *  @brief 	tcprtimage2模块的内部参数数据结构定义
 *  @date 	2007.03
 */
#ifndef SERV_INFO_H_20070302
#define SERV_INFO_H_20070302
////
#include <devinfo.h>
#include <media_api.h>

#include "tcprt_usr_info.h"

/** @struct av_server_t
 *  @brief  描述远程音视频上行服务信息的数据结构
 */
typedef struct{
    pthread_mutex_t     l_mutex;                                         ///<侦听用的互斥体
	pthread_mutex_t     s_mutex;                                        ///<访问音视频上行服务用的互斥体
	int		                lan_usrs;				               ///<当前的局域网用户数
	int		                wan_usrs;				               ///<当前的广域网用户数
    int                          listen_fd;                                        ///<侦听服务的描述符
	media_source_t 	  video_enc[MAX_VIDEO_ENCODER]; ///<视频编码器实例
	int				vir0_lan_users;					///<当前虚拟设备0局域网用户数
	int 				vir0_wan_users;					///<当前虚拟设备0广域网用户数
	int				vir1_lan_users;					///<当前虚拟设备1局域网用户数
	int				vir1_wan_users;					///<当前虚拟设备1广域网用户数
	av_usr_t	av_usr_list[TCPRTIMG_MAX_AVUSR_NO+1];	 ///<音视频上行服务用户信息列表,多一个用于处理忙
}av_server_t;

/** @struct aplay_server_t
 *   @brief  描述远程音频下行服务信息的数据结构
 */
typedef struct{
    pthread_mutex_t     l_mutex;                                                             ///<侦听用的互斥体
	pthread_mutex_t     s_mutex;                                                            ///<访问音视频上行服务用的互斥体??下行
	int		            usrs;				                                          ///<当前访问音频下行服务的用户数
    int					listen_fd;                                                           ///<侦听服务的描述符
	aplay_usr_t	        usr_list[TCPRTIMG_MAX_APLAY_NO+1];	              ///<音视频上行服务用户信息列表,多一个用于处理忙

}aplay_server_t;

/** @struct tcprtimg_svr_t
 *  @brief  tcprtimage2的所有参数和信息结构描述
 */
typedef struct{
	int		         mic_gain;		    ///<mic增益
	float            ain0_gain;			
	float 			 ain1_gain;
	float            ain2_gain;
	float            ain3_gain;
	float		     audio_gain;          ///<音频输出增益
	int		         max_lan_usrs;	   ///<最大局域网用户数
	int		         max_wan_usrs;	   ///<最大广域网用户数
	int              max_aplay_usrs; ///<音频下行服务最大用户数
	int		         av_svr_port;	   ///<音视频上行服务端口
	int		         audio_play_port; ///<音频下行服务端口
       
    int              audio_pkt_size;    ///<音频上行数据包大小	
	int		         th_timeout;	   ///<长时间没有数据交互时判断超时的时间
	int		         th_drop_p;		   ///<缓冲池数据多后开始丢视频p帧的阈值
	int		         th_drop_v;		   ///<缓冲池数据多后开始丢弃所有视频的阈值
	int		         th_drop_a;		   ///<开始丢弃所有数据的阈值
	in_addr_t         eth0_addr;           ///<网口0的局域网地址
	in_addr_t         eth0_mask;          ///<网口0的子网掩码
	in_addr_t         eth1_addr;           ///<网口0的局域网地址
	in_addr_t         eth1_mask;          ///<网口0的子网掩码       
	av_server_t	  	  av_server;           ///<音视频上传服务
	aplay_server_t    aplay_server;      ///<音频下行服务   
///for 3022
	int                   virdev_num;	  ///<设备中虚拟设备个数
////zw-20091229 为了对付的cdma，只要双向语音不要图像
	struct sockaddr_in     audio_only_usr_addr[6];	///<只听得标志,0又听又看，1为只听不看

	int                 pkts_limit;                 ///<网卡缓冲区大小切换的阀值
	int                 tcp_max_buff;               ///<缓冲区大小
	int			        playback_pre;		///报警前的录像回放时间
	int			        playback_dly;		///报警后的录像回放时间
	int 			frame_rate;
	int            bitratecon;
	int            targetbitrate;
	int            maxbitrate;


        
}tcprtimg_svr_t;




#endif

