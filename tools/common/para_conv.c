/*用于将参数配置文件进行多转一和一转多的工具，wsy august 2006*/
#include <stdio.h>
#include <stdlib.h>
#include "file_def.h"
#include "iniparser.h"
#include "confparser.h"
#include "commonlib.h"
#include <devinfo.h>

#define DHCP_FILE	"/etc/sysconfig/network-scripts/ifcfg-eth0"

#define CHAP_FILE 	"/etc/ppp/chap-secrets"	
#define PAP_FILE  	"/etc/ppp/pap-secrets"
#define PPP0_FILE 	"/etc/sysconfig/network-scripts/ifcfg-ppp0"
#define VSFTPD_FILE "/etc/vsftpd.conf"
#define BOA_FILE 	"/etc/boa/boa.conf"

#define CANT_FIND_KEY "no_find_key" //getstring找不到相应关键词时的default返回

#define VERSION "2.1"           //版本号
/*
//2.1 变更此工具为在ip1004下可使用，相应参数要改变
//2.0 不再转换devinfo.ini的resource:disk_capacity
//1.9 增加gt1000.ini的net_config:route_mac_bind和route_mac与config的映射
//1.8 增加devinfo:devtypestring->config:HOST_NAME
//1.7 执行-s时把gt1000.ini的devinfo:devtypestring设置到config文件的HOST_NAME	
//1.6 设置boa不再每设置一次多一行空行
//1.5 将PORT和port:telnet_port对应起来，而不是netconfig:telnet_port
//1.42 解决了ini_to_conf()函数在conf文件中没有制定key时会出段错误的问题,修改了一些显示字符串,修改了ini_to_conf函数中修改变量的实现

//1.41 网口信息改为单独的节[eth0][eth1]...
//1.4 纠正了get_boa_str返回字符串问题，及其导致的boa相关操作不正常的问题
//1.3 纠正了日志把一转多写成"多转一"的问题
//1.2 进行了如下改动
		1.日志记录尽量简洁，如：
			没有变量或变量被删除了以"NULL"表示,变化了以"->"表示
		如 devinfo:devtype NULL->1
		2.因为所有变量内容都存在于gt1000.ini中，所以日志记录只要记录gt1000.ini的变化就可以了;
			例如：
				执行para_conv -m 的时候只要记录leave_fac:mon 08->09
				执行para_conv -s 的时候只要记录net_config:eth0_ipaddr 192.168.3.1->192.168.3.29
		3.当以-m或-s参数启动时，记录工具信息(版本号，参数)到日志  DONE
			如:run para_conv -s ,version:1.01 
		4.按文档设置默认值，没有默认值的不存在则不创建
		5.devinfo的每个变量都原样放到gt1000.ini,实现dev_to_gt1000函数
		6.密码变成****
		7.日志左对齐
		8.日志加是否修改，便于grep
		
//1.1 去掉了flags以确保正确性和程序的简单
//1.0 initial
*/
static int change_flag=0;

//输出帮助信息并退出
int print_helpmsg(void)
{
	printf("\n欢迎使用para_conv配置文件转换工具!(version %s)\n\n",VERSION);
	printf("使用说明: para_conv [OPTION]\n\n");
	printf("\t-h\t输出帮助信息\n");
	printf("\t-v\t输出版本信息\n");
	printf("\t-m\t多转一，即 多个配置文件 -> gt1000.ini\n");
	printf("\t-s\t一转多，即   gt1000.ini -> 其他多个配置文件\n\n");
	return(0);
}

//输出版本信息并退出
int print_version(void)
{
	printf("\nThis is para_conv version %s\n\n",VERSION);
	return (0);
}


//从fp指向的chap类型文件中读出usr,pswd,成功返回0，否则返回负值
int get_chap_user_pswd(FILE *fp, char *usr, char *pswd)
{
	char buf[500];
	char tmp[100];
	char *lp,*lk,*lm,*ln;
	
	if((fp==NULL)||(usr==NULL)||(pswd==NULL))
	{	
		gtloginfo("get_chap_user_pswd传进参数为空,返回\n");
		printf("get_chap_user_pswd传进参数为空,返回\n");
		return -1;
	}
		
	while(1)
	{
		if(fgets(buf,500,fp)==NULL)//should never get here
			{
				return -1;
			}
		if(buf[0]=='#')//注释行不处理
			{
				continue;
			}
		lp=index(buf,'\"');//寻找第一个双引号
		if(lp!=NULL)
			{	
				lp++;
				lk=index(lp,'\"');//寻找第二个双引号
				if(lk!=NULL)
					{
						strncpy(tmp,lk,100);//把第二个引号之后的cpy给tmp
						*lk='\0';	
						strncpy(usr,lp,100);//把user部分取出来cpy给usr
						lm=index(tmp,'*');//寻找*号
						if(lm!=NULL)
							{
								lm=index(lm,'\"');//寻找第三个引号
								if(lm!=NULL)
								{
									lm++;
									ln=index(lm,'\"');//寻找最后一个引号
									if(ln!=NULL)
									{
										*ln='\0';
										strncpy(pswd,lm,100);//把pswd部分取出来cpy给pswd
										return 0;
									}
								}
							}
					}			
			}

	}
	return -1; //有什么条件没满足而返回了
}

