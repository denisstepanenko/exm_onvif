/**
	@file 	ftw_sort.c
	@brief	递归遍历指定目录下的所有目录
*/
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include <ftw.h>
#include<errno.h>
#include"filelib.h"
#include<fixdisk.h>
//#include"./test_ftwsort/memwatch.h"



/*
ver:2.1
2007-12-07
a) 增加对fn的检查,如果为NULL则什么也不做;
b) 当ftw_sort()有错误时，如果err_fn不为NULL，就调用它;返回错误码
c) 保留根目录不删除


ver:2.0:
2007-06-20

在调用scandir()函数后加入了释放内存的函数
-----------------------------------------------------------------------
ver:1.0:
2007-05-21

返回非FN_RTURN_CONTINUE则关闭当前目录，否则这个目录
下的这个目录就没有人来关闭了，最后会导致在
ls /proc/pid/fd -l 时看到这个目录还在打开着，就不好了，
就会有错了.
-----------------------------------------------------------------------
*/

///ftw_sort.c中显示调试信息的开关
//#define DEG_FTW_NO_CHDIR					(0)	///<显示ftw_sort调试信息开关
//#define DEG_FTW_DIS_DIR					(1)	///<显示ftw_sort遍历的目录
//#define DEG_FTW_NO_CHDIR_ERROR			(1)  ///<用于测试当scandir出错时的开关

/**
	@brief  rm_file_select.c中记录目录的信息
*/
struct DIR_INFO
{
//int while_loop;					//while执行的计数
//int check_loop;					//check执行的计数
const char *rm_dir;				///<需要遍历的目录路径
const char *rm_file_path;			///<需要保留的文件路径
char retu_flag;					///<返回标志
char rm_empty_flag;				///<删除空目录的标志
char fn_continue;					///<fn继续访问下一平行节点标志
int retu_val;						///<返回值变量
long ftw_dir_no;					///<记录目录层数
DIR *pre_dp;					///<上层目录的目录流
};

/**
	@brief  	此函数递归遍历指定的目录，由ftw_sort()调用
	@param  *dir    要遍历的目录名
	@param  (*fn)      有新文件或目录时调用的函数,fn返回非0值时退出遍历,  ftw_sort返
			回fn  返回的值
//	@param  dir_depth 最大遍历目录深度，如果超过这个深度ftw_sort将不再往下一层
//			遍历
	@param  sort_mode    遍历时的排序模式,取值为FTW_SORT_ALPHA...
	@param  rm_empty_dir  1:如果发现空目录则删除0:不删除空目录
	@param  (*err_fn)  当执行scandir访问目录错误时调用的函数，有错误发生
			后返回 err_fn返回的值,终止遍历,dir_file表示出错的目录或
			文件,errnum表示错误号，其值为出错时的errno
	@param  *dir_info  记录目录信息的结构
	@return	遍历中断则返回fn()函数的返回值,全部遍历完则返回 0.若有
			 错误发生则返回-1(如scandir出错),错误码见errno
*/

int check_dir_user(const char *dir,
					int (*fn)(const char *file,const struct stat *sb,int flag,void *user_data_tmp),
					void *user_data_tmp,
					int sort_mode,
					int rm_empty_dir,
					int (*err_fn)(const char *dir_file,	int errnum),
					struct  DIR_INFO  *dir_info
			)

