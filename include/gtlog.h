/*
gtlog库:

说明:

	级别顺序是error>warn>info>dbg
	输出级别要有开关控制
	正式发布时，只输出info及以上级别的日志

P.S.
	在vmmain其他代码中也要按这四个级别进行记录,但暂时用gtlog系列函数
	不在日志内容前加标记
*/

#ifndef GT_LOG_H
#define GT_LOG_H
#ifdef __cplusplus
extern "C" {
#endif


#ifndef _WIN32
#include <syslog.h>
#else
#define __inline__ __inline
#endif
//打开日志
#ifndef _WIN32
	#define gtopenlog(name) openlog(name,LOG_CONS|LOG_NDELAY|LOG_PID,LOG_LOCAL0 );//LOG_USER);
#else
	#define gtopenlog(name)
#endif
/**********************************************************************************
 *      函数名: log_headstring()
 *      功能:   将带有头标志信息及可变参数的信息记录到日志文件
 *      输入:   head_str:头参数信息字符串,如"[debug]"
 *				format:参数格式，同printf等
 *      返回值: 
 *		 注：字符串的总长度不要超过450字节
 *			  使用本函数要连接gtlog库
 *	应用程序应该不用直接使用此函数
 **********************************************************************************/
int log_headstring(const char *head_str,const char *format,...);


/*	使用时同printf,但会在日志内容之前加上[ERR]标记
	应用举例:出错返回时
*/
//int gtlogerror(const char *format,...);
#ifdef _WIN32
#define gtlogerr printf
#else
#define gtlogerr(args...)  log_headstring("[ERR]",##args)
#endif
/*	使用时同printf,但会在日志内容之前加上[WARN]标记
  	应用举例:将某个不符合要求的输入参数修正为缺省值时*/
//int gtlogwarn(const char *format,...);
#ifdef _WIN32
#define gtlogwarn printf
#else
#define gtlogwarn(args...) log_headstring("[WARN]",##args)
#endif
/*	使用时同printf,
	应用举例:初始化库函数时;记录和xvs交换的信息
*/
//int gtloginfo(const char *format,...);
#ifdef _WIN32
#define gtloginfo printf
#else
#define gtloginfo(args...) log_headstring(NULL,##args) 
#endif
#define gtlogfault gtlogerr

/*	使用时同printf,但会在日志内容之前加上[DBG]标记
	应用举例:记录调用套接口时的参数信息(用于调试xvslib库)
*/
#ifdef _WIN32
#define gtlogdebug printf
#else
#define gtlogdebug(args...) log_headstring("[DBG]",##args)
#endif

#ifdef __cplusplus
	}
#endif

#endif