//向fp指向的chap类型文件中写入usr和pswd
int set_chap_user_pswd(FILE *fp, char *usr, char *pswd)
{
	char buf[5500];
	char line[100];
	char *lp;

	if((fp==NULL)||(usr==NULL)||(pswd==NULL))
	{	
		gtloginfo("set_chap_user_pswd传进参数为空,返回\n");
		printf("set_chap_user_pswd传进参数为空,返回\n");
		return -1;
	}
	sprintf(line,"\n\"%s\" * \"%s\"",usr,pswd);

	rewind(fp);
	fread(buf,5500,1,fp);
	lp=index(buf,'"');//找到第一个引号
	if(lp!=NULL)
		{
			*lp='\0';
		}
	strcat(buf,line);
	rewind(fp);
	fwrite(buf,strlen(buf),1,fp);
	
	return 0;
}
//从boa文件中取出key对应str,成功返回0，否则返回负值
int get_boa_str(FILE *fp, char *key, char *str)
{
	char buf[500];
	char *lp,*lk,*lm;
	char tmp;
	if((fp==NULL)||(key==NULL))
	{	
		gtloginfo("get_boa_str传进参数为空,返回\n");
		printf("get_boa_str传进参数为空,返回\n");
		return -1;
	}
	
	while(1)
	{
		if(fgets(buf,500,fp)==NULL)
			{
				return -1;
			}
		if(buf[0]=='#')
			continue;
		lp=strstr(buf,key);
		if(lp!=NULL)
			{
				lp++;
				lk=index(lp,' ');
				if(lk!=NULL)
					{
						lk++;
						while(1)
							{
								if(*lk==' ')
									lk++;
								else
									break;
							}
						
						lm=index(lk,' ');
					
						if(lm==NULL)
							{
							
							lm=index(lk,'\t');
							}
						
						if(lm==NULL)
							{
							lm=index(lk,'\n');
							}
						if(lm==NULL)
							{
							lm=index(lk,'\0');
							}
						if(lm==NULL)
							{
							return -1;
							}
						tmp=*lm;
						*lm='\0';
						break;
					}			
			}
	}
	//printf("lk is %s\n",lk);
	sprintf(str,lk,strlen(lk));
	return 0;
}

//向fp指向的boa文件中的key节写入value的值，不成功返回-1，成功返回0
int set_boa_str(FILE *fp,char *key,char *value)
{
	char keyline[100];
	char tmp[15000];
	char line[200];
	char  buf[15000];
	char *lp,*lk;

	if((fp==NULL)||(key==NULL)||(value==NULL))
	{	
		gtloginfo("set_boa_str传进参数为空,返回\n");
		printf("set_boa_str传进参数为空,返回\n");
		return -1;
	}
	sprintf(keyline,"\n%s ",key);//"Port "
	sprintf(line,"%s %s\n",key,value);
	
	rewind(fp);
	fread(tmp,15000,1,fp);
	
	lp=strstr(tmp,keyline);
	if(lp==NULL)//没有该节，创建
		{
			strncat(tmp,line,15000);
			//printf("strlen tmp is %d\n",strlen(tmp));
			rewind(fp);
			fwrite(tmp,strlen(tmp),1,fp);
		}
	else //有该节，改写
		{
			
			lk=index(lp+1,'\n');
			if(lk==NULL)
				{
					sprintf(lp,"\n%s",line);
					
				}
			else
			{
				lk++;
				strncpy(buf,lk,strlen(lk));
				sprintf(lp,"\n%s%s",line,buf);
			}
			rewind(fp);
			fwrite(tmp,strlen(tmp),1,fp);
		}
	
	return 0;
}

/*将ini文件的inisec节读出，与conf文件的confsec节读出内容比较，若不同则改写ini
  若哪个不存在则创建哪个并赋缺省值*/
