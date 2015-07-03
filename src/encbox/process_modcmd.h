#ifndef PROCESS_GATECMD_H
#define PROCESS_GATECMD_H
#include <gate_cmd.h>
#include "mod_socket.h"
#include "mod_cmd.h"
#include "process_modcmd.h"




int process_net_index(char *index,struct query_index_struct *query);
//初始化与主进程通讯的命令通道
int init_com_channel(void);

int	creat_modcmdproc_thread(void);


int send_state2main(void);

//设置第encno个编码器的错误状态，若有改变则发送给主进程
int set_enc_error(int encno,int flag);

//设置第ch通道的视频丢失，若有改变则发送给主进程
int set_video_loss(int ch,int flag);

//设置第ch通道的移动侦测和视频遮挡,并发送给主进程
int set_video_motion_blind(int ch,int motion, int blind);

int require_up_audio(int no);
#endif
