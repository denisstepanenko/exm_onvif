/*进行必要的系统信息转换
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <devinfo.h>
#include <iniparser.h>
#include <confparser.h>
#include <errno.h>
/**********************************************************************************************
 * 函数名	:check_valid_mac()
 * 功能	:本函数用于检查mac地址的合法性
 * 输入	:macstr:描述mac地址的字符串"AA:BB:CC:DD:EE:FF"
 *			 maclen:字符串的长度
 * 返回值	:1表示合法 0表示非法 -1表示参数错误
 **********************************************************************************************/
static int check_valid_mac(char *macstr,int maclen)
{
	int i;
	int num;
	if(macstr==NULL)
		return -1;
	num=0;
	for(i=0;i<maclen;i++)
	{
		if(macstr[i]==':')
			num++;
	}
	if(num==5)
		return 1;
	return 0;
}
/**********************************************************************************************
 * 函数名	:conv_guid2mac()
 * 功能	:将guid信息转化为mac地址字符串
 * 输入	:
 *		eth_num:网口编号
 *		guid:存放guid的缓冲区
 *		guid_len:缓冲区中guid的有效字节数
 * 输出	:
 *	mac_buf:返回时填充mac地址的缓冲区，字符串"AA:BB:CC:DD:EE:FF"
 * 返回值	:负值表示出错 正值表示填充的长度
 **********************************************************************************************/
static int conv_guid2mac(int eth_num,char *guid,int guid_len,char *mac_buf)
{
	int 	i;
	char gtemp[20];
	if((eth_num!=0)&&(eth_num!=1))
		return -EINVAL;
	if((guid==NULL)||(guid_len<=0)||(mac_buf==NULL))
		return -1;
	if(guid_len>sizeof(gtemp))
		guid_len=sizeof(gtemp);
	memset(gtemp,0,sizeof(gtemp));
	for(i=0;i<guid_len;i++)
		gtemp[i]=guid[i];
	if(eth_num==1)
		gtemp[4]+=1;	///网口1的mac地址次高位为最高位加1 //shixin 2007.07.04 changed
	sprintf(mac_buf,"%02x:%02x:%02x:%02x:%02x:%02x",(int)gtemp[5],(int)gtemp[4],(int)gtemp[3],(int)gtemp[2],(int)gtemp[1],(int)gtemp[0]);
	return (strlen(mac_buf)+1);	
}
/**********************************************************************************************
 * 函数名	:get_conf_mac()
 * 功能	:从配置文件中获取mac地址字符串
 * 输入 : eth_num:网卡编号 0,1
 * 输出	:
 *	  mac_buf:存放mac地址字符串的缓冲区
 * 返回值	:负值表示出错 正值表示填充的长度
 **********************************************************************************************/

static int get_conf_mac(int eth_num,char *mac_buf)
{
	char *fileconfig="/conf/config";	//存放mac地址的配置文件
	confdict *conf;
	char *mac;
	if((eth_num!=0)&&(eth_num!=1))
		return -EINVAL;
  	conf=confparser_load(fileconfig);
	if (conf==NULL) 
	{
            printf("cannot parse conf file file [%s]\n", fileconfig);
            return -1 ;
       }
	if(eth_num==0)	
		mac=confparser_getstring(conf,"MAC_ADDRESS","NULL");
	else
		mac=confparser_getstring(conf,"MAC1_ADDRESS","NULL");
	memcpy(mac_buf,mac,strlen(mac)+1);
	confparser_freedict(conf);
	return strlen((mac_buf)+1);
	
}
/**********************************************************************************************
 * 函数名	:set_conf_mac()
 * 功能	:设置mac地址到配置文件
 *	  eth_num:网卡编号
 * 	  mac_buf:存放mac地址字符串的缓冲区
 * 返回值	:0表示成功 负值表示失败
 **********************************************************************************************/
static int set_conf_mac(int eth_num,char *mac_buf)
{
	char *fileconfig="/conf/config";	//存放mac地址的配置文件
	FILE  *fp=NULL;
	confdict *conf;
	int	ret;
	if((eth_num!=0)&&(eth_num!=1))
		return -EINVAL;
  	conf=confparser_load(fileconfig);
	if (conf==NULL) 
	{
            printf("cannot parse conf file file [%s]\n", fileconfig);
            return -1 ;
       }
	
	fp = fopen(fileconfig,"w");
	if(fp==NULL)
	{
               fprintf(stderr, "Cannot open output file[%s]",fileconfig);
	 		 confparser_dump_conf(fileconfig,conf,fp);			   
               return -1;
       }		

	if(eth_num==0)
		ret=confparser_setstr(conf,"MAC_ADDRESS",mac_buf);
	else
		ret=confparser_setstr(conf,"MAC1_ADDRESS",mac_buf);
	confparser_dump_conf(fileconfig,conf,fp);			
	fclose(fp);
	return 0;
	
}


/*本函数用于检测mac地址是否正确，如
 * 不正确则把/conf/devinfo中的devguid值改写成mac地址形式并写入/conf/config */
