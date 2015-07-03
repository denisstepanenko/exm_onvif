#ifndef __PUT_ERRR_H
#define __PUT_ERRR_H

#define	ERR_NO						(0)	//没有错误
#define	ERR_NO_DISK				(1)	//找不到硬盘
#define	ERR_NO_FORMAT_DISK		(2)	//没有格式化磁盘
#define	ERR_OPEN_FILE				(3)	//打开文件错误
#define	ERR_WRITE_FILE				(4)	//写文件错误
#define	ERR_READ_FILE				(5)	//读文件失败
#define	ERR_AUDIO_OPEN			(6)	//打开音频设备错误
#define	ERR_AUDIO_SET_PARA		(7)	//音频设备参数设置错误
#define	ERR_AUDIO_SET_GAIN		(8)	//音频增益调节错误
#define	ERR_AUDIO_REC				(9)	//录音错误
#define	ERR_AUDIO_CLOSE			(10)	//关闭音频设备失败
#define	ERR_TEST					(99)	//测试失败

#define	STR_ERR_NO					("正确")
#define	STR_ERR_NO_DISK			("找不到硬盘，请检查硬盘连接是否正确")
#define	STR_ERR_NO_FORMAT_DISK	("磁盘没有格式化分区")
#define	STR_ERR_OPEN_FILE			("读写测试时打开文件错误")
#define	STR_ERR_WRITE_FILE		("写文件错误")
#define	STR_ERR_READ_FILE			("读文件错误")
#define	STR_ERR_AUDIO_OPEN		("打开音频设备错误")
#define	STR_ERR_AUDIO_SET_PARA	("音频设备参数设置错误")
#define	STR_ERR_AUDIO_SET_GAIN	("音频增益调节错误")
#define	STR_ERR_AUDIO_REC			("录音错误")
#define	STR_ERR_AUDIO_CLOSE		("关闭音频设备失败")
#define	STR_ERR_TEST				("测试失败")
#define	STR_ERR_UNKNOW			("未知错误号")

/**********************************************************************************************
* 函数名   :get_err_str()
* 功能  :       返回错误码字符串
* 输入  :      errr	错误码					
*						
* 输出  :       void        
* 返回值:   描述错误的字符串
**********************************************************************************************/
char *get_err_str(int errr);

#endif