int conf_to_ini(char *confname, dictionary *ini, confdict *conf, char * inisec, char *confsec, char * def)
{
	char *inistr;
	char *confstr;
	
	if((confname==NULL)||(ini==NULL)||(conf==NULL)||(inisec==NULL)||(confsec==NULL))
	{	
		gtloginfo("conf_to_ini传进参数为空,返回\n");
		printf("conf_to_ini传进参数为空,返回\n");
		return -1;
	}
	
	confstr=confparser_getstring(conf,confsec,NULL);
	if(confstr==NULL)//找不到相应变量
	{
		if(def==NULL)//无缺省值
			return -1;
		else
		{
			printf("%s没有%s\t,创建缺省为%s\n",confname,confsec,def);
			confparser_setstr(conf,confsec,def);
			change_flag=1;
			confstr=def;
		}
	}
	
	inistr=iniparser_getstring(ini, inisec, CANT_FIND_KEY);
	#if 0 //shixin del these 2006.11.16
	if(strcmp(inistr,CANT_FIND_KEY)==0)//找不到相应变量则创建
		{
			printf("gt1000.ini没有%s,\t创建为%s\n",inisec,confstr);
			gtloginfo("%-30sNULL->%s\n",inisec,confstr);
			iniparser_setstr(ini,inisec,confstr);
			change_flag=1;
			inistr=confstr;
		}
	#endif
	if(strcmp(inistr,confstr)!=0)
	{
			printf("%-30s%s->%s\n",inisec,inistr,confstr);
			gtloginfo("%-30s%s->%s\n",inisec,inistr,confstr);
			change_flag=1;
			iniparser_setstr(ini,inisec,confstr);
	}
	return 0;
}

/*将ini文件的inisec节读出，与conf文件的confsec节读出内容比较，若不同则改写conf
  若哪个不存在则创建哪个并赋缺省值*/
int ini_to_conf(char *confname,dictionary *ini, confdict *conf, char * inisec, char *confsec, char * def)
{
	char *inistr;
	char *confstr;
	
	if((confname==NULL)||(ini==NULL)||(conf==NULL)||(inisec==NULL)||(confsec==NULL))
	{	
		gtloginfo("conf_to_ini传进参数为空,返回\n");
		printf("conf_to_ini传进参数为空,返回\n");
		return -1;
	}
	inistr=iniparser_getstring(ini, inisec,NULL);
	if(inistr==NULL)//找不到相应变量则创建
	{
		if(def==NULL)
			return -1;
		else
		{
			printf("gt1000.ini没有%s,\t创建为%s\n",inisec,def);
			iniparser_setstr(ini,inisec,def);
			change_flag=1;
			inistr=def;	//shixin changed NULL->def
		}
	}

	confstr=confparser_getstring(conf,confsec,CANT_FIND_KEY);
#if 0 //shixin del these 2006.11.16
	if(strcmp(confstr,CANT_FIND_KEY)==0)//找不到相应变量
		{
				printf("%s没有%s\t,创建缺省为%s\n",confname,confsec,def);
				confparser_setstr(conf,confsec,def);
				change_flag=1;
				confstr=NULL;
		}
#endif	
	if(strcmp(inistr,confstr)!=0)
	{
		printf("[%s] %s : %s->%s\n",confname,confsec,confstr,inistr);
		gtloginfo("%-30s%s->%s\n",inisec,confstr,inistr);
		change_flag=1;
		confparser_setstr(conf,confsec,inistr);
	}
	return 0;
}

/*将devinfo文件的每个节读出，与gt1000文件相应节读出内容比较，
  若不同则改写gt1000并设置变更
  若gt1000.ini不存在该节则创建*/
int dev_to_gt1000(dictionary *gt1000)
{
	dictionary *devinfo;
	char *gt1000str;
	int i;
	
	if(gt1000==NULL)
		{
			printf("gt1000 dict为NULL,退出dev_to_gt1000\n");
			gtloginfo("gt1000 dict为NULL,退出dev_to_gt1000\n");
			return -1;
		}
	
	devinfo=iniparser_load(DEVINFO_PARA_FILE);
	if (devinfo==NULL) 
        {
             printf("cannot parse ini file [%s]",DEVINFO_PARA_FILE);
			 gtloginfo("解析%s失败退出\n",DEVINFO_PARA_FILE);	
             return(-1) ;
		}
	
	for (i=0 ; i<devinfo->size ; i++) 
	{
        if (devinfo->key[i]==NULL)
            continue ;
        if (strchr(devinfo->key[i], ':')==NULL)//没有:,为节名 
        	continue;
        if (strstr(devinfo->key[i], "disk_capacity") != NULL)//磁盘容量，不转换
    		continue;
    		
        //对每一条变量, 看gt1000是否有，如果没有则创建
        gt1000str=iniparser_getstring(gt1000,devinfo->key[i],CANT_FIND_KEY);
		if(strcmp(gt1000str,CANT_FIND_KEY)==0)//没有
        	{
        		iniparser_setstr(gt1000,devinfo->key[i],devinfo->val[i]);
				change_flag=1;
				printf("gt1000.ini的%s创建为%s\n",devinfo->key[i],devinfo->val[i]);
				gtloginfo("%-30sNULL->%s\n",devinfo->key[i],devinfo->val[i]);

        	}
        else
        	{
        		
				if(strncmp(gt1000str,devinfo->val[i],strlen(devinfo->val[i]))!=0)//有但不相同
					{
						gtloginfo("%-30s%s->%s\n",devinfo->key[i],gt1000str,devinfo->val[i]);
						iniparser_setstr(gt1000,devinfo->key[i],devinfo->val[i]);
						printf("gt1000.ini的%s修改为%s\n",devinfo->key[i],devinfo->val[i]);
						change_flag=1;
					}
				else
					continue;
				 
        	}
       
	}

	iniparser_freedict(devinfo);
	
	return 0;
}


