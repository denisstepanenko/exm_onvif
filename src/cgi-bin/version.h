#ifndef __VERSION_H
#define	__VERSION_H
	/*
		0.24 增加no.8~no.11 共四个虚拟端子
	    0.23 fixbug ALARM_CONF 多了multichannle一节
	    0.22 fixbug MAX_MOD_SOCKET_CMD_LEN
		0.21 支持对讲设备AAC版本
		0.19-20 buzalarm videox:inst_place
		0.18 判断快速设置多通道选项勾选与否，屏蔽4通道回放选项
		0.17 增加格式化返回信息
		0.16 修改不能格式化分区的问题
		0.15 修改web页面hd1显示不正常的问题
		0.14 增加安装地址选项，按照osd标准限值字符数
		0.13 www 修正回放没有多路的选项
		0.12 增加multichannel选项
		0.11 www修改var status支持ie	
		0.10 修改遗留bug ，make_json_str在ip1004.ini中没有值返回0而不是null
		0.09 增加a_channel字段支持录像绑定音频
		0.08 在网页中增加功能，硬盘单个分区格式化，硬盘重新分区，按时间段显示日志
 		0.07 修改重启命令中reboo为hwrbt
		0.06 增加对cookies的判断
		0.05 增加对录像编码参数配置
		0.04 增加了查看日志功能，修改音频上下行范围[0,15]
		0.03 增加了支持网页配置音频上下行功能
			增加了支持修改视遮挡，移动侦测灵敏度的配置
		0.02 支持了新的字段bitratecon;netencoder:maxbitrate
			修复了上个版本帧率，分辨率不能修改的问题
		0.01 首发
	*/
	const char VERSION[]="0.24";

	#define VER_FILE_LEN	50
typedef struct ERR_PARA
{
	char 	ipaddr;
	char	net_mask;
	char	default_gate;
	char	dns_svr;
	char	rmt_gate1;
	char	rmt_gate2;
	char	rmt_gate3;
	char	rmt_gate4;
	char	alarm_server;
	char	cmd_port;
	char	video_port;
	char	audio_port;
	char	com0_port;
	char	com1_port;
	char	file_port;
	char	pasv_port;
	char	telnet_port;
	
}err_para;
	
typedef struct PROG_VERSION
{
	char ipmain[VER_FILE_LEN];
	char rtimage[VER_FILE_LEN];
	char cgi[VER_FILE_LEN];
	char encbox[VER_FILE_LEN];
}prog_version;
#endif

