#include<getopt.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>
#include<ftw.h>
#include<sys/types.h>
#include<dirent.h>
#include<utime.h>
#include<filelib.h>
#include<fixdisk.h>
#include<gtlog.h>
#include<devinfo.h>
#include<iniparser.h>
#include<errno.h>
#include"sfcp.h"
#include"tab_crc32.h"


SFCP_PATH_T sf_dir;
int errr;

/**********************************************************************************
 *      函数名: print_help()
 *      功能:   打印帮助信息
 *      输入:   无
 *      输出:   无
 *      返回值: 无
 *********************************************************************************/
void print_help(void)
{
	printf("\n");
	printf("  用法：\n");
	printf("	./sfcp [选项]... 源... 目录\n");
	printf("   或:	./sfcp [选项]... 源    目的\n\n");
	printf("	-r:	复制目录及目录内的所有项目\n");
	printf("	-f:	在做删除处理时不给出提示信息\n");
	printf("	-d:	拷贝时保留链接\n");
	printf("	-u:	显示复制过程\n");
	printf("	--help:		显示此帮助信息并退出\n");
	printf("	--version:	输出版本信息并退出\n\n");
	exit(1);
}


/**********************************************************************************
 *      函数名: print_ver()
 *      功能:   打印版本
 *      输入:   无
 *      输出:   无
 *      返回值: 无
 *********************************************************************************/
void print_ver(void)
{
	printf("\nversion : %s\n\n",SFCP_VERSION);
	exit(1);
}


/**********************************************************************************
 *      函数名: get_filename_from_dir()
 *      功能:   从路径中解析文件名
 *      输入:   dir			路径
 *				filename	文件名
 *      输出:   无
 *      返回值: 
 *********************************************************************************/
int get_filename_from_dir(char *dir,char *filename)
{
	char *ret=NULL;
	
	if(dir==NULL || filename==NULL)
	{
		return -1;
	}

	// 目录的路径+文件名
	ret=strrchr(dir,'/');
	if(ret==NULL)
	{
		//当在本地路径时，直接是文件名
		memcpy(filename,dir,strlen(dir));
	}
	else
	{
		memcpy(filename,ret,strlen(ret));
	}


	
	return 0;
}


/**********************************************************************************
 *      函数名: set_file_len()
 *      功能:   获取文件长度
 *      输入:   sf_p	指向SFCP_PATH_T的指针
 *			   len	文件长度
 *      输出:   无
 *      返回值: 无
 *********************************************************************************/
void get_file_len(SFCP_PATH_T *sf_p,int len)
{
	sf_p->src_file_len=len;
}


/**********************************************************************************
 *      函数名: check_files_size()
 *      功能:   校验源文件和目标文件大小
 *      输入:   src_file		源文件名
 *			   dst_file		目标文件
 *      输出:   无
 *      返回值: 正确返回0，负值错误码
 *********************************************************************************/
int check_files_size(const char *src_file,const char *dst_file)
{
	struct stat buf;
	unsigned src_size;
	unsigned dst_size;
	int ret;

	//获取源文件状态
	memset(&buf,0,sizeof(struct stat));
	errno=0;
	ret=stat(src_file,&buf);
	if(ret!=0)
	{
		errr=errno;
		printf("校验时获取源文件[%s]状态错误[%s]line:%d\n",src_file,strerror(errr),__LINE__);
		gtlogerr("校验时获取源文件[%s]状态错误[%s]line:%d\n",src_file,strerror(errr),__LINE__);
		return -1;
	}
	src_size=buf.st_size;
	//printf("源文件[%s:%d-Bytes]\n",src_file,src_size);

	//获取目标文件状态
	ret=0;
	memset(&buf,0,sizeof(struct stat));
	errno=0;
	ret=stat(dst_file,&buf);
	if(ret!=0)
	{
		errr=errno;
		printf("校验时获取目标文件[%s]状态错误[%s]line:%d\n",dst_file,strerror(errr),__LINE__);
		gtlogerr("校验时获取目标文件[%s]状态错误[%s]line:%d\n",dst_file,strerror(errr),__LINE__);
		return -1;
	}
	dst_size=buf.st_size;
	//printf("目标文件[%s:%d-Bytes]\n",dst_file,dst_size);
	
	if(src_size==dst_size)
	{
		//printf("源文件和目标文件大小校验正确\n");
	}
	else
	{
		printf("源文件[%s:%d-Bytes]与目标文件[%s:%d-Bytes]大小不同.\n",src_file,src_size,dst_file,dst_size);
		gtlogerr("源文件[%s:%d-Bytes]与目标文件[%s:%d-Bytes]大小不同.\n",src_file,src_size,dst_file,dst_size);
		return -1;
	}

	return 0;
}


