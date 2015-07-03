/*
	基于mpdisk和fileindex库，供hdmodule和diskman调用
								--wsy Dec 2007
*/


#include "fileindex.h"
#include "mpdisk.h"
#include "hdutil.h"
#include <errno.h>
#include "file_def.h"
#include <sys/time.h>


//用于查询最老的可删除分区的时间
struct oldest_filetime_struct
{
	char oldest_partition[100];	//最老的分区名称"/hqdata/sda2"
	unsigned int  oldest_file_time;//最老的文件创建时间，time_t
};


//完成的功能:将filename放到相应的目录
//mountpath: 分区挂载目录，如"/hqdata/sda2"
//filename,形如./#93889/HQ_C00_D20051024125934_L30_T00.AVI
//移动了旧文件返回0，否则返回负值
int relocate_avi_file(char * mountpath ,char *filename)
{
	char *p,*t;
	char temp[100];
	char filepath[200];
	char fix[100];
	int ret=0;
	char command[300];
	if((filename==NULL)||(mountpath == NULL))
		return -1;
	p=rindex(filename,'/');
	if (p==NULL)
		return -1;
	p++;
	t=strstr(p,IMG_FILE_EXT); 
	if(t==NULL)
		return -1;
	t=t+4;
	*t='\0';
	strcpy(temp,p);
	//printf("temp is %s\n",temp);
	if (hdutil_filename2dirs(mountpath,temp,filepath)!=0)
		return -1;
	//printf("path is %s\n",path);
	ret=hdutil_create_dirs(filepath);
	if(ret<0)
	{
		gtlogerr("无法创建目录%s,错误%d: %s\n",filepath,-ret,strerror(-ret));	
		fix_disk(filepath,-ret);
		return -1;
	}
	
	sprintf(command,"mv %s %s",filename,filepath);
	system(command);	
	
	return 0;
	
}

#define	HDERR_RBT_FILE	"/log/hderr_reboot.txt"	//记录最近一次因为硬盘故障重启设备的时间
#define HDERR_RBT_TIME	3600
 
/*************************************************************************
* 	函数名:hdutil_get_hderr_reboot_flag()
*	功能:	判断是否HDERR_RBT_TIME内已因为硬盘问题重启过
*	输入:	无
* 	返回值:	1表示应该重启，0表示不要重启
*************************************************************************/
int hdutil_get_hderr_reboot_flag(void)
{	
	FILE *fp = NULL;
	char buf[40];
	time_t time_lastrbt;
	time_t timenow;
	
	
	time(&timenow);
	if(access(HDERR_RBT_FILE,F_OK)==0)	//存在
	{
		
		fp = fopen(HDERR_RBT_FILE,"r+");
		if(fp!= NULL)
		{
			fgets(buf,40,fp);
			fclose(fp);
			time_lastrbt = atoi(buf);
			if((time_lastrbt <= 0) || ((timenow - time_lastrbt) < HDERR_RBT_TIME)) //最近一次rbt时间太近
				return 0;
		}	
	}
	
	//否则写入该文件并返回1
	fp = fopen(HDERR_RBT_FILE,"w+");
	if(fp!= NULL)
	{
		fprintf(fp,"%d",timenow);
		fclose(fp);
	}
	return 1;
}

//清理分区下的lost,found目录,返回值1表示清理到了旧文件，0表示没有
int init_lost_found( char *path)
{	
	char command[200];
	char filename[150];
	FILE *fp;
	int result = 0; //清理到了旧文件则置1
	
	if(path == NULL)
		return -EINVAL;
		
	mkdir("/var/tmp",0755);
  //here the variable path represents the mountpath.
	sprintf(command,"find %s/lost+found -name '*%s' > %s",path,IMG_FILE_EXT,RESULT_TXT);
	system(command);
	fp=fopen(RESULT_TXT,"r+");
	//逐行读出到filename并处理
 	while (fgets(filename,150,fp)!=NULL)
	{
		printf("relocating %s",filename);
		if(relocate_avi_file(path, filename) == 0) //移动了旧文件
			result = 1;
	}
	fclose(fp);
	remove(RESULT_TXT);

	//删除lost+found文件目录下的一切
	sprintf(command,"rm -rf %s/lost+found/*",path);
	system(command);
	
	return result;

}




//处理单个分区。被hdutil_init_all_partitions调用
//功能:	处理系统当前所有存在的分区，清理lostfound目录，初始化索引文件
int init_partitions_fn(IN  char * devname, IN  char * mountpath, IO void *arg)
{
	char cmd[200];
	int partition_total;
	int ret;
	
	if((devname ==NULL)||(mountpath == NULL))
		return -EINVAL;

	ret = init_lost_found(mountpath);
	if(ret == 1)//lost_found目录里清理到了旧文件		
		fileindex_create_index(mountpath,1); //强制重新创建索引文件
	else
		fileindex_create_index(mountpath,0); //若没有索引文件，才重新创建				
	return 0;
}



