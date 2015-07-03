//与远程进行文件交互的函数
#include "ipmain.h"
#include "remote_file.h"
#include <gate_cmd.h>
#include <devinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <commonlib.h>
#include <gt_errlist.h>
#include "ipmain_para.h"
enum REMOTE_FILE_TYPE{
	RMT_FILE_ERR=1,		//设备处理请求时有错误发生
	RMT_FILE_GTPARA=2,	//ip1004.ini .gz格式
	RMT_FILE_ALARMPARA=3,	//alarm.ini .gz格式
	RMT_FILE_CERT=6,			//证书文件,.zip格式,包含三个文件dev-gateway.crt dev-peer.crt dev-peer.key
	RMT_FILE_GTINFO=7,          //设备档案信息文件gtinfo.dat
};




//直接向网关发送一条命令的响应，而不通过统一的消息服务线程
int send_gate_ack_direct(int fd,WORD rec_cmd,WORD result,int env,int enc,int dev_no)
{
	DWORD send_buf[25];//响应命令包不会超过100字节
	struct gt_pkt_struct *send=NULL;
	struct gt_usr_cmd_struct *cmd;
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);
	int rc;
	getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);
	
	send=(struct gt_pkt_struct *)send_buf;
	cmd=(struct gt_usr_cmd_struct *)send->msg;
	cmd->cmd=DEV_CMD_ACK;
	cmd->en_ack=0;
	cmd->reserve0=0;
	cmd->reserve1=0;
	rc=virdev_get_devid(dev_no,cmd->para);	

	memcpy(&cmd->para[rc],(char *)&result,2);
	memcpy(&cmd->para[rc+2],(char *)&rec_cmd,2);
	cmd->len=rc+SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para) -2+4;
	rc=gt_cmd_pkt_send(fd,send,(cmd->len+2),NULL,0,env,enc);
#ifdef SHOW_WORK_INFO
	printf("%s向%s发送命令%04x的结果(direct)%04x rc=%d",devlog(dev_no),inet_ntoa(peeraddr.sin_addr),rec_cmd,result,rc);
#endif
	gtloginfo("%s向%s发送命令%04x的结果(direct)%04x rc=%d",devlog(dev_no),inet_ntoa(peeraddr.sin_addr),rec_cmd,result,rc);
	return 0;	
}


