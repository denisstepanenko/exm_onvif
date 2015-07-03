/* ip2004 系统设备信息处理函数 
 *
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <iniparser.h>
#include <devinfo.h>
#include "guid.h"
#include "devtype.h"

#if EMBEDED==0
//	#define FOR_PC_MUTI_TEST		//支持在同一台pc机上启动多个程序
#endif

#include <file_def.h>

static int init_flag=0;			///已经调用过init_devinfo标志
static const char version[]="0.04";	//devinfo库的版本号
// 0.04 zw 2010-07-09 修改devtype_gtvs3000.h中的GTMV3121的ide值为3，表示用的是TF卡，加以区分SD卡
// 0.03 zw 2010-06-18 给gtvs3024L添加输入端子数6，输出端子数4
// 0.02 增加对gtvm的支持,增加获取版本号的接口get_prog_ver()
// 0.01 开始将devinfo改为动态库

#define	DEV_GUID_BYTE		8	//GUID占用的字节数
typedef struct{
	GTSeriesDVSR	*dvsr;					//设备描述
	unsigned char 	guid[DEV_GUID_BYTE];		//设备GUID
	char		guid_str[DEV_GUID_BYTE*4];	//guid的字符串描述
	int		disk_capacity;			//磁盘容量以M为单位
	struct	tm	lv_fac_time;			//设备出厂时间
	char		batch_seq[100];			//设备生产批次
	char		board_seq[100];			//设备板卡批次
	char 		prog_ver_str[256];              ///<应用程序版本号xxx-xxx-xxx
	int		lfc_flag;			//设备出厂标志，1=出厂，0=在家 //2008-06-26 zw add
}devinfo_struct;



static devinfo_struct info={
	.dvsr			=	NULL,
	.disk_capacity		=	0,

};
static  char *def_dev_guid="0067007400000000";//{0x0,0x0,0x0,0x0,'t',0x0,'g',0x0};//static int dev_type =0; //设备类型

GTSeriesDVSR	*get_current_dvsr(void)
{
	return info.dvsr;
}
//检查devinfo.ini中的变量是否和对应的型号信息一致
//返回负值表示出错 0表示完全一致 1表示不一致，并且已经设置到ini结构中了
static int fix_devinfo_file(dictionary      *ini,GTSeriesDVSR	*dvsr)
{
	int change_flag=0;
	int num;
	char *pstr=NULL;
	if((ini==NULL)||(dvsr==NULL))
		return -EINVAL;
	//型号代码
	num=iniparser_getint(ini,"devinfo:devtype",-1);
	if(num!=dvsr->type)
	{
		iniparser_setint(ini,"devinfo:devtype",dvsr->type);
		gtloginfo("devinfo:devtype %d->%d",num,dvsr->type);
		change_flag=1;
	}

	//触发端子数
	num=iniparser_getint(ini,"resource:trignum",-1);
	if(num!=dvsr->trignum)
	{
		iniparser_setint(ini,"resource:trignum",dvsr->trignum);
		gtloginfo("resource:trignum %d->%d",num,dvsr->trignum);
		change_flag=1;
	}
	
	//输出端子数
	num=iniparser_getint(ini,"resource:outnum",-1);
	if(num!=dvsr->outnum)
	{
		iniparser_setint(ini,"resource:outnum",dvsr->outnum);
		gtloginfo("resource:outnum %d->%d",num,dvsr->outnum);
		change_flag=1;
	}

	//串口数
	num=iniparser_getint(ini,"resource:com",-1);
	if(num!=dvsr->com)
	{
		iniparser_setint(ini,"resource:com",dvsr->com);		
		gtloginfo("resource:com %d->%d",num,dvsr->com);
		change_flag=1;
	}

	//是否有画面分割器
	num=iniparser_getint(ini,"resource:quad",-1);
	if(num!=dvsr->quad)
	{
		iniparser_setint(ini,"resource:quad",dvsr->quad);
		gtloginfo("resource:quad %d->%d",num,dvsr->quad);
		change_flag=1;
	}

	//视频输入数
	num=iniparser_getint(ini,"resource:videonum",-1);
	if(num!=dvsr->videonum)
	{
		iniparser_setint(ini,"resource:videonum",dvsr->videonum);
		gtloginfo("resource:videonum %d->%d",num,dvsr->videonum);
		change_flag=1;
	}

	num=iniparser_getint(ini,"resource:audionum",-1);
        if(num!=dvsr->audionum)
        {
                iniparser_setint(ini,"resource:audionum",dvsr->audionum);
                gtloginfo("resource:audionum %d->%d",num,dvsr->audionum);
                change_flag=1;
        }

	//视频编码器数
	num=iniparser_getint(ini,"resource:videoencnum",-1);
	if(num!=dvsr->videoencnum)
	{
		iniparser_setint(ini,"resource:videoencnum",dvsr->videoencnum);
		gtloginfo("resource:videoencnum %d->%d",num,dvsr->videoencnum);
		change_flag=1;
	}

	//是否有存储设备
	num=iniparser_getint(ini,"resource:ide",-1);
	if(num!=dvsr->ide)
	{
		iniparser_setint(ini,"resource:ide",dvsr->ide);
		gtloginfo("resource:ide %d->%d",num,dvsr->ide);
		change_flag=1;
	}
/*yk del 冗余代码*/
#if 0
	//是否有存储设备
	num=iniparser_getint(ini,"resource:ide",-1);
	if(num!=dvsr->ide)
	{
		iniparser_setint(ini,"resource:ide",dvsr->ide);
		gtloginfo("resource:ide %d->%d",num,dvsr->ide);
		change_flag=1;
	}