/*将devinfo文件的指定节读出，与/conf/config文件相应节读出内容比较，
  若不同则改写config并设置变更
  若gt1000.ini不存在该节则创建*/
int dev_to_conf(confdict *dict, char *devsec,char *cfgsec)
{
	dictionary *devinfo;
	char *devinfostr;
	char *confstr;
	
	if(dict==NULL)
	{
		printf("config dict为NULL,退出dev_to_conf \n");
		gtloginfo("config dict为NULL,退出dev_to_conf \n");
		return -1;
	}
	
	devinfo=iniparser_load(DEVINFO_PARA_FILE);
	if (devinfo==NULL) 
    {
         printf("cannot parse ini file [%s]",DEVINFO_PARA_FILE);
		 gtloginfo("解析%s失败退出\n",DEVINFO_PARA_FILE);	
         return(-1) ;
	}


    //读devinfo的相应节的值
    devinfostr=iniparser_getstring(devinfo,devsec,CANT_FIND_KEY);
	if(strcmp(devinfostr,CANT_FIND_KEY)==0)//没有
	{
		printf("devinfo.ini没有%s节\n",devsec);
		gtloginfo("devinfo.ini没有%s节\n",devsec);
		return 0;
	}
    else
	{
		confstr = confparser_getstring(dict,cfgsec,CANT_FIND_KEY);
		if(strncmp(devinfostr,confstr,strlen(devinfostr))!=0)//有但不相同
		{
			gtloginfo("%-30s%s->%s\n",cfgsec,confstr,devinfostr);
			confparser_setstr(dict,cfgsec,devinfostr);
			printf("/conf/config文件的%s修改为%s\n",cfgsec,devinfostr);
		}
	}
	
	iniparser_freedict(devinfo);
	
	return 0;
}




//处理chap到ini之间转换的函数，处理变动后写入chap文件，

int chap_to_ini(dictionary *ini,char *filename, char *sec1, char *sec2,char *def)
{
	char chapusr[100];
	char chappswd[100];
	char *iniusr;
	char *inipswd;
	FILE *chapfp;
	
	
	if((ini==NULL)||(filename==NULL)||(sec1==NULL)||(sec2==NULL))
	{	
		gtloginfo("chap_to_ini传进参数为空,返回\n");
		printf("chap_to_ini参数为空,返回\n");
		return -1;
	}

	sprintf(chapusr,"%s",def);
	sprintf(chappswd,"%s",def);
	chapfp=fopen(filename,"r+");	
	if(chapfp==NULL)
		{
			gtloginfo("无法打开chap file %s\n",filename);
			printf("无法打开chap file %s\n",filename);
		}
	
	else
		{
			if(get_chap_user_pswd(chapfp,chapusr,chappswd)!=0)
				{
					printf("取chap文件中的usr,pswd失败,赋缺省值%s\n",def);
					set_chap_user_pswd(chapfp,def,def);	
					change_flag=1;
				}
			fclose(chapfp);
		}

	iniusr=iniparser_getstring(ini, sec1, CANT_FIND_KEY);
	if(strcmp(iniusr,CANT_FIND_KEY)==0)//找不到相应变量则创建
		{
			printf("gt1000.ini没有%s,\t创建为%s\n",sec1,chapusr);
			gtloginfo("%-30sNULL->%s\n",sec1,chapusr);
			change_flag=1;
			iniparser_setstr(ini,sec1,chapusr);
			iniusr=chapusr;
		}
	inipswd=iniparser_getstring(ini, sec2, CANT_FIND_KEY);
	if(strcmp(inipswd,CANT_FIND_KEY)==0)//找不到相应变量则创建
		{
			printf("gt1000.ini没有%s,\t创建为****\n",sec2);
			gtloginfo("%-30sNULL->****\n",sec2);
			change_flag=1;
			iniparser_setstr(ini,sec2,chappswd);
			inipswd=chappswd;
		}

	if(strcmp(iniusr,chapusr)!=0)
		{
			printf("gt1000.ini的%s按chap由%s修改为%s\n",sec1,iniusr,chapusr);
			gtloginfo("%-30s%s->%s\n",sec1,iniusr,chapusr);
			change_flag=1;
			iniparser_setstr(ini,sec1,chapusr);
		}
	if(strcmp(inipswd,chappswd)!=0)
		{
			printf("gt1000.ini的%s按chap由****修改为****\n",sec2);
			gtloginfo("%-30s****->****\n",sec2);
			change_flag=1;
			iniparser_setstr(ini,sec2,chappswd);
		}
	return 0;
}

