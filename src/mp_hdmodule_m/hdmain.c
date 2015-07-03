#include "mp_hdmodule.h"
#include "hdmod.h"
#include <signal.h>
#include <devinfo.h>
#include "process_modcmd.h"
#include <commonlib.h>
#include <venc_read.h>
#include <gtthread.h>
#include <aenc_read.h>
#include "diskinfo.h"
#include "mpdisk.h"
#include "hdutil.h"


//获取一个默认属性结构
//用完了应该释放
int get_gtthread_attr(pthread_attr_t *attr)
{
    int rc;
    if(attr==NULL)
        return -1;
    memset((void*)attr,0,sizeof(pthread_attr_t));
    rc=pthread_attr_init(attr);
    if(rc<0)
        return -1;
    rc=pthread_attr_setdetachstate(attr,PTHREAD_CREATE_DETACHED);//分离状态
    rc=pthread_attr_setschedpolicy(attr,SCHED_OTHER);
    return 0;
    
}


void second_proc(void)
{
    while(1)
    {
        sleep(1);
        hd_second_proc();
    }
}



void dump_sysinfo(void)
{
    return;
    //FIXME 加以补充
}

//在日志上记录退出过程
static void exit_log(int signo)
{
    switch(signo)
    {
        case SIGPIPE:
            printf("hdmodule process_sig_pipe \n"); 
            return ;
        break;
        case SIGTERM:
            gtloginfo("hdmodule 被kill,程序退出!!\n");
            stop_Allrecordfilethread();
            close_all_res();
            exit(0);
        break;
        case SIGKILL:
            gtloginfo("hdmodule SIGKILL,程序退出!!\n");
            stop_Allrecordfilethread();
            close_all_res();
            exit(0);
        break;
        case SIGINT:
            gtloginfo("hdmodule 被用户终止(ctrl-c)\n");
            stop_Allrecordfilethread();
            close_all_res();
            exit(0);
        break;
        case SIGUSR1://输出系统信息到指定文件
            dump_sysinfo();
        break;
        case SIGSEGV:
            gtloginfo("#########hdmodule 发生段错误#########\n");
            stop_Allrecordfilethread();
            close_all_res();
            printf("hdmodule segmentation fault\n");
            exit(0);
        break;
    }
    return;
}

#ifdef FOR_PC_MUTI_TEST
#include "pc_multi_test.c"
#endif
int process_opt_h(void)
{
    printf("录像服务程序version:%s\n",VERSION);
    printf("用法:hdmodule [OPTION] [argument]\n");
    printf("OPTION 选项说明\n");
    printf("-h:显示帮助信息\n");
       printf("-v:显示版本信息并退出程序\n");
    return 0;
}
int process_argument(int argc,char **argv)
{
    int oc;
    if(argc<2)
    {
        return 0;
    }
    printf("*************************************************\n");
    while((oc=getopt(argc,argv,"hv"))>=0)
        {
                switch(oc)
                {
            case 'h':
                process_opt_h();
                exit(0);
            break;
                     case 'v':
                            printf("hdmodule version:%s\n",VERSION);
                            create_lockfile_save_version(HDMOD_LOCK_FILE,VERSION);
                            printf("*************************************************\n");
                            exit(0);
                     break;
            default:
            break;
                }
        }

    printf("*************************************************\n\n\n");
    return 0;
}

int main(int argc,char *argv[])
{

    pthread_attr_t  thread_attr,*attr;
    int lock_file;
    char pbuf[100];
    int i,ret;

    
#ifdef FOR_PC_MUTI_TEST
    gen_multi_devices();
#endif
    gtopenlog("hdmodule");
    setsid();
    process_argument(argc,argv);    
    lock_file=create_and_lockfile(HDMOD_LOCK_FILE);
    if(lock_file<=0)
    {
        printf("hdmodule module are running!!\n");
        gtlogerr("hdmodule模块已运行，故启动无效退出\n");       
        exit(0);
    }   
    sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
    write(lock_file,pbuf,strlen(pbuf)+1);//将进程的id号存入锁文件中
    //以上完成判断模块是否已经执行的功能
    #ifdef FOR_PC_MUTI_TEST
    SetOutputTty(lock_file);// 将输出影射到另一个描述符
    #endif

    signal(SIGKILL,exit_log);
    signal(SIGTERM,exit_log);
    signal(SIGINT,exit_log);
    signal(SIGSEGV,exit_log);
    signal(SIGPIPE,exit_log);
    printf("启动hdmodule(ver:%s).......\n",VERSION);
    gtloginfo("启动hdmodule(ver:%s).......\n",VERSION);
#if EMBEDED
    system("rm -f /tmp/*.widx");//清理之前的临时文件
#endif
    init_devinfo();     //读取设备信息，guid等
    ret = init_video_record_enc_all();//初始化视频缓冲池
    if(ret != 0)
    {
        gtloginfo("init_video_record_enc error\n");
    }
    ret = init_audio_rec_enc_all();
    if(ret != 0)
    {
        gtloginfo("init_video_record_enc error\n");
    }
    init_com_channel();
    if(get_gtthread_attr(&thread_attr)==0)
        attr=&thread_attr;
    else
        attr=NULL;


    //fileindex_init_filelock();  //zw-add
    InitAllDabase();

    creat_hdmodule_modsocket_thread();


    init_hdenc();   //应放在init_tw2824()之前调用 否则单路高清晰录像的视频通道初始化会出错
  
    get_trig_status(); //取得并设置trig状态
    get_save_status(); //取得并设置save状态



    //先转换ing文件，然后再进行硬盘空间的转换，否则如果磁盘剩余空间过小的话hdmodule就会阻塞在下面的循环中，而diskman那边又不能从数据库文件中
    //找出符合ing=0的可删除文件，因为有时会存在有很多ing=1的情况，这样就需要先将这些ing文件转换为avi文件，然后再删除。
    convert_old_ing_files();


    //查找设备一共有几个高清录像通道，3022系列有2个，3024有1个,ip1004有1个
    for(i=0; i<MAX_RECORD_CHANNEL_M; i++)
    {
        if(get_hdch(i)->enable == 1)
        {

             /*根据配置的参数启动音频录像*/
            start_audio_thread(get_hdch(i));
            usleep(500000);
            start_recordfilethread(get_hdch(i));
            usleep(500000);
        }
    }

    if(attr!=NULL)
    {
        pthread_attr_destroy(attr);
    }
    signal(SIGUSR1,exit_log);
    second_proc();
    exit(1);
}


