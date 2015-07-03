#include <file_def.h>
#include <dirent.h>
#include <ftw.h>
#include <errno.h>
#include <sys/time.h>
#include <pwd.h>
#include "devinfo.h"
#include <mod_cmd.h>
#include "diskmanager.h"
#include <iniparser.h>
#include "process_modcmd.h"
#include "commonlib.h"
#include <sys/time.h>
#include <devinfo.h>
#include "fileindex.h"
#include <sys/types.h>
#include <dirent.h>
#include "gt_errlist.h"
#include "filelib.h"
#include "hdutil.h"
#include "fixdisk.h"
#include "diskinfo.h"
#include "process_modcmd.h"

#define SYSINFO_DUMP_FILE "/var/diskmaninfo.txt" //进程信息文件


#define  MAX_PARTITION_NUM   16
static int auto_unlock_time; //磁盘满后自动解锁的时间间隔,配置文件中以小时为单位,参数结构中以秒为单位
static int hderr_reboot_enable; //磁盘有故障是否允许重启，从配置文件读取,默认为1

static struct diskman_state_struct diskman_state={0,0,0};
static int fulllock=0;//是否log了"可能加锁文件过多"
static int diskfull_counter[MAX_PARTITION_NUM]={0}; //磁盘满等待解锁进阶计数器
static pthread_t diskman_thread_id=-1;



int get_hd_rmval(void)
{
    if(get_hd_type()==1)
        return HD_RMVAL;
    else
        return CF_RMVAL;
}


DWORD get_diskmanstatint(void)
{
    DWORD stat;
    memcpy((void*)&stat,(void*)&diskman_state,sizeof(DWORD));
    return stat;
}

//把系统参数打印到指定文件
void dump_sysinfo(void) 
{
    //wsy 
    struct timeval tv;
    struct tm *ptime;
    time_t ctime;
    FILE *dfp;
    char *filename=SYSINFO_DUMP_FILE;
    dfp=fopen(filename,"w");
    if(dfp==NULL)
        return ;
    fprintf(dfp,"\tdiskman system runtime info\n\n");

    if(gettimeofday(&tv,NULL)<0)
    {
        fprintf(dfp,"\t获取系统时间时出错\n");
    }
    else
    {
        ctime=tv.tv_sec;
        ptime=localtime(&ctime);
        if(ptime!=NULL)
        {
            fprintf(dfp,"\ttime:%d-%d-%d %d:%d:%d.%03d\n\n",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min,ptime->tm_sec,(int)tv.tv_usec/1000);    
        }
    }

    //打印相关参数
    fprintf(dfp,"diskman_state_struct\n{\n\tcf_err\t\t= %d\n\tdisk_full\t= %d\n}\n",diskman_state.cf_err,diskman_state.disk_full);
    fprintf(dfp,"\nauto_unlock_time\t= %d //磁盘满后自动解锁的时间间隔\n",auto_unlock_time);
    fprintf(dfp,"\nfulllock\t\t= %d //是否log了'可能加锁文件过多'\n",fulllock);
    fprintf(dfp,"\ndiskfull_counter[0]\t= %d     //磁盘满等待解锁进阶计数器\n",diskfull_counter[0]);
    fprintf(dfp,"\ndiskfull_counter[1]\t= %d     //磁盘满等待解锁进阶计数器\n",diskfull_counter[1]);
    fprintf(dfp,"\ndiskfull_counter[2]\t= %d     //磁盘满等待解锁进阶计数器\n",diskfull_counter[2]);
    fprintf(dfp,"\ndiskfull_counter[3]\t= %d     //磁盘满等待解锁进阶计数器\n",diskfull_counter[3]);
    
    fclose(dfp);
    return;
}