/**********************************************************************************
 *      函数名: check_file_crc32()
 *      功能:   用CRC校验文件内容
 *      输入:   src_file		源文件名
 *			   dst_file		目标文件
 *      输出:   无
 *      返回值: 正确返回0，负值错误码
 *********************************************************************************/
 int check_file_crc32(char *src,char *dst)
{
	FILE *src_stream=NULL;
	FILE *dst_stream=NULL;
	char read_buf[RD_BUF_MAX_LEN];//RD_BUF_MAX_LEN
	int rd_len_cnt;
	int rd_len_src;
	int rd_len_dst;
	int ret;
	struct stat stbuf;
	unsigned long crc_src;
	unsigned long crc_dst;

	if(src==NULL || dst==NULL)
	{
		printf("src==null,dst==null\n");
		return -1;
	}

	//打开源文件
	errno=0;
	src_stream=fopen(src,"r");
	if(src_stream==NULL)
	{
		errr=errno;
		printf("打开文件[%s]失败[%s]line[%d]\n",src,strerror(errr),__LINE__);
		gtlogerr("打开文件[%s]失败[%s]line[%d]\n",src,strerror(errr),__LINE__);
		return -1;
	}
	
	//打开目标文件
	errno=0;
	dst_stream=fopen(dst,"r");
	if(dst_stream==NULL)
	{
		errr=errno;
		printf("建立文件%s失败[%s]line[%d]\n",dst,strerror(errr),__LINE__);
		gtlogerr("建立文件%s失败[%s]line[%d]\n",dst,strerror(errr),__LINE__);
		fclose(src_stream);
		return -1;
	}

	//获取目标文件大小
	memset(&stbuf,0,sizeof(struct stat));
	errno=0;
	ret=lstat(src,&stbuf);
	if(ret<0)
	{
		errr=errno;
		printf("获取文件[%s]状态失败[%s]line[%d]\n",src,strerror(errr),__LINE__);
		gtlogerr("获取文件[%s]状态失败[%s]line[%d]\n",src,strerror(errr),__LINE__);
		fclose(src_stream);
		fclose(dst_stream);
		return -1;
	}
	
	//开始拷贝文件	
	rd_len_cnt=0;
	while(1)
	{
		//读源文件
		memset(read_buf,0,sizeof(read_buf));
		errno=0;
		rd_len_src=fread((void *)read_buf,1,sizeof(read_buf),src_stream);
		//printf("校验读出的文件长度rd_len=%d\n",rd_len);
		if(rd_len_src>=0)
		{
			rd_len_cnt+=rd_len_src;
		}
		else
		{
			errr=errno;
			printf("校验读取源文件[%s]错误[%s]line[%d[\n",src,strerror(errr),__LINE__);
			gtlogerr("校验读取源文件[%s]错误[%s]line[%d[\n",src,strerror(errr),__LINE__);
			fclose(src_stream);
			fclose(dst_stream);
			return -1;
		}			
		//fflush(src_stream);
		crc_src=0;
		if(rd_len_src!=0)
		{
			crc_src=tab_crc32((const unsigned char *)read_buf,rd_len_src);
			//printf("源文件CRC校验值[%ld]\n",crc_src);
		}
		
		//读目标文件
		memset(read_buf,0,sizeof(read_buf));
		errno=0;
		rd_len_dst=fread((void *)read_buf,1,sizeof(read_buf),dst_stream);
		//printf("写文件的长度wr_len=%d\n",wr_len);
		if(rd_len_dst<0)
		{
				errr=errno;
			printf("校验读取目的文件[%s]错误[%s]line:[%d]\n",src,strerror(errr),__LINE__);
			gtlogerr("校验读取目的文件[%s]错误[%s]line:[%d]\n",src,strerror(errr),__LINE__);
			fclose(src_stream);
			fclose(dst_stream);
			return -1;
		}	
		//fflush(dst_stream);
		crc_dst=0;
		if(rd_len_src==0)
		{
			if(rd_len_dst!=0)
			{
				printf("CRC校验源文件[%s]为0，目标文件[%s]读长度不为0，错误\n",src,dst);
				gtlogerr("CRC校验源文件[%s]为0，目标文件[%s]读长度不为0，错误\n",src,dst);
				return -1;
			}
		}
		
		if(rd_len_dst!=0)
		{
			crc_dst=tab_crc32((const unsigned char *)read_buf,rd_len_dst);
			//printf("目标文件CRC校验值[%ld]\n",crc_dst);
		}
		
		if(crc_src!=crc_dst)
		{
			printf("源文件[%s][%ld]与目标文件[%s]CRC校验[%ld]错误\n",src,crc_src,dst,crc_dst);
			gtlogerr("源文件[%s][%ld]与目标文件[%s]CRC校验[%ld]错误\n",src,crc_src,dst,crc_dst);
			fclose(src_stream);
			fclose(dst_stream);
			return -1;	
		}
	
		if(rd_len_cnt==stbuf.st_size)
		{
			//printf("CRC232校验文件复制完.\n");
			break;
		}
		
		if(sf_dir.opt_d_flag==0)
		{
			if(rd_len_src==0)
			{
				//printf("符号链接当文件复制完\n");
				break;
			}
		}
	}
	
	fclose(src_stream);
	fclose(dst_stream);

	

	return 0;
}

/**********************************************************************************
 *      函数名: tip_cover()
 *      功能:   
 *      输入:
 *      输出:   无
 *      返回值: 
 *********************************************************************************/
int tip_cover(void)
{
	int u_chose;
		
	u_chose=getchar();
	getchar();
	if(u_chose=='y')
	{
		return 1;
	}
	
	return 0;
}


/**********************************************************************************
 *    函数名: tip_cover()
 *    功能: 将数据全部写入缓冲区 
 *	输入:	ptr		指向欲写入的数据地址
 *				size		单位结构的字符数
 *				nmemb	有多少个单位结构，总的写入字数=size*nmemb
 *				stream	已打开的文件指针
 *     
 *      输出:   无
 *      返回值: 返回实际写入的字符数
 *********************************************************************************/
int sf_write_buf(const char *ptr,int size,int nmemb,FILE *stream)
{
	unsigned long left_len=0;
	unsigned long wr_cnt=0;
	int ret;
		
	if(ptr==NULL || stream==NULL)
	{
		printf("sf_write_buf数据指针ptr为空line:%d.\n",__LINE__);
		gtlogerr("sf_write_buf数据指针ptr为空line:%d.\n",__LINE__);
		return -1;
	}

	left_len=size*nmemb;
	//printf("%d\n",__LINE__);
	while(left_len>0)
	{
		ret=fwrite(ptr,1,left_len,stream);
		if(ret<=0)
		{
			errr=errno;
			if(errr==EINTR)
			{
				ret=0;
			}
			else
			{
				if(errr>0)       // 其他错误 没有办法
				{
					return -errr;
				}
				else
				{
					return -1;
				}
			}	
		}
		left_len-=ret;      
		wr_cnt +=ret;
 		ptr+=ret;			// 从剩下的地方继续写 
        }
	fflush(stream);
	
        return wr_cnt;;
}
		

/**********************************************************************************
 *      函数名: copy_files()
 *      功能:   拷贝源文件内容到目标文件
 *      输入:	src		源文件
 *				dst		目标目录
 *      输出:   无
 *      返回值: 成功返回0，负值错误码
 *********************************************************************************/
