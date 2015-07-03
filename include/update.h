
#ifndef _UPDATE_H_
#define _UPDATE_H_

#define IN
#define OUT
#define INOUT

void kill_apps4update(void);

/*  函数名 check_update_space()
	功能: 清除旧的升级工作目录,检查磁盘空间是否足够,并填充update工作目录字符串
	输入: updatefilesize(单位:byte)
	输出: updatedir,升级的工作目录,如"/hqdata/update"
	返回值: -ERR_NO_SPACE: 空间不足
			RESULT_SUCCESS: 成功
*/
int check_update_space(IN int updatefilesize, OUT char * updatedir);


/*以输入的各项参数生成指定格式的升级命令字符串*/
int generate_updatemsg(IN char * username, IN char * pswd, IN char *ftpip, IN int port,
      IN char *path, OUT char *updatemsg);

      
/*远程软件升级
  函数输入 updatefilesize 升级包大小，以字节为单位
  函数输入 updatemsg  升级信息,格式 "wget -c ftp://usr:pswd@192.168.1.160:8080/path/xxx.tar.gz"
  函数返回值 0为成功，否则为错误码	
*/	
int update_software(int updatefilesize,char *updatemsg,int interval);


/*
	功能: 将传入的tar.gz文件解压缩，并执行升级
	返回值: 零或错误码

*/
int direct_update_software(IN char* gzfilename, IN char *updatedir );

#endif
