#ifndef DEVICE_ACCESS_H
#define DEVICE_ACCESS_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iniparser.h>



//设备访问文件中的访问类型
#define 		ACCESS_RTSTREAM	"rt_stream"		//实时音视频上传
#define 		ACCESS_AUDIOPLY	"audio_play"		//音频下传
#define		ACCESS_COM		"com"			//透明串口
#define		ACCESS_CMD		"cmd"			//命令服务
#define		ACCESS_SET	1						//设置访问列表
#define		ACCESS_CLR	0						//清除访问列表

 /**********************************************************************************************
 * 函数名	:SetDeviceAccess()
 * 功能	: 设置/清除指定地址的访问权限
 * 输入	:AccType:访问类型(ACCESS_RTSTREAM,ACCESS_AUDIOPLY,ACCESS_COM,ACCESS_CMD)
 *			 Addr:允许访问的地址字符串
 *			 Type:设置/清除(ACCESS_SET,ACCESS_CLR)
 * 输出	:无
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/
int SetDeviceAccess(char *AccType,char *Addr,int Type);

 /**********************************************************************************************
 * 函数名	:CheckDeviceAccess()
 * 功能	: 检查指定地址是否有访问权限
 * 输入	:AccType:访问类型(ACCESS_RTSTREAM,ACCESS_AUDIOPLY,ACCESS_COM,ACCESS_CMD)
 *			 Addr:要检查的地址字符串
 * 输出	:无
 * 返回值	:1表示有权限 0表示没有权限 负值表示出错
 **********************************************************************************************/
int CheckDeviceAccess(char *AccType,char *Addr);

#endif