#endif
#if 0
	//
	if(dvsr->ide==0)
	{
		num=iniparser_getint(ini,"resource:disk_capacity",-1);
		if(num!=0)
		{
			iniparser_setint(ini,"resource:disk_capacity",0);
			gtloginfo("resource:disk_capacity %d->%d",num,0);
			change_flag=1;
		}
	}
	
#endif
	
	//网口数
	num=iniparser_getint(ini,"resource:eth_port",-1);
	if(num!=dvsr->eth_port)
	{
		iniparser_setint(ini,"resource:eth_port",dvsr->eth_port);
		gtloginfo("resource:eth_port %d->%d",num,dvsr->eth_port);
		change_flag=1;
	}


	return change_flag;
	
}
/**********************************************************************************************
 * 函数名       :get_devinfo_version
 * 功能 :	获取devinfo库的版本信息
 * 输入 :无
 * 返回值       :秒数devinfo库版本信息的字符串指针
  **********************************************************************************************/
const char *get_devinfo_version(void)
{
	return version;
}
/**********************************************************************************************
 * 函数名	:init_devinfo()
 * 功能	:初始化设备信息
 * 输入	:无
 * 返回值	:0表示正常，负值表示出错
 * 注		:应用程序在刚启动的时候需要调用这个函数从/conf/devinfo.ini中读取系统信息
 *			如果/conf/devinfo.ini不存在，则把设备信息设置成初始值，并返回-1
  **********************************************************************************************/