/*
	将操作配置文件出错的信息发送给远程计算机
*/
static int send_rw_para_file_err(int fd,int env,int enc,int dev_no)
{
	return send_gate_ack_direct(fd,USR_RW_DEV_PARA,ERR_DVC_UPDATE_FILE,env,enc,dev_no);
}

	    
//处理远程的写操作
/*
	返回值  -1 参数错误
			    -2 不能打开指定的文件
			    -3 文件格式错误(不能解压缩文件)
			    -4 证书文件错误
			    -5 设备内部错误
*/		
static int process_remote_write(int devno, int filetype,char *filebuf,int buflen)
{
	#define RECV_PARA_TMP "/tmp/para"
	int ret;
	char tbuf[256];
	FILE *fp=NULL;
	char *filename=NULL;		//配置文件名及最终路径
	char *name=NULL;		//配置文件名(不包括路径)
	char tmpfile[100];			//临时存放数据的文件
	char savefile[100];
	char send_dir[100];		//存放发送文件的目录	
	char *crt_file=NULL;		//远程发来的证书文件名
	char *key_file=NULL;		//远程发来的key文件名

	sprintf(send_dir,"%s/%d/dev%d",RECV_PARA_TMP,getpid(),devno);
#if EMBEDED==1
	//删除以前的临时目录
	sprintf(tbuf,"rm -rf %s\n",send_dir);
	system(tbuf);
#endif		
	//创建临时目录
	sprintf(tbuf,"mkdir -p %s\n",send_dir);
	system(tbuf);

	sprintf(tmpfile,"%s/rmt.dat",send_dir);
	
	fp=fopen(tmpfile,"w");
	if(fp==NULL)
	{
		printf("不能创建临时文件%s\n",tmpfile);
		gtloginfo("不能创建临时文件%s\n",tmpfile);
		return -2;
	}
	fwrite(filebuf,1,buflen,fp);
	fclose(fp);	
	switch(filetype)
	{
		case RMT_FILE_GTPARA:		//ip1004.ini
		case RMT_FILE_ALARMPARA:	//alarm.ini
			//将收到的内容写入一个文件
			/*
			if(virdev_get_virdev_number()==2)
			{
				switch(devno)
				{
					case (0):	filename =  MAIN_PARA_FILE_DEV0;
								break;
					case (1):	filename = 	MAIN_PARA_FILE_DEV1;
								break;
					default :	break;
				}
			}
			else
			*/
			filename = IPMAIN_PARA_FILE;
			name=strrchr(filename,'/');	//找到最后一个 '/'以确定文件名
			if(name==NULL)
				name=filename;
			else
				name++;
			//更名
			sprintf(savefile,"%s/%s.gz",send_dir,name);
			sprintf(tbuf,"mv %s %s",tmpfile,savefile);
			ret=system(tbuf);
			
			//解压缩文件
			sprintf(tbuf,"gunzip %s",savefile);
			ret=system(tbuf);
			if(ret!=0)
			{
				gtloginfo("解压缩文件%s出错\n",savefile);
				return -3;
			}
			//将文件拷贝到指定目录的指定文件
			sprintf(tbuf,"/ip1004/iniset %s/%s %s",send_dir,name,filename);
			//sprintf(tbuf,"cp %s/%s %s -f",send_dir,name,filename);
			ret=system(tbuf);
			gtloginfo("更新配置文件%s/%s->%s ret=%d",send_dir,name,filename,ret);
			/*
			if(virdev_get_virdev_number()==2)
			{
				sprintf(tbuf,"/ip1004/ini_conv -m");
				ret=system(tbuf);
			}
			*/
			//wsy add,转换配置文件
			sprintf(tbuf,"/ip1004/para_conv -s");
			ret=system(tbuf);
			printf("设备硬复位...\n");
			gtloginfo("设备硬复位...\n");
			system("/ip1004/hwrbt 5");
		break;
		case RMT_FILE_CERT://证书文件
			sprintf(tbuf,"unzip -j -d %s %s",send_dir,tmpfile);			
			ret=system(tbuf);
			if(ret!=0)
			{
				gtloginfo("解压缩文件%s出错\n",tmpfile);
				ret=-3;
				goto out;
			}
			crt_file=alloca(256);
			key_file=alloca(256);
			if((crt_file==NULL)||(key_file==NULL))
			{
				gtlogerr("%s:%d 堆栈内存不足!\n",__FILE__,__LINE__);
				ret=-5;
				goto out;
			}
			sprintf(crt_file,"%s/dev-gateway.crt",send_dir);
			sprintf(key_file,"%s/dev-peer.key",send_dir);
			//lc do 验证证书
			ret=env_check(crt_file,key_file);
			printf("远程发来的证书文件进行验证!\n");
			ret = 0;
			if(ret!=0)
			{
				gtlogerr("远程发来的证书文件错误:%d!!\n",ret);
				ret=-4;
				goto out;
			}
			
			sprintf(tbuf,"cp -f %s/dev-peer.crt %s/dev-peer.key %s/dev-gateway.crt /conf/cert",send_dir,send_dir,send_dir);
			ret=system(tbuf);
			gtloginfo("设备证书已更新,软复位...!\n");
			system("/ip1004/swrbt 3");
		break;
              case RMT_FILE_GTINFO:///档案信息文件
                     filename="/conf/gtinfo.dat";
                     sprintf(tbuf,"rm -f %s",filename);
                     system(tbuf);
                     sprintf(tbuf,"mv %s %s",tmpfile,filename);
                     system(tbuf);
                     printf("设备信息文件:%s 更新!\n",filename);
                     gtloginfo("设备信息文件:%s 更新!\n",filename);
                   
              break;
		default:
			       printf("w:不支持的文件类型:%d!\n",filetype);
			gtloginfo("w:不支持的文件类型:%d!\n",filetype);
			return -1;
		break;
	}

out:

	#if EMBEDED==1
	//删除用过的目录
		sprintf(tbuf,"rm -rf %s\n",send_dir);
		system(tbuf);
	#endif	
	
	return 0;

}