int trans_data_src_to_dst(const char *src,const char *dst)
{
	FILE *src_stream=NULL;
	FILE *dst_stream=NULL;
	char read_buf[RD_BUF_MAX_LEN];//RD_BUF_MAX_LEN
	int rd_len;
	int rd_len_cnt;
	int wr_len;
	int ret;
	struct stat stbuf;


	if(src==NULL || dst==NULL)
	{
		return -1;
	}

	
	//打开源文件
	src_stream=fopen(src,"r");
	if(src_stream==NULL)
	{
			errr=errno;
		printf("打开源文件[%s]失败[%s],line:%d\n",src,strerror(errr),__LINE__);
		gtlogerr("打开源文件[%s]失败[%s],line:%d\n",src,strerror(errr),__LINE__);
		return -1;
	}

	if(access(dst,F_OK)==0)
	{
		ret=remove(dst);
		if(ret<0)
		{
			printf("重新复制文件失败line:%d\n",__LINE__);
			gtlogerr("重新复制文件失败line:%d\n",__LINE__);
			return -1;
		}
			
	}
	
	//打开或建立目标文件
	dst_stream=fopen(dst,"wb");
	if(dst_stream==NULL)
	{
			errr=errno;
		printf("建立目的文件[%s]失败[%s]line:%d\n",dst,strerror(errr),__LINE__);
		gtlogerr("建立目的文件[%s]失败[%s]line:%d\n",dst,strerror(errr),__LINE__);
		fclose(src_stream);
		return -1;
	}
	else
	{
		//printf("建立文件[%s]成功.\n",dst);
	}

	memset(&stbuf,0,sizeof(struct stat));
	//当符号链接拷贝
	errno=0;
	ret=lstat(src,&stbuf);
	if(ret<0)
	{
			errr=errno;
		printf("获取文件[%s]状态失败[%s]line:[%d]\n",src,strerror(errr),__LINE__);
		gtlogerr("获取文件[%s]状态失败[%s]line:[%d]\n",src,strerror(errr),__LINE__);
		fclose(src_stream);
		fclose(dst_stream);
		return -1;
	}

	//printf("lstat文件长度为[%ld]\n",stbuf.st_size);

	if(sf_dir.opt_u_flag==1)
	{
		printf("[%s]----->[%s]\n",src,dst);
	}
	
	//开始拷贝文件	
	rd_len_cnt=0;
	while(1)
	{
		//读文件
		rd_len=0;
		memset(read_buf,0,sizeof(read_buf));
		rd_len=fread((void *)read_buf,1,sizeof(read_buf),src_stream);
		//printf("复制读出的文件[%s]长度rd_len=%d\n",src,rd_len);
		if(rd_len>0)
		{
			rd_len_cnt+=rd_len;
		}
		else //if(rd_len<0) 
			{
				if(rd_len==0)
				{
					rd_len_cnt+=rd_len;
				}
				else
				{
						errr=errno;
					printf("复制读取源文件[%s]错误[%s]line:%d\n",src,strerror(errr),__LINE__);
					gtlogerr("复制读取源文件[%s]错误[%s]line:%d\n",src,strerror(errr),__LINE__);
					fclose(src_stream);
					fclose(dst_stream);
					return -1;
				}
			}			
		//fflush(src_stream);

		//写文件
#if 0
		errno=0;
		wr_len=fwrite(read_buf,1,rd_len,dst_stream);
		//printf("写文件[%s]的长度wr_len=%d\n",dst,wr_len);
		if(wr_len!=rd_len)
		{
				errr=errno;
				printf("写文件[%s]错误[%s]line:[%d]\n",dst,strerror(errr),__LINE__);
				gtlogerr("写文件[%s]错误[%s]line:[%d]\n",dst,strerror(errr),__LINE__);
				fclose(src_stream);
				fclose(dst_stream);
				return -1;	
		}	
#endif

		wr_len=sf_write_buf(read_buf,1,rd_len,dst_stream);
		if(wr_len!=rd_len)
		{
				errr=errno;
				printf("写文件[%s]错误[%s]line:[%d]\n",dst,strerror(errr),__LINE__);
				gtlogerr("写文件[%s]错误[%s]line:[%d]\n",dst,strerror(errr),__LINE__);
				fclose(src_stream);
				fclose(dst_stream);
				return -1;	
		}	
		if(rd_len_cnt==stbuf.st_size)
		{
			//printf("文件复制完.\n");
			break;
		}
		if(sf_dir.opt_d_flag==0)
		{
			if(rd_len==0)
			{
				//printf("符号链接当文件复制完\n");
				break;
			}
		}
	}
	fflush(dst_stream);
	
	fclose(src_stream);
	fclose(dst_stream);
	
	return 0;
}


/**********************************************************************************
 *      函数名: set_file_time()
 *      功能:   修改文件时间属性
 *      输入:   src		源文件
 *				dst		目标目录
 *      输出:   无
 *      返回值: 成功返回0，负值错误码
 *********************************************************************************/
int change_file_attri(const char *src,const char *dst)
{
	int ret;
	struct utimbuf tmp_time;
	mode_t tmp_mode;
	mode_t d_mode;
	struct stat stbuf;

	memset(&tmp_time,0,sizeof(struct utimbuf));
	memset(&tmp_mode,0,sizeof(mode_t));
	memset(&d_mode,0,sizeof(mode_t));
	memset(&stbuf,0,sizeof(struct stat));
	ret=lstat(src,&stbuf);
	if(ret<0)
	{
			errr=errno;
		printf("获取文件[%s]状态失败[%s]line:%d\n",src,strerror(errr),__LINE__);
		gtlogerr("获取文件[%s]状态失败[%s]line:%d\n",src,strerror(errr),__LINE__);
		return -1;
	}
	tmp_time.actime = stbuf.st_atime;
	tmp_time.modtime = stbuf.st_mtime;
	tmp_mode=stbuf.st_mode;
	//printf("src-actime=[%ld]\n",tmp_time.actime);
	//printf("src-modtiime=[%ld]\n",tmp_time.modtime);
	//printf("lstat---src-mode=[0x%x]\n",tmp_mode);

	memset(&stbuf,0,sizeof(struct stat));
	ret=stat(src,&stbuf);
	if(ret<0)
	{
			errr=errno;
		printf("获取文件[%s]状态失败[%s]line:%d\n",src,strerror(errr),__LINE__);
		gtlogerr("获取文件[%s]状态失败[%s]line:%d\n",src,strerror(errr),__LINE__);
		return -1;
	}
	//printf("stat---src-mode=[0x%x]\n",stbuf.st_mode);
	d_mode=stbuf.st_mode;

	
	errno=0;
	ret=utime(dst,&tmp_time);
	if(ret<0)
	{
			errr=errno;
		printf("设置目标文件[%s]时间属性错误[%s]line:%d\n",dst,strerror(errr),__LINE__);
		gtlogerr("设置目标文件[%s]时间属性错误[%s]line:%d\n",dst,strerror(errr),__LINE__);
		return -1;		
	}

	if(sf_dir.opt_d_flag==1)
	{
		errno=0;
		ret=chmod(dst,tmp_mode);
		if(ret<0)
		{
				errr=errno;
			printf("设置目标文件[%s]属性失败[%s]line:%d\n",dst,strerror(errr),__LINE__);
			gtlogerr("设置目标文件[%s]属性失败[%s]line:%d\n",dst,strerror(errr),__LINE__);
			return -1;
		}
	}
	else
	{
		ret=chmod(dst,d_mode);
		if(ret<0)
		{
				errr=errno;
			printf("设置目标文件[%s]属性失败[%s]line:%d\n",dst,strerror(errr),__LINE__);
			gtlogerr("设置目标文件[%s]属性失败[%s]line:%d\n",dst,strerror(errr),__LINE__);
			return -1;
		}
	}




	memset(&tmp_time,0,sizeof(struct utimbuf));
	memset(&tmp_mode,0,sizeof(mode_t));
	memset(&stbuf,0,sizeof(struct stat));
	errno=0;
	ret=lstat(dst,&stbuf);
	if(ret<0)
	{
			errr=errno;
		printf("获取文件[%s]状态失败[%s],line:%d\n",dst,strerror(errr),__LINE__);
		gtlogerr("获取文件[%s]状态失败[%s],line:%d\n",dst,strerror(errr),__LINE__);
		return -1;
	}
	tmp_time.actime = stbuf.st_atime;
	tmp_time.modtime = stbuf.st_mtime;
	tmp_mode=stbuf.st_mode;
	//printf("dst-actime=[%ld]\n",tmp_time.actime);
	//printf("dst-modtiime=[%ld]\n",tmp_time.modtime);
	//printf("dst-mode=[0x%x]\n",tmp_mode);





	return 0;
}

