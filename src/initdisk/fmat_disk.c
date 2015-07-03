#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <file_def.h>
#include <string.h>
#include "errno.h"
#include <sys/ioctl.h>

#include <diskinfo.h>
#include <devinfo.h>
#include "fmat_disk.h"

#define FMAT_OK		300
#define NO_DISK		301
#define NO_HDA1		302
#define MOUNT_ERR	303
#define FDISK_ERR	304
#define FMAT_ERR	305

// 格式化磁盘失败错误信息

static const char* format_OK	=	"磁盘格式化成功";
static const char* format_err	=	"格式化磁盘失败";
static const char* no_disk		=	"没有硬盘或cf卡";
static const char* no_hda1		=	"找不到/dev/hda1节点";
static const char* mount_err	=	"加载磁盘失败";
static const char* fdisk_err	=	"磁盘分区失败";

/*设置当前的录像分区*/
void set_record_partition(char *partition_name)
{

    dictionary    *ini=NULL;
    FILE            *fp=NULL;

    ini=iniparser_load_lockfile(DISK_INI_FILE,1,&fp);
    if(ini==NULL)
    {
          printf("set record partition,cannot parse ini file [%s]\n", DISK_INI_FILE);
          gtlogerr("init_devinfo() cannot parse ini file [%s]", DISK_INI_FILE);
          return;
    }

    gtloginfo("格式化后将录像分区设为%s\n",partition_name);
    //当前录像分区设为sda1
    iniparser_setstr(ini, "diskinfo:record_disk", partition_name);
    save_inidict_file(DISK_INI_FILE,ini,&fp);
    if(fp!=NULL)
    {
        unlock_file(fileno(fp));
        fsync(fileno(fp));
        fclose(fp);
    }

    iniparser_freedict(ini);

}
/*
*****************************************************
*函数名称: disk_format
*函数功能: 格式化磁盘 按照磁盘的容量选择不同的格式化参数
*输入：char* disk_name 磁盘名称
*输出：
返回值:0 成功，1表示没有磁盘，2表示格式化失败
*修改日志：
*****************************************************
*/ 
int disk_format(char* disk_name)
{
	int ret;
	long int cap;
	char cmd[200];
	int i;
	
	cap = get_sys_disk_capacity(disk_name);
	if (cap<=0)
	{
		printf("找不到%s IDE 设备\n", disk_name);
		return 1;
	}
	printf("找到  %s IDE设备\n", disk_name);
		
	if(cap<200)
	{
		printf("%s capacity = %ld less than 200M \n", disk_name, cap);
		printf("没有检测到可用的存储设备!!\n");
		return 1;
	}
	else 
	{
		printf("检测到可用的硬盘%s disk capacity = %ldG\n", disk_name, cap/1000);
		for(i=1;i<= get_sys_partition_num(disk_name);i++)
		{
			sprintf(cmd, "/sbin/mke2fs -T ext3 /dev/%s%d -b 4096 -j -L hqdata%d -m 1 -i 1048576 ",disk_name,i,i);/*yk change ext3*/
			ret = system(cmd);       //wsy, 1M /node
			if(ret!=0)
			{
				return 2;
			}
			//wsyadd tune2fs
			sprintf(cmd,"/sbin/tune2fs -i 0 -c 0 /dev/%s%d\n",disk_name,i);
			ret = system(cmd);     
			if(ret!=0)
			{
				return 2;
			}
			sprintf(cmd,"/sbin/tune2fs -m 1 /dev/%s%d\n",disk_name,i);
			ret = system(cmd);       
			if(ret!=0)
			{
				return 2;
			}
			
		}
	}
	return 0;
}

