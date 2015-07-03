/*
 * 使配置文件增加新的节
 */
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <confparser.h>
#include <file_def.h>
#include <commonlib.h>
#include <errno.h>
#include "converts.h"
#include <devinfo.h>
#define VERSION "0.52"
int main(void)
{
	confdict *conf=NULL;
	FILE	*fp=NULL;
	int 	ret;
	conf=confparser_load_lockfile("/etc/vsftpd.conf",1,&fp);
	if(conf==NULL)
	{
		printf("can't parse /etc/vsftpd.conf\n");
		exit(1);
	}
	ret=confparser_setstr(conf,"xferlog_enable","NO");
	printf("ret=%d!\n",ret);
	confparser_dump_conf("/etc/vsftpd.conf",conf,fp);
	confparser_freedict(conf);	
	exit(0);
}

