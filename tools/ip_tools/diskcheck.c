/*测试并修理硬盘或cf卡的程序，wsy,july@2006*/
#include <iniparser.h>
#include <commonlib.h>
#include <file_def.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <diskinfo.h>
#include "mpdisk.h"
#include "errno.h"
#include "devinfo.h"

#define  MOUNT_FILE ("/proc/mounts")

#if 0
#define DEVINFO_PARA_FILE   "/conf/devinfo.ini" //存放设备固定信息的文件，这些信息一般不会修改
#define HD_DEVICE_NODE      "/dev/hda"          //硬盘设备节点
#define HD_PART_NODE        "/dev/hda1"         //硬盘分区节点

int disk_format(char* disk_name)
{//应该放到库中
        int ret;
        long int cap;
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
                printf("没有检测到可用的CF卡!!\n");
                return 1;
        }
        else if((cap>=200)&&(cap<=2000))
        {
                printf("检测到可用的CF卡\n");
                printf("%s disk capacity = %ldM\n", disk_name, cap);
                ret = system("mke2fs /dev/hda1 -b 4096 -j -L hqdata -m 1 -i 65536\n");  //再格式化
                if(ret!=0)
                {
                        return 2;
                }
        }
        else            // if(cap[i]>2000)
        {
                printf("检测到可用的硬盘\n");
                printf("%s disk capacity = %ldG\n", disk_name, cap/1000);
                ret = system("mke2fs /dev/hda1 -b 4096 -j -L hqdata -m 1 -i 524288\n"); //再格式化
                if(ret!=0)
                {
                        return 2;
                }
        }
        return 0;
}

int format_disk(char *dev_name)//格式化磁盘
{
    int ret;
    if(dev_name == NULL)
        return -EINVAL;
    printf("开始格式化%s，如果长时间未完成，说明其已损坏不可修复\n",dev_name);
    //system("fdisk /dev/hda -f -a");
    //system("mke2fs -j /dev/hda1");
    ret=disk_format();

    return 0;
}

#endif

#define VERSION     "v1.03"
//v1.03 2010-06-25  zw      添加对当前分区的挂载情况的判断，避免重复挂载打印出来的警告信息
//v1.02 2009-08-10  wsy     diskinfo库支持sd卡
//v1.01 2009-06-10  wsy     增加版本号，纠正e2fsck路径，记录system返回值，格式化硬盘后强制硬重启等

static int formated_disk_flag = 0 ;//记录是否格式化过分区，若格式化过，必须硬重启

void set_cferr_flag(int flag)
{   
    return;
}


int kill_programs_and_umount(char *devname)
{
    char cmd[100];
    printf("终止应用程序并卸载磁盘%s..",devname);
    system("killall -9 tcpsvd 2> /dev/null ");
    system("killall -15 watch_proc 2>/dev/null");
    system("killall -15 hdmodule 2>/dev/null");
    system("killall -15 diskman 2>/dev/null");
    system("killall -9 sqlite3 2>/dev/null");
    system("killall -9 e2fsck 2>/dev/null");
    sleep(1);
    sprintf(cmd,"umount %s*",devname);  
    system(cmd);
    //printf("完毕.\n\n");
    gtloginfo("卸载磁盘,终止应用程序\n");
    return 0;
}   

////zw-add--->
/********************************************************
 *函数名:check_mounts()
 *输  入:mountpath  路径名
 *输  出:无
 *返回值:已经挂载放回1，否则返回0
 *备  注:无
 * ******************************************************/
int check_mounts(IN char *mountpath)
{
    char buff[200];
    char mount_dir[100];
    FILE *fp=NULL;
    char *str=NULL;
    char *p=NULL;
    int i;
    int mount_f=0;

    memset(buff,0,sizeof(buff));
    memset(mount_dir,0,sizeof(mount_dir));
    fp=fopen(MOUNT_FILE,"r");
    if(fp==NULL)
    {
        printf("open [%s] error,exit\n",MOUNT_FILE);
        return -1;
    }

    while(1)
    {
        str=fgets(buff,sizeof(buff),fp);
        if(str==NULL)
        {
                break;
        }
        else
        {
                //printf("读到的内容是:%s\n",str);
        }

        p=strstr(str,mountpath);
        if(p!=NULL)
        {
            //printf("已经加载目录:%s\n",p);
            mount_f=1;
        }
    }
    fclose(fp);

    if(mount_f==1)
        return 1;   

    return 0;
}   
////<--zw-add

