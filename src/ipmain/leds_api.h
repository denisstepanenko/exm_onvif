#ifndef LEDS_API_H
#define LEDS_API_H
#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long

#define LEDS_MAJOR     255 	//changed by shixin 254
#define LEDS_NAME      "LEDS"
#define LEDS_VERSION   "Version 0.7"

//0.7 用于IP1004 LED灯模式 目前3个灯: 网络/错误/报警，且仅支持2元模式
//网络灯: 注册上网关则常亮，否则不亮  错误灯:任何错误则常亮，否则不亮   报警等:有报警时常亮，复位后不亮
//0.6 网络灯增加一种闪烁方式,2长1短; 用于所有的用户名密码验证失败
//0.5 加入对内核2.6的支持
//0.4增加了键盘连接故障的err灯显示
//0.3重写大部分，增加了net灯指示各种adsl情况的功能
//0.2增加了state灯指示报警状态功能

#ifdef __KERNEL__

#if HZ==100
#define RETURN_TIME 25//单位:1/100秒
#elif HZ==200
#define RETURN_TIME 50
#else
	#error "unsupport HZ value"
#endif

#endif

#define LONGSIG 		5// 单位:RETURN_TIME/100秒
#define LONGPAUSE   	3
#define SHORTSIG 	1 //100ms
#define SHORTPAUSE 	1 //100ms
#define CYCLEPAUSE  	10 //完成一个cycle后的中断

#define MAX_DISPLAY_TYPE  33 //显示种类最大值

struct ts_struct {
	WORD longtime; //长
	WORD shorttime; //短
};

struct led_polling_struct 
{
	int cycle_done;	//cycle完成标志
	int long_count;	//长信号输出中计数器
	int short_count; //短信号输出中计数器
	int long_done;   //长信号输出完个数
	int short_done;	 //短信号输出完个数
	int long_num;    //长信号个数
	int short_num;   //短信号个数
	int pause_count; //短间隔计数器
	int long_pause_count; //长间隔计数器
	int cycle_pause_count; //循环间隔计数器
};

#define SET_NET_LED 		1 
#define SET_ERROR_LED   	12
#define SET_ALARM_LED		3

#define NET_LED				1
#define ALARM_LED			2
#define ERR_LED				3

#define NET_DISPLAY_TYPE	8//7  //灯的闪烁方式种类
#define ALARM_DISPLAY_TYPE	4
#define ERR_DISPLAY_TYPE	33

#define NET_NO_ADSL 	0
#define NET_NO_MODEM	1
#define NET_INVAL_PASSWD 2
#define NET_LOGINED	   4
#define NET_INVAL_USR  3
#define NET_ADSL_OK		5		
#define NET_GATE_CONNECTED	6	
#define NET_REGISTERED	7
#define NET_PAP_FAILED	8	

/*
#define STATE_RESET		0
#define STATE_UPDATING	1
#define STATE_ALARMING  2
#define STATE_ACK_ERR	3
#define STATE_ACKED		4
*/
#define STATE_ALARMING 2

int init_leds(void);
#if 0
int set_errorled_flash_mode(struct ts_struct *tsnew, int count); //count为字节数,应为132
int set_stateled_flash_mode(struct ts_struct *tsnew, int count);
int set_netled_flash_mode(struct ts_struct *tsnew, int count);
#endif

int set_error_led_state(void);//0常灭，其他的见表
int set_net_led_state(int state);	//0长灭，1一长一短，2一长二短，3一长三短..见表
int set_state_led_state(int state); //具体数字见表
int get_current_netled(void); //返回目前的net灯状态
int get_current_alarmled(void);

#endif