int init_devinfo(void)
{
	dictionary      *ini=NULL;
    	char *pstr=NULL;
    	int status;
	struct GT_GUID guid;
	int num;
//	struct tm *ptime=NULL;
//	time_t ctime;
	FILE *fp=NULL;
	int  write_file_flag=0;
	
	
	if(init_flag)
		return 0;
	init_flag=1;
	memset((void*)&info,0,sizeof(devinfo_struct));
	ini=iniparser_load_lockfile(DEVINFO_PARA_FILE,1,&fp);
	if(ini==NULL)
	{
		
                printf("init_devinfo() cannot parse ini file file [%s]", DEVINFO_PARA_FILE);
                gtlogerr("init_devinfo() cannot parse ini file file [%s]", DEVINFO_PARA_FILE);
                return -1 ;
        }
       // iniparser_dump_ini(ini,stdout); //将ini文件内容显示在屏幕上，实际使用时没有用，应去掉
	pstr=iniparser_getstring(ini,"devinfo:devguid",def_dev_guid);
	guid=hex2guid(pstr);
	memcpy((char*)info.guid,(char*)(&guid),sizeof(guid));
	guid2hex(guid,info.guid_str);
	
	//设备型号字符串
	pstr=iniparser_getstring(ini,"devinfo:devtypestring",T_GTIP2004_STR);
	info.dvsr=get_dvsr_by_typestr(pstr);
	if(info.dvsr==NULL)
		info.dvsr=get_dvsr_by_typestr(T_GTIP2004_STR);//默认为IP2004

	//设备型号代码
	num=iniparser_getint(ini,"devinfo:devtype",-1);
	if(num!=conv_dev_str2type(pstr))	//pstr存放型号字符串
	{
		iniparser_setint(ini,"devinfo:devtype",conv_dev_str2type(pstr));
		write_file_flag=1;
	}
	

	if(fix_devinfo_file(ini,info.dvsr)==1)
		write_file_flag=1;

	pstr=iniparser_getstring(ini,"devinfo:batchseq","NULL");
	sprintf(info.batch_seq,"%s",pstr);

	pstr=iniparser_getstring(ini,"devinfo:cpuboard","NULL");
	sprintf(info.board_seq,"%s",pstr);
	//	ctime=time(NULL); //	ptime=localtime(&ctime);	//	memcpy((void*)&info.lv_fac_time,(void *)ptime,sizeof(info.lv_fac_time));


	info.lv_fac_time.tm_year=iniparser_getint(ini,"leave_fac:year",2000)-1900;
	info.lv_fac_time.tm_mon=iniparser_getint(ini,"leave_fac:mon",1)-1;
	info.lv_fac_time.tm_mday=iniparser_getint(ini,"leave_fac:day",1);
	info.lv_fac_time.tm_hour=iniparser_getint(ini,"leave_fac:hour",0);
	info.lv_fac_time.tm_min=iniparser_getint(ini,"leave_fac:min",0);
	info.lv_fac_time.tm_sec=iniparser_getint(ini,"leave_fac:sec",0);	
	info.lfc_flag=iniparser_getint(ini,"leave_fac:lfc_flag",-1);

	num=iniparser_getint(ini,"resource:disk_capacity",-1);
	info.disk_capacity=num;
	if(info.disk_capacity<0)
	{
		iniparser_setstr(ini,"resource:disk_capacity","0");
		write_file_flag=1;		
	}	

	if(write_file_flag)
		save_inidict_file(DEVINFO_PARA_FILE,ini,&fp);
	else
	{
		if(fp!=NULL)
		{
			unlock_file(fileno(fp));
			fsync(fileno(fp));
			fclose(fp);
		}	
	}

	iniparser_freedict(ini);
	return 0;
}

/**********************************************************************************************
 * 函数名	:get_devid()
 * 功能	:应用程序调用获得设备的dev_id(二进制应该已经调用过了init_devinfo())
 * 输入	:无
 * 输出 	:buf:应用程序需要存放devid的缓冲区地址,返回时填充buf的长度必须足够长大于DEV_GUID_BYTE
 * 返回值	:正值表示填充到buf中的有效字节数，负值表示出错
  **********************************************************************************************/
int get_devid(unsigned char *buf)
{
    if(buf==NULL)
	    return -1;
    memcpy(buf,info.guid,DEV_GUID_BYTE);
    return DEV_GUID_BYTE;
}

/**********************************************************************************************
 * 函数名	:get_devid_str()
 * 功能	:应用程序调用获得设备的guid的字符串
 * 输入	:无
 * 返回值	:指向描述guid信息的字符串指针
  **********************************************************************************************/
char* get_devid_str(void)
{
	return info.guid_str;
}

/**********************************************************************************************
 * 函数名	:get_lfc_flag()
 * 功能	:应用程序调用获得设备出厂标志
 * 输入	:无
 * 返回值	:1=出厂，0=在家, -1出错
  **********************************************************************************************/
int get_lfc_flag(void)
{
	return info.lfc_flag;
}


/**********************************************************************************************
 * 函数名	:set_devid_str()
 * 功能	:应用程序调用设置设备的guid值,字符串格式
 * 输入	:id_str:字符串表示的设备guid值
 * 输出 	:
 * 返回值	:0表示成功 负值表示出错 -EINVAL表示输入参数格式不对
  **********************************************************************************************/