{	
	DIR *dp;						//定义一个子目录流
	struct stat statbuf;			//文件信息
	struct dirent *entry;			//指向目录数据项的指针
	struct dirent **namelist;
	int strr_length = 0;			//调用strrchr后返回的字符长度
	int work_dir_length = 0;		//work_dir目录的长度
	int remove_res = 0;
	int dir_total = 0;				//当前目录下用scandir读取到的总子目录数
	int dir_total_bak=0;
	int file_order = 0;				//记录namelist的下标
	int dir_no = 0;				//记录每层目录下的文件数
	int fn_ret_val = 0;
	int check_dir_val = 0;
	
	//const char *work_dir;				//指向当前目录的指针
	char work_dir[256];
	char temp_work_dir[256];	//文件名长度限制在1024个字符内，可具体更改
	//char *mem_dir;				//每次递归调用内的目录记录
	char mem_dir[256];			//每次递归调用内的目录记录

	//防止编译器警告
	remove_res = remove_res;
//	dir_depth = dir_depth;
	sort_mode = sort_mode;
	rm_empty_dir = rm_empty_dir;
	file_order = file_order;
	dir_no = dir_no;
	
	if(dir_info->retu_flag == 1)
	{
		return (dir_info->retu_val);
	}

	strcpy(mem_dir,dir);
	
#ifdef DEG_FTW_NO_CHDIR
	printf("mem_dir==%s\n",mem_dir);
#endif	
	strcpy(work_dir,dir);

	//dir_info->check_loop ++;			//测试fn返回值时使用
	
#ifdef DEG_FTW_NO_CHDIR
	//printf("entry----check_loop==%d\n",check_loop);

	printf("entry---dir==%s\n",dir);
#endif	

#if 0
	if(dir_info->rm_file_path != NULL)
	{
		if(strcmp(dir_info->rm_file_path,mem_dir) == 0)
		{
#ifdef DEG_FTW_NO_CHDIR
			printf("i can not delet the important dir\n");
#endif
			return 0;
		}
	}
#endif

	//取得目录信息
	if ((lstat(dir, &statbuf)) != 0) 
	{
#ifdef DEG_FTW_NO_CHDIR
		printf("%s==error\n", dir);		
#endif
		//读取stat错误;
		//zw-mod-20071207---->
		if(fn!=NULL)
		{
			fn_ret_val = fn(dir,&statbuf,FTW_NS,user_data_tmp);
			//printf("fn--%d\n",__LINE__);
			if(fn_ret_val !=0)
			{
				dir_info->retu_val = fn_ret_val; 	
				return (dir_info->retu_val);
			}
		}	
		//zw-mod-20071297<----

		//dir_info->retu_val = -1;
		//return (dir_info->retu_val);
	}
	
	//先判断是否为目录，是目录的话才有可能
	//递归调用函数check_dir()
	if (S_ISDIR(statbuf.st_mode)) 
	{
#ifdef DEG_FTW_DIS_DIR
		printf("DIR:\t\t%s\n",dir);		
#endif

		dir_info->ftw_dir_no++;
 #ifdef DEG_FTW_NO_CHDIR
		printf("entry--->dir_info->ftw_dir_no=[%ld]\n",dir_info->ftw_dir_no);
 #endif
 
		//检查遍历的目录是否超过最大深度
//		if(dir_info->ftw_dir_no > dir_depth)
		if(dir_info->ftw_dir_no > DIR_DEPTH)
		{
#ifdef DEG_FTW_NO_CHDIR
			printf("over the limit\n");
 #endif
			dir_info->ftw_dir_no--;
			return (dir_info->retu_val);
		}

		//if(dir_info->check_loop > 40)		
		//fn_ret_val = fn(dir,&statbuf,23);
		
		//zw-mod-20071207---->
		if(fn!=NULL)
		{
			fn_ret_val = fn(dir,&statbuf,FTW_D,user_data_tmp);
			//printf("fn--%d\n",__LINE__);
			if(fn_ret_val !=0)
			{
				//closedir(dir_info->pre_dp);
				dir_info->retu_val = fn_ret_val;
				return (dir_info->retu_val);
			}
		}
		//zw-mod-20071207<----

		
		//打开目录
		if(!(dp = opendir(dir))) 
		{	
			//不可读取的目录
			//zw-mod-20071207---->
			if(fn!=NULL)
			{
				fn_ret_val = fn(dir,&statbuf,FTW_DNR,user_data_tmp);	
				//printf("fn--%d\n",__LINE__);
				if(fn_ret_val !=0)
				{	
					printf("opendir error\n");
					dir_info->retu_val = fn_ret_val;
					return (dir_info->retu_val);
				}				
			}
			//zw-mod-20071207<----
			
			//dir_info->retu_val = -1;
			//return (dir_info->retu_val);
		}

		dir_info->pre_dp = dp;

		//对当前目录下的子目进行排序
		if(sort_mode==1)
		{
#ifdef DEG_FTW_NO_CHDIR_ERROR
			if(dir_info->ftw_dir_no == 3) dir = "test.";
#endif
			errno = 0;
			//调用scandir进行排序
			if((dir_total = scandir(dir,&namelist,0,alphasort))<0)
			{				
				dir_total_bak = dir_total;
				printf("scandir <0\n");
				
				//置位返回标志
				dir_info->retu_flag = 1;
				
				//调用错误处理函数
				//dir_info->retu_val = err_fn(dir,dir_total);

				if(err_fn!=NULL)
				{
					err_fn(dir,errno);
				}
				//返回错误值
				dir_info->retu_val=-errno;
				return (dir_info->retu_val);
			}
			dir_total_bak=dir_total;
		}	

		//将由scandir返回的当前目录下的文件个数
		//赋值给局部变量保存
		dir_no = dir_total;		

#ifdef DEG_FTW_NO_CHDIR
		//显示每层目录下的目录,判断是否为空目录
		printf("dir_total = %d\n",dir_total);	
		for(file_order=0;file_order<dir_total;file_order++)
		{
			printf("dir_name==\t%s\n",namelist[file_order]->d_name);
		}
#endif		
		//读取目录，返回下一个读取的目录进入点
		//while ((entry = readdir(dp))!=NULL) 
		for(file_order=0;file_order<dir_total;file_order++)
		{			
			//printf("DIR-NAME==%s\n",entry->d_name);
			entry = namelist[file_order];
			//while_loop++;
#ifdef DEG_FTW_NO_CHDIR
			//printf("while_loop===%d\n",while_loop);			
			printf("WORK_DIR==%s\n",work_dir);
			printf("DIR-NAME==%s\n",entry->d_name);
#endif
			
			//判断是否为"."    或 ".."，是的话就结束当前循环
			//然后继续判断此目录下的下一个目录的情况
			if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0)
			{
#ifdef DEG_FTW_NO_CHDIR
				printf("DIR=.=..==\n");
#endif
				//结束当前循环,继续判断这个目录下的下一个目录或文件
				continue;
			}

			//执行到这里说明已经不是"."    或 ".."  而是一个目录
			//然后用这个目录作为参数递归调用
#ifdef DEG_FTW_NO_CHDIR
			printf("before memset=work_dir==%s\n",work_dir);
			//printf("work==%d===temp==%x===\n",work_dir,temp_work_dir);
#endif
			memset(temp_work_dir,0,sizeof(temp_work_dir));			
#ifdef DEG_FTW_NO_CHDIR
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
#ifdef DEG_FTW_NO_CHDIR			
			printf("here?\n");
			printf("before check_dir==work_dir==%s\n",work_dir);
#endif
			//递归调用，参数是刚才找到的除"."  和".. "外的目录或文件的名字
			//lint -e534
			check_dir_val = check_dir_user(	work_dir,
							fn,
							//dir_depth,
							user_data_tmp,
							sort_mode,
							rm_empty_dir,
							err_fn,
							dir_info
							);
#if 0
			//对返回值进行判断，是否直接返回
			if(check_dir_val == FN_RETURN_CONTINUE )
			{
				//dir_info->retu_val = check_dir_val;
				return (0);
			}
#endif


			
			if((check_dir_val != FN_RETURN_CONTINUE) && (check_dir_val != 0))
				{	
					/*	
						返回非FN_RTURN_CONTINUE则关闭当前目录，否则这个目录
						下的这个目录就没有人来关闭了，最后会导致在
						ls /proc/pid/fd -l 时看到这个目录还在打开着，就不好了，
						就会有错了.2007-05-21
					*/
					closedir(dp);					
					dir_info->retu_flag = 1;
					dir_info->retu_val = check_dir_val;
					return (dir_info->retu_val);
				}


			//lint +e534			
			if(dir_info->rm_empty_flag == 1)
			{
				dir_no --;

				//空目录删除标志重新初始化为0
				dir_info->rm_empty_flag = 0;
			}

			//下面的内容是调整当前目录路径，也就是返回到在调用check_dir前的目录
			//的上一层目录
			//lint -e1055
			work_dir_length = strlen(work_dir);
			//lint +e1055
#ifdef DEG_FTW_NO_CHDIR
			printf("strrchr(%s)==%s\n",work_dir,strrchr(work_dir,'/'));
#endif
			//lint -e668
			strr_length = strlen(strrchr(work_dir,'/'));
			memset(temp_work_dir,0,sizeof(temp_work_dir));
			strncpy(temp_work_dir,work_dir,work_dir_length-strr_length);
			//work_dir = strdup(temp_work_dir);
			strcpy(work_dir,temp_work_dir);
			
#ifdef DEG_FTW_NO_CHDIR				
			printf("after check_dir===work_dir==%s\n",work_dir);
#endif
		
		}

		/*
			下面的作用是用来释放由scandir调用的参数namelist所
			占用的内存空间，否则在执行ftw_sort的时候，占用
			系统资源的内存在不停的增长，增长，增长。
			2007-06-20
		*/	
#if 1
		while(dir_total_bak--)
		{
			free(namelist[dir_total_bak]);
		}
		free(namelist);
#endif
 
		
#if 1		
		if((closedir(dp))== ERROR_VAL)
		{
			printf("closedir error\n");
			//dir_info->retu_val = -1;
			//return (dir_info->retu_val);
		}
#endif		
			dir_info->ftw_dir_no--;

	}
	else
	{
#ifdef DEG_FTW_DIS_DIR
		//执行到这里说明不是目录，是文件
		printf("document:\t%s\n",dir);
#endif
		
		//不是目录是一般文件
		//zw-mod-20071207---->
		if(fn!=NULL)
		{
			fn_ret_val = fn(dir,&statbuf,FTW_F,user_data_tmp);
			//printf("fn--%d\n",__LINE__);
			if(fn_ret_val !=0)
			{
				dir_info->retu_val = fn_ret_val;
				return (dir_info->retu_val);
			}
		}
		//zw-mod-20071207<----
		
	}
	
