#include <mod_com.h>
#include <gt_com_api.h>
#include <devinfo.h>
#include "process_modcmd.h"
#include "videoencoder.h"
#include <unistd.h>
#include "commonlib.h"
#include "devres.h"
#include "gtthread.h"
#include "gtlog.h"
//#include <gt_errlist.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "ip_device.h"
#include "onvif_system.h"





//static pthread_mutex_t  state_mutex=PTHREAD_MUTEX_INITIALIZER;	//用于保护访问状态的互斥体

static int	com_fd= -1; //发送和接收命令的udp socket
static pthread_t modsocket_thread_id=-1;
static probe_dev_ack_t tmp_device[MAX_IP_DEVICE];
extern onvif_device_t temp_onvif_device[MAX_ONVIF_DEVICE*2];


//初始化与主进程通讯的命令通道
int init_com_channel(void)
{
	com_fd	=	mod_socket_init(0,0);	
	return 0;
}

//static DWORD old_enc_state=0;

/*
//设置第ch通道的视频丢失，若有改变则发送给主进程
int set_video_loss(int ch,int flag)
{

	struct videoenc_state_struct *videoencstate;
	videoencstate=get_videoenc_state();
	
	pthread_mutex_lock(&state_mutex);
	switch(ch)
		{
			case 0:	
					videoencstate->video_loss0=flag;
					break;
			case 1:	
					videoencstate->video_loss1=flag;
					break;
			case 2:	
					videoencstate->video_loss2=flag;
					break;
			case 3:	
					videoencstate->video_loss3=flag;
					break;
			default:
					break;
		}
	if (get_videoencstatint()==old_enc_state)
	{
		pthread_mutex_unlock(&state_mutex);
		return 0;
	}
	old_enc_state=get_videoencstatint();
	pthread_mutex_unlock(&state_mutex);
	send_state2main();
	return 0;
}
int set_video_motion_blind(int ch,int motion, int blind)
{

	struct videoenc_state_struct *videoencstate;
	videoencstate=get_videoenc_state();
	

	pthread_mutex_lock(&state_mutex);
	switch(ch)
		{//没有视频则不会报移动侦测
			case 0:	
					if(videoencstate->video_loss0)
					{	
						videoencstate->video_motion0=0;
						videoencstate->video_blind0=0;
					}
					else
					{
						videoencstate->video_motion0=motion;
						videoencstate->video_blind0=blind;
					}
					break;
			case 1:	
					if(videoencstate->video_loss1)
					{	
						videoencstate->video_motion1=0;
						videoencstate->video_blind1=0;
					}
					else
					{
						videoencstate->video_motion1=motion;
						videoencstate->video_blind1=blind;
					}
					break;
			case 2:	
					if(videoencstate->video_loss2)
					{	
						videoencstate->video_motion2=0;
						videoencstate->video_blind2=0;
					}
					else
					{
						videoencstate->video_motion2=motion;
						videoencstate->video_blind2=blind;
					}
					break;
			case 3:	
					if(videoencstate->video_loss3)
					{	
						videoencstate->video_motion3=0;
						videoencstate->video_blind3=0;
					}
					else
					{
						videoencstate->video_motion3=motion;
						videoencstate->video_blind3=blind;
					}
					break;
			default:
					break;
		}
	
	if (get_videoencstatint()==old_enc_state)
	{
	
		pthread_mutex_unlock(&state_mutex);
		return 0;
	}
	//printf("set motion %d, blind %d\n",motion,blind);
	old_enc_state=get_videoencstatint();
	pthread_mutex_unlock(&state_mutex);
	send_state2main();
	return 0;
}




//设置第encno个编码器的错误状态，若有改变则发送给主进程
int set_enc_error(int encno,int flag)
{
	DWORD state;
	struct videoenc_state_struct *videoencstate=NULL;
	videoencstate=get_videoenc_state();
	pthread_mutex_lock(&state_mutex);
	switch(encno)
		{
			case 0:	
					videoencstate->video_enc0_err=flag;
					break;
			case 1:	
					videoencstate->video_enc1_err=flag;
					break;
			case 2:	
					videoencstate->video_enc2_err=flag;
					break;
			case 3:	
					videoencstate->video_enc3_err=flag;
					break;
			case 4:	
					videoencstate->video_enc4_err=flag;
					break;
			default:
					break;
		}
	state=get_videoencstatint();
	if (state==old_enc_state)
	{
		pthread_mutex_unlock(&state_mutex);
		return 0;
	}
	old_enc_state=state;
	pthread_mutex_unlock(&state_mutex);
	send_state2main();
	return 0;
}



*/