//处理ini到chap之间转换的函数，需处理变动后写入chap文件，

int ini_to_chap(dictionary *ini,char *filename, char *sec1, char *sec2,char *def)
{


	char chapusr[100];
	char chappswd[100];
	char *iniusr;
	char *inipswd;
	FILE *chapfp;
	
	
	if((ini==NULL)||(filename==NULL)||(sec1==NULL)||(sec2==NULL))
	{	
		gtloginfo("chap_to_ini传进参数为空,返回\n");
		printf("chap_to_ini传进参数为空,返回\n");
		return -1;
	}

	iniusr=iniparser_getstring(ini, sec1, CANT_FIND_KEY);
	if(strcmp(iniusr,CANT_FIND_KEY)==0)//找不到相应变量则创建
		{
			printf("gt1000.ini没有%s,\t创建缺省为%s\n",sec1,def);
			iniparser_setstr(ini,sec1,def);
			change_flag=1;
			iniusr=def;
		}
	inipswd=iniparser_getstring(ini, sec2, CANT_FIND_KEY);
	if(strcmp(inipswd,CANT_FIND_KEY)==0)//找不到相应变量则创建
		{
			printf("gt1000.ini没有%s,\t创建缺省为****\n",sec2);
			iniparser_setstr(ini,sec2,def);
			inipswd=def;
			change_flag=1;
		}

	
	chapfp=fopen(filename,"r+");
	if(chapfp==NULL)
		{
			gtloginfo("无法打开chap file %s\n",filename);
			printf("无法打开chap file %s\n",filename);
			return 0; 
		}
	else
		{
	
			if(get_chap_user_pswd(chapfp,chapusr,chappswd)!=0)
				{
					printf("取chap文件中的usr,pswd失败,赋值%s,%s\n",iniusr,inipswd);
					gtloginfo("%-30sNULL->%s\n%-30sNULL->****\n",sec1,iniusr,sec2);
					set_chap_user_pswd(chapfp,iniusr,inipswd);
					change_flag=1;
				}
		

	if((strcmp(iniusr,chapusr)!=0)||(strcmp(inipswd,chappswd)!=0))
		{
			printf("chap文件%s中的%s,按gt1000改为%s\n",filename,sec1,iniusr);
			printf("chap文件%s中的%s,按gt1000改为****\n",filename,sec2);
			gtloginfo("%-30s%s->%s\n",sec1,chapusr,iniusr);
			gtloginfo("%-30s****->****\n",sec2);
			set_chap_user_pswd(chapfp,iniusr,inipswd);
			change_flag=1;
		}

	fclose(chapfp);
		}

	return 0;

}

//处理ini到boa之间转换的函数，需处理变动后写入boa文件，
int ini_to_boa(dictionary *ini, char *secini, char *secboa, char *def)
{
	char *iniport;
	char *boaport;
	char tmp[100];
	FILE *fpboa;	

	if((ini==NULL)||(secini==NULL)||(secboa==NULL))
	{	
		gtloginfo("ini_to_boa传进参数为空,返回\n");
		printf("ini_to_boa传进参数为空,返回\n");
		return -1;
	}

	boaport=tmp;
	iniport=iniparser_getstring(ini, secini, CANT_FIND_KEY);
	if(strcmp(iniport,CANT_FIND_KEY)==0)//找不到相应变量则创建
		{
			printf("gt1000.ini没有%s,\t创建缺省为%s\n",secini,def);
			iniparser_setstr(ini,secini,def);
			iniport=def;
		}

	fpboa=fopen(BOA_FILE,"r+");
	if(fpboa==NULL)
		{
			gtloginfo("无法打开boa file %s,\n",BOA_FILE);
			printf("无法打开boa file %s\n",BOA_FILE);
			return -1;
		}
	else
		{
			get_boa_str(fpboa,secboa,boaport);
			if(boaport==NULL)
				{
					printf("无法读取boa文件的相应值,赋值%s\n",iniport);
					gtloginfo("%-30sNULL->%s\n",secini,iniport);
					set_boa_str(fpboa,secboa,iniport);
					change_flag=1;
				}
		}

	if((strcmp(iniport,boaport)!=0))
		{
			printf("boa文件中的%s,按gt1000由%s改为%s\n",secboa,boaport,iniport);
			gtloginfo("%-30s%s->%s\n",secini,boaport,iniport);
			set_boa_str(fpboa,secboa,iniport);
			change_flag=1;
		}
	fclose(fpboa);
	
	
	return 0;

}

