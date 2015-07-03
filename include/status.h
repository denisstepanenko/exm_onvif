//定义设备状态结构

#ifndef STATUS_H
#define STATUS_H

#ifndef DWORD
#define DWORD	unsigned long
#endif

struct per_state_struct{			//GT1000外设连接状态
	unsigned 		video_loss0		:1;	//bit0视频输入丢失
	unsigned 		video_loss1		:1;
	unsigned		video_loss2		:1;
	unsigned 		video_loss3		:1;
	unsigned 		video_loss4		:1;
	unsigned 		video_loss5		:1;
	unsigned 		video_loss6		:1;
	unsigned 		video_loss7		:1;
	unsigned 		video_loss8		:1;
	unsigned 		video_loss9		:1;
	unsigned 		video_loss10	:1;
	unsigned 		video_loss11	:1;
	unsigned		video_blind0	:1;//bit12 第1路视频遮挡
	unsigned		video_blind1	:1;
	unsigned 		video_blind2	:1;
	unsigned 		video_blind3	:1;
	unsigned		audio_loss0		:1;	//bit16 音频输入丢失
	unsigned		audio_loss1		:1;
	unsigned		audio_loss2		:1;
	unsigned		audio_loss3		:1;
	unsigned		disk_full		:1;	//磁盘满
	unsigned 		keyboard_err	:1;	//键盘连接故障
	unsigned		err_connect_xvs	:1;	//连接外部dvs故障
	unsigned		video_blind4to15:1;	//第4-15路视频遮挡
	unsigned 		pwr_loss		:1;	//外接触发用电源故障
	unsigned		audio_out_err	:1;	//音频输出故障
	unsigned		video_loss12	:1;	//视频丢失
	unsigned		video_loss13	:1;	//视频丢失
	unsigned		video_loss14	:1;	//视频丢失
	unsigned		video_loss15	:1;	//视频丢失
	unsigned		upnp_err	:1; //upnp端口映射异常
	unsigned 		xvslogin_err	:1;	 //登录外接xvs帐号错误	
};
struct dev_state_struct{		//GT1000系统内部状态
	unsigned		link_err		:1;	//断线
	unsigned		mem_err			:1;	//内存故障
	unsigned		flash_err		:1;	//flash故障
	unsigned		hd_err			:1;	//硬盘故障	//这位不用了
	unsigned		cf_err			:1;	//存储卡故障
	unsigned		audio_dec_err	:1;	//音频解码设备故障
	unsigned		reserve			:2;
	unsigned		video_enc0_err	:1;	//netenc
	unsigned		video_enc1_err	:1;	//hq0
	unsigned		video_enc2_err	:1;	//hq1
	unsigned		video_enc3_err	:1;	//hq2
	unsigned		video_enc4_err	:1;	//hq3
	//06.09.06	新加故障
	unsigned		quad_dev_err	:1;	//画面分割器故障
	unsigned 		watch_51_err	:1;	//51模块故障
	//unsigned		reserve1		:17;
	unsigned        pow_overflow_err:1;
	unsigned        pow_underflow_err : 1;
	unsigned        reserve1        :15;
};

struct	trig_state_struct{	//GT1000系统报警状态
	unsigned		trig0			:1;//外触发
	unsigned		trig1			:1;
	unsigned		trig2			:1;
	unsigned		trig3			:1;
	unsigned		trig4			:1;
	unsigned		trig5			:1;
	unsigned		trig6			:1;//震动触发//trig_vib
	unsigned		trig7			:1;//外部触发电源//trig_pwr
	unsigned		trig8			:1;
	unsigned		trig9			:1;
	unsigned		motion0			:1;//移动触发
	unsigned		motion1			:1;
	unsigned		motion2			:1;
	unsigned		motion3			:1;
	unsigned		motion4			:1;
	unsigned		motion5			:1;
	unsigned		motion6			:1;
	unsigned		motion7			:1;
	unsigned		motion8			:1;
	unsigned		motion9			:1;
	unsigned		motion10		:1;
	unsigned		motion11		:1;
	unsigned		motion12		:1;
	unsigned		motion13		:1;
	unsigned		motion14		:1;
	unsigned		motion15		:1;
	unsigned		trig10			:1; //端子10
	unsigned		trig11			:1;
	unsigned		trig12			:1;
	unsigned		trig13			:1;
	unsigned		trig14			:1;
	unsigned		trig15			:1;
};
#endif