/**********************************************************************************
 *      函数名: copy_files()
 *      功能:   
 *      输入:   src		源文件
 *				dst		目标目录
 *      输出:   无
 *      返回值: 成功返回0，负值错误码
 *********************************************************************************/
int copy_files(const char *src,const char *dst)
{
	char dst_tmp[DST_FILENAME_MAX_LEN];
	char dst_filename[128];
	int crc_err_cnt;
	int cp_err_cnt;
	int ret;
	struct stat stbuf;


	if(src==NULL || dst==NULL)
	{
		return -1;
	}


	//检查源文件存在不
	if(access(src,F_OK)!=0)
	{
		printf("源文件[%s]不存在\n",src);
		return -1;
	}



        memset(dst_tmp,0,sizeof(dst_tmp));
        memset(&stbuf,0,sizeof(struct stat));
        memset(dst_filename,0,sizeof(dst_filename));

	
	ret=lstat(dst,&stbuf);
	//if(ret<0)
	//{
	//	printf("获取目的文件[%s]状态失败[%s]\n",dst,strerror(errno));
	//	gtlogerr("获取目的文件[%s]状态失败[%s]\n",dst,strerror(errno));
	//	//return -1;
	//}
		
	if(S_ISDIR(stbuf.st_mode))
	{
		//从源文件路径中分析出源文件名
		get_filename_from_dir((char *)src,dst_filename);
		//printf("从路径分解出来的文件名为[%s]\n",&dst_filename[1]);
		sprintf(dst_tmp,"%s%s",dst,&dst_filename[1]);
		//printf("拼凑出来的目标文件路径为[%s]\n",dst_tmp);
	}
	else
	{
		sprintf(dst_tmp,"%s",dst);
		//printf("[%s] is a file\n",dst_tmp);
	}
	
       // printf("dst=[%s]\n",dst);


	//检查目标目录下有没有同名文件，有，删除，没有，创建
	errno=0;
	if(access(dst_tmp,F_OK)==0)
	{
		if(sf_dir.opt_f_flag==1)
		{	
			ret=remove(dst_tmp);
			if(ret==0)
			{
				//printf("删除同名文件[%s]成功.\n",dst_tmp);
			}
			else
			{
					errr=errno;
				printf("删除同名文件[%s]失败[%s]line:[%d].\n",dst_tmp,strerror(errr),__LINE__);
				gtlogerr("删除同名文件[%s]失败[%s]line:[%d].\n",dst_tmp,strerror(errr),__LINE__);
				return -1;
			}
		}
		else
		{
			printf("删除已存在文件 [%s] ? ",dst_tmp);
			if(tip_cover())
			{
				errno=0;
				ret=remove(dst_tmp);
				if(ret==0)
				{
					//printf("删除同名文件[%s]成功.\n",dst_tmp);
				}
				else
				{
						errr=errno;
					printf("删除同名文件[%s]失败[%s]line:[%d].\n",dst_tmp,strerror(errr),__LINE__);
					gtlogerr("删除同名文件[%s]失败[%s]line:[%d].\n",dst_tmp,strerror(errr),__LINE__);
					return -1;
				}	
			}
			else
			{
				//printf("不删除文件.\n");
				return 1;	
			}
		
		}
	}
	//else
	//{
	//	printf("[%s]line:%d\n",strerror(errno),__LINE__);
	//}

	//printf("dst_tmp=%s,line:%d\n",dst_tmp,__LINE__);

	crc_err_cnt=1;
	cp_err_cnt=1;
	while(1)
	{
		
		if(crc_err_cnt>3)
		{
			printf("重新校验次数超过3次，退出.\n");
			gtlogerr("重新校验次数超过3次，退出.\n");
			exit(1);
		}

		if(cp_err_cnt>3)
		{
			printf("重新拷贝次数超过3次，退出.\n");
			gtlogerr("重新拷贝次数超过3次，退出.\n");
			exit(1);
		}

		
		ret=trans_data_src_to_dst(src,dst_tmp);
		//printf("%s--->%s,ret=%d\n",src,dst_tmp,ret);
		if(ret<0)
		{
			printf("传输源文件[%s]到[%s]错误,第%d次\n",src,dst_tmp,cp_err_cnt);
			gtlogerr("传输源文件[%s]到[%s]错误,第%d次\n",src,dst_tmp,cp_err_cnt);	
			cp_err_cnt++;
			continue;
		}
		
		ret=check_files_size(src,dst_tmp);
		//printf("check_files_size...ret=%d,,line:%d\n",ret,__LINE__);
		if(ret<0)
		{
			printf("校验文件[%s][%s]大小错误，第%d次\n",src,dst_tmp,crc_err_cnt);
			gtlogerr("校验文件[%s][%s]大小错误，第%d次\n",src,dst_tmp,crc_err_cnt);
			crc_err_cnt++;
			continue;
		}

		ret=	check_file_crc32((char *)src,dst_tmp);	
		//printf("check_file_crc...ret=%d,line:%d\n",ret,__LINE__);
		if(ret<0)
		{	
			printf("文件[%s][%s]CRC校验错误,第%d次\n",src,dst_tmp,crc_err_cnt);
			gtlogerr("文件[%s][%s]CRC校验错误，第%d次\n",src,dst_tmp,crc_err_cnt);
			crc_err_cnt++;
			continue;
		}
		else 
		{
			if(crc_err_cnt!=1||cp_err_cnt!=1)
			{
				//printf("校验全部正确\n");
				printf("文件重新复制成功(拷贝%d次，校验%d次).line:%d\n",cp_err_cnt,crc_err_cnt,__LINE__);
				gtloginfo("文件重新复制成功(拷贝%d次，校验%d次).line:%d\n",cp_err_cnt,crc_err_cnt,__LINE__);
			}
			break;
		}
	}

	ret=change_file_attri(src,dst_tmp);
	if(ret<0)
	{
		printf("修改文件[%s]属性返回值错误\n",dst_tmp);
		gtlogerr("修改文件[%s]属性返回值错误\n",dst_tmp);
		return ret;
	}

	
	return 0;
}



/**********************************************************************************
 *      函数名: copy_dirs()
 *      功能:   回调函数，由ftw回调,复制目录
 *      输入:   src		源文件
 *				dst		目标目录
 *      输出:   无
 *      返回值: 成功返回0，负值错误码
 *********************************************************************************/
