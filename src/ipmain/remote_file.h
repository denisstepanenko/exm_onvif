#ifndef REMOTE_FILE_H
#define REMOTE_FILE_H
#include <gt_com_api.h>
#include "mod_socket.h"
//和网关传递配置文件使用的文件格式 
/*访问设备配置文件 USR_RW_DEV_PARA
使用此命令来读写设备的参数配置文件（目前只在工程安装软件里面使用）,
设备收到此命令后应该以DEV_PARA_RETURN返回
   0x0108 USR_RW_DEV_PARA FTP文件下载
  {
  	  type(2)         类型（2:ip1004.ini 3:alarm.ini其他值保留）
	  mode(2)		操作模式:0：读 1：写
	  filelen(4)		文件长度(在写模式下有效,长度不能超过60k)
	  file(n)			文件内容(仅在写模式下有效)
}

*/
int usr_rw_para_file(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no);
/**********************************************************************************************
 * 函数名	:send_para_to_rmt()
 * 功能	:将设备的配置文件发送给远程计算机
 * 输入	:fd 连接到远程计算机的文件描述符，fd不可以是负值
 *			 type:文件 值定义同struct usr_rwdevpara_struct中的定义 
 *         			（2:ip1004.ini 3:alarm.ini其他值保留）
 *			env:使用的数字信封格式
 *			enc:要使用的加密格式
 *			enack:是否需要确认
 *返回值	:0表示成功负值表示失败
 *			: -10 参数错误
 *		   	: -11不支持的格式
 **********************************************************************************************/
int send_para_to_rmt(int fd,int type,int env,int enc,int enack,int dev_no);

#endif