int test_partition(IN char * devname, IN char * mountpath, IO void * fn_arg)
{
    char diskname[100];
    char cmd[200];
    char testfile[100];
    int total;
    char c;
    int i;
    int ret;
    int fix_time=0;//记录修理次数
    struct dirent **namelist;
    
    if((mountpath == NULL)||(devname == NULL))
        return -EINVAL;
        
test:
    //先判断是否已经挂载
    if(check_mounts(mountpath)==1)
    {
        //printf("分区[%s]已挂载[%s],卸载\n",devname,mountpath);
        kill_programs_and_umount(mountpath);    
    }
    
    //printf("挂载[%s]-->[%s]\n",devname,mountpath);    
    //printf("测试%s分区的读写scandir操作\n",partition_name);
    sprintf(cmd,"mount %s %s",devname,mountpath);
    system(cmd);
    
    
    //测试挂载正常
    if(get_disk_total(mountpath)<=1000)//<分区大小少于1G
    {
        printf("%s分区挂载不正常,需要修理\n",devname);
        gtloginfo("%s分区挂载不正常,需要修理\n",devname);
        goto fix;
    }
    
    //创建目录是否正常
    sprintf(testfile,"%s/indextest",mountpath);
    mkdir(testfile,0755);
    if (access(testfile,F_OK)!=0)
    {
        printf("\n%s分区无法创建文件，需要修理\n",devname);
        gtloginfo("%s分区无法创建文件，需要修理\n",devname);
        goto fix;
    }
    //scandir
    total=scandir(mountpath,&namelist,0,alphasort);
    i=total;
    while(total--)
        free(namelist[total]);
    free(namelist);
    if(i<3)//如果是-1说明不成功，如果<3说明没有找到刚建立的indextest,都不行
    {
        printf("\n%s分区scandir失败，需要修理\n",devname);
        gtloginfo("%s分区scandir失败，需要修理\n",devname);
        remove(testfile);
        goto fix;
    }
    //删除目录
    remove(testfile);
    if (access(testfile,F_OK)==0)
    {
        printf("\n%s分区无法删除文件，需要修理\n",devname);
        gtloginfo("%s分区无法删除文件，需要修理\n",devname);
        goto fix;
    }
    
    printf("%s分区通过初步测试!\n",devname);
    gtloginfo("%s分区通过初步测试\n",devname);
    return 0;
fix:
    switch(fix_time)
    {
        case(0):printf("用e2fsck修复分区%s\n",devname);
                gtloginfo("用e2fsck修复分区%s\n",devname);
                kill_programs_and_umount(devname);
                sprintf(cmd,"e2fsck -y -f %s",devname);
                ret = system(cmd);
                printf("修复%s完毕,结果为%d,重新测试\n",devname,ret);
                gtloginfo("修复%s完毕,结果为%d,重新测试\n",devname,ret);
                fix_time++;
                goto test;
                
        case(1):printf("%s分区修复无效，可以尝试格式化该分区损失分区内数据,确定吗?(y/N):\n",devname);
                gtloginfo("%s分区修复无效，可以尝试格式化该分区损失分区内数据,确定吗?(y/N):\n",devname);
                scanf("%c",&c); 
                if((c=='y')||(c=='Y'))
                {
                    gtloginfo("用户选择格式化%s,若长时间无响应说明失败无法修复\n",devname);
                    kill_programs_and_umount(devname);
                    sprintf(cmd,"mke2fs %s -b 4096 -j -L hqdata -m 1 -i 1048576 ",devname);
                    ret = system(cmd);
                    printf("格式化%s完毕，结果为%d,重新测试\n",devname,ret);
                    gtloginfo("格式化%s完毕，结果为%d,重新测试\n",devname,ret);
                    formated_disk_flag = 1;
                    fix_time++;
                    goto test;
                }
                else
                {
                    gtloginfo("用户决定不格式化%s\n",devname);
                    printf("!!!!!!!!!!%s分区的故障已无法修复!!!!!!!!\n",devname);
                    gtloginfo("%s分区的故障已无法修复\n",devname);
                    return 0;
                }
                
        case(2):printf("%s分区格式化后仍无效，是否重分区整个硬盘损失所有数据?(y/N):\n",devname);
                gtloginfo("%s分区格式化后仍无效，是否重分区整个硬盘损失所有数据?(y/N):\n",devname);
                scanf("%c",&c); 
                if((c=='y')||(c=='Y'))
                {
                    strncpy(diskname,devname,8);
                    diskname[8]='\0';
                    gtloginfo("用户选择重分区%s,若长时间无响应说明分区失败无法修复\n",diskname);
                    kill_programs_and_umount(diskname);
                    sprintf(cmd,"/ip1004/initdisk -d %s -B 1",diskname);
                    ret = system(cmd);
                    printf("重分区%s完毕,结果为%d,重新测试\n",diskname,ret);
                    gtloginfo("重分区%s完毕,结果为%d,重新测试\n",diskname,ret);
                    fix_time++;
                    goto test;
                }
                else
                {
                    gtloginfo("用户决定不分区%s\n",diskname);
                    printf("!!!!!!!!%s分区的故障已无法修复!!!!!!!!\n",devname);
                    gtloginfo("%s分区的故障已无法修复\n",devname);
                    return 0;
                }   
            
        case(3): 
                printf("!!!!!!!!%s分区的故障已无法修复!!!!!!!!!!\n",devname);
                gtloginfo("%s分区的故障已无法修复\n",devname);
                return 0;
        default: return 0;
    
    
    
    
    
    }
    
    
}