//发送videoenc状态给主进程
int send_state2main(void)
{
	DWORD *state;	
	DWORD socketbuf[20];
	mod_socket_cmd_type *cmd;
	pid_t *pid;
	
	cmd=(mod_socket_cmd_type *)socketbuf;
	cmd->cmd	=	VIDEOENC_STATE_RETURN;
	cmd->len	=	4+sizeof(pid_t);
	pid=(pid_t*)cmd->para;
	*pid=getpid();
	state=(DWORD*)&cmd->para[sizeof(pid_t)];
	*state=get_videoencstatint();
	return mod_socket_send(com_fd,MAIN_PROCESS_ID,VIDEOENC_MOD_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);
}






/*************************************************************************
 * 以下是主进程转发过来的网关命令
*************************************************************************/


//处理由主进程转发过来的网关命令
static int process_gate_cmd(struct gt_usr_cmd_struct *cmd,gateinfo *gate)
{
	switch(cmd->cmd)
	{
		case USR_LOCAL_STREAM_SETING:// 设定高清晰路像质量
			gtloginfo("recv a USR_LOCAL_STREAM_SETING cmd!\n");
		break;
		case USR_NET_STREAM_SETING:// 设定网络传输图像质量
			gtloginfo("recv a USR_NET_STREAM_SETING cmd!\n");
		break;
		case USR_SET_VIDEO_AD_PARAM:// 视频转换参数设置
			gtloginfo("recv a USR_SET_VIDEO_AD_PARAM cmd!\n");
		break;
		default:
			printf("videoenc recv a unknow bypass cmd 0x%04x\n",cmd->cmd);
			gtloginfo("videoenc recv a unknow bypass cmd 0x%04x\n",cmd->cmd);			
			send_ack_to_main(com_fd,VIDEOENC_MOD_ID,cmd->cmd,ERR_EVC_NOT_SUPPORT,gate);
		break;
	}
	return 0;
}



/*************************************************************************
 * 以上是主进程转发过来的网关命令
*************************************************************************/





	

void request_notice(int no);

void  process_request_aplay(int no)
{
	
	int is_down=get_onvif_para(no)->audio.is_down;
	if(is_down==0){				//没对讲

		request_notice(no);
		gtloginfo("[%d]路播放提示音\n",no);

	}else         //正在对讲

		gtloginfo("[%d]路正在对讲,就不播放声音了\n",no);


}
				