//和网关传递配置文件使用的文件格式 
/*访问设备配置文件 USR_RW_DEV_PARA
使用此命令来读写设备的参数配置文件（目前只在工程安装软件里面使用）,
设备收到此命令后应该以DEV_PARA_RETURN返回
   0x0108 USR_RW_DEV_PARA FTP文件下载
  {
  	  type(2)         类型（2:ip1004.ini 3:alarm.ini其他值保留）
	  mode(2)		操作模式:0：读 1：写
	  filelen(4)		文件长度(在写模式下有效,长度不能超过60k)
	  file(n)			文件内容(仅在写模式下有效)
}

*/
int usr_rw_para_file(int fd,struct gt_usr_cmd_struct *cmd,int env,int enc,int dev_no)
{
	struct usr_rwdevpara_struct *set;
//	char temp_file[100];
	int filetype;
	int ret=0;
	if((fd<0)||(cmd->cmd!=USR_RW_DEV_PARA))
		return -1;
	set=(struct usr_rwdevpara_struct *)cmd->para;
#ifdef SHOW_WORK_INFO
	printf("收到读写%s USR_RW_DEV_PARA 命令type=%d mode=%d \n",devlog(dev_no),set->type,set->mode);
#endif
	gtloginfo("收到读写%s USR_RW_DEV_PARA 命令type=%d mode=%d \n",devlog(dev_no),set->type,set->mode);
	filetype=set->type;
	if(set->mode==1)
	{//写
		if(process_remote_write(dev_no,filetype,set->file,set->filelen)==0)
		{//写入成功
			ret=send_gate_ack_direct(fd,USR_RW_DEV_PARA,RESULT_SUCCESS,env,enc,dev_no);
		}
		else
		{//写入失败
			return send_rw_para_file_err(fd,env,enc,dev_no);	
		}
	}
	else
	{//读
		ret = send_para_to_rmt(fd,filetype,env,enc,0,dev_no);
		if(ret<0)
		{
			//SendGateAckDirect(fd,USR_RW_DEV_PARA,ERR_DVC_INTERNAL,env,enc);
			send_rw_para_file_err(fd,env,enc,dev_no);
		}
	}
	return ret;
}
#if 0
/*
	接收到的文件是gz格式的
	接收一个远程文件到本地
	存放到预先定义好的路径
	收到的文件是gz格式的
	type: 2:ip1004.ini 3:alarm.ini
	返回值  -1 参数错误
			    -2 不能打开指定的文件
			    -3 文件格式错误(不能解压缩文件)
*/
static int RecvARmtPara(int type,char *filebuf,int filelen)
{
	#define RECV_PARA_TMP 	"/tmp/recvpara"
	char *filename=NULL;//配置文件名及最终路径
	char *name=NULL;		//配置文件名(不包括路径)
	char tbuf[100];
	char savefile[100];
	int ret;
	FILE *fp=NULL;
	if((filebuf==NULL)||(filelen<0))
		return -1;
	if(type==2)	
		filename=VSMAIN_PARA_FILE;
	else if(type==3)
		filename=MOTION_ALARM_PARA_FILE;
	else
	{
		gtloginfo("RecvARmtPara() 参数错误:不支持的type:%d\n",type);
		return -1;
	}
	gtloginfo("准备更新配置文件%s...\n",filename);
	name=strrchr(filename,'/');	//找到最后一个 '/'以确定文件名
	if(name==NULL)
		name=filename;
	else
		name++;
	
#if EMBEDED==1
	//删除以前的临时目录
	sprintf(tbuf,"rm -rf %s\n",RECV_PARA_TMP);
	system(tbuf);
#endif	
	//创建临时目录
	sprintf(tbuf,"mkdir -p %s\n",RECV_PARA_TMP);
	system(tbuf);

	//将文件写入一个文件
	sprintf(savefile,"%s/%s.gz",RECV_PARA_TMP,name);
	fp=fopen(savefile,"w");
	if(fp==NULL)
	{
		gtloginfo("不能创建临时文件%s\n",savefile);
		return -2;
	}
	fwrite(filebuf,1,filelen,fp);
	fclose(fp);

	//解压缩文件
	sprintf(tbuf,"gunzip %s",savefile);
	ret=system(tbuf);
	if(ret!=0)
	{
		gtloginfo("解压缩文件%s出错\n",savefile);
		return -3;
	}
	//将文件拷贝到指定目录的指定文件
	sprintf(tbuf,"cp %s/%s %s -f",RECV_PARA_TMP,name,filename);
	ret=system(tbuf);
	gtloginfo("更新配置文件%s/%s->%s ret=%d",RECV_PARA_TMP,name,filename,ret);

	//wsy add,转换配置文件
	sprintf(tbuf,"/ip1004/para_conv -s");
	ret=system(tbuf);
	
#if EMBEDED==1
	//删除用过的临时目录
	sprintf(tbuf,"rm -rf %s\n",RECV_PARA_TMP);
	system(tbuf);
#endif		
	
	return 0;

	
}
#endif