int test_all_partitions(int partition_num)
{
    init_devinfo();
    if(get_devtype()<T_GTVS3021) //gt1k
    {
            return test_partition("/dev/sda1","/hqdata",NULL);
    }
    else    //多个分区
        return mpdisk_process_all_partitions(test_partition,NULL);
}

int main(void)
{
    int partition_num;
    int diskno,i;

    
    init_devinfo();
    gtopenlog("testcf");
    gtloginfo("##########开始执行%s磁盘修复程序! version %s##########\n\n",get_devtype_str(),VERSION);
    printf("\n##########开始执行%s磁盘修复程序! version %s##########\n\n",get_devtype_str(),VERSION);
    printf("注意!若因磁盘故障而执行本程序，请确保最近已硬重启过而仍有磁盘故障\n\n");
    
    printf("第1步. 检查硬盘节点:");
    diskno = get_sys_disk_num();
    if(diskno==0)
    {
        printf("\n\n错误!! 没有任何硬盘节点，硬盘无法修复\n");
        gtloginfo("没有硬盘节点,硬盘无法修复\n");
    
        return -1;
    }
    else
    {
        printf("找到%d个硬盘节点\n\n",diskno);
        gtloginfo("找到%d个硬盘节点\n",diskno);
        
    }
    
    printf("第2步. 检查磁盘类型,容量,分区数和状态: \n\n");
    for(i=0;i<diskno;i++)
    {
        partition_num = get_sys_disk_partition_num(get_sys_disk_devname(i));
        printf("磁盘%s,有%d个分区，总容量%ldG\n",get_sys_disk_devname(i),partition_num,get_sys_disk_capacity(get_sys_disk_name(i))/1000);
        gtloginfo("磁盘%s,有%d个分区，总容量%ldG\n",get_sys_disk_devname(i),partition_num,get_sys_disk_capacity(get_sys_disk_name(i))/1000);
        if(partition_num == 0)
        {
            
            printf("无可用分区，磁盘%s无法修复,建议用initdisk格式化并分区\n",get_sys_disk_devname(i));
            gtloginfo("无可用分区，磁盘%s无法修复，建议用initdisk格式化并分区\n",get_sys_disk_devname(i));
        }
    }
    
    printf("\n第3步. 对所有可用分区进行读写测试..\n");
    
    test_all_partitions(partition_num);

    if(formated_disk_flag == 1) //格式化了分区，必须重起才能使用    
    {
        printf("完毕。如果设备短时间内再次报磁盘错误，也说明设备已无法远程修复。\n\n");
        printf("\n检查结束，因格式化了分区，执行硬重启..");
        gtloginfo("检查结束，因格式化了分区，执行硬重启\n");
        system("/ip1004/hwrbt");
    }
    else
    {
        printf("\n检查结束，重起应用程序..");
        gtloginfo("重起应用程序\n");
        system("tcpsvd -vE 0.0.0.0 21 ftpd /hqdata/ &");
        system("/ip1004/watch_proc > /dev/null 2>/dev/null &");
        gtloginfo("检查结束\n");    
    }
    return 0;

}
