#include<stdio.h>
#include<stdlib.h>
#include<devinfo.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<gtlog.h>
#include<diskinfo.h>
#include<guid.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include<commonlib.h>
#include<errno.h>
#include <sys/ioctl.h>


#define DISK_FILE		("/log/diskstate.txt")
#define DISK_NAME		("/dev/sda")

//#define USE_SD

#define BUF_LEN  100
#define PACKAGE    			("init_all_ide_drvs")

#define VERSION 				("1.18")
//ver:1.18	增加读取tf/sd卡的分区表部分，判断要不要格式化这张卡
//ver:1.17	增加区分是tf卡还是sd，用以自动加载不同的驱动程序
//ver:1.16	将sd的节点名改为/dev/hda，视为硬盘处理，但只分一个区
//		这个版本需要配合sd卡的驱动(sd卡的节点名为/dev/hda的那版),initdisk_ver:1.17
//ver:1.15	将sd卡挂载到/hqdata/hda1下
//ver:1.14	diskinfo库支持sd卡
//ver:1.13	修改init_ide_drvs的返回值处理
//ver:1.12	支持给SD卡分区，格式化，挂载到/hqdata/hda1下
//ver:1.11	修改在/dev下建立的sd卡节点为sda，代替原来的cpesda
//ver:1.10	修改init_ide_drvs中的gtlogerr()中的错误码格式为[%d],不是[0x%d]的格式
//ver:1.09	在检测硬盘之前添加下面的驱动:
//			/lib/modules/ide-core.ko
//			/lib/modules/ide-generic.ko
//			/lib/modules/ide-disk.ko
//ver:1.08	修改ver:1.07中创建符号链接/dev/hda1的方式为/dev/cpesda1
//ver:1.07	修改ver:1.06版本为只负责加载sd驱动，创建符号连接
//ver:1.06	支持检测SD卡分区，格式化，挂载
//ver:1.05	出厂前如果挂载失败格式化硬盘
//ver:1.04
//添加tune2fs /dev/hda1 -m 0
//修改LOCK_FILE路径为/tmp/init_all_ide_drvs

//#define LOCK_FILE 	("/lock/vserver/init_all_ide_drvs")
#define LOCK_FILE	("/lock/ipserver/init_all_ide_drvs")
#define REC_FILE		("/log/init_all_ide_drvs_state.txt")

#if 0
#define DDISK0		("/dev/hda")
#define DDISK1		("/dev/hdb")
#define DDISK2		("/dev/hdc")
#define DDISK3		("/dev/hdd")
#endif

//mount 时用到的
#define MDISKA		("/hqdata/sda")
#define MDISKB		("/hqdata/sdb")
#define MDISKC		("/hqdata/sdc")
#define MDISKD		("/hqdata/sdd")

#define MDPATH			("/hqdata/")

/**********************************************************************************************
* 函数名   :get_fac_flag()
* 功能  :       获取出厂标志
* 输入  :      			
* 输出  :       void        
* 返回值:   1出厂后,0出厂前
**********************************************************************************************/
int get_fac_flag(void)
{
	int ret;
	
	ret=get_lfc_flag();

	return ret;

}

/**********************************************************************************************
* 函数名   :print_help()
* 功能  :       帮助信息
* 输入  :      			
* 输出  :       void        
* 返回值:   void
**********************************************************************************************/
void print_help(int exval)
{
	int i;
	unsigned char *tmp[]={"\n功能:\n",
						"(1) 对未格式化完的硬盘进行重新格式化;\n",
						"(2) 在未出厂状态下，发现未格式化的硬盘自动格式化,出厂后只记录日志;\n",
						"(3) 使用e2fsck检查每个分区节点;\n",
						"(4) 将分区节点mount到相应目录;\n\n",
						NULL};

	
	for(i=0;tmp[i]!=NULL;i++)
	{
		printf("%s",tmp[i]);
	}


         printf("程序: %s, 版本: %s \n", PACKAGE, VERSION);
         printf("%s [-h] [-V] \n\n", PACKAGE);
         printf("  -h              打印帮助信息并退出\n");
         printf("  -V              打印版本信息并退出\n\n");
	 printf("  -B   <重起控制> 0  默认为格式化后不重起  1 重起  \n");

         exit(exval);
}

