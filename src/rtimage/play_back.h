#ifndef PLAY_BACK_H
#define PLAY_BACK_H

#include "rtimage2.h"

typedef struct{

	pthread_mutex_t 	mutex;
	int pb_first_fg[MAX_VIDEO_ENCODER]; //开始链接回放指针的标志
	int pb_audio_flag[MAX_AUDIO_ENCODER];
	int playback_flag[MAX_VIDEO_ENCODER];//正在回放的通道
	int pb_ct[MAX_VIDEO_ENCODER];
	int pb_venc[MAX_VIDEO_ENCODER];//回放指针链接缓冲池的状态-1:已链接0:未链接
	int pb_aenc[MAX_AUDIO_ENCODER];
	int pb_vct[MAX_VIDEO_ENCODER];
	int pb_act[MAX_AUDIO_ENCODER];
	int current_net_ch[TCPRTIMG_MAX_AVUSR_NO+1];
	int frame_adjust[MAX_VIDEO_ENCODER];
	int aframe_adjust[MAX_AUDIO_ENCODER];
	int default_screen;



}playback_t;
playback_t * get_playback_parm(void);

void init_playback_parm(void);


int set_net_scr_ch(int usr_no,int ch);

int get_net_scr_ch(int usr_no);

int get_playback_stat(int enc_no);

void set_playback_en(int no);
void mutichannel_set_playback_en(int no);


/*

	结束回放状态
*/

void set_playback_cancel(int no);
/*

	只将回放状态切换到实时画面状态
*/
void set_playback_to_live(void);
void mutichannel_set_playback_to_live(int enc_no);
int get_playback_frame_adjust(int no);

void set_playback_frame_adjust(int no,int frame_ct);


#endif