#ifdef DEG_FTW_NO_CHDIR
	//printf("=exit==(%d)=done.\n",check_loop);
	printf("mem_dir==%s\n",mem_dir);
#endif
	if(strcmp(dir_info->rm_dir,mem_dir)!=0 )
	{				
		//remove_res = remove(mem_dir);
	}
	else
	{
#ifdef DEG_FTW_NO_CHDIR	
		printf("return the ==%s==\n",mem_dir);
#endif
	}

#ifdef DEG_FTW_NO_CHDIR
	printf("leaving<----dir_info->ftw_dir_no=[%ld]\n",dir_info->ftw_dir_no);
#endif

	if((dir_no == 2) && (rm_empty_dir == 1))
	{	
		dir_info->rm_empty_flag = 1;
		
		//保留根目录zw-mod-20071207---->
		if(dir_info->ftw_dir_no!=0)
		{
#ifdef DEG_FTW_NO_CHDIR
			printf("leaving[%ld],,,remove=%s\n",dir_info->ftw_dir_no,dir);
#endif
			remove(dir);
		}
		else
		{
#ifdef DEG_FTW_NO_CHDIR
			printf("i am 0\n");
#endif
		}
		
		//zw-mod-20071207<----
	}	

	//释放内存
	//free(mem_dir);
	//printf("dir=%s\n",dir);
	return dir_info->retu_val;
	 
}





