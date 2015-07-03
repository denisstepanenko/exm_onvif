/*
 * 使配置文件增加新的节
 */
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <iniparser.h>
#include <file_def.h>
#include <commonlib.h>
#include <errno.h>
#include "converts.h"
#include <devinfo.h>
#include <confparser.h>
#define PARA_BAK_FILE		"/conf/ip1004_bak.ini"
#define	DEVINFO_PARA_FILE_BAK	"/conf/devinfo_bak.ini"
#define CONFIG_FILE		"/conf/config"


//在vm_tools的Makefile中定义 #define USE_VM			//wsy,使用gtvm时定义

#define VERSION "0.62"
// 0.62 加入对max_wan_usr 默认配置
// 0.61 加入对/conf/devinfo文件丢失后的恢复
// 0.60 加入对/conf/config文件丢失后的恢复
// 0.59 加入对upnp和dhcp变量的增加
// 0.58 加入对GTVM的支持
// 0.57 加入fix eth1 mac地址的功能
// 0.56 加入报警提示音的变量actiondefine:40
// 0.55 加入将leave_fac中分散的时间转换为一个字符串
// 0.54 加入对/conf/config文件中的eth_port变量的支持,解决了配置文件和备份文件比较时信息记录不准确的问题
// 0.53 加入从备份文件中恢复被破坏的配置文件的功能
// 0.52 加入对setalarm默认值和设备程序默认值不匹配的问题的支持
// 0.51 加入对报警抓图配置文件的支持
// 0.5 加入了对配置文件中报警部分使能的恢复
// 0.4 修改了mac_conv的实现，使其支持单独修改mac地址
// 0.3 将察看配置文件节的功能，增加对devinfo.ini文件节的判断
// 0.2 加入如果配置文件中没有报警属性变量则根据alarm:trig_inl来生成报警属性
typedef struct{
	//中需要检查，如果没有则需要增加的变量结构
	char *key;	//变量名
	char *defval;	//变量值的字符串形式
}KeyStruct;
static KeyStruct IP1004NeedKeys[]=
{//ip1004.ini 
	{"actiondefine:40",		"报警提示音"		},
	{"netencoder:max_wan_usr",      "8"}
};

static  char *IP1004NeedSection[]={
//这些节没有则会增加节
	"alarmversion",
	"actiondefine",
	"alarm0",
	"alarm1",
	"alarm2",
	"alarm3",
	"alarm4",
	"alarm5",
	"motion0",
	"motion1",
	"motion2",
	"motion3",
	"a_user_def"
};
static char *DevinfoNeedSection[]={
//这些节没有则会增加相应的节
	"devinfo",	
	"resource",
	"leave_fac"
};

static char *restore_section[]={
//这些节没有则会从备份文件中恢复
	"install",
	"port",
	"remote_gate",
	"netencoder",
	"video0"	
};

