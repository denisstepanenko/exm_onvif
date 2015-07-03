/** 
	@file		rm_file_select.c
	@brief	删除指定目录下的子目录
*/
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include"filelib.h"

///rm_select_file.c中的宏定义,显示调试信息的开关
//#define DEG_RM_NO_CHDIR					(0)///<显示rm_file_select调试开关
//#define DEG_RM_DIS_DIR					(0)///<显示rm_file_select目录或文件开关 


/**
	@brief  rm_file_select.c中记录目录的信息
*/
struct DIR_RM_INFO
{
//int while_loop;					//while执行的计数
//int check_loop;					//check执行的计数
const char *rm_dir;				///<需要遍历的目录路径
const char *rm_file_path;			///<需要保留的文件路径
int retu_val;						///<返回值变量
};


/**
	@fn  	int ftw_no_chdir(const char *dir,struct DIR_RM_INFO *dir_rm_info)  
	@brief     此函数递归遍历指定目录 
	@param  dir指定要遍历的目录
	@param  *dir_rm_info目录结构信息
	@return   ftw_no_chdir成功执行返回0，否则返回-1

	由函数rm_file_select()调用，主要完成遍历指定目录，并
  	根据保留路径参数的值处理需要保留的目录，如果
  	函数rm_file_select()的file_path = NULL    则删除指定目录下所有
  	子目录和文件，否则保留指定目录下的子目录和文
  	件删除其他的目录和文件 。
*/
int ftw_no_chdir(const char *dir,struct DIR_RM_INFO *dir_rm_info)
{
	DIR *dp;						//定义一个子目录流
	struct stat statbuf;			//文件信息
	struct dirent *entry;			//指向目录数据项的指针
	int strr_length = 0;			//调用strrchr后返回的字符长度
	int work_dir_length = 0;		//work_dir目录的长度
	int remove_res = 0;
	//const char *work_dir;			//指向当前目录的指针
	char work_dir[1024];
	char temp_work_dir[1024];	//文件名长度限制在1024个字符内，可具体更改
	//char *mem_dir;				//每次递归调用内的目录记录
	char mem_dir[1024];			//每次递归调用内的目录记录

	remove_res = remove_res;
	//调用malloc会导致不可重入
	//mem_dir =(char *) malloc(strlen(dir));
	//mem_dir = strdup(dir);

	strcpy(mem_dir,dir);
	
#ifdef DEG_RM_NO_CHDIR
	printf("==entry-ftw_no_chdir==\n");
	printf("mem_dir===%s\n",mem_dir);
#endif	
	strcpy(work_dir,dir);
	//check_loop ++;
#ifdef DEG_RM_NO_CHDIR
	//printf("entry----check_loop==%d\n",check_loop);
	printf("entry---dir==%s\n",dir);
#endif

	if(dir_rm_info->rm_file_path != NULL)
	{
		if(strcmp(dir_rm_info->rm_file_path,mem_dir) == 0)
		{
#ifdef DEG_RM_NO_CHDIR
			printf("i can not delet the important dir\n");
#endif
			return 0;
		}
	}

	//取得目录信息
	if ((lstat(dir, &statbuf)) != 0) 
	{
#ifdef DEG_RM_NO_CHDIR
		printf("%s==error\n", dir);		
#endif
		dir_rm_info->retu_val = -1;
		return (dir_rm_info->retu_val);
	}
	
	//先判断是否为目录，是目录的话才有可能
	//递归调用函数check_dir()
	if (S_ISDIR(statbuf.st_mode) || S_ISLNK(statbuf.st_mode)) 
	{
#ifdef DEG_RM_DIS_DIR
		printf("DIR:\t\t%s\n",dir);
#endif
		//打开目录
		if(!(dp = opendir(dir))) 
		{
			printf("opendir error\n");
			dir_rm_info->retu_val = -1;
			return (dir_rm_info->retu_val);
		}

		//读取目录，返回下一个读取的目录进入点
		while ((entry = readdir(dp))!=NULL) 
		{
			//while_loop++;
#ifdef DEG_RM_NO_CHDIR
			//printf("while_loop===%d\n",while_loop);			
			printf("WORK_DIR==%s\n",work_dir);
			printf("DIR-NAME==%s\n",entry->d_name);
#endif
			
			//判断是否为"."    或 ".."，是的话就结束当前循环
			//然后继续判断此目录下的下一个目录的情况
			if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0)
			{
#ifdef DEG_RM_NO_CHDIR
				printf("DIR=.=..==\n");
#endif
				//结束当前循环,继续判断这个目录下的下一个目录或文件
				continue;
			}

			//执行到这里说明已经不是"."    或 ".."  而是一个目录
			//然后用这个目录作为参数递归调用
#ifdef DEG_RM_NO_CHDIR
			printf("before memset=work_dir==%s\n",work_dir);
			//printf("work==%d===temp==%x===\n",work_dir,temp_work_dir);
#endif
			memset(temp_work_dir,0,sizeof(temp_work_dir));			
#ifdef DEG_RM_NO_CHDIR
			printf("while(1)==work_dir==%s\n",work_dir);
#endif
			//修改目录，使当前目录指向他的一个子目录，添加他的子目录
			//名字到当前目录的后面
			strcat(temp_work_dir,work_dir);			
			strcat(temp_work_dir,"/");
			strcat(temp_work_dir,entry->d_name);
			//使用strdup是为了防止将数组temp_work_dir的内存地址直接传给指针work_dir，
			//那样的话当前面对temp_work_dir清零时就把指针也给清零了
			//work_dir = strdup(temp_work_dir);
			strcpy(work_dir,temp_work_dir);
#ifdef DEG_RM_NO_CHDIR			
			printf("here?\n");
			printf("before check_dir==work_dir==%s\n",work_dir);
#endif
			//递归调用，参数是刚才找到的除"."  和".. "外的目录或文件的名字
			//lint -e534
			ftw_no_chdir(work_dir,dir_rm_info);
			//lint +e534
			//下面的内容是调整当前目录路径，也就是返回到在调用check_dir前的目录
			//的上一层目录
			//lint -e1055
			work_dir_length = strlen(work_dir);
			//lint +e1055
#ifdef DEG_RM_NO_CHDIR
			printf("strrchr(%s)==%s\n",work_dir,strrchr(work_dir,'/'));
#endif
			//lint -e668
			strr_length = strlen(strrchr(work_dir,'/'));
			memset(temp_work_dir,0,sizeof(temp_work_dir));
			strncpy(temp_work_dir,work_dir,work_dir_length-strr_length);
			//work_dir = strdup(temp_work_dir);
			strcpy(work_dir,temp_work_dir);
			
#ifdef DEG_RM_NO_CHDIR				
			printf("after check_dir===work_dir==%s\n",work_dir);
#endif
		}
//		closedir(dp);
		
		if((closedir(dp))== ERROR_VAL)
		{
			printf("closedir error\n");
			//dir_rm_info->retu_val = -1;
			//return (dir_rm_info->retu_val);
		}

	}
	else
	{
#ifdef DEG_RM_DIS_DIR
		//执行到这里说明不是目录，是文件
		printf("document:\t%s\n",dir);
#endif
	}
