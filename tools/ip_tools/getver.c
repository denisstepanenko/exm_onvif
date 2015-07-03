/*
 * 读取指定的目录，将目录中的程序版本信息显示在屏幕 
 */
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <commonlib.h>
#include <sys/stat.h>
#define LOCK_FILE_DIR	"/lock/ipserver/"


int main(void)
{
	int ret;
	struct dirent **namelist;
        struct dirent *name;
	char filename[256];
	char version[256];
	char prog_name[100];
	int total,i;
	struct stat statbuf;
	char *ptr;
	//printf("getver LOCK_FILE_DIR is %s\n",LOCK_FILE_DIR);
	total=scandir(LOCK_FILE_DIR,&namelist,0,alphasort);
	//printf("getver scandir total is %d\n",total);
	if(total<=2)
	{
		printf("没有找到运行的ip1004应用程序!!\n");
		exit(1);
	}

	//chdir(LOCK_FILE_DIR);
	
	for(i=0;i<total;i++)
	{
		name=namelist[i];
		sprintf(filename,"%s%s",LOCK_FILE_DIR,name->d_name);
		lstat(filename,&statbuf);
/*		
			if(S_ISDIR(statbuf.st_mode))
				ptr = "dir";
			else if(S_ISCHR(statbuf.st_mode))
				ptr = "char";
			else if(S_ISBLK(statbuf.st_mode))
				ptr = "blk";
			else if(S_ISFIFO(statbuf.st_mode))
				ptr = "fifo";
			else if(S_ISREG(statbuf.st_mode))
				ptr = "reg";
			else 
				ptr = "unknow";

			printf("%s is %s \n",name->d_name,ptr);
*/
		if(S_ISDIR(statbuf.st_mode)==1)
			continue;
		else if(S_ISREG(statbuf.st_mode)==1)
		{
			memset(prog_name,' ',sizeof(prog_name));
			sprintf(prog_name,"%s",name->d_name);
			//sprintf(filename,"%s%s",LOCK_FILE_DIR,name->d_name);
			prog_name[strlen(name->d_name)]=(char)' ';
			prog_name[20]='\0';
			//printf("filename is %s\n",filename);
			ret=get_prog_version(version,filename);
			if(ret==0)
			{
				printf("%s:%s\n",prog_name,version);
			}
			else
				printf("%s:unknow\n",prog_name);
		}
	}
	while(total--) 
		free(namelist[total]);
	free(namelist);
	
	
	return 0;
}