//返回文件名对应的文件录像时间(整数表示)
int get_fname_time(char*filename)
{
    char *p,*t;
    char tmp;
    time_t filetime;
    struct tm ftime;
    if(filename==NULL)
        return -1;
    p=strstr(filename,"_D");
    if(p==NULL)
        return -1;
    p+=2;

    t=p+4;
    tmp=*t;
    *t='\0';
    ftime.tm_year=atoi(p)-1900;
    *t=tmp;

    p=t;
    t+=2;
    tmp=*t;
    *t='\0';
    ftime.tm_mon=atoi(p)-1;
    *t=tmp;

    p=t;
    t+=2;
    tmp=*t;
    *t='\0';
    ftime.tm_mday=atoi(p);
    *t=tmp;
    
    p=t;
    t+=2;
    tmp=*t;
    *t='\0';
    ftime.tm_hour=atoi(p);
    *t=tmp;


    p=t;
    t+=2;
    tmp=*t;
    *t='\0';
    ftime.tm_min=atoi(p);
    *t=tmp;


    p=t;
    t+=2;
    tmp=*t;
    *t='\0';
    ftime.tm_sec=atoi(p);
    *t=tmp;

    filetime=mktime(&ftime);
    return filetime;
    
}

void set_cferr_flag(int flag)
{   
    if(get_ide_flag()==0) //没有CF卡
        return;
    
    if(diskman_state.cf_err!=flag)
    {
        diskman_state.cf_err=flag;
        send_state2main();
    }
}



void set_disk_full_flag(int flag)
{
    if(get_ide_flag()==0)
        return;
    
    if(diskman_state.disk_full!=flag)
    {
        diskman_state.disk_full=flag;
        send_state2main();
    }
    return ;
}


//被scandir调用用来确保不将query产生的.txt记入目录或文件结构
int notxt (const struct dirent *dir) 
{
    if((strstr(dir->d_name,"up")!=NULL)||(strstr(dir->d_name,"index")!=NULL)||(strstr(dir->d_name,"lost")!=NULL)||(strstr(dir->d_name,"iframe")!=NULL))
        return 0;
    else 
        return 1;
}

int disk_full(char *mountpath, int partitionindex) //磁盘满而且不能删除时调用
{
    set_disk_full_flag(1);

    if(++diskfull_counter[partitionindex] >= (auto_unlock_time/DISK_MAN_LOOP)) //应解锁
    {
        diskfull_counter[partitionindex] = 0;
        fileindex_lock_by_time(mountpath,0, -1,-1,-1,-1);
        gtloginfo("磁盘满%d秒,自动解锁所有文件\n",auto_unlock_time);
    }
    
    return 0;
}

//返回文件名代表的录像长度
int get_length_file(char *path)
{
    char *lp;
    int result;
    if(path==NULL)
        return -1;
    lp=strstr(path,"_L");
    if(lp==NULL)
        return -1;
    lp=lp+2;
    result= atoi(lp);
    //gtloginfo("长度%d,lp%s\n",result,lp);
    return result;  
}



int lockfile_in_partition(IN  char *devname, IN char* mountpath, IO void *fn_arg)
{
    struct lockfile_struct * lockfile;
    
    if((mountpath == NULL)||(fn_arg == NULL))
        return -EINVAL;
        
    lockfile = (struct lockfile_struct *) fn_arg;

    fileindex_lock_by_time(mountpath, lockfile->mode,lockfile->start,lockfile->stop,lockfile->trig,lockfile->ch);
    return 0;
}

int lock_file_by_time(time_t start_time, time_t stop_time,int mode ,int channel)
{
    struct lockfile_struct lockfile;
    
    lockfile.mode   =   mode;
    lockfile.start  =   start_time;
    lockfile.stop   =   stop_time;
    lockfile.trig   =   -1;
    lockfile.ch     =   channel;
    
    return mpdisk_process_all_partitions(lockfile_in_partition,&lockfile);

}

