/** @file	tcprt_usr_info.h
 *  @brief 	描述tcprtimage2用户信息的结构定义
 *  @date 	2007.03
 */
#ifndef TCPRT_USR_INFO_H_20070301
#define TCPRT_USR_INFO_H_20070301
//系统头文件
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <pthread.h>
#include <soundapi.h>

////

/** @struct socket_attrib_t
 *   @brief  描述socket属性的结构 
 */
typedef struct{
	int	send_buf_len;	        ///<发送缓冲区大小
	int	recv_buf_len;	        ///<接收缓冲区大小

	int	 send_buf_remain;	  ///<发送缓冲区(socket)当前的字节数(使用ioctl从socket读出缓冲区内未发送出去的字节数)
	int    send_buffers;             ///<放入发送缓冲区中的字节数(write时增加,输出给网络后减少,这个值应该永远小于send_buf_len)

       int    recv_buf_remain;      ///<接收缓冲区内当前字节数

}socket_attrib_t;


/////////////////////////音视频上行服务用到的结构定义/////////////////////
/** @struct map_frame_t
 *   @brief  stream_send_map_t中描述一帧音视频数据的结构 
 */
typedef struct{
	int	flag;		///<标记 音频或视频
	int	size;		///<此帧的大小
}map_frame_t;

/** @struct stream_send_map_t
 *   @brief  发送媒体数据的socket缓冲区映像描述结构 
 */
typedef struct{
	int	              v_frames;			                          ///>缓冲区map中的视频帧数
	int	              a_frames;			                          ///>缓冲区map中的音频帧数
	int	              head;				                          ///>缓冲区队首位置
	int	              tail;				                                 ///>缓冲区队尾位置,head=tail表示没有map中没有元素
	map_frame_t	frame_map[MAX_MAP_BUF_FRAMES];    ///>发送socket缓冲区的帧结构	
}stream_send_map_t;

/** @struct stream_send_info_t
 *   @brief  发送媒体数据的描述信息结构 
 */
typedef struct{
    int    first_flag;                        ///<首次发送视频数据标志
    int    send_ack_flag;               ///<已发送响应标志 0表示未发送 非0表示已发送
	int	send_i_flag;		            ///<已发送I帧标志
	int require_i_flag;                 //已请求i帧标志
    int    drop_p_flag;                  ///<开始丢弃P帧数据的标志
	int	drop_v_flag;	            ///<开始丢弃视频数据标志,0表示不需要丢弃
	int	drop_a_flag;	            ///<开始丢弃音频数据标志,0表示不需要丢弃
	int	drop_v_frames;	            ///<丢弃的视频帧数
	int	drop_a_frames;	            ///<丢弃的音频帧数

	int	last_v_seq;		            ///<上一帧视频的序号
	int 	last_a_seq;		            ///<上一帧音频的序号

       int     total_put;                   ///<总共放入发送缓冲区中的字节数
       int     total_out;                  ///<总共发送出去的字节数 
       int 		jump_flag;					//设备回放跳转的标志
       stream_send_map_t    map;     ///<发送的媒体信息映像
}stream_send_info_t;

/** @struct av_usr_t
 *  @brief  描述远程音视频上行服务连接用户信息的结构 
 */
typedef struct{
	unsigned long		         magic;			///<用户结构信息魔数,0x55aa表示已经初始化,其它值表示未初始化
	pthread_mutex_t            u_mutex;		///<访问用户数据时需要的互斥体
	pid_t			         pid;			///<pid 用户子进程号
       pthread_t                       thread_id;         ///<线程id
	int			                no;			///<用户序号
	struct sockaddr_in	         addr;			///<远程用户地址
	int			                fd;				///<用户连接描述符,负值表示未连接
	int			                th_timeout;	///<判定超时的阈值(连接后长时间没有收到命令或长时间发不出数据),以秒为单位
	int	                              th_drop_p;		///<开始丢弃p帧的阈值
	int	                              th_drop_v;		///<开始丢弃所有视频的
	int	                              th_drop_a;		///<开始丢弃音频数据的阈值
	int			                timeout_cnt;	///<计算超时的计数器
	struct timeval 		  start_time;		///<连接的起始时间;
	struct timeval 		  last_cmd_time;	///<最后一次收到命令的时间
	char			                name[40];		///<订阅服务的用户名	
	int			                serv_stat;		///<服务状态,0:没有收到请求服务命令 1:已订阅了视频服务 3:既订阅了视频服务也订阅了音频服务
	int			                venc_no;		///<用户请求连接的视频编码器编号
	int			                aenc_no;		///<用户请求连接的音频编码器编号(目前只有一个音频编码器,所以此参数无效)
	socket_attrib_t		  sock_attr;		///<socket属性
	stream_send_info_t	  send_info;		///<发送媒体数据的信息结构
	int                   stream_idx;       //新增加的sdk端idx
	int 				used;

}av_usr_t;
//////////////////////////////////////////////////////////////////////////////////////////////////////////




/////////////////////////音频下行服务用到的结构定义/////////////////////


typedef struct{
    int                                    total_recv;          ///<总共收到的字节数
    int                                    total_play;          ///<总共播放的字节数
    int                                    play_buf_used;  ///<已经使用的播放缓冲区字节数
}stream_recv_info_t;

/** @struct aplay_usr_t
 *   @brief  描述远程音频下行服务连接用户信息的结构 
 */
typedef struct{
    unsigned long                    magic;                ///<用户结构信息魔数,0x55aa表示已经初始化,其它值表示未初始化
    //snd_dev_t                         *play_dev;        ///<音频播放设备指针
    int                           play_dev;
    pthread_mutex_t               u_mutex;		///<访问用户数据时需要的互斥体
    pid_t			                pid;			///<pid 用户子进程号
    pthread_t                          thread_id;         ///<线程id
    int			                no;			///<用户序号
    struct sockaddr_in	         addr;			///<远程用户地址
    int			                fd;				///<用户连接描述符,负值表示未连接
    int			                th_timeout;	///<判定超时的阈值(连接后长时间没有收到命令或长时间发不出数据),以秒为单位
    int			                timeout_cnt;	///<计算超时的计数器
    struct timeval 		         start_time;		///<连接的起始时间;
    struct timeval 		         last_cmd_time;	///<最后一次收到命令的时间
    char			                name[40];		///<订阅服务的用户名	
    int			                serv_stat;		///<服务状态,0:没有收到请求服务命令 1:已经收到音频订阅命令,准备提供服务 负值:还没有连接
    int			                aenc_no;		///<用户请求连接的音频编码器编号(目前只有一个音频编码器,所以此参数无效)
    int                                     a_fmt;               ///<音频格式AUDIO_PLAY_TYPE_UPCM,AUDIO_PLAY_TYPE_PCM,
    int                                     a_sam_rate;     ///<音频采样率
    socket_attrib_t		         sock_attr;		///<socket属性
    stream_recv_info_t            recv_info;          ///<接收芯片数据的信息结构
}aplay_usr_t;



///////////////////////////////////////////////////////////////////////////////////////

#endif