void process_cmd(char * data,int size,short cmd,probe_dev_ack_t *device)
{

	probe_dev_ack_t *probe_ack=(probe_dev_ack_t*)data;
	ack_t * ack=(ack_t *)data;

			
	switch(cmd)
	{
		case USR_PROBE_DEVICE+0x0700:
			printf("ip:%s\n",inet_ntoa(probe_ack->ip));
			printf("guid:%s\n",probe_ack->guid);
			printf("mac:%x%x%x%x%x%x\n",probe_ack->mac[0],probe_ack->mac[1],probe_ack->mac[2],probe_ack->mac[3],probe_ack->mac[4],probe_ack->mac[5]);
			printf("cmd_port:%d\n",probe_ack->cmd_port);
			printf("audio_down_port:%d\n",probe_ack->audio_down_port);
			memcpy(&device->ip,&probe_ack->ip,sizeof(struct in_addr));		
			memcpy(&device->guid,&probe_ack->guid,16);		
			memcpy(&device->mac,&probe_ack->mac,6);		
			memcpy(&device->cmd_port,&probe_ack->cmd_port,2);		
			memcpy(&device->audio_down_port,&probe_ack->audio_down_port,2);		
			break;
		case USR_SET_IP+0x0700:
			printf("receive USR_SET_IP_ACK status:%d\n",ack->status);
			break;
		case USR_SET_RATE+0x0700:
			printf("receive USR_SET_RATE_ACK status:%d\n",ack->status);
			break;
		case USR_SET_PRECISION+0x0700:
			printf("receive USR_SET_PRECISION_ACK status:%d\n",ack->status);
			break;
		case USR_SET_CHANNEL+0x0700:
			printf("receive USR_SET_CHANNEL_ACK status:%d\n",ack->status);
			break;
		case USR_SET_ENV+0x0700:
			printf("receive USR_SET_ENV_ACK status:%d\n",ack->status);
			break;
		case USR_REQUIRE_UP_AUDIO+0x0700:
			printf("receive USR_REQUIRE_UP_AUDIO_ACK status:%d\n",ack->status);
			//gtloginfo("receive USR_REQUIRE_UP_AUDIO_ACK status:%d\n",ack->status);
		case USR_STOP_UP_AUDIO+0x0700:
			printf("receive USR_STOP_UP_AUDIO_ACK status:%d\n",ack->status);
			//gtloginfo("receive USR_STOP_UP_AUDIO_ACK status:%d\n",ack->status);
			break;
		case USR_SEND_DOWN_AUDIO+0x0700:
			printf("receive USR_SEND_DOWN_AUDIO_ACK status:%d\n",ack->status);
			break;
		case USR_STOP_DOWN_AUDIO+0x0700:
			printf("receive USR_STOP__DOWN_AUDIO_ACK status:%d\n",ack->status);
			break;
		
		case USR_QUERY_STATUS+0x0700:
			printf("receive USR_QUERY_STATUS_ACK status:%d\n",ack->status);
			break;
		default:
			printf("UNKNOWN CMD!!\n");
			break;
	}

}
int  process_pkt(int fd,char * data,int * size,short * cmd,struct sockaddr_in * addr)
{
	int len;
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	struct sockaddr_in client_addr;
	unsigned int length=sizeof(client_addr);
	char buffer[MAX_CMD_LEN];	
	if(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))<0)
	{
		printf("socket option  SO_RCVTIMEO not support\n");
		return ERR;
	}
	
	
	len=recvfrom(fd,buffer,MAX_CMD_LEN,0,(struct sockaddr*)addr,&length);
	if(len<6)
	{
		perror("recvfrom error");
		//gtloginfo("recvfrom ERR len[%d]\n",len);
		return ERR;
	}
	ipcall_pkt_t *p=(ipcall_pkt_t *)buffer;
	if(p->pkt_head!=head)
	{
		printf("head error:%#x\n",p->pkt_head);
		gtloginfo("recvfrom HEAD ERR[%d]\n",p->pkt_head);
		return ERR;	
	
	}
	*size=p->pkt_len;
	memcpy(data,buffer+8,p->pkt_len);
	*cmd=p->cmd;
	printf("*cmd=%#x\n",*cmd);
	return OK;

}
int exchange_cmd(ipcall_pkt_t *pkt,int no,short cmd)
{
	struct sockaddr_in peer,serv;
	int size;
	int cmdfd;
	short ret_cmd;
	char data[MAX_CMD_LEN]={0};
	pkt->pkt_head=head;
	pkt->cmd=cmd;
	probe_dev_ack_t * device=tmp_device;
	audio_server_t *audio=&get_onvif_para(no)->audio;

	memcpy(pkt->data,&audio->up_port,sizeof(short));
	pkt->pkt_len=8;


	if((cmdfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
	{
		perror("Creating cmdfd   failed.");
		return ERR;
	}
	bzero(&peer,sizeof(peer));
	peer.sin_family=AF_INET;
	peer.sin_port=htons(audio->cmd_port);
	memcpy(&peer.sin_addr.s_addr,&audio->ip_addr,sizeof(in_addr_t));
	//gtloginfo("require ip[%s]		port[%d]	up_port[%d]\n",inet_ntoa(&peer.sin_addr.s_addr),audio->cmd_port,audio->up_port);
	//printf("require ip[%s]		port[%d]	up_port[%d]\n",inet_ntoa(&peer.sin_addr.s_addr),audio->cmd_port,audio->up_port);
	int addrlen=sizeof(peer);
	size=sendto(cmdfd,pkt,sizeof(ipcall_pkt_t)+pkt->pkt_len-4,0,(struct sockaddr *)&peer,addrlen);
	if(size<0)
	{
		perror("write error");
		close(cmdfd);
		return ERR;
		
	}
	while(1)
	{
	
		if(process_pkt(cmdfd,data,&size,&ret_cmd,&serv)==ERR)
			break;
		process_cmd(data,size,ret_cmd,device);
		close(cmdfd);
		return OK ;//收到一条信令并返回
	}
    close(cmdfd);
	return ERR;//超时没收到信令
}

void set_aenc_status(int no,int status)
	
{
	/*
	switch(no)
	{
		case 0:
			  venc_status.audio_enc0_err=status;
			  break;
		case 1:
			  venc_status.audio_enc1_err=status;
			  break;
		
		case 2:
			  venc_status.audio_enc2_err=status;
			  break;
		
		case 3:
			  venc_status.audio_enc3_err=status;
			  break;
		
		default:
			  break;
	}
	*/


}
int  get_aenc_status(int no)
	
{
	/*
	int status;
	switch(no)
	{
		case 0:
			  status=venc_status.audio_enc0_err;
			  break;
		case 1:
			  status= venc_status.audio_enc1_err;
			  break;
		
		case 2:
			  status=venc_status.audio_enc2_err;
			  break;
		
		case 3:
			  status=venc_status.audio_enc3_err;
			  break;
		
		default:
			  break;
	}
	return status;
	*/
	return 0;


}

int require_up_audio(int no)
{
	char buf[20];
	short cmd = USR_REQUIRE_UP_AUDIO;
	printf("%s no=%d\n",__FUNCTION__,no);
	if(no<0||no>=MAX_IP_DEVICE)
		return 0;

	audio_server_t *audio=&get_onvif_para(no)->audio;

	if(audio->enc_no==-1)//内置编码器不发送信令
		return 0;
	audio->is_up=1;

	if(exchange_cmd((ipcall_pkt_t *)buf,no,cmd)==ERR)
	{
		set_aenc_status(no,1);
	
		return -1;

	}
	else
	{
		set_aenc_status(no,0);
		gtloginfo("%s [%d]return OK",__FUNCTION__,no);
		
		return 0;
	}
	


	
}

int stop_up_audio(int no)
{
	char buf[20];
	short cmd =USR_STOP_UP_AUDIO;
	audio_server_t *audio=&get_onvif_para(no)->audio;
	if(audio->enc_no==-1)
		return 0;

	audio->is_up=0;
	if(exchange_cmd((ipcall_pkt_t *)buf,no,cmd)==ERR)
	{
		set_aenc_status(no,1);
		return -1;
	}

	else
	{
		set_aenc_status(no,0);
		return 0;
	}
	


	
}

int send_down_audio_ack(char no,char status)
{

	DWORD socketbuf[20];
	mod_socket_cmd_type *cmd; 
    cmd=(mod_socket_cmd_type *)socketbuf;
    cmd->cmd    =   SEND_DOWN_AUDIO_ACK;
    cmd->len    =   sizeof(int);
	cmd->para[0]=(char)no;
	cmd->para[1]=(char)status;
	return mod_socket_send(com_fd,RTIMAGE_PROCESS_ID,VIDEOENC_MOD_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);

}
int send_down_audio(char no,char mode)
{

	char buf[20];
	int ret=GT_SUCCESS;
	short cmd =USR_SEND_DOWN_AUDIO;
	audio_server_t *audio=&get_onvif_para(no)->audio;
	
	printf("正常请求 [%d]\n",no);

	if(no>=MAX_IP_DEVICE)
	{
		ret=GT_FAIL;
		goto SUBSCRIBE_FAIL;
	}

	if(audio->enc_no==-1) //内置编码器不发送信令
	{
		ret=GT_FAIL;
		goto NOT_SUPPORT;
	}

	if(audio->is_down==1) //正在下行语音
	{
		ret=GT_FAIL;
		goto BUSY;
	}
	audio->is_down=1;
	if(exchange_cmd((ipcall_pkt_t *)buf,no,cmd)==ERR)
	{
		set_aenc_status(no,1);
		gtloginfo("%s [%d]return ERR",__FUNCTION__,no);
		ret=GT_FAIL;
		goto SUBSCRIBE_FAIL;
	}
	else
	{
		set_aenc_status(no,0);
		gtloginfo("%s [%d]return OK",__FUNCTION__,no);
		ret=GT_SUCCESS;
	}
	send_down_audio_ack(no,0); 
	return ret;

NOT_SUPPORT:
	send_down_audio_ack(no,2);
	return ret;
BUSY:
	send_down_audio_ack(no,3);
	return ret;
SUBSCRIBE_FAIL:
	send_down_audio_ack(no,4);
	return ret;






		
}

int stop_down_audio(int no)
{

	char buf[20];	
	short cmd =USR_STOP_DOWN_AUDIO;
	if(no>=MAX_IP_DEVICE)
		return 0;
	audio_server_t *audio=&get_onvif_para(no)->audio;
	if(audio->enc_no==-1)
		return 0;
	audio->is_down=0;
	if(exchange_cmd((ipcall_pkt_t *)buf,no,cmd)==ERR)
	{
		set_aenc_status(no,0);
		return -1;
	}
	else
	{
		set_aenc_status(no,1);
		return 0;
	}
		
	
}

void check_aenc_status(int no)
{
	/*
	char buf[20];
	short cmd =USR_QUERY_STATUS;
	audio_server_t *audio=&get_para()->audio[no];
	if(exchange_cmd((ipcall_pkt_t *)buf,no,cmd)==ERR)
		set_aenc_status(no,1);
	else
		set_aenc_status(no,0);
		*/


}
void request_notice(int no)
{
	char buf[20];
	short cmd =USR_PLAY_NOTICE_SOUND;
	if(exchange_cmd((ipcall_pkt_t *)buf,no,cmd)==ERR)
		set_aenc_status(no,1);
	else
		set_aenc_status(no,0);


}
void request_busy(int no)
{
	char buf[20];
	short cmd =USR_PLAY_BUSY_SOUND;
	if(exchange_cmd((ipcall_pkt_t *)buf,no,cmd)==ERR)
		set_aenc_status(no,1);
	else
		set_aenc_status(no,0);


}
int  echo_str(unsigned char * dst,char * src)
{
	int ret=0;
	ret=sprintf((char *)dst,src);
	return ret;
}
int process_probe_device(void)
{
	int size;
	int fd;
	ipcall_pkt_t probe;
	struct sockaddr_in multi_addr;
	struct ip_mreq mreq;
	short cmd;	
	int i=0;
	int ret=0;
	char sendbuf[MAX_MOD_SOCKET_CMD_LEN]={0};
	
	probe_dev_ack_t * device=tmp_device;
	fd=socket(AF_INET,SOCK_DGRAM,0);
	if(fd<0)
	{
	    perror("socket error");
	    return ERR;
	}
	multi_addr.sin_family=AF_INET;
	multi_addr.sin_port=htons(MULTI_CAST_PORT);
	multi_addr.sin_addr.s_addr=inet_addr(MULTI_CAST_ADDR);
	
	if((bind(fd,(struct sockaddr *)&multi_addr,sizeof(multi_addr))==-1))
	{
	
		perror("bind error");
		return ERR;
	
	}
	mreq.imr_multiaddr.s_addr=inet_addr(MULTI_CAST_ADDR);
	mreq.imr_interface.s_addr=htonl(INADDR_ANY);
	int loop=1;
	if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&loop,sizeof(int))<0)
	{
		perror("setsockopt reuseaddr err");
		return ERR;
	
	}
	loop=0;
	if(setsockopt(fd,IPPROTO_IP,IP_MULTICAST_LOOP,&loop,sizeof(int))<0)
	{
		perror("setsockopt loop err");
		return ERR;
	
	}
	if(setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq))<0)
	{
	

		perror("setsockopt membership err");
		return ERR;
	
	}
	char data[MAX_CMD_LEN];
	memset(data,0,sizeof(data));
	memset(&probe,0,sizeof(probe));
	probe.pkt_head=head;
	probe.cmd=USR_PROBE_DEVICE;
	probe.pkt_len=4;
	size=sendto(fd,&probe,sizeof(probe),0,(struct sockaddr*)&multi_addr,sizeof(multi_addr));
	if(size<0)
	{
		perror("sendto error");
		gtloginfo("encbox send probe device err\n");
		close(fd);
		return ERR;
		
	}
	while(i<MAX_IP_DEVICE)
	{
		if(process_pkt(fd,(char*)&data,&size,&cmd,&multi_addr)==ERR)
		{
			break;
		}
		process_cmd((char *)&data,size,cmd,device+i);
		i++;
	}
	printf("I GOT %d IP DEVICE(S)\n",i);
	gtloginfo("I GOT %d IP DEVICE(S)\n",i);
    
    close(fd);
	mod_socket_cmd_type *send_cmd=(mod_socket_cmd_type *)&sendbuf;//发送命令
	send_cmd->cmd=PROBE_IP_ACK;
	int j;
	ret+=echo_str(send_cmd->para+ret,"<html>");
	ret+=echo_str(send_cmd->para+ret,"<META content=\"text/html; charset=gb2312\" http-equiv=Content-Type>");
	ret+=echo_str(send_cmd->para+ret,"<body>");
	ret+=echo_str(send_cmd->para+ret,"<h1>设备信息：</h1>");
	ret+=echo_str(send_cmd->para+ret,"<table border=\"1\">");
	ret+=echo_str(send_cmd->para+ret,"<tr>");
	ret+=echo_str(send_cmd->para+ret,"<td>ip地址</td>");
	ret+=echo_str(send_cmd->para+ret,"<td>GUID</td>");
	ret+=echo_str(send_cmd->para+ret,"<td>MAC</td>");
	ret+=echo_str(send_cmd->para+ret,"<td>命令端口</td>");
	ret+=echo_str(send_cmd->para+ret,"<td>下行端口</td>");
	ret+=echo_str(send_cmd->para+ret,"</tr>");
		
	for(j=0;j<i;j++)
	{
		ret+=echo_str(send_cmd->para+ret,"<tr>");
		ret+=sprintf((char*)send_cmd->para+ret,"<td>%s</td>",inet_ntoa(device->ip));
		ret+=sprintf((char*)send_cmd->para+ret,"<td>%s</td>",device->guid);
		ret+=sprintf((char*)send_cmd->para+ret,"<td>%02x:%02x:%02x:%02x:%02x:%02x</td>",device->mac[0],device->mac[1],device->mac[2],device->mac[3],device->mac[4],device->mac[5]);
	
		ret+=sprintf((char*)send_cmd->para+ret,"<td>%d</td>",device->cmd_port);
		ret+=sprintf((char*)send_cmd->para+ret,"<td>%d</td>",device->audio_down_port);
		ret+=echo_str(send_cmd->para+ret,"</tr>");
		

		device++;

	}
	sprintf((char*)send_cmd->para+ret, "</table> </body> </html>");

	send_cmd->len=ret;
	printf("ret=%d\n",ret);
	return mod_socket_send(com_fd,CGI_MAIN_ID,VIDEOENC_MOD_ID, send_cmd, sizeof(mod_socket_cmd_type)-4+ret);

}
extern int temp_onvif_count;
int process_probe_onvif_device(void)
{
	
	int i;
	int ret=0;
	char sendbuf[MAX_MOD_SOCKET_CMD_LEN]={0};
	mod_socket_cmd_type *send_cmd=(mod_socket_cmd_type *)&sendbuf;//发送命令
	struct timeval tv;
	tv.tv_sec=2;
	tv.tv_usec=0;
	int err;

	
	temp_onvif_count=0; //每次初始值设为0
	memset(&temp_onvif_device,0,sizeof(temp_onvif_device));

	if(onvif_lib_start_probe(mod_probe_cb,NULL)==-1)
		goto BUSY;

	onvif_device_t * device=temp_onvif_device;

	//设置2秒超时
	do{
		err=select(0,NULL,NULL,NULL,&tv);
	}while(err<0 && errno==EINTR);

	send_cmd->cmd=PROBE_ONVIF_DEV_ACK;
	ret+=echo_str(send_cmd->para+ret,"<html>");
	ret+=echo_str(send_cmd->para+ret,"<META content=\"text/html; charset=gb2312\" http-equiv=Content-Type>");
	ret+=echo_str(send_cmd->para+ret,"<body>");
	ret+=echo_str(send_cmd->para+ret,"<h1>ONVIF设备信息：</h1>");
	ret+=echo_str(send_cmd->para+ret,"<table border=\"1\">");
	ret+=echo_str(send_cmd->para+ret,"<tr>");
	ret+=echo_str(send_cmd->para+ret,"<td>ip地址</td>");
	ret+=echo_str(send_cmd->para+ret,"</tr>");
		
	for(i=0;i<temp_onvif_count;i++)
	{
		if(device->ip!=NULL)
		ret+=sprintf((char*)send_cmd->para+ret,"<tr>");
		ret+=sprintf((char*)send_cmd->para+ret,"<td>%s</td>",device->ip);
		ret+=echo_str(send_cmd->para+ret,"</tr>");
		device++;

	}
	sprintf((char*)send_cmd->para+ret, "</table> </body> </html>");
	send_cmd->len=ret;
	return mod_socket_send(com_fd,CGI_MAIN_ID,VIDEOENC_MOD_ID, send_cmd, sizeof(mod_socket_cmd_type)-4+ret);
BUSY:
	
	send_cmd->cmd=PROBE_ONVIF_DEV_ACK;
	ret+=echo_str(send_cmd->para+ret,"<html>");
	ret+=echo_str(send_cmd->para+ret,"<META content=\"text/html; charset=gb2312\" http-equiv=Content-Type>");
	ret+=echo_str(send_cmd->para+ret,"<body>");
	ret+=echo_str(send_cmd->para+ret,"<table border=\"1\">");
	ret+=echo_str(send_cmd->para+ret,"<h1>不要频繁请求</h1>");
	sprintf((char*)send_cmd->para+ret, "</table> </body> </html>");
	send_cmd->len=ret;
	return mod_socket_send(com_fd,CGI_MAIN_ID,VIDEOENC_MOD_ID, send_cmd, sizeof(mod_socket_cmd_type)-4+ret);

	



}





