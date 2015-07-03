#ifndef DISPLAY_OSD_H
#define DISPLAY_OSD_H

#include "pthread.h"
#include "mod_socket.h"

#define OSD_NET_CH		0	//网络通道
#define OSD_LOCAL_CH 	1	//本地高清晰存储




#define	MAX_DISPLAY_CHAR_D1		18	//D1时最多能显示的字符数目
#define MAX_DISPLAY_CHAR_CIF	80	//cif时最多能显示的字符数目

#define OSD_POSX_MAX	29  //to be fixed~
#define OSD_POSY_MAX	17  //to be fixed

#define	INSTPLACE_DEF_POSX	1		//osd显示安装地点时的缺省输出位置,下同
#define INSTPLACE_DEF_POSY	0		//to be fixed

#define FULL_CAMERANAME_POSX 58
#define FULL_CAMERANAME_POSY 17	
#define QUAD0_CAMERANAME_POSX 29
#define QUAD0_CAMERANAME_POSY 8 

#define NOQUAD_CAMERANAME_POSX	31
#define NOQUAD_CAMERANAME_POSY	17

struct osd_init_struct
{	
	int inst_place_display;		//是否显示安装地点osd,1为显示，其他为不显示
	int inst_place_max_len;		//安装地点允许的最大字符数,以字符为单位
	int inst_place_posx;		//安装地点osd的输出横坐标,以左上角为原点，取值0-44,缺省1
	int inst_place_posy;		//安装地点osd的输出纵坐标,以左上角为原点，取值0-17,缺省1
};



void osd_second_proc(void);

//往相应通道(net/local)写osd信息,自行判断是否需要写，以及内容和坐标
int write_osd_info(int osd_ch, int vidpicsize);

//将install:osd_max_len变量设置到ip1004.ini文件
int osd_set_maxlen_to_file(void);


int creat_osd_proc_thread(pthread_attr_t *attr,void *arg);


#endif
