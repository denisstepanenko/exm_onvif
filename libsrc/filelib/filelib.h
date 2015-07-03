/**
	@file 	filelib.h
	@brief	rm_file_select() 和 ftw_sort()的头文件
*/

#ifndef LIBFILE_H
#define LIBFILE_H

#include<sys/stat.h>
#include "gtlog.h"

#define FTW_SORT_ALPHA				(1)		///<按字母顺序排序(默认方式)

#define FN_RETURN_CONTINUE		(1)		///<fn返回此值则继续
#define FN_SPACE_ENOUGH			2	//fn返回此值表示因可用空间足够而返回
#define FN_DISK_ERR				3	//fn返回此值表示因磁盘错误返回

#define DIR_DEPTH			(100)	//目录深度

///  @brief  linux库函数
extern char *get_current_dir_name(void);


/** 
	@brief		在ftw_sort()中调用的外部定义的函数，由用户自己定义
	@param		*dir_file  当前路径目录
	@param		errnum 错误号
	@return		用户自己定义，返回整形值

	当执行scandir访问目录错误时调用的函数，有错误发生
	后返回 err_fn返回的值,终止遍历,dir_file表示出错的目录或
	文件,errnum表示错误号，其值为出错时的errno
*/
extern int err_fn(const char *dir_file,int errnum);


extern int err_fn_user(const char *dir_file,int errnum);

/** 
	@brief		在ftw_sort()中调用的外部定义的函数，由用户自己定义
	@param		file 当前路径
	@param 		*sb  stat结构指针
	@param 		flag 旗标，有以下几种可能 FTW_F---一般文件 
					FTW_D--一般目录  
					FTW_DNR---不可读取得目录，此目录以下将不被遍历 
					FTW_SL----符号连接 
					FTW_NS----无法取得的stat结构数据，有可能是权限问题
*/
extern int fn(const char *file,const struct stat *sb,int flag);

extern int fn_user(const char *file,const struct stat *sb,int flag,void *user_data);



/**
	@brief		接口函数，内部调用check_dir()	
	@param		*dir    要遍历的目录名
	@param		(*fn)      有新文件或目录时调用的函数,fn返回非0值时退出遍历,  ftw_sort返
				回fn  返回的值
	@param		dir_depth 最大遍历目录深度，如果超过这个深度ftw_sort将不再往下一层
				遍历
	@param		sort_mode    遍历时的排序模式,取值为FTW_SORT_ALPHA...
	@param 		rm_empty_dir  1:如果发现空目录则删除0:不删除空目录
	@param		(*err_fn)  当执行scandir访问目录错误时调用的函数，有错误发生
				后返回 err_fn返回的值,终止遍历,dir_file表示出错的目录或
				文件,errnum表示错误号，其值为出错时的errno
	@return		遍历中断则返回fn()函数的返回值,全部遍历完则返回 0.若有
				错误发生则返回-1(如scandir出错),错误码见errno

*/
int  ftw_sort(	const char *dir,
					int (*fn)(const char *file,const struct stat *sb,int flag),
					int dir_depth,
					int sort_mode,
					int rm_empty_dir,
					int (*err_fn)(const char *dir_file,	int errnum)
					
			);


/**
	@brief		接口函数，内部调用check_dir()	
	@param		*dir    要遍历的目录名
	@param		(*fn)      有新文件或目录时调用的函数,fn返回非0值时退出遍历,  ftw_sort返
				回fn  返回的值
			user_data	用户数据
	@param		sort_mode    遍历时的排序模式,取值为FTW_SORT_ALPHA...
	@param 		rm_empty_dir  1:如果发现空目录则删除0:不删除空目录
	@param		(*err_fn)  当执行scandir访问目录错误时调用的函数，有错误发生
				后返回 err_fn返回的值,终止遍历,dir_file表示出错的目录或
				文件,errnum表示错误号，其值为出错时的errno
	@return		遍历中断则返回fn()函数的返回值,全部遍历完则返回 0.若有
				错误发生则返回-1(如scandir出错),错误码见errno

*/
int ftw_sort_user(      const char *dir,
                                        int (*fn)(const char *file,const struct stat *sb,int flag,void *user_data),
                                        void *user_data,
                                        int sort_mode,
                                        int rm_empty_dir,
                                        int (*err_fn)(const char *dir_file,     int errnum)
                        );

///rm_file_select.c文件的宏定义
#define ERROR_VAL					(-1)///<rm_file_select错误值 

/**
	@fn		int rm_file_select(const char *dir,const char *file_path)
	@brief	rm_file_select接口函数	
	@param	*dir遍历的 目录
	@param	*file_path 需要保留的目录，如果file_path = NULL 则删除
	@return	ftw_no_chdir的返回值
	
	函数内部再次调用ftw_no_chdir
	指定目录下的所有目录,   保留file_path指定的目录，删除其他
	的目录和文件。
  
 */
int rm_file_select(const char * dir, const char * file_path);






#endif