///devinfo.ini中如果没有这些变量则会从备份文件中恢复
static char *restore_devinfo_section[]={
	"devinfo",
	"leave_fac",
	"resource",
};
static char *restore_conf_var[]={
///conf/config 文件如果没有这些变量则会调用para_conv -s恢复
	"MAC_ADDRESS",
	"ETH_PORT",
	"ETH0_IPADDR",
	"ETH0_NETMASK",
	"LOGIN_PORT"
};
static int NeedSectionFlag[100];//存放是否需要增加节的标志
int ProcessAlarmSec(dictionary      *ini)
{
	int i;
	//int Ret;
	int Cnt=0;
	int Val;
	unsigned long temp;
	char *pstr=NULL;
	char Section[256];
	int AlarmAttrib[8];//报警输入属性
	int AlarmEnable[8];//报警使能属性
	int BadFlag=0;
	pstr=iniparser_getstring(ini,"alarm:trig_in",NULL);
	if(pstr==NULL)
	{
		temp=0;
	}
	else
		temp=atohex(pstr);	
	for(i=0;i<8;i++)
	{
		if(temp&(1<<i))
			AlarmAttrib[i]=1;
		else
			AlarmAttrib[i]=0;
	}
	for(i=0;i<6;i++)
	{//alarm0~alarm5
		sprintf(Section,"alarm%d:attrib",i);
		Val=iniparser_getint(ini,Section,-100);
		if((Val!=0)&&(Val!=1))
		{
			//配置文件中原来没有该变量
			iniparser_setint(ini,Section,AlarmAttrib[i]);
			gtloginfo("restore %s %d->%d\n",Section,Val,AlarmAttrib[i]);
			printf("restore %s %d->%d\n",Section,Val,AlarmAttrib[i]);
			Cnt++;
			BadFlag=1;	//由于setalarm设置软件的默认值(使能标志的默认值为0)，所以通过这个来判断配置文件的
					//使能位是否需要修改
		}
		
	}

	for(i=0;i<8;i++)
		AlarmEnable[i]=1;	//报警是重要的事情,如果读不到则默认值为允许报警
        for(i=0;i<6;i++)
        {//alarm0~alarm5
                sprintf(Section,"alarm%d:enable",i);
                Val=iniparser_getint(ini,Section,-100);
        	if(((Val!=0)&&(Val!=1))||(BadFlag))
                {
                        //配置文件中原来没有该变量
                        iniparser_setint(ini,Section,AlarmEnable[i]);
			gtloginfo("restore %s %d->%d\n",Section,Val,AlarmEnable[i]);
			printf("restore %s %d->%d\n",Section,Val,AlarmEnable[i]);
                        Cnt++;
                }

        }	
       for(i=0;i<6;i++)
        {//alarm0~alarm5
	//报警是重要的事情,默认为布防
                sprintf(Section,"alarm%d:setalarm",i);
                Val=iniparser_getint(ini,Section,-100);
                if(((Val!=0)&&(Val!=1))||(BadFlag))
                {
                        //配置文件中原来没有该变量
                        iniparser_setint(ini,Section,AlarmEnable[i]);
                        gtloginfo("restore %s %d->%d\n",Section,Val,1);
                        printf("restore %s %d->%d\n",Section,Val,1);
                        Cnt++;
                }

        }


	
	return Cnt;
}
int FixVideoEncPara(dictionary *ini)
{///将视频编码器参数中的变码流去掉
	int total;
	int i;
	int change_cnt=0;
	char key[200];
	int num;
	total=get_videoenc_num();
	for(i=0;i<total;i++)
	{	
		sprintf(key,"%s:bitratecon",get_video_enc_ini_sec(i));
		num=iniparser_getint(ini,key,-1);
		if(num==0)
		{//变码流
			printf("fix enc%d bitratecon vbt->hbr\n",i);
			gtloginfo("fix enc%d bitratecon vbr->hbr\n",i);
			printf("set enc%d min=%d max=%d\n",i,256,2000);
			gtloginfo("set enc%d min=%d max=%d\n",i,256,2000);
			iniparser_setint(ini,key,2);	///设置成混合码流
			sprintf(key,"%s:minbitrate",get_video_enc_ini_sec(i));
			iniparser_setint(ini,key,256);
			sprintf(key,"%s:maxbitrate",get_video_enc_ini_sec(i));
			iniparser_setint(ini,key,2000);
			change_cnt++;
		}
	}
	return change_cnt;
}
int CheckAndAddKeys(char *FileName,KeyStruct *Keys,int Total)
{
	int SaveFlag=0;
	int i,ret;
	dictionary *ini=NULL;
	FILE	*fp=NULL;
	KeyStruct *k=NULL;
	printf("%s need %d keys\n",FileName,Total);
	ini=iniparser_load_lockfile(FileName,0,&fp);
        if(ini==NULL)
        {
                printf("can't load file %s!!\n",FileName);
                return -errno;
        }
	for(i=0;i<Total;i++)
	{
		k=&Keys[i];
		ret=iniparser_find_entry(ini,k->key);
                if(ret!=1)
                {
                        printf("can't find key:%s\n",k->key);
			iniparser_setstr(ini,k->key,k->defval);
			printf("add key [%s]=%s to %s\n",k->key,k->defval,FileName);
        		gtloginfo("add key [%s]=%s to %s\n",k->key,k->defval,FileName);
	                SaveFlag++;
                }
	}
	if(SaveFlag>0)
	{
		save_inidict_file(FileName,ini,&fp);
		fp=NULL;
	}
	if(fp!=NULL)
		fclose(fp);
	iniparser_freedict(ini);	
	return SaveFlag;
}
int CheckAndAddSection(char *FileName,char *Sections[],int Total)
{
	int		ret;
	int 		i;
	int		LostSecCnt=0;
	char 		*Sec;
	dictionary      *ini;
	FILE 		*fp=NULL;
	int         lock;
	
	printf("%s need %d sections\n",FileName,Total);
	ini=iniparser_load(FileName);
	if(ini==NULL)
	{
		printf("can't load file %s!!\n",FileName);
		return -errno;
	}
	for(i=0;i<Total;i++)
	{
		Sec=Sections[i];
		ret=iniparser_find_entry(ini,Sec);
		if(ret!=1)
		{
			printf("can't find entry:%s\n",Sec);
			LostSecCnt++;
			NeedSectionFlag[i]=1;
		}
		else
		{
			NeedSectionFlag[i]=0;
		}
	}
	iniparser_freedict(ini);
	
	if(LostSecCnt!=0)
	{	
		fp=fopen(FileName,"a");
		if(fp==NULL)
		{
			printf("can't create %s!!\n",FileName);
			return -errno;
		}
		lock=lock_file(fileno(fp),0);
		if(lock<0)
		{
			gtloginfo("iniparser_load_lockfile lock=%d(%s)!!!!\n",lock,strerror(errno));
			fclose(fp);
			return -1;
		}
		else
		{		
			fprintf(fp,"\n");
			for(i=0;i<Total;i++)
			{
				Sec=Sections[i];
				if(NeedSectionFlag[i])
				{
					fprintf(fp,"[%s]\n",Sec);
					gtloginfo("add section[%s] to %s\n",Sec,FileName);
				}
			}
		}
		unlock_file(fileno(fp));
		fclose(fp);
	}		
	return 0;
	
}
//判断ip1004.ini是否被破坏,如果破坏则进行恢复
static int restore_conf_file(void)
{       
        confdict 	*conf=NULL;
        int total=sizeof(restore_conf_var)/sizeof(char*);
        int i,ret,lost_cnt=0;
	char *vstr=NULL;
        char buf[100];
        conf=confparser_load("/conf/config");
        if(conf==NULL)
        {
                printf("can't load file %s!!\n","/conf/config");
                gtlogerr("can't load file %s!!\n","/conf/config");
        	lost_cnt=100;       
        }
	else
	{
        
        	for(i=0;i<total;i++)
        	{
                	vstr=confparser_getstring(conf,restore_conf_var[i],NULL);
                	if(vstr==NULL)
                	{
                        	printf("can't find entry:%s\n",restore_conf_var[i]);
                        	gtlogerr("can't find entry:%s\n",restore_conf_var[i]);
                        	lost_cnt++;
                	}
        	}       
        	confparser_freedict(conf);
	}
        if(lost_cnt!=0)
        {
                sprintf(buf,"/ip1004/para_conv -s\n");
                ret=system(buf);
                printf("/conf/config被破坏,从/conf/ip1004.ini中恢复,ret=%d!\n",ret);
                gtlogwarn("/conf/config被破坏,从/conf/ip1004.ini中恢复,ret=%d!\n",ret);
        }
        return lost_cnt;
}