//处理boa到ini之间转换的函数，需处理boa变动(if any)后写入boa文件，
int boa_to_ini(dictionary *ini, char *secini, char *secboa, char *def)
{
	char *iniport;
	char *boaport;
	FILE *fpboa;
	char tmp[100];

	
	if((ini==NULL)||(secini==NULL)||(secboa==NULL))
	{	
		gtloginfo("boa_to_ini传进参数为空,返回\n");
		printf("boa_to_ini传进参数为空,返回\n");
		return -1;
	}
	
	boaport=tmp;
	fpboa=fopen(BOA_FILE,"r+");
	if(fpboa==NULL)
		{
			gtloginfo("无法打开boa file %s\n",BOA_FILE);
			printf("无法打开boa file %s\n",BOA_FILE);
			boaport=def;
		}
	else

		{
			get_boa_str(fpboa,secboa,boaport);
			if(boaport==NULL)
				{
					printf("无法读取boa文件的相应值,赋缺省值%s\n",def);
					set_boa_str(fpboa,secboa,def);
					change_flag=1;
				}
		
	iniport=iniparser_getstring(ini, secini, CANT_FIND_KEY);
	if(strcmp(iniport,CANT_FIND_KEY)==0)//找不到相应变量则创建
		{
			printf("gt1000.ini没有%s,\t创建为%s\n",secini,boaport);
			gtloginfo("%-30sNULL->%s\n",secini,boaport);
			iniparser_setstr(ini,secini,boaport);
			change_flag=1;
		}
	if((strcmp(iniport,boaport)!=0))
		{
			printf("gt1000.ini的%s按boa由%s修改为%s\n",secini,iniport,boaport);
			gtloginfo("%-30s%s->%s\n",secini,iniport,boaport);
			iniparser_setstr(ini,secini,boaport);
			change_flag=1;
		}
	

		}
	fclose(fpboa);

	return 0;
}

/*根据value值，向dhcp文件中写入相应的字符串*/
int set_dhcp_file(int value)
{
	confdict * conf;
	FILE *fp;
	char str[10];
	conf=confparser_load_lockfile(DHCP_FILE,1,&fp);
	if (conf==NULL) 
        {
             printf("cannot parse conf file [%s]",DHCP_FILE);
			 gtloginfo("解析%s失败退出\n",DHCP_FILE);	
             return -2 ;
		}

	if(value == 1)
		sprintf(str,"dhcp");
	else
		sprintf(str,"none");
	confparser_setstr(conf, "BOOTPROTO",str );
	confparser_dump_conf(DHCP_FILE,conf,fp);
	fclose(fp);
	confparser_freedict(conf);
	return 0;

}