int copy_dirs(const char *file_dir,const char *dst_dir)
{
	//int len;
	int ret;
	//char remv_tmp[DST_FILENAME_MAX_LEN];
	
	if(file_dir==NULL || dst_dir==NULL)
	{
		return -1;
	}

	//检查目标目录是否存在，存在先删除里面的文件
	if(access(dst_dir,F_OK)!=0)
	{
		errno=0;
		ret=mkdir(dst_dir,0777);
		if(ret==0)
		{
			//printf("创建目录[%s]成功\n",dst_dir);
		}
		else
		{
				errr=errno;
			printf("创建目录[%s]失败[%s]line:%d\n",dst_dir,strerror(errr),__LINE__);
			gtlogerr("创建目录[%s]失败[%s]line:%d\n",dst_dir,strerror(errr),__LINE__);
			return -1;
		}
	}
	

	return 0;
}


/**********************************************************************************
 *      函数名: creat_lnk()
 *      功能:   回调函数，由ftw回调,复制目录
 *      输入:   src		源文件
 *				dst		目标目录
 *      输出:   无
 *      返回值: 成功返回0，负值错误码
 *********************************************************************************/
 int creat_lnk(char *lnk_buf,char *tmp_buf)
{
	int ret;

	errno=0;
	ret=symlink(lnk_buf,tmp_buf);
	if(ret<0)
	{
			errr=errno;
		printf("创建[%s] 错误[%s].line:%d\n",tmp_buf,strerror(errr),__LINE__);
		gtlogerr("创建[%s] 错误[%s].line:%d\n",tmp_buf,strerror(errr),__LINE__);
		return -1;
	}


	return 0;
}

/**********************************************************************************
 *      函数名: copy_lnk()
 *      功能:   解析命令行参数
 *      输入:	lnk_buf		符号链接字符串指针
 *				src			源符号链接路径指针
 *				tmp_buf		目标符号链接指针tmp_buf=dst_path+filename
 *				dst_path		目标路径指针
 *				filename		文件名指针
 *				name_len	文件名长度
 *				get_flag		掉用get_filename_from_dir标志
 *      输出:   无
 *	 备注: 调用前需要对lnk_buf,tmp_buf,进行初始化
 *      返回值:  成功返回0，负值错误码
 *********************************************************************************/
int copy_lnk(char *lnk_buf,char *src,char *tmp_buf,char *dst_path,char *filename,int name_len,int get_flag)
{
	int ret;
	int exist_flag;
	struct stat buf;

	//memset(lnk_buf,0,sizeof(lnk_buf));
	//printf("link----[%s]--\n",src);
	errno=0;
	ret=readlink(src,lnk_buf,LNK_PATH_MAX_LEN);
	if(ret<0)
	{
			errr=errno;
		printf("获取符号链接[%s] 错误[%s].line:%d\n",src,strerror(errr),__LINE__);
		gtlogerr("获取符号链接[%s] 错误[%s].line:%d\n",src,strerror(errr),__LINE__);
		return -1;
	}
	//printf("lnk_buf[%s]\n",lnk_buf);
	//memset(tmp_buf,0,sizeof(tmp_buf));
	
	if(get_flag==1)
	{
		memset(filename,0,name_len);
		get_filename_from_dir((char *)src,filename);
		sprintf(tmp_buf,"%s%s",dst_path,&filename[1]);
	}
	else
	{
		sprintf(tmp_buf,"%s",dst_path);
	}

	//判断符号链接使用lstat
	memset(&buf,0,sizeof(struct stat));
	errno=0;
	exist_flag=1;
	ret=lstat(tmp_buf,&buf);
	if(ret<0)
	{	
		errr=errno;
		//有可能在目的路径下咩有那个文件，如果是其他的错误就报告日志
		if(errr==2)
		{
			exist_flag=0;
			ret=creat_lnk(lnk_buf,tmp_buf);
			if(ret<0)
				return ret;
			
		}
		else
		{
			printf("获取[%s]状态失败errno=%d,[%s]line:%d\n",tmp_buf,errr,strerror(errr),__LINE__);
			gtlogerr("获取[%s]状态失败errno=%d[%s].line:%d\n",tmp_buf,errr,strerror(errr),__LINE__);
			return -1;
		}
	}


	if(exist_flag==1)
	{
		//如果文件存在，先去试图删它
		if(S_ISLNK(buf.st_mode))
		{	
			if(sf_dir.opt_f_flag==1)
			{
				errno=0;
		 	       ret=unlink(tmp_buf);
      				if(ret<0)
        			{
        				errr=errno;
       				printf("删除 [%s] 错误[%s]line:[%d]\n",tmp_buf,strerror(errr),__LINE__);
					gtlogerr("删除 [%s] 错误[%s]line:[%d]\n",tmp_buf,strerror(errr),__LINE__);
      		          		return -1;
      		  		}
      		  		else
        			{
         				//printf("del [%s] ok\n",tmp_buf);
       	 		}
	
			}
			else
			{
				printf("删除已存在的符号链接[%s] ?",tmp_buf);
				if(tip_cover())
				{
					errno=0;
					ret=unlink(tmp_buf);
      					if(ret<0)
       				{
       					errr=errno;
       					printf("删除 [%s] 错误[%s]line:[%d]\n",tmp_buf,strerror(errr),__LINE__);
						gtlogerr("删除 [%s] 错误[%s]line:[%d]\n",tmp_buf,strerror(errr),__LINE__);
       					return -1;
       				}
       				else
       				{
       					//printf("del [%s] ok\n",tmp_buf);
       				}
				}	
				else
				{
					return 1;
				}

			}
										
		}
		else
		{
			printf("不是符号链接为什么进到这里来line:%d\n",__LINE__);
			gtlogerr("不是符号链接为什么进到这里来line:%d\n",__LINE__);
			return -1;
		}
	
		ret=creat_lnk(lnk_buf,tmp_buf);
		if(ret<0)
			return ret;
	}

	
	if(sf_dir.opt_u_flag==1)
	{
		printf("[%s]----->[%s]\n",src,tmp_buf);
	}

	return 0;
}


/**********************************************************************************
 *      函数名: show_directions()
 *      功能:   回调函数，由ftw回调
 *      输入:   无
 *      输出:   无
 *      返回值: 成功返回0，负值错误码
 *********************************************************************************/
