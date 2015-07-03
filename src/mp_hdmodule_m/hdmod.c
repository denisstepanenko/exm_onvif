#include "mp_hdmodule.h"
#include "mpdisk.h"
#include <gt_errlist.h>
#include <math.h>
#include "hdutil.h"
#include "hdmod.h"
#include "process_modcmd.h"
#include <commonlib.h>
#include <devinfo.h>
#include <venc_read.h>
#include <avilib.h>
#include <aenc_read.h>
#include "audiofmt.h"
#include "filelib.h"
#include "diskinfo.h"
#include "fixdisk.h"
#include "ftw.h"
#include "fileindex.h"

////zw-test
#include <sys/time.h>
#include <unistd.h>
////zw-test


#ifdef RECORD_PS_FILE
#include "669/es2ps.h"
#endif 

#ifdef USE_FFMPEG_LIB
    //读取音频格式
static int audio_save_fmt=1;                        ///音频存储格式 1:u-pcm 3:raw-pcm 4:mpeg2
#endif
//static int CFflag=1;//是否有CF卡
static struct hd_enc_struct g_hdenc[MAX_HQCHANNEL];   //第0路高清晰录像结构
static struct hdmod_state_struct hdmod_state={0,0,0};//,0,0,0};

static int hd_playback_flag=0;

int get_hd_minval(void)
{
    if(get_hd_type()==1)
        return HD_MINVAL;
    else
        return CF_MINVAL;
}

/**********************************************************************************************
 * 函数名   :disk_get_record_partition()
 * 功能 :获取录像分区
 * 输入 :无
  * 输出 :partition_name:当前录像的硬盘分区名
 * 返回值   :0表示正常，负值表示出错
 * 注       :应用程序在刚启动的时候需要调用这个函数从/conf/diskinfo.ini中读取系统信息
 *          如果/conf/diskinfo.ini不存在，则把设备信息设置成初始值，并返回-1
  **********************************************************************************************/
int disk_get_record_partition(struct hd_enc_struct *phdenc)
{

    dictionary    *ini=NULL;
    char            *pstr=NULL;
    FILE            *fp=NULL;
    
    ini=iniparser_load_lockfile(DISK_INI_FILE,1,&fp);
    if(ini==NULL)
    {
          printf("disk_get_record_partition() cannot parse ini file file [%s]", DISK_INI_FILE);
          gtlogerr("disk_get_record_partition() cannot parse ini file file [%s]", DISK_INI_FILE);
          return -1 ;
    }


    //当前录像盘
    pstr = iniparser_getstring(ini,"diskinfo:record_disk",NULL);
    if(pstr != NULL)
    {
        if((strlen(pstr) == 4) && (pstr[0] == 's') && (pstr[1] == 'd'))//必须是sdXX,如sda1
        {
            strcpy(phdenc->partition,"/hqdata/");
            strncat(phdenc->partition,pstr,strlen("sda1"));   
        }
        else
        {
            gtlogerr("get record_disk error:%s,len:%d , set to sda1\n",pstr,strlen(pstr));
            strcpy(phdenc->partition,"/hqdata/sda1");
        }
    }
    else
    {
        gtloginfo("no record_disk, set to sda1\n");
        strcpy(phdenc->partition,"/hqdata/sda1");
    }
    printf("record disk =%s\n",phdenc->partition);
  
    if(fp!=NULL)
    {
        unlock_file(fileno(fp));
        fsync(fileno(fp));
        fclose(fp);
    }   

    iniparser_freedict(ini);
    
    return 0;
    
}

/**********************************************************************************************
 * 函数名	:is_keyframe()
 * 功能	:判断一帧数据是否是关键帧,本函数在文件切割时会用到
 * 输入	:frame 需要被判断的数据帧
 * 返回值	:0 表示非关键侦
 *			 1表示是关键侦
 **********************************************************************************************/
int is_keyframe(struct stream_fmt_struct *frame)
{
	if((frame->media==MEDIA_VIDEO)&&(frame->type==FRAMETYPE_I))
		return 1;
	return 0;
}


struct pool_head_struct *get_stream_pool(int channel)
{
        return &(g_hdenc[channel].streampool);
}
struct hdmod_state_struct *get_hdmod_stat(void)
{
    return &hdmod_state;
}
static int old_cferr_state=0;

void set_cferr_flag(int flag)
{   

    if(get_ide_flag()==0)//没有存储
        return;
    
    if(old_cferr_state!=flag)
    {
        //gtloginfo("test,flag is %d,old is %d\n",flag,old_cferr_state);
        hdmod_state.cf_err=flag;
        send_state2main();
        old_cferr_state=flag;
    }
}



//将文件信息转换成相应的文件名
int finfo2filename(struct file_info_struct *info,char *filename)//wsy,若有标志则length按切分长度
{
    char *name;
    struct tm *ptime;
    if((info==NULL)||(filename==NULL))  
        return -1;
    ptime=localtime(&info->stime);
    if(ptime==NULL)
        return -1;  
    name=filename;
    
    
    if(info->remote==1)
    {
        //sprintf(name,"%s/%04d/%02d/%02d/%02d/HQ_C%02d_D%04d%02d%02d%02d%02d%02d_L%02d_T%02X%c%c%s",
        //wsy changed to support multi-partition
        sprintf(name,"%s/%04d/%02d/%02d/%02d/HQ_C%02d_D%04d%02d%02d%02d%02d%02d_L%02d_T%02X%c%c%s",
        info->partition,
        (1900+ptime->tm_year),
        ptime->tm_mon+1,
        ptime->tm_mday,
        ptime->tm_hour,
        info->channel,
        (1900+ptime->tm_year),
        ptime->tm_mon+1,
        ptime->tm_mday,
        ptime->tm_hour,
        ptime->tm_min,
        ptime->tm_sec,
        info->len,
        info->trig,
        REMOTE_TRIG_FLAG,
        LOCK_FILE_FLAG,
        IMG_FILE_EXT);
    }
    else
    {
        if(info->trig)
            {
                sprintf(name,"%s/%04d/%02d/%02d/%02d/HQ_C%02d_D%04d%02d%02d%02d%02d%02d_L%02d_T%02X%c%s",
                info->partition,
                (1900+ptime->tm_year),
                ptime->tm_mon+1,
                ptime->tm_mday,
                ptime->tm_hour,
                info->channel,
                (1900+ptime->tm_year),
                ptime->tm_mon+1,
                ptime->tm_mday,
                ptime->tm_hour,
                ptime->tm_min,
                ptime->tm_sec,
                info->len,
                info->trig,
                LOCK_FILE_FLAG,
                IMG_FILE_EXT);
            }
        else
            {
                sprintf(name,"%s/%04d/%02d/%02d/%02d/HQ_C%02d_D%04d%02d%02d%02d%02d%02d_L%02d_T%02X%s",
                info->partition,
                (1900+ptime->tm_year),
                ptime->tm_mon+1,
                ptime->tm_mday,
                ptime->tm_hour,
                info->channel,
                (1900+ptime->tm_year),
                ptime->tm_mon+1,
                ptime->tm_mday,
                ptime->tm_hour,
                ptime->tm_min,
                ptime->tm_sec,
                info->len,
                info->trig,
                IMG_FILE_EXT);
            }
    }
    
    return 0;   
    
}


/**********************************************************************************************
 * 函数名   :disk_get_next_partition(char* partition_name)
 * 功能 :获取下一个分区
 * 输入 :partition_name:当前分区名，类似/hqdata/sda1
  * 输出 :partition_name:下一个分区名
 * 返回值   :0表示正常，负值表示出错
  **********************************************************************************************/
int disk_get_next_partition(char* partition_name)
{

    char        diskno; /*硬盘号*/
    int           partitionnamelen;
    int           disknum;
    char        partitionno;/*分区号*/

    partitionnamelen = strlen(partition_name);
    if(partitionnamelen < strlen("/hqdata/sda1"))
    {
          printf("disk_get_next_partition() partition_name:%s error\n",partition_name);
          gtlogerr("disk_get_next_partition() partition_name:%s error",partition_name);
          return -1;
     }
        

    disknum = get_sys_disk_num();

    /*硬盘号sda....sdd*/
    diskno = partition_name[10];  

    /*分区号sda1....sda3....sdb1...sdb4.....*/
    partitionno = partition_name[11];

    partitionno++;
    if(partitionno > '4')
    {
        partitionno = '1';
        diskno++;
        if(diskno -'a' >=  disknum)
        {
           diskno = 'a';
        }
    }

    /*修改分区*/    
    /*硬盘号sda....sdd*/
    partition_name[10] = diskno;  

    /*分区号sda1....sda3....sdb1...sdb4.....*/
    partition_name[11] = partitionno;
    

    return 0;    

      
}
/**********************************************************************************************
 * 函数名   :disk_get_next_record_partition(char* partition_name)
 * 功能 :获取下一个录像分区
 * 输入 :partition_name:当前录像分区
 * 输出 :partition_name:下一个录像分区名
 * 返回值   :0表示正常，负值表示出错
  **********************************************************************************************/
int disk_get_next_record_partition(char* partition_name)
{

    dictionary    *ini=NULL;
    char            *pstr=NULL;
    FILE            *fp=NULL;
    int               seachpartition = 0;/*查找的分区数*/
    int               disknum;


    gtloginfo("%s disk free is %dM,chang to next disk\n",partition_name,get_disk_free(partition_name));
    
    disknum = get_sys_disk_num();
    while(get_disk_free(partition_name) < get_hd_minval())
    {

        //printf("current disk:%s\n",partition_name);
        disk_get_next_partition(partition_name);
        //printf("current disk:%s\n",partition_name);

        seachpartition++;
        /*分区总数是disknum*4，下面判断是遍历了所有的分区*/
        if(seachpartition >= disknum*4)
        {
            printf("disk is full,record disk is not find,seach partition:%d\n",seachpartition);
            gtlogerr("disk is full,record disk is not find,seach partition:%d\n",seachpartition);
            return -1;
        }
        
    }

    gtloginfo("current disk is full ,chang to next disk:%s,disk free:%dM\n",partition_name,get_disk_free(partition_name) );
    ini=iniparser_load_lockfile(DISK_INI_FILE,1,&fp);
    if(ini==NULL)
    {
          printf("init_devinfo() cannot parse ini file file [%s]\n", DISK_INI_FILE);
          gtlogerr("init_devinfo() cannot parse ini file file [%s]", DISK_INI_FILE);
          return -1 ;
    }

    //当前录像盘
    pstr=strstr(partition_name,"sd");
    iniparser_setstr(ini, "diskinfo:record_disk", pstr);
    save_inidict_file(DISK_INI_FILE,ini,&fp);
    if(fp!=NULL)
    {
        unlock_file(fileno(fp));
        fsync(fileno(fp));
        fclose(fp);
    }   

    iniparser_freedict(ini);
    return 0;
    
}



//获取整数表示的hdmod状态
DWORD get_hdmodstatint(void)
{
    DWORD stat;
    memcpy((void*)&stat,(void*)&hdmod_state,sizeof(DWORD));
    return stat;
}
//removed by shixin static int hdsave_mode[HQCHANNEL]={1};//高清晰录像工作模式 1表示需要正常工作 0表示需要停止
//返回指定通道应有的工作模式
// 1表示需要正常工作 0表示需要停止
// 改变工作状态的工作是在启动和停止高清晰录像线程的时候做的
int get_hqsave_mode(int ch)
{//changed by shixin
    struct hd_enc_struct    *hd=NULL;
    hd=get_hdch(ch);
    if(hd==NULL)
        return 0;
    return hd->hdsave_mode;
}



int convert_old_ing_files_fn(IN char * devname, IN char * mountpath, IO void *arg)
{   
    if(mountpath == NULL)
        return -EINVAL;
    printf("[%s:%d],path:%s\n",__FILE__,__LINE__,mountpath);    
    return  fileindex_convert_ing(mountpath);
}



int convert_old_ing_files(void)
{
  //在每个分区调用convert_old_ing_files_fn(),在每个分区都转换
    return mpdisk_process_all_partitions(convert_old_ing_files_fn,NULL);
}



/*
    函数名：  dump_clearinfo_to_log
    函数功能: 把收到的clear_hdmod_trig_flag信息打印到指定文件
    编写时间: wsy@Jan.2006
*/
void dump_clearinfo_to_log(void)
{
    struct timeval tv;
    struct tm *ptime;
    time_t ctime;
    struct stat buf;
    FILE *dfp;
    
    char *filename = ALARMLOG_FILE;

    //判断文件是否过大
    stat(ALARMLOG_FILE,&buf);
    if( (buf.st_size>>10) >= ALARMLOGFILE_MAX_SIZE )    //超过一定长度
    {
        remove(ALARMLOG_FILE_0);
        rename(ALARMLOG_FILE,ALARMLOG_FILE_0);
    }
    
    dfp = fopen(filename,"a+");
    if(dfp == NULL)
        return ;
    //写时间
    if(gettimeofday(&tv,NULL) < 0)
    {
        fprintf(dfp,"<获取系统时间时出错 >   ");
    }
    else
    {
        ctime = tv.tv_sec;
        ptime = localtime(&ctime);
        if(ptime != NULL)
            fprintf(dfp,"<%04d-%02d-%02d %02d:%02d:%02d>   ",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min,ptime->tm_sec);    
    }
    gtloginfo("清除trig标志准备写信息\n");
    //写信息
    fprintf(dfp,"[CLEAR]            \n");
    fclose(dfp);

    
}

/*
    函数名：  dump_hqsaveinfo_to_log
    函数功能: 把收到的hqsave/hqstop信息打印到指定文件,mode=1为save
    编写时间: wsy@Jan.2006
*/
void dump_saveinfo_to_log(int mode,int ch,int time)
{
    struct timeval tv;
    struct tm *ptime;
    time_t ctime;
    struct stat buf;
    FILE *dfp;
    
    char *filename = ALARMLOG_FILE;

    //判断文件是否过大
    stat(ALARMLOG_FILE,&buf);
    if( (buf.st_size>>10) >= ALARMLOGFILE_MAX_SIZE )    //超过一定长度
    {
        remove(ALARMLOG_FILE_0);
        rename(ALARMLOG_FILE,ALARMLOG_FILE_0);
    }
    
    dfp = fopen(filename,"a+");
    if(dfp == NULL)
        return ;
    //写时间
    if(gettimeofday(&tv,NULL) < 0)
    {
        fprintf(dfp,"<获取系统时间时出错 >   ");
    }
    else
    {
        ctime = tv.tv_sec;
        ptime = localtime(&ctime);
        if(ptime != NULL)
            fprintf(dfp,"<%04d-%02d-%02d %02d:%02d:%02d>   ",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min,ptime->tm_sec);    
    }
    //写信息
    if(mode==1)
        fprintf(dfp,"[SAVE]  %05d秒    \n",time);
    if(mode==0)
        fprintf(dfp,"[STOP]             \n");
    fclose(dfp);
}

/*
    函数名:   check_alarmlog_for_trigflag
    函数功能: 重起时调用,检查alarmlog中的clear和alarm条目
    返回值:   trigflag,若为0表示已被clear,若为1表示最后一条是报警
              alarm_log_struct结构  
    编写时间: wsy@Jan.2006
*/
int check_alarmlog_for_trigflag(struct alarm_log_struct *log)
{
    FILE *stream;
    int i = 0;
    int trigflag = 0; 
    int row = 1;
    
    //打开文件读到本地缓冲区中
    stream = fopen(ALARMLOG_FILE,"r");

    if(stream == NULL)
        return 0;
    
    while(i >= 0) //直到读不到完整的行为止
    {
        i = fseek(stream,-row*sizeof(struct alarm_log_struct),SEEK_END);
        fread(log,sizeof(struct alarm_log_struct),1,stream);
        if(strncmp(log->type,"[CLEAR]",7) == 0) //这一条是CLEAR
        {
            break;
        }
        if(strncmp(log->type,"[ALARM]",7) == 0) //这一条是ALARM
        {
            trigflag = 1;
            break;
        }
        row++;
    }

    fclose(stream);
    return trigflag; 
}