/*
	将一个文件发送到远程计算机
*/
int SendAFile2Rmt(int fd,char *filename,int type,int env,int enc,int enack,int dev_no)
{
	struct return_para_struct *rtn;
	struct stat filestat;
	char *sendfile;
	FILE *fp;
	void *sendbuf=NULL;
	struct gt_pkt_struct *send=NULL;
	struct gt_usr_cmd_struct *sendcmd;

	int ret;

	if(fd<0)
		return -EINVAL;
	if(filename==NULL)
		return -EINVAL;
	sendfile=filename;

	ret=stat(sendfile,&filestat);
	if(ret!=0)
		return -ENOENT;

	fp=fopen(sendfile,"rb");
	if(fp==NULL)
		return -ENOENT;

	ret=posix_memalign(&sendbuf,sizeof(unsigned long),filestat.st_size+1024);
	if((ret!=0)||(sendbuf==NULL))
		return -ENOMEM;

	send=(struct gt_pkt_struct *)sendbuf;
	sendcmd=(struct gt_usr_cmd_struct *)send->msg;
	sendcmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(sendcmd->para)-2+sizeof(struct return_para_struct)-4+filestat.st_size;
	sendcmd->cmd=DEV_PARA_RETURN;
	sendcmd->en_ack=enack;
	sendcmd->reserve0=0;
	sendcmd->reserve1=0;
	rtn=(struct return_para_struct *)sendcmd->para;
	virdev_get_devid(dev_no,rtn->dev_id);
	rtn->type=type;
	rtn->filelen=filestat.st_size;
	rtn->reserve=0;
	fread(rtn->file,1,filestat.st_size,fp);
	ret=gt_cmd_pkt_send(fd,send,(sendcmd->len+2),NULL,0,env,enc);

	gtloginfo("将%s发送给远程计算机ret=%d\n",sendfile,ret);
	if(sendbuf!=NULL)
		free(sendbuf);		
	if(fp!=NULL)
		fclose(fp);		
	return ret;		
}

//将一些附加信息加入ini文件
//设备固件版本号,设备当前时间等
static int add_info2ini(char *filename)
{
	struct tm 	*ptime;
	time_t 		ctime;	
	char			timestr[50];
	dictionary      *ini;
	char			firmware[100];
	char			rtversion[30];
	char			hdversion[30];
	FILE*           lockfp=NULL;
	ini=iniparser_load_lockfile(filename,1,&lockfp);
        if (ini==NULL) 
        {
             printf("vsmain  cannot parse ini file file [%s]", filename);
             return -1 ;
        }

	//获取设备程序版本号
	memset(firmware,0,sizeof(firmware));
	get_prog_version(rtversion,RT_LOCK_FILE);
	get_prog_version(hdversion,HDMOD_LOCK_FILE);
	sprintf(firmware,"%s-%s-%s-k%s",VERSION,rtversion,hdversion,get_kernel_version());

	//获取设备当前时间
	ctime=time(NULL);
	ptime=localtime(&ctime);
	sprintf(timestr,"%04d-%02d-%02d %02d:%02d:%02d",ptime->tm_year+1900,
												    ptime->tm_mon+1,
												    ptime->tm_mday,
												    ptime->tm_hour,
												    ptime->tm_min,
												    ptime->tm_sec);
	printf("filename is %s\n",filename);
	iniparser_setstr(ini,"devinfo:firmware",firmware);
	iniparser_setstr(ini,"devinfo:cur_time",timestr);
	save_inidict_file(filename,ini,&lockfp);
	
	iniparser_freedict(ini);	
	return 0;
}

