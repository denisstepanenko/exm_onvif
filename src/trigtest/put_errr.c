#include <stdio.h>
#include "put_errr.h"

/**********************************************************************************************
* 函数名   :get_err_str()
* 功能  :       返回错误码字符串
* 输入  :      errr	错误码					
*						
* 输出  :       void        
* 返回值:   描述错误的字符串
**********************************************************************************************/
char *get_err_str(int errr)
{
	if(errr<0)
		errr=-errr;

	switch(errr)
	{
		case ERR_NO:
			return STR_ERR_NO;

		case ERR_TEST:
			return STR_ERR_TEST;
			
		default:
			return STR_ERR_UNKNOW;
	}

	return NULL;
}