//多转一并退出
int para_conv_mto1(void)
{
	confdict *conf;
	dictionary *gt1000;
	FILE *fpgt1000;
	FILE *fp;
	int i;
	
	gtloginfo("run para_conv -m,配置文件多转一，version:%s\n",VERSION);
	printf("\nrun para_conv -m,配置文件多转一，version:%s\n",VERSION);
	
	change_flag=0;
	//解析gt1000.ini文件
	gt1000=iniparser_load_lockfile(IPMAIN_PARA_FILE,1,&fpgt1000);
	if (gt1000==NULL) 
        {
             printf("cannot parse ini file [%s]",IPMAIN_PARA_FILE);
			 gtloginfo("解析%s失败退出\n",IPMAIN_PARA_FILE);	
             return(-1) ;
		}
	
	//从devinfo转
	printf("\n------%s ->  %s------\n\n",DEVINFO_PARA_FILE,IPMAIN_PARA_FILE);
	dev_to_gt1000(gt1000);
		
	//从config转
	printf("\n------%s ->  %s------\n\n",CONFIG_FILE,IPMAIN_PARA_FILE);
	
	conf=confparser_load_lockfile(CONFIG_FILE,1,&fp);
	if (conf==NULL) 
        {
             printf("cannot parse conf file [%s]",CONFIG_FILE);
			 gtloginfo("解析%s失败退出\n",CONFIG_FILE);	
             return -2 ;
		}
	
	dev_to_conf(conf,"devinfo:devtypestring","HOST_NAME");
	conf_to_ini(CONFIG_FILE,gt1000,conf,"eth0:ipaddr", 	"ETH0_IPADDR",	NULL);
	conf_to_ini(CONFIG_FILE,gt1000,conf,"eth0:netmask",	"ETH0_NETMASK",	NULL);
	conf_to_ini(CONFIG_FILE,gt1000,conf,"eth0:mac_addr",	"MAC_ADDRESS",	NULL);
	if(get_eth_num()>=2)
	{
		conf_to_ini(CONFIG_FILE,gt1000,conf,"eth1:ipaddr", 	"ETH1_IPADDR",	"");
		conf_to_ini(CONFIG_FILE,gt1000,conf,"eth1:netmask",	"ETH1_NETMASK",	"");
		conf_to_ini(CONFIG_FILE,gt1000,conf,"eth1:mac_addr",	"MAC1_ADDRESS",	"");
	}
	conf_to_ini(CONFIG_FILE,gt1000,conf,"net_config:route_mac_bind",	"ROUTE_MAC_BIND","0");
	conf_to_ini(CONFIG_FILE,gt1000,conf,"net_config:route_mac",	"ROUTE_MAC","0");
	
	conf_to_ini(CONFIG_FILE,gt1000,conf,"net_config:internet_mode",	"INTERNET_MODE",NULL);
	conf_to_ini(CONFIG_FILE,gt1000,conf,"net_config:route_default",	"ROUTE_DEFAULT",NULL);
	conf_to_ini(CONFIG_FILE,gt1000,conf,"net_config:dns_server",	"DNS_SERVER",	NULL);
	conf_to_ini(CONFIG_FILE,gt1000,conf,"port:telnet_port",	"LOGIN_PORT",	NULL);
	conf_to_ini(CONFIG_FILE,gt1000,conf,"net_config:use_dhcp", "USE_DHCP", NULL);
	conf_to_ini(CONFIG_FILE,gt1000,conf,"net_config:use_upnp", "USE_UPNP", NULL);
	confparser_dump_conf(CONFIG_FILE,conf,fp);
	fclose(fp);
	confparser_freedict(conf);

	//顺便修改一下dhcp文件
	set_dhcp_file(iniparser_getint(gt1000, "net_config:use_dhcp",0));

	
	//从vsftpd转
	printf("\n------%s ->  %s------\n\n",VSFTPD_FILE,IPMAIN_PARA_FILE);
	
	conf=confparser_load_lockfile(VSFTPD_FILE,1,&fp);
	if (conf==NULL) 
        {
             printf("cannot parse conf file [%s]",VSFTPD_FILE);
			 gtloginfo("解析%s失败退出\n",VSFTPD_FILE);	
             return -2 ;
		}

	conf_to_ini(VSFTPD_FILE,gt1000,conf,"port:ftp_port", 	 "listen_port",	 "21");
	conf_to_ini(VSFTPD_FILE,gt1000,conf,"port:pasv_min_port","pasv_min_port","9011");
	conf_to_ini(VSFTPD_FILE,gt1000,conf,"port:pasv_max_port","pasv_max_port","9030");
	confparser_dump_conf(VSFTPD_FILE,conf,fp);
	fclose(fp);
	confparser_freedict(conf);

	//从boa转
	printf("\n------%s ->  %s------\n\n",BOA_FILE,IPMAIN_PARA_FILE);
	
	i=boa_to_ini(gt1000,"port:web_port","Port","8094");


	//从chap转
	printf("\n------%s ->  %s------\n\n",CHAP_FILE,IPMAIN_PARA_FILE);
	i=chap_to_ini(gt1000,CHAP_FILE,"net_config:adsl_user","net_config:adsl_passwd","0");

		

	save_inidict_file(IPMAIN_PARA_FILE,gt1000,&fpgt1000);

	iniparser_freedict(gt1000);
	if(change_flag==1)
		{
			gtloginfo("修改了参数文件(para file modified!)\n");
			change_flag=0;
		}
	printf("\n全部转换完毕,退出程序.\n");
	return 0;
}

