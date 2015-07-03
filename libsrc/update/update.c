/*

	升级库文件 by wsy
*/
///// lsk 2009-5-18
#ifdef  __cplusplus
extern "C" {
#endif


#include "update.h"
#include <commonlib.h>
#include <gt_errlist.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <file_def.h>
#include <devinfo.h>
#include <xmlparser.h>
#include <gtlog.h>


#define PROG_PATH 		"/conf"
#define VAR_TMP_PATH 	"/var/tmp"
#define HD_UPDATE_PATH  "/ip1004"

#define RAMDISK_DEV 	"/dev/mtd/4"
#define KERNEL_DEV 		"/dev/mtd/3"
#define UPFILE 			"up"   //升级脚本文件

/*以输入的各项参数生成指定格式的升级命令字符串*/
int generate_updatemsg(IN char * username, IN char * pswd, IN char *ftpip, IN int port,
      IN char *path, OUT char *updatemsg)
{
	if(updatemsg==NULL)
		return -1;
	sprintf(updatemsg, "wget -c ftp://%s:%s@%s:%d%s",username,pswd,ftpip,port,path);	
	return 0;
}

/*
	根据升级包里的xml文件的Device字段，判断本设备是否需要升级
	返回值:1:需要升级 0:不需要升级 -1:错误
*/
int is_update_devtype(char * xmlfile)
{
	IXML_Document *document=NULL;
	IXML_NodeList *nodelist=NULL;
	IXML_Node 	  *node=NULL;
	IXML_Node	 *childnode=NULL;
//	IXML_Node	*child=NULL;
	DOMString	  value;
	char		   *devtype;
	
	DWORD bufdoc[20000];
	
	int number=0; //xml文件中含有Device字段的个数
	int i;
	int rc=0; //返回值
	
	if(xmlfile==NULL)
		return -1;
	
	devtype=get_devtype_str();
	if(devtype==NULL) //设备没有型号字符串,先升级了再说
		return 1;
	//gtloginfo("test!!!本设备为 %s\n",devtype); 

	document=(IXML_Document *)bufdoc;

	document=ixmlLoadDocument(xmlfile);
	if(document==NULL)
	{
		gtloginfo("xml解析器加载文件%s失败\n",xmlfile);
		return -1;
	}

	nodelist=ixmlDocument_getElementsByTagName(document,"Device");
	if(nodelist==NULL)
	{
		gtloginfo("xml解析器取device字段内容失败\n");
		return -1;
	}
	number=ixmlNodeList_length(nodelist);
	//gtloginfo("number is %d\n",number);
	if(number==0)
		return 0;
	
	for(i=0;i<number;i++)
	{
		node=ixmlNodeList_item(nodelist,i);
		if(node==NULL)
		{
			continue;
		}
		childnode=ixmlNode_getFirstChild(node);
		if(!childnode)
			continue;

	    value=ixmlNode_getNodeValue(childnode);
		if(value==NULL)
			{
				continue;
			}
		if(i==0)
			gtloginfo("升级包支持的设备列表: %d. %s\n",i+1,value);
		else
			gtloginfo("                      %d. %s\n",i+1,value);
		
		if(strncasecmp(value,devtype,100)==0) //符合要求
			{
				rc=1;
				//break;        ///<remed by shixin 为了能够将所有的型号记录日志
			}
	}

	return rc;
	
}


/*远程软件升级
  函数输入 updatefilesize 升级包大小，以字节为单位
  函数输入 updatemsg  升级信息,格式 "wget -c ftp://usr:pswd@192.168.1.160:8080/path/xxx.tar.gz"
  函数返回值 0为成功，否则为错误码(负的),包括负的以下值(如0- 0x1010)

	ERR_DVC_INVALID_NAME	0x1010  //升级文件名字格式错误
	ERR_DVC_LOGIN_FTP    	0x1011  //无法登录ftp服务器
	ERR_DVC_NO_FILE      	0x1012  //ftp服务器上无指定的文件或用户对其无读权限
	ERR_DVC_UNTAR        	0x1013  //解压文件失败
	ERR_NO_SPACE         	0x1014  //设备存储空间不够，无法升级
	ERR_DVC_PKT_NO_MATCH	0x1015	//升级包与设备型号不匹配
	ERR_EVC_NOT_SUPPORT		0x1006  //设备收到一个不支持的命令
 
*/



/*  函数名 check_update_space()
	功能: 清除旧的升级工作目录,检查磁盘空间是否足够,并填充update工作目录字符串
	输入: updatefilesize(单位:byte)
	输出: updatedir,升级的工作目录,如"/hqdata/update"
	返回值: -ERR_NO_SPACE: 空间不足
			RESULT_SUCCESS: 成功
*/
int check_update_space(IN int updatefilesize, OUT char * updatedir)
{
	//lc do 更改升级机制  不再使用/hqdata/update，直接查看/ip1004/下大小，并把升级包放置该目录，解压其中update脚本，执行该脚本，拷贝升级包至/conf/目录下
	//system("rm -rf /hqdata/update/*");
	//system("rm -rf /var/tmp/update/*");
	//mkdir ("/hqdata/update",0755);
	//if((get_disk_free(HDSAVE_PATH)<5*(updatefilesize>>20))||(access(HDSAVE_PATH,R_OK)!=0)||(access(HDSAVE_PATH,W_OK)!=0))
	if((get_disk_free(HD_UPDATE_PATH)<3*(updatefilesize>>20))||(access(HD_UPDATE_PATH,R_OK)!=0)||(access(HD_UPDATE_PATH,W_OK)!=0))
	
	{
			//if(get_disk_free(VAR_TMP_PATH)<5*(updatefilesize>>20)) //changed by shixin from 10
			printf("升级包%ld字节,需要%dM,update目录%dM\n",updatefilesize,3*(updatefilesize>>20),get_disk_free(HD_UPDATE_PATH));
			gtloginfo("升级包%ld字节,需要%dM,update目录%dM\n",updatefilesize,3*(updatefilesize>>20),get_disk_free(HD_UPDATE_PATH));
			/*
			if(get_disk_free(VAR_TMP_PATH)<3*(updatefilesize>>20)) //changed by shixin from 10
			{
				printf("no enough space, cannot update\n");
				gtloginfo("磁盘空间不足,无法升级\n");
	    		gtloginfo("升级包%ld字节,需要%dM,实际内存%dM，update目录%dM\n",updatefilesize,(updatefilesize>>20)*3,get_disk_free(VAR_TMP_PATH),get_disk_free(HD_UPDATE_PATH));
	    		return -ERR_NO_SPACE;
			}
			else 
			{	
				sprintf(updatedir,"%s/%s",VAR_TMP_PATH,"update");
				printf("work in %s instead\n",VAR_TMP_PATH);
			}
			*/
			return -ERR_NO_SPACE;
	}
	else 
		sprintf(updatedir,"%s",HD_UPDATE_PATH);

	printf("磁盘空间充足,在%s目录进行升级\n",updatedir);
	gtloginfo("磁盘空间充足,在%s目录进行升级\n",updatedir);
	
	
	return RESULT_SUCCESS;

}

void kill_apps4update(void)
{
    gtloginfo("为升级结束部分应用程序,暂停图像和录像服务...\n");
    printf("为升级结束部分应用程序,暂停图像和录像服务...\n");
 
	system("killall -15 watch_proc");
	system("killall -15 rtimage");
	system("killall -15 encbox");
	system("killall -15 diskman");
	system("killall -15 hdmodule");
	system("ipcrm -M 0x30000");
	system("ipcrm -M 0x30001");
	system("ipcrm -M 0x30002");
	system("ipcrm -M 0x30003");
	system("ipcrm -M 0x30004");
	system("ipcrm -M 0x30005");
    
}

int update_software(IN int updatefilesize,IN char *updatemsg,int interval)
{
	int errortimes=0;	
	int errortype=0;
	char *lp,*lk;
	int i;
	char update[50]; // /ip1004/ 工作目录
	char temp[100]; // //var/tmp/update/temp ,解压目录
	char filename[100]; //压缩包文件名
	char contentname[100];//update1.58-xx-xxx.txt,解压信息记录文件
	char xmlname[100];
	char makedir[100];
	char untar[100];
	char mv[100];
	char rmupdate[100];
	char up[100];
	char upname[100];
	char buf[2000];
	char tempcmd[100];
	FILE *fp;
	char xmlpath[200];
	struct stat filebuf;
	int is_devtype=0;//是否是xml中指定的devtype


	//0.清除升级目录
	memset(tempcmd,0,100);
	sprintf(tempcmd,"mkdir -p %s","/ip1004/");
	i=system(tempcmd);   
	sprintf(tempcmd,"mkdir -p %s/temp","/ip1004");  
	i=system(tempcmd);
	i=chdir("/ip1004/");
	sprintf(tempcmd,"cp /ip1004/hardreboot /tmp/ -frv");
	system(tempcmd);
	sprintf(tempcmd,"cp /ip1004/hwrbt /tmp/ -frv");
	system(tempcmd);
	sprintf(tempcmd,"rm -rf %s/*","/ip1004");
	system(tempcmd);
	//1.依次检查磁盘空间是否足够，不足够则直接返回错误码,否则置temp为工作目录
	i = check_update_space(updatefilesize, update);
	if(i != RESULT_SUCCESS)
		return i;
	
	//2.解析字符串，检查是否wget命令，(最后是否.tar.gz文件)
	//lp=strstr(updatemsg,".tar.gz");
	lk=strstr(updatemsg,"wget ");
	//if((lp==NULL)||((*(lp+7))!='\0')||(lk==NULL)||(lk!=updatemsg))  //wsy,不检查扩展名
	if((lk==NULL)||(lk!=updatemsg))
			{
			printf("invalid update msg name\n");
			gtloginfo("传来的updatemsg不合格式，取消升级");
	    	return -ERR_DVC_INVALID_NAME;
		}

	//3.errortimes置0,开始循环下载
	while(errortimes<5)
	{
		//3.1 通过系统服务建立一个干净的/ip1004/,并在/ip1004/下工作
		memset(tempcmd,0,100);
		sprintf(tempcmd,"cp /ip1004/hardreboot /tmp/ -frv");
		system(tempcmd);
		sprintf(tempcmd,"cp /ip1004/hwrbt /tmp/ -frv");
		system(tempcmd);
		sprintf(rmupdate,"rm -rf %s/*",update);  
		i=system(rmupdate);
		sprintf(makedir,"mkdir -p %s",update);
		i=system(makedir);   
		sprintf(makedir,"mkdir -p %s/temp",update);  
		i=system(makedir);
		i=chdir(update);

		//3.2 从updatemsg中取出相应的filename,并填充contentname,xmlname
		lp=rindex(updatemsg,'/');
		lp++;
		strcpy(filename,lp);
		sprintf(xmlname,"%s",lp);
//		lk=strstr(xmlname,".tar.gz");
//		*lk='\0';
		sprintf(contentname,"%s.txt",xmlname);
		sprintf(upname,"%s.%s",xmlname,UPFILE);
		strcat(xmlname,".xml");
		
		//3.3 下载tar包,遇错continue
#ifdef SHOW_WORK_INFO
	printf("updatemsg received: %s \n",updatemsg);		
#endif
		i=system(updatemsg);
		if(i!=0)
		{
			if(i==2)
					errortype=ERR_DVC_LOGIN_FTP;
			else
					errortype=ERR_DVC_NO_FILE;
			errortimes++;
			gtloginfo("第%d次下载不成功\n",errortimes);
			continue;
		}
		
		//3.4 检查下载的文件size
		stat(filename,&filebuf);
		if((int)filebuf.st_size!=updatefilesize)
			{
				printf("wrong size!!update %d, st_size %d\n",updatefilesize,(int)filebuf.st_size);
				gtloginfo("升级文件大小不符wrong size!!update %d, st_size %d\n",updatefilesize,(int)filebuf.st_size);
				errortimes++;
				errortype=ERR_DVC_WRONG_SIZE;
				continue;
			}

		//lc 2014-2-21 从接受文件开始计时，停止发送心跳，5分钟变为间隔
		//使能心跳功能
		if(interval > 0)
			update_set_com_mode(1,interval);

        kill_apps4update();
        
		//3.5 令成功下载的.tar.gz文件可执行，并解压
		chmod(filename,777);
		sprintf(temp,"%s/temp",update);
              //changed by shixin ,如果不把输入输出重定向的话则有可能出现解压缩失败的情况(由watch_proc 启动vsmain时)
		sprintf(untar,"tar zxvf %s -C %s/temp 0</dev/null 1> %s 2>>%s",filename,update,contentname,contentname);			
		printf("ready to untar upfile...%s\n",untar);
		i=system(untar);
		if(i!=0) //untar失败
			{
				printf("untar failed i=%d\n",i);
				gtloginfo("解压缩%s失败,ret=%d\n",filename,i);
				errortimes++;
				errortype=ERR_DVC_UNTAR;
				continue;
			}
		else
		{
			errortype=0;
			break;
		}
	}		

	if(errortype!=0)//若五次循环失败,放弃，退出
		{
			gtlogerr("五次下载解压都失败,错误为%s\n",get_gt_errname(errortype));	
			return -errortype;
		}

	//4 解压成功后检查是否有xml
#ifdef SHOW_WORK_INFO
		printf("content %s, xml %s",contentname,xmlname);
#endif
		fp=fopen(contentname,"r+");
		fread(buf,1,2000,fp);
		fclose(fp);
		if(strstr(buf,"xml")==NULL)
			{
				gtlogerr("解压后未发现xml文件\n");
				
				return -ERR_EVC_NOT_SUPPORT;
			}

	//5 解压成功后检查是否有UPFILE文件,并复制到/log/update/下
		chdir(temp);			 
		if(access(UPFILE,F_OK)!=0)
			{	
				gtlogerr("解压后未发现升级脚本\n");
				
				return -ERR_EVC_NOT_SUPPORT;
			}
		chmod(UPFILE,777);
		if(access("/log/update",F_OK)!=0);
			mkdir("/log/update",0755);
		
		sprintf(mv,"cp -f %s /log/update/%s",UPFILE,upname);
		system(mv);

	//6 将升级脚本文件复制更名到/log/update/下，并记日志

		sprintf(mv,"mv -f ../%s /log/update/",contentname);
		system(mv);
		sprintf(mv,"mv -f updatedesp.xml /log/update/%s",xmlname);
		system(mv);

		gtloginfo("收到升级命令包%s,备份文件于/log/update/%s\n",filename,xmlname);


//7 检查是否该给本设备升级 wsy add in Nov 2006
		
		
		sprintf(xmlpath,"/log/update/%s",xmlname);
		is_devtype=is_update_devtype(xmlpath);
		//gtloginfo("test,is_devtype结果是%d\n",is_devtype);
		if(is_devtype!=1)
			{
				gtlogerr("升级设备型号不符合,本设备是%s\n",get_devtype_str());
				return -ERR_DVC_PKT_NO_MATCH;
			}
		else
			{
				gtloginfo("升级设备型号符合,进行升级\n");
			}
		
	//8 执行升级
	    system("chmod +x ./up");//shixin added 
		sprintf(up,"/bin/sh %s 1>>/log/update/%s 2>>/log/update/%s",UPFILE,contentname,contentname);
		i=system(up);
		if(i!=0)
		{	
			gtlogwarn("执行升级脚本出错,返回%d\n",i);
			return -ERR_EVC_NOT_SUPPORT;
		}
		i=system(rmupdate);
		return 0;
		
	
}

/*
	功能: 将传入的tar.gz文件解压缩，并执行升级
	返回值: 零或错误码

*/
int direct_update_software(IN char* gzfilename, IN char *updatedir )
{
	char temp[100];
	char cmd[200];
	char contentname[100];
	char xmlname[100];
	char upname[100];
	BYTE buf[2000];
	char xmlpath[100];
	char *lk;
	FILE *fp;
	int result=0;
	
	chmod(gzfilename,777);

	sprintf(cmd,"rm -rf %s/temp",updatedir);
	system(cmd);

	sprintf(temp,"%s/temp",updatedir);
	sprintf(cmd, "mkdir -p %s",temp);
	system(cmd);

	//lc do 腾出内存空间
	printf("为/ip1004/腾出空间 begin!\n");
	system("killall -15 watch_proc");
	system("killall -15 rtimage");
	system("killall -15 encbox");
	system("killall -15 diskman");
	system("killall -15 hdmodule");
	system("ipcrm -M 0x30000");
	system("ipcrm -M 0x30001");
	system("ipcrm -M 0x30002");
	system("ipcrm -M 0x30003");
	system("ipcrm -M 0x30004");
	system("ipcrm -M 0x30005");
	printf("为/ip1004/腾出空间 end!\n");

	sleep(5);

	chdir(temp);
	//填充xmlname, contentname, upname
	/* /ip1004/up_direct_%04d%02d%02d.tar.gz */
	sprintf(xmlname,"%s",rindex(gzfilename,'/')+1);
	lk=strstr(xmlname,".tar.gz");
	*lk='\0';                           /*xmlname="up_direct_xxx"*/
	sprintf(contentname,"%s.txt",xmlname);    /*contentname="up_direct_xxx.txt"*/
	sprintf(upname,"%s.%s",xmlname,"up");   /*upname="up_direct_xxx.up"*/
	strcat(xmlname,".xml");             /*xmlname="up_direct_xxx.xml"*/
	
	
	
   	sprintf(cmd,"tar zxvf %s -C %s 0</dev/null 1> %s 2>>%s",gzfilename,temp,contentname,contentname);			
	printf("ready to untar upfile...%s\n",cmd);
	//sprintf(cmd,"tar zxvf %s -C %s %s 0</dev/null 1> %s 2>>%s",gzfilename,temp,,contentname,contentname);			
	//printf("ready to untar upxmlfile...%s\n",cmd);
	result=system(cmd);

	
	if(result!=0) //untar 失败
	{
		printf("untar failed ret=%d\n",result);
		gtlogerr("解压缩%s失败,ret=%d\n",gzfilename,result);
		return ERR_DVC_UNTAR;
	}
	
#ifdef SHOW_WORK_INFO
	printf("content %s, xml %s",contentname,xmlname);
#endif
	fp=fopen(contentname,"r+");
	fread(buf,1,2000,fp);
	fclose(fp);
	if(strstr((const char*)buf,"xml")==NULL)	// lsk 2009 -5-18
	{
		gtlogerr("解压后未发现xml文件\n");
		return ERR_EVC_NOT_SUPPORT;
	}

//解压成功后检查是否有UPFILE文件,并复制到/log/update/下
	chdir(temp);
	system("pwd");
	if(access(UPFILE,F_OK)!=0)
	{	
		gtlogerr("解压后未发现升级脚本\n");
		return ERR_EVC_NOT_SUPPORT;
	}
	chmod(UPFILE,777);
	if(access("/log/update",F_OK)!=0);
	mkdir("/log/update",0755);

	sprintf(cmd,"cp -f %s /log/update/%s",UPFILE,upname);
	system(cmd);

// 将升级脚本文件复制更名到/log/update/下，并记日志

	sprintf(cmd,"mv -f ./%s /log/update/",contentname);
	system(cmd);
	sprintf(cmd,"mv -f updatedesp.xml /log/update/%s",xmlname);
	system(cmd);

	gtloginfo("升级备份文件于/log/update/%s\n",xmlname);


	// 检查是否该给本设备升级 
		
		
	sprintf(xmlpath,"/log/update/%s",xmlname);

	//gtloginfo("test,is_devtype结果是%d\n",is_devtype);
	if(is_update_devtype(xmlpath)!=1)
	{
		gtlogerr("升级设备型号不符合,本设备是%s\n",get_devtype_str());
		return ERR_DVC_PKT_NO_MATCH;
	}
	else
	{
		gtloginfo("升级设备型号符合,进行升级\n");
	}
		
	// 执行升级
	system("pwd");
	chdir(temp);
    system("chmod 777 up");//lc added 
    //system("cd ./ip1004/");
	sprintf(cmd,"/bin/sh %s 1>>/log/update/%s 2>>/log/update/%s",UPFILE,contentname,contentname);
	result=system(cmd);
	if(result!=0)
	{	
		gtlogwarn("执行升级脚本出错,返回%d\n",result);
		return ERR_EVC_NOT_SUPPORT;
	}
		
	return 0;
}
#ifdef  __cplusplus
}
#endif

