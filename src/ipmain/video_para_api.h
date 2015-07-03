#ifndef VIDEO_PARA_API_H
#define VIDEO_PARA_API_H
#include "ipmain.h"
#include "mod_socket.h"
//hi3520A include
//#include "sample_comm.h"

//显示模式定义 
#define SCR_FULL			0	//全屏显示
#define SCR_QUAD		1	//4画面显示 
//#define SCR_DUAL		2	//双画面显示

#define DEF_BLIND_ALARMTIME		0
#define DEF_BLIND_UNALARMTIME  	3
#define DEF_BLIND_SEN			    0

#define MAX_CAMERANAME_LENGTH	    20 //to be fixed

#define SET_VIDEO_SATURATION	1
#define SET_VIDEO_SHARPNESS		2
#define SET_VIDEO_BRIGHTNESS	3
#define SET_VIDEO_HUE			4
#define SET_VIDEO_CONTRAST		5

#define USER_PIC_PATH   "/conf/pic_704_576_p420_novideo01.yuv"

struct enc_front_struct{	//视频前端转换参数
	int bright;
	int hue;
	int contrast;
	int saturation;
	int enable;		//是否有效  1表示有效 0表示无效
	char name[MAX_CAMERANAME_LENGTH*2];	//视频名称，如"大堂摄像头",缺省为"视频x",字节数不大于10个
};

struct motion_struct{		//移动检测参数
	unsigned long ch;			//视频输入通道
	unsigned long sen;		//灵敏度 0表示不需要移动侦测
	unsigned long alarm;    //若有移动是否报警,驱动程序不使用
	WORD starthour;  //起始hour,起始min ,结束hour,结束min，驱动程序不使用
	WORD startmin;
	WORD endhour;
	WORD endmin;
	WORD area[12];//区域
};
struct blind_struct{		//遮挡报警设置
	int ch;
	int sen;		//报警灵敏度 0表示不需要遮挡报警
	int alarm_time; //遮挡报警时间
	int cancelalarm_time; //无遮挡取消报警时间
};

struct quad_dev_struct
{
	int current_net_ch;			//当前网络显示通道,0-3表示相应全屏，4表示4分割
	int last_net_ch;
	//int current_local_ch;		//当前本地显示通道,0-3表示相应全屏，4表示4分割
	struct motion_struct 	motion[4];
	struct blind_struct 	blind[4];
};

struct video_para_struct
{
	struct enc_front_struct enc_front[4];
	struct quad_dev_struct quad;
};

//设置视频参数(亮度，色度等用到的结构)
struct set_video_para_struct{
	int		ch;		//视频输入通道
	int		val;		// 值
};

struct video_info_struct{
	int  sec_flag;				//秒标志
	unsigned long loss;		//视频丢失标志,1表示丢失
	unsigned long motion;		//移动触发标志,1表示有移动触发
	unsigned long blind;		//镜头遮挡标志,1表示有镜头遮挡
};



/*
 * 初始化视频解码设备
 * 返回值 0:成功，-1:失败
*/
int init_video_vadc(struct video_para_struct *init_para);

/*
 * 反初始化
 */
int uninit_video_vadc();

/********************************************************
 * 将网络传输通道的显示切换为全屏，第ch通道
 * 返回值   0：成功,负值表示错误
 * 输入:		ch：	需要切换的目标摄像头通道(有效值0,1,2,3)	
 ***************************************************/
int set_net_scr_full(BYTE ch,struct sockaddr *addr,char *username);

/********************************************************
 * 将本地录像通道的显示切换为全屏，第ch通道
 * 返回值   0：成功,负值表示错误
 * 输入:		ch：	需要切换的目标摄像头通道(有效值0,1,2,3)	
 ***************************************************/
 //int set_local_scr_full(BYTE ch);
 
/********************************************************
 * 将网络传输通道的显示切换为四分割
 * 返回值   0：成功,负值表示错误
 * 输入:		ch：	需要切换的目标摄像头通道(有效值0,1,2,3)	
 ***************************************************/
 int set_net_scr_quad(struct sockaddr *addr,char *username);
 
/********************************************************
 * 将本地录像通道的显示切换为四分割
 * 返回值   0：成功,负值表示错误
 * 输入:		ch：	需要切换的目标摄像头通道(有效值0,1,2,3)	
 ***************************************************/ 
//int set_local_scr_quad(void);



/********************************************************
 * 设置vda各通道工作参数
 * 返回值   0：成功,负值表示错误
 * 输入:		ch：	需要切换的目标摄像头通道(有效值0,1,2,3)	
 ***************************************************/ 
int init_video_vda_param(struct video_para_struct *vadc);

/*********************************************************
 * 设置vda各通道移动侦测灵敏度及区域
 * 返回值  0 成功，负值表示出错
 * 输入:  ch 目标摄像头通道(0,1,2,3)  sen 灵敏度  area 移动侦测区域
 ********************************************************/
int set_motion_vda_sen(int ch,int sen,WORD *area);

/*********************************************************
 * 设置vda各通道遮挡报警灵敏度
 * 返回值  0 成功，负值表示出错
 * 输入:  ch 目标摄像头通道(0,1,2,3)  sen 灵敏度 
 ********************************************************/
int set_blind_vda_sen(int ch,int sen);





//设置视频输入亮度,val为0~100
int set_video_bright(BYTE ch,int val);
//设置视频输入色度,val为0~100
int set_video_hue(BYTE ch,int val);

//设置视频输入对比度，val为0~100
int set_video_contrast(BYTE ch,int val);

//设置视频输入饱和度，val为0~100
int set_video_saturation(BYTE ch,int val);


int creat_vda_proc_thread(pthread_attr_t *attr,void *arg);

int video_blind_detected(unsigned long blind);

void vadc_second_proc(void);

#endif