int recreate_db_fn(IN  char * devname, IN  char * mountpath, IO void *arg)
{
	char cmd[200];
	int partition_total;
	
	if(mountpath == NULL)
		return -EINVAL;
	
	fileindex_create_index(mountpath,1); //强制重新创建索引文件
	
	return 0;
}

 /*************************************************************************
 * 	函数名:	hdutil_create_dirs()
 *	功能:	根据文件名逐级判断是否存在该目录，如果没有则创建
 *	输入:	file,文件名
 *	输出:	
 * 	返回值:	0表示成功，负值表示失败
 *************************************************************************/
int hdutil_create_dirs(char *file)
{
	char dir[256];
	char t,*p;
	if(file==NULL)
		return -1;
	p=strrchr(file,'/');
	if(p==NULL)	//当前目录
		return 0;
	t=*p;
	*p='\0';
	if(access(file,F_OK)==0)
	{
		*p=t;	//有最后一层目录
		return 0;
	}

	*p=t;
	strncpy(dir,file,256);

	p=dir;
	t=*p;
	while(p!=NULL)
	{
		*p=t;
		p++;	//第一个字符不做处理
		p=strchr(p,'/');//第一个 /
		if(p==NULL)
			break;
		t=*p;
		*p='\0';
		
		if(access(dir,F_OK)==0)
		{
			continue;
		}
		if(mkdir(dir,0755)<0)
		{//不能创建目录
			return -errno;
		}
		
	}
	return 0;
	
}


 /*************************************************************************
 * 	函数名:	hdutil_lock_filename()
 *	功能:	将文件名加上锁标志
 *	输入:	filename,文件名
 *	输出:	tname,加锁后的文件名
 * 	返回值:	0表示成功，负值表示失败
 *************************************************************************/
int hdutil_lock_filename(char *filename,char* tname)
{
	char *p;
	if((filename==NULL)||(tname==NULL))
		return -1;
		
	strncpy(tname,filename,strlen(filename)+1);

	p=index(tname,LOCK_FILE_FLAG);
	if(p!=NULL)
	{
		//文件已经加锁		
		return 0;
	}
	p=strstr(tname,IMG_FILE_EXT);
	if(p==NULL)
	{
		//正在录像的文件不能被加锁
		return 0;
	}
	
	p=index(tname,REMOTE_TRIG_FLAG);
	if(p!=NULL)
	{
		sprintf(p,"%c%c%s",REMOTE_TRIG_FLAG,LOCK_FILE_FLAG,IMG_FILE_EXT);
		return 0;
		//已加标志	
	}	
	sprintf(strstr(tname,IMG_FILE_EXT),"%c%s",LOCK_FILE_FLAG,IMG_FILE_EXT);
	
	return 0;	
	
}

 /*************************************************************************
 * 	函数名:	hdutil_unlock_filename()
 *	功能:	将有锁定标志的文件去掉锁定标志
 *	输入:	filename,文件名
 *	输出:	tname,解锁后的文件名
 * 	返回值:	0表示成功，负值表示失败
 *************************************************************************/
int hdutil_unlock_filename(char *filename,char *tname)
{
	char *p;
	if((filename==NULL)||(tname==NULL))
		return -1;
	strncpy(tname,filename,strlen(filename)+1);
	p=index(tname,LOCK_FILE_FLAG);
	if(p==NULL)
	{//文件已经解锁		
		return 0;
	}
	if(strstr(tname,RECORDING_FILE_EXT)!=NULL)//ing文件  
		return 0;
	sprintf(p,"%s",IMG_FILE_EXT);
	return 0;
}


/*************************************************************************
 *	函数名	hdutil_init_all_partitions()
 *	功能:	处理系统当前所有存在的分区，
 			清理lostfound目录，
 			初始化索引文件 
 *	输入:	
 *	输出:	
 * 	返回值:	0
 *************************************************************************/
int hdutil_init_all_partitions(void)
{
	mpdisk_process_all_partitions(init_partitions_fn,NULL);
	return 0;
}


/*************************************************************************
 *	函数名	hdutil_recreat_db_for_all()
 *	功能:	处理系统当前所有存在的分区，
 			强制重新创建文件索引
 *	输入:	
 *	输出:	
 * 	返回值:	0
 *************************************************************************/
int hdutil_recreat_db_for_all(void)
{
	mpdisk_process_all_partitions(recreate_db_fn,NULL);
	return 0;
} 