int set_devid_str(char *id_str)
{

	char				guid_temp[DEV_GUID_BYTE*4]; 
	struct GT_GUID 	guid;
	dictionary      		*ini=NULL;
	char 			*pstr=NULL;
	FILE 			*fp=NULL;
	
	if(id_str==NULL)
		return -EINVAL;
	guid=hex2guid(id_str);
	guid2hex(guid,guid_temp);
	if(strncasecmp(guid_temp,id_str,DEV_GUID_BYTE*2)!=0)
		return -EINVAL;	//GUID值非法
	ini=iniparser_load_lockfile(DEVINFO_PARA_FILE,1,&fp);
	if(ini==NULL)
	{
                printf("init_devinfo() cannot parse ini file file [%s]", DEVINFO_PARA_FILE);
                return -ENOENT ;
        }
	memcpy((void*)info.guid,(void*)&guid,DEV_GUID_BYTE);
	sprintf(info.guid_str,"%s",guid_temp);
	
	pstr=iniparser_getstring(ini,"devinfo:devguid","NULL");
	gtloginfo("devinfo:devguid %s->%s\n",pstr,guid_temp);
	iniparser_setstr(ini,"devinfo:devguid",guid_temp);	
	save_inidict_file(DEVINFO_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);
	return 0;	
}
/**********************************************************************************************
 * 函数名	:set_devid()
 * 功能	:应用程序调用设置设备的guid值,二进制格式
 * 输入	:buf:二进制描述的guid值，长度为DEV_GUID_BYTE字节
 * 输出 	:
 * 返回值	:0表示成功 负值表示出错 -EINVAL表示输入参数格式不对
  **********************************************************************************************/
int set_devid(unsigned char *buf)
{
	struct GT_GUID guid;
	char id_str[DEV_GUID_BYTE*4];
	if(buf==NULL)
		return -EINVAL;
	memcpy((void*)&guid,buf,DEV_GUID_BYTE);
	guid2hex(guid,id_str);
	return set_devid_str(id_str);
}

/**********************************************************************************************
 * 函数名	:get_hd_capacity()
 * 功能	:获取设备的磁盘容量信息(以MB为单位)
 * 输入	:无
 * 返回值	:正值表示设备的磁盘容量，负值表示出错
 **********************************************************************************************/
int get_hd_capacity(void)
{
	return info.disk_capacity;
}

/**********************************************************************************************
 * 函数名	:get_hd_type()
 * 功能	:获取磁盘类型
 * 输入	:无
 * 返回值	:0-CF,1-HD,-1-none
 **********************************************************************************************/
int get_hd_type(void)//0-CF,1-HD,-1-none
{
	if(info.disk_capacity<200)
		return -1;
	if(info.disk_capacity>20*1024)
		return 1;
	return 0;
}


/**********************************************************************************************
 * 函数名	:get_hd_nodename()
 * 功能	:获取磁盘节点名称
 * 输入	:diskno, 磁盘编号，0，1，2。。
 * 返回值	:节点名称，NULL表示失败
 **********************************************************************************************/
char * get_hd_nodename(int diskno)
{
	switch(diskno)
	{
		case 0:	return "/dev/sda"; 
		case 1:	return "/dev/sdb";
		case 2: return "/dev/sdc";
		case 3: return "/dev/sdd";
		default:	return NULL;
	}
}







/**********************************************************************************************
 * 函数名	:set_hd_capacity()
 * 功能	:设置设备的磁盘容量值
 * 输入	:value:程序检测到的磁盘容量值MB为单位
 * 返回值	:0表示成功，负值表示出错
 **********************************************************************************************/
int set_hd_capacity(int value)
{
	dictionary      *ini;
	FILE *fp=NULL;
	ini=iniparser_load_lockfile(DEVINFO_PARA_FILE,1,&fp);
    	if (ini==NULL) {
            printf("init_devinfo() cannot parse ini file file [%s]", DEVINFO_PARA_FILE);
            return -1 ;
    	}
    	if(info.disk_capacity==value)
    	{
		iniparser_freedict(ini);
		if(fp!=NULL)
		{
                        unlock_file(fileno(fp));
                        fsync(fileno(fp));
                        fclose(fp);
		}
    		return 0;
    	}
	info.disk_capacity=value;
	//gtloginfo("test!!!!!!!!设置为%d\n",value);
	iniparser_setint(ini,"resource:disk_capacity",value);
	save_inidict_file(DEVINFO_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);
	return 0;
}

/**********************************************************************************************
 * 函数名	:set_devtype_str()
 * 功能	:设置设备的型号字符串
 * 输入	:type_str:描述设备型号的字符串
 * 返回值	:0表示成功，负值表示失败,-EINVAL表示参数是不支持的类型
 **********************************************************************************************/
