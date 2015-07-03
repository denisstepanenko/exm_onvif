/*
	用于虚拟设备(.virdev > 1的设备)的设备信息返回
							Feb 2009
*/

#ifndef DEVINFO_VIRDEV_H
#define DEVINFO_VIRDEV_H

#include <devinfo.h>
#include <devinfo_virdev.h>
#include <guid.h>
static char virdev1_guid_str[64];

/**********************************************************************************************
 * 函数名	:virdev_get_virdev_number()
 * 功能	:应用程序调用获得设备内含的虚拟设备个数(应该已经调用过了init_devinfo())
 * 输入	: 无
 * 输出 	:
 * 返回值	:正值表示虚拟设备个数，负值表示出错
  **********************************************************************************************/
int virdev_get_virdev_number(void)
{
	int devtype;

	devtype=get_devtype();

	if((devtype==T_GTVS3022)||(devtype==T_GTMV3122))
		return 2;
	else
		return 1;
}

/**********************************************************************************************
 * 函数名	:virdev_get_devid()
 * 功能	:应用程序调用获得虚拟设备的dev_id(应该已经调用过了init_devinfo())
 * 输入	: virdev_no,虚拟设备号(从0到virdev_get_virdev_number()-1)
 * 输出 	:buf:应用程序需要存放devid的缓冲区地址,返回时填充buf的长度必须足够长大于DEV_GUID_BYTE
 * 返回值	:正值表示填充到buf中的有效字节数，负值表示出错
  **********************************************************************************************/
int virdev_get_devid(int virdev_no,unsigned char *buf)
{
	int ret;
	int devtype;

	ret=get_devid(buf);
	if(ret<=0)
		return ret;
	
	devtype=get_devtype();
	if((devtype==T_GTVS3022)||(devtype==T_GTMV3122))
	{
		if(virdev_no!=0)
		{
			buf[ret-1]+=0x01;	//最高位的字节+1
		}
	}
	return ret;
}


/**********************************************************************************************
 * 函数名	:virdev_get_devid_str()
 * 功能	:应用程序调用获得虚拟设备的guid的字符串
 * 输入	:virdev_no,虚拟设备号(从0到virdev_get_virdev_number()-1)
 * 返回值	:指向描述guid信息的字符串指针
  **********************************************************************************************/
char* virdev_get_devid_str(int virdev_no)
{
	int ret;
	char idbuf[32];
	struct GT_GUID	*pid=(struct GT_GUID*)idbuf;
	char *pguid=get_devid_str();
	int devtype;
	
	
	devtype=get_devtype();
	if((devtype==T_GTVS3022)||(devtype==T_GTMV3122))
	{
		if(virdev_no!=0)
		{
			ret=virdev_get_devid(virdev_no,idbuf);
			guid2hex(*pid,virdev1_guid_str);	
			pguid=virdev1_guid_str;
		}
		
	}
	return pguid;
}



/**********************************************************************************************
 * 函数名	:virdev_get_total_com()
 * 功能	:获取指定虚拟设备的串口数
 * 输入	:virdev_no,虚拟设备号(从0到virdev_get_virdev_number()-1)
 * 返回值	:正值表示设备的总串口数,负值表示出错
 **********************************************************************************************/
int virdev_get_total_com(int virdev_no)
{
	int devtype;
	
	devtype=get_devtype();
	if((devtype==T_GTVS3022)||(devtype==T_GTMV3122))
	{
		return 1;
	}
	else
		return get_total_com();
}


/**********************************************************************************************
 * 函数名	:virdev_get_video_num()
 * 功能	:获取指定虚拟设备可接入的视频总数
 * 输入	:virdev_no,虚拟设备号(从0到virdev_get_virdev_number()-1)
 * 返回值	:可接入的视频数
 **********************************************************************************************/
int virdev_get_video_num(int virdev_no)
{
	int devtype;
	
	devtype=get_devtype();
	if((devtype==T_GTVS3022)||(devtype==T_GTMV3122))
	{
		return 1;
	}
	else
		return get_video_num();
	
}
/**********************************************************************************************
 * 函数名	:virdev_get_videoenc_num()
 * 功能	:获取指定虚拟设备视频编码器数
 * 输入	:virdev_no,虚拟设备号(从0到virdev_get_virdev_number()-1)
 * 返回值	:视频编码器数
 **********************************************************************************************/
int virdev_get_videoenc_num(int virdev_no)
{
	int devtype;

	devtype=get_devtype();
	if((devtype==T_GTVS3022)||(devtype==T_GTMV3122))
	{
		return 1;
	}
	else
		return get_videoenc_num();
}


/**********************************************************************************************
 * 函数名	:virdev_get_trigin_num()
 * 功能	:获取指定虚拟设备输入端子数
 * 输入	:virdev_no,虚拟设备号(从0到virdev_get_virdev_number()-1)
 * 返回值	:指定虚拟设备上的输入端子总数
 **********************************************************************************************/
int	virdev_get_trigin_num(int virdev_no)
{
	int devtype;
	
	devtype=get_devtype();
	if((devtype==T_GTVS3022)||(devtype==T_GTMV3122))
	{
		if(virdev_no!=0)
			return 0;
	}
	return get_trigin_num();	
}
/**********************************************************************************************
 * 函数名	:virdev_get_alarmout_num()
 * 功能	:获取指定虚拟设备输出端子数
 * 输入	:virdev_no,虚拟设备号(从0到virdev_get_virdev_number()-1)
 * 返回值	:指定虚拟设备上的输出端子数
 **********************************************************************************************/
int virdev_get_alarmout_num(int virdev_no)
{
	int devtype;
	
	devtype=get_devtype();
	
	if((devtype==T_GTVS3022)||(devtype==T_GTMV3122))
        {
                if(virdev_no!=0)
                        return 0;
        }
	return get_alarmout_num();
}



/**********************************************************************************************
 * 函数名	:virdev_get_audio_num()
 * 功能	:获取指定虚拟设备音频通道个数
 * 输入	:virdev_no,虚拟设备号(从0到virdev_get_virdev_number()-1)
 * 返回 :指定虚拟设备音频通道个数
 **********************************************************************************************/
int virdev_get_audio_num(int virdev_no)
{
	int devtype;
	
	devtype=get_devtype();
	if((devtype==T_GTVS3022)||(devtype==T_GTMV3122))
        {
                if(virdev_no!=0)
                        return 0;
        }
	return get_audio_num();
}



#endif