/**********************************************************************************************
* 函数名   :env_parser()
* 功能  :       解析参数
* 输入  :      
* 输出  :       void        
* 返回值:   boot_flag	重启标志，1重启，0不重启,默认不重启
**********************************************************************************************/
//int env_parser(multicast_sock* net_st,unsigned char*result, int argc, char * argv[])
int env_parser(int argc,char *argv[])

{
	int opt;
	int boot_flag=0;

	while((opt = getopt(argc, argv, "hVB:")) != -1) 
	{
		switch(opt)
		{
			case 'h':
				print_help(0);
				break;

			case 'V':
				printf("\n%s %s\n\n", PACKAGE, VERSION); 
				exit(0);
				break;

                        case 'B':
                                boot_flag = atoi(optarg);
                                if((boot_flag!=0)&&(boot_flag!=1))
                                {
                                        printf("输入参数错误 -B (0,1)%d\n", boot_flag);
                                        boot_flag = 0;
                                }
				break;
   
			case ':':
				fprintf(stderr, "%s: Error - Option `%c' needs a value\n\n", PACKAGE, optopt);
				print_help(1);
				break;
	
			case '?':
				fprintf(stderr, "%s: Error - No such option: `%c'\n\n", PACKAGE, optopt);
				print_help(1);
				break;

			default:
				break;
		}
	}
	return boot_flag;
}

/****************************************************************************************
 *函数名称: init_sd_drv()
 *功    能: 加载sd卡驱动，创建符号链接
 *输    入: ide_flag 为2表示sd，为3表示tf卡，其他值错误退出
 *输    出: 无
 *返    回: 无
 * **************************************************************************************/
void init_sd_drv(int ide_flag)
{
        char diskcmd[100];
        int ret=0;
	int fd;
	int fst;

        //检查节点是否存在 

        //加载SD卡的驱动/lib/modules/ftsdc010.ko
	switch(ide_flag)
	{
		case 2:
			sprintf(diskcmd,"/sbin/insmod %s","/lib/modules/ftsdc010_sd.ko");
			break;
		case 3:
			sprintf(diskcmd,"/sbin/insmod %s","/lib/modules/ftsdc010_tf.ko");
			break;
		default:
			printf("*******[ERR]*******给我的不是硬盘，不是SD卡，也不是TF卡\n");
			return;
	}

        printf("加载驱动[%s]\n",diskcmd);
        ret=system(diskcmd);

	printf("waitting...\n");	
	sleep(2);
	//mdev -s
        memset(diskcmd,0,sizeof(diskcmd));
        sprintf(diskcmd,"mdev -s");
        printf("执行[%s]\n",diskcmd);
        ret=system(diskcmd);

#if  USE_SD //20090811节点名和硬盘的一样，不用符号链接了
        //创建符号连接 
        memset(diskcmd,0,sizeof(diskcmd));
	sprintf(diskcmd,"%s","ln -s /dev/sda /dev/hda");
        printf("创建符号链接[%s]\n",diskcmd);
        ret=system(diskcmd);
#endif

	memset(diskcmd,0,sizeof(diskcmd));
#if USE_SD
	sprintf(diskcmd,"mknod /dev/sda1 b 254 1");
#else
	sprintf(diskcmd,"mknod /dev/sda1 b 254 1");
#endif
	printf("创建节点[%s]\n",diskcmd);
	ret=system(diskcmd);

	
#if USE_SD
	//创建符号连接
	memset(diskcmd,0,sizeof(diskcmd));
	sprintf(diskcmd,"%s","ln -s /dev/sda1 /dev/hda1");
        printf("创建符号链接[%s]\n",diskcmd);
        ret=system(diskcmd);
#endif
	//zw-add 2010-07-26-------->
	//下面这部分调用ftsdc010.ko读取SD/TF卡的分区表，用来判断这个卡
	//是啥样的文件系统类型，如果不是linux格式(0x83)的就给它格了。像
	//FAT32/16这样的就给格了
	fd=open("/dev/sda",O_RDONLY);
	if(fd<0)
	{
		printf("open /dev/sda error\n");
		return ;
	}
	fst=0;
	ret=ioctl(fd,0x4513,&fst);
	if(ret<0)
	{
		printf("获取磁盘类型错误\n");
	}
	printf("这个磁盘分区类型为[0x%02x] :",(int)fst);
	close(fd);

	if(fst!=0x83)
	{
		printf("不是linux的格式,要格式化一下\n");
		system("/ip1004/initdisk -B 1");
	}
	else
	{
		printf("不需格式化，接着用\n");
	}
	//<-----zw-add 2010-07-26

	#if 1
	//通过sd卡的动作，更新/proc/partitions中sd卡分区信息
	memset(diskcmd,0,sizeof(diskcmd));
	sprintf(diskcmd,"%s","fdisk -l /dev/sda");
	printf("使用命令[%s]查看SD分区.\n",diskcmd);
	ret=system(diskcmd);
	#endif


}			  