//格式化硬盘为单分区,供gt1k选用
int init_ide_drv_single_partition(multicast_sock *ns , FILE *fp, char *diskname, int diskno, int disknum)
{
	
    int ret = -1;
    long int cap=0;
    unsigned char buf[200];
    
	// format disk
    memset(buf,0,sizeof(buf));
    ret = system("fdisk /dev/hda -f -a \n");        //先分区
    if(ret!=0)
    {
            fprintf(fp, "%d:%s,",FDISK_ERR, fdisk_err);
            send_test_report(ns, "磁盘分区失败", 80);// lsk 2007 -6-1
            result_report(FDISK_ERR, ns);// lsk 2007 -6-1
            goto endpoint;
    }
    send_test_report(ns, "磁盘分区成功", 30);
    // 按照磁盘的大小进行格式化
 	memset(buf,0,sizeof(buf));
    send_test_report(ns, "格式化硬盘开始...", 40);
    ret = disk_format("hda");       //格式化磁盘
    if(ret!=0)
    {
            fprintf(fp, "%d:%s,",FMAT_ERR, format_err);
            sprintf(buf, "%s",format_err);
            send_test_report(ns, buf, 90);
            result_report(FMAT_ERR, ns);// lsk 2007 -6-1
            goto endpoint;
    }
    memset(buf,0,sizeof(buf));
    send_test_report(ns, "磁盘格式化成功", 80);
//判断有无hda1节点
    memset(buf,0,sizeof(buf));
    if(access("/dev/hda1",F_OK)!=0)
    {
            fprintf(fp, "%d:%s,",NO_HDA1, no_hda1);
            sprintf(buf, "%s",no_hda1);
            send_test_report(ns, buf, 90);
            result_report(NO_HDA1, ns);// lsk 2007 -6-1
            goto endpoint;
    }

// mount disk
    memset(buf,0,sizeof(buf));
    ret = system("mount /dev/hda1 /hqdata\n");
    printf("mount /dev/hda1 /hqdata ret =%d \n", ret);
    if(ret!=0)
    {
            fprintf(fp, "%d:%s,",MOUNT_ERR, mount_err);
            sprintf(buf, "%s",mount_err);
            send_test_report(ns, buf, 90);
            result_report(MOUNT_ERR, ns);// lsk 2007 -6-1
            goto endpoint;
    }
    fprintf(fp, "%d:%s,",FMAT_OK, format_OK);
    memset(buf,0,sizeof(buf));
    sprintf(buf, "%s",format_OK);
    send_test_report(ns, buf, 80);


    system("rm -rf /hqdata/update");
    system("mkdir /hqdata/update");
    system("mount -t tmpfs none /hqdata/update");
// lsk 2006 -12-15
//      cap = get_hd_capacity();
    memset(buf,0,sizeof(buf));
    cap = get_sys_partition_capacity("hda", 1);
 if((cap<0)||(cap<200))
    {
            fprintf(fp, "%d:%s,",NO_DISK, no_disk);
//              sprintf(buf, "%s",NO_DISK);
            result_report(NO_DISK, ns);// lsk 2007 -6-1
            goto endpoint;
    }
    if(cap>2000)
    {
            fprintf(fp, "磁盘容量:%ldGB,", cap/1000);
            sprintf(buf, "磁盘容量:%ldGB", cap/1000);
    }
    else
    {
            fprintf(fp, "磁盘容量:%ldMB,",cap);
            sprintf(buf, "磁盘容量:%ldMB", cap);
    }
    send_test_report(ns, buf, 90);
    ret=0;
    gtloginfo("fmatdisk succeed\n");
endpoint:
    if(ret)
    {
    	gtlogerr("fmatdisk failed\n");
		return ret;
    }	
    send_test_report(ns, "format disk finished", 100);
    result_report(0, ns);
	ret = 0;
//lsk 2007 -6-1
//      result_report(FORMAT_FLAG , ns);        //发送结果
	return ret;
}