int set_devtype_str(char *type)
{
	GTSeriesDVSR 	*dvsr=NULL;
	dictionary      		*ini=NULL;
	FILE				*fp=NULL;
	char				*ptr=NULL;
	if(type==NULL)
		return -EINVAL;
	dvsr=get_dvsr_by_typestr(type);
	if(dvsr==NULL)
		return -EINVAL;
	ini=iniparser_load_lockfile(DEVINFO_PARA_FILE,1,&fp);
	if(ini==NULL)
	{
                printf("init_devinfo() cannot parse ini file file [%s]", DEVINFO_PARA_FILE);
                return -ENOENT ;
        }

	ptr=iniparser_getstring(ini,"devinfo:devtypestring","NULL");
	gtloginfo("devinfo:devtypestring %s->%s",ptr,type);
	iniparser_setstr(ini,"devinfo:devtypestring",type);

	
	ptr=iniparser_getstring(ini,"devinfo:devtype","-1");
	gtloginfo("devinfo:devtypestring %d->%d",atoi(ptr),conv_dev_str2type(type));
	iniparser_setint(ini,"devinfo:devtype",conv_dev_str2type(type));
	save_inidict_file(DEVINFO_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);
	return 0;	
}
/**********************************************************************************************
 * 函数名	:get_devtypestr()
 * 功能	:应用程序调用获得描述设备类型的字符串
 * 输入	:无
 * 返回值	:描述设备类型字符串的指针
 **********************************************************************************************/
char * get_devtype_str(void)
{
	return conv_dev_type2str(info.dvsr->type);
}
/**********************************************************************************************
 * 函数名	:get_devtype()
 * 功能	:应用程序调用获得设备类型代码
 * 输入	:无
 * 返回值	:设备类型代码
 **********************************************************************************************/
int 	get_devtype(void)
{
	return info.dvsr->type;
}

/**********************************************************************************************
 * 函数名	:get_batch_seq()
 * 功能	:获取生产批次字符串
 * 输入	:无
 * 返回值	:指向生产批次字符串的指针
 **********************************************************************************************/
char *get_batch_seq(void)
{
	return info.batch_seq;
}

/**********************************************************************************************
 * 函数名	:get_board_seq()
 * 功能	:获取板卡批次字符串
 * 输入	:无
 * 返回值	:指向板卡批次字符串的指针
 **********************************************************************************************/
char *get_board_seq(void)
{
	return info.board_seq;
}

/**********************************************************************************************
 * 函数名	:set_batch_seq()
 * 功能	:设置生产批次字符串
 * 输入	:生产批次信息
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/
int set_batch_seq(char *seq)
{
	int len;
	dictionary      		*ini=NULL;
	FILE				*fp=NULL;
	char				*pstr=NULL;
	len=strlen(seq);
	if((len+1)>sizeof(info.batch_seq))
		len=sizeof(info.batch_seq)-1;
	memcpy(info.batch_seq,seq,len+1);
	info.batch_seq[len]='\0';
	ini=iniparser_load_lockfile(DEVINFO_PARA_FILE,1,&fp);
	if(ini==NULL)
	{
                printf("init_devinfo() cannot parse ini file file [%s]", DEVINFO_PARA_FILE);
                return -ENOENT ;
        }
	pstr=iniparser_getstring(ini,"devinfo:batchseq","NULL");
	gtloginfo("devinfo:batchseq %s->%s",pstr,info.batch_seq);
	iniparser_setstr(ini,"devinfo:batchseq",info.batch_seq);

	save_inidict_file(DEVINFO_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);
	return 0;
	
}

/**********************************************************************************************
 * 函数名	:set_board_seq()
 * 功能	:设置板卡批次字符串
 * 输入	:板卡批次信息
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/
int set_board_seq(char *seq)
{
	int len;
	dictionary      		*ini=NULL;
	FILE				*fp=NULL;
	char				*pstr=NULL;
	len=strlen(seq);
	if((len+1)>sizeof(info.board_seq))
		len=sizeof(info.board_seq)-1;
	memcpy(info.board_seq,seq,len+1);
	info.board_seq[len]='\0';
	ini=iniparser_load_lockfile(DEVINFO_PARA_FILE,1,&fp);
	if(ini==NULL)
	{
                printf("init_devinfo() cannot parse ini file file [%s]", DEVINFO_PARA_FILE);
                return -ENOENT ;
        }
	pstr=iniparser_getstring(ini,"devinfo:cpuboard","NULL");
	gtloginfo("devinfo:cpuboard %s->%s",pstr,info.board_seq);
	iniparser_setstr(ini,"devinfo:cpuboard",info.board_seq);

	save_inidict_file(DEVINFO_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);
	return 0;
}
/**********************************************************************************************
 * 函数名	:get_lvfac_time()
 * 功能	:获取设备的出厂时间
 * 输入	:无
 * 返回值	:设备出厂时间的结构指针
 **********************************************************************************************/
