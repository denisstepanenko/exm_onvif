#ifndef INFODUMP_H
#define INFODUMP_H


#include "mod_socket.h"

//#include "gate_cmd.h"

#define SYSINFO_DUMP_FILE	"/tmp/vsmaininfo.txt" 			//系统信息文件

#define ALARM_LOG_VALID_HOURS	24	//报警和端子输入日志有效期,单位:小时

#define MAX_LOG_NUMBER		4000	//数据库中最多管理的日志条数

#define ALARM_DB	"/log/alarm.db"       //报警日志的数据库

/**********************************************************************************************
 * 函数名	:dump_sysinfo()
 * 功能	:将系统信息输出到指定文件
 * 输入	:无	 
 * 返回值	:无
 **********************************************************************************************/
void dump_sysinfo(void);

/**********************************************************************************************
 * 函数名	:dump_alarminfo_to_log()
 * 功能	:将报警信息字传输出到指定文件
 * 输入	:	trig:当前触发状态
 *			time:触发时间
 *			alarmid:报警号
 * 返回值	:无
 **********************************************************************************************/
void dump_alarminfo_to_log(DWORD trig,time_t time,char *alarmid);


/**********************************************************************************************
 * 函数名	:check_alarmlog()
 * 功能	:检查未ACK的近期报警,并逐条予以发送
 * 输入	:   
 * 返回值	:0
 **********************************************************************************************/
int check_alarmlog(void);


/**********************************************************************************************
 * 函数名	:check_triginlog()
 * 功能	:检查未ACK的近期端子变化信息,并逐条予以发送
 * 输入	:   
 * 返回值	:0
 **********************************************************************************************/
int check_triginlog(void);



/**********************************************************************************************
 * 函数名	:dump_trigininfo_to_log()
 * 功能	:将输入端子变化状态传输出到指定文件
 * 输入	:oldtrigin:之前的输入状态
 *		newtrigin:之后的输入状态
 *		time:发生改变的时间
 *		trigin_id:报警号
 * 返回值	:无
 **********************************************************************************************/
void dump_trigininfo_to_log(DWORD oldtrigin, DWORD newtrigin, DWORD time, char *trigin_id);


/**********************************************************************************************
 * 函数名	:dump_triginack_to_log()
 * 功能	:将网关发来输入端子变化状态的ACK写到指定文件
 * 输入	:	result, ack返回结果
 *			trigin_id,报警号
 * 返回值	:无
 **********************************************************************************************/
void dump_triginack_to_log(int result, char *trigin_id);


/**********************************************************************************************
 * 函数名	:dump_alarmack_to_log()
 * 功能	:将报警ack信息传输出到指定文件
 * 输入	:	result: ack结果
 *			alarmid:报警号
 * 返回值	:无
 **********************************************************************************************/
void dump_alarmack_to_log(int result, char * alarmid);


#endif