//判断gt1000.ini是否被破坏,如果破坏则进行恢复
static int restore_file(void)
{
	dictionary      *ini=NULL;
	int total=sizeof(restore_section)/sizeof(char*);
    	int i,ret,lost_cnt=0;
	char buf[100];
    	ini=iniparser_load(IPMAIN_PARA_FILE);
        if(ini==NULL)
        {
                printf("can't load file %s!!\n",IPMAIN_PARA_FILE);
                gtlogerr("can't load file %s!!\n",IPMAIN_PARA_FILE);
		lost_cnt=100;
        }
	else
	{
	    for(i=0;i<total;i++)
	    {
                ret=iniparser_find_entry(ini,restore_section[i]);
                if(ret!=1)
                {
                        printf("can't find entry:%s\n",restore_section[i]);
			gtlogerr("can't find entry:%s\n",restore_section[i]);
                        lost_cnt++;
                }
	    }
		iniparser_freedict(ini);
	}
	if(lost_cnt!=0)
	{
		ret=ini_diff(PARA_BAK_FILE,IPMAIN_PARA_FILE);
		sprintf(buf,"cp -f %s %s\n",PARA_BAK_FILE,IPMAIN_PARA_FILE);
		ret=system(buf);
		printf("%s被破坏,从%s中恢复,ret=%d!\n",IPMAIN_PARA_FILE,PARA_BAK_FILE,ret);
		gtlogerr("%s被破坏,从%s中恢复,ret=%d!\n",IPMAIN_PARA_FILE,PARA_BAK_FILE,ret);
	}
	return lost_cnt;
}