/*************************************************
函数名称：usr_lock_file_time
简要描述：按时段加解锁文件，被模块命令接收函数调用，
          将传来的结构解析,把时间转化为秒数赋给全局变量
          将通道按位解析调用lock_file_by_time函数
输入：    操作路径path,加解锁文件结构指针lockfile
输出：    返回值为调用具体加解锁函数的执行结果  
修改日志：wsy@Jan11,2006创建
*************************************************/
int usr_lock_file_time(char *path, struct usr_lock_file_time_struct *lockfile)
{
    struct gt_time_struct *start; //起始时间
    struct gt_time_struct *stop;  //结束时间
    struct tm timestart;
    struct tm timestop;
    int i;
    WORD ch;
    int mode;
    int result = 0;
    time_t start_timet,stop_timet;
    char mountpath[32];
    
    if( (path==NULL) || (lockfile==NULL) )
    {
        return -1;
    }
    start = &(lockfile->start);
    stop = &(lockfile->stop);
    ch = (WORD)lockfile->lock_ch;
    mode = lockfile->mode;
    
    gttime2tm(start,&timestart);
    gttime2tm(stop,&timestop);
    start_timet = mktime(&timestart);
    stop_timet = mktime(&timestop);
    
    for(i = 0; i < 4/*MAX_RECORD_CHANNEL*/; i++)
    {
        if((ch & 1<<i) == 1)
        {
            if(start_timet >= stop_timet)
            {
                if(mode==1)
                {
                    gtloginfo("起始时间>=结束时间，%d通道全锁\n",ch);
                }
                else
                {
                    gtloginfo("起始时间>=结束时间，%d通道全解锁\n",ch);
                    
                }
                //result = hdutil_lock_all_files(mode);
                sprintf(mountpath,"/hqdata/sda%d",i+1);
                result = fileindex_lock_by_time(mountpath,mode, -1,-1,-1,i);
                
            }   
            else
            {
                //result = lock_file_by_time( start_timet, stop_timet,mode,i);
                sprintf(mountpath,"/hqdata/sda%d",i+1);
                result = fileindex_lock_by_time(mountpath,mode,start_timet, stop_timet,-1,i);
            }
        
        }
    }
    return result;
}

//删除最老的mpg等文件夹文件,path:路径,number:一次删除的数目
int remove_oldest_smallfiles(char *path,int number)
{
    
    int total,i,ret,no=0;
    struct dirent **namelist;
    char filename[100];


    
    mkdir(path,0755);
    if((path==NULL)||(access(path,F_OK|R_OK|W_OK|X_OK)!=0)||(isdir(path)!=0))
    {
        gtloginfo("%s不存在或无法access,不进行scandir,也无法完成磁盘整理操作",path);
        return -1;
    }
  //Acturally,the files are NOT deleted as the sort of the Creatting-Time but based on the order of their names.Whatever,some files will be deleted.
    total=scandir(path,&namelist,0,alphasort);
    if(total==-1)
    {
            gtloginfo("文件系统scandir %s 出错,%s",path,strerror(errno));
            return -1;
    }
    for(i=0;i<number;i++)
    {
    
        sprintf(filename,"%s/%s",path,namelist[i+2]->d_name);
        ret=remove(filename);
        if(ret==0)
            no++;
    }
    while(total--)
        free(namelist[total]);
    free(namelist);
    return no;
}



//从指定分区中删除最老的文件，到磁盘容量足够为止,并顺便把空目录也给清了
//磁盘容量足够则返回0，否则返回负值
int remove_oldest_from_partition(IN char *partition)
{
    int occupied = 0;
    int disk_free;
    int rm_stop;
    
    if(partition == NULL)
        return -EINVAL;
    
    occupied = get_disk_free(partition);
    if(occupied<0)
    {
        gtloginfo("[%s:%d]获取磁盘剩余空间失败\n",__FILE__,__LINE__,occupied);
        return -1;
    }
    printf("remove_oldest_from_partition,get_disk_free:%d\n",occupied);
   
    if(get_hd_type()==1)//HD
    {
        rm_stop =   HD_RM_STOP;
    }
    else    
    {
        rm_stop =   CF_RM_STOP;
    }
        
    while(1)
    {
        fileindex_del_oldest(partition,10);
        disk_free = get_disk_free(partition);
        printf("del_oldest 10 files, get_disk_free:%d\n",disk_free);
        
        if((occupied == disk_free)||(disk_free >= rm_stop))//删无可删，or 已经删够
            break;
        else
            occupied = disk_free;
    }
    
    //删除所有空目录
    ftw_sort(partition, NULL,500,FTW_SORT_ALPHA,1,fix_disk_ftw);//删除所有空目录

    if(get_disk_free(partition)>=rm_stop)
        return 0;
    else
        return -EPERM;
}

