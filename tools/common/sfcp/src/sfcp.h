#ifndef __SFCP_H
#define	__SFCP_H

//版本
#define SFCP_VERSION		("2.02")
//ver:2.02 2008-08-27 修改判断符号链接文件是否存在的条件
//ver:2.01 2008-08-27 修改复制失败时记录日志的提示信息
//ver:2.00 2008-08-25 修改版本号；拷贝时复制3次，校验3次都不合格时退出；
//ver:1.00 2008-08-14 建立源文件；


#define	DST_FILENAME_MAX_LEN		(512)		//目标文件名最大长度
#define	SRC_FILENAME_MAX_LEN		(512)		//源文件名最大长度
#define	RD_BUF_MAX_LEN			(8192)		//从源文件读的缓冲区最大长度
#define	FILENAME_MAX_LEN			(128)		//文件名的最大长度
#define LNK_PATH_MAX_LEN		(128)		//link-path-max-len
#define DIR_MAX_DEEP			(5000)		//遍历目录的最大深度

//保存文件目录信息的结构
typedef struct
{
	char src_path[SRC_FILENAME_MAX_LEN];			//源文件路径
	char dst_path[DST_FILENAME_MAX_LEN];			//目的文件路径
	char dst_path_bak[DST_FILENAME_MAX_LEN];		//目的路径的备份
	//char sub_path[SRC_FILENAME_MAX_LEN];		//相对路径
	//char old_sub_path[SRC_FILENAME_MAX_LEN];		//src的上一次进入的目录
	char cur_dst_path[DST_FILENAME_MAX_LEN];		//当前的目标文件路径
	//char file_name[FILENAME_MAX_LEN];
	char sub_path_nh[SRC_FILENAME_MAX_LEN];		//没有源头的目录
	char sub_path_nf[SRC_FILENAME_MAX_LEN];		//没有文件尾的目录
	char sub_path_nf_old[SRC_FILENAME_MAX_LEN];	//上次的目录
	unsigned long src_file_len;						//源文件长度
	unsigned long dst_file_len;						//目标文件长度
	int	sub_file_total;							//子目录下的文件数
	int	arg_total;
	int	creat_ndir_flag;
	int	src_file_flag;
	int	opt_r_flag;				//命令行选项r
	int	opt_f_flag;				//f选项标志
	int	opt_d_flag;				//d选项标志
	int 	opt_u_flag;				//显示复制标志
}SFCP_PATH_T;







#endif
