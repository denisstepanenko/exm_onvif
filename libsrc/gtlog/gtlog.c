
///GTlog库,在windows下为运行程序当前目录的gtlog.txt文件
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#ifndef _WIN32
#include <syslog.h>
#else
#include <time.h>
#define	GTLOG_NAME	"./gtlog.txt"
FILE	*log_fp=NULL;				///<日志记录指针
#endif

/**********************************************************************************
 *      函数名: log_headstring()
 *      功能:   将带有头标志信息及可变参数的信息记录到日志文件
 *      输入:   head_str:头参数信息字符串,如"[debug]"
 *                              format:参数格式，同printf等
 *      返回值: 
 *               注：字符串的总长度不要超过450字节
 *                        使用本函数要连接gtlog库
 *      应用程序应该不用直接使用此函数
 **********************************************************************************/
int log_headstring(const char *head_str,const char *format,...)
{
#ifdef _WIN32
	struct tm* lt=NULL;
	time_t	curtime;  
#endif
	char	data[5120];
	char	*wp=data;	///开始存放格式列表的指针
	va_list ap;			///参数列表

#ifdef _WIN32
	curtime=time(NULL);
	lt=localtime(&curtime);
	if(lt!=NULL)
	{
		sprintf(data,"<%04d-%02d-%02d %02d:%02d:%02d>",
				lt->tm_year+1900,
				lt->tm_mon+1,
				lt->tm_mday,
				lt->tm_hour,
				lt->tm_min,
				lt->tm_sec			
			);
		wp+=strlen(data);
	}
#endif
	///写入头信息
	if(head_str!=NULL)
	{
		sprintf(wp,"%s",head_str);
		wp+=strlen(head_str);
	}

	///写入参数信息
	va_start(ap,format);
	vsprintf(wp,format, ap);
	va_end(ap);
#ifndef _WIN32
	syslog(LOG_INFO,data);
#else
	if(log_fp==NULL)
	{
		log_fp=fopen( GTLOG_NAME, "a" );
	}
	if(log_fp!=NULL)
	{
		fwrite(data,1,strlen(data),log_fp);
		fflush(log_fp);
	}
#endif
	return 0;
}