#if 0
//删除系统中的最老文件,
//策略为,根据索引文件判断最老的分区;然后在最老的分区下根据索引文件进行删除,并清理空目录
int remove_oldest_file()
{   
    int ret;
    char oldest_partition[100];
    int disktotal=0;//从proc读取当前硬盘的总容量
    int parttotal=0;//从get_disk_total读取当前所有分区的总容量
    int i;
        
    ret = hdutil_get_oldest_file_partition(oldest_partition); //寻找最老的可删除文件所在分区
    if(ret!= 0)
    {
        gtloginfo("没有找到最老的分区,ret = %d\n",ret);
        if(ret == -ENOENT)
        {
            if(hderr_reboot_enable)
            {
                if(hdutil_get_hderr_reboot_flag()== 1)
                {
                    gtlogerr("磁盘满但访问分区索引失败,20秒后重启系统以图修复\n");
                    sleep(20);
                    system("/ip1004/hwrbt &");
                }
                else
                {
                    set_cferr_flag(1);
                    printf("磁盘满但访问分区索引失败,重启太多次，暂时不再重启\n");
                }
            }
            else
            {
                set_cferr_flag(1);
                gtloginfo("磁盘满但访问分区索引失败,配置文件不允许重启\n");
            }
        }
        else
        {
                set_cferr_flag(1);
        }
        return 0;
    }
    printf("[%s:%d]找到最老文件所在分区为[%s]\n",__FILE__,__LINE__,oldest_partition);
    ret = remove_oldest_from_partition(oldest_partition);
    if(ret!= 0) //仍然删除不够
    {   
        disk_full(); 
        if(fulllock==0)
        {
            gtloginfo("返回%d,已扫描所有目录并删除但空间不足(%s分区空余%d M/共%d M)，可能加锁文件过多?\n",ret,oldest_partition,get_disk_free(oldest_partition),get_disk_total(oldest_partition));   
            fulllock=1;
        }
        //检查此时是否是硬盘都被卸载的情况
        for(i=0;i<get_sys_disk_num();i++)
        {
            disktotal+= get_sys_disk_capacity(get_sys_disk_name(i));    
        }
        parttotal=mpdisk_get_sys_disktotal();
        
        
        if((disktotal>=1000)&&(parttotal<1000))
        {
            if(hderr_reboot_enable)
            {
                if(hdutil_get_hderr_reboot_flag()== 1)
                {
                    gtlogerr("所有硬盘分区已被卸载,所有挂载点空间%dM,20秒后重启以图修复\n",parttotal);
                    sleep(20);
                    system("/ip1004/hwrbt &");
                }
                else
                {
                    set_cferr_flag(1);
                    printf("所有硬盘分区已被卸载,所有挂载点空间%dM,重启太多次不再重启\n",parttotal);
                }
            }
            else
            {
                set_cferr_flag(1);
                gtloginfo("所有硬盘分区已被卸载,所有挂载点空间%dM,配置文件不允许重启\n",parttotal);
                
            }
        }
        
    }   
    else
    {
        fulllock=0;
        set_disk_full_flag(0);
    }
    
    return 0;
 }
#endif







