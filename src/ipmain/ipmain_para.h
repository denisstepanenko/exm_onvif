#ifndef IPMAIN_PARA_H
#define IPMAIN_PARA_H
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iniparser.h>
#include "devinfo.h"
#include "display_osd.h"
#include "video_para_api.h"
#include "mod_socket.h"

extern int GATE_PERIOD_TIME;
#define MAX_DEV_NO 1

/*#	联动动作对应表
#	代码	动作

#	0 	无效
#	1	端子0输出1
#	2	端子0输出0
#	3	端子1输出1	
#	4	端子1输出0
#	5	端子2输出1
#	6	端子2输出0
#	7	端子3输出1
#	8	端子3输出0
#	9	摄像头切4分割
#	10	摄像头切0通道全屏
#	11	摄像头切1通道全屏
#	12	摄像头切2通道全屏
#	13	摄像头切3通道全屏
*/
struct alarm_trigin_struct
{								//描述一个报警输入事件的结构
	int trigin; 					//报警通道号
	int enable;					//是否有效
	int setalarm; 				// 1为布防，0为撤防
	int alarm;					//是否报警,1为是，0为否
	int imact[MAX_TRIG_EVENT]; 	//立即执行
	int ackact[MAX_TRIG_EVENT];    //确认执行，     
	int rstact[MAX_TRIG_EVENT];	//复位执行
	int	starthour;				//wsy add,布防有效起始时间
	int startmin;
	int endhour;				//布防有效结束时间
	int endmin;
};

struct alarm_motion_struct
{	//系统可以处理的报警事件参数结构
	//触发报警的端口，有6个输入端子0~5和4个移动侦测6~9	
	struct alarm_trigin_struct trigin[MAX_TRIG_IN];
	struct alarm_trigin_struct motion[MAX_VIDEO_IN];
	int audioalarm;     //是否需要声音提示,1为需要,0为不需要
    int playback_enable;    //是否需要核警的录像回放，1为需要，0为不需要  //zw-add 2012-04-24
    //lc do 增加端子触发报警对应视频映射关系
    int alarm_trigin_video_ch[MAX_TRIG_IN];
};


typedef struct{

	struct sockaddr_in *rmt_gate0;		//远程动态网关服务器地址
	struct sockaddr_in *rmt_gate1;		//静态远程网关服务器地址
	struct sockaddr_in *rmt_gate2;			
	struct sockaddr_in *rmt_gate3;
	struct sockaddr_in *rmt_gate4;
	struct sockaddr_in *alarm_gate;		//紧急报警网关服务器地址
	BYTE	inst_place[100];		//安装地点
	struct osd_init_struct osd;		//osd相关
	WORD    cmd_port;           	//命令服务端口
	int		ip_chg_flag;			//ip地址变化标志
	int 	sendgateini_flag;		//需要发送给网关配置文件
}dev_para_struct;

//主进程的参数结构
struct ipmain_para_struct
{
	 dev_para_struct	devpara[MAX_DEV_NO];
	 int		ini_version;				//ini配置文件的版本号
	 int		total_com;				//可以使用的透明串口数
	 in_addr_t lan_addr;				//局域网ip地址
	 in_addr_t lan_mask;				//子网掩码
	 in_addr_t wan_addr;				//公网ip地址
	WORD	rmt_env_mode;			//和远程服务器通讯使用的数字签名类型
	WORD	rmt_enc_mode;			//和远程服务器通讯命令使用的加密类型
	WORD	trig_in;					//报警输入属性 (按位表示通道)0常开 1常闭

	WORD	tin_mask;				//报警输入衍码(按位表示通道)
	
	WORD	alarm_out;				//报警输出属性0常开 1常闭(按位表示通道)
	WORD	alarm_mask;				//报警输出掩码
	
	struct alarm_motion_struct alarm_motion; 	//wsy add 报警联动结构
	struct video_para_struct vadc;		//视频adc芯片，2835或9910
	//int	   hq_follow_net;						//在只有单路高清晰压缩时,高清晰通道跟随网络通道图像变化
	//int	   hq_save_ch;						//高清晰录像的通道(有分割器时才有效)
	int	   net_ch;								//网络的视频通道
	int    alarm_playback_ch;                 //当前核警回放通道，有时为具体通道号，无时则为-1
	
	WORD	video_in_ch;					//系统支持的最大视频输入数
	WORD	video_mask;					//视频输入有效位掩码(会影响到视频丢失报警)
	WORD    image_port;						//图像服务端口
	WORD    audio_port;        					//音频服务端口
	WORD	telnet_port;						//telnet服务端口
	WORD	ftp_port;						//ftp服务端口	
	WORD	web_port;						//web配置页面服务端口
	WORD	com0_port;						//透明串口0端口号
	WORD	com1_port;						//透明串口1端口号
	WORD    pb_port;                        //录像点播端口
	WORD    dev_type;      					//设备类型,0为有2824，1为没有

	int		inst_ack;						//安装被确认标志 0表示未被正式安装 1表示已经被正式安装,未被正式安装时不需要证书也可以访问
	int		valid_cert;						//已经有合法证书标志
	int     	reset_modem; 					//是否重起猫,1表示是
	int 		reset_modem_time; 				//重起猫的间隔，秒数
	int     	internet_mode; 					//上网方式,0-adsl, 1-专线, 2-局域网
	//int		log_watch_lost;					//记录不能接收到watch模块信息
	//int 	snap_file_interval;				//报警抓图的间隔
	//int  	snap_file_num;					//报警抓图的张数
	//char   snap_file_path[100];			//报警抓图的路径
	int 	net_ch_osd_picsize;			//网络编码器尺寸
	//int		local_ch_osd_picsize;		//本地编码器尺寸
	int     multi_channel_enable;         //是否启用多通道
	int     current_audio_down_channel;  //当前下行通道号
	int     power_monitor_enable;
	int     buzzer_alarm_enable;