int check_alarmlog_for_hqsaveflag(struct alarm_log_struct *log)
{
    FILE *stream;
    int i=0;
    int row=1;
    int saveflag=0; //若有未stop的save则置为1
    
    //打开文件读到本地缓冲区中
    stream=fopen(ALARMLOG_FILE,"r");

    if(stream==NULL)
        return 0;
    while(i>=0) //直到读不到完整的行为止
    {
        i=fseek(stream,-row*sizeof(struct alarm_log_struct),SEEK_END);
        fread(log,sizeof(struct alarm_log_struct),1,stream);
        if(strncmp(log->type,"[STOP] ",7)==0) //这一条是STOP
        {
            break;
        }
        if(strncmp(log->type,"[CLEAR]",7)==0) //这一条是CLEAR
        {
            break;
        }
        if(strncmp(log->type,"[SAVE] ",7)==0) //这一条是SAVE
        {
            saveflag=1;
            break;
        }
        row++;
    }

    fclose(stream);
    return saveflag; 
}

    
/*
    函数功能:从alarm_log结构中计算出续录指向的时间
*/
int get_record_time(struct alarm_log_struct *info)
{
    char timeinfo[24];
    char *lp,*lk;

    struct tm timetrig; //报警时间的tm结构
    
    if(info==NULL)
        return -1;
        
    strncpy(timeinfo,info->time,23);
    timeinfo[23]='\0';
    //printf("time is %s\n",time);

    //计算年份
    lp=index(timeinfo,'<');
    lp++;
    timetrig.tm_year=atoi(lp)-1900;
    //printf("year is %04d\n",timetrig.tm_year);

    //计算月份
    lk=index(lp,'-');
    lk++;
    timetrig.tm_mon=atoi(lk)-1;
    //printf("month is %02d\n",timetrig.tm_mon);

    //计算日期
    lp=index(lk,'-');
    lp++;
    timetrig.tm_mday=atoi(lp);
    //printf("date is %02d\n",timetrig.tm_mday);

    //计算小时差别
    lk=index(lp,' ');
    lk++;
    timetrig.tm_hour=atoi(lk);
    //printf("hour is %02d\n",timetrig.tm_hour);

    //计算分钟差别
    lp=index(lk,':');
    lp++;
    timetrig.tm_min=atoi(lp);
    //printf("min is %02d\n",min);

    //计算秒数
    lk=index(lp,':');
    lk++;
    timetrig.tm_sec=atoi(lk);
    //printf("sec is %02d\n",sec);

    return mktime(&timetrig);

}
#if 0
//将字符串转换为16进制数字
DWORD atohex(char *buffer)
{
    int i,len;
    DWORD hex=0;
    char *p;
    DWORD ret;
    char ch;
    char buf[12];
    if(buffer==NULL)
        return 0;
    memcpy(buf,buffer,sizeof(buf));
    p=strrchr(buf,'x');//找到最后一个x的位置
    if(p==NULL)
    {
        p=strrchr(buf,'X');
    }   
    if(p==NULL)
        p=buf;
    else
        p++;
    len=strlen(p);

    if(len>8)
    {
        i=len-8;
        p+=i;
        len=strlen(p);  
    }
    p+=len;
    p--;
    for(i=0;i<len;i++)
    {
        ch=(char)toupper((int)*p);
        *p=ch;
        if(isdigit(*p))
        {
            ret=*p-'0';
        }
        else //字母
        {
            //if(!isupper(*p))
            *p=(char)toupper((int)*p);
            ret=*p-'A'+10;
        }
        hex|=ret<<(i*4);
        p--;
    }
    return hex; 
    
}
#endif



//
int get_hqsave_time(struct alarm_log_struct *info)
{
    char data[12];
    int time;
    
    if(info==NULL)
        return -1;

    strncpy(data,info->data,11);
    data[11] = '\0';
    time=(atoi(data));
    //gtloginfo("time is %d测试\n",time);
    return time;
}



/*
    函数名称: get_record_ch
    函数功能: 从alarm_log结构中计算出还需要续录的触发
    函数输入: 从alarmlog中读出的最后一条alarm_log结构
    返回值:   触发状态
    编写时间: wsy@Jan.2006
*/
int get_record_ch(struct alarm_log_struct *info)
{
    char data[12];
    DWORD trig;

    if(info==NULL)
        return -1;

    strncpy(data,info->data,11);
    data[11] = '\0';
    trig=(DWORD)(atohex(data));
    //loginfo("ch is 0x%04x测试\n",(int)trig);
    return trig;
}


/*
    函数名称: get_trig_status
    函数功能: 初始化后调用，检查alarmlog中是否需要续录报警录像并进行相应设置
    函数输入: 空
    返回值:   空
    说明:因为3022的报警只有虚拟设备0有，故直接对应视频通道0
    编写时间: wsy@Jan.2006
*/
void get_trig_status(void)
{
    struct alarm_log_struct log;
    struct hd_enc_struct *hd_enc;
    int trig=0;
    int rec_len=0; //报警的绝对时间
    int trig_ch=0;
    time_t timenow; 
    
    trig=check_alarmlog_for_trigflag(&log);
    if(trig!=0)
    {
        //取出时间
        rec_len=get_record_time(&log);
        hd_enc=get_hdch(0);
        if(hd_enc==NULL)
        return;

        timenow=time((time_t *)NULL);
        if(rec_len>timenow)//如果报警时间比现在时间还后，则返回
            return;//这是为了避免出现电池故障引起的症状。。
            
        //gtloginfo("reclen+hd_enc->dly_rec-timenow=%d\n",(int)(rec_len+hd_enc->dly_rec-timenow));
        if(rec_len+hd_enc->dly_rec>timenow) //若需要继续录和加锁，解析出trig
        {
            trig_ch=get_record_ch(&log);
            hd_enc=get_hdch(0);
            hd_enc->state=2;
            hd_enc->trig=trig_ch;
            hd_enc->recordlen=rec_len+hd_enc->dly_rec-timenow;
            gtloginfo("重起前0x%04x报警录像还要续录%d秒\n",trig_ch,(int)(rec_len+hd_enc->dly_rec-timenow));
            return;
        }
    } 
    
    
    return;
}


/*
    函数名称: get_save_status
    函数功能: 初始化后调用，检查alarmlog中是否需要续录手动录像并进行相应设置
    函数输入: 空
    返回值:   空
    说明:因为3022的报警只有虚拟设备0有，故直接对应视频通道0
    编写时间: wsy@Jan.2006
*/
void get_save_status(void)
{
    struct alarm_log_struct log;
    struct hd_enc_struct *hd_enc;
    int save=0;
    int rec_len=0; 
    int rec_time=0;
    time_t timenow;
    
    save=check_alarmlog_for_hqsaveflag(&log);
    if(save!=0)
    {
        hd_enc=get_hdch(0);
        if(hd_enc==NULL)
        return;

        timenow=time((time_t *)NULL);

        
        //比较时间+录像时间和现在时间,计算出还需要录的时间
        rec_len=get_record_time(&log);
        if(rec_len>timenow)//如果报警时间比现在时间还后，则返回
            return;//这是为了避免出现电池故障引起的症状。。
        rec_time=get_hqsave_time(&log);
        if(rec_len+rec_time>timenow)    //若需要继续录和加锁，解析出CH
        {
            
            hd_enc=get_hdch(0);
            hd_enc->state=2;
            hd_enc->remote_trigged=1;
            hd_enc->remote_trig_time=rec_time+rec_len-timenow;
            gtloginfo("重起前手动录像还要续录%d秒\n",(int)(rec_time+rec_len-timenow));
            return;
        }
    } 
    return;
}







//清除指定通道的trig标志
int clear_hdmod_trig_flag(int channel)
{

    struct hd_enc_struct *hd;
    
    gtloginfo("清除%d通道trig标志\n",channel);
    
    hd=get_hdch(channel);
    
    hd->trig=0;
    hd->recordlen=0;
    if(hd->remote_trigged==0)
    {
        if((hd->rec_type==0)&&(hd->pre_rec!=0))
            hd->state=1;
        else
            hd->state=0;
    }
	else
		hd->cutflag=1;
    return 0;
}


//将文件名加上手工录像标志
int rtflag_filename(char *filename,char* tname)
{
    char *p;
    
    if((filename==NULL)||(tname==NULL))
        return -1;
    
    strncpy(tname,filename,strlen(filename)+1);
    p=index(tname,REMOTE_TRIG_FLAG);
    if(p!=NULL)
    {
        return 0;
        //已加标志不能再加  
    }   
    p=index(tname,LOCK_FILE_FLAG);
    if(p!=NULL)
    {
        //文件已经加锁
        sprintf(p,"%c%c%s",REMOTE_TRIG_FLAG,LOCK_FILE_FLAG,IMG_FILE_EXT);
        return 0;
    }
    p=strstr(tname,IMG_FILE_EXT);
    if(p==NULL)
    {
        //正在录像的文件不能被加标志
        return 0;
    }
    sprintf(p,"%c%s",REMOTE_TRIG_FLAG,IMG_FILE_EXT);
    return 0;   
}



int set_msgmnb_value(int queueid,int msgnum)
{

    struct msqid_ds buf;
    

    if (msgnum<115)
        return 0;
    
    if(msgctl(queueid,IPC_STAT,&buf)<0)
    {
        gtloginfo("无法取得消息队列的结构\n");
        return -1;
    }
    //gtloginfo("test,buf.msg_qbytes=%ld\n",buf.msg_qbytes);
    buf.msg_qbytes=msgnum*128;
    
    if(msgctl(queueid,IPC_SET,&buf)<0)
    {
        gtloginfo("无法增加消息队列的长度\n");
        return -1;
    }
    gtloginfo("消息队列长度增加为%ld\n",buf.msg_qbytes);
    
    return 0;
    

}


//从配置文件中读取高清晰录像相关参数
int read_hqsave_para_file(char *filename,char *section, int channel)
{
    char parastr[100];
    dictionary      *ini;   
    char *cat;
    int section_len;
    int msgnum=0;
    char sec[100];
    struct hd_enc_struct *hdenc = get_hdch(channel);
    FILE            *fp=NULL;

    if((filename==NULL)||(section==NULL)||(hdenc == NULL))
        return -1;
    
    section_len=strlen(section);
    if(section_len>30)
        return -1;
    ini=iniparser_load_lockfile(filename,1,&fp);
    if (ini==NULL) 
    {
        printf("hdsave  cannot parse ini file file [%s]", filename);
        gtloginfo("从配置文件%s中load ini失败,返回-1\n",filename);
        return -1 ;
    }
    memcpy(parastr,section,section_len);
    parastr[section_len]=':';
    section_len++;
    parastr[section_len]='\0';
    cat=strncat(parastr,"pre_rec",strlen("pre_rec"));   
    hdenc->pre_rec=iniparser_getint(ini,parastr,600);
    if(hdenc->pre_rec>3600)
    {
        gtloginfo("设置的预录时间%d过长，根据码流等调整为3600秒\n",hdenc->pre_rec);
        hdenc->pre_rec=3600;
    }
    
    parastr[section_len]='\0';
    cat=strncat(parastr,"dly_rec",strlen("dly_rec"));   
    hdenc->dly_rec=iniparser_getint(ini,parastr,30);

    parastr[section_len]='\0';
    cat=strncat(parastr,"cut_len",strlen("cut_len"));   
    hdenc->max_len=iniparser_getint(ini,parastr,30);
    if(hdenc->max_len<90)
    {
        gtloginfo("设置的切分时间%d过短,调整为90秒\n",hdenc->max_len);
        hdenc->max_len=90;
    }
    if(hdenc->max_len>1800)
    {
        gtloginfo("设置的切分时间过长,调整为1800秒\n",hdenc->max_len);
        hdenc->max_len=1800;
    }
    

    parastr[section_len]='\0';
    cat=strncat(parastr,"del_typ",strlen("del_typ"));   
    hdenc->del_typ=iniparser_getint(ini,parastr,0);

    parastr[section_len]='\0';
    cat=strncat(parastr,"rec_type",strlen("rec_type")); 
    hdenc->rec_type=iniparser_getint(ini,parastr,0);

#if 0
    parastr[section_len]='\0';
    cat=strncat(parastr,"a_channel",strlen("a_channel")); 
    hdenc->audiochannel = iniparser_getint(ini,parastr,0);
    if((hdenc->audiochannel < 0)||(hdenc->audiochannel >= MAX_AUDIO_ENCODER))
    {
        gtloginfo("音频端口无效%d，调整为0端口\n",hdenc->audiochannel);
        hdenc->audiochannel = 1;
    }
#endif    
#if 0
    //当前录像盘
    cat=iniparser_getstring(ini,"diskinfo:record_disk",NULL);
    if(cat != NULL)
    {
        if((strlen(cat) == 4) && (cat[0] == 's') && (cat[1] == 'd'))//必须是sdXX,如sda1
        {
            strcpy(hdenc->partition,"/hqdata/");
            strncat(hdenc->partition,cat,strlen("sda1"));   
        }
        else
        {
            gtlogerr("get record_disk error:%s,len:%d , set to sda1\n",cat,strlen(cat));
            strcpy(hdenc->partition,"/hqdata/sda1");
        }
    }
    else
    {
        gtloginfo("no record_disk, set to sda1\n");
        strcpy(hdenc->partition,"/hqdata/sda1");
    }
    printf("record disk =%s\n",hdenc->partition);
#endif




    sprintf(sec,"video%d:enable",channel);
    hdenc->enable = iniparser_getint(ini,sec,1);
        
    if(hdenc->enable == 0) //无效
    {
        gtloginfo("channel %d 视频无效，不开启录像线程\n",channel);
    }
    else
    {
        if(hdenc->rec_type==1)//触发预录模式 
        {
            gtloginfo("channel %d 移动触发预录模式\n",channel);
        }
        else
        {
            hdenc->rec_type=0;
            gtloginfo("channel %d 连续预录模式\n",channel);
        }
    }
    //为防止消息队列溢出，当pre_rec/cut_len>115时，增加消息队列长度
    msgnum=2*hdenc->pre_rec/hdenc->max_len;
    set_msgmnb_value(hdenc->qid,msgnum);

#ifdef USE_FFMPEG_LIB
    //读取音频格式
    ///目前不是u-pcm 就是 mpeg2
    if(channel == 0)
        audio_save_fmt=iniparser_getint(ini,"hqpara:audio_fmt",1);      //音频存储格式1为u-pcm 
    else
        audio_save_fmt=iniparser_getint(ini,"hqpara1:audio_fmt",1);      //音频存储格式1为u-pcm 

#endif

    //读出报警抓图的索引路径
    sprintf(sec,"alarm:snap_pic_path"); 
    
    cat=iniparser_getstring(ini,sec,ALARM_SNAPSHOT_PATH);
    if(cat==NULL)
        return -1;
    //gtloginfo("test,read out is WOW, %s\n",cat);
    sprintf(hdenc->alarmpic_path,"%s",cat);


    if(fp!=NULL)
    {
        unlock_file(fileno(fp));
        fsync(fileno(fp));
        fclose(fp);
    } 
    iniparser_freedict(ini);
    return 0;   

}

