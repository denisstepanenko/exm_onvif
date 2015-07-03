#include <signal.h>
#include <devinfo.h>
#include <commonlib.h>
#include <gtthread.h>
#include "diskinfo.h"
#include "mpdisk.h"
#include "hdutil.h"
#include "fileindex.h"
#include "gtsocket.h"
#include "playback.h"
#include "playback_file.h"
#include "playback_pool.h"
#include "gtsf.h"
#include "playback_modcmd.h"


struct hd_playback_struct g_playback[PLAYBACK_NUM];
static struct hdmod_state_struct playback_state= {0};





int playback_listen_init()
{

     pthread_t   thread_id;
    gt_create_thread(&thread_id, playback_listen_thread, NULL);
    return 0;
}



//获取整数表示的hdmod状态
DWORD get_playbackmodstat(void)
{
    DWORD stat;
    memcpy((void*)&stat,(void*)&playback_state,sizeof(DWORD));
    return stat;
}

int playbackInit()
{
    int i;

    memset(g_playback,0,sizeof(g_playback));
    for(i = 0; i < PLAYBACK_NUM; i++)
    {
    
        g_playback[i].framrate =  PLAYBACK_FRAMERATE_25;
        g_playback[i].pre_connect =  PRE_CONNECT_SEC_MAX;
        g_playback[i].speed =  PLAYBACK_NSPEED;
    
    }

    return PLAYBACK_SUCCESS;

}


playback_struct * getplayback(int index)
{

    if(( index < 0) ||( index >= PLAYBACK_NUM))
    {
        return NULL;
    }
    
    return &g_playback[index];

}

extern SOCKS_TABLE  socketable[MAX_PLAYBACK_NUM];

int playbackClose(int index)
{
    int ret;
	int temp;
    
    if((index < 0)||(index >= PLAYBACK_NUM))
    {
         printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
         return PLAYBACK_ERR_PARAM;

    }

    if(g_playback[index].state == PLAYBACK_STAT_IDLE)
    {
         printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
         return PLAYBACK_ERR_NO_OPEN;
    }

    if(g_playback[index].sourcefrom == PLAYBACK_SOURCE_POOL)
    {
       printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "pool");
       ret = playbackclosepool(&g_playback[index]);

    }
    else
    {
       printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "file");
       ret = playbackclosefile(&g_playback[index]);
	   
	   socketable[index].IsUsed=0;
	   socketable[index].fd=0;

    }
    
	temp = g_playback[index].audio_source;
    memset(&g_playback[index], 0, sizeof(g_playback[index]));
    g_playback[index].framrate =  PLAYBACK_FRAMERATE_25;
    g_playback[index].pre_connect =  PRE_CONNECT_SEC_MAX;
    g_playback[index].sourcefrom =  PLAYBACK_SOURCE_POOL;
    g_playback[index].speed =  PLAYBACK_NSPEED;
	g_playback[index].audio_source = temp;
        
    return PLAYBACK_SUCCESS;

}
static int netsend_streamid(int socket, int streamid)
{

    int  result;
    char *pdata;
    int  getbufflen = 4;
    gtsf_stream_fmt  *pStream;
    char  buf[200];


    pStream = (gtsf_stream_fmt *)buf;
        /*数据打包*/
    pStream->mark = GTSF_MARK;
    pStream->encrypt_type = 0;
    pStream->len = getbufflen;
    pStream->type = MEDIA_STREAMID;

    pdata = buf + GTSF_HEAD_SIZE;
    memcpy(pdata,&streamid,4);
    result = net_write_buf(socket ,(char*)pStream,getbufflen+GTSF_HEAD_SIZE);
    if(result != GTSF_HEAD_SIZE+4)
    {
        printf("tcp send errot %d,   result %d\n",getbufflen,result);
        return PLAYBACK_ERR_CONNECT; 
    }
    else
    {
        
        return PLAYBACK_SUCCESS;
    }        
}

/**************************************************************************
  *函数名	:playback_Connect_Server
  *功能	:连接网络服务器
  *参数	: pplaybackopen:回放的接口
  *返回值	:返回正值表示回放ID.其他返回错误码
  *************************************************************************/

