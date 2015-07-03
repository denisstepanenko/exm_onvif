#include "osd_api.h"
#include "display_osd.h"
#include "time.h"
#include "ipmain.h"
#include "devinfo.h"
#include "ipmain_para.h"
#include "netcmdproc.h"
#include "commonlib.h"
#include "iniparser.h"


int write_scr_time(void)
{
	struct tm *ptime;
	//int posx,posy,color,len;
	//unsigned char pstr[100];
	int color = 0;
	time_t ctime;
	ctime=time(NULL);
	ptime=localtime(&ctime);	
	if(ptime==NULL)
		return -1;
#if 0	
	/*to be fixed below....wsy*/
	posx=25;//shixintest  40;
	posy=0; 
	color=0;
	len=0;
	sprintf(pstr,"%d-%02d-%02d %02d:%02d:%02d",
			ptime->tm_year+1900,	//年
			ptime->tm_mon+1,	//月
	 		ptime->tm_mday, 		//日
			ptime->tm_hour,		//时
			ptime->tm_min,		//分
			ptime->tm_sec);		//秒
	osd_write(OSD_NET_CH, posx,posy,pstr,len,color);
	osd_write(OSD_LOCAL_CH,posx,posy,pstr,len,color);

#endif
	osd_display_time(OSD_NET_CH,ptime,color);
 	osd_display_time(OSD_LOCAL_CH,ptime,color);
	return 0;
}



//汉字的长度和字母长度都是1
size_t hzstrlen( char *s)
{
        int len=0;
        char *p=s;
        if(s==NULL)
                return 0;
        while(*p!='\0')
        {
                len++;
                if(*p>0x7f)
                        p++;
                p++;
        }
        return len;
}

int get_full_cameraname_posx(int vidpicsize)
{
	if((get_devtype()==T_GTVS3024)&&(vidpicsize != 0)) //3024,且size为cif/hd1
		return FULL_CAMERANAME_POSX-1;
	else
		return FULL_CAMERANAME_POSX;
}


//screen_ch : 0,1,2,3分别代表四分割的四个位置
int get_cameraname_display_place(IN int screen_ch, IN char* pstr, IN int vidpicsize, OUT int * posx, OUT int * posy)
{
	if((posx == NULL)||(posy == NULL)||(pstr == NULL))
		return -EINVAL;
		
	if(get_quad_flag()) //有四分割的设备	
	{
		switch(screen_ch)
		{
			case(0):	*posx =	QUAD0_CAMERANAME_POSX-strlen(pstr);
						*posy = QUAD0_CAMERANAME_POSY;
						return 0;
			case(1):	*posx =	get_full_cameraname_posx(vidpicsize)-strlen(pstr);
						*posy = QUAD0_CAMERANAME_POSY;
						return 0;
			case(2):	*posx =	QUAD0_CAMERANAME_POSX-strlen(pstr);
						*posy = FULL_CAMERANAME_POSY;
						return 0;
			case(3):	*posx =	get_full_cameraname_posx(vidpicsize)-strlen(pstr);
						*posy = FULL_CAMERANAME_POSY;
						return 0;
			default:	return -EINVAL;
		}
	}
	else	//单路设备
	{
		*posx = NOQUAD_CAMERANAME_POSX - strlen(pstr);
		*posy = NOQUAD_CAMERANAME_POSY;
		return 0;
	}
}


//osd_ch: OSD_NET_CH或OSD_LOCAL_CH
//video_ch: 0-3表示全屏，4表示四分割
//vidpicsize,显示尺寸，0-D1,1-CIF,2-HD1
int write_cameraname_to_channel(IN int osd_ch, IN  int video_ch, IN int vidpicsize)
{
	int posx,posy;
	char *pstr;
	int ret;
	int i;
	int color = 0;
	
	
	if((osd_ch!=OSD_NET_CH)&&(osd_ch!=OSD_LOCAL_CH))
		return -EINVAL;
		
	if(video_ch >= (get_video_num() + get_quad_flag()))
		return -EINVAL;
			
	
	if(video_ch == 4)//四分割
	{
		for(i=0;i<4;i++)
		{
			pstr = get_mainpara()->vadc.enc_front[i].name;
			get_cameraname_display_place(i,pstr,vidpicsize,&posx, &posy);
			ret = osd_write(osd_ch, posx, posy, pstr,0, color,1);
		}
	}
	else //全屏
	{
		pstr = get_mainpara()->vadc.enc_front[video_ch].name;
		get_cameraname_display_place(3,pstr,vidpicsize,&posx, &posy);
		ret = osd_write(osd_ch, posx, posy, pstr,0, color,1);
	}

	return ret;
}

/*************************************************
  在屏幕上相应叠加摄像头的名称osd
  输入: osd_ch,取值为OSD_LOCAL_CH或OSD_NET_CH
  		video_ch,通道号，0-3表示相应摄像头全屏，4表示四分割	
  		vidpicsize,显示尺寸，0-D1,1-CIF,2-HD1
  注意:全屏时叠加在大屏幕右下角，四分割时叠加在每个小画面的右下角
*************************************************/
int display_camera_name(int osd_ch, int video_ch,int vidpicsize)
{
	if((video_ch>4)||(video_ch <0))
		return -EINVAL;
	
	return write_cameraname_to_channel(osd_ch,video_ch,vidpicsize);
		
}


/*************************************************
	//wsyadd
	在屏幕上叠加安装地点字符串osd
	输入: 	osd_ch,取值为OSD_LOCAL_CH或OSD_NET_CH
		    posx,输出坐标的x轴偏移量,取值0-44; 
			posy,输出坐标的y轴偏移量，取值0-17
*************************************************/
int display_install_place(int osd_ch, int posx, int posy)
{
	char inst_place[100];
	int color=0;
	int ret;
	if((virdev_get_virdev_number()==2)&&(osd_ch == OSD_LOCAL_CH))
		get_dev_sitename(1,inst_place);
	else
		get_dev_sitename(0,inst_place);
	ret = osd_write(osd_ch,posx,posy,inst_place,0,color,0);
	return ret;
}