//设置正在处理抓图标志
int set_takingpicflag(int value, int ch)
{
    struct hd_enc_struct *hd;
    
    hd=get_hdch(ch);
    if(hd==NULL)
        return -1;
    
    if(pthread_mutex_lock(&hd->mutex)==0)
    {
        hd->takingpicflag=value;
        pthread_mutex_unlock(&hd->mutex);
    }
    return 0;
}
//设置需要报警抓图标志
int set_alarmpic_required(int value, int ch)
{

    struct hd_enc_struct *hd;
    
    hd=get_hdch(ch);
    if(hd==NULL)
        return -1;
    
    if(pthread_mutex_lock(&hd->mutex)==0)
    {
        hd->alarmpic_required=value;
        pthread_mutex_unlock(&hd->mutex);
    }
    return 0;
}
//设置正在报警抓图标志
int set_alarmpicflag(int value, int ch)
{
    struct hd_enc_struct *hd;
    
    hd=get_hdch(ch);
    if(hd==NULL)
        return -1;
    
    if(pthread_mutex_lock(&hd->mutex)==0)
    {
        hd->alarmpicflag=value;
        pthread_mutex_unlock(&hd->mutex);
    }
    return 0;
}
//初始化高清晰录像数据结构
int init_hdenc(void)
{   
    int rc;
    int i;
    //查找是否有上次未录完的文件，若有，将其改名为_OLD.AVI
    if(access(HDSAVE_PATH,F_OK)<0)
    {
        mkdir(HDSAVE_PATH,0777);
    }

    for(i=0;i<MAX_RECORD_CHANNEL_M;i++)
    {
        rc=init_hdenc_ch(i);
        set_takingpicflag(0,i);
        set_alarmpic_required(0,i);
        set_alarmpicflag(0,i);
    }
    
    set_cferr_flag(0);   
    send_state2main();
    return 0;   
}



//在互斥锁保护下设置指定通道的semflag
int set_semflag(int channel,int value)
{
    struct hd_enc_struct *hd;
    hd=get_hdch(channel);
    if(hd==NULL)
        return -1;
    if(pthread_mutex_lock(&hd->mutex)==0)
    {
        hd->semflag=value;
        pthread_mutex_unlock(&hd->mutex);
    }
    return 0;
}

int init_hqpara_default_val(struct hd_enc_struct    *hdenc)
{
    hdenc->pre_rec=600;
    hdenc->dly_rec=300;
    hdenc->max_len=30;
    hdenc->del_typ=0;
    hdenc->bitrate=1024;    
    hdenc->queryflag=0;
    
    return 0;
}
//初始化第channel路高清晰录像数据结构
int init_hdenc_ch(int channel)
{
    struct hd_enc_struct    *hdenc;
    int qid;
    //int check,ret;

    //key_t key;
    
//    if((channel>=get_total_hqenc_num())&&(channel<0))
//        return -1;

    printf("init_hdenc_ch channel=%d\n",channel);
    
    hdenc=get_hdch(channel);
    memset((void*)hdenc,0,sizeof(hdenc));
    pthread_mutex_init(&hdenc->mutex, NULL);//使用缺省设置
    init_hqpara_default_val(hdenc);
    read_hqsave_para_file(HDMOD_PARA_FILE,"hqpara",channel);

 
    hdenc->channel = channel;
    hdenc->audiochannel = channel;
    sprintf(hdenc->partition,"/hqdata/sda%d",channel+1);
    hdenc->recordlen=0;

    hdenc->thread_id=-1;
    hdenc->keyframe_pool_thread_id=-1;
    if((hdenc->pre_rec!=0)&&(hdenc->rec_type==0))
        hdenc->state=1;

    hdenc->watchcnt=0;
    hdenc->keyframe_cnt=0;
    hdenc->readenc_flag=0;
    hdenc->takingpicflag=0;
    hdenc->picerrorcode=0;
    hdenc->pictime=0;
    hdenc->timemax=0;
    hdenc->takepic_thread_id=-1;
    hdenc->remote_trig_time=0;
    hdenc->remote_trigged=0;
    hdenc->semflag = 0;
    hdenc->threadexit = 0;

#if 0
    printf("22222  init_hdenc_ch channel=%d\n",channel);
    //创建和初始化缓冲池
    //ret=mkrtpool(get_stream_pool(0),D1_MAX_FRAME_SIZE,POOLSIZE+4); //创建缓冲池
    ret=mkrtpool(get_stream_pool(hdenc->channel),MAX_FRAME_SIZE,1); //创建缓冲池,modified by wsy,使用devinfo.h里的size定义

#ifdef SHOW_WORK_INFO
    printf("mkrtpool rc=%d\n",ret);
#endif

    ret=initrtpool(get_stream_pool(hdenc->channel));

#ifdef SHOW_WORK_INFO
    printf("initrtpool rc=%d\n",ret);
#endif

    switch(channel)
    {
        case(0):sprintf(hdenc->devname,"%s",HQDEV0);break;
        case(1):sprintf(hdenc->devname,"%s",HQDEV1);break;
        case(2):sprintf(hdenc->devname,"%s",HQDEV2);break;
        case(3):sprintf(hdenc->devname,"%s",HQDEV3);break;
        default: break;

    }
#endif    
    
    pthread_mutex_init(&hdenc->audio_mutex, NULL);//使用缺省设置
    pthread_cond_init(&hdenc->audio_cond,NULL);
    pthread_mutex_init(&hdenc->file_mutex,NULL);
    hdenc->audio_thread_id=-1;
    
    return 0;
}

//获取指向第0路压缩结构的指针
/*struct compress_struct *get_encoder(int channel)
{
    if((channel<get_total_hqenc_num())&&(channel>=0))
        return &(hdenc[channel].encoder);
    else 
        return NULL;
}*/
    
//获取指向第0路记录的结构指针
struct hd_enc_struct    *get_hdch(int channel)
{
    if((channel<MAX_RECORD_CHANNEL_M)&&(channel>=0))
        return &g_hdenc[channel];
    else 
        return NULL;
}
//将秒数转换成相应的path名
int time2path (struct dir_info_struct *pathdir, time_t time)
{
    struct tm *p;
  
    p=localtime(&time);
    pathdir->year=1900+p->tm_year,
    pathdir->month=1+p->tm_mon,
    pathdir->date=p->tm_mday,
    pathdir->hour=p->tm_hour;
  
    return 0;
}