static int process_modsocket_cmd(int sourceid,mod_socket_cmd_type *modsocket)
{
	int rc;
	rc=0;
	switch(modsocket->cmd)
	{
		case GATE_BYPASSTO_MOD_CMD://由主进程转发过来的网关命令

			rc= process_gate_cmd((struct gt_usr_cmd_struct *)&modsocket->para,&modsocket->gate);
			gtloginfo("recv GATE_BYPASSTO_MOD_CMD cmd !\n");
			//para的开头4个字节存放的是主进程对网关的描述符
			break;

		case MAIN_QUERY_STATE:

			gtloginfo("recv MAIN_QUERY_STATE cmd !\n");
			send_state2main();
			break;

		case PROBE_IP_DEVICE:

			gtloginfo("recv PROBE_DEVICE\n");
			process_probe_device();
			break;
		case PROBE_ONVIF_DEV:

			gtloginfo("recv PROBE_ONVIF_DEVICE\n");
			process_probe_onvif_device();
			break;



		case REQUIRE_UP_AUDIO:
			gtloginfo("recv REQUIRE_UP_AUDIO\n");
			if(require_up_audio(*(int *)(modsocket->para))<0)	
				printf("require_up_audio failed\n");
			break;
			printf("require_up_audio\n");


		case STOP_UP_AUDIO:

			gtloginfo("recv STOP_UP_AUDIO\n");
			if(stop_up_audio(*(int *)(modsocket->para))<0)	
				printf("stop_up_audio failed\n");
			break;

		case SEND_DOWN_AUDIO:

			gtloginfo("recv SEND_DOWN_AUDIO\n");
			
			if(send_down_audio(modsocket->para[0],modsocket->para[1])!=0)	
			
				printf("send_down_audio failed\n");
			break;

		case STOP_DOWN_AUDIO:
			gtloginfo("recv STOP_DOWN_AUDIO\n");
			
			if(stop_down_audio(*(int *)(modsocket->para))<0)	

				printf("stop_down_audio failed\n");
			
			break;

		case MAIN_REQUEST_APLAY:
				gtloginfo("rec MAIN_REQUEST_APLAY %d\n",*(int *)(modsocket->para));
				process_request_aplay(*(int *)(modsocket->para));
				break;


		
		default:
			printf("videoenc recv a unknow cmd 0x%x:",modsocket->cmd);
			gtloginfo("videoenc recv a unknow cmd 0x%x:",modsocket->cmd);
		break;
	}
	return rc;

}


int	creat_modcmdproc_thread(void)
{
	return creat_modsocket_thread(&modsocket_thread_id,com_fd,VIDEOENC_MOD_ID, "videoenc", process_modsocket_cmd);
}