//一转多并退出
int para_conv_1tom(void)
{
	confdict *conf;
	dictionary *gt1000;
	FILE *fpgt1000;
	FILE *fp;

	gtloginfo("run para_conv -s,配置文件一转多，version:%s\n",VERSION);
	printf("\nrun para_conv -s,配置文件一转多，version:%s\n",VERSION);
	change_flag=0;
	gt1000=iniparser_load_lockfile(IPMAIN_PARA_FILE,1,&fpgt1000);
	if (gt1000==NULL) 
        {
             printf("cannot parse ini file [%s]",IPMAIN_PARA_FILE);
			 gtloginfo("解析%s失败退出\n",IPMAIN_PARA_FILE);	
             return(-1) ;
		}

	
	//先处理config文件
	printf("\n------%s ->  %s------\n\n",IPMAIN_PARA_FILE,CONFIG_FILE);
	
	conf=confparser_load_lockfile(CONFIG_FILE,1,&fp);
	if (conf==NULL) 
        {
             printf("cannot parse conf file [%s]",CONFIG_FILE);
			 gtloginfo("解析%s失败退出\n",CONFIG_FILE);	
             return -2 ;
		}
	ini_to_conf(CONFIG_FILE,gt1000,conf,"eth0:ipaddr", 	"ETH0_IPADDR",	NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"eth0:netmask",	"ETH0_NETMASK",	NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"eth0:mac_addr",	"MAC_ADDRESS",	NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"net_config:use_dhcp", "USE_DHCP", NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"net_config:use_upnp", "USE_UPNP", NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"net_config:route_mac_bind",	"ROUTE_MAC_BIND",	"0");
	ini_to_conf(CONFIG_FILE,gt1000,conf,"net_config:route_mac",	"ROUTE_MAC",	"0");
	if(get_eth_num()>=2)
	{
		ini_to_conf(CONFIG_FILE,gt1000,conf,"eth1:ipaddr", 	"ETH1_IPADDR",	NULL);
		ini_to_conf(CONFIG_FILE,gt1000,conf,"eth1:netmask",	"ETH1_NETMASK",	NULL);
		ini_to_conf(CONFIG_FILE,gt1000,conf,"eth1:mac_addr",	"MAC1_ADDRESS",	NULL);
	}
	ini_to_conf(CONFIG_FILE,gt1000,conf,"net_config:internet_mode",	"INTERNET_MODE",NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"net_config:route_default",	"ROUTE_DEFAULT",NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"net_config:dns_server",	"DNS_SERVER",	NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"port:telnet_port",	"LOGIN_PORT",	NULL);
	ini_to_conf(CONFIG_FILE,gt1000,conf,"devinfo:devtypestring","HOST_NAME",NULL);
	confparser_dump_conf(CONFIG_FILE,conf,fp);
	fclose(fp);
	confparser_freedict(conf);


	//再处理vsftpd文件
	printf("\n------%s ->  %s------\n\n",IPMAIN_PARA_FILE,VSFTPD_FILE);
	
	conf=confparser_load_lockfile(VSFTPD_FILE,1,&fp);
	if (conf==NULL) 
        {
             printf("cannot parse conf file [%s]",VSFTPD_FILE);
			 gtloginfo("解析%s失败退出\n",VSFTPD_FILE);	
             return -2 ;
		}
	
	ini_to_conf(VSFTPD_FILE,gt1000,conf,"port:ftp_port", 	 "listen_port",	 "21");
	ini_to_conf(VSFTPD_FILE,gt1000,conf,"port:pasv_min_port","pasv_min_port","9011");
	ini_to_conf(VSFTPD_FILE,gt1000,conf,"port:pasv_max_port","pasv_max_port","9030");
	confparser_dump_conf(VSFTPD_FILE,conf,fp);
	fclose(fp);
	confparser_freedict(conf);

	//再处理boa文件
	printf("\n------%s ->  %s------\n\n",IPMAIN_PARA_FILE,BOA_FILE);
	ini_to_boa(gt1000,"port:web_port","Port","8094");

	//再处理pap文件
	printf("\n------%s ->  %s------\n\n",IPMAIN_PARA_FILE,PAP_FILE);
	ini_to_chap(gt1000,PAP_FILE,"net_config:adsl_user","net_config:adsl_passwd","0");

	//再处理chap文件
	printf("\n------%s ->  %s------\n\n",IPMAIN_PARA_FILE,CHAP_FILE);
	ini_to_chap(gt1000,CHAP_FILE,"net_config:adsl_user","net_config:adsl_passwd","0");

	//再处理ppp0文件
	printf("\n------%s ->  %s------\n\n",IPMAIN_PARA_FILE,PPP0_FILE);

	conf=confparser_load_lockfile(PPP0_FILE,1,&fp);
	if (conf==NULL) 
        {
             printf("cannot parse conf file [%s]",PPP0_FILE);
			 gtloginfo("解析%s失败退出\n",PPP0_FILE);	
             return -2 ;
		}
	
	ini_to_conf(PPP0_FILE,gt1000,conf,"net_config:adsl_user","USER","0");
	confparser_dump_conf(PPP0_FILE,conf,fp);
	fclose(fp);
	confparser_freedict(conf);

	//处理dhcp文件
	set_dhcp_file(iniparser_getint(gt1000, "net_config:use_dhcp",0));

	

	//最后整理gt1000.ini
	
	save_inidict_file(IPMAIN_PARA_FILE,gt1000,&fpgt1000);
	iniparser_freedict(gt1000);
	if(change_flag==1)
		{
			gtloginfo("修改了参数文件(para file modified!)\n");
			change_flag=0;
		}
	printf("\n全部转换完毕,退出程序.\n");
	return 0;
}



int main(int argc,char **argv)
{
/*   处理命令行参数:
 *		启动时没有带参数则显示帮助信息；
 *		-h:输出帮助信息；
 *		-v:输出转换工具的版本信息；
 *		-m:表示多转一，即 多个配置文件->gt1000.ini；
 *		-s:表示一转多，即gt1000.ini->其他多个配置文件；*/
	init_devinfo();		 

//printf("hqdata %d M, update %d M\n",get_disk_total("/hqdata"),get_disk_total("/hqdata/update"));
	if((argc!=2)||(argv[1]==NULL))
		{
			print_helpmsg();
			return 0;
		}
	if(strncmp(argv[1],"-m",2)==0)
		{
			para_conv_mto1();
			return 0;
		}
	if(strncmp(argv[1],"-s",2)==0)
		{
			para_conv_1tom();
			return 0;
		}
	if(strncmp(argv[1],"-v",2)==0)
		{
			print_version();
			return 0;
		}
	else
		{
			print_helpmsg();
			return 0;
		}
	return 0;
}
