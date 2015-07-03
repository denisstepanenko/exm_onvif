#ifndef _IP_DEVICE_H
#define _IP_DEVICE_H


#define USR_PROBE_DEVICE		0x0101
#define PROBE_DEVICE_ACK		0x0801

#define USR_SET_IP				0x0102 
#define USR_SET_IP_ACK			0x0802

#define USR_SET_RATE            0x0103
#define USR_SET_RATE_ACK		0x0803

#define USR_SET_PRECISION       0x0104
#define USR_SET_PRECISION_ACK	0x0804

#define USR_SET_CHANNEL         0x0105
#define USR_SET_CHANNEL_ACK		0x0805

#define USR_SET_ENV				0x0106
#define USR_SET_ENV_ACK			0x0806

#define USR_REQUIRE_UP_AUDIO		0x0107
#define USR_REQUIRE_UP_AUDIO_ACK	0x0807

#define USR_STOP_UP_AUDIO		0x0117
#define USR_STOP_UP_AUDIO_ACK	0x0817

#define USR_SEND_DOWN_AUDIO		0x0109
#define USR_SEND_DOWN_AUDIO_ACK	0x0809

#define USR_STOP_DOWN_AUDIO		0x0118
#define USR_STOP_DOWN_AUDIO_ACK 0x0818

#define USR_QUERY_STATUS			0x0119
#define USR_QUERY_STATUS_ACK		0x0819

#define USR_PLAY_NOTICE_SOUND       0x010b
#define USR_PLAY_NOTICE_SOUND_ACK       0x080b

#define USR_PLAY_BUSY_SOUND         0x010c
#define USR_PLAY_BUSY_SOUND_ACK         0x080c



#define OK	1
#define ERR 2
#define MULTI_CAST_ADDR "225.0.0.1"
#define MULTI_CAST_PORT 3333
//#define HEAD "@@"
static const short head = 0x4040;
#define MAX_CMD_LEN 1024 
#define MAX_IP_DEVICE 16


typedef struct{
	short pkt_head;
	short pkt_len;
	short cmd;
	char reserve[2];
	char data[0];
}ipcall_pkt_t;

typedef struct{
    int audio_head;
	int audio_length;
   char data[0];
}audio_t;

typedef struct{
	struct in_addr		ip;
	unsigned char		guid[16];
	unsigned char		mac[6];
	unsigned short		cmd_port;
	unsigned short		audio_down_port;
	char                reserve1[2];
}probe_dev_ack_t;


typedef struct {	
	struct in_addr		ip;
	struct in_addr		mask;
	char                mac[6];
	char             reserve[2];
}set_ip_t;

typedef struct {		
	int		rate;	
}set_rate_t;

typedef struct {	
	int			precision;
}set_precision_t;

typedef enum{
	MONO=1,
	STEREO=2
}channel_e;

typedef struct {	
	channel_e			channel;
}set_channel_t;

typedef enum{
	G711U=1,
	PCM=2,
	AAC=3
}env_e ;

typedef struct {		
	env_e				env_type;	
}set_env_t;

typedef struct {
	int		listen_port;			
}require_up_audio_t;


typedef struct{
	int				status;
}ack_t;
#endif
