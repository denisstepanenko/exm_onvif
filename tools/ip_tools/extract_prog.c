/**
 	文件名称: extract_prog.c
 	文件描述: 释放程序包到指定的文件夹
 	修改时间: 08.08.26~08.08.28 
 	修改日志: 修改main主函数，增加一条可解压缩的路径
 */
#include <stdio.h>
#include <stdlib.h>                            
#include <unistd.h>            
#include <gtlog.h>                                                                                                                                       
#include <errno.h>    

/**
	记录每次修改后的版本号

	0.03		修改宏定义，修改注释
			使得程序看着规范，易懂
			
	0.02		在第一版上去掉多余的变量，版本使用宏定义
			解决了过多浪费内存，版本修改慢问题
			
	0.01		第一版
*/
#define VERION ("0.03")

static const char	prog_pkt[]      = "/conf/ip1004.tar.lzma";			///< 打包好的设备程序包文件名
static const char	backup_pkt[]  = "/hqdata/hda1/firmware/ip1004.tar.lzma";	///< 备份的程序文件包
static const char	target_dir[]    = "/";	///< 压缩包自己带路径,解压缩的目标目录


int main(int argc,char *argv[])
{
	int ret;					 ///< system 函数返回值，0为真
	int err;					 ///< errno 返回值   
	char cmdbuf[256];  

	printf( "启动extract_proc ( ver : %s )\n", VERION);	 ///<显示启动时的版本号

	gtopenlog( "extract_prog" );	 ///< 记录日志
	gtloginfo( "启动extract_prog ( ver : %s )", VERION);	///< 记录日志版本
	
	sprintf( cmdbuf,"tar axvf %s >/tmp/extract.txt 2>>/tmp/extract.txt ", prog_pkt );
	printf( "prog_cmd = %s \n", cmdbuf );
	
	errno = 0;
	ret = system( cmdbuf);
	err = errno;
	printf( "prog_ret = %d, prog_errno = %d !! \n", ret, err );
	
	if( ret!= 0 )
	{
		gtlogerr(" 源路径程序包解压缩失败, prog_ret = %d, prog_errno = %d", ret, err );	///< 记录错误日志

		printf( "源数据毁坏，从备份数据读取\n" );		
		sprintf( cmdbuf,"tar axvf %s >/tmp/extract.txt 2>>/tmp/extract.txt ", backup_pkt);
		printf( "backup_cmd = %s \n", cmdbuf );
		
		errno = 0;
		ret = system( cmdbuf );		///< 从备份路径解压缩数据
		err = errno;
		printf( "bakckup_ret = %d , backup_errno = %d !! \n", ret, err );		
		if( ret != 0 )
		{
			gtlogerr( "备份路径程序包解压缩失败,backup_ret = %d, backup_errno = %d", ret, err );
			printf( "数据均已损毁，无法解压缩\n" );                     
		}
		
	}



	exit(0);
}