#ifdef DEG_RM_NO_CHDIR
	//printf("=exit==(%d)=done.\n",check_loop);
	printf("mem_dir==%s\n",mem_dir);
#endif
	if(strcmp(dir_rm_info->rm_dir,mem_dir)!=0 )
	{				
		remove_res = remove(mem_dir);
	}
	else
	{
#ifdef DEG_RM_NO_CHDIR	
		printf("return the ==%s==\n",mem_dir);
#endif
	}

	//释放内存
	//free(mem_dir);
	return dir_rm_info->retu_val;
	
}





/**
	@fn 		int rm_file_select(const char *dir,const char *file_path)
	@brief   	rm_file_select接口函数  
	@param  *dir遍历的 目录
	@param  *file_path 需要保留的目录，如果file_path = NULL 则删除
			指定目录下的所有目录,   保留file_path指定的目录，删除其他
 			的目录和文件。
 	@return	ftw_no_chdir的返回值

 	函数内部再次调用ftw_no_chdir
 */

int rm_file_select(const char *dir,const char *file_path)
{
	struct DIR_RM_INFO *dir_rm_info;
	struct DIR_RM_INFO dir_info;
	
	int ftw_no_chdir_val = 0;
/*
	//初始化结构指针，使用了malloc
	//lint -e10
	if((dir_rm_info = (struct DIR_RM_INFO *)malloc(sizeof(struct DIR_RM_INFO)))==NULL)
	{
		ftw_no_chdir_val = -1;
		return ftw_no_chdir_val;
	}
	//lint +e10
*/

	//指针初始化
	dir_rm_info = &dir_info;
	
	//初始化结构成员
	dir_rm_info->rm_dir = dir;					//遍历目录初始化
	dir_rm_info->rm_file_path = file_path;		//保留目录初始化	 
	dir_rm_info->retu_val = 0;					//返回值初始化
	
	//调用遍历函数进行遍历
	ftw_no_chdir_val = ftw_no_chdir(dir,dir_rm_info);

	return ftw_no_chdir_val;
}