//被hdutil+get_oldest_file_partition调用，
//如果输入的分区中最老的文件比之前在别的分区找到的还老，就将本分区定为"目前最老的分区"
int get_oldest_partition_fn(IN char * devname, IN char * mountpath, IO void * arg)
{
	int oldest_time_in_path ;

	struct oldest_filetime_struct * oldest;
	
	if((mountpath == NULL)||(arg == NULL))
		return -EINVAL;
	oldest = (struct oldest_filetime_struct *)arg;
	oldest_time_in_path = fileindex_get_oldest_file_time(mountpath);
	
	if((oldest_time_in_path > 0)&&(oldest_time_in_path < oldest->oldest_file_time))	
	{	
		oldest->oldest_file_time = oldest_time_in_path;
		strncpy(oldest->oldest_partition,mountpath,strlen(mountpath)+1);
		printf("%s:  oldest_partition %s\n",mountpath,oldest->oldest_partition);
		
	}
	else
	{
		//printf("oldest_time_in_path %s is %d,compare to %d\n",mountpath,oldest_time_in_path,*(oldest->oldest_file_time));
	}	
	return 0;

}
/*************************************************************************
 * 	函数名:	hdutil_get_oldest_file_partition()
 *	功能:	找到当前最早的可删除文件所在的分区
 *	输入:	
 *	输出:	oldest_partition:最早的可删除文件所在的分区
 * 	返回值:	0表示成功,负值表示没有找到
 *************************************************************************/
int hdutil_get_oldest_file_partition(OUT char* oldest_partition)
{
	int i,j;
	struct oldest_filetime_struct oldest;

	if(oldest_partition == NULL)
		return -EINVAL;
	
	oldest.oldest_file_time = 0xffffffff;

  //在每个分区都掉用一下get_oldest_partition_fn().
	mpdisk_process_all_partitions(get_oldest_partition_fn,&oldest);
	//printf("oldest_file_time is %d\n",oldest.oldest_file_time);		

  //执行到这里，已经将最老的那个文件名和这个文件所在的分区更新到了oldest结构中了。		
	if((oldest.oldest_file_time > 0)&&(oldest.oldest_file_time != 0xffffffff))
	{
    //返回最老文件所在的分区路径
		strncpy(oldest_partition, oldest.oldest_partition, strlen(oldest.oldest_partition)+1);
		return 0;
	}
	else
		return -ENOENT;

}




//按索引，锁当前目录下所有的录像文件
int lock_all_fn(IN char * devname, IN  char * mountpath, IO void *arg)
{
	int result = 0;
	char newname[200];
	int *mode;
	
	if((mountpath == NULL)||(arg == NULL))
		return -EINVAL;
	
	mode = (int *)arg;
	
	
	result = fileindex_lock_by_time(mountpath,*mode, -1,-1,-1,-1);

	return result;
}



/*************************************************************************
 * 	函数名:	hdutil_lock_all_files()
 *	功能:	加解锁所有的录像文件
 *	输入:	mode, 1为加锁，0为解锁
 *	输出:	
 * 	返回值:	0表示成功,负值表示失败
 *************************************************************************/
int hdutil_lock_all_files(int mode)
{
	return mpdisk_process_all_partitions(lock_all_fn,&mode);

}



/*************************************************************************
 * 	函数名:	hdutil_filename2dirs()
 *	功能:	为文件名解析出其应该创建的目录名称
 *	输入:	partition,形如"/hqdata/sda1"
 *			filename，形如HQ_C00_D20051024125934_L30_T00.AVI
 *  输出:	dir,形如/hqdata/sda1/2005/10/24/12/HQ_C00_D20051024125934_L30_T00.AVI
 * 	返回值:	0表示成功,负值表示失败
 *************************************************************************/
int hdutil_filename2dirs(char *partition, char *filename, char *dir)
{
	char *p;
	char path[200];
	char year[5];
	char month[3];
	char date[3];
	char hour[3];
	char part_name[20]; //形如"sda1"
	//printf("filename is %s\n",filename);
	if((partition == NULL)||(filename==NULL)||(dir==NULL))
		return -1;
		
	p=strstr(partition,"/sd");
	if(p==NULL)
		return -1;
	
	sprintf(part_name,p+1);
	
	p=strstr(filename,"_D");
	if(p==NULL)
		return -1;	
	p=p+2;
	strncpy(year,p,4);
	year[4]='\0';
	p=p+4;
	strncpy(month,p,2);
	month[2]='\0';
	p=p+2;
	strncpy(date,p,2);
	date[2]='\0';
	p=p+2;
	strncpy(hour,p,2);
	hour[2]='\0';
	sprintf(path,"%s/%s/%s/%s/%s/%s/%s",HDMOUNT_PATH,part_name,year,month,date,hour,filename);
	strncpy(dir,path,200);
	//printf("path is %s\n",path);
	return 0;
}



