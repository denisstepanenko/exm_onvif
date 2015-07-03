#ifndef MEDIA_SVR_H
#define MEDIA_SVR_H
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mshmpool.h>
#include <VEncApi.h>
//#include <AEncApi.h>

#define	MAX_VENC_NUM				2			//支持的最大视频编码器数目
#define 	MAX_AV_USER_NUM			8			//同时访问音视频最大用户数
#define	MAX_WAN_USER_NUM		1			//默认的公网访问用户数
#define	MAX_LAN_USER_NUM			2			//MAX_AV_USER_NUM-MAX_WAN_USER_NUM	//默认的局域网用户访问数
#define	MAX_BUF_FRAMES			300		//最大的缓冲贞数(用于分配缓冲区)
#define	DEFAULT_DROP_P			30			//默认的丢弃P贞的阈值
#define	DEFAULT_DROP_A			20			//默认的丢弃音频的阈值

typedef struct{
//缓冲音视频数据的结构
	int				Flag;		//标记，音频还是视频
	int				Size;			//数据的长度(包括头尾标记)
}BUF_FRAME_T;
typedef struct{
//网络发送缓冲区信息
	int					VFrames;				//缓冲区中的视频贞数
	int					AFrames;				//缓冲区中的音频包数
	int					Head;				//头位置(最老的数据)
	int					Tail;					//尾位置(放最新的数据的位置)
	//head==tail表示没有数据
	BUF_FRAME_T	Bufs[MAX_BUF_FRAMES];	//存放缓冲贞信息
}SEND_BUF_T;
typedef struct{
	pthread_mutex_t		Mutex;			//操作用户数据用到的互斥体
	pthread_t				ThreadId;		//接收用户命令的线程号
	int					No;				//用户编号
	int					Enable;			//用户有效标志
	int					SockOLen;		//socket发送缓冲区的大小
	int					NetFd;			// 网络连接描述符,如果NetFd<0表示该用户无效
//	int					NonBlock;		//使用非阻塞方式的socket
	int					ThTimeOut;		//判定超时的阈值
	int					TimeOut;		//接收数据的超时计数器
	struct sockaddr_in		Addr;			//远程连接的地址		
	char 				UsrName[32];	//用户名
	struct timeval 		ConnectStart;	//开始建立连接的时间
	struct timeval 		CmdStart;		//收到订阅服务的起始时间
	struct timeval		LastATime;		//最后发出的声音包时间
	struct timeval 		LastVTime;		//最后发出的视频包时间
	int					VEncNo;			//该用户需要的视频编码器通道
	int					FirstFlag;		//第一次进行通讯标志
	int					AudioFlag;		//是否需要声音 0表示不需要 1表示需要
	int					ThDropP;			//丢弃视频贞的阈值
	int					ThDropA;			//丢弃音频数据的阈值
	int					DropFlag;		//丢弃数据标志 0表示不需要丢弃
	int					DropAFlag;		//丢弃音频标志
	int					DropFrames;	//丢弃的视频贞数
	int					LastVSeq;		//上一包视频数据的序号
	int					LastASeq;		//上一包音频数据的序号

	unsigned long			BufBytes;		//放入缓冲区的字节数
	unsigned long			SendBytes;		//已发送的字节数
	unsigned long			SendBufBytes;	//发送缓冲区内的数据	
	SEND_BUF_T			SendBufInfo;		//发送缓冲区的信息
}AVUSR_TYPE;

typedef struct{//编码器属性结构
	int				EncType;	//编码器类型
	int				State;		//编码器状态  0:表示未初始化 1表示正常 2表示故障
}AENC_ATTRIB;

typedef union{
	ENC_ATTRIB		VEncAttrib;		//视频编码器的属性
	AENC_ATTRIB		AEncAttrib;		//音频编码器的属性
}ATTRIB_TYPE;
//媒体源结构
#define		MEDIA_TYPE_VIDEO		0
#define		MEDIA_TYPE_AUDIO		1
typedef struct{
	pthread_mutex_t	Mutex;
	int				MediaType;				//媒体类型
	int				No;						//资源编号
	int				DevState;				//-1表示还没有连接到具体的编码设备缓冲池 
	pthread_t			ThreadId;				//线程id
	int				MaxDataLen;			//该设备的数据块最大长度(动态刷新)
	MSHM_POOL		MPool;					//媒体用到的资源共享缓冲池
	void *			*ReadBuf;				//读取数据用的缓冲区(按DWORD对齐)
	int				BufLen;					//缓冲区长度,负值表示内存分配失败
	ATTRIB_TYPE		*Attrib;					//设备属性		
}MEDIA_SOURCE;
typedef struct{
	pthread_mutex_t	Mutex;						//操作数据结构用到的互斥体
	int				SvrPort;						//服务端口号
	int				MaxUsr;						//支持的最大用户数
	int				MaxWanUsr;					//最大广域网用户数
	int				MaxLanUsr;					//最大局域网用户数
	int				UsrNum;					//用户数
	int				WanUsrNum;					//广域网用户数量
	int				LanUsrNum;					//局域网用户数量
	MEDIA_SOURCE	VEnc[MAX_VENC_NUM];			//视频编码器
	MEDIA_SOURCE	AEnc;						
	AVUSR_TYPE		Users[MAX_AV_USER_NUM+1];	//多一个用于处理忙状态
}AVSERVER_TYPE;

int posix_memalign(void **memptr, size_t alignment, size_t size);
void MediaSvrSecondProc(void);
int CreateUsrThreads(int svr_port);
int SendAckPkt(int Fd,WORD Cmd,WORD Result,char *buf,int datalen);
#endif

