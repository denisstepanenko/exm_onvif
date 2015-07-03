#include "watch_media.h"

#include "video_enc_show.h"
//#include "audio_enc_show.h"

#include "tcprtimg_show.h"

#include <signal.h>
#include <devinfo.h>

#include "libterm.h"

//显示媒体服务状态
static void DisplayMediaState(void)
{
	WriteTermStr(C_HWHITE,0,"\t\t%s 服务信息\n\n",get_devtype_str());

	DisplayVideoEncState();
	display_audio_enc_state();
	display_audio_dec_state();
	//DisplayTcprtimgState();
	
	return;
}
static void process_ctrl_c(int signo)
{
	if(signo!=SIGINT)
		return;
	WriteTermStr(C_HWHITE,0,"退出媒体察看程序!!\n");
}
int main(int argc,char *argv[])
{
	struct timeval tv;
	int	Sel=0;
	char c;
       fd_set readfds;
	int	interval=1;
	init_devinfo();
	InitVEncState();
	init_audio_enc_state();
	InitTcprtimgState();
	init_audio_dec_state();

	tv.tv_sec = interval;
	tv.tv_usec = 0;
	FD_ZERO (&readfds);	
	signal(SIGINT,process_ctrl_c);
	while(Sel>=0)
	{

		ClearTermScr();			//清屏
		DisplayMediaState();

	
		tv.tv_sec = interval;
		tv.tv_usec = 0;
		FD_SET (0, &readfds);
		Sel=select (1, &readfds, NULL, NULL, &tv);
		if(Sel==0)
		{
			//continue;
		}
		else if(Sel>0)
		{
			if (FD_ISSET (0, &readfds)) 
			{
				if (read (0, &c, 1) <= 0)
				{
					printf("read <=0\n");
					exit(1);
				}
			}
		}

	
	}

	exit(0);	
}