int show_directions(const char *file,const struct stat *sb,int flag)
{
	struct stat buf;
	int len;
	int len_sub;
	int ret;
	char *tmp_sub=NULL;
	char tmp_sub_nh[SRC_FILENAME_MAX_LEN];
	char tmp_buf[SRC_FILENAME_MAX_LEN];
	char lnk_buf[LNK_PATH_MAX_LEN];


	//检查指针有效
	if(file==NULL || sb==NULL)
	{
		return -1;
	}

	//sf_dir->src_path的路径应该被包含在file_dir路径中，这样使用当前的目录加上sf_dir->src_path的
	//长度的偏移量就是当前相对的当前目录了，就可以把这个目录接在目标目录后了。
	//相对目录去头,目的是拼接目的目录
	len=strlen(sf_dir.src_path);
	memset(tmp_sub_nh,0,sizeof(DST_FILENAME_MAX_LEN));
	if((*(file+len))=='/')
	{
		sprintf(tmp_sub_nh,"%s",(file+len+1));
	}
	else
	{
		sprintf(tmp_sub_nh,"%s",(file+len));
	}
		
	//将相对目录去尾,目的是比较当前的目录有没有改变
	memset(&buf,0,sizeof(struct stat));
	errno=0;
	ret=lstat(file,&buf);
	if(ret<0)
	{
			errr=errno;
		printf("获取源文件[%s]状态失败[%s]\n",file,strerror(errr));
		gtlogerr("获取源文件[%s]状态失败[%s]\n",file,strerror(errr));
		return -1;
	}
	
	if(S_ISREG(buf.st_mode))
	{
		memset(sf_dir.sub_path_nf,0,sizeof(sf_dir.sub_path_nf));
		sprintf(sf_dir.sub_path_nf,"%s",file);
		tmp_sub=strrchr(file,'/');		
		if(tmp_sub!=NULL)
		{
			//printf("取得的尾为[%s]\n",tmp_sub);
			len_sub=strlen(tmp_sub);
		}
		else
		{
			//printf("tmp_sub为空\n");
			len_sub=strlen(sf_dir.sub_path_nf);
		}
		len=strlen(sf_dir.sub_path_nf);
		sf_dir.sub_path_nf[len-len_sub]='\0';
		//printf("从文件目录中去尾后的新目录为[%s]\n",sf_dir.sub_path_nf);
	}

	//printf("file=[%s]------sf_dir.sub_path_nf=[%s]\n",file,sf_dir.sub_path_nf);


	//检查看是不是进了新目录
	ret=strcmp(sf_dir.sub_path_nf,sf_dir.sub_path_nf_old);
	if(ret!=0)
	{
		//printf("换了新目录了\n");
		memset(sf_dir.sub_path_nf_old,0,sizeof(sf_dir.sub_path_nf_old));
		memcpy(sf_dir.sub_path_nf_old,sf_dir.sub_path_nf,sizeof(sf_dir.sub_path_nf));
	}

	memset(sf_dir.cur_dst_path,0,sizeof(sf_dir.cur_dst_path));
	sprintf(sf_dir.cur_dst_path,"%s",sf_dir.dst_path);
	
	if(strlen(tmp_sub_nh)!=0)
	{
		strcat(sf_dir.cur_dst_path,"/");
		strcat(sf_dir.cur_dst_path,tmp_sub_nh);
	}
	//printf("此时的目标路径为[%s]\n",sf_dir.cur_dst_path);

	//开始调用复制模块	
	if(S_ISLNK(buf.st_mode))		
		{
			//printf("[%s]-----符号链接\n",file);
			if(sf_dir.opt_d_flag==1)
			{
				memset(lnk_buf,0,sizeof(lnk_buf));
				memset(tmp_buf,0,sizeof(tmp_buf));
				ret=copy_lnk(lnk_buf,(char *)file,tmp_buf,sf_dir.cur_dst_path,NULL,0,0);
				if(ret<0)
				{
					printf("拷贝符号链接[%s]失败line:%d\n",file,__LINE__);
					gtlogerr("拷贝符号链接[%s]失败line:%d\n",file,__LINE__);
					return ret;
				}
			}
			else
			{
				//lnk-->file
				//copy_files(tmp_buf,sf_dir.dst_path);
				ret=copy_files(file,sf_dir.cur_dst_path);
				if(ret<0)
				{
					printf("符号链接[%s]当文件拷贝失败line:%d \n",file,__LINE__);
					gtlogerr("符号链接[%s]当文件拷贝失败line:%d \n",file,__LINE__);
					return ret;
				}
			}
		}
	else if(S_ISREG(buf.st_mode))
		{
			//printf("[%s]----文件line:%d\n",file,__LINE__);
			ret=copy_files(file,sf_dir.cur_dst_path);
				if(ret<0)
				{
					printf("拷贝文件[%s]失败line:%d\n",file,__LINE__);
					gtlogerr("拷贝文件[%s]失败line:%d\n",file,__LINE__);
					return ret;
				}

		}
	else if(S_ISDIR(buf.st_mode))
		{
			//printf("[%s]---目录\n",file);
			ret=copy_dirs(file,sf_dir.cur_dst_path);
				if(ret<0)
				{
					printf("拷贝目录[%s]失败line:%d\n",file,__LINE__);
					gtlogerr("拷贝目录[%s]失败line:%d\n",file,__LINE__);
					return ret;
				}
		}

	//printf("\n\n");

	return 0;
}



/**********************************************************************************
 *      函数名: err_fn()
 *      功能:   错误回调函数
 *      输入:   *df_dir			指向SFCP_PATH_T 的指针
 *      输出:   无
 *      返回值:  成功返回0，负值错误码
 *	 注:
 *	当执行scandir访问目录错误时调用的函数，有错误发生
	后返回 err_fn返回的值,终止遍历,dir_file表示出错的目录或
	文件,errnum表示错误号，其值为出错时的errno
 *********************************************************************************/
int err_fn(const char *dir_file,int errnum)
{
	printf("访问目录[%s]错误,errno:[0x%x]\n",dir_file,errnum);
	return -1;
}


/**********************************************************************************
 *      函数名: fix_src_dst_dir()
 *      功能:   根据源路径修改目的路径
 *      输入:	src		源路径
 *				dst		目的路径
 *				filename	暂存源路径中的要要拷贝的路径名
 *      输出:   无
 *      返回值:  成功返回0，负值错误码
 *********************************************************************************/
int fix_src_dst_dir(char *src,char *dst,char *filename)
{
	int ret;
	int len;

	//printf("源路径是目录，准备提取源路径中的目的路径\n");
	//printf("提取前src=[%s]\n",src);
	//printf("dst=[%s]\n",dst);
	get_filename_from_dir(src,filename);
	//printf("提取到的路径为=[%s]\n",filename);

	if(strlen(dst)+strlen(filename)>DST_FILENAME_MAX_LEN)
	{
		printf("目的路径文件名%s过长\n",dst);
		gtlogerr("目的路径文件名%s过长\n",dst);
		return -1;
	}

	len=strlen(dst);
	if(*(dst+len-1)!='/')
	{
		//printf("不含/\n");
		strcat(dst,filename);
	}
	else if(*(dst+len-1)=='/')
		{
			//printf("含有/\n");
			strcat(dst,(char *)&filename[1]);
		}
	
	//printf("修改后额dst=[%s]\n",dst);

	errno=0;
	if(access(dst,F_OK)!=0)
	{
		//为源路径中含有*准备
		if(sf_dir.opt_r_flag==1)
		{
			//printf("目录[%s]不存在，准备创建\n",dst);
			errno=0;
			ret=mkdir(dst,0777);
			if(ret==0)
			{
				//printf("创建目录[%s]成功\n",dst);
			}
			else
			{
					errr=errno;
				printf("创建目录[%s]失败:%s\n",dst,strerror(errr));
				gtlogerr("创建目录[%s]失败:%s\n",dst,strerror(errr));
				return -1;
			}
		}
	}
	
	return 0;
}