int playbackOpen(viewer_subscribe_record_struct *pplaybackopen)
{

    time_t timenow;
    int i;
    int result;
    int fd;
    struct gt_time_struct *start; //起始时间
    struct gt_time_struct *stop;  //结束时间
    struct tm timestart;
    struct tm timestop;
    time_t start_timet,stop_timet;  

    for(i = 0; i < PLAYBACK_NUM; i++)
    {
        if(g_playback[i].state ==  PLAYBACK_STAT_IDLE)
        {
            /*先占用资源，避免冲突*/
            g_playback[i].state = PLAYBACK_STAT_USED;
            g_playback[i].playbackindex = i;
            break;    
        }
    }
    if(i == PLAYBACK_NUM)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "user full");
        return PLAYBACK_ERR_USER_FULL;
    }

    
    /*g_playback[i].starttime、endtime在打印的时候使用,比较的时候使用start,stop*/
    memcpy(&g_playback[i].starttime, &(pplaybackopen->starttime),sizeof(struct gt_time_struct));
    memcpy(&g_playback[i].endtime, &(pplaybackopen->endtime),sizeof(struct gt_time_struct));

    printf("check starttime:%d%d%d%d-%d-%d endtime:%d%d%d %d-%d-%d\n", 
    g_playback[i].starttime.year,g_playback[i].starttime.month,g_playback[i].starttime.day,
    g_playback[i].starttime.hour,g_playback[i].starttime.minute,g_playback[i].starttime.second,
    g_playback[i].endtime.year,g_playback[i].endtime.month,g_playback[i].endtime.day,
    g_playback[i].endtime.hour,g_playback[i].endtime.minute,g_playback[i].endtime.second);
    

    start = &(g_playback[i].starttime);
    stop = &(g_playback[i].endtime);
    gttime2tm(start,&timestart);
    gttime2tm(stop,&timestop);
    start_timet = mktime(&timestart);
    stop_timet = mktime(&timestop);

    g_playback[i].start =  start_timet;
    g_playback[i].stop =  stop_timet;
    g_playback[i].channel =  g_playback[i].channel;
    memcpy(g_playback[i].peeraddr, pplaybackopen->peer_ip,sizeof(pplaybackopen->peer_ip));
    g_playback[i].peerport =  pplaybackopen->peer_port;
    g_playback[i].speed =  pplaybackopen->speed;


    printf("check starttime:%d endtime:%d\n", (int)start_timet,(int)stop_timet);
    

    /*判断开始时间是否在内存中*/
    timenow = time((time_t *)NULL);   
    if(0/*timenow -start_timet <= PRE_CONNECT_SEC_MAX*/)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "pool");
        g_playback[i].sourcefrom = PLAYBACK_SOURCE_POOL;
        g_playback[i].pre_connect =  timenow - start_timet;
        result = playback_connect_media(&g_playback[i]);
      
    }
    else
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "file");
        g_playback[i].sourcefrom = PLAYBACK_SOURCE_FILE;
        result = playback_openfile(&g_playback[i]);
    }
   if(result != PLAYBACK_SUCCESS)
    {
        printf("open file  error\n");
        playbackClose(i);
        return result;
    }

   printf("connect %s port:%d ....\n",g_playback[i].peeraddr,g_playback[i].peerport);
   /*建立连接*/
    //fd = tcp_connect_addr(g_playback[i].peeraddr,g_playback[i].peerport,10);
    fd = tcp_connect_block(g_playback[i].peeraddr,g_playback[i].peerport,10);

    if((int)fd < 0)
    {
    	printf("tcp connect error\n");
       playbackClose(i);
       return PLAYBACK_ERR_CONNECT;
       
    }   

    printf("send  streamid,socket:%d streamid:%d ....\n",fd,pplaybackopen->stream_idx);
    result = netsend_streamid(fd,pplaybackopen->stream_idx);
   if(result != PLAYBACK_SUCCESS)
    {
        printf("send stream id error,%x\n",result);
        playbackClose(i);
        return result;
    }
    g_playback[i].socket = fd;
    g_playback[i].state = PLAYBACK_STAT_OK;

    return i;

}