//将install:osd_max_len变量设置到ip1004.ini文件
int osd_set_maxlen_to_file(void)
{
	dictionary *ini = NULL;
	FILE *fp = NULL;
	int maxlen = 0;
	char entry[30];
	
	maxlen = osd_get_max_len();
	if(maxlen < 0)
	{
		gtloginfo("取osd安装地点最大可写字符数失败返回%d\n",maxlen);
		return -1;
	}
	
	ini=iniparser_load_lockfile(IPMAIN_PARA_FILE,1,&fp);
	if(ini==NULL)
	{
		return -EINVAL;	
	}
	else
	{
		sprintf(entry,"install:osd_max_len");
		iniparser_setint(ini,entry,maxlen);		
		gtloginfo("osd安装地点最大可写字符数为%d,写入配置文件\n",maxlen);
		save_inidict_file(IPMAIN_PARA_FILE,ini,&fp);
		iniparser_freedict(ini);
		return 0;
	}

}


//往相应通道(net/local)写osd信息,自行判断是否需要写，以及内容和坐标
//只包括clear_scr和camera_name
int write_osd_info(int osd_ch, int vidpicsize)
{

	struct ipmain_para_struct *mainpara;
	int video_ch = 0;
	int ret;
	if(get_osd_flag() == 1)
	{	
		mainpara=get_mainpara();
		osd_clear_scr(osd_ch);
		
		if(get_quad_flag()==1)//有视频分割器才显示镜头名称
		{
			if(osd_ch == OSD_NET_CH)
				video_ch = mainpara->vadc.quad.current_net_ch;
			//if(osd_ch == OSD_LOCAL_CH)
			//	video_ch = mainpara->vadc.quad.current_local_ch;	
			ret = display_camera_name(osd_ch,video_ch,vidpicsize);
		}
		
		return ret;
	}
	else
		return -EINVAL;

	return 0;
}

static int osd_display_cnt = 5; //计数器，每10秒显示一次安装地点


static void * osd_proc_thread(void * para)
{
	
	gtloginfo("start osd_proc_thread...\n");
	printf("start osd_proc_thread...\n");
	
	write_osd_info(OSD_NET_CH,get_mainpara()->net_ch_osd_picsize);
	//write_osd_info(OSD_LOCAL_CH,get_mainpara()->local_ch_osd_picsize);
	/*
	if(virdev_get_virdev_number()==2) //虚拟设备
	{
		if(get_mainpara()->devpara[0].osd.inst_place_display == 1)
		{
			display_install_place(OSD_NET_CH,get_mainpara()->devpara[0].osd.inst_place_posx,get_mainpara()->devpara[0].osd.inst_place_posy);
		}
		if(get_mainpara()->devpara[1].osd.inst_place_display == 1)
		{
			display_install_place(OSD_LOCAL_CH,get_mainpara()->devpara[1].osd.inst_place_posx,get_mainpara()->devpara[1].osd.inst_place_posy);
		}
	}
	else
	*/
	{
		if(get_mainpara()->devpara[0].osd.inst_place_display == 1)
		{
			display_install_place(OSD_NET_CH,get_mainpara()->devpara[0].osd.inst_place_posx,get_mainpara()->devpara[0].osd.inst_place_posy);		
			display_install_place(OSD_LOCAL_CH,get_mainpara()->devpara[0].osd.inst_place_posx,get_mainpara()->devpara[0].osd.inst_place_posy);
		}	
	}
	
	while(1)
	{
		//if(get_quad_flag()==0)//3021,22,需要不停补写
		{
			if((osd_display_cnt <=30)&&(((++osd_display_cnt) % 10) == 0))
			{	

//				osd_display_cnt = 0;
				write_osd_info(OSD_NET_CH,get_mainpara()->net_ch_osd_picsize);
				//write_osd_info(OSD_LOCAL_CH,get_mainpara()->local_ch_osd_picsize);
				if(virdev_get_virdev_number()==2) //虚拟设备
				{
					if(get_mainpara()->devpara[0].osd.inst_place_display == 1)
					{
						display_install_place(OSD_NET_CH,get_mainpara()->devpara[0].osd.inst_place_posx,get_mainpara()->devpara[0].osd.inst_place_posy);
					}
					if(get_mainpara()->devpara[1].osd.inst_place_display == 1)
					{
						display_install_place(OSD_LOCAL_CH,get_mainpara()->devpara[1].osd.inst_place_posx,get_mainpara()->devpara[1].osd.inst_place_posy);
					}
				}
				else
				{
					if(get_mainpara()->devpara[0].osd.inst_place_display == 1)
					{
						display_install_place(OSD_NET_CH,get_mainpara()->devpara[0].osd.inst_place_posx,get_mainpara()->devpara[0].osd.inst_place_posy);
						display_install_place(OSD_LOCAL_CH,get_mainpara()->devpara[0].osd.inst_place_posx,get_mainpara()->devpara[0].osd.inst_place_posy);
					}	
				}
			}
		}	
		
		
		write_scr_time();
		
		//sleep(1);
		usleep(1000*100);
	}
	return NULL;
}	

pthread_t osd_thread_id = -1;

int creat_osd_proc_thread(pthread_attr_t *attr,void *arg)
{
	return pthread_create(&osd_thread_id,attr, osd_proc_thread, NULL);
}



void osd_second_proc(void)
{
	return;
}

