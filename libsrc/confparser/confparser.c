/*
 * conf文件解析器
 *
 */
/**
   @file    confparser.c
   @author  shixin
*/
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "confparser.h"

#define CONF_INVALID_KEY     ((char*)-1)
/*
 *对取到的行进行处理,包括:去掉行中的"号,空格和制表符,换行,跳过注释
 *
 */
static int process_line(char *buf,int len)
{
	char *p;
	if(buf==NULL)
		return -1;
	//跳过注释
	p=index(buf,'#');
	if(p!=NULL)
	{
		*p='\0';
		if(p==buf)
			return 0;
	}
	//清除引号
	while(1)
	{
		p=index(buf,'"');
		if(p==NULL)
			break;
		memcpy(p,(p+1),len-(p-buf));
	}
	//清除空格
	while(1)
	{
		p=index(buf,' ');
		if(p==NULL)
			break;
		memcpy(p,(p+1),len-(p-buf));
	}
	//清除制表符
	while(1)
	{
		p=index(buf,'\t');
		if(p==NULL)
			break;
		memcpy(p,(p+1),len-(p-buf));
	}
	//清除换行
	while(1)
	{
		p=strchr(buf,'\n');
		if(p==NULL)
			break;
		memcpy(p,(p+1),len-(p-buf));
	}
	return 0;
}

 /**********************************************************************************************
 * 函数名	:confparser_load_lockfile()
 * 功能		:调出conf文件到数据结构并且锁住文件,返回时lockf填充的是文件指针
 * 输入		:confname:要被打开的conf文件名
 * 返回值	:返回描述conf结构的指针，以后用该指针来访问文件中的变量，
 *				NULL表示出错
 **********************************************************************************************/
#include <errno.h>
confdict * confparser_load_lockfile(char * confname,int wait,FILE**lockf)
{
	FILE *fp;
	confdict *conf;
	int lock;

	if(confname==NULL)
	{
		return NULL;
	}
	//if(lockf==NULL)
	//	return NULL;
	*lockf=NULL;
	fp=fopen(confname,"r");
	if(fp==NULL)
	{
		return NULL;
	}
	lock=lock_file(fileno(fp),wait);
	fsync(fileno(fp));
	if(lock<0)
	{	
		fclose(fp);
		return NULL;
	}
	else
	{		
		conf=confparser_load(confname);
		if(conf==NULL)
		{
			fclose(fp);
		}
		
		else
			*lockf=fp;
		return conf;
	}
}


 /**********************************************************************************************
 * 函数名	:confparser_load()
 * 功能	:从conf文件中获取数据结构，返回指向数据结构的指针
 * 输入	:confname:要被打开的conf文件名
 * 返回值	:返回描述conf结构的指针，以后用该指针来访问文件中的变量，
 *				NULL表示出错,
 **********************************************************************************************/
confdict * confparser_load(char * confname)
{
        FILE *conf=NULL;
        int i;
        int lines=0;
        confdict * dict=NULL;
	 char *get=NULL;
        if(confname==NULL)
           	return NULL;
        conf=fopen(confname,"r");
        if(conf==NULL)
        {
        	printf("confparser_load open %s error!!\n",confname);
              return NULL;
        }
	dict=malloc(sizeof(confdict));
        if(dict==NULL)
	{
	 	fclose(conf);
		printf("confparser_load malloc failed!!\n");
		return NULL;
	}
		
	for(i=0;i<CONF_MAXLINE;i++)
	{
		get=fgets(dict->filebuf[i],MAX_CHAR_LINE,conf);
		if(get==NULL)
			break;
		else
		{
			process_line(dict->filebuf[i],MAX_CHAR_LINE);
			lines++;
		}
	}
	
	dict->lines=lines;
	fclose(conf);
	return dict;
}

/**********************************************************************************************
 * 函数名	:confparser_freedict()
 * 功能		:释放已经用过且不再使用的数据结构
 * 输入		:dict:之前用confparser_load返回的指针
 * 输出		:无
 * 返回值	:无
 **********************************************************************************************/
void confparser_freedict(confdict *dict)
{
	if(dict!=NULL)
		free(dict);

}

 /**********************************************************************************************
 * 函数名	:confparser_getstring()
 * 功能		:从数据结构中获取一个指定名字的变量（字符串形式），如果没有该变量则返回def
 * 输入		:dict:之前用confparser_load返回的指针
 *			:key:变量名
 *			 def:默认值
 * 输出		:无
 * 返回值	:变量值字符串
 **********************************************************************************************/
