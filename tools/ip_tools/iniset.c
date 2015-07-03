/*用于将一个ini文件内所有变量设置到另一个ini文件的工具，wsy october 2006*/
#include <stdio.h>
#include <stdlib.h>
#include "file_def.h"
#include "iniparser.h"
#include "commonlib.h"

#define CANT_FIND_KEY "无此key" //getstring找不到相应关键词时的default返回

#define VERSION "1.2"           //版本号

//1.2 移植到GTIP-E设备中
//1.1 比较参数的值时,按两者中最长的长度比较,以免出现不能缩短参数值的问题
//	  最后将dic结构设置到target文件，而不是/conf/ip1004.ini
	  
//1.0 initial


//输出帮助信息并退出
int print_helpmsg(void)
{
	printf("\n欢迎使用iniset文件设置工具!(version %s)\n\n",VERSION);
	printf("使用说明: iniset [OPTION] 来源文件 目标文件\n\n");
	printf("    功能: 将来源文件中的每个变量设置到目标文件\n\n");
	printf("\t-h\t输出帮助信息\n");
	printf("\t-v\t输出版本信息\n\n");
	return(0);
}

//输出版本信息并退出
int print_version(void)
{
	printf("\nThis is iniset version %s\n\n",VERSION);
	return (0);
}

#if 0
//搬到iniparser库中了(shixin)
/*将source文件的每个节读出，与target文件相应节读出内容比较，
  若不同则改写target,若target不存在该节则创建,均记日志*/
int ini_set_file(char *source,char *target)
{
	dictionary *srcini,*tgtini;
	FILE *filetgt=NULL;
	int i;
	char *key=NULL;
	int changeflag=0;
	int compare_len=0; //需要比较的字节长度
	
	if((source==NULL)||(target==NULL))
		{
			printf("传来文件名为NULL,退出ini_set_file\n");
			gtloginfo("传来文件名为NULL,退出ini_set_file\n");
			return -1;
		}
	
	srcini=iniparser_load(source);
	if (srcini==NULL) 
        {
             printf("cannot parse ini file [%s]",source);
			 gtloginfo("解析%s失败退出\n",source);	
             return(-1);
		}

	tgtini=iniparser_load_lockfile(target,1,&filetgt);
	for (i=0 ; i<srcini->size ; i++) //对于源文件的每个变量
	{
        if (srcini->key[i]==NULL)
            continue ;
        if (strchr(srcini->key[i], ':')==NULL)//没有:,为节名 
        	continue;
    
        //看目标文件是否有，如果没有则创建
        key=iniparser_getstring(tgtini,srcini->key[i],NULL);
		if(key==NULL)//没有，创建
        	{
        		changeflag++;
        		iniparser_setstr(tgtini,srcini->key[i],srcini->val[i]);
			   printf("<%s> %s NULL->%s\n",target,srcini->key[i],srcini->val[i]);
			gtloginfo("<%s> %s NULL->%s\n",target,srcini->key[i],srcini->val[i]);				
        	}
        else //有
        	{
        		
				//if(strncmp(key,srcini->val[i],strlen(srcini->val[i]))!=0)//有但不相同
				compare_len=(strlen(key)>strlen(srcini->val[i])) ? strlen(key) : strlen(srcini->val[i]); 

				if(strncmp(key,srcini->val[i],compare_len)!=0)//有但不相同
				{
					changeflag++;
					gtloginfo("<%s> %s %s->%s\n",target,srcini->key[i],key,srcini->val[i]);
					printf   ("<%s> %s %s->%s\n",target,srcini->key[i],key,srcini->val[i]);
					iniparser_setstr(tgtini,srcini->key[i],srcini->val[i]);
				}
        	}
       
	}

	if(changeflag!=0)
		gtloginfo("%s被设置(ini file set!)\n",target);
	iniparser_freedict(srcini);
	//save_inidict_file(IPMAIN_PARA_FILE,tgtini,&filetgt);
	save_inidict_file(target,tgtini,&filetgt);
	iniparser_freedict(tgtini);
	return 0;
}

#endif



int main(int argc,char **argv)
{

/*   处理命令行参数:
 *		启动时没有带参数则显示帮助信息；
 *		-h:输出帮助信息；
 *		-v:输出转换工具的版本信息；
 *		-s:后跟源文件
 *		-t:后跟目标文件*/

	gtopenlog("iniset");
	if((argc==3)&&(argv[1]!=NULL)&&(argv[2]!=NULL))
	{
		gtloginfo("start iniset %s->%s\n",argv[1],argv[2]);
		printf("start iniset %s->%s\n",argv[1],argv[2]);
		ini_set_file(argv[1],argv[2]);
		return 0;
	}
	else
	{
		if((argc==2)&&(strncmp(argv[1],"-v",2)==0))
		{
			print_version();
			return 0;
		}

		else
		{
			print_helpmsg();
			return 0;
		}

	}
}