/**
	@brief	接口函数，内部调用check_dir()	
	@param  *dir    要遍历的目录名
	@param  (*fn)      有新文件或目录时调用的函数,fn返回非0值时退出遍历,  ftw_sort返
					回fn  返回的值
	@param  dir_depth 最大遍历目录深度，如果超过这个深度ftw_sort将不再往下一层
			遍历
	@param  sort_mode    遍历时的排序模式,取值为FTW_SORT_ALPHA...
	@param  rm_empty_dir  1:如果发现空目录则删除0:不删除空目录
	@param  (*err_fn)  当执行scandir访问目录错误时调用的函数，有错误发生
			后返回 err_fn返回的值,终止遍历,dir_file表示出错的目录或
			文件,errnum表示错误号，其值为出错时的errno
	@return	遍历中断则返回fn()函数的返回值,全部遍历完则返回 0.若有
			错误发生则返回-1(如scandir出错),错误码见errno

*/
#if 0
int ftw_sort_user(	const char *dir,
					int (*fn)(const char *file,const struct stat *sb,int flag),
					int dir_depth,
					int sort_mode,
					int rm_empty_dir,
					int (*err_fn)(const char *dir_file,	int errnum)
			)
#endif

int ftw_sort_user(      const char *dir,
                                        int (*fn)(const char *file,const struct stat *sb,int flag,void *user_data),
                                        void *user_data,
                                        int sort_mode,
                                        int rm_empty_dir,
                                        int (*err_fn)(const char *dir_file,     int errnum)
                        )