struct tm * get_lvfac_time(void)
{
	return &info.lv_fac_time;
}

/**********************************************************************************************
 * 函数名	:get_lvfac_time()
 * 功能	:获取设备的出厂时间
 * 输入	:无
 * 返回值	:0表示正确 负值表示出错  -EINVAL表示 参数错误
 **********************************************************************************************/
int  set_lvfac_time(struct tm *lv)
{
	time_t 			seconds;
	struct tm 		*t=NULL;
	char 			oldtime[100];
	char 			newtime[100];
	dictionary      		*ini=NULL;
	FILE				*fp=NULL;
	if(lv==NULL)
		return -EINVAL;
	seconds=mktime(lv);
	if((int)seconds<0)
		return -EINVAL;

	ini=iniparser_load_lockfile(DEVINFO_PARA_FILE,1,&fp);
	if(ini==NULL)
	{
                printf("init_devinfo() cannot parse ini file file [%s]", DEVINFO_PARA_FILE);
                return -ENOENT ;
        }
	



	
	t=lv;
	sprintf(newtime,"%04d-%02d-%02d %02d:%02d:%02d",t->tm_year+1900,
											t->tm_mon+1,
											t->tm_mday,
											t->tm_hour,
											t->tm_min,
											t->tm_sec);
	t=&info.lv_fac_time;
	sprintf(oldtime,"%04d-%02d-%02d %02d:%02d:%02d",t->tm_year+1900,
											t->tm_mon+1,
											t->tm_mday,
											t->tm_hour,
											t->tm_min,
											t->tm_sec);

	iniparser_setint(ini,"leave_fac:year",lv->tm_year+1900);
	iniparser_setint(ini,"leave_fac:mon",lv->tm_mon+1);
	iniparser_setint(ini,"leave_fac:day",lv->tm_mday);
	iniparser_setint(ini,"leave_fac:hour",lv->tm_hour);
	iniparser_setint(ini,"leave_fac:min",lv->tm_min);
	iniparser_setint(ini,"leave_fac:sec",lv->tm_sec);	
	iniparser_setint(ini,"leave_fac:lfc_flag",1);
	gtloginfo("leave_fac %s->%s\n",oldtime,newtime);
	
	save_inidict_file(DEVINFO_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);
	memcpy((void*)&info.lv_fac_time,lv,sizeof(struct tm ));
	return 0;
}


/**********************************************************************************************
 * 函数名	:get_total_com()
 * 功能	:获取设备的总串口数
 * 输入	:无
 * 返回值	:正值表示设备的总串口数,负值表示出错
 **********************************************************************************************/
int get_total_com(void)
{
	return info.dvsr->com;
}

/**********************************************************************************************
 * 函数名	:get_video_num()
 * 功能	:获取设备可接入的视频总数
 * 输入	:无
 * 返回值	:可接入的视频数
 **********************************************************************************************/
int get_video_num(void)
{
	return info.dvsr->videonum;
}


/**********************************************************************************************
 * 函数名	:get_videoenc_num()
 * 功能	:获取视频编码器数
 * 输入	:无
 * 返回值	:视频编码器数
 **********************************************************************************************/
int get_videoenc_num(void)
{
	return info.dvsr->videoencnum;
}

/**********************************************************************************************
 * 函数名	:get_ide_flag()
 * 功能	:获取设备是否有硬盘标志
 * 输入	:无
 * 返回值	:1表示有硬盘 或CF卡 2表示有SD卡 0表示没有
 **********************************************************************************************/
int	get_ide_flag(void)
{
	return info.dvsr->ide;
}