	//lc add 2014-10-20
	int     gatedown_relay_count;
	int     gatedown_relay_chn;
};

char * devlog(int dev_no);

typedef struct {//
	pthread_mutex_t mutex;
	int 	pb[20];	//
}Alarm_Playback_Tag;

Alarm_Playback_Tag pb_Tag;

///根据文件名获取该文件的存放路径的完整名
const char *get_audio_file(char *name);

/**********************************************************************************************
 * 函数名	:init_para()
 * 功能	:系统参数初始化，将参数设置为默认值
 * 输入	:无
 * 返回值	:无
 **********************************************************************************************/
void  init_para(void);

/**********************************************************************************************
 * 函数名	:read_config_ini_file()
 * 功能	:将config文件中的参数信息读取到参数结构中
 * 输入	:filename:配置文件名
 * 输出	:para:返回时填充参数的结构指针
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int read_config_ini_file(char *filename, struct ipmain_para_struct *para);

/**********************************************************************************************
 * 函数名	:readmain_para_file()
 * 功能	:将ip1004.ini文件中的参数信息读取到参数结构中
 * 输入	:filename:配置文件名
 * 输出	:para:返回时填充参数的结构指针
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int  readmain_para_file(char *filename,struct ipmain_para_struct *para);

/**********************************************************************************************
 * 函数名	:refresh_netinfo()
 * 功能	:刷新网络状态参数
 * 输入	:无
 * 返回值	:无
 **********************************************************************************************/
void refresh_netinfo(void);

/**********************************************************************************************
 * 函数名	:save_setalarm_para()
 * 功能		:以整数形式的值存入ini结构中的报警布撤防参数变量名
 * 输入		:type:报警类型，0为端子触发，1为移动侦测
 *			 ch:相应的通道数
 *			 setalarm:布防(1)或撤防(0)
 *			 starthour: 布防起始小时
 *			 startmin:布防起始分钟
 *			 endhour: 布防结束小时
 *			 endmin: 布防结束分钟
 * 输出	: ini:描述ini文件的结构指针,返回时被填充新值
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int save_setalarm_para(dictionary *ini,int type, int ch,int setalarm, int starthour, int startmin, int endhour, int endmin);

/**********************************************************************************************
 * 函数名	:save_video_para()
 * 功能	:以整数形式的值存入ini结构中的视频参数变量名
 * 输入	:
 *			 ch:视频通道号
 *			 type:参数变类型 "bright"...
 *			 val:变量值
 * 输出	: ini:描述ini文件的结构指针,返回时被填充新值
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int save_video_para(dictionary      *ini,int ch,char *type,int val);

/**********************************************************************************************
 * 函数名	:save_video_para_hex()
 * 功能	:以16进制整数形式的值存入ini结构中的视频参数变量名
 * 输入	:
 *			 ch:视频通道号
 *			 type:参数变量类型 "bright"...
 *			 val:变量值
 * 输出	: ini:描述ini文件的结构指针,返回时被填充新值
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int save_video_para_hex(dictionary *ini,int ch,char *type,int val);

/**********************************************************************************************
 * 函数名	:dump_para_to_file()
 * 功能	:将内存中存储的参数以文本的方式输出到一个已打开的文件中
 * 输入	:无
 * 输出	:已打开的文件指针
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int dump_para_to_file(FILE *fp);


/**********************************************************************************************
 * 函数名	:CopyPara2Bak()
 * 功能	:将配置文件更新到相应的备份
 * 输入	:type:配置文件类型2:ip1004.ini 3:alarm.ini
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int CopyPara2Bak(int dev_no,int type);
	
/**********************************************************************************************
 * 函数名	:CheckParaFileChange()
 * 功能	:检查配置文件是否有变化
 * 输入	:type表示配置文件类型  2:ip1004.ini 3:alarm.ini
 * 返回值	:返回0表示无变化  1表示有变化 -1表示类型错误
 **********************************************************************************************/
int CheckParaFileChange(int dev_no,int type);
/**********************************************************************************************
 * 函数名	:AddParaFileVersion()
 * 功能	:将配置文件的版本号加1
 * 输入	:type表示配置文件类型  2:ip1004.ini 3:alarm.ini
 * 返回值	:新的版本号,负值表示失败
 **********************************************************************************************/
int AddParaFileVersion(int dev_no,int type);
/**********************************************************************************************
 * 函数名	:get_internet_mode_str()
 * 功能	:获取internet连接方式字符串
 * 输入	:无
 * 返回值	:描述internet连接方式的字符串
 **********************************************************************************************/
char *get_internet_mode_str(void);

/**********************************************************************************************
 * 函数名	:get_mainpara()
 * 功能	:获取设备参数结构指针
 * 输入	:无
 * 返回值	:设备参数结构指针
 **********************************************************************************************/
struct ipmain_para_struct * get_mainpara(void);

/**********************************************************************************************
 * 函数名	:get_serial_mode()
 * 功能	:获取串口0调试模式
 * 输入	:无
 * 返回值	:1:串口输出为心跳  0:串口输出为调试
 **********************************************************************************************/
int get_serial_mode();

int get_serial_interval();

#endif