int init_ide_drv_multi_partition(multicast_sock *ns , FILE *fp, char *diskname, int diskno, int disknum)
{
	unsigned char buf[200];
	char cmd[200];
	char partition2_name[100];
	char disk_node[100];
	int ret;
	char devname[20];
	long cap;
	
	if((fp==NULL)||(ns==NULL)||(diskname==NULL))
		return -EINVAL;
	
	memset(buf,0,sizeof(buf));
/*yk del 20130705 fdisk can't use this way*/
#if 0
	sprintf(cmd,"/sbin/fdisk %s -f -p ",diskname);
	ret = system(cmd);	//先分区
	sprintf(cmd,"/sbin/fdisk %s -f ",diskname);
	ret = system(cmd);	//先分区
	sprintf(cmd,"/sbin/fdisk %s -f -p ",diskname);
	ret = system(cmd);	//先分区
	sprintf(cmd,"/sbin/fdisk %s -f -p ",diskname);
	ret = system(cmd);	//先分区
#endif
	long long  tmp_cap;//硬盘大小G
	tmp_cap=get_disk_capacity("sda");
	printf("the cap is:%lld G\n",tmp_cap);
	/*删除所有分区*/
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nd\n1\nw\n","sda");
	ret = system(cmd);	//先分区
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nd\n2\nw\n","sda");
	ret = system(cmd);	//先分区
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nd\n3\nw\n","sda");
	ret = system(cmd);	//先分区
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nd\n4\nw\n","sda");
	ret = system(cmd);	//先分区
	


	printf("\n\n\n\n\n");
	sleep(1);
	//exit(1);
	/*有这个命令把硬盘分成四个区*/
	memset(buf,0,sizeof(buf));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nn\np\n1\n1\n+%lldG\nw\n",\ 
					"sda",tmp_cap/4);
	ret = system(cmd);
	memset(buf,0,sizeof(buf));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nn\np\n2\n\n+%lldG\nw\n",\ 
					"sda",tmp_cap/4);
	ret = system(cmd);
	memset(buf,0,sizeof(buf));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nn\np\n3\n\n+%lldG\nw\n",\ 
					"sda",tmp_cap/4);
	ret = system(cmd);
	memset(buf,0,sizeof(buf));
	sprintf(cmd,"/sbin/fdisk /dev/%s<<EOF\nn\np\n4\n\n\n\nw\n",\ 
					"sda");
	ret = system(cmd);



	//wsy,以上是对于fdisk的workaround， to be fixed
	
	sprintf(partition2_name,"%s2",diskname);
	printf("partition2_name:%s\n",diskname);
	if((ret!=0)||(access(partition2_name,F_OK)!=0))
	{	
	    sprintf(buf,"%d:磁盘%s分区失败\n",FDISK_ERR,diskname);
		fprintf(fp, buf);
		printf("%s",buf);
		
		send_test_report(ns,buf, 90);// lsk 2007 -6-1
		result_report(FDISK_ERR, ns);// lsk 2007 -6-1
		gtlogerr("%s",buf);
		return -1;
	}
	else
	{
		sprintf(buf,"磁盘%s分区成功\n",diskname);
		send_test_report(ns, buf,15+20*(diskno+1)/disknum);
		printf("%s",buf);
		gtloginfo("%s",buf);
	}
// 按照磁盘的大小进行格式化
	memset(buf,0,sizeof(buf));
	sprintf(buf,"磁盘%s格式化开始...\n",diskname);
	send_test_report(ns, buf, 15+40*(diskno+1)/disknum);
	gtloginfo("%s",buf);
	printf("debug:%s\n",buf);
	strncpy(disk_node,diskname+5,4);
	ret = disk_format(disk_node);	//格式化磁盘
	if(ret!=0)
	{
		gtloginfo("disk_format!!!\n");
		fprintf(fp, "%s-%d:%s,",diskname,FMAT_ERR, format_err);
		sprintf(buf, "%s-%s",diskname,format_err);
		send_test_report(ns, buf, 90);
		result_report(FMAT_ERR, ns);// lsk 2007 -6-1
		return -1;
	}
	memset(buf,0,sizeof(buf));
	sprintf(buf,"磁盘%s格式化成功\n",diskname);
	send_test_report(ns,buf,15+60*(diskno+1)/disknum);
	fprintf(fp,buf);
	gtloginfo("%s",buf);
	printf("debug:%s",buf);