/**********************************************************************************************
* 函数名   :init_ide_drvs()
* 功能  :       初始化硬盘
* 输入  :      			
* 输出  :       void        
* 返回值:   void
**********************************************************************************************/
int init_ide_drvs(void)
{
	int fd;
	int ret;
//	unsigned int u_ret;
	int ret_n;
//	unsigned int m_ret;
	unsigned char cmd_buf[64];
	int part_num=0;
	int disk_num=0;
	int j;
	int i;
	int ide_flag=0;
//	unsigned char *disks[]={DDISK0,DDISK1,DDISK2,DDISK3,NULL};
//	unsigned char *mdisks[]={MDISKA,MDISKB,MDISKC,MDISKD,NULL};

	unsigned char disks[32];
	unsigned char mdisks[32];
	unsigned char *tmp=NULL;

	ide_flag=get_ide_flag();
	printf("ide_flag:%d\n",ide_flag);
	if(ide_flag!=1) 
	{
		
		//此时此刻，ide不是sd就是cf-20100709
		init_sd_drv(ide_flag);
		//zw-20090811 if(access("/dev/sda",F_OK)==0)  //将sd的节点改为和硬盘的一样，/dev/hda
		if(access("/dev/sda",F_OK)==0)
		{
			printf("有SD卡\n");
		} 
	}
	else
	{
		//printf("有%d个硬盘\n",disk_num);
		//yk del 20130704
		//system("/sbin/insmod  /lib/modules/ide-core.ko");
		//system("/sbin/insmod  /lib/modules/ide-generic.ko");
		//system("/sbin/insmod  /lib/modules/ide-disk.ko");
	}

	//检查硬盘上次格式化时被中断没
	printf("开始检查硬盘上次格式化时是否被中断-->");
	fd=access(DISK_FILE,F_OK);
	if(fd==0)
	{
		gtloginfo("果然被中断了\n");
		printf("果然被中断了\n");
		system("/ip1004/initdisk -B 1");
	}
	else
	{
		gtloginfo("没有被中断了\n");
		printf("没有被中断\n");
	}

	//检查有几个磁盘
	disk_num= get_sys_disk_num();
	if(ide_flag==2)
	{
		//SD修正 ,09-06-12 这个现在需要手动修改为1个sd卡
		/////disk_num=1;
		printf("使用的是sd，自己判断的值，sd卡个数disk_num=[%d]\n",disk_num);
	}

	if(disk_num<=0)
	{
		gtlogerr("没有发现磁盘.\n");
		return -1;
		//exit(1); //为啥当时这里要用exit
	}
	printf("有%d个硬盘\n",disk_num);

	printf("开始检查硬盘节点....\n");
	//检查每个硬盘节点是不是都健在
	for(i=0;i<disk_num;i++)
	{
		memset(disks,0,sizeof(disks));	
		tmp=get_sys_disk_devname(i);
		if(tmp==NULL)
		{
			printf("返回硬盘节点名错误\n");
			gtlogerr("返回硬盘节点名错误\n");
			continue;
		}
		else
		{
			//SD
			#if USE_SD //暂时不用特殊照顾sd卡，当作硬盘处理 20090811-zw
			if(ide_flag==2)
			{
				//09-06-12 目前只支持一个SD卡，写死为/dev/sda,像/dev/sdb,/dev/sdc这些不会出现
				sprintf(disks,"%s","/dev/sda");	
			}
			else
			#endif
				sprintf(disks,"%s",tmp);
				//yk add 20130704
				printf("disks =%s\n",disks);
		}

		ret=access(disks,F_OK);
		if(ret<0)
		{
#if 0
			if(get_lfc_flag()==0)
			{	
				//未出厂调用initdisk处理
				printf("%s不在，未出厂,调用initdisk\n",disks[i]);
				system("/gt1000/initdisk -B 1");
			}
			else
			{
				gtlogerr("没有硬盘节点%s\n",disks[i]);
				printf("没有硬盘节点%s\n",disks[i]);			
			}
#endif
			gtlogerr("硬盘节点%s访问错误\n",disks);
			continue;
		}

		//获取本块硬盘的分区节点个数
		//此时不知硬盘是否已经分区，先获取磁盘分区个数,然后根据个数判断是否分区 
		ret= get_sys_disk_partition_num(disks);		//   /dev/hdx
		printf("[%s]容量为[%ld]MB\n",disks,get_sys_partition_capacity("sda",1));//yk 20130704 change hda sda
////
		#if USE_SD //暂时不用特殊照顾sd卡，当作硬盘处理
		if(ide_flag==2)
		{
			 ////ret=1;	//sd卡只有一个分区，可是目前在/proc/partitions里又没有sd卡分区的信息，暂时先这样
			 printf("现在是sd卡,磁盘名字为[%s],磁盘分区个数为[%d]\n",disks,ret);
		}
		#endif
////
		printf("%s有%d个分区\n",disks,ret);
		gtloginfo("%s有%d个分区\n",disks,ret);
		if(ret>=0)
		{
			//还没有分区
			if(ret==0)
			{
				printf("还没有分区\n");
				if(get_lfc_flag()==1)
				{	
					//出厂后就只能记日志
					gtlogerr("已出厂,但磁盘还没有分区\n");
					printf("出厂后的没有分区\n");
					continue;
				}
				else
				{
					//出厂前就调用initdisk				
					printf("出厂前没有分区,调用initdisk分区.\n");
					gtloginfo("出厂前没有分区,调用initdisk分区.\n");
					system("/ip1004/initdisk -B 1");
					
					//调用initdisk后重启获取磁盘分区个数
					ret_n=0;
					ret_n=get_sys_disk_partition_num(disks);
					printf("出厂前，没有分区，调用initdisk分区后，重新检测分区,使用[%s]搜索,个数为[%d]\n",disks,ret_n);
////
					#if USE_SD
					if(ide_flag==2)
					{
						//ret=1; //sd卡同上
						ret=ret_n;
						printf("[%s]容量为[%ld]\n",disks,get_sys_partition_capacity("sda",1));
					}
					#endif
////
					if(ret_n>0)
					{
						part_num=ret_n;
					}
					else
					{
						gtlogerr("出厂前磁盘还没有分区\n");
						printf("出厂前没有分区\n");
						continue;
					}
				}		
			}
			else
			{
				printf("已经分区了\n");
				part_num=ret;
			}
		}
		else if(ret<0)
			{
				printf("获取磁盘分区节点个数错误\n");
				gtlogerr("获取磁盘分区节点个数错误\n");
				return -1;
			}


		

		for(j=1;j<part_num+1;j++)
		{
			memset(mdisks,0,sizeof(mdisks));
			tmp=get_sys_disk_name(i);
			if(tmp==NULL)
			{
				printf("获取%s第[%d]个分区节点错误\n",disks,j);
				gtlogerr("获取%s第[%d]个分区节点错误\n",disks,j);
			}
			else
			{
				sprintf(mdisks,"%s",tmp);
				printf("---mdisks=[%s]\n",mdisks);
			}
		
			printf("\n\n开始检查第%d个分区节点------>\n",j);
			//检查第x个磁盘的第n个分区是否存在
			memset(cmd_buf,0,sizeof(cmd_buf));
			sprintf(cmd_buf,"%s%d",disks,j);		//   /dev/hdij
			ret=access(cmd_buf,F_OK);
			printf("开始检查[%s]是否存在...\n",cmd_buf);
			if(ret<0)
			{
				if(get_lfc_flag()==1)
				{
					//已出厂,记日志
					gtlogerr("磁盘共有[%d]个分区节点,第[%d]个节点无法访问\n",part_num,j);
					printf("磁盘共有[%d]个分区节点,第[%d]个节点无法访问\n",part_num,j);
				}
				else
				{
					//还没出厂,重分区
					printf("设备还没出厂，重新分区和格式化\n");
					gtloginfo("设备还没出厂，重新分区和格式化\n");
					system("/ip1004/initdisk -B 1");
				}
			}
			else////
			{
				printf("[%s]存在\n",cmd_buf);
			}
#if 1
			memset(cmd_buf,0,sizeof(cmd_buf));
			sprintf(cmd_buf,"umount %s%s%d",MDPATH,mdisks,j);
			printf("使用%s\n",cmd_buf);
			system(cmd_buf);
#endif
			//使用e2fsck检查------e2fsck -y /dev/hda1
			memset(cmd_buf,0,sizeof(cmd_buf));
			sprintf(cmd_buf,"%s%s%d","e2fsck -y ",disks,j);
			printf("使用命令[%s]检查\n",cmd_buf);
			system(cmd_buf);


			//使用tune2fs调整文件系统-----tune2fs /dev/hda1 -m 0
			memset(cmd_buf,0,sizeof(cmd_buf));
			sprintf(cmd_buf,"%s %s%d %s","tune2fs",disks,j,"-m 0");
			printf("使用命令[%s]调整文件系统\n",cmd_buf);
			system(cmd_buf);

			
			//准备mount,检查目标节点在没
			memset(cmd_buf,0,sizeof(cmd_buf));
			sprintf(cmd_buf,"%s%d",mdisks,j);		// /hqdata/hdij
			printf("准备mount使用命令[access %s]检查\n",cmd_buf);
			ret=access(cmd_buf,F_OK);
			if(ret<0)
			{
				//     /hqdata那里没有节点,新创建一个
				memset(cmd_buf,0,sizeof(cmd_buf));
				sprintf(cmd_buf,"mkdir %s%s%d",MDPATH,get_sys_disk_name(i),j);
				printf("没有,创建节点,%s\n",cmd_buf);
				system(cmd_buf);
			}
			else
				printf("%s%d在呢\n",mdisks,j);

			//开始mount
			#if 0
			//zw-del 2009-09-20
			memset(cmd_buf,0,sizeof(cmd_buf));
			if(get_ide_flag()==2)
			{	//wsyadd
				sprintf(cmd_buf,"mkdir /hqdata/hda1");
				system(cmd_buf);
				sprintf(cmd_buf,"mount /dev/sda1 /hqdata/hda1");
			}
			else
			#endif
				sprintf(cmd_buf,"mount %s%d %s%s%d",disks,j,MDPATH,get_sys_disk_name(i),j);	
			printf("开始mount,命令:%s\n",cmd_buf);
			errno=0;
			ret=system(cmd_buf);
			if(errno==0)			
			{
				//errrno=0-->system成功执行完，ret为system执行cmd_buf后，cmd_buf返回的值
				if((ret!=0))
				{	
					if(get_lfc_flag()==0)
					{
						//挂载失败，又是在出厂前，格式化
						gtlogerr("失败:[状态:出厂前]将%s%d分区挂载到%s%s%d失败，错误码:[%d],错误原因:%s,执行重新格式化硬盘\n",disks,j,MDPATH,get_sys_disk_name(i),j,ret,strerror(ret));
						system("/ip1004/initdisk -B 1");
					}
					else
					{
						gtlogerr("失败:[状态:出厂后]将%s%d分区挂载到%s%s%d失败，错误码:[%d],错误原因:%s\n",disks,j,MDPATH,get_sys_disk_name(i),j,ret,strerror(ret));
					}
				}
				else
				{
					printf("mount成功\n");
				}
			}
		}
		
	}

	return 0;
}

 
/**********************************************************************************************
 * 函数名   :check_db_file()
 * 功能 : 	检查数据库操作中是否断电，通过检查测试文件是否存在的方法，有的话删掉   
 * 输入  :                       
 * 输出  :       void        
 * 返回值:   void
 **********************************************************************************************/