/**********************************************************************************************
 * 函数名	:send_para_to_rmt()
 * 功能	:将设备的配置文件发送给远程计算机
 * 输入	:fd 连接到远程计算机的文件描述符，fd不可以是负值
 *			 type:文件 值定义同struct usr_rwdevpara_struct中的定义 
 *         			（2:ip1004.ini 3:alarm.ini其他值保留）
 *			env:使用的数字信封格式
 *			enc:要使用的加密格式
 *			enack:是否需要确认
 *返回值	:0表示成功负值表示失败
 *			: -10 参数错误
 *		   	: -11不支持的格式
 **********************************************************************************************/
int send_para_to_rmt(int fd,int type,int env,int enc,int enack,int dev_no)
{
	#define SEND_PARA_TMP 	"/tmp/para"
	int ret;
	char tbuf[256];
	char *para_file=NULL;
	const char *file_name;
	char send_name[100];
	char send_dir[100];//存放发送文件的目录
	char tmp_file[100];// 临时ini文件
	if(fd<0)
		return -1;
	sprintf(send_dir,"%s/%d/dev%d",SEND_PARA_TMP,getpid(),dev_no);
#if EMBEDED==1
	//删除以前的临时目录
	sprintf(tbuf,"rm -rf %s\n",send_dir);
	system(tbuf);
#endif	
	//创建临时目录
	sprintf(tbuf,"mkdir -p %s\n",send_dir);
	system(tbuf);
	switch(type)
	{
		case RMT_FILE_GTPARA:
		case RMT_FILE_ALARMPARA:	//发送配置文件ip1004.ini .gz格式
		para_file=IPMAIN_PARA_FILE;//MOTION_ALARM_PARA_FILE不使用了
			
		file_name=strrchr(para_file,'/');	//找到最后一个 '/'以确定文件名
		if(file_name==NULL)
			file_name=para_file;
		else
			file_name++;

		//zw-modified 2011-11-12 同时修改发送文件和配置文件
		add_info2ini(para_file);//zsk modified， 以前是tmp_file 

		//拷贝一个副本
		sprintf(tbuf,"cp -f %s %s/%s\n",para_file,send_dir,file_name);
		system(tbuf);
		sprintf(tmp_file,"%s/%s",send_dir,file_name);
		printf("KKK TEST %s\n",tmp_file);
		sprintf(tbuf,"gzip %s/%s\n",send_dir,file_name);
		system(tbuf);

		sprintf(send_name,"%s/%s.gz",send_dir,file_name);

	
		break;
		case RMT_FILE_CERT:
			sprintf(send_name,"%s/cert.zip",send_dir);
			sprintf(tbuf,"zip %s %s %s %s ",send_name,DEV_CERT_FILE,CERT_FILE,KEY_FILE);
			system(tbuf);
	
		break;
              case RMT_FILE_GTINFO:
                    sprintf(tbuf,"cp -f /conf/gtinfo.dat /var/tmp/gtinfo%d.dat",getpid());
                    system(tbuf);
                    
                    sprintf(send_name,"/var/tmp/gtinfo%d.dat",getpid());
                    if(check_file(send_name)==0)
                    {//没有文件则创建一个空文件
                        sprintf(tbuf,"touch %s",send_name);
                        system(tbuf);
                    }
              break;
		default:
			       printf("r:不支持的文件类型:%d!\n",type);
			gtloginfo("r:不支持的文件类型:%d!\n",type);
			return -1;
		break;		
	}

	ret=SendAFile2Rmt(fd,send_name,type, env, enc,enack,dev_no);
	//删除用过的文件
	sprintf(tbuf,"rm -f %s\n",send_name);
	system(tbuf);			
	#if EMBEDED==1
	//删除用过的目录
		sprintf(tbuf,"rm -rf %s\n",send_dir);
		system(tbuf);
	#endif			

	return ret;		
}