/**********************************************************************************
 *      函数名: creat_dir()
 *      功能:   创建目录
 *      输入:   dir			路径
 *      输出:   无
 *      返回值:  成功返回0，负值错误码
 *********************************************************************************/
int creat_dir(char *dir)
{
	int ret;
	
	errno=0;
	if(access(dir,F_OK)!=0)
	{
		ret=mkdir(dir,0777);
		if(ret==0)
		{
			//printf("创建目录[%s]成功\n",dst_dir);
		}
		else
		{
			errr=errno;
			printf("创建目录[%s]失败[%s]line:%d\n",dir,strerror(errr),__LINE__);
			gtlogerr("创建目录[%s]失败[%s],line:%d\n",dir,strerror(errr),__LINE__);
			return -1;
		}

		sf_dir.creat_ndir_flag=1;
	}
	return 0;
}


/**********************************************************************************
 *      函数名: check_same_dir()
 *      功能:   检查源路径和目的路径是否是相同的
 *      输入:   dir			路径
 *      输出:   无
 *      返回值:  成功返回0，负值错误码
 *********************************************************************************/
int check_same_dir(char *src,char *dst)
{
	int len;
	char tmp_buf[DST_FILENAME_MAX_LEN];
	
	if(src==NULL || dst==NULL)
	{
		return 0;
	}

	//printf("检查src=[%s]，目的[%s],line:%d\n",src,dst,__LINE__);

		if(strcmp(src,dst)==0)
		{
			printf("源路径[%s]与目的路径[%s]相同,避免递归拷贝line:%d\n",src,dst,__LINE__);
			gtlogerr("源路径[%s]与目的路径[%s]相同,避免递归拷贝line:%d\n",src,dst,__LINE__);
			exit(1);
		}

	memset(tmp_buf,0,sizeof(tmp_buf));
	sprintf(tmp_buf,"%s",dst);
	len=strlen(dst);
	if(tmp_buf[len-1]=='/')
	{
		//printf("目的路径[%s]含有/\n",tmp_buf);
		tmp_buf[len-1]='\0';
		//printf("去掉后dst=[%s]\n",tmp_buf);
		if(strcmp(src,tmp_buf)==0)
		{
			printf("源路径[%s]与目的路径[%s]相同,避免递归拷贝line:%d\n",src,tmp_buf,__LINE__);
			gtlogerr("源路径[%s]与目的路径[%s]相同,避免递归拷贝line:%d\n",src,tmp_buf,__LINE__);
			exit(1);
		}
	}

	return 0;
}