int check_db_file(void)
{
        //int ret;
        int i;
        char tmp_file[64];
        char rm_cmd[128];

        for(i=0;i<4;i++)
        {
                memset(tmp_file,0,sizeof(tmp_file));
                memset(rm_cmd,0,sizeof(rm_cmd));
                sprintf(tmp_file,"%s%d%s","/hqdata/sda",i+1,"/creating_db");
                printf("检查文件%s是否存在?---",tmp_file);
                if(access(tmp_file,F_OK)==0)    //文件存在，删
                {
                        sprintf(rm_cmd,"%s%s","rm -f ",tmp_file);
                        printf("文件:%s存在，执行%s\n",tmp_file,rm_cmd);
                        system(rm_cmd);
                        memset(tmp_file,0,sizeof(tmp_file));
                        sprintf(tmp_file,"%s%d%s","/hqdata/sda",i+1,"/index.db");
                        if(access(tmp_file,F_OK)==0)
                        {
                                memset(rm_cmd,0,sizeof(rm_cmd));
                                sprintf(rm_cmd,"%s%s","rm -f ",tmp_file);
                                printf("文件:%s存在，执行%s\n",tmp_file,rm_cmd);
                                system(rm_cmd);
                        }
                }
                else    
                {
                        printf("不存在\n");
                }
        }

        return 0;
}

