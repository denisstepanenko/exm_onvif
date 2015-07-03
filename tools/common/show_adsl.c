#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<iniparser.h>


#define	INI_FILE_NAME		("/conf/gt1000.ini")

/**********************************************************************************
* 	函数名:	show_adsl()
*	功能:	显示adsl信息
*	输入:	none
*	输出:	none
*	返回值:	0表示成功；负值表示错误码
*********************************************************************************/
int show_adsl(void)
{
	dictionary *ini;
	char *data_tmp=NULL;
	
	//判断ini文件名
        if(INI_FILE_NAME==NULL)
        {
                printf("error ini file.\n");
                return -1;
        }
	
	//载入ini文件
        ini=iniparser_load(INI_FILE_NAME);
        if (ini==NULL)
        {
		printf("cannot parse file [%s]",INI_FILE_NAME );
                return -1 ;
        }


	data_tmp=iniparser_getstr(ini,"net_config:internet_mode");
	if(strcmp(data_tmp,"0")==0)
	{
		printf("联网方式:	adsl接入\n");
	}
	if(strcmp(data_tmp,"1")==0)
	{
		printf("联网方式:	局域网接入\n");
	}
	if(strcmp(data_tmp,"2")==0)
	{
		printf("联网方式:	专线接入\n");	
	}

	data_tmp=iniparser_getstr(ini,"net_config:adsl_user");
	printf("    ADSL用户名:	%s\n",data_tmp);

	data_tmp=iniparser_getstr(ini,"net_config:adsl_passwd");
	printf("    ADSL密码:	%s\n",data_tmp);

	//free ini
	iniparser_freedict(ini);	



	return 0;
}

/**********************************************************************************
* 	函数名:	XVS_set_state_callback()
*	功能:	注册xvs报告状态的回调函数
*	输入:	XVS_state_callback函数指针,该函数输入为XVS_login时获得的操作句柄及XVS_status_t类型的指针
*	输出:	无
*	返回值:	0表示成功；负值表示错误码
*********************************************************************************/
int main(void)
{
	int ret;
	//printf("start...\n");
	ret=show_adsl();
	if(ret<0)
	{
		printf("ini文件错误\n");
	}
	//printf("return.\n");
	return 0;
}

