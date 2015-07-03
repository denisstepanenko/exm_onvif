#ifndef DEVICE_STATE_H
#define DEVICE_STATE_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iniparser.h>


//设备状态文件中的节名
#define 		DEV_VENC0		"venc0"		//视频编码器0
#define 		DEV_VENC1		"venc1"		//视频编码器1
#define 		DEV_VENC2		"venc2"		//视频编码器2
#define 		DEV_VENC3		"venc3"		//视频编码器3
#define 		DEV_VENC4		"venc4"		//视频编码器4
#define		DEV_QUAD		"quad"		//画面分割器
#define		DEV_SIMCOM	"simcom"		//虚拟串口
#define		DEV_IDEDISK	"idedisk"		//硬盘或cf卡
#define		DEV_LED		"leds"		//指示灯驱动
#define		DEV_AENC0		"aenc0"		//音频编码设备
#define		DEV_ADEC0		"adec0"		//音频解码设备

//设备状态文件中的变量名
#define		DEV_INSTALL	"install"		//设备正常安装标志
#define		DEV_STATE		"state"		//当前工作状态 0表示正常
#define		DEV_ERRNUM	"errnum"		//设备出错的次数




 /**********************************************************************************************
 * 函数名	:SetDeviceStateValInt()
 * 功能	: 设置指定设备的指定状态变量的值(整数形式)
 * 输入	:DevName:设备类型，在头文件中定义DEV_VENC0,DEV_QUAD等
 *			 ValName:要设置的变量明 在头文件中定义 DEV_INSTALL,DEV_STATE,DEV_ERRNUM
 *			 Val:要设置的值
 * 输出	:无
 * 返回值	:0表示成功，负值表示失败
 **********************************************************************************************/
int SetDeviceStateValInt(char *DevName,char *ValName,int Val);

 /**********************************************************************************************
 * 函数名	:GetDeviceStateValInt()
 * 功能	: 获取指定设备的指定状态变量的值(整数形式)
 * 输入	:DevName:设备类型，在头文件中定义DEV_VENC0,DEV_QUAD等
 *			 ValName:要设置的变量明 在头文件中定义 DEV_INSTALL,DEV_STATE,DEV_ERRNUM
 *			 DefVal:默认值
 * 输出	:无
 * 返回值	:变量的值
 **********************************************************************************************/
int GetDeviceStateValInt(char *DevName,char *ValName,int DefVal);

 /**********************************************************************************************
 * 函数名	:IncDeviceStateValInt()
 * 功能	: 将指定设备的指定状态变量的值加1(整数形式)
 * 输入	:DevName:设备类型，在头文件中定义DEV_VENC0,DEV_QUAD等
 *			 ValName:要设置的变量明 在头文件中定义 DEV_INSTALL,DEV_STATE,DEV_ERRNUM
 * 输出	:无
 * 返回值	:增加后的值
 **********************************************************************************************/
int IncDeviceStateValInt(char *DevName,char *ValName);

/**********************************************************************************************
 * 函数名	:SetDeviceStateValStr()
 * 功能	:设置指定设备的指定状态变量的值(字符串形式)
 * 输入	:DevName:设备类型，在头文件中定义DEV_VENC0,DEV_QUAD等
 *			 ValName:要设置的变量明 在头文件中定义 DEV_INSTALL,DEV_STATE,DEV_ERRNUM
 *			 SVal:要设置的字符串指针
 * 输出	:无
 * 返回值	:0表示成功负值表示出错
 **********************************************************************************************/
int SetDeviceStateValStr(char *DevName,char *ValName,char *SVal);

/**********************************************************************************************
 * 函数名	:SetDeviceState()
 * 功能	:设置设备的存在标志及错误次数
 * 输入	:DevName:设备类型，在头文件中定义DEV_VENC0,DEV_QUAD等
 *			 InstFlag:是否存在标志1表示存在，0表示不存在
 *			 ErrNum:出错次数值
 * 输出	:无
 * 返回值	:0表示成功负值表示出错
 *			本函数一般用于程序刚启动时
 **********************************************************************************************/
int SetDeviceState(char *DevName,int InstFlag,int ErrNum);

/**********************************************************************************************
 * 函数名	:GetVEncStateSec()
 * 功能	:根据视频编码器号获取设备名
 * 输入	:EncNo:视频编码器号
 * 输出	:无
 * 返回值	:设备名字符串
 *			本函数一般用于程序刚启动时
 **********************************************************************************************/
char *GetVEncStateSec(int EncNo);


#endif