#if 0
//判断有无hda1节点
	memset(buf,0,sizeof(buf));
	if(access("/dev/hda1",F_OK)!=0)
 	{
		fprintf(fp, "%d:%s,",NO_HDA1, no_hda1);
		sprintf(buf, "%s",no_hda1);
		send_test_report(ns, buf, 90);
		result_report(NO_HDA1, ns);// lsk 2007 -6-1
		goto endpoint;
 	}
#endif
// lsk 2006 -12-15
//	cap = get_hd_capacity();
	memset(buf,0,sizeof(buf));
	strcpy(devname,strstr(diskname,"sd"));/*yk change hd->sd 20130708*/
	gtloginfo("devname is %s, %s",devname,diskname);
	cap = get_sys_disk_capacity(devname);//get_sys_partition_capacity(get_sys_disk_name(i), 1);
	if(cap<200)                
	{
		sprintf(buf, "%d:磁盘%s无法读出容量\n", NO_DISK,diskname);	
		fprintf(fp,buf);
		gtlogerr("%s",buf);
		result_report(NO_DISK, ns);// lsk 2007 -6-1
		return -1;	
	}
	else
	{
		sprintf(buf, "磁盘%s 容量:%ldGB\n", diskname,cap/1000);	
		gtloginfo("%s",buf);
	}
	fprintf(fp,buf);
	send_test_report(ns, buf, 15+80*(diskno+1)/disknum);
	ret=0;
	gtloginfo("fmatdisk %s succeed\n",diskname);
	
	return 0;


}

/*****************************************************
*函数名称: init_ide_drv
*函数功能: 格式化单个硬盘
*输入:	multicast_sock *ns 网络参数数据结构
*		fp: 存储格式化结果的文件的指针
*		diskname: 需要格式化的磁盘名，形如/dev/hda
*		diskno:该磁盘的编号，0到disknum-1
*		disknum:系统一共有的磁盘个数
*输出：
返回值:0 成功，其他表示失败
*修改日志：
*****************************************************/


int init_ide_drv(multicast_sock *ns , FILE *fp, char *diskname, int diskno, int disknum)
{
	if((fp==NULL)||(ns==NULL)||(diskname==NULL))
		return -EINVAL;
	
	init_devinfo();
	
	//if(strstr(get_devtype_str(),"IP1004")!=NULL) //IP1004系列 YK CHANGE 20130708
	if(1)
	{
		return init_ide_drv_multi_partition(ns,fp,diskname,diskno,disknum);
	}	
	else
	{
		return init_ide_drv_single_partition(ns,fp,diskname,diskno,disknum);
	}

}

/****************************************************
 *函数名称: init_sd_drv
 *函数功能: SD卡分区
 *输    入: multicast_sock *ns 网络参数数据结构
 *输    出:
 *返    回: 无
 ****************************************************/
void init_sd_drv(multicast_sock *ns, char *diskname)
{
	char diskcmd[100];
	int ret=0;
	
	//检查节点是否存在
	//开始给SD卡分区，就分一个区
	memset(diskcmd,0,sizeof(diskcmd));
	sprintf(diskcmd,"fdisk %s -f a",diskname);
	printf("给sd卡分区的命令是[%s]\n",diskcmd);
	ret=system(diskcmd);

	#if USE_SD
	//在这个版本中，sd的驱动里将sd的节点名改为/dev/hda，
	//所以不用再创建符号连接了
	if(access("/dev/sda1",F_OK)==0)
	{
		memset(diskcmd,0,sizeof(diskcmd));
		////sprintf(diskcmd,"ln -s /dev/cpesda1 /dev/hda1");
		sprintf(diskcmd,"ln -s /dev/sda1 /dev/hda1");
		printf("分区后执行:%s\n",diskcmd);
		ret=system(diskcmd);
	}
	else
	{
		printf("sd卡分区错误，没有/dev/sda1\n");
		gtlogerr("sd卡分区错误，没有/dev/sda1");
		return ;
	}
	#endif

	//开始格式化	
	memset(diskcmd,0,sizeof(diskcmd));
	sprintf(diskcmd,"mke2fs -j %s1",diskname);	
	printf("给sd卡格式化的命令是[%s]\n",diskcmd);
	ret=system(diskcmd);
	
}