char * confparser_getstring(confdict *dict,char * key, char * def)
{
	int i;
	char *p,*name=NULL,*val=NULL;
	char *q;
	int qflag;
	
	if((dict==NULL)||(key==NULL))
		return def;
	
	for(i=0;i<dict->lines;i++)
	{
		p=dict->filebuf[i];
		name=strstr(p,key);
		if((name==NULL)||(name!=p))
			continue;
		else
		{
			q=name+strlen(key);
			if(*q!='=')		//重名而已
				continue;
			if(*(q+1)=='\0') //形为"USER=" 
				return def;
			break;
		}
	}
	if((name==NULL)||(name!=p))
		return def;
	
	val=strchr(name,'=');
	if(val==NULL)
		return def;
	val++;
	return val;	
}


 /**********************************************************************************************
 * 函数名	:confparser_setstr()
 * 功能	:将一个字符串数据设置到指定的变量名
 * 输入	:dict:之前用confparser_load返回的指针
 *			:key:变量名
 *			 val:要设置的字符串指针
 * 输出	:无
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/
int confparser_setstr(confdict *dict, char * key, char * val)
{
	char *oldval;
	int num;
	int i;
	char *p,*name=NULL;
	char *q;
	int qflag;
	if((dict==NULL)||(key==NULL))
		return -1;

	for(i=0;i<dict->lines;i++)
	{
		p=dict->filebuf[i];
		name=strstr(p,key);
		if(name==NULL)
			continue;
		else
		{
			q=name+strlen(key);
			if((*q!='=')&&(*q!=' '))		//重名而已
				continue;
			sprintf(dict->filebuf[i],"%s=%s",key,val);	//shixin changed 2006.11.16
			//sprintf(q+1,"%s",val);//有该节名
			return 0;
		}
	}
	//无节名
	{
		num=dict->lines;
		sprintf(dict->filebuf[num],"%s=%s",key,val);
		dict->lines++;
	}
	return 0;
}
/**********************************************************************************************
 * 函数名	:confparser_getint()
 * 功能	:从数据结构中获取一个整形的变量值，如果没有找到则返回notfound
 * 输入	:dict:之前用confparser_load返回的指针
 *			:key:变量名
 *			 notfound:默认值
 * 输出	:无
 * 返回值	:变量的值
 **********************************************************************************************/
int confparser_getint(confdict *dict, char * key, int notfound)
{
	char *str;
	if((dict==NULL)||(key==NULL))
		return notfound;
	str=confparser_getstring(dict,key,CONF_INVALID_KEY);
	if(str==CONF_INVALID_KEY)
		return notfound;
	else
		return atoi(str);
}

 /**********************************************************************************************
 * 函数名	:confparser_dump_conf()
 * 功能	: 将数据结构输出到一个打开的文件流
 * 输入	:	name:文件的名称
 			dict:之前用confparser_load返回的指针
 *			 f:已经打开的文件流
 * 输出	:无
 * 返回值	:无
 **********************************************************************************************/
void confparser_dump_conf(char *name,confdict* dict, FILE * f)
{
	int i,num;
	if((dict==NULL)||(f==NULL)||(name==NULL))
		return;
	//printf("confparser_dump_conf name=%s f=%x!!\n",name,f);
	f=freopen(name,"w",f);
	if(f==NULL)
		return;
	num=dict->lines;
	for(i=0;i<num;i++)
		{
			if(dict->filebuf[i][0]!='\0')
				fprintf(f,"%s\n",dict->filebuf[i]);
		}
	return;
}
#if 0
 /**********************************************************************************************
 * 函数名	:confparser_getstring_no_enter_no_space()
 * 功能	: 从conf文件取出字符串，不带最后的空格或回车符or '#'!
 * 输入	: dict:之前用confparser_load返回的指针
 *		  key:变量名
 *		  def:默认值		
 * 输出	:无
 * 返回值	:变量值字符串 add by wsy
 **********************************************************************************************/
char *confparser_getstring_no_enter_no_space(confdict * dict, char * key, char * def)
{
	
	char *str=NULL;
	char *lp;
	str=confparser_getstring(dict,key,def);
	
	lp=index(str,'\n');
	if(lp!=NULL)
		*lp='\0';
	lp=index(str,' ');
	if(lp!=NULL)
		*lp='\0';
	lp=index(str,'#');
	if(lp!=NULL)
		*lp='\0';
	return str;
}

 /**********************************************************************************************
 * 函数名	:confparser_setstr_with_enter()
 * 功能	:将一个字符串数据设置到指定的变量名并在末尾加上回车
 * 输入	:	dict:之前用confparser_load返回的指针
 *			:key:变量名
 *			 val:要设置的字符串指针
 * 输出	:无
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/

int confparser_setstr_with_enter(confdict * dict, char * key, char * val)
{
	char *lp;
	lp=index(val,'\n');
	if(lp==NULL)
		{
			lp=index(val,'\0');
			*lp='\n';
			lp++;
			*lp='\0';
		}
	confparser_setstr(dict,key,val);
	return 0;
}

int main(viod)
{
	int i;
	confdict *dict;
	dict=confparser_load("config");
	if(dict==NULL)
	{
		printf("open config failed!\n");
		exit(1);
	}
	else
		printf("open config success lines=%d\n",dict->lines);	
	printf("ETH0_IPADDR=%s\n",confparser_getstring(dict,"ETH0_IPADDR",NULL));
	printf("INTERNET_MODE=%d\n",confparser_getint(dict,"INTERNET_MODE",100));

	confparser_dump_conf(dict,stdout);

	return 0;
}
//by wsy, tried to patch confparser_functions
#endif
