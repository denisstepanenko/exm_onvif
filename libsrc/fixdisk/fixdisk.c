#include <time.h>
#include <sys/time.h>
#include <sys/vfs.h>
#include "fixdisk.h"
#include <sys/stat.h>
#include "file_def.h"
#include "devinfo.h"
#include <errno.h>
#include <filelib.h>

/*
 * 函数名	:is_disk_error()
 * 功能	:判断传进来的errno是否是应该修理磁盘的问题
 * 输入	:error:错误码
 * 返回值	:1表示应修理磁盘，0表示不用修理磁盘
*/
int is_disk_error(int error)
{
	if((error==EIO)||(error==EACCES)||(error=EBUSY)||(error=EROFS))
		return 1;
	else
		return 0;
}


/*
	名称:	fixdisk_log()
	功能:   把修理磁盘的相关信息如type,reason等写在fixlog里，
			并加上时间和存储介质类型
	输入:	fixlog,修理日志的文件名称
			type为修理选项,y,c,f之一，
			reason为修理原因字符串
	返回值:	无
*/
void fixdisk_log(char *fixlog,char type,char *reason)
{

	struct timeval tv;
	struct tm *ptime;
	time_t ctime;
	struct stat buf;
	FILE *dfp;
	

	dfp=fopen(fixlog,"a+");
	if(dfp==NULL)
		return ;

	//写时间
	if(gettimeofday(&tv,NULL)<0)
	{
		fprintf(dfp,"<获取系统时间时出错 >   ");
	}
	else
	{
		ctime=tv.tv_sec;
		ptime=localtime(&ctime);
		if(ptime!=NULL)
			fprintf(dfp,"<%04d-%02d-%02d %02d:%02d:%02d> ",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min,ptime->tm_sec);	
	}
	//写信息
	fprintf(dfp,"  *%c  ",type);
	switch(get_hd_type())
		{
			case(1): fprintf(dfp," HD ");break;
			case(0): fprintf(dfp," CF ");break;
			default: fprintf(dfp," -- ");break;
		}
	fprintf(dfp," #%ld  ",mktime(ptime));
	fprintf(dfp," %s ",reason);	
	fclose(dfp);

	
}
	
//检查日志，看是否短期内已被修理过，返回值为下次要用的修复选项，y,f,c或q(退出)
//如果上次重起在FIXDISK_INTERVAL秒内,则用更耗时的选项修复，否则用-y修复
//参数fixlog, 形如"/var/tmp/fixdisk-hda2.txt",表示需要修理的分区的修理日志

char check_last_fixdisk(char * fixlog)
{
	FILE * fp;
	char temp[200];
	char log[200];
	struct timeval tv;
	struct tm *ptime;
	time_t ctime;
	char *lp;
	
	fp=fopen(fixlog,"r+"); 
	
	if (fp==NULL)
		return 'y'; //还没有修过
	
	fseek(fp,-200,SEEK_END);//读取最后一行
	while(fgets(temp,200,fp)!=NULL)
	{
		strcpy(log,temp);
	}

	//处理log,<2006-05-29 14:02:06>  y  no CF avail格式

	//先看是否在修理
	lp=strstr(log,"$DONE");
	if(lp==NULL)
		return 'q';
	//gtloginfo("test,有$done标志!!!!!!!!\n");

	//读取时间，作判断
	lp=index(log,'#');
	if(lp==NULL)
		return 'y';
	lp++;

	if(gettimeofday(&tv,NULL)<0)
		return 'y';
	else
	{
		ctime=tv.tv_sec;
		ptime=localtime(&ctime);
		if((ptime!=NULL)&&(mktime(ptime)-atoi(lp)<FIXDISK_INTERVAL))
			{
				//在短期之内刚用某选项修复过
				lp=index(log,'*');
				if(lp!=NULL)
					{
						switch(*(lp+1))
							{
								case 'y': // wsydel if(use_harddisk!=0)//若不是cf卡则不要用-f修
									//	  	return 'q';
									//	  else	
										  	return 'f';
										  break;
								case 'f':  if(get_hd_type()!=0 )//wsyadd, 硬盘顶多用-f
										return 'q';
									 else
										 return 'c';
										  break;
								case 'c': 
								case 'q': return 'q';break;
								default:  return 'y';
							}
					}

			}
			
	}
	return 'y';
}
	
/*
	函数名: get_error_partition()
	功能: 解析出出错的目录或文件所属的分区字符串
	输入:path,发生错误的目录或文件名称
	输出:partition,需要修理的分区，形如"sda3"
		 fixlog,相应的修理日志，形如"/var/tmp/fixdisk-sda2.txt"
	返回值:成功返回0，否则返回负值
*/

int get_error_partition(IN char *path, OUT char *partition, OUT char *fixlog)
{
	char *lp;
	if((path==NULL)||(partition ==NULL))
		return -EINVAL;
	lp = strstr(path,"/sd");
	if(lp==NULL)
	{
		sprintf(partition,"sda1");//兼容
	}
	else
		sprintf(partition,"%s",lp+1);
	lp = index(partition,'/');
	if(lp!=NULL)
		*lp = '\0';
	sprintf(fixlog,"/var/tmp/fixdisk-%s.txt",partition);
	return 0;	
}

/*
 * 函数名	:fix_disk()
 * 功能	:如果errno应该修理磁盘，则检查相关分区的修理记录并用相关选项进行修理
 * 输入	:path:形如"/hqdata/hda2/xxx"的字符串，只要前缀是"/hqdata/hda2"就可以
 *	 	 errno:错误码，正值
 * 返回值:无
*/
void fix_disk( char *path,int diskerrno)
{
	char type;
	char cmd[50];
	int usehd;
	char partition[100];
	char fixlog[100];
	int lock_file;
	int i;

	//set_cferr_flag(1);
#if EMBEDED==1
	
	if(get_ide_flag()==0) //没存储设备
		return;
	if(is_disk_error(diskerrno) == 0) //不是文件系统错误
		return ;
		
	get_error_partition(path,partition,fixlog);
	
	set_cferr_flag(1);
	type=check_last_fixdisk(fixlog);
	if(type=='q')
	{
		gtloginfo("fix_disk，已经修理过%s,失败了或还没修完，退出程序卸载分区\n",partition);
		//wsy改，退出程序把这个分区给卸载掉免得再用
		close_all_res(); 
		sprintf(cmd,"/ip1004/diskumount %s &",partition);
		system(cmd);
		exit(0);
		//return ;
	}
	gtloginfo("启动fixdiskproblem,以%c选项修理磁盘%s\n",type,partition);
	gtloginfo("修理原因:访问%s:%s\n",path,strerror(diskerrno));
	fixdisk_log(fixlog,type,strerror(diskerrno));
	close_all_res(); 
	//执行fixdisk,带参数	
	sprintf(cmd,"/ip1004/fixdiskproblem -%c %s &",type,partition);
	system(cmd);
	exit(0);			
#endif
	return;
}
/*wsyadd, 用于处理调用ftw_sort出错时，需要修理磁盘的情况*/
int  fix_disk_ftw(const char *dirname, int err)
{
        if(dirname==NULL)
                return -EINVAL;

        gtlogerr("ftw 出错, %s, %s\n",dirname, strerror(err));
        fix_disk(dirname,err);
        return FN_DISK_ERR;
}	