/**********************************************************************************************
 * 函数名	:get_trigin_num()
 * 功能	:获取输入端子数
 * 输入	:无
 * 返回值	:设备上的端子总数
 **********************************************************************************************/
int	get_trigin_num(void)
{
	return info.dvsr->trignum;
}


/**********************************************************************************************
 * 函数名	:get_alarmout_num()
 * 功能	:获取输出端子数
 * 输入	:无
 * 返回值	:设备上的输出端子数
 **********************************************************************************************/
int get_alarmout_num(void)
{
	return info.dvsr->outnum;
}


/**********************************************************************************************
 * 函数名	:get_eth_num()
 * 功能	:获取网口数
 * 输入	:无
 * 返回值	:设备上的网口数
 **********************************************************************************************/
int get_eth_num(void)
{
	return info.dvsr->eth_port;
}


/**********************************************************************************************
 * 函数名	:get_disk_no()
 * 功能	:获取磁盘数目
 * 返回值	:磁盘数目
 **********************************************************************************************/
int get_disk_no(void)
{
	return info.dvsr->ide; 
}



/**********************************************************************************************
 * 函数名	:get_quad_flag()
 * 功能	:获取画面分割器是否有效标志
 * 输入	:无
 * 返回值	:1表示有画面分割器 0表示没有
 **********************************************************************************************/
int get_quad_flag(void)
{
	return info.dvsr->quad;
}

/**********************************************************************************************
 * 函数名	:get_osd_flag()
 * 功能	:获取是否有osd标志
 * 输入	:无
 * 返回值	:1表示有0表示没有
 **********************************************************************************************/
int get_osd_flag(void)
{
	return info.dvsr->osd;
}

/**********************************************************************************************
 * 函数名	:get_cpu_iic_flag()
 * 功能	:获取是否使用cpu的iic总线控制视频ad转换芯片
 * 输入	:无
 * 返回值	:1表示使用0表示不使用
 **********************************************************************************************/
int get_cpu_iic_flag(void)
{
	return info.dvsr->use_cpu_iic;
}


/**********************************************************************************************
 * 函数名	:get_audio_num()
 * 功能	:获取音频通道个数
 * 输入	:无
 * 返回 :音频通道个数
 **********************************************************************************************/
int get_audio_num(void)
{
	return info.dvsr->audionum;
}

/***********************************************************************************************
 * 函数名:get_prog_ver
 * 功能  :获取设备软件版本号字符串
 * 输入  :无
 * 返回  :描述设备软件版本号的字符串指针
 **********************************************************************************************/
char *get_prog_ver(void)
{
	char str1[20];
	char str2[20];
	char str3[20];

	char *pstr=NULL;
	pstr=conv_dev_type2str(info.dvsr->type);
	if(strstr(pstr,"GTVM")!=NULL)
	{//GTVM
		get_prog_version(str1,"/lock/vserver/vmmain");
                get_prog_version(str2,"/lock/vserver/vm_tcprtimg");
		sprintf(info.prog_ver_str,"vm-%s-%s",str1,str2);
	}
	else 
	{//GTVS
		get_prog_version(str1,IPMAIN_LOCK_FILE);
		get_prog_version(str2,RT_LOCK_FILE);
		get_prog_version(str3,HDMOD_LOCK_FILE);
		sprintf(info.prog_ver_str,"%s-%s-%s",str1,str2,str3);
	}
	return info.prog_ver_str;
}
/***********************************************************************************************
 * 函数名:get_dev_family
 * 功能  :获取设备属于哪个系列的产品
 * 输入  :无
 * 返回  :产品系列代码 GTDEV_FAMILY_GTVS GTDEV_FAMILY_GTVM
 **********************************************************************************************/
///GT硬件产品系列定义
#define GTDEV_FAMILY_GTVS	0	///GTVS系列视频服务器
#define GTDEV_FAMILY_GTVM	1	///GTVM系列虚拟服务器

int get_dev_family(void)
{
	char *pstr=NULL;
        pstr=conv_dev_type2str(info.dvsr->type);
        if(strstr(pstr,"GTVM")!=NULL)
        {//GTVM
		return GTDEV_FAMILY_GTVM;
	}
	else
	{
		return GTDEV_FAMILY_GTVS;
	}
}