/**************************************************************************
  *函数名	:playbackfileOpen
  *功能	:连接网络服务器
  *参数	: pplaybackopen:回放的接口
  *返回值	:返回正值表示回放ID.其他返回错误码
  *************************************************************************/

int playbackfileOpen(viewer_subscribe_record_struct *pplaybackopen)
{

    time_t timenow;
    int i;
    int result;
    struct gt_time_struct *start; //起始时间
    struct gt_time_struct *stop;  //结束时间
    struct tm timestart;
    struct tm timestop;
    time_t start_timet,stop_timet;  

    for(i = 0; i < PLAYBACK_NUM; i++)
    {
        if(g_playback[i].state ==  PLAYBACK_STAT_IDLE)
        {
            /*先占用资源，避免冲突*/
            g_playback[i].state = PLAYBACK_STAT_USED;
            g_playback[i].playbackindex = i;
            break;    
        }
    }
    if(i == PLAYBACK_NUM)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "user full");
        return PLAYBACK_ERR_USER_FULL;
    }

    
    /*g_playback[i].starttime、endtime在打印的时候使用,比较的时候使用start,stop*/
    memcpy(&g_playback[i].starttime, &(pplaybackopen->starttime),sizeof(struct gt_time_struct));
    memcpy(&g_playback[i].endtime, &(pplaybackopen->endtime),sizeof(struct gt_time_struct));

    printf("check starttime:%d%d%d%d-%d-%d endtime:%d%d%d %d-%d-%d\n", 
    g_playback[i].starttime.year,g_playback[i].starttime.month,g_playback[i].starttime.day,
    g_playback[i].starttime.hour,g_playback[i].starttime.minute,g_playback[i].starttime.second,
    g_playback[i].endtime.year,g_playback[i].endtime.month,g_playback[i].endtime.day,
    g_playback[i].endtime.hour,g_playback[i].endtime.minute,g_playback[i].endtime.second);
    

    start = &(g_playback[i].starttime);
    stop = &(g_playback[i].endtime);
    gttime2tm(start,&timestart);
    gttime2tm(stop,&timestop);
    start_timet = mktime(&timestart);
    stop_timet = mktime(&timestop);

    g_playback[i].start =  start_timet;
    g_playback[i].stop =  stop_timet;
    g_playback[i].channel =  pplaybackopen->channel;
    memcpy(g_playback[i].peeraddr, pplaybackopen->peer_ip,sizeof(pplaybackopen->peer_ip));
    g_playback[i].peerport =  pplaybackopen->peer_port;
    g_playback[i].speed =  pplaybackopen->speed;


    printf("check starttime:%d endtime:%d,speed:%d\n", (int)start_timet,(int)stop_timet,g_playback[i].speed);
    
    printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "file");
    g_playback[i].sourcefrom = PLAYBACK_SOURCE_FILE;
    result = playback_openfile(&g_playback[i]);

   if(result != PLAYBACK_SUCCESS)
    {
        printf("open file  error\n");
        playbackClose(i);
        return result;
    }


//    g_playback[i].socket = fd;
//    g_playback[i].state = PLAYBACK_STAT_OK;

    return i;

}

/**************************************************************************
  *函数名	:playbackConnectOK
  *功能	:置准备好的状态
  *参数	: playbackId:回放ID,fd:回放的socket
  *返回值	:返回0成功.其他返回错误码
  *************************************************************************/

int playbackConnectOK(int playbackId, int fd)
{

    if((playbackId < 0) || (playbackId >= PLAYBACK_NUM))
    {
        printf("[Func]:%s [Line]:%d [id]:%d\n", __FUNCTION__, __LINE__,playbackId );
        return PLAYBACK_ERR_PARAM;    
    }

    if(g_playback[playbackId].state !=PLAYBACK_STAT_USED)
    {
       printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "NO OPEN");
        return PLAYBACK_ERR_NO_OPEN;    
    }


    g_playback[playbackId].socket = fd;
    g_playback[playbackId].state = PLAYBACK_STAT_OK;


    return 0;

}



