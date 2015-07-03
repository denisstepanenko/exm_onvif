#include "ipmain.h"
#include "leds_api.h"
#include <file_def.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "netcmdproc.h"
#include "devstat.h"
#include <gate_cmd.h>
#include "mod_socket.h"
#include "devinfo.h"
static int leds_fd = -1;
static int current_net =0;
static int current_alarm=0;//纪录目前的闪烁情况
	
int init_leds(void)
{
#if EMBEDED	
	leds_fd=open(LEDS_DEV,O_RDWR);
	if(leds_fd<0)
	{
		printf("can't open leds device !\n");
		return -1;
	}
	else
	{
		#ifdef SHOW_WORK_INFO
			printf("open leds device success=%d.\n",leds_fd);	
		#endif
	}
#endif
	return 0;
}

/*
//count为字节数,用于更新errorled的ts表
int set_errorled_flash_mode(struct ts_struct *tsnew, int count)
{

#if EMBEDED	
	if(count !=ERR_DISPLAY_TYPE*4)
		return -1;
	return ioctl(leds_fd,SET_ERROR_DISPLAY,tsnew);
#else
	return 0;
#endif
}

//count为字节数,应为7*4=28,用于更新netled的ts表
int set_netled_flash_mode(struct ts_struct *tsnew, int count)
{
#if EMBEDED	
	if(count !=NET_DISPLAY_TYPE*4)
		return -1;
	return ioctl(leds_fd,SET_NET_DISPLAY,tsnew);
#else
	return 0;
#endif
}

//count为字节数,应为4*4=16,用于更新stateled的ts表
int set_stateled_flash_mode(struct ts_struct *tsnew, int count)
{
#if EMBEDED	
	if(count !=STATE_DISPLAY_TYPE*4)
		return -1;
	return ioctl(leds_fd,SET_STATE_DISPLAY,tsnew);
#else
	return 0;
#endif
}
*/

//lc to do 目前简化操作，上层只传入0/1，表示是否闪烁 state = 0/1
/*
int set_net_led_state(int state) 
{
	current_net=state;
	//gtloginfo("测试，灯设为%d\n",state);
	return ioctl(leds_fd,SET_NET_LED,&state);
}

int set_state_led_state(int state)
{
	current_state=state;
	return ioctl(leds_fd,SET_STATE_LED,&state);
}

int set_error_led_state(void)
{
	struct ip1004_state_struct *gtstate;
	struct per_state_struct *perstate;
	struct dev_state_struct *devstate;
	unsigned long errbuffer=0;
	gtstate=get_ip1004_state(0);
	
	perstate=&gtstate->reg_per_state;
	devstate=&gtstate->reg_dev_state;
	errbuffer |= devstate->mem_err;
	errbuffer<<=1;
	errbuffer |= devstate->flash_err;
	errbuffer<<=1;
	errbuffer |= devstate->video_enc0_err;
	errbuffer<<=1;
	if(virdev_get_virdev_number()==2)	
		errbuffer |= get_ip1004_state(1)->reg_dev_state.video_enc0_err;
	else
		errbuffer |= get_ip1004_state(0)->reg_dev_state.video_enc1_err;
	errbuffer<<=1;
	errbuffer |= devstate->video_enc2_err;
	errbuffer<<=1;
	errbuffer |= devstate->video_enc3_err;
	errbuffer<<=1;
	errbuffer |= devstate->video_enc4_err;
	errbuffer<<=1;
	errbuffer |= devstate->audio_dec_err;
	errbuffer<<=1;
	if(get_lfc_flag()==1)//已出厂
		errbuffer |= devstate->cf_err;
	errbuffer<<=1;
	if(get_lfc_flag()==1)//已出厂
		errbuffer |= devstate->hd_err;
	errbuffer<<=1;
	errbuffer |= perstate->keyboard_err;
	errbuffer<<=1;
	errbuffer |= perstate->video_loss0;
	errbuffer<<=1;
	if(virdev_get_virdev_number()==2)
		errbuffer |= get_ip1004_state(1)->reg_per_state.video_loss0;
	else
		errbuffer |= get_ip1004_state(0)->reg_per_state.video_loss1;
	errbuffer<<=1;
	errbuffer |= perstate->video_loss2;
	errbuffer<<=1;
	errbuffer |= perstate->video_loss3;
	errbuffer<<=1;
	if(get_lfc_flag()==1)//已出厂
		errbuffer |= perstate->disk_full;
	errbuffer<<=1;
	errbuffer |= perstate->pwr_loss;
	errbuffer<<=1;
	errbuffer<<=14;

	return ioctl(leds_fd,SET_ERROR_LED,&errbuffer);
}
*/

int set_net_led_state(int state) 
{
	current_net=state;
	//gtloginfo("测试，灯设为%d\n",state);
	int iostate = (state == 0)? 0:7;
	return ioctl(leds_fd,SET_NET_LED,&iostate);
}

int set_alarm_led_state(int state)
{
	current_alarm=state;
	int iostate = (state == 0)? 0:4;
	return ioctl(leds_fd,SET_ALARM_LED,&state);
}

int set_error_led_state(void)
{
	struct ip1004_state_struct *gtstate;
	struct per_state_struct *perstate;
	struct dev_state_struct *devstate;
	unsigned long errbuffer=0;
	gtstate=get_ip1004_state(0);
	
	perstate=&gtstate->reg_per_state;
	devstate=&gtstate->reg_dev_state;
	errbuffer |= devstate->mem_err;
	errbuffer<<=1;
	errbuffer |= devstate->flash_err;
	errbuffer<<=1;
	errbuffer |= devstate->video_enc0_err;
	errbuffer<<=1;
	if(virdev_get_virdev_number()==2)	
		errbuffer |= get_ip1004_state(1)->reg_dev_state.video_enc0_err;
	else
		errbuffer |= get_ip1004_state(0)->reg_dev_state.video_enc1_err;
	errbuffer<<=1;
	errbuffer |= devstate->video_enc2_err;
	errbuffer<<=1;
	errbuffer |= devstate->video_enc3_err;
	errbuffer<<=1;
	errbuffer |= devstate->video_enc4_err;
	errbuffer<<=1;
	errbuffer |= devstate->audio_dec_err;
	errbuffer<<=1;
	if(get_lfc_flag()==1)//已出厂
		errbuffer |= devstate->cf_err;
	errbuffer<<=1;
	if(get_lfc_flag()==1)//已出厂
		errbuffer |= devstate->hd_err;
	errbuffer<<=1;
	errbuffer |= perstate->keyboard_err;
	errbuffer<<=1;
	errbuffer |= perstate->video_loss0;
	errbuffer<<=1;
	if(virdev_get_virdev_number()==2)
		errbuffer |= get_ip1004_state(1)->reg_per_state.video_loss0;
	else
		errbuffer |= get_ip1004_state(0)->reg_per_state.video_loss1;
	errbuffer<<=1;
	errbuffer |= perstate->video_loss2;
	errbuffer<<=1;
	errbuffer |= perstate->video_loss3;
	errbuffer<<=1;
	if(get_lfc_flag()==1)//已出厂
		errbuffer |= perstate->disk_full;
	errbuffer<<=1;
	errbuffer |= perstate->pwr_loss;
	errbuffer<<=1;
	errbuffer<<=14;

	return ioctl(leds_fd,SET_ERROR_LED,&errbuffer);
}


int get_current_netled(void)
{
	return current_net;
}

int get_current_alarmled(void)
{
	return current_alarm;
}