/*
 * 将测试结果数据结构解析后存入指定的文件
 */
/*
*****************************************************
*函数名称: format_dev_disk
*函数功能: 格式化磁盘线程
*输入： multicast_sock *ns 网络参数数据结构
*		
*输出：
*修改日志：
*****************************************************
*/ 
void format_dev_disk(multicast_sock *ns , unsigned char*file_name, int init_all_flag, char *diskname)
{
	int i;
	int ret = -1;
//	long int cap=0;
	int disknum = 0; 
	FILE *fp = NULL;
	unsigned char buf[200];
//	char cmd[200];
	memset(buf,0,sizeof(buf));
	gtopenlog("initdisk");
// 打开临时记录文件
	fp = fopen(file_name,"w");
	if(fp==NULL)
	{
		printf("can not open %s\n", file_name);
		sprintf(buf, "can not open %s",file_name);
		send_test_report(ns, buf, 80);
		return ;
	}
	send_test_report(ns, "开始initdisk格式化硬盘", 10);
	gtloginfo("start format disk"); // lsk 2007 -4 -27
//在文件中记录格式化磁盘的信息
	fprintf(fp,"[%s]\n",FMAT_NODE);
	fprintf(fp,"%s=", REPORT);
	fflush(fp);
	
//杀死相关进程

	system("killall -9 watch_proc \n");
	system("killall -15 hdmodule\n");
	system("killall -9 diskman\n");
	system("killall -9 tcpsvd\n");
	system("killall -9 e2fsck\n");
	system("killall -9 playback\n");
	sleep(2);

//卸载磁盘
	//if(strstr(get_devtype_str(),"GTIP1004")!=NULL||strstr(get_devtype_str(),"GTIP2004")!=NULL)//gtvs3k系列  //yk change gtvs3k->GTIP1004
		system("umount /hqdata/sd*");
/*
	else
	{
		system("umount /hqdata/update");
		system("umount /hqdata");
	}
*/

//判断有无硬盘
	memset(buf,0,sizeof(buf));
	
	ret=get_ide_flag();

	if(ret==1)	//装的是硬盘
	{
		printf("准备获取磁盘个数...\n");
		disknum = get_sys_disk_num();
		if(disknum==0)
		{
			gtlogerr("no disk\n");
			fprintf(fp, "%d:%s,",NO_DISK, no_disk);
			result_report(NO_DISK, ns);// lsk 2007 -6-1
			goto endpoint;
		}
		sprintf(buf,"find %d disk(s)\n",disknum);
		gtloginfo("%s",buf);
		send_test_report(ns, buf, 15);//20);
		fprintf(fp,buf);

		printf("系统磁盘个数为[%d]\n",disknum);
	
		if(init_all_flag == 1)//格式化所有存在的硬盘
		{
			gtloginfo("debug:init_all_flag=1\n");
			for(i=0;i<disknum;i++)
			{
				// 分区
				ret=init_ide_drv(ns,fp,get_sys_disk_devname(i),i,disknum);
			}
		}
		else//格式化指定的硬盘
		{
			gtloginfo("debug:init_all_flag!=1    sdfa\n");
			ret = init_ide_drv(ns,fp,diskname,0,1);
		}
endpoint:
		if(ret)
		{	
		gtlogerr("fmatdisk failed\n");
		}
		else
		{	
			send_test_report(ns, "格式化硬盘成功结束", 100);
			result_report(0, ns);
			gtloginfo("fmatdisk succeed\n");
			set_record_partition("sda1");/*设置sda1为当前的录像分区*/
		}
	}
	else if((ret==2)||(ret==3))
		{
			//装的是SD卡
			//ret=3时装的是TF卡 zw-add 2010-07-12
			init_sd_drv(ns,get_sys_disk_devname(0));
			
		}
	else if(ret==0)
		{
			printf("获取设备磁盘标志错误,ret=[%d]\n",ret);
		}
	
	fclose(fp);
}