int playbackReadFrame(int playbackId, void *buf, int buf_len, int *seq, int *flag)
{

    int ret;

    if((playbackId < 0) || (playbackId >= PLAYBACK_NUM))
    {
        printf("[Func]:%s [Line]:%d [id]:%d\n", __FUNCTION__, __LINE__,playbackId );
        return PLAYBACK_ERR_PARAM;    
    }

    if(g_playback[playbackId].state !=PLAYBACK_STAT_USED)
    {
       printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "NO OPEN");
        return PLAYBACK_ERR_NO_OPEN;    
    }
    
    
    if(g_playback[playbackId].sourcefrom == PLAYBACK_SOURCE_POOL)
    {
        ret = playbackreadpoolframe(&g_playback[playbackId], buf,  buf_len, seq, flag);
    }
    else
    {
    
        //ret = playbackreadfileframe(&g_playback[playbackId], buf,  buf_len, seq, flag);
    }
    return ret;
    
}

void static playback_streaminfo(void)
{

    int i;
        
    for(i = 0; i < PLAYBACK_NUM; i++)
    {
        if(g_playback[i].state !=  PLAYBACK_STAT_IDLE)
        {
            printf("playback index:%d, state:%d\n", i, g_playback[i].state);
            printf("starttime:%d%d%d%d-%d-%d endtime:%d%d%d %d-%d-%d\n", 
            g_playback[i].starttime.year,g_playback[i].starttime.month,g_playback[i].starttime.day,
            g_playback[i].starttime.hour,g_playback[i].starttime.minute,g_playback[i].starttime.second,
            g_playback[i].endtime.year,g_playback[i].endtime.month,g_playback[i].endtime.day,
            g_playback[i].endtime.hour,g_playback[i].endtime.minute,g_playback[i].endtime.second);
           printf("connect ip:%s:%d,send packet:%d\n", g_playback[i].peeraddr,g_playback[i].peerport, 
            g_playback[i].packetsum);
        }
        
    }

}

//在日志上记录退出过程
static void exit_log(int signo)
{
    switch(signo)
    {
        case SIGTERM:
            gtloginfo("playback 被kill,程序退出!!\n");
            exit(0);
        break;
        case SIGKILL:
            gtloginfo("playback SIGKILL,程序退出!!\n");
            close_all_res();
            exit(0);
        break;
        case SIGINT:
            gtloginfo("playback 被用户终止(ctrl-c)\n");
            close_all_res();
            exit(0);
        break;
        case SIGUSR1://输出系统信息到指定文件
            playback_streaminfo();
        break;
        case SIGSEGV:
            gtloginfo("#########playback 发生段错误#########\n");
            close_all_res();
            printf("playback segmentation fault\n");
            exit(0);
        break;
    }
    return;
}


int main(int argc,char *argv[])
{

    int lock_file;
    char pbuf[100];
    int i;

    gtopenlog("playback");
    setsid();

    //process_argument(argc,argv);    
    lock_file=create_and_lockfile(PLAYBACK_LOCK_FILE);
    if(lock_file<=0)
    {
        printf("playback module are running!!\n");
        gtlogerr("playback模块已运行，故启动无效退出\n");       
        exit(0);
    }   
    sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
    write(lock_file,pbuf,strlen(pbuf)+1);//将进程的id号存入锁文件中
    //以上完成判断模块是否已经执行的功能


    signal(SIGKILL,exit_log);
    signal(SIGTERM,exit_log);
    signal(SIGINT,exit_log);
    signal(SIGSEGV,exit_log);
    signal(SIGUSR1,exit_log);
    printf("启动playback(ver:%s).......\n",VERSION);
    gtloginfo("启动playback(ver:%s).......\n",VERSION);

    init_com_channel();
    InitAllDabase();

    creat_playback_modsocket_thread();
    playbackInit();

 

#if 0    
    playback_open_struct playbackopen = 
    {98,1008,1,PLAYBACK_NSPEED,{2013,12,17,9,20,30},{2013,12,17,9,20,50}};

    i = playbackOpen(&playbackopen);
#endif    
    //playback_openfile1();
    for(i = 0; i < PLAYBACK_NUM; i++)
    {
        readfile_thread(i);
    }

    /*被动模式*/
    playback_listen_init();
        

    while(1)
    {   
        sleep(10);
    }

    return 0;
    
}