//初始化磁盘管理线程用到的变量
int init_diskman(void)
{
    char path[100];
    
    memset((void*)&diskman_state,0,sizeof(diskman_state));

    //如果没有索引文件目录，则创建
    sprintf(path,"%s/index",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    //如果没有抓图索引目录，则创建
    sprintf(path,"%s/picindex",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    //如果没有抓图存储目录，则创建
    sprintf(path,"%s/iframe",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    //如果没有升级目录，则创建
    sprintf(path,"%s/update",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);

    send_state2main();
    return 0;
}



/*written by wsy,2006 June,给定/hqdata下的目录，返回每次删除的文件数*/
int get_remove_number(char *dirname)
{
    if(strcmp(dirname,MPGDIR)==0)
        { 
#ifdef SNAP_TO_AVIFILE   
            return RM_MPG_ONCE*2;
#else
            return RM_MPG_ONCE;
#endif
        }
    if(strcmp(dirname,PICINDEXDIR)==0)
            return RM_PICINDEX_ONCE;
    if(strcmp(dirname,INDEXDIR)==0)
            return RM_INDEX_ONCE;
    if(strcmp(dirname,UPDATEDIR)==0)
            return RM_UPDATE_ONCE;
    return 0;
}

/*written by wsy,2006 June,给定/hqdata下的目录，返回这个目录下的最大文件数*/
int get_filenumber_limit(char *dirname)
{
    if(strcmp(dirname,MPGDIR)==0)
        { 
#ifdef SNAP_TO_AVIFILE   
            return MPGNUMBER*2;
#else
            //return MPGNUMBER; //zw-modified-2011-06-16
            return MPGNUMBER*2;
#endif
        }
    if(strcmp(dirname,PICINDEXDIR)==0)
            return PICINDEXNUMBER;
    if(strcmp(dirname,INDEXDIR)==0)
            return INDEXNUMBER;
    if(strcmp(dirname,UPDATEDIR)==0)
            return UPDATENUMBER;

    return 0;
}

/*written by wsy,2006 June,检查清理/hqdata下的指定目录*/
void manage_dir(char *dirname)
{
    int number=0;
    int ret=0;
    struct dirent **namelist;


    //加上如果没有该目录就创建该目录
    if(access(dirname,F_OK)!=0)
        mkdir(dirname,0755);
    number=scandir(dirname,&namelist,0,alphasort);
    if(number==-1)
    {
        gtloginfo("文件系统scandir %s 出错,%s",dirname,strerror(errno));
        return;
    }
    if((number-2)>get_filenumber_limit(dirname))
    {   
        ret=remove_oldest_smallfiles(dirname,get_remove_number(dirname));
#ifdef SHOW_WORK_INFO
        printf("remove oldest %s %d",dirname,ret);
#endif
    }
    while(number--) 
        free(namelist[number]);
    free(namelist);
    return;
}

//被check_disk_status_fn调用，如果发现硬盘df和du不符，就重启
//返回0表示成功，负值表示失败
int check_disk_status_fn(IN  char * devname, IN  char * mountpath, IO void *arg)
{
    int disktotal,diskfree;
    int dirtotal;
    struct dirent **namelist;

    disktotal   = get_disk_total(mountpath);
    diskfree    = get_disk_free(mountpath);
    
    if(disktotal-diskfree > 1024)   //占用超过1G
    {
        dirtotal=scandir(mountpath,&namelist,0,alphasort);
        if(dirtotal == 0)
        {
                      //当scandir()出错时的处理,当前的处理是重启TODO...
            if(hderr_reboot_enable)
            {
                if(hdutil_get_hderr_reboot_flag()== 1)
                {
                    gtlogerr("%s分区占用%dM/%dM,但无目录节点,20秒后重启以图修复\n",mountpath,disktotal-diskfree,disktotal);
                    sleep(20);
                    system("/ip1004/hwrbt &");
                }
                else
                {   
                    while(dirtotal--)
                        free(namelist[dirtotal]);
                    free(namelist);
                    set_cferr_flag(1);
                    printf("%s分区占用%dM/%dM,但无目录节点,已重启太多次不再重启\n",mountpath,disktotal-diskfree,disktotal);
                }
            }
            else
            {
                while(dirtotal--)
                            free(namelist[dirtotal]);
                free(namelist);
                set_cferr_flag(1);
                gtloginfo("%s分区占用%dM/%dM,但无目录节点,配置文件不允许重启\n",mountpath,disktotal-diskfree,disktotal);
            }
        }
        else
        {
            while(dirtotal--)
                free(namelist[dirtotal]);
            free(namelist);
        }
    }
    return 0;

}



//负责管理磁盘的线程
static void *diskman_thread(void *para)
{
    long freespace;
    struct passwd *user;
    int loguid=-1;
    int ret;
    long disktotal = 0;

    gtloginfo("start diskman_thread...\n");
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);     //设置线程为可取消的
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    
    user=getpwnam("gtlog");
    if(user!=NULL)
        loguid=user->pw_uid;

    if(access("/hqdata/update",R_OK|W_OK|F_OK)!=0)
        mkdir("/hqdata/update",0755);

    while(1)
    {
              //下面4行将对各自的目录进行维护，包括检测文件是否存在，文件个数是否超出限制值，超出则删除
        //检查清理hqdata/iframe下的mpg和avi文件
        manage_dir(MPGDIR);
        //检查清理/hqdata/picindex下的txt文件
        manage_dir(PICINDEXDIR);
        //检查清理/hqdata/update下的升级记录
        manage_dir(UPDATEDIR);
        //检查清理/hqdata/index下的txt文件
        manage_dir(INDEXDIR);   
    
              //其实就是对每个分区使用scandir()查看一下，看scandir()是否会出错，如果出错了就根据配置参数重启设备    
        mpdisk_check_disk_status(&check_disk_status_fn);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
        int i;
        int j;
        char partitionname[100];
        char mount_path[100];
        int parttotal;

        //检查此时是否是硬盘都被卸载的情况
        for(i=0;i<get_sys_disk_num();i++)
        {
            disktotal+= get_sys_disk_capacity(get_sys_disk_name(i));    
        }
        parttotal=mpdisk_get_sys_disktotal();

        if((disktotal>=1000)&&(parttotal<1000))
        {
            if(hderr_reboot_enable)
            {
                if(hdutil_get_hderr_reboot_flag()== 1)
                {
                    gtlogerr("所有硬盘分区已被卸载,所有挂载点空间%dM,20秒后重启以图修复\n",parttotal);
                    sleep(20);
                    system("/ip1004/hwrbt &");
                }
                else
                {
                    set_cferr_flag(1);
                    printf("所有硬盘分区已被卸载,所有挂载点空间%dM,重启太多次不再重启\n",parttotal);
                }
            }
            else
            {
                set_cferr_flag(1);
                gtloginfo("所有硬盘分区已被卸载,所有挂载点空间%dM,配置文件不允许重启\n",parttotal);
                
            }
        }
        
 
        for(j=1;j<=get_sys_partition_num(get_sys_disk_name(0));j++)
        {
            memset(partitionname,0,sizeof(partitionname));
            memset(mount_path,0,sizeof(mount_path));
            get_sys_disk_partition_name(0,j, partitionname);
            partitionname2mountpath(partitionname,mount_path);

           long partition_free= 0;
	    partition_free = get_disk_free(mount_path);


            if(partition_free < get_hd_rmval()) //磁盘空间不足
            {   
                ret = remove_oldest_from_partition(mount_path);
                if(ret!= 0) //仍然删除不够
                {   
                    disk_full(mount_path, j); 
                    if(fulllock==0)
                    {
                        gtloginfo("返回%d,已扫描所有目录并删除但空间不足(%s分区空余%d M/共%d M)，可能加锁文件过多?\n",ret,mount_path,get_disk_free(mount_path),get_disk_total(mount_path));   
                        fulllock=1;
                    }
                }   
                else
                {
                    fulllock=0;
                    set_disk_full_flag(0);
                }
                     
            }
            else 
            {
                set_disk_full_flag(0);
                diskfull_counter[j]=0;
            }
        }


        sleep(DISK_MAN_LOOP); //cancelation

    }

    return NULL;
}

int creat_diskman_thread(pthread_attr_t *attr,void *arg)
{
    return pthread_create(&diskman_thread_id,attr, diskman_thread, arg);//创建磁盘管理线程
}

int read_diskman_para_file(char *filename)
{

    int val;
    dictionary      *ini;
    
    if(filename==NULL)
        return -1;
    
    ini=iniparser_load(filename);
    if(ini==NULL)
    {
        printf("diskman  cannot parse ini file file [%s]", filename);
        gtloginfo("从配置文件%s中load ini失败,返回-1\n",filename);
        return -1 ;
    }

    val=iniparser_getint(ini,"product:auto_unlock_time",24);
    val*=3600;
    auto_unlock_time=val;


    hderr_reboot_enable = iniparser_getint(ini,"product:hderr_reboot_enable",1);

    
    iniparser_freedict(ini);
    return 0;
}
//#endif