/**********************************************************************************************
* 函数名   :main()
* 功能  :       main
* 输入  :      			
* 输出  :       void        
* 返回值:   void
**********************************************************************************************/
int main(int argc,char *argv[])
{
	int lock_file=-1;
	int boot_flag;
	int ret;
	char pbuf[100];
	
	gtopenlog("init_all_ide_drvs");
	boot_flag=0;

	init_devinfo();
	
	boot_flag=env_parser(argc,argv);

	memset(pbuf,0,sizeof(pbuf));
	lock_file=create_and_lockfile(LOCK_FILE);
	if(lock_file<=0)
	{
		printf("initidealldrv are running!!\n");
		gtlogerr("initidealldrv 模块已运行，故启动无效退出\n");		
		exit(0);
	}
        sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
        write(lock_file,pbuf,strlen(pbuf)+1);

	printf("%s",pbuf);

	system("killall -9 watch_proc");
	system("killall -15 hdmodule");
	system("killall -9 diskman");//yk add 20130731

	ret=init_ide_drvs();
	if(ret<0)
	{
		close(lock_file);
		printf("initidealldrv 初始化ide设备失败,退出\n");
		gtlogerr("initidealldrv 初始化ide设备失败，退出\n");
		return -1;	
	}
	else
	{
		printf("initlostfd\n");
		gtloginfo("initlostfd\n");
		check_db_file();
		system("/ip1004/initlostfd");
		close(lock_file);
	}

        if(boot_flag ==1)
        {
                printf("swrbt\n");
                gtloginfo("swrbt\n");
		system("/ip1004/swrbt");                                                      //系统软复位
        }

	return 0;
}
