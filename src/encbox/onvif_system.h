#include "onviflib.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX_ONVIF_DEVICE 4
#define GT_SUCCESS 0
#define GT_FAIL    1
#define SECOND		(1*1000*1000)
typedef struct{

	video_mediainfo_t v_info;
	audio_mediainfo_t a_info;

}onvif_stream_t;
typedef enum{

	STATUS_INIT=0,
	STATUS_LOGIN,
	STATUS_MEDIA_CONNECED,
	STATUS_MEDIA_CONNECTING,
	STATUS_MEDIA_DISCON,

}status_e;

typedef struct{

	int channel;
	stream_type_t type; //stream type


}callback_parm_t;
typedef struct
{
	short up_port;    //server端接受上行数据端口 
	short cmd_port;   //ipdev 命令端口
	short audio_down_port; //ipdev接受下行数据的端口 
	char mac[6];  
	char guid[16];
	in_addr_t  ip_addr;
	in_addr_t  mask;
	int  is_up;      //正在上行
	int  is_down;    //正在对讲
	int  enc_no;    //内/外部编码器:-1 内(onvif)
	int  bits;      //采样精度
	int  channel;   //通道数
	int  sampling;  //采样率
	int enc_type;   
	int status;
	pthread_t thread_id;
	pthread_mutex_t mutex;
}audio_server_t;

typedef struct{

	char ip[24];
	dev_handler_t  token;                //already inited by onvif sdk ,initial -1
	int is_running;           //flag of star stream 
	status_e  ready_status;					//flag of on/offline
	int is_media_init;
	int channel;
	char username[20];
	char password[20];
	onvif_stream_t main_stream;
	onvif_stream_t sec_stream;
	pthread_mutex_t   mutex;
	pthread_t         thread_id;
	audio_server_t audio;

}onvif_device_t;

onvif_device_t * get_onvif_para(int no);
int init_media_system(void);
int init_onvif_system(void);

void create_onvif_device_thread(void);
void create_ip_device_thread(void);





void mod_probe_cb(dev_handler_t token,device_info_t * pinfo);