//返回值 <0表示出错
//0表示两个文件描述的mac地址相同
// 1表示已经将ini中的mac更新到config文件中
/**********************************************************************************************
 * 函数名	:mac_check_conv()
 * 功能	:检测mac地址是否正确，如不正确则把/conf/devinfo中的devguid值转换
 *			 成mac地址形式并写入/conf/config 
 * 参数:	eth_num:网卡编号0,1,...
 * 返回值	:0表示成功 负值表示失败
 **********************************************************************************************/
int mac_check_conv(int eth_num)
{
	const char  GUID_PROTO[8] = {0x0,0x0,0x0,0x0,'t',0x0,'g',0x0};	//新设备guid
	char newmac[32];
	char oldmac[32];
	char guid[32];
	int	guid_len;	
	if((eth_num!=0)&&(eth_num!=1))///只支持两块网卡
		return -EINVAL;
	guid_len=get_devid(guid);
	if(guid_len<0)
		return -1;
	if(memcmp(guid,GUID_PROTO,sizeof(GUID_PROTO))==0)
		return 0;//说明是新设备不用进行mac地址转换
	
	if(get_conf_mac(eth_num,oldmac)<0)
		return -1;

	if(check_valid_mac(oldmac,strlen(oldmac))<=0)
	{
		if(conv_guid2mac(eth_num,guid,guid_len,newmac)<0)	//将guid转换为mac地址
			return -1;
		set_conf_mac(eth_num,newmac);
		printf("eth%d oldmac=%s newmac=%s!!\n",eth_num,oldmac,newmac);
		return 1;
	}
	

	return 0;
	
}

/*本函数用于检测mac地址是否正确，如
 * 不正确则把/conf/devinfo中的devguid值改写成mac地址形式并写入/conf/config */
//返回值 <0表示出错
//0表示两个文件描述的mac地址相同
// 1表示已经将ini中的mac更新到config文件中
#if 0
int mac_check_conv(void)
{

	char parastr[100];
	dictionary *ini1;
	confdict *conf1;	
	int section_len;
	int i;
	char section[10];
	char macaddress[30];
	char *devguid,*oldmac,*newmac;
	char *cat,*lk;
	FILE *fp;
	char *fileini="/conf/devinfo.ini";//guid文件
	char *fileconfig="/conf/config";//启动脚本用到的配置文件
	
	//初始化并从/conf/devinfo.ini中读出devguid的值到devguid中
	sprintf(section,"devinfo");
	if((fileini==NULL)||(fileconfig==NULL))
		return -1;
	section_len=strlen(section);
	ini1=iniparser_load(fileini);
	if (ini1==NULL) 
		{
            printf("cannot parse ini file file [%s]\n", fileini);
            return -1 ;
       }
 	memcpy(parastr,section,section_len);
	parastr[section_len]=':';
	section_len++;
	parastr[section_len]='\0';
	cat=strncat(parastr,"devguid",strlen("devguid"));	
	devguid=iniparser_getstring(ini1,parastr,"NULL");
	printf("devguid= %s\n",devguid);

    //把devguid改写成macaddress
    	section_len=strlen(devguid);
    	if(section_len!=16)
 	{
		printf("devguid length not correct! %d\n",section_len);
		iniparser_freedict(ini1);
		return -2;
    	}
    	lk=devguid+4;
    	for (i=0;i<6;i++)
    	{
			macaddress[3*i]=*lk;
			lk++;
			macaddress[3*i+1]=*lk;
			lk++;
			macaddress[3*i+2]=':';
    	}	
    	macaddress[17]='\0';	
  	 // printf("new macaddress=%s\n",macaddress);
    	newmac=macaddress;
    	iniparser_freedict(ini1);

  	//从/conf/config中取出MAC ADDRESS进行比较
	
  	conf1=confparser_load(fileconfig);
	if (conf1==NULL) 
	{
            printf("cannot parse conf file file [%s]\n", fileconfig);
            return -1 ;
       }
		
	oldmac=confparser_getstring(conf1,"MAC_ADDRESS","NULL");
	//printf("old mac=%s\n",oldmac);
	//printf("oldmac= %s\n",oldmac);
	//若两者相同则直接退出
	//若mac地址合法则直接退出
	//remed by shixin if(check_valid_mac(oldmac,strlen(oldmac)))//strcmp(oldmac,newmac)==0)
	if(memcmp(oldmac,newmac,strlen(newmac))==0)
	{
		//printf("no need to modify! correct MAC_ADDRESS\n");
		confparser_freedict(conf1);
		return 0;
	}
	//若不同则进行修改，把macaddress写入/conf/config
	else
	{	
			
		fp = fopen(fileconfig,"w");
		if(fp==NULL)
              	{
                      fprintf(stderr, "Cannot open output file[%s]",fileconfig);
                      return -3;
              	}		

		i=confparser_setstr(conf1,"MAC_ADDRESS",newmac);
		//iniparser_dump_ini(conf1,stdout);
		confparser_dump_conf(conf1,fp);			
		fclose(fp);
			
	}
    

	//convert_file2ini(fileconfig);	
	confparser_freedict(conf1);
 	return 1;
}
#endif


