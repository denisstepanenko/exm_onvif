#include "ipmain.h"
#include "hdmodapi.h"
#include "maincmdproc.h"
#include "gate_cmd.h"
#include "mod_com.h"
#include "mod_cmd.h"


/*****************************************************************************
 *函数名: alarm_snapshot()
 *功能:   让hdmodule模块进行报警抓图
 *输入:   takepic,为takepic_struct结构,包括时间戳,张数，间隔，通道
 *返回值: 0表示成功，负值表示失败
 ****************************************************************************/
int alarm_snapshot(struct takepic_struct *takepic,WORD env,WORD enc)
{
	DWORD send_buf[200];
	mod_socket_cmd_type *send;
	struct takepic_struct *pic;
	
	if(takepic==NULL)
		return -1;

	//gtloginfo("test,alarm_snapshot中,%d interval , %d number, %d channel\n",takepic->interval,takepic->takepic,takepic->channel);
	
	send = (mod_socket_cmd_type *)send_buf;
	send->cmd=ALARM_SNAPSHOT;
  //zw-add -2012-05-15
  send->gate.dev_no=0;
  send->gate.env=env;
  send->gate.enc=enc;
  //zw-add 2012-05-15

	pic=(struct takepic_struct*)send->para;
	send->len = sizeof(struct takepic_struct);
 	memcpy(pic,takepic,sizeof(struct takepic_struct));
	return main_send_cmd(send,HQSAVE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);

}


/*
 *	高清晰录像控制
 * 	ch:要控制的压缩通道号
 * 	mode:	0: 表示停止录像线程
			1: 表示启动录像线程
			2: 表示重新启动录像线程
*/
static int hdrecord_ctrl(int ch,int mode)
{
	DWORD send_buf[30];
	mod_socket_cmd_type *send;
	
	struct hdrecord_ctrl_struct *ctrl;

	send=(mod_socket_cmd_type *)send_buf;	
	send->cmd=HDRECORD_CTRL;
	ctrl=(struct hdrecord_ctrl_struct*)send->para;
	ctrl->channel=ch;
	ctrl->mode=mode;
	send->len = sizeof(struct hdrecord_ctrl_struct);
	return main_send_cmd(send,HQSAVE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
	
}
/**********************************************************************************************
 * 函数名	:refresh_hdmodule_para()
 * 功能	:让hdmodule重新读取配置
 * 输入	:无	 
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int refresh_hdmodule_para(void)
{
	DWORD send_buf[30];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	send->cmd=MAIN_REFRESH_PARA;
	send->len = 0;
	return main_send_cmd(send,HQSAVE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
}
/**********************************************************************************************
 * 函数名	:trig_record_event()
 * 功能	:触发一次录像事件
 * 输入	:ch录像通道
 *			 trig:触发事件(录像原因)
 *			reclen:希望进行多长时间的录像(实际录像时还会加上延时录像),传0即可
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int trig_record_event(int ch,int trig_flag,int reclen)
{
	DWORD send_buf[30];
	struct trig_record_event_struct *trig;
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	
	send->cmd=TRIG_RECORD_EVENT;
	trig=(struct trig_record_event_struct *)send->para;
	trig->channel=ch;
	trig->trig_flag=trig_flag;
	trig->reclen=reclen;
	send->len = sizeof(struct trig_record_event_struct);
	return main_send_cmd(send,HQSAVE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);

}

/**********************************************************************************************
 * 函数名	:clear_hdmod_trig_flag()
 * 功能	:清除高清晰录像模块的触发状态
 * 输入	:无
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int clear_hdmod_trig_flag(DWORD trig)
{
	DWORD send_buf[30];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	send->cmd=CLEAR_TRIG_FLAG;
	//lc 2014-2-11 将trig发送过去
	*((int *)send->para)=trig; //channel
	send->len = sizeof(int);
	return main_send_cmd(send,HQSAVE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);		
}



/*********************************************************************************************
*函数名	:	alarm_playback()
*功能		:	发送录像回放命令给tcprtimg
*输入		:	无
*返回值	:	0表示成功，负值表示失败
**********************************************************************************************/
int alarm_playback(int ch)
{
	DWORD	send_buf[50];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	send->cmd=MAIN_PLAYBACK_IMG_CMD;
	send->len = sizeof(int);
	memcpy((BYTE *)send->para,(BYTE *)&ch,sizeof(int));
	
	return main_send_cmd(send,RTIMAGE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
}

/*********************************************************************************************
*函数名	:	alarm_enc_playback()
*功能		:	发送录像回放命令给videoenc
*输入		:	无
*返回值	:	0表示成功，负值表示失败
**********************************************************************************************/
int alarm_enc_playback(int ch)
{
	DWORD	send_buf[30];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	send->cmd=MAIN_PLAYBACK_ENC_CMD;
	send->len = sizeof(int);
	memcpy((BYTE *)send->para,(BYTE *)&ch,sizeof(int));
	
	return main_send_cmd(send,VIDEOENC_MOD_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
}



/*********************************************************************************************
*函数名	:	alarm_hd_playback()
*功能		:	发送录像回放命令给hdmodule
*输入		:	无
*返回值	:	0表示成功，负值表示失败
**********************************************************************************************/
int alarm_hd_playback(void)
{
	DWORD	send_buf[30];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	send->cmd=MAIN_PLAYBACK_ENC_CMD;
	send->len = 0;
	return main_send_cmd(send,HQSAVE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
}
/*********************************************************************************************
*函数名	:	alarm_stop_playback()
*功能		:	发送停止录像回放命令给videoenc
*输入		:	无
*返回值	:	0表示成功，负值表示失败
**********************************************************************************************/
int alarm_cancel_playback(void)
{
	DWORD	send_buf[30];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	send->cmd=RTIMG_PLAYBACK_STOP_CMD;
	send->len = 0;
	return main_send_cmd(send,VIDEOENC_MOD_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
}

/*********************************************************************************************
*函数名	:	alarm_cancel_playback_rtimg()
*功能		:	发送停止录像回放命令给tcprtimg
*输入		:	ch 切换回的通道号
*返回值	:	0表示成功，负值表示失败
*备注		:	此函数将发送停止录像回放命令给tcprtimg模块
**********************************************************************************************/
int alarm_cancel_playback_rtimg(int ch)
{
	DWORD	send_buf[50];
	mod_socket_cmd_type *send;
	send=(mod_socket_cmd_type *)send_buf;
	send->cmd=RTIMG_PLAYBACK_STOP_CMD;
	send->len = sizeof(int);
	memcpy((BYTE *)send->para,(BYTE *)&ch,sizeof(int));
	
	return main_send_cmd(send,RTIMAGE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
}


/**********************************************************************************************
 * 函数名	:restart_hd_record()
 * 功能	:重新启动指定通道的高清晰录像
 * 输入	:ch:要重新启动的高清析录像通道号
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int restart_hd_record(int ch)
{
	return hdrecord_ctrl(ch,2);
}

 /**********************************************************************************************
 * 函数名	:start_hd_record()
 * 功能	:启动指定通道的高清晰录像
 * 输入	:ch:要启动的高清析录像通道号
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int start_hd_record(int ch)
{
	return hdrecord_ctrl(ch,1);
}

/**********************************************************************************************
 * 函数名	:stop_hd_record()
 * 功能	:停止指定通道的高清晰录像
 * 输入	:ch:要停止的高清析录像通道号
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int stop_hd_record(int ch)
{
	return hdrecord_ctrl(ch,0);
}