/**********************************************************************************
 *      函数名: parser_args()
 *      功能:   解析命令行参数
 *      输入:   *df_dir			指向SFCP_PATH_T 的指针
 *      输出:   无
 *      返回值:  成功返回0，负值错误码
 *********************************************************************************/
 int parser_args(SFCP_PATH_T *df_dir,char *argv[])
{
	struct stat buf;
	struct stat sub_buf;
	int len;
	int i;
	int dst_flag=0;
	int multi_dirs_flag=0;
	int ret;
	char tmp_buf[SRC_FILENAME_MAX_LEN];
	char lnk_buf[LNK_PATH_MAX_LEN];
	char dst_tmp[DST_FILENAME_MAX_LEN];

	//检查指针有效
	if(df_dir==NULL)
	{
		return -1;
	}
					
	memset(&buf,0,sizeof(struct stat));
	memset(&sub_buf,0,sizeof(struct stat));

        //printf("argc=%d\n",df_dir->arg_total);
        for(i=0;i<df_dir->arg_total-1;i++)
        {
               if(argv[df_dir->arg_total-1-i][0]!='-')
                {
                        if(dst_flag==0)
                        {
				sprintf(df_dir->dst_path,"%s",argv[df_dir->arg_total-1-i]);
                               
				//check dst_path exist
				errno=0;
				if(access(df_dir->dst_path,F_OK)==0)
				{
					errno=0;
					ret=lstat(df_dir->dst_path,&buf);
					if(ret<0)
					{
							errr=errno;
						printf("获取源文件[%s]状态失败[%s]\n",df_dir->src_path,strerror(errr));
						gtlogerr("获取源文件[%s]状态失败[%s]\n",df_dir->src_path,strerror(errr));
						return -1;
					}
					if(S_ISDIR(buf.st_mode))
					{
						//dst_path include '/'
						len=strlen(df_dir->dst_path);
						//printf("len=[%d]---df_dir->dst_path[len-1]=%c\n",len,df_dir->dst_path[len-1]);
						if(df_dir->dst_path[len-1]=='/')
						{
							//printf("df_dir->dst-path include '/' sight\n");
						}
						else
						{
							//printf("df_dir->dst-paht dosen't include,but i fix it\n");
							df_dir->dst_path[len]='/';
							//printf("df_dir->dst-path=[%s]\n",df_dir->dst_path);
						}
                                		//printf("Dst-dir=[%s]\n",df_dir->dst_path);			
						memset(df_dir->dst_path_bak,0,sizeof(df_dir->dst_path_bak));
						memcpy(df_dir->dst_path_bak,df_dir->dst_path,sizeof(df_dir->dst_path));
					}
					
				}
				else
				{
					//printf("目标路径%s,[%s],line:%d\n",df_dir->dst_path,strerror(errno),__LINE__);
					//gtlogerr("目标路径%s,[%s],line:%d\n",df_dir->dst_path,strerror(errno),__LINE__);
	////				creat_dir(df_dir->dst_path);					
					memset(df_dir->dst_path_bak,0,sizeof(df_dir->dst_path_bak));
					memcpy(df_dir->dst_path_bak,df_dir->dst_path,sizeof(df_dir->dst_path));
				}
				
				dst_flag=1;
                            continue;
                        }
                        else
                        {
                                sprintf(df_dir->src_path,"%s",argv[df_dir->arg_total-1-i]);
                        }
				//printf("Dst-dir=[%s]\n",df_dir->dst_path);
                       	//printf("[%d]--->Src-dir=[%s]\n",i,df_dir->src_path);			
				//printf("df_dir->dst_path=[%s],line:%d\n",df_dir->dst_path,__LINE__);
				//printf("df_dir->dst_path_bak=[%s],line:%d\n",df_dir->dst_path_bak,__LINE__);
			check_same_dir(df_dir->src_path,df_dir->dst_path);
				
			errno=0;
			ret=lstat(df_dir->src_path,&buf);
			if(ret<0)
			{
					errr=errno;
				printf("获取源文件[%s]状态失败[%s]\n",df_dir->src_path,strerror(errr));
				gtlogerr("获取源文件[%s]状态失败[%s]\n",df_dir->src_path,strerror(errr));
				return -1;
			}
                        if(S_ISDIR(buf.st_mode))
                        {
              ////
              			memset(df_dir->dst_path,0,sizeof(df_dir->dst_path));
					memcpy(df_dir->dst_path,df_dir->dst_path_bak,sizeof(df_dir->dst_path_bak));
        				creat_dir(df_dir->dst_path);		
						
					memset(df_dir->dst_path_bak,0,sizeof(df_dir->dst_path_bak));
					memcpy(df_dir->dst_path_bak,df_dir->dst_path,sizeof(df_dir->dst_path));
		////
 				memset(dst_tmp,0,sizeof(dst_tmp));
				////memset(df_dir->dst_path,0,sizeof(df_dir->dst_path));
				////memcpy(df_dir->dst_path,df_dir->dst_path_bak,sizeof(df_dir->dst_path_bak));
				if(df_dir->creat_ndir_flag==0)
				{
					fix_src_dst_dir(df_dir->src_path,df_dir->dst_path,dst_tmp);
				}
				
				//src dir =1
				if(multi_dirs_flag==0)
				{
			
                                	//printf("准备拷贝目录\n");
					//printf("df_dir->dst_path=[%s],line:%d\n",df_dir->dst_path,__LINE__);
					//printf("df_dir->dst_path_bak=[%s],line:%d\n",df_dir->dst_path_bak,__LINE__);
					if(df_dir->opt_r_flag==1)
					{		

                                		ret=ftw_sort(df_dir->src_path,show_directions,DIR_MAX_DEEP,FTW_SORT_ALPHA,0,err_fn);
						if(ret<0)
						{
							printf("ftw_sort遍历目录返回值错误line:%d\n",__LINE__);
							gtlogerr("ftw_sort遍历目录返回值错误line:%d\n",__LINE__);
							return ret;
						}
					}
					else
					{
						printf("滤过目录[%s]\n",df_dir->src_path);
						//printf("ftw_only-1 deep fs\n");
					      //ret=ftw_sort(df_dir->src_path,show_directions,0,FTW_SORT_ALPHA,0,err_fn);
						//if(ret<0)
						//{
						//	printf("只遍历1层目录时错误\n");
						//	return ret;
						//}
					}

					multi_dirs_flag=0;
				}
                        }
                        else if(S_ISREG(buf.st_mode))
                                {
                                       	//printf("准备拷贝文件\n");
						//printf("df_dir->dst_path=[%s],line:%d\n",df_dir->dst_path,__LINE__);
						//printf("df_dir->dst_path_bak=[%s],line:%d\n",df_dir->dst_path_bak,__LINE__);
						df_dir->src_file_flag=1;
						if(strlen(df_dir->dst_path_bak)!=0)
						{
							//printf("将path-bak--->path\n");
                                      		 memset(df_dir->dst_path,0,sizeof(df_dir->dst_path));
					   		 memcpy(df_dir->dst_path,df_dir->dst_path_bak,sizeof(df_dir->dst_path_bak));
						}

						ret=copy_files(df_dir->src_path,df_dir->dst_path);
						if(ret<0)
						{
							printf("copy_file拷贝文件返回值失败line:%d\n",__LINE__);
							gtlogerr("copy_file拷贝文件返回值失败line:%d\n",__LINE__);
							return ret;
						}
                                }
			else if(S_ISLNK(buf.st_mode))
				{
					memset(df_dir->dst_path,0,sizeof(df_dir->dst_path));
					memcpy(df_dir->dst_path,df_dir->dst_path_bak,sizeof(df_dir->dst_path_bak));

					if(df_dir->opt_d_flag==1)
					{
						//printf("准备拷贝符号链接------\n");
						memset(lnk_buf,0,sizeof(lnk_buf));
						memset(tmp_buf,0,sizeof(tmp_buf));
						memset(dst_tmp,0,sizeof(dst_tmp));

						ret=copy_lnk(lnk_buf,df_dir->src_path,tmp_buf,df_dir->dst_path,dst_tmp,sizeof(dst_tmp),1);
						if(ret<0)
						{
							printf("copy_lnk拷贝符号链接返回值失败line:%d\n",__LINE__);
							gtlogerr("copy_lnk拷贝符号链接返回值失败line:%d\n",__LINE__);
							return ret;
						}						
					}
					else
					{
						//lnk-->file
						//printf("准备把符号链接当文件拷贝\n");
						//printf("src=[%s],dst=[%s]\n",df_dir->src_path,df_dir->dst_path);

						ret=copy_files(df_dir->src_path,df_dir->dst_path);
						if(ret<0)
						{
							printf("把符号链接当文件拷贝返回值失败line:%d\n",__LINE__);
							gtlogerr("把符号链接当文件拷贝返回值失败line:%d\n",__LINE__);
							return ret;
						}
						
					}
				}
                }

        }
	
	return 0;
}



/**********************************************************************************
 *      函数名: main()
 *      功能:   main
 *      输入:   命令行参数
 *      输出:   无
 *      返回值: 成功返回0，负值错误码
 *********************************************************************************/
int main(int argc,char *argv[])
{
	int opt;
	const char *shortopts="rfdu";
	const struct option longopts[]={
			{"help",0,NULL,'h'},
			{"version",0,NULL,'v'},
			{NULL,0,NULL,0},
			};
	int ret;

	//SFCP_PATH_T sf_dir;
	gtopenlog("sfcp");
	
	memset(&sf_dir,0,sizeof(SFCP_PATH_T));

	sf_dir.arg_total=argc;
	while((opt=getopt_long(argc,argv,shortopts,longopts,NULL))!=-1)
	{
		switch(opt)
		{
			case 'r':
				sf_dir.opt_r_flag=1;
				break;

			case 'f':
				sf_dir.opt_f_flag=1;
				break;

			case 'd':
				sf_dir.opt_d_flag=1;
				break;

			case 'u':
				sf_dir.opt_u_flag=1;
				break;
		
			case 'h':
                		print_help();
                		break;
            	
			case 'v':
                		print_ver();
                		break;

            		case '?':   // 未知命令参数
                		printf("未知选项\n");
				gtlogerr("未知选项:[%c]\n",opt);
				exit(1);
                		break;

            		default:
	               	printf("错误选项\n");
				gtlogerr("错误选项:[%c]\n",opt);
				exit(1);
                		break;	
		}
	}

	ret=parser_args(&sf_dir,argv);
	if(ret<0)
	{
		exit(1);
	}
	else
	{
		exit(0);
	}

	return 0;
}