//根据hd的信息以及当前时间产生文件名返回给filename
int make_file_name(struct hd_enc_struct *hd,char * filename)
{
    time_t ctime;
    struct file_info_struct finfo;
    
    if((hd==NULL)||(filename==NULL))
        return -1;
        
    ctime=time(NULL);
    finfo.stime=ctime;
    finfo.channel=hd->channel;
    finfo.type=hd->state;
    finfo.len=0;
    finfo.trig=hd->trig;
    finfo.remote=hd->remote_trigged;
    sprintf(finfo.partition,hd->partition);
    //gtloginfo("test,remote is %d\n",finfo.remote);
    return finfo2filename(&finfo,filename);
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


//创建并写入I桢文件
//ifile:创建的文件名指针,调用者的缓冲区应该足够大，最好是100
int  create_hqpic_file(struct stream_fmt_struct *frame, char *indexname,char *ifile,int channel)
{
    FILE    *filep=NULL;
    FILE    *fp=NULL;
    int       ret;
    //int devtype;
    char    filename[100];
    struct timeval *tm;
    char    path[20];
    char    *p=NULL;
    int     devtype;
    media_attrib_t *media_attrib=NULL;
    media_format_t *media_format=NULL;
    video_format_t *v_fmt=NULL;
    char    *v_avitag=NULL; //视频编码格式的avi标记
    //char mpg_head[4]={0x00,0x00,0x01,0x00};
    
    
    tm=(struct timeval *)&frame->tv;
    sprintf(path,"%s/iframe",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    sprintf(filename,"%s/iframe/%d-%06d.mpg",HDSAVE_PATH,(int)tm->tv_sec,(int)tm->tv_usec);
    filep=fopen(filename,"w+");
    if(filep==NULL)
        return -1;
    //写入从16字节开始的图片数据长度
    ret=fprintf(filep,"%d",(int)frame->len); //?
    
    
    //写入视频编码格式字符串
    
    media_attrib=get_venc_attrib_keyframe(get_hqenc_video_ch(channel));
    media_format=&media_attrib->fmt;
    
    v_fmt=&media_format->v_fmt;
    
    switch(v_fmt->format)
    {
        case VIDEO_H264:
            v_avitag="H264";
        break;
        case VIDEO_MJPEG:
            v_avitag="MJPEG";
        break;
        case VIDEO_MPEG4:
            v_avitag="MPEG4";
        break;
        default:
            v_avitag="MPEG4";
        break;

   }
    
    ret=fseek(filep,8,SEEK_SET);
    ret=fprintf(filep,"%s",v_avitag); //?
#ifdef  REAL_D1_TEST_FOR_3000
    //deinterlace标志
    devtype = get_devtype();
    if((devtype>=T_GTVS3021)&&(devtype<T_GTVM3001))
    {
        if((v_fmt->v_width==720)&&(v_fmt->v_height==576))
        {
            ret=fseek(filep,15,SEEK_SET);
            ret=fprintf(filep,"%c",0x01);
        }
    }

#endif  

    
    //写入图片数据
    ret=fseek(filep,16,SEEK_SET);
    fwrite(frame->data,frame->len,1,filep); //?

    
    

    fclose(filep);


    chmod(filename,0755);
    fp=fopen(indexname,"a+");
    if(fp==NULL)
        return -1;
    if (PATH_TYPE)      
        ret=fprintf(fp,"%s\n",filename);
    else
    {
        p=strstr(filename,"/iframe");
        if(p!=NULL)
            ret=fprintf(fp,"%s\n",p);
        else
            ret=fprintf(fp,"%s\n",filename); ///<没找到,直接输出文件名
    }
    fclose(fp);
    chmod(indexname,0755);
    if(ifile!=NULL)
    {
        memcpy(ifile,filename,strlen(filename)+1);
    }
    
        
    return 0;
    
}
//给出一个时间戳和差距时间(毫秒为单位)，返回相减的时间戳
int  get_time_before(struct timeval *timenow, int diff_in_msec,struct timeval *timebefore)
{
    int diffsec,diffusec;
    
    if((timenow==NULL)||(timebefore==NULL))
        return -1;
    
    diffsec=diff_in_msec/1000;
    diffusec=1000*(diff_in_msec%1000);
    if(timenow->tv_usec>=diffusec)
    {
        timebefore->tv_usec=timenow->tv_usec-diffusec;
        timebefore->tv_sec=timenow->tv_sec-diffsec;
    }
    else
    {
        timebefore->tv_usec=timenow->tv_usec-diffusec+1000000;
        timebefore->tv_sec=timenow->tv_sec-diffsec-1;
    }
    return 0;
}

//计算给定时间戳的差距，秒为单位的double类型
double diff_timeval(struct timeval *timenow, struct timeval *timerequired)
{
    long sec;
    long usec;
    double diff; 
    
    if((timenow==NULL)||(timerequired==NULL))
        return -1;
    
    sec=timenow->tv_sec-timerequired->tv_sec;
    usec=timenow->tv_usec-timerequired->tv_usec;
    diff=sec+(double)usec/1000000;
    
    return(diff);       
}


//调用抓图线程
int usr_take_pic(gateinfo *gate, struct takepic_struct *takepic)
{

    double length;
    takepic_info *takepicinfo;
    int rc=-1;
    pthread_attr_t  thread_attr,*attr;
    struct hd_enc_struct *hd;

    if(takepic==NULL)
    {
        gtloginfo("抓图传来指针为空\n");
        return -1;
    }
    else
    {
        hd=get_hdch(takepic->channel);
        if(hd==NULL)
        {
            gtloginfo("抓图通道号%d超过视频编码器个数，无法抓图\n",takepic->channel);
            return -EINVAL;
        }
        memcpy(&hd->current_takepic,takepic,sizeof(struct takepic_struct));
    }
    if(get_gtthread_attr(&thread_attr)==0)
        attr=&thread_attr;
    else
        attr=NULL;
    takepicinfo = (takepic_info *)malloc(sizeof(takepic_info));
    memcpy(&takepicinfo->gate,gate,sizeof(gateinfo));
    memcpy(&takepicinfo->takepic,takepic,sizeof(struct takepic_struct));
    rc=pthread_create(&hd->takepic_thread_id,attr,takepic_thread,(void *)(takepicinfo));
    if(rc==0)
    {           
        hd->pictime=0;
        length=takepic->interval*takepic->takepic*3/1000;
        hd->timemax=30+(int)length;
#ifdef SHOW_WORK_INFO
        printf("takepic_timemax=%d\n\n",hd->timemax);
#endif      
        usleep(500000);
    }
    else
    {
        gtloginfo("无法为%d通道创建抓图线程,%d",takepic->channel,rc);

        if(get_takingpicflag(takepic->channel)==1)
            set_takingpicflag(0,takepic->channel);
        if(get_alarmpicflag(takepic->channel)==1)
            set_alarmpicflag(0,takepic->channel);
    }

    if(attr!=NULL)
    {
        pthread_attr_destroy(attr);
    }
    
    return rc;
    
}


//被调用来中止抓图线程的清场函数
void takepic_thread_cleanup(void *para)
{
    struct takepic_struct   takepic;
    char cmd[100];
    FILE *fp;
    struct stat buf;
    char indexnametype1[100];
    char indexnametype0[100]; //type=0时的索引名
    struct timeval timeprint;
    struct hd_enc_struct *hd;
    char alarmindex[200];


    
    takepic_info takepicinfo;
    memcpy((void*)&takepicinfo,para,sizeof(takepicinfo));
    memcpy(&takepic,&takepicinfo.takepic,sizeof(takepic));
    memcpy(&timeprint,&takepic.time,8);
    hd=get_hdch(takepic.channel);
    if(hd==NULL)
        return;
    gtloginfo("超时中止%d通道抓图线程\n",takepic.channel);
   //printf("\n\n come into takepic_thread_cleanup!!\n\n");
    
    sprintf(indexnametype1,"%s/picindex/%d-%d.txt",HDSAVE_PATH,(int)timeprint.tv_sec,(int)timeprint.tv_usec);
    sprintf(indexnametype0,"/picindex/%d-%d.txt",(int)timeprint.tv_sec,(int)timeprint.tv_usec);

    if(get_takingpicflag(takepic.channel)==1) //普通抓要送回网关
    {
        if(PATH_TYPE==1)
            get_hq_pic_answer(&takepicinfo.gate,ERR_DVC_INTERNAL,(BYTE*)&timeprint,(char*)indexnametype1);
        if(PATH_TYPE==0)
            get_hq_pic_answer(&takepicinfo.gate,ERR_DVC_INTERNAL,(BYTE*)&timeprint,(char*)indexnametype0);

        set_takingpicflag(0,takepic.channel);
    }
    if((get_alarmpic_required(takepic.channel)==1)||(get_alarmpicflag(takepic.channel)==1))//报警抓要
    {   
        sprintf(alarmindex,"%s%s",HDSAVE_PATH,hd->alarmpic_path);
        sprintf(cmd,"cp %s %s",indexnametype1,alarmindex);
        system(cmd);
        fp=fopen(alarmindex,"r+");
        if(fp!=NULL)
        {
            stat(alarmindex,&buf);
            if(buf.st_size!=0)
                {
                    gtloginfo("报警抓图被超时取消,索引为已抓到的图\n");
                }
            else//给*号
                {
                    gtloginfo("报警抓图被超时取消,没抓到图,索引写错误原因\n");
                    fprintf(fp,"*%s",get_gt_errname(ERR_DVC_INTERNAL));
                }
            fclose(fp);
        }
        else
            gtloginfo("无法打开报警抓图索引文件%s\n",alarmindex);
        set_alarmpicflag(0,takepic.channel);
        set_alarmpic_required(0,0);
    }
    sem_destroy(&hd->sem);
    set_semflag(takepic.channel,0);
    return;
}



//获取正在处理抓图标志
int get_takingpicflag(int channel)
{
    int value=-1;
    struct hd_enc_struct *hd;
    
    hd=get_hdch(channel);
    if(hd==NULL)
        return -1;
    
    if(pthread_mutex_lock(&hd->mutex)==0)
    {
        value= hd->takingpicflag;
        pthread_mutex_unlock(&hd->mutex);
    }
    
    return value;
}
//获取需要报警抓图标志
int get_alarmpic_required(int channel)
{
    int value=-1;
    struct hd_enc_struct *hd;

    hd=get_hdch(channel);
    if(hd==NULL)
        return -1;
    
    if(pthread_mutex_lock(&hd->mutex)==0)
    {
        value= hd->alarmpic_required;
        pthread_mutex_unlock(&hd->mutex);
    }
    return value;
}
//获取报警抓图标志
int get_alarmpicflag(int channel)
{
    int value=-1;
    struct hd_enc_struct *hd;
    
    hd=get_hdch(channel);
    if(hd==NULL)
        return -1;
    if(pthread_mutex_lock(&hd->mutex)==0)
    {
        value= hd->alarmpicflag;
        pthread_mutex_unlock(&hd->mutex);
    }
    return value;
}



#ifdef SNAP_TO_AVIFILE//added by shixin
//indexname暂时没有用 
int create_hqpic_avi_file(struct stream_fmt_struct *frame, char *indexname,char *ifile)
{
    FILE *filep;
    int ret;
    char filename[100];
    struct timeval *tm;
    char path[20];
    struct hd_enc_struct *hd;
    AVIVarHeader avihead;
    avi_t *aviinfo = NULL;
    
    if((frame==NULL))//||(indexname==NULL))||(ifile==NULL))
    {
        return -1;  
    }

    tm=(struct timeval *)&frame->tv;
    sprintf(path,"%s/iframe",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    sprintf(filename,"%s/iframe/%d-%06d.avi",HDSAVE_PATH,(int)tm->tv_sec,(int)tm->tv_usec);
    filep=fopen(filename,"w+");
    if(filep==NULL)
        return -1;
    hd=get_hdch(0);
    //memcpy((void*)&avihead,(void*)&hd->avi,sizeof(avihead));
    aviinfo=AVI_open_output_file(filename);
    if(aviinfo==NULL)
    {
        return -2;
    }
    AVI_write_frame(aviinfo, frame->data, frame->len, 1);
    AVI_close(aviinfo);
    chmod(filename,0755);
    if(ifile!=NULL)
    {
        memcpy(ifile,filename,strlen(filename)+1);
    }
    fclose(filep);
    return 0;

}
#endif
//处理抓图



void *takepic_thread(void *takepic)//抓图线程
{
    
    char indexname[100];
    char shortindexname[100];
    FILE *fp,*fpindex;
    char filename[100];
    char path[30];
    int picno=0;
    char cmd[100];
    int ret;
    int i,number=0;
    int error=0;
    char alarmindex[200];
    int waitsemflag=0;//为0则还在初抓，为1则已经是后续抓
    struct pool_ele_struct *ele,*elebak;
    int interval;
    struct timeval *tm,timeprint;
    struct takepic_struct *takepicpara;
    double diff,bakdiff=1000;
    double point=0;
    struct hd_enc_struct *hd;
    struct stream_fmt_struct *frame, *bakframe; 
    int firstpictaken=0;
    int bakused=1;//firstpictaken为1则已经取过第一张照片;bakused为1则表示上一个元素已经用过或不存在
    int active=0;

    if(takepic==NULL)
    {   
        error=ERR_EVC_NOT_SUPPORT;
        gtloginfo("传来抓图指针为空,不能抓图!\n");
        goto send_result;
    }
    takepic_info *takepicinfo;
    takepicinfo = (takepic_info *)takepic;
    takepicpara = &takepicinfo->takepic;
    
    memcpy((void*)&timeprint,(void*)takepicpara->time,sizeof(timeprint));
    gtloginfo("抓图线程中,张数%d,间隔%d,通道%d,时间%ld-%ld\n",takepicpara->takepic,takepicpara->interval,takepicpara->channel,timeprint.tv_sec,timeprint.tv_usec);  
    
    hd=get_hdch(takepicpara->channel);
    if(hd==NULL)
    {
        error=ERR_DVC_INTERNAL;
        gtloginfo("取不到hd结构，不能抓图\n");
        goto send_result;
    }
    printf("start takepic_thread... \n");

    


    //设置取消
    ret=pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    ret=pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
    pthread_cleanup_push(takepic_thread_cleanup,(void *)takepicinfo);
    //初始化局部变量
    
    set_semflag(takepicpara->channel,0); 
    picno=takepicpara->takepic;
    interval=takepicpara->interval;
    //Step1.判断剩余空间是否小于GET_PIC_SPACE
    if(get_disk_free(HDSAVE_PATH)<GET_PIC_SPACE)
    {
        
        printf("space not enough, cannot get picture\n");
        //gtlogerr("空间不足,%d M,不予抓图\n",get_disk_free(HDSAVE_PATH));
        gtlogerr("空间不足,%d M不予抓图\n",get_disk_free(HDSAVE_PATH));
        
        error=ERR_NO_SPACE;
        goto send_result;
    }

    //Step2.初始化抓图数目。若要求的抓图张数过多，只允许抓TAKE_PIC_MAX张
    if(picno>TAKE_PIC_MAX)
    {   
        picno=TAKE_PIC_MAX;
        gtloginfo("抓图张数超过%d,只能抓%d张\n",TAKE_PIC_MAX,TAKE_PIC_MAX); 
    }
    if(picno<=0)
    {
     gtloginfo("抓图张数%d小于等于0,不进行抓图\n",picno);
     error=ERR_ENC_NOT_ALLOW;
     goto send_result;
     }

    
    //Step3.创建且初始化一个索引文件.txt
    sprintf(path,"%s/picindex",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    sprintf(indexname,"%s/picindex/%d-%08d.txt",HDSAVE_PATH,(int)timeprint.tv_sec,(int)timeprint.tv_usec);
    sprintf(shortindexname,"/picindex/%d-%08d.txt",(int)timeprint.tv_sec,(int)timeprint.tv_usec);
    fp=fopen(indexname,"w+");
    if(fp==NULL)
    {
        error=ERR_NO_SPACE;
        gtlogerr("打不开指定文件%s,不能抓图\n",indexname);
        goto send_result;
    }
    else
        fclose(fp); 

    if(hd->picerrorcode!=0)//编码器错误
    {   
        gtloginfo("编码器出现错误0x%04x，不能抓图!\n",hd->picerrorcode);
        error=hd->picerrorcode;
        goto send_result;
    }
    
    //Step4.把缓冲区中的元素逐个取出，比较时间戳，若符合需要则取出并写入文件
    
    getpicture:
    active=get_pool_active_num(get_stream_pool(takepicpara->channel));
    for(i=number;i<number+active;i++)
    {

        if(picno==0) 
            break;  
        ele=get_active_ele(get_stream_pool(takepicpara->channel)); //取出一个缓冲区元素
        frame=(struct stream_fmt_struct*)&ele->element;
        tm=(struct timeval *)&frame->tv;
        
        diff=diff_timeval(tm,&timeprint);

        if(diff<point) //在时间戳之前
        {   
            if((i==number+active-1)&&(firstpictaken==0))//最后一张，取,情况0
            {
                create_hqpic_file(frame,indexname,filename,takepicpara->channel);               

#ifdef SNAP_TO_AVIFILE//added by shixin
                create_hqpic_avi_file(frame,indexname,NULL);
#endif
                point=diff+(double)interval/1000;
                picno--;
                bakused=1;
                ret=free_ele(get_stream_pool(takepicpara->channel),elebak);
                if(picno!=0) 
                    elebak=ele;
                firstpictaken=1;
#ifdef SHOW_WORK_INFO
                printf("take this one,0\n");
#endif
            }
            else
            {
                bakframe=frame; //备份
                bakdiff=diff;
                ret=free_ele(get_stream_pool(takepicpara->channel),elebak);

                if(picno!=0) 
                    elebak=ele;
                bakused=0;                      
#ifdef SHOW_WORK_INFO
                printf("backup,0\n");
#endif  
            }
        }   
        else//在时间戳之后
        {
        
            if(firstpictaken==0) //还没取第一张,则若有上一张就取上一张，否则取这一张
            {
                if(bakused==0)
                {
                    //test=(struct timeval *)&bakframe->tv;
                    //gtloginfo("情况1 frame %d-%06d\n",(int)test->tv_sec,(int)test->tv_usec);
                    create_hqpic_file(bakframe,indexname,filename,takepicpara->channel);
#ifdef SNAP_TO_AVIFILE//added by shixin
                    create_hqpic_avi_file(bakframe,indexname,NULL);
#endif
                    point=bakdiff+(double)interval/1000;
                    picno--;
                    bakframe=frame; //备份
                    bakdiff=diff;
                    ret=free_ele(get_stream_pool(takepicpara->channel),elebak);
                    if(picno!=0) 
                        elebak=ele;
                    bakused=0;
                    firstpictaken=1;
#ifdef SHOW_WORK_INFO
                    printf("take last one，1\n");
#endif
                }
                else
                {
                    //test=(struct timeval *)&frame->tv;
                    //gtloginfo("情况2 frame %d-%06d\n",(int)test->tv_sec,(int)test->tv_usec);
                    create_hqpic_file(frame,indexname,filename,takepicpara->channel);
#ifdef SNAP_TO_AVIFILE//added by shixin
                    create_hqpic_avi_file(frame,indexname,NULL);
#endif                      
                    point=diff+(double)interval/1000;
                    picno--;
                    bakused=1;
                        ret=free_ele(get_stream_pool(takepicpara->channel),elebak);
                    if(picno!=0) 
                        elebak=ele;
                    firstpictaken=1;
#ifdef SHOW_WORK_INFO
                    printf("take this one，2");
#endif
                }
            }
            else //已取过第一张，随后的都无所谓
            {
                //如果上一张用过了，则取这一张
                if(bakused==1)
                {
                    //test=(struct timeval *)&frame->tv;
                    //gtloginfo("情况3 frame %d-%06d\n",(int)test->tv_sec,(int)test->tv_usec);
                    create_hqpic_file(frame,indexname,filename,takepicpara->channel);
#ifdef SNAP_TO_AVIFILE//added by shixin
                    create_hqpic_avi_file(frame,indexname,NULL);
#endif
                    point=diff+(double)interval/1000;
                    picno--;
                    bakused=1;
                    ret=free_ele(get_stream_pool(takepicpara->channel),elebak);
                    if(picno!=0) 
                        elebak=ele;
#ifdef SHOW_WORK_INFO
                    printf("take this one 3,\n");
#endif
                }
                else //如果上一张没用过，则比较谁更近就取谁
                {
                    if(fabs(bakdiff-point)<fabs(diff-point)) //上一张更近 
                    {
                        //test=(struct timeval *)&bakframe->tv;
                        //gtloginfo("情况4 frame %d-%06d\n",(int)test->tv_sec,(int)test->tv_usec);
                        create_hqpic_file(bakframe,indexname,filename,takepicpara->channel);
#ifdef SNAP_TO_AVIFILE//added by shixin
                        create_hqpic_avi_file(bakframe,indexname,NULL);
#endif
                        point=bakdiff+(double)interval/1000;
                        picno--;
                        bakframe=frame; //备份
                        bakdiff=diff;
                        ret=free_ele(get_stream_pool(takepicpara->channel),elebak);
                        if(picno!=0) 
                            elebak=ele;
                        bakused=0;
#ifdef SHOW_WORK_INFO
                        printf("take last one...4 it's nearer\n");
#endif
                    }
                    else //这一张更近
                    {
                        create_hqpic_file(frame,indexname,filename,takepicpara->channel);
#ifdef SNAP_TO_AVIFILE//added by shixin
                        create_hqpic_avi_file(frame,indexname,NULL);
#endif
                        point=diff+(double)interval/1000;
                        picno--;
                        ret=free_ele(get_stream_pool(takepicpara->channel),elebak);
                        if(picno!=0) 
                            elebak=ele;
                        bakused=1;
#ifdef SHOW_WORK_INFO       
                        printf("take this one.5.nearer\n");
#endif
                    }   
                }
            }
        }
    }
    
#ifdef SHOW_WORK_INFO
    printf("picno= %d left\n",picno);
#endif
    //Step5.检查剩下的需抓图片数，决定重新Step4或结束抓图发送结果
    if(picno!=0)
    {
        if(waitsemflag!=1)
        {
            ret=sem_init(&hd->sem,0,0);
            waitsemflag=1;
        }
        number=number+active;
        set_semflag(takepicpara->channel, 1);
        pthread_testcancel();
        ret=sem_wait(&hd->sem);
        goto getpicture;
    
    }
    gtloginfo("第%d路照片抓完\n",takepicpara->channel);

    sem_destroy(&hd->sem);
    ret=free_ele(get_stream_pool(takepicpara->channel),ele);
    set_semflag(takepicpara->channel,0);
    chmod(indexname,0755);

    //发送结果
send_result:
  printf("[%s:%d]get_takingpicflag=%d\n",__FILE__,__LINE__,get_takingpicflag(takepicpara->channel));
    if(get_takingpicflag(takepicpara->channel)==1)
    {
        if(PATH_TYPE==1)
            ret=get_hq_pic_answer(&takepicinfo->gate, error,takepicpara->time,indexname);
        else
            ret=get_hq_pic_answer(&takepicinfo->gate,error,takepicpara->time,shortindexname);
        ret=set_takingpicflag(0,takepicpara->channel);
    }
    if((get_alarmpicflag(takepicpara->channel)==1)||(get_alarmpic_required(takepicpara->channel)==1))//有报警抓图需求,要将文件改名
    {
        sprintf(alarmindex,"%s%s",HDSAVE_PATH,hd->alarmpic_path);
        if(error==0)
        {
            sprintf(cmd,"cp %s %s",indexname,alarmindex);
            gtloginfo("报警抓图将%s拷成%s\n",indexname,alarmindex);
            system(cmd);
        }
        else
        {
            gtloginfo("报警抓图失败,原因%s,写入索引文件\n",get_gt_errname(error));
            fpindex=fopen(alarmindex,"w+");
            if(fpindex!=NULL)
            {
                fprintf(fpindex,"*%s",get_gt_errname(error));
                fclose(fpindex);
            }
            else
                gtloginfo("打不开报警抓图索引文件，错误%s\n",strerror(errno));
        }
        set_alarmpicflag(0,takepicpara->channel);
        set_alarmpic_required(0,takepicpara->channel);
    }


    pthread_cleanup_pop(0);
    pthread_exit(0); 
}   


//查到一条符合要求的记录,若符合则记入index文件并返回正值
int process_filename_to_index(int channel,char *filename, int indexno, FILE *fp)
{
    char *ing, *lp, *lk;
    char ingname[200],temp[200],flag[50];
    struct hd_enc_struct * hd;
    
    if((filename ==NULL)||(fp ==NULL))
        return -EINVAL;     
    
    
    //如果是_OLD.AVI则不记入索引
    if(strstr(filename,OLD_FILE_EXT)!=NULL)
        return -ENOENT;
    
    ing=strstr(filename,RECORDING_FILE_EXT);
    if(ing!=NULL) //ing文件，单独处理，虚拟一个文件名
    {
        hd = get_hdch(channel);
        if(hd == NULL)
            return -EINVAL;
        sprintf(ingname,"%s",filename);
        pthread_mutex_lock(&hd->mutex); 
        hd->queryflag=1;
        pthread_mutex_unlock(&hd->mutex);   
        ing=strstr(ingname,RECORDING_FILE_EXT); 
        *ing='\0';  //ingname='HQ......T00'
        lp=index(ingname,'L');
        lp++;       //lp='00_T00'
        sprintf(temp,"%s",lp); 
        *lp='\0'; //ingname='HQ.._L'
        lk=index(temp,'T');  //temp='00_T00'
        lk++;    //lk='00'
        if(atoi(lk)!=0)  //如果触发状态不为0则要加锁
            sprintf(flag,"%c%s",LOCK_FILE_FLAG,IMG_FILE_EXT);
        else 
            strcat(flag,IMG_FILE_EXT); //此时flag="(@).AVI"
        sprintf(temp,"%02d_T%02d%s",hd->max_len,atoi(lk),flag); //temp='120_T00(@).AVI'                                     
        strcat(ingname,temp);
        return fprintf(fp,"%s\n",strstr(ingname,"/sd"));//剥去/hqdata
            
    }
    else    
        return fprintf(fp,"%s\n",strstr(filename,"/sd"));       
}


int query_index_in_partition(IN char *devname, IN char* mountpath, IO void *fn_arg)
{
    struct query_index_process_struct * query;
    
    if((mountpath == NULL)||(fn_arg == NULL))
        return -EINVAL;
        
    query = (struct query_index_process_struct *) fn_arg;
    if(query->index_fp == NULL)
        return -EINVAL;
    return fileindex_query_index(mountpath,query);
}



//改为从索引文件中查,
int query_record_index(char *indexname,int ch,time_t start,time_t stop,int trig)
{
    int result;//查询结果
    FILE *fp;
    struct stat statbuf;
    struct query_index_process_struct queryindex;
    char path[20];
    char cmd[200];
    char tmpname[100];//存放排序前的索引名称
    
    
    sprintf(path,"%s/index",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    sprintf(tmpname,"%s/index/%d-tmp.txt",HDSAVE_PATH,(int)start);
    fp=fopen(tmpname,"w+");
    if(fp==NULL)
        return -1;
    
    
    queryindex.index_fp =   fp;
    queryindex.ch       =   -1;//ch;ip1004只有4画面，全部查
    queryindex.start    =   start;
    queryindex.stop     =   stop;
    queryindex.trig_flag=   trig;      
    result = mpdisk_process_all_partitions(query_index_in_partition, &queryindex);

    fclose(fp);
    stat(tmpname,&statbuf);
    //printf("st_size is %d\n",statbuf.st_size);
    if(statbuf.st_size !=0)
    {
        sprintf(indexname,"%s/index/%d.txt",HDSAVE_PATH,(int)start);
        sprintf(cmd,"/ip1004/record_sort %s %s",tmpname,indexname);
        system(cmd);
        sprintf(indexname,"/index/%d.txt",(int)start);
        return 0;
    }
    else
        return -ERR_DVC_NO_RECORD_INDEX;
}

//static volatile int rtstream_cnt=0;

void hd_playback_en(void)
{
    hd_playback_flag=1;
}

void hd_playback_cancel(void)
{
    hd_playback_flag=0;
}




//高清晰记录模块秒处理程序
void hd_second_proc(void)
{
    struct hd_enc_struct    *hd_enc;    
    media_attrib_t *attrib=NULL;//
    media_attrib_t *attrib_keyframe=NULL;
    int intid;
    int ret;
    int i;

    for(i=0; i<MAX_RECORD_CHANNEL_M; i++)
    { 
    //激活音频编码器
        hd_enc=get_hdch(i);
        intid = hd_enc->audio_thread_id;
        if(intid > 0)
        {
            attrib=get_aenc_rec_attrib(hd_enc->audiochannel);  //NULL表示还没有连接到音频编码器
            if((attrib!=NULL))
            {
                if(++hd_enc->audio_cnt<10)
                {       
                }
                else
                {   
                    //gtloginfo("wsytest,reactive_audio_enc\n");
                    ret=reactive_audio_rec_enc(hd_enc->audiochannel);
                    if(ret<0)                   
                      {
                          gtloginfo("重新激活音频编码器%d失败!ret =%d\n",0,ret);
                          printf("重新激活音频编码器%d失败ret= %d\n",0,ret);
                      } 
                  //else
                      //gtloginfo("wsytest,reactive_audio_enc done\n");
                    hd_enc->audio_cnt=0;
                }
            }
        }
        else
        {
            hd_enc->audio_cnt=0;
        }
    }
    
  for(i=0; i<MAX_RECORD_CHANNEL_M; i++)
  {
      hd_enc=get_hdch(i);
      //为录像线程激活视频编码器
      intid=hd_enc->thread_id;
      if(intid>0)
      {
          attrib=get_venc_attrib_record(hd_enc->channel);  //NULL表示还没有连接到视频编码器
          if(attrib!=NULL)
          {
              if(++hd_enc->watchcnt<10)
              {     
                
              }
              else
              {         
                  if(hd_enc->watchcnt%10==0)
                  {
                      //gtloginfo("录像线程%ds没有读到视频数据\n",hd_enc->watchcnt);            
                      if((hd_enc->watchcnt<100)&&(hd_enc->watchcnt >= 10))//超过100秒就不记了
                      {
                          if(hd_enc->readenc_flag == 1)
                          {
                              gtloginfo("录像线程%ds没有读到视频数据\n",hd_enc->watchcnt);          
                              printf("record thread %ds没有读到视频数据\n",hd_enc->watchcnt);
                          }
                          else
                          {
                              gtloginfo("录像线程%ds没有写入视频数据\n",hd_enc->watchcnt);
                              printf("record thread %ds没有写入视频数据\n",hd_enc->watchcnt);
                          }
                      }

                      pthread_mutex_lock(&hd_enc->file_mutex);
                      if((hd_enc->watchcnt >= 10)&&(hd_enc->aviinfo != NULL))
                      { 
                          gtloginfo("因%d秒无数据，关闭录像文件\n",hd_enc->watchcnt);
                          close_record_file(hd_enc);        
                      }
                      pthread_mutex_unlock(&hd_enc->file_mutex);
                      ret = reactive_video_record_enc(hd_enc->channel);
                      if(ret<0)
                      {
                          printf("重新激活视频编码器%d失败:%d\n",hd_enc->channel,ret);
                          gtloginfo("重新激活视频编码器%d失败:%d\n",hd_enc->channel,ret);
                      }
                  }
              }
          }
      }
      else
      {
          hd_enc->watchcnt=0;
      }
#if 0    //ip1004不考虑抓图
    //为keyframe线程激活视频编码器
      intid=hd_enc->keyframe_pool_thread_id;
      if(intid>0)
      {
          attrib_keyframe=get_venc_attrib_keyframe(get_hqenc_video_ch(hd_enc->channel));
          if(attrib_keyframe!=NULL)
          {
              if(++hd_enc->keyframe_cnt<10)
              {     
              }
              else
              {         
                  if(hd_enc->keyframe_cnt%10==0)
                  {
                      if(hd_enc->keyframe_cnt<100)//超过100秒就不记了
                      {
                          printf("keyframe线程%ds没有读到视频数据\n",hd_enc->keyframe_cnt);
                          gtloginfo("keyframe thread %ds没有读到视频数据\n",hd_enc->keyframe_cnt);          
                      }
                      if(reactive_video_enc(get_hqenc_video_ch(hd_enc->channel))<0)                 
                      {
                          gtloginfo("keyframe线程重新激活视频编码器%d失败!\n",get_hqenc_video_ch(hd_enc->channel));
                          printf("keyframe线程重新激活视频编码器%d失败!\n",get_hqenc_video_ch(hd_enc->channel));
                      } 
                  }
              }
          }
      }
      else
      {
          hd_enc->keyframe_cnt=0;
      }

     //处理抓图相关
      if((get_takingpicflag(hd_enc->channel)==1)||(get_alarmpicflag(hd_enc->channel)==1)) //正在抓图
      {
          if(hd_enc->pictime<hd_enc->timemax)
            {
         hd_enc->pictime++;
       }
          else
          {
              intid=(int)hd_enc->takepic_thread_id;
              if(intid>0)
                {
                    sem_post(&hd_enc->sem);
                    ret=pthread_cancel(hd_enc->takepic_thread_id);
           printf("    CANCEL ==========================\n");
                    hd_enc->takepic_thread_id=-1;//added by shixin
                }
              hd_enc->pictime=0;
          }
      }
#endif      
    //处理录像状态，加锁等相关
        
        if(hd_enc->state!=0)
        {
            if(hd_enc->aviinfo != NULL)
            {
                if(++hd_enc->filelen>=hd_enc->max_len)//切分
                {
                    hd_enc->cutflag=1;  
                }   
            }
            if(hd_enc->remote_trig_time>0)
            {
                if((--hd_enc->remote_trig_time)==0)
                {     
                    if(hd_enc->recordlen==0)
                    {
                        if((hd_enc->rec_type==0)&&(hd_enc->pre_rec !=0))
                        {
                            hd_enc->state=1;
                        }
                        else
                        {
                            hd_enc->state=0;            
                        }
                    }
                    else
                    {
                      hd_enc->cutflag=1;
                    }
                    hd_enc->remote_trigged=0;
                }
            }

            if(hd_enc->recordlen>0) //处理停止录像
            {
                if((--hd_enc->recordlen)==0)
                {     
                    if(hd_enc->remote_trigged==0)
                    {
                        if((hd_enc->pre_rec>0)&&(hd_enc->rec_type==0))//||(hd_enc->pre_rec==0))//wsy fixed from &&
                        {
                            hd_enc->state=1;
                        }
                        else
                        {
                            hd_enc->state=0;
                        }
                    }
                    else
                    {
                        hd_enc->cutflag=1;
                    }
                    hd_enc->trig=0;
                }
            }
        }
        pthread_mutex_unlock(&hd_enc->mutex);
    }
}



int lock_recent_file(struct hd_enc_struct *hd)//从index.db取并且锁
{
    time_t timenow;
    
    if(hd==NULL)
        return -EINVAL;
        
    timenow=time((time_t *)NULL);
    fileindex_lock_by_time(hd->partition,1, timenow-hd->pre_rec,timenow,-1,hd->channel);
    return 0;
}

/*触发一次录像事件
  *hd:描述录像及采集设备的数据结构
  *trig:触发事件(录像原因)
  *reclen:希望进行多长时间的录像(实际录像时还会加上延时录像),传0即可
  */
int trig_record_event(struct hd_enc_struct *hd,WORD trig,int reclen)
{
    if(hd==NULL)
        return -1;

    
#ifdef SHOW_WORK_INFO
    printf("we want to record %d seconds 'record',trig = 0x%x!\n",reclen,trig);
#endif
    if((trig==0)&&(hd->rec_type==0))
        return 0;
    
    if(hd->enable == 0)//not enabled
    {
        gtloginfo("%d路视频禁用,触发录像无效\n",hd->channel);
        return -1;
    }   

    pthread_mutex_lock(&hd->mutex);

    if(trig==0)//有效但不报警的录像
    {
        if(hd->recordlen<MOTION_DLY_REC+reclen)
            hd->recordlen=MOTION_DLY_REC+reclen;
        if(hd->state==0)
            hd->state=1;
    }
    else
    {
        if((hd->state==2)&&(hd->trig!=trig))
            hd->cutflag=1;//如果触发状态不一样则切割文件
        else
            hd->state=2;
        hd->trig=hd->trig|trig;
        reclen+=hd->dly_rec;
        if(hd->recordlen<reclen)
            hd->recordlen=reclen;
    }
    //gtloginfo("test,trig_record_event函数中处理完毕\n");
    pthread_mutex_unlock(&hd->mutex);
    return 0;
}

//处理远程发来的高清晰录像指令
int remote_start_record(struct hd_enc_struct *hd,int reclen)
{
    if(hd==NULL)
        return -1;
    if(reclen==0)
        reclen=65535;
    pthread_mutex_lock(&hd->mutex);
    gtloginfo("远程要求手工录像%d秒\n",reclen);
    hd->remote_trig_time=reclen;
#ifdef SHOW_WORK_INFO
    printf("recv rmt record reclen=%d hd->recordlen=%d \n",reclen,hd->recordlen);
#endif
    
    if((hd->remote_trigged!=1)&&(hd->state!=0))
        hd->cutflag=1; 
    hd->remote_trigged=1;
    hd->state=2;
    
    pthread_mutex_unlock(&hd->mutex);
    dump_saveinfo_to_log(1,hd->channel,reclen);
    return 0;
}
//处理远程发来的停止高清晰录像指令
int remote_stop_record(struct hd_enc_struct *hd)
{
    if(hd==NULL)
        return -1;
    gtloginfo("远程要求%d路hqstop\n",hd->channel);
    pthread_mutex_lock(&hd->mutex);
    /*trig=hd->trig;
    rec_len=0;
    (trig!=0)//有报警信息
    {
        for(i=0;i<TOTAL_TRIG_IN-1;i++)//报警联动时改此处
        {
            if((trig&(1<<i))!=0)
            {
                newlen=get_ch_alarm_rec_time(i);
                if(rec_len<newlen)
                    rec_len=newlen;//如果有报警信号则应该录满报警信号指定的时间
            }
        }       
    }
    if(rec_len==0) //目前没有未完的本地触发
    {*/
    if(hd->remote_trigged==1)
        hd->cutflag=1;
    hd->remote_trigged=0;
    if(hd->trig==0)
    {
        if((hd->pre_rec==0)||(hd->rec_type==1))
            hd->state=0;
        else
            hd->state=1;
        hd->recordlen=0;
    }
    
    hd->remote_trig_time=0;
    
    
    /*
    else //目前还有未完的本地触发
    {
        hd->trig=hd->trig&0x3FF;
        if(hd->recordlen<rec_len)
        {
            hd->recordlen=rec_len;
        }
    }*/
    pthread_mutex_unlock(&hd->mutex);
    dump_saveinfo_to_log(0,hd->channel,0);
    return 0;
}

#ifdef RECORD_PS_FILE
    static FILE* OUT_FP = NULL;
    static PS_handle_t *ps_fd;
    static unsigned char head_buf[1024*256]={0};
    int ps_len;
#endif


void fix_adts_header(char* data,int len)
{
	unsigned int obj_type=0;
	unsigned int num_data_block = (len-8)/1024;
	char * adts_header = data+1;
	char channels = 2;
	int rate_idx = 8;

	int frame_length = len-8+7;
	adts_header[0]=0xff;
	adts_header[1]=0xf9;
	adts_header[2]=obj_type<<6;
	adts_header[2]|=(rate_idx<<2);

	adts_header[2]|=(channels&0x4)>>2;
	adts_header[3]=(channels&0x3)<<6;
	adts_header[3]|=(frame_length&0x1800)>>11;
	adts_header[4]=(frame_length&0x1ff8)>>3;
	adts_header[5]=(frame_length&0x7)<<5;

	adts_header[5]|=0x1f;
	adts_header[6]=0xfc;
	adts_header[6]|=num_data_block&0x03;

}


//根据hd的信息创建一个文件
avi_t * create_record_file(struct hd_enc_struct *hd)
{
    char *p;
    char tname[128];
    int ret;
    char    *v_avitag=NULL;     ///<视频编码格式的AVI标记
    avi_t *aviinfo=NULL;
    media_attrib_t *media_attrib=NULL;
    media_format_t *media_format=NULL;
    video_format_t *v_fmt=NULL;
    audio_format_t *a_fmt=NULL;
    int file_errno=0;

    
    if(hd==NULL)
        return NULL;

    if(make_file_name(hd,hd->filename)<0)
    {
        printf("create_record_file make_file_name error!\n");
        return NULL;
    }
    
    
    p=strstr(hd->filename,IMG_FILE_EXT);
    if(p!=NULL)
    {
        strcpy(tname,hd->filename);
        *p='\0';
        strncat(hd->filename,RECORDING_FILE_EXT,100); 
        //rename(tname,hd->filename);
    }
        

    ret=hdutil_create_dirs(hd->filename);//changed by shixin
    if(ret<0)
    {
        file_errno=errno;
        gtlogerr("无法为%s创建目录,错误%d: %s\n",hd->filename,file_errno,strerror(file_errno));
        fix_disk(hd->filename, file_errno);
        return NULL;
    }
        
        //ret=AVIFileOpen(hd->filename,&hd->avi);
    media_attrib=get_venc_attrib_record(hd->channel);
    media_format=&media_attrib->fmt;
    v_fmt=&media_format->v_fmt;
    
    media_attrib=get_aenc_rec_attrib(hd->audiochannel);
    if(media_attrib!=NULL)
    {
        media_format=&media_attrib->fmt;
        a_fmt=&media_format->a_fmt;
  
    }
    //加入索引
    ret = fileindex_add_to_partition(hd->partition,hd->filename);
    aviinfo=AVI_open_output_file(hd->filename);
    if(aviinfo==NULL)
    {
        
        file_errno=errno;
        printf("can't create avi file %s\n",hd->filename);
        gtlogerr("无法创建avi文件%s,fopen失败 %s\n",hd->filename,strerror(file_errno));
        fix_disk(hd->filename,file_errno);
        return NULL;
    }   
    else 
    {
        
        
        //设置音视频参数
/*      
        gtloginfo("v_fmt->v_frate is %d\n",v_fmt->v_frate);
        gtloginfo("v_width is %d\n",v_fmt->v_width);
        gtloginfo("v_fmt->v_height is %d\n",v_fmt->v_height);
        gtloginfo("format is %d\n",a_fmt->a_wformat);
        gtloginfo("channel is %d\n",a_fmt->a_channel);
        gtloginfo("sampling is %d\n",a_fmt->a_sampling);
        gtloginfo("nr_frame is %d\n",a_fmt->a_nr_frame);
        gtloginfo("bitrate is %d\n",a_fmt->a_bitrate);
        gtloginfo("bits is %d\n",a_fmt->a_bits);
*/

            switch(v_fmt->format)
            {
                case VIDEO_H264:
                    v_avitag="H264";
                break;
                case VIDEO_MJPEG:
                    v_avitag="MJPG";
                break;
                case VIDEO_MPEG4:
                    v_avitag="divx";
                break;
                default:
                    v_avitag="divx";
                break;

            }

        AVI_set_video(aviinfo,v_fmt->v_width,v_fmt->v_height,v_fmt->v_frate,v_avitag);//"divx" for compressor
        if(a_fmt!=NULL)
        {
            
                #ifdef USE_FFMPEG_LIB   
                      if(audio_save_fmt==4)
                     {
                        AVI_set_audio(aviinfo,2,a_fmt->a_bitrate,a_fmt->a_bits,0x50,(a_fmt->a_bits)*(a_fmt->a_bitrate));
                     }
                      else
                        AVI_set_audio(aviinfo,a_fmt->a_channel,a_fmt->a_bitrate,a_fmt->a_bits,a_fmt->a_wformat,(a_fmt->a_bits)*(a_fmt->a_bitrate));
                #else
            if(get_audio_num()>0)
                    AVI_set_audio(aviinfo,a_fmt->a_channel,a_fmt->a_bitrate,a_fmt->a_bits,a_fmt->a_wformat,(a_fmt->a_bits)*(a_fmt->a_bitrate));
                #endif
                }
        //填充avi header
        AVI_fix_header(aviinfo,0);
#ifdef RECORD_PS_FILE
        char mpgname[200];
        sprintf(mpgname,"%s-ps.mpg",hd->filename);
        OUT_FP=fopen(mpgname,"wb");
        if(OUT_FP==NULL)
        {
            gtloginfo("fopen %s failed \n",mpgname);
            return aviinfo;
        }
        ps_fd=ps_require();
        if(ps_fd==NULL)
        {
            gtloginfo("open ps_fd failed\n");
            return aviinfo;
        }
        if(v_fmt->format == VIDEO_H264)
            ret=ps_set_video(ps_fd, V_DECODE_H264, v_fmt->v_width, v_fmt->v_height, v_fmt->v_frate, 300000);
        else    
            ret=ps_set_video(ps_fd, V_DECODE_MPEG4, v_fmt->v_width, v_fmt->v_height, v_fmt->v_frate, 300000);
        if(ret<0)
        {
            gtloginfo("ps_set_video failed\n");
            return aviinfo;
        }
    //zw-add 2012-04-23---->
    ret=fileindex_add_to_partition(hd->partition,mpgname)
    if(ret<0)
    {
      gtlogerr("[%s:%d]error ret=%d,file add [%s] into db error\n",__FILE__,__LINE__,ret,mpgname);
      return aviinfo;
    }
    //zw-add 2012-04-23<----
#endif
        return aviinfo;
    }
}

//修订已被查询的文件名,传来参数为name和newname
//格式为HQ_C00_D20060621142504_L120_T00.ING
int modify_queried_filename(char *name,char *newname)
{
    struct hd_enc_struct *hd;
    int cut_len;
    char *lp,*lk,*lm;
    char buf[100];

    if((name==NULL)||(newname==NULL)) 
        return -1;
    lm=strstr(name,"_C");
    if(lm==NULL)
        return -2;
    hd=get_hdch(atoi(lm)); //从"_C00_"读取
    if(hd==NULL)
        return -1;
    cut_len=hd->max_len;
    lp=strstr(name,"_L");
    if(lp==NULL)
        return -1;
    lp=lp+2;
    lk=index(lp,'_');
    memcpy(buf,lk,30);
    *lp='\0';
    sprintf(newname,"%s%02d%s",name,cut_len,buf);

    return 0;
}

//按照hd的信息关闭一个文件
int close_record_file(struct hd_enc_struct *hd)
{

    struct file_info_struct finfo;
    char newname[100];
    
    avi_t *aviinfo;
    int error=0;

#ifdef ZW_DB_TEST
    struct timeval tv;
    gettimeofday(&tv,NULL);
    gtloginfo("[%ld:%ld]entering [%s:%d]\n",tv.tv_sec,tv.tv_usec,__FUNCTION__,__LINE__);
#endif


    if(hd==NULL)
        return -1;
            
    if(hd->filename[0]=='\0')
        return 0;
#ifdef RECORD_PS_FILE
    ps_write_end(ps_fd, head_buf, ps_len, 256*1024);
    printf("ps_len in end is  %x\n", ps_len);
    fwrite(head_buf, sizeof(unsigned char),ps_len, OUT_FP);
    fflush(OUT_FP);
    fclose(OUT_FP);
    ps_release(ps_fd);
#endif
    aviinfo=hd->aviinfo;
    if(aviinfo == NULL)
    {
        return 0;
    }
    if(aviinfo->fdes>0)
    {
        AVI_close(aviinfo);
        hd->aviinfo=NULL;   ///added by shixin
#ifdef SHOW_WORK_INFO
        printf("closing avi file:%s\n",hd->filename);
#endif
        if(hdutil_filename2finfo(hd->filename,&finfo)<0)//把fname转化为finfo
        {
            printf("close_record_file->filename2finfo error!\n");
            gtloginfo("关闭文件%s错误:%s\n",hd->filename,strerror(errno));  
        }
        else
        {
            finfo.len=hd->filelen;
        
            if(finfo2filename(&finfo,newname)<0)//将文件信息转化成newname失败
            {
                perror("close_record_file & make new name \n");
                gtloginfo("关闭文件%s创建新文件名错误:%s\n",hd->filename,strerror(errno));  
            }
            else
            {
                if((hd->state==2)||(finfo.trig!=0)||(finfo.remote==1))//手工录像或触发报警时
                {
                    hdutil_lock_filename(newname,newname);//锁住文件
                }
                    
                if(hd->queryflag==1) ////wsy,若有被查询标记则之后改变文件名的长度部分
                {       
                    modify_queried_filename(newname,newname);
                    hd->queryflag=0;
                }
            
                if(fileindex_rename_in_partition(hd->partition,hd->filename,newname)<0)
                {
                    error=errno;
                    perror("rename file:");
                    gtlogerr("重命名avi文件%s->%s失败,%s\n",hd->filename,newname,strerror(error));
                    fix_disk(hd->filename,error);
                    return -1;
                }

                #ifdef ZW_DB_TEST
                gettimeofday(&tv,NULL);
                gtloginfo("[%ld:%ld]entering [%s:%d]重命名数据库中文件名完毕\n",tv.tv_sec,tv.tv_usec,__FUNCTION__,__LINE__);
                #endif
                
            }
        }
            
    }
    hd->filename[0]='\0';
    hd->cutflag=0;
    hd->filelen=0;
    hd->aviinfo = NULL;

#if ZW_DB_TEST
    gettimeofday(&tv,NULL);
    gtloginfo("[%ld:%ld]leaving [%s:%d]\n",tv.tv_sec,tv.tv_usec,__FUNCTION__,__LINE__);
#endif


    return 0;
    
}

//启动录音线程
int start_audio_thread(struct hd_enc_struct *hd_new)
{
    pthread_attr_t  thread_attr,*attr;
    int ret;

    if(hd_new==NULL)
        return -1;
    if((int)(hd_new->audio_thread_id)>0)//已经启动
    {
        return 0;
    }
    if(get_gtthread_attr(&thread_attr)==0)
        attr=&thread_attr;
    else
        attr=NULL;
    ret=pthread_create(&hd_new->audio_thread_id,attr,record_audio_thread,(void*)hd_new);    //启动第0路录音线程
    if(attr!=NULL)
    {
        pthread_attr_destroy(attr);
    }
    if(ret==0)
    {
        gtloginfo("成功启动%d通道录音线程,thread_id=%d\n",hd_new->audiochannel,(int)(hd_new->audio_thread_id));
        return 0;
    }
    else 
    {
        printf("start record_audio_thread  problem!!\n");
        gtlogerr("启动%d通道录音线程失败\n",hd_new->audiochannel);
        return -1;
    }   
}


//启动高清晰录像线程
int start_recordfilethread(struct hd_enc_struct *hd_new)
{

    int ret;
    int intid;
    pthread_attr_t  thread_attr,*attr;
    
    if(hd_new==NULL)
        return -1;
    
    intid=(int)hd_new->thread_id;
    if(intid>0)//已经启动
        return 0;
    
    if(get_gtthread_attr(&thread_attr)==0)
        attr=&thread_attr;
    else
        attr=NULL;
    
    pthread_mutex_lock(&hd_new->mutex);
    hd_new->hdsave_mode=1;
    pthread_mutex_unlock(&hd_new->mutex);

    hd_new->threadexit = 0;
    ret=pthread_create(&hd_new->thread_id,attr,record_file_thread,(void*)hd_new);   //启动第0路压缩线程
    if(attr!=NULL)
    {
        pthread_attr_destroy(attr);
    }
    if(ret==0)
    {
        gtloginfo("成功启动%d通道录象线程工作于%s,thread_id=%d\n",hd_new->channel,hd_new->partition,(int)hd_new->thread_id);
        return 0;
    }
    else 
    {
        printf("start record_file_thread  problem!!\n");
        gtlogerr("启动%d通道录象线程失败\n",hd_new->channel);
        return -1;
    }   
}

//停止高清晰录像线程
int stop_recordfilethread(struct hd_enc_struct *hd_new)
{
    int ret;
    int intid;
    int i;
    
    if(hd_new == NULL)
        return -1;
    intid = (int)hd_new->thread_id;
    if(intid <= 0)//已经被停止
        return 0;
    
    hd_new->threadexit = 1;



    /*程序正常退出*/
    usleep(500000);
    intid = (int)hd_new->audio_thread_id;
    if(intid > 0)
    {
        ret=pthread_cancel(hd_new->audio_thread_id);
        if (ret!=0)
        {
            printf("cancel audio !\n");
            gtlogerr("取消音频通道%d录象线程%d失败%d\n",hd_new->audiochannel,hd_new->audio_thread_id,ret);
        }
        printf("强制关闭音频 %d  !!!!!!\n",hd_new->channel);
    }
    else
    {
        printf("音频 %d  正常退出!!!!!!\n",hd_new->channel);
    }

        
    intid = (int)hd_new->thread_id;
    if(intid <= 0)
    {
         printf("视频 %d  正常退出 !!!!!!\n",hd_new->channel);
         return 0;
    }
    printf("视频 %d  强制退出!!!!!!\n",hd_new->channel);
    /*无法正常停止，需要强制关闭*/
    pthread_mutex_lock(&hd_new->file_mutex);
    close_record_file(hd_new);
    pthread_mutex_unlock(&hd_new->file_mutex);  
    
    ret=pthread_cancel(hd_new->thread_id);
    if (ret!=0)
    {
        printf("cancel record_file_thread error!\n");
        gtlogerr("取消通道%d录象线程%d失败%d\n",hd_new->channel,intid,ret);
        return -2;
    }   

    disconnect_video_record_enc(hd_new->channel);
    pthread_mutex_lock(&hd_new->mutex);
    hd_new->thread_id=-1;
    hd_new->hdsave_mode=0;
    pthread_mutex_unlock(&hd_new->mutex);
    gtloginfo("成功取消录象线程");

    return 0;   
}

//停止高清晰录像线程
int stop_Allrecordfilethread()
{
    int ret;
    int intid;
    int i;
    struct hd_enc_struct *hd_new;

    //查找设备一共有几个高清录像通道，3022系列有2个，3024有1个,ip1004有1个
    for(i=0; i<MAX_RECORD_CHANNEL_M; i++)
    {
        hd_new = get_hdch(i);
        if(hd_new == NULL)
            return -1;
        intid = (int)hd_new->thread_id;
        if(intid <= 0)//已经被停止
            return 0;
        
        hd_new->threadexit = 1;

    }

    /*程序正常退出*/
    usleep(500000);
    for(i = 0; i < MAX_RECORD_CHANNEL_M; i++)
    {
        printf("检查第%d通道音视频是否关闭 \n",i);
        hd_new = get_hdch(i);
        if(hd_new == NULL)
            return -1;

        intid = (int)hd_new->audio_thread_id;
        if(intid > 0)
        {
            ret=pthread_cancel(hd_new->audio_thread_id);
            if (ret!=0)
            {
                printf("cancel audio !\n");
                gtlogerr("取消音频通道%d录象线程%d失败%d\n",hd_new->audiochannel,hd_new->audio_thread_id,ret);
            }
            printf("强制关闭音频 %d  !!!!!!\n",hd_new->channel);
            hd_new->audio_thread_id = -1;
        }
        else
        {
            printf("音频 %d  正常退出!!!!!!\n",hd_new->channel);
        }

        
        intid = (int)hd_new->thread_id;
        if(intid <= 0)
        {
             printf("视频 %d  正常退出 !!!!!!\n",hd_new->channel);
             continue;
        }
        printf("视频 %d  强制退出!!!!!!\n",hd_new->channel);
        /*无法正常停止，需要强制关闭*/
        pthread_mutex_lock(&hd_new->file_mutex);
        close_record_file(hd_new);
        pthread_mutex_unlock(&hd_new->file_mutex); 
        printf("准备kill 视频 %d !!!!!!\n",hd_new->channel);
        ret=pthread_cancel(hd_new->thread_id);
        if (ret!=0)
        {
            printf("cancel record_file_thread error!\n");
            gtlogerr("取消通道%d录象线程%d失败%d\n",hd_new->channel,intid,ret);
//            continue;
        }   

        printf("断开视频 %d 编码器!!!!!!\n",hd_new->channel);
        disconnect_video_record_enc(hd_new->channel);
        pthread_mutex_lock(&hd_new->mutex);
        hd_new->thread_id=-1;
        hd_new->hdsave_mode=0;
        pthread_mutex_unlock(&hd_new->mutex);
        gtloginfo("成功取消录象线程");

    }

    return 0;   
}

//被调用来中止线程的函数
//传过来的结构为空或在cancel过程中出错则返回-1;成功则返回0;若在start过程中出错返回-2
int restart_recordfilethread(struct hd_enc_struct *hd_new)
{
    int ret;
    
    if(hd_new==NULL)
        return (-1);
    
    ret=stop_recordfilethread(hd_new);
    if(ret<0)
        return -1;
    sleep(1);       //safe
    hd_new->threadexit = 0;
    start_audio_thread(hd_new);
    usleep(500000);
    ret=start_recordfilethread(hd_new);
     /*根据配置的参数启动音频录像*/

    if(ret<0)
        return -2;
    return 0;    
} 
//被调用的清场函数,录音线程
void record_audio_thread_cleanup(void *para)
{
    disconnect_audio_rec_enc(0);
    get_hdch(0)->audio_thread_id=-1;
}
//被调用的清场函数
void record_file_thread_cleanup(void *para)
{
    struct hd_enc_struct *hd_enc;
    struct msg dmsg;
    int qid;

    gtloginfo("test,清场\n");
    if(para==NULL)
        return;
    
    hd_enc=(struct hd_enc_struct*)para;
    qid=hd_enc->qid;
    //清空队列
    while((msgrcv(qid,&dmsg,100,1,IPC_NOWAIT))>0)
    {       
    }
    //关闭当前文件
    pthread_mutex_lock(&hd_enc->file_mutex);
    close_record_file(hd_enc);
    pthread_mutex_unlock(&hd_enc->file_mutex);
    hd_enc->watchcnt=0;

    disconnect_video_record_enc(hd_enc->channel);
    change_thread_id(hd_enc->channel,-1);//added by shixin 
    
}

//改变线程id的函数
void change_thread_id(int channel,int id) 
{
    struct hd_enc_struct *hd;
    hd=get_hdch(channel);
    pthread_mutex_lock(&hd->mutex);
    hd->thread_id=id;
    pthread_mutex_unlock(&hd->mutex);
}


//管理I帧缓冲池,供抓图使用的线程
void *keyframe_pool_thread(void *hd)
{
    struct hd_enc_struct    *hd_enc=NULL;
    int ret=0;
    struct stream_fmt_struct *frame=NULL;
    int buflen;
    int first_flag=1;
    int old_seq=-1,new_seq=-1;
    int video_flag;
    int stat_err_cnt=0;
    struct pool_ele_struct *freerm=NULL;
    struct pool_ele_struct *recdata=NULL;
    struct timeval *tm=NULL;
    
    if(hd==NULL)
    {
        //remed by shixin change_thread_id(hd_enc->channel,-1);
        return NULL;
    }
    hd_enc=(struct hd_enc_struct*)hd;
    gtloginfo(" start channel %d keyframe_pool_thread...\n",hd_enc->channel);   

//  frame=(struct stream_fmt_struct*)hd_enc->keyframebuf;
//  buflen=hd_enc->fblen-sizeof(struct stream_fmt_struct);//防止缓冲区溢出
    frame=get_video_read_keyframe_buf(get_hqenc_video_ch(hd_enc->channel));
    if(frame==NULL)
    {
        //remed by shixin change_thread_id(hd_enc->channel,-1);
            gtloginfo("通道%d(I)frame为空，返回NULL.......\n",hd_enc->channel);
            sleep(1);
            return NULL;
    }
    buflen=get_video_read_keyframe_buf_len(get_hqenc_video_ch(hd_enc->channel));
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); //延迟取消到某时间点
//remed by shxin 不能用录像线程的清场函数pthread_cleanup_push(record_file_thread_cleanup,(void *)hd_enc);//设置清场函数
    pthread_testcancel();

    /*多次连接缓冲池直到成功*/
    while(1)
    {
        ret=connect_video_enc_keyframe(get_hqenc_video_ch(hd_enc->channel),"hdmodule-pool");
        if(ret==0)
        {
            hd_enc->picerrorcode=0;
#ifdef SHOW_WORK_INFO
            printf("connect videoenc success\n");
#endif
            gtloginfo("%d通道I帧缓冲池线程成功连接视频编码器%d\n",hd_enc->channel,get_hqenc_video_ch(hd_enc->channel));
            break;
        }
        else
        {
            if(hd_enc->picerrorcode==0)
                {
                    gtloginfo("%d通道I帧缓冲池线程连接视频编码器%d失败\n",hd_enc->channel,get_hqenc_video_ch(hd_enc->channel));
                    hd_enc->picerrorcode=ERR_EVC_NOT_SUPPORT;
                }
            printf("connect video enc failed, ret=%d!!\n",ret);
            sleep(1);
        }
    }

    while(1)
    {
        ret=get_venc_stat_keyframe(get_hqenc_video_ch(hd_enc->channel));
        if(ret==ENC_STAT_OK)
        {
            printf("%d通道I帧缓冲池视频编码器%d状态正常!\n",hd_enc->channel,get_hqenc_video_ch(hd_enc->channel));
            gtloginfo("%d通道I帧缓冲池视频编码器%d状态正常!\n",hd_enc->channel,get_hqenc_video_ch(hd_enc->channel));
            break;
        }
        else
        {
            if(++stat_err_cnt==15)
            {
                printf("%d通道I帧缓冲池视频编码器%d状态异常,stat=%d!\n",hd_enc->channel,get_hqenc_video_ch(hd_enc->channel),ret);
                gtlogerr("%d通道I帧缓冲池视频编码器%d状态异常,stat=%d!\n",hd_enc->channel,get_hqenc_video_ch(hd_enc->channel),ret);
            }
            printf("videoenc%d state=%d!!!\n",get_hqenc_video_ch(hd_enc->channel),ret); 
            //gtloginfo("videoenc%d state=%d!!!\n",get_hqenc_video_ch(hd_enc->channel),ret);    
        }
        sleep(1);
    }
    
    while(1)
    {
        do  /*读到I帧为止*/
        {
            ret=read_video_keyframe(get_hqenc_video_ch(hd_enc->channel),frame,buflen,&new_seq,&video_flag);
            if(ret<0)
            {
                printf("read_video_keyframe failed ret=%d\n",ret);
                usleep(100000);
                continue;   
            }
            hd_enc->keyframe_cnt=0;
            if(video_flag==FRAMETYPE_I)
            {
                first_flag=0;
                old_seq=new_seq;
            }
            else
            {
                old_seq++;
                if(old_seq!=new_seq)
                {                   
                    printf("I thread read_video_frame old_seq+1=%d newseq=%d video_flag=%d!!\n",old_seq,new_seq,video_flag);
                    //shixin remed if(old_seq!=0)
                    //  gtloginfo("I thread read_video_frame old_seq+1=%d newseq=%d video_flag=%d!!\n",old_seq,new_seq,video_flag);
                    old_seq=new_seq;                    
                    first_flag=1;
                    continue;
                }
            }
        }while(first_flag);


        if(is_keyframe(frame))
        {
            if((frame->len+14) > MAX_FRAME_SIZE)//容量检查,太大的I帧就不要了
            {
                gtloginfo("该I帧数据量过大，不放入缓冲池\n");
                continue;
            }
            
            tm=(struct timeval *)&frame->tv;
            //把I桢存在队列尾部
            freerm=get_free_eleroom(get_stream_pool(hd_enc->channel));
            if(freerm==NULL)//changed by shixin 如果缓冲池满则扔掉第一个数据
            {
                recdata=get_active_ele(get_stream_pool(hd_enc->channel));//取出第一个元素
                if(recdata==NULL)
                {
                    printf("get eletype failed!!!\n");
                    gtloginfo("从缓冲池%d中获取元素失败!\n",hd_enc->channel);
                }
                else
                {
                    ret=free_ele(get_stream_pool(hd_enc->channel),recdata); //放回缓冲池                    
                }
                freerm=get_free_eleroom(get_stream_pool(hd_enc->channel));
            }
                
            if(freerm!=NULL)
            {
                set_ele_type(freerm,FRAMETYPE_I);
                memcpy(freerm->element,frame,frame->len+sizeof(struct stream_fmt_struct));//14);    //shixin modified
                ret=put_active_ele(get_stream_pool(hd_enc->channel),freerm);//放到有效元素队列尾部
                if((ret==0)&&(hd_enc->semflag==1))
                    sem_post(&hd_enc->sem);
            } 

        }
    }
//  pthread_cleanup_pop(0);
    //remed by shixin change_thread_id(hd_enc->channel,-1);
    disconnect_video_enc_keyframe(get_hqenc_video_ch(hd_enc->channel));
    //gtloginfo("test,-1 here\n");
    return NULL;
}

#if 0
/*连接音视频缓冲池.参数prerec表示从当前时间提前的秒数
  正常返回0,不正常返回负值*/
int connect_media_pre(struct hd_enc_struct *hd_enc,int prerec)
{
    int ret;
    int stat_err_cnt=0;
    int connect_enc_failed=0;
    if(hd_enc==NULL)
        return -1;

    /*多次连接视频缓冲池直到成功*/
    while(1)
    {
        ret=connect_video_enc(get_hqenc_video_ch(hd_enc->channel),"hdmodule",prerec);
        if(ret==0)
        {
            printf("connect videoenc success\n");
            //gtloginfo("%d通道录像线程成功连接视频编码器\n",hd_enc->channel);
            break;
        }
        else
        {
            if(connect_enc_failed==0)
            {
                    gtloginfo("连接视频编码器失败\n");
            }
            connect_enc_failed++;
            printf("connect video enc failed(%d), ret=%d!!\n",connect_enc_failed,ret);
            if(connect_enc_failed==40)
                {
                    gtlogerr("连接视频编码器失败%d次!!!",connect_enc_failed);
                    
                }
            sleep(2);
        }
    }
    while(1)
    {
        ret=get_venc_stat(get_hqenc_video_ch(hd_enc->channel));
        if(ret==ENC_STAT_OK)
        {
            printf("视频编码器%d状态正常!\n",get_hqenc_video_ch(hd_enc->channel));
            //gtloginfo("视频编码器%d状态正常!\n",get_hqenc_video_ch(hd_enc->channel));
            
            break;
            
        }
        else
        {
            if(++stat_err_cnt==15)
            {
                printf("视频编码器%d状态异常,stat=%d!\n",get_hqenc_video_ch(hd_enc->channel),ret);
                gtlogerr("视频编码器%d状态异常,stat=%d!\n",get_hqenc_video_ch(hd_enc->channel),ret);
            }
            printf("videoenc%d state=%d!!!\n",get_hqenc_video_ch(hd_enc->channel),ret); 
            //gtloginfo("videoenc%d state=%d!!!\n",get_hqenc_video_ch(hd_enc->channel),ret);    
        }
        sleep(1);
    }

    //连接音频
    ret=connect_audio_rec_enc(0,"hdmodule",prerec);
    if(ret==0)
    {
        printf("connect audioenc success\n");
        //gtloginfo("连接音频编码器成功\n");
    }
    else
    {
        gtloginfo("连接音频编码器失败\n");
        return 0;
    }
    ret=get_aenc_rec_stat(0);
    if(ret==ENC_STAT_OK)
    {
        printf("音频编码器%d状态正常!\n",0);
        //gtloginfo("音频编码器%d状态正常!\n",0);
    }
    else
    {
        printf("音频编码器%d状态异常,stat=%d!\n",0,ret);
        gtlogerr("音频编码器%d状态异常,stat=%d!\n",0,ret);
    }       
    return 0;
}
#endif


static int a_connect_enc[4]={0};//当前是否连接着视频缓冲池
static int connect_enc[4]={0};//当前是否连接着视频缓冲池

#ifdef USE_FFMPEG_LIB
#include <avcodec.h>
#include <avformat.h>
#include <audiofmt.h>
//将单声道数据转换为双声道数据再乘4
int conv_8km_2_32ks(char *source,char *target,int srclen)
{   
        unsigned short *s=(unsigned short*)source,*t=(unsigned short*)target;
        int i,j;
        int len=srclen/2;
        int conv_len=0;
        for(i=0;i<len;i++)
        {
                for(j=0;j<8;j++)
                {
                        *t=*s;
                        t++;
                }
                s++;
                conv_len++;
        }
        return conv_len*2*8;
}
static unsigned char write_buf[1024*150];
static int write_place=0;
static int read_place=0;
static int usage=0;
int ffmpeg_write_audio_file(AVCodecContext *c,avi_t *AVI,char *data,int len)
{
    int ret;
    int pkt_size=c->frame_size*4;//一次转换的字节数
    unsigned char temp_buf[1024];
//    printf("call ffmpeg_write_audio_file len=%d usage=%d write_place=%d read_place=%d\n",len,usage,write_place,read_place);
    if(usage<=0)
    {
        //printf("usage=%d write=%d read=%d!!\n",usage,write_place,read_place);
        memcpy(write_buf,data,len);
        read_place=0;
        write_place=len;
        usage=len;
    }
    else
    {
        memcpy(&write_buf[write_place],data,len);
        write_place+=len;
        usage+=len;
    }
    while(usage>=pkt_size)
    {
        ret = avcodec_encode_audio(c, temp_buf, sizeof(temp_buf), ( const short *)&write_buf[read_place]);
        ret=AVI_write_audio(AVI,temp_buf,ret);
        usage-=pkt_size;
        read_place+=pkt_size;        
    }
    return 0;
}
#endif
//录声音的线程
void *record_audio_thread(void *hd)
{
#ifdef USE_FFMPEG_LIB
    AVCodecContext *avctx_opts=NULL;
    AVCodec *enc=NULL;
    unsigned char comp_buf[10240];
    unsigned char temp_buf[20480];
    int         frame_size;
#endif
    struct hd_enc_struct    *hd_enc=NULL;
    struct stream_fmt_struct *frame;
    int buflen;
    int ret;
    int old_seq=-1,new_seq=-1;
    int audio_flag;
    int aenc_fail_cnt=0;
    int stat_err_cnt=0;
    int audio_channel;
	int *phead=NULL;

    if(hd==NULL)
    {
        return NULL;
    }
    

    hd_enc=(struct hd_enc_struct*)hd;

    gtloginfo(" start audio thread channel %d video:%d ...\n",hd_enc->audiochannel,hd_enc->channel); 
#ifdef SHOW_WORK_INFO
    printf("enter record audio channel %d video:%d ...\n",hd_enc->audiochannel,hd_enc->channel); 
#endif


    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); //延迟取消到某时间点
    pthread_cleanup_push(record_audio_thread_cleanup,(void *)hd_enc);//设置清场函数
    pthread_testcancel();

    audio_channel = hd_enc->audiochannel;
    frame=get_audio_rec_read_buf(audio_channel);
    buflen=get_audio_rec_read_buf_len(audio_channel);//防止缓冲区溢出
    if(frame==NULL)
    {
        gtloginfo("音频编码器%dframe为空，返回NULL.......\n",0);
        sleep(1);
        hd_enc->audio_thread_id=-1;
        return NULL;
    }
#ifdef USE_FFMPEG_LIB
    if(audio_save_fmt==4)
    {
        /* must be called before using avcodec lib */
            avcodec_init();

        /* register all the codecs (you can also register only the codec
           you wish to have smaller code */
            avcodec_register_all();
        /* find the MP2 encoder */
        enc = avcodec_find_encoder(CODEC_ID_MP2);//);
        if (!enc) {
            fprintf(stderr, "codec not found\n");
            exit(1);
        }
        avctx_opts=avcodec_alloc_context();
        avctx_opts->bit_rate = 64000;
        avctx_opts->sample_rate = 32000;
        avctx_opts->channels = 2;
        frame_size= avctx_opts->frame_size;
        /* open it */
        if (avcodec_open(avctx_opts, enc) < 0) {
            fprintf(stderr, "could not open codec\n");
            exit(1);
        }
    }
        printf("audio_save_fmt=%d!!\n",audio_save_fmt);
#endif
    while(!hd_enc->threadexit)
    {   

        //printf("audio_start \n");

        printf("audio wait audio channel %d video:%d,!!!!!!!!!!!!!!!!!!!!\n",hd_enc->audiochannel,hd_enc->channel); 
        pthread_mutex_lock(&hd_enc->audio_mutex);
        pthread_cond_wait(&hd_enc->audio_cond,&hd_enc->audio_mutex);
        pthread_mutex_unlock(&hd_enc->audio_mutex);

        //printf("audio_ connect_audio_rec_enc\n");
        printf("audio connect channel %d video:%d ...\n",hd_enc->audiochannel,hd_enc->channel); 
        /*多次连接音频缓冲池直到成功*/
        while((!hd_enc->threadexit) && (!a_connect_enc[hd_enc->channel]))
        {//连接音频编码器
            ret=connect_audio_rec_enc(audio_channel,"hdmodule",hd_enc->pre_connect);
            if(ret==0)
            {
                printf("connect audioenc%d success\n",0);
                gtloginfo("连接音频编码器%d 成功!\n",0);
                break;
            }
            else
            {
                if(aenc_fail_cnt==0)
                {
                        gtloginfo("连接音频编码器%d失败\n",0);
                }
                aenc_fail_cnt++;
                printf("connect audio enc failed(%d), ret=%d!!\n",aenc_fail_cnt,ret);
                if(aenc_fail_cnt==40)
                {
                    gtlogerr("连接音频编码器失败%d次!!!",aenc_fail_cnt);
                    
                }
                sleep(2);
            }
        }  

        //printf("audio_ connect_audio_rec_enc over\n");
        while(!hd_enc->threadexit)
        {//等待音频编码器正常
            ret=get_aenc_rec_stat(audio_channel);
            if(ret==ENC_STAT_OK)
            {
                gtloginfo("音频编码器%d状态正常!\n",0);
                printf("音频编码器%d状态正常!\n",0);
                break;
            }
            else
            {
                if(++stat_err_cnt==15)
                {
                    printf("音频编码器%d状态异常,stat=%d!\n",0,ret);
                    gtlogerr("音频编码器%d状态异常,stat=%d!\n",0,ret);
                }
                printf("audioenc%d state=%d!!!\n",0,ret);   
            }
            sleep(1);
        }

        printf("read audio frame channel %d video:%d ...\n",hd_enc->audiochannel,hd_enc->channel); 
        old_seq=-1;
        while(connect_enc[hd_enc->channel]&&(!hd_enc->threadexit))//已连接上视频编码器
        {

            ret=read_audio_rec_frame(audio_channel,frame,buflen,&new_seq,&audio_flag);
            if(ret<0)
            {
                printf("test,read_audio_frame failed ret=%d\n",ret);
                usleep(100000);
                continue;   //changed by shixin break;
            }
            hd_enc->audio_cnt=0;
            old_seq++;
            if(old_seq!=new_seq)
            {
                printf("read_audio_frame old_seq+1=%d newseq=%d\n",old_seq,new_seq);
                old_seq=new_seq;                    
                continue;
            }
            
            //printf("read_audio_frame ret=%d,len:%d hd_enc->threadexit:%d \n",ret,frame->len,hd_enc->threadexit);

            pthread_mutex_lock(&hd_enc->file_mutex);
            if(hd_enc->aviinfo!=NULL)                   ///added by shixin
            {
#ifdef USE_FFMPEG_LIB   
            if(audio_save_fmt==4)
            {
                ret=conv_ulaw2raw(comp_buf,frame->data,frame->len);
                ret=conv_8km_2_32ks(comp_buf, temp_buf,ret);   
                ffmpeg_write_audio_file(avctx_opts,hd_enc->aviinfo,temp_buf,ret);
            }
            else
                ret=AVI_write_audio(hd_enc->aviinfo,frame->data,frame->len);
#else

			phead = (int*)frame->data;
			if (*phead == 0x77061600)
			{
				//lc add 20150402  aac raw data ,add adts header for avi to play
				fix_adts_header(frame->data,frame->len);
				
				ret=AVI_write_audio(hd_enc->aviinfo,frame->data+8-7,frame->len-8+7);
			}else{
                ret=AVI_write_audio(hd_enc->aviinfo,frame->data,frame->len);
			}
#endif
            }
            pthread_mutex_unlock(&hd_enc->file_mutex);
    
        }
        //视频编码器已经断开

        //printf("disconnect_audio_rec_enc hd_enc->threadexit:%d \n",hd_enc->threadexit);
        //ret=disconnect_audio_rec_enc(audio_channel);    //也断开音频编码器
    }
    printf("audio thread %d run over\n",hd_enc->channel);
    pthread_cleanup_pop(0);
    pthread_mutex_lock(&hd_enc->audio_mutex);
    hd_enc->audio_thread_id=-1;
    pthread_mutex_unlock(&hd_enc->audio_mutex);
    disconnect_audio_rec_enc(audio_channel);
    //printf("audio over here\n");
    return NULL;
}

//文件记录线程
void *record_file_thread(void *hd)
{       
    struct hd_enc_struct    *hd_enc=NULL;
    struct stream_fmt_struct *frame=NULL;
    int buflen;
    int first_flag=1;
    int state=0,closeflag=0,newstate=0;//state=0表示没有录像 1表示预录 2表示报警录像
    int ret;
    int old_seq=-1,new_seq=-1;
    int video_flag;
    int err,disknum;
#ifdef RECORD_PS_FILE
    int frametype;
#endif
    time_t end_last_file=0; //上个文件结束的时间点
    time_t timenow=0;
    avi_t *aviinfo=NULL;


    if(hd==NULL)
    {
        return NULL;
    }

    hd_enc = (struct hd_enc_struct*)hd;
    /*如果没有硬盘，录像线程也要退出*/
    disknum =  get_sys_disk_num();
    if(disknum ==  0)
    {
        printf("no disk ,record thread return...\n");
        gtloginfo("没有找到硬盘，录像线程启动后退出...");     
        return NULL;
    }

    hd_enc = (struct hd_enc_struct*)hd;
    gtloginfo(" start channel %d record_file_thread...\n",hd_enc->channel);     

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); //延迟取消到某时间点
    pthread_cleanup_push(record_file_thread_cleanup,(void *)hd_enc);//设置清场函数
    pthread_testcancel();
    frame=get_video_record_buf(hd_enc->channel);
    buflen=get_video_record_buf_len(hd_enc->channel);//防止缓冲区溢出
    
    if(frame==NULL)
    {
        change_thread_id(hd_enc->channel,-1);
        gtloginfo("通道%d frame为空，返回NULL.......\n",hd_enc->channel);
        sleep(1);
        return NULL;
    }   
    
start:

    //printf("1111111111111111111111channel %d threadexit=%d\n",hd_enc->channel,hd_enc->threadexit);
    while(!hd_enc->threadexit)
    {   

        newstate=hd_enc->state;
        switch(state)
        {
            case 0://什么也不需要做，
                if(newstate!=0)
                {
                    if(hd_enc->rec_type==0)
                    {
                        hd_enc->pre_connect=0;
                    }   
                    else
                    {
                        timenow=time((time_t *)NULL);
                        if((timenow-end_last_file)>=MOTION_PRE_REC)
                            hd_enc->pre_connect=MOTION_PRE_REC;
                        else
                            hd_enc->pre_connect=(timenow-end_last_file);
                    }
                    ret = connect_video_record_enc_succ(hd_enc->channel,"hdmodule",hd_enc->pre_connect);
                    ret = connect_audio_rec_enc(hd_enc->audiochannel,"hdmodule",hd_enc->pre_connect);//连接音频编码器,异常处理在录音线程中
                    if(ret == 0)
                    {
                        a_connect_enc[hd_enc->channel] = 1;
                    }
                   
                    state=newstate;
                    if(newstate == 2)
                    {
                        lock_recent_file(hd_enc);
                        
                    }
                    connect_enc[hd_enc->channel] = 1;
                    pthread_mutex_lock(&hd_enc->audio_mutex);
                    pthread_cond_signal(&hd_enc->audio_cond);
                    pthread_mutex_unlock(&hd_enc->audio_mutex);
                }

            break;
            case 1://预录 
                    if (newstate==2) //将被触发
                        lock_recent_file(hd_enc);
            break;
            case 2://触发录像
            break;
            default:
            break;
        }
        if(connect_enc[hd_enc->channel] != 1)
        {
            sleep(1); //在没有连接缓冲池的情况下,避免空循环占用太多cpu
            continue;
        }
        
        do
        {

            hd_enc->readenc_flag = 1;
            ret=read_video_record_frame(hd_enc->channel,frame,buflen,&new_seq,&video_flag);
            if(ret<0)
            {
                printf(" read video %d record frame failed ret=%d\n",hd_enc->channel,ret);
                usleep(100000);
                continue;
            }
            //printf("video_frame,read:%d,framelen:%d seq:%x,video_flag:%d!\n",ret,frame->len,new_seq,video_flag);
            ret=get_video_record_enc_remain(hd_enc->channel);
            if((ret>=100)&&((ret%10) == 0))//每10条记录一次
            {
                printf("warn:%d record_file_thread 有%d帧视频没有读取!\n",hd_enc->channel,ret);
                gtlogwarn("warn:record_file_thread 有%d帧视频没有读取!\n",hd_enc->channel,ret);
            }

            hd_enc->readenc_flag = 0;
            hd_enc->watchcnt=0;

            if(video_flag==FRAMETYPE_I)
            {
                //这里将first_flag=0的目的是想退出while(first_flag),否则将会返回重复调用read_video_frame(),直到获取的是I帧为止
                first_flag=0;
                old_seq=new_seq;
            }
            else
            {
                old_seq++;
                if(old_seq!=new_seq)
                {                   
                    printf("read %d video_frame old_seq+1=%d newseq=%d video_flag=%d!!\n",hd_enc->channel,old_seq,new_seq,video_flag);
                    old_seq=new_seq;                    
                    first_flag=1;
                    continue;
                }
            }
        }while(first_flag&&(!hd_enc->threadexit));
        /*读到了I帧*/

        hd_enc->watchcnt=0;     

        closeflag=0;
        if(hd_enc->cutflag)
        {
            //查看文件是否需要切分了，当文件超过预设大小后需要通过切分来关闭文件
            closeflag=1;    
            //gtloginfo("test,cutflag close\n");
        }
        else if ((state!=newstate)&&(state!=0))
        {
            /*录像的状态改变，对应的文件也要跟着改变*/
            closeflag=1;
            //gtloginfo("test,state close,state=%d,newstate =%d\n",state,newstate);
        }
    
        state=newstate; 

         /*如果有关闭的动作，则立即关闭*/
         pthread_mutex_lock(&hd_enc->file_mutex);    
        if((closeflag)&&(hd_enc->aviinfo != NULL))
         {
             close_record_file(hd_enc);
         }
         pthread_mutex_unlock(&hd_enc->file_mutex); 
    
        //为啥使用这个接口，使用video_flag简单的判断不行吗?
        if(is_keyframe(frame)&&(!hd_enc->threadexit))
        {
            /*当前的帧需要录像*/
            if(newstate!=0)
            {
                pthread_mutex_lock(&hd_enc->file_mutex);    
                if(hd_enc->aviinfo == NULL)
                {
                        //现在是多分区
                    if(get_disk_free(hd_enc->partition) < get_hd_minval()) 
                    {
#if 0                    
                        /*获取下一个录像分区*/
                        if(disk_get_next_record_partition(hd_enc->partition) < 0)
                        {
#endif                        
                            /*如果四个盘都满，或者硬盘没有mount上，退出录像程序*/
                            hd_enc->threadexit = 1;
                            pthread_mutex_unlock(&hd_enc->file_mutex); 
                            gtloginfo("通道%d 分区空间都不足，停止录像，退出录像线程\n", hd_enc->channel);
                            goto start;
#if 0                            
                        }
#endif                        
                        
                    }

                    printf("create_record_file channel %d  path:%s \n",hd_enc->channel,hd_enc->partition);

                    //如果当前状态处于非空闲状态，关闭后还需要再新开一个文件继续录像
                    errno=0;
                    aviinfo=create_record_file(hd_enc);
                    hd_enc->aviinfo = aviinfo;
                    if(aviinfo == NULL)
                    {
                        err=errno;
                        printf("通道%d 创建录像文件失败,返回avi_t为空，error = %s\n",hd_enc->channel,strerror(err));
                        if(err!=ENOSPC)
                        {
                            gtloginfo("通道%d 创建录像文件失败,返回avi_t为空，error = %s\n",hd_enc->channel,strerror(err));
                        }
                        pthread_mutex_unlock(&hd_enc->file_mutex);  
                        disconnect_video_record_enc(hd_enc->channel);  //added by shixin 
                        sleep(1);
                        state=0;
                        goto start;
                    }
                }

                //printf("read_record_file channel %d  file:%s \n",hd_enc->channel,hd_enc->filename);
                //将当前的这个I帧写到AVI文件中去
                //printf("AVI_write_frame,write framelen:%d seq:%x,video_flag:%d!\n",frame->len,new_seq,video_flag);
                ret=AVI_write_frame(aviinfo,frame->data,frame->len,is_keyframe(frame)); //shixin added 
                pthread_mutex_unlock(&hd_enc->file_mutex); 
#ifdef RECORD_PS_FILE
                if(is_keyframe(frame))
                    frametype = I_FRAME;
                else
                    frametype = P_FRAME;
                ps_len=ps_write_frame(ps_fd, V_STREAM, frame->data, frame->len, head_buf,1024*256);
                //printf("wsytest,ps_len = %d\t,framelen %d\n",ps_len,frame->len);
                fwrite(head_buf, sizeof(unsigned char),ps_len, OUT_FP);
                fflush(OUT_FP);
#endif
                continue;
            }
            else
            {
                if(get_venc_attrib_record(hd_enc->channel)!=NULL)
                {
                    ret=disconnect_video_record_enc(hd_enc->channel);
                    connect_enc[hd_enc->channel]=0;
                    end_last_file=time((time_t *)NULL);                         
                    old_seq=-1;
                    new_seq=-1;
                }
            }
                
        }
        

//printf("3333333333333333333333  channel %d threadexit=%d\n",hd_enc->channel,hd_enc->threadexit);        
      //这里处理的是一般的情况，非keyframe都会从这里被写入到avi文件中去
        if((state!=0)&&(!hd_enc->threadexit))
        {
        
            //printf("AVI_write_frame,write framelen:%d seq:%x,video_flag:%d!\n",frame->len,new_seq,video_flag);
            pthread_mutex_lock(&hd_enc->file_mutex);
            if(hd_enc->aviinfo != NULL)
            {
                ret=AVI_write_frame(aviinfo,frame->data,frame->len,is_keyframe(frame)); 
            }
            pthread_mutex_unlock(&hd_enc->file_mutex);
#ifdef RECORD_PS_FILE
            if(is_keyframe(frame))
                frametype = I_FRAME;
            else
                frametype = P_FRAME;
            ps_len=ps_write_frame(ps_fd, V_STREAM, frame->data, frame->len, head_buf,1024*256);
            //printf("wsytest,ps_len = %d\t,framelen %d\n",ps_len,frame->len);
            fwrite(head_buf, sizeof(unsigned char),ps_len, OUT_FP);
            fflush(OUT_FP);
#endif
        }
                   
    }
    /*关闭录像文件*/
    pthread_mutex_lock(&hd_enc->file_mutex); 
    close_record_file(hd_enc);
    pthread_mutex_unlock(&hd_enc->file_mutex); 
    
    pthread_cleanup_pop(0);
    change_thread_id(hd_enc->channel,-1);
    disconnect_video_record_enc(hd_enc->channel);
    printf("record file thread %d run over\n",hd_enc->channel);
    return NULL;
}