{
	struct DIR_INFO *dir_info;				//定义指向结构的指针
	struct DIR_INFO ftw_dir_info;			//定义一个 目录信息的结构
	
	int ftw_no_chdir_val = 0;					//返回值初始化

	//避免编译器警告
	//dir_depth = dir_depth;				
	sort_mode = sort_mode;
	rm_empty_dir = rm_empty_dir;	
/*
	//初始化结构指针
	//lint -e10
	if((dir_info = (struct DIR_RM_INFO *)malloc(sizeof(struct DIR_RM_INFO)))==NULL)
	{
		ftw_no_chdir_val = -1;
		return ftw_no_chdir_val;
	}
	//lint +e10
*/

	int while_cnt=0;

	//指针初始化
	dir_info = &ftw_dir_info;	

	//初始化结构成员
	dir_info->rm_dir = dir;					//遍历目录初始化	 
	dir_info->retu_val = 0;					//返回值初始化	
	dir_info->ftw_dir_no = 0;				//记录遍历的目录深度
	dir_info->retu_flag = 0;					//初始化返回标志
	dir_info->rm_empty_flag = 0;			//初始化删除空目录标志	
	dir_info->fn_continue = 0;				//初始化fn访问下一个节点的标志
	dir_info->pre_dp=NULL;
	//dir_info->check_loop = 0;				//测试fn返回值时使用
	
	if(err_fn==NULL)
	{
		return -1;
	}
	

	printf("test ftw_sort_user\n");


	//调用遍历函数进行遍历
	ftw_no_chdir_val = check_dir_user(	dir,
							fn,
							//dir_depth,
							user_data,
							sort_mode,
							rm_empty_dir,
							err_fn,
							dir_info
							);

	printf("finish the dir=%s------%d\n",dir,while_cnt);

	
	return ftw_no_chdir_val;
}

#if 0
/*wsyadd, 用于处理调用ftw_sort出错时，需要修理磁盘的情况*/
int  fix_disk_ftw(const char *dirname, int err)
{
	if(dirname==NULL)
		return -EINVAL;
	
	gtlogerr("ftw 出错, %s, %s\n",dirname, strerror(err));
	fix_disk(dirname,err);
	return FN_DISK_ERR;
}
#endif