int hex2ascii(char s)
{
	int value;
	value=toupper(s);
	switch(s)
		{
			case('F'):  value=63;break;
			case('E'):  value=62;break;
			case('D'):  value=61;break;
			case('C'):  value=60;break;
			case('B'):  value=59;break;
			case('A'):  value=58;break;			
		}
	return (value);	
}

//获取指向字符串尾的指针
//字母和数字是有效字符，其他都是无效字符

char *get_strtail(char *pstr)
{
	char *p;
	if(pstr==NULL)
		return NULL;
	p=pstr;
	if(strlen(p)>100)
		return NULL;
	while(*p!=0)
	{
		if(isalnum(*p)==0)
			break; //不是字母或数字
		p++;
	}
	return p;
}


/*************************************************************************
 * 	函数名:	hdutil_filename2finfo()
 *	功能:	将文件名转换成对应的文件信息
 *	输入:	filename，文件名
 *  输出:	info,文件信息相关数据结构
 * 	返回值:	0表示成功,负值表示失败
 *************************************************************************/
int hdutil_filename2finfo(char *filename,struct file_info_struct *info)
{
	char name[101],ctemp;
	char temp[5];
	struct tm ftime;
	int namelen,i,plen;
	int trig;
	//BYTE val;
	int state=0; //0表示正在查找标记'-'
			    // 1表示刚查到一个标记'_'
	char *p,*t;
	
	if((filename==NULL)||(info==NULL))
		return -1;
	//memset((void*)info,0,sizeof(struct file_info_struct ));
	namelen=strlen(filename);
	if(namelen>100)
		return -1;
	memcpy(name,filename,namelen+1);
	p=strstr(filename,"sd");
	if(p!= NULL)
	{
		p+=4;  //p = "/2007"
		strncpy(info->partition,filename,p-filename);
		info->partition[12]='\0';
	}
	else
	{
		sprintf(info->partition,"%s","/hqdata/sda1");
		info->partition[12]='\0';
	}
	p=name;
	state=0;
	if(index(name,REMOTE_TRIG_FLAG)==NULL)
		info->remote=0;
	else
		info->remote=1;
	if(index(name,LOCK_FILE_FLAG)==NULL)
		info->lock=0;
	else
		info->lock=1;
	if(strstr(name,RECORDING_FILE_EXT)==NULL)
		info->ing_flag=0;
	else
		info->ing_flag=1;
	while((*p!='\n')&&(*p!='\0'))
	{

		if(state==0)
		{
			if(*p=='_')
			{
				state=1;
			}
			p++;
		}
		else
		{
			t=get_strtail(p);
			if(t==NULL)
			{
				//error
				return -1;
			}
			ctemp=*t;
			*t='\0';
			switch(*p)
			{
				case 'c':
				case 'C'://channel
					p++;
					info->channel=atoi(p);						
				break;
				case 'd'://日期，时间
				case 'D':
					p++;
					memcpy(temp,p,4);
					temp[4]='\0';
					p+=4;
					ftime.tm_year=atoi(temp)-1900;

					memcpy(temp,p,2);
					temp[2]='\0';
					p+=2;
					ftime.tm_mon=atoi(temp)-1;

					memcpy(temp,p,2);
					temp[2]='\0';
					p+=2;
					ftime.tm_mday=atoi(temp);
					
					memcpy(temp,p,2);
					temp[2]='\0';
					p+=2;
					ftime.tm_hour=atoi(temp);
					
					memcpy(temp,p,2);
					temp[2]='\0';
					p+=2;
					ftime.tm_min=atoi(temp);

					memcpy(temp,p,2);
					temp[2]='\0';
					p+=2;
					ftime.tm_sec=atoi(temp);

					info->stime=mktime(&ftime);
					
				break;
				case 'l':
				case 'L'://长度
					p++;
					info->len=atoi(p); 						
				break;
				case 't': //触发状态
				case 'T':
					p++;
					trig=0;
					plen=strlen(p);
					if(plen>4)
					{
						p+=(plen-4);
						plen=4;
					}
					
					//p+=plen;
					for(i=0;i<plen;i++)
					{
						trig<<=4;
						trig=trig+hex2ascii(*p)-'0';
						p++;
					}
					info->trig=trig;

				break;
				default:
				break;
			}
			*t=ctemp;
			p=t;		
			state=0;
		}
	
	}
	return 0;
}