//判断devinfo.ini是否被破坏
static int restore_devinfo_file(void)
{
        dictionary      *ini=NULL;
        int total=sizeof(restore_devinfo_section)/sizeof(char*);
        int i,ret,lost_cnt=0;
        char buf[100];
        ini=iniparser_load(DEVINFO_PARA_FILE);
        if(ini==NULL)
        {
                printf("can't load file %s!!\n",DEVINFO_PARA_FILE);
                gtlogerr("can't load file %s!!\n",DEVINFO_PARA_FILE);
                lost_cnt=100;
        }
        else
        {
            for(i=0;i<total;i++)
            {
                ret=iniparser_find_entry(ini,restore_devinfo_section[i]);
                if(ret!=1)
                {
                        printf("can't find entry:%s\n",restore_devinfo_section[i]);
                        gtlogerr("can't find entry:%s\n",restore_devinfo_section[i]);
                        lost_cnt++;
                }
            }
                iniparser_freedict(ini);
        }
        if(lost_cnt!=0)
        {
                ret=ini_diff(DEVINFO_PARA_FILE_BAK,DEVINFO_PARA_FILE);
                sprintf(buf,"cp -f %s %s\n",DEVINFO_PARA_FILE_BAK,DEVINFO_PARA_FILE);
                ret=system(buf);
                printf("%s被破坏,从%s中恢复,ret=%d!\n",DEVINFO_PARA_FILE,DEVINFO_PARA_FILE_BAK,ret);
                gtlogerr("%s被破坏,从%s中恢复,ret=%d!\n",DEVINFO_PARA_FILE,DEVINFO_PARA_FILE_BAK,ret);
        }
        return lost_cnt;
}
int fix_config_file(void)
{//查看config文件中的内容,并设置相应的值
	confdict *conf=NULL;
	FILE *fp=NULL;
	int	num;
	char	buf[20];
	int	needwrite=0;
	conf=confparser_load_lockfile(CONFIG_FILE,1,&fp);
	if(conf==NULL)
	{
		printf("can't parse %s!!\n",CONFIG_FILE);
		gtlogerr("can't parse %s!!\n",CONFIG_FILE);
		return -1;
	}
	
	//网口数
	num=confparser_getint(conf,"ETH_PORT",-1);
	if(num!=get_eth_num())
	{
		sprintf(buf,"%d",get_eth_num());
		confparser_setstr(conf,"ETH_PORT",buf);
		printf("%s:%s %d->%d\n",CONFIG_FILE,"ETH_PORT",num,get_eth_num());
		gtloginfo("%s:%s %d->%d\n",CONFIG_FILE,"ETH_PORT",num,get_eth_num());
		needwrite++;		
	}
	num=confparser_getint(conf,"USE_DHCP",-1);
	if(num<0)
	{
		confparser_setstr(conf,"USE_DHCP","0");
		printf("%s:%s NONE->0\n",CONFIG_FILE,"USE_DHCP");
		gtloginfo("%s:%s NONE->0\n",CONFIG_FILE,"USE_DHCP");
		needwrite++;
	}
        num=confparser_getint(conf,"USE_UPNP",-1);
        if(num<0)
        {
                confparser_setstr(conf,"USE_UPNP","0");
                printf("%s:%s NONE->0\n",CONFIG_FILE,"USE_UPNP");
                gtloginfo("%s:%s NONE->0\n",CONFIG_FILE,"USE_UPNP");
                needwrite++;
        }


	if(needwrite!=0)
	{
		confparser_dump_conf(CONFIG_FILE,conf,fp);
	}
	confparser_freedict(conf);
	if(fp!=NULL)
		fclose(fp);
	return needwrite;
	
}
int fix_devinfo_file(void)
{
	dictionary      *ini=NULL;
	char		*date=NULL;
	char		date_buf[100];
	int 		year,mon,day,hour,min,sec;
	int		change_flag=0;
	ini=iniparser_load(DEVINFO_PARA_FILE);
	if(ini==NULL)
		return -ENOENT;
	//[leave_fac] 
	//将分散的时间结构转换为YYYY-MM-DD HH:MM:SS的字符串放到date变量中
	year=iniparser_getint(ini,"leave_fac:year",-1);
	mon=iniparser_getint(ini,"leave_fac:mon",-1);
	day=iniparser_getint(ini,"leave_fac:day",-1);
	hour=iniparser_getint(ini,"leave_fac:hour",0);
	min=iniparser_getint(ini,"leave_fac:min",0);
	sec=iniparser_getint(ini,"leave_fac:sec",0);
	if((year<0)||(mon<0)||(day<0))//配置文件中没有出厂日期信息
		return 0;
	sprintf(date_buf,"%04d-%02d-%02d %02d:%02d:%02d",year,mon,day,hour,min,sec);
	date=iniparser_getstring(ini,"leave_fac:date",NULL);
	if(date==NULL)
	{
		iniparser_setstr(ini,"leave_fac:date",date_buf);
		change_flag++;
	}
	else
	{
		if(strcmp(date_buf,date)!=0)
		{
			iniparser_setstr(ini,"leave_fac:date",date_buf);
                	change_flag++;
		}
	}
	/////

	if(change_flag>0)
		save_inidict_file(DEVINFO_PARA_FILE,ini,NULL);
	iniparser_freedict(ini);
	return 0;
}
int main(void)
{
	dictionary      *ini;
	int ret;

	int change_flag=0;

	char buf[100];
	gtopenlog("patchconf");
	printf("run patchconf version:%s\n",VERSION);
	gtloginfo("run patchconf version:%s\n",VERSION);
	restore_devinfo_file();
	fix_devinfo_file();
	CheckAndAddSection(DEVINFO_PARA_FILE,DevinfoNeedSection,sizeof(DevinfoNeedSection)/sizeof(char*));

	init_devinfo();
	ret=mac_check_conv(0);//监测mac地址是否合法，如果非法则改写为guid转化的数字
	if(get_eth_num()>1)
		ret=mac_check_conv(1);
	restore_file();//判断是否需要恢复IPMAIN_PARA_FILE
	CheckAndAddSection(IPMAIN_PARA_FILE,IP1004NeedSection,sizeof(IP1004NeedSection)/sizeof(char*));
	CheckAndAddKeys(IPMAIN_PARA_FILE,IP1004NeedKeys,sizeof(IP1004NeedKeys)/sizeof(KeyStruct));	
	ini=iniparser_load(IPMAIN_PARA_FILE);
	if(ini==NULL)
	{
		printf("can't load file %s!!\n",IPMAIN_PARA_FILE);
		exit(1);
	}
	if(ProcessAlarmSec(ini)>0)
	{//处理报警参数
		change_flag=1;
	}
	if(FixVideoEncPara(ini)>0)
	{//处理视频编码器参数
		change_flag=1;
	}	
	if(change_flag)
	{
		save_inidict_file(IPMAIN_PARA_FILE,ini,NULL);
	}	
	iniparser_freedict(ini);

//比较devinfo.ini文件和备份是否相同
        ret=ini_diff(DEVINFO_PARA_FILE_BAK,DEVINFO_PARA_FILE);
        if(ret!=0)

        {       //更新备份文件

                sprintf(buf,"cp -f %s %s\n",DEVINFO_PARA_FILE,DEVINFO_PARA_FILE_BAK);

                ret=system(buf);

                printf("更新备份配置文件%s,ret=%d!\n",DEVINFO_PARA_FILE_BAK,ret);

                gtloginfo("更新备份配置文件%s,ret=%d!\n",DEVINFO_PARA_FILE_BAK,ret);

        }



	//比较gt1000.ini文件和备份是否相同
	//sprintf(buf,"cmp %s %s\n",IPMAIN_PARA_FILE,PARA_BAK_FILE);
	//ret=system(buf);
	ret=ini_diff(PARA_BAK_FILE,IPMAIN_PARA_FILE);
	if(ret!=0)
	{	//更新备份文件
		sprintf(buf,"cp -f %s %s\n",IPMAIN_PARA_FILE,PARA_BAK_FILE);
                ret=system(buf);
                printf("更新备份配置文件%s,ret=%d!\n",PARA_BAK_FILE,ret);
                gtloginfo("更新备份配置文件%s,ret=%d!\n",PARA_BAK_FILE,ret);
	}
	restore_conf_file();
	fix_config_file();
	exit(0);
}

