#include "playback.h"
//#include "hdmod.h"
#include <mod_com.h>
#include <gt_com_api.h>
#include <gt_errlist.h>
//#include <ime6410api.h>
#include "gtsocket.h"
#include <devinfo.h>
#include "playback_modcmd.h"
#include <unistd.h>
#include "commonlib.h"
#include "mod_socket.h"
#include "playback_file.h"

static int  com_fd= -1; //发送和接收命令的udp socket
static pthread_t modsocket_thread_id=-1;
extern struct hd_playback_struct g_playback[PLAYBACK_NUM];
//初始化与主进程通讯的命令通道
int init_com_channel(void)
{
    com_fd  =   mod_socket_init(0,0);   
    return 0;
}

//发送状态给主进程
int send_state2main(void)
{
    DWORD *state;   
    DWORD socketbuf[20];
    mod_socket_cmd_type *cmd;
    pid_t *pid;
    
    cmd=(mod_socket_cmd_type *)socketbuf;
    cmd->cmd    =   HDMOD_STATE_RETURN;
    cmd->len    =   4+sizeof(pid_t);
    pid=(pid_t*)cmd->para;
    *pid=getpid();
    state=(DWORD*)&cmd->para[sizeof(pid_t)];
    *state=get_playbackmodstat();
    return mod_socket_send(com_fd,MAIN_PROCESS_ID,PLAYBACK_PROCESS_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);
}


//查询索引，被usr_query_index_cmd调用
//传来的index是索引结果文件的名称，query是起始+终止时间的结构
//返回0表示正常
//-1 内部错误
//-2 远程传来的参数错误
int process_timesection(int channel, char *index, usr_query_timesection_struct *query)
{

    struct tm tm;
    struct gt_time_struct *rstart,*rstop;
    time_t start,stop;

    
    if((index==NULL)||(query==NULL))
        return -1;
        
    sprintf(index,"%s","not found");

    rstart=(struct gt_time_struct *)&query->starttime;
    rstop=(struct gt_time_struct *)&query->endtime;

    gttime2tm(rstart,&tm);
    start = mktime(&tm);
    if(start<0)
    {
        start=0;
    }
    gttime2tm(rstop,&tm);   
    stop = mktime(&tm);
    if(stop<0)
    {
        stop=0;
    }
        
    if(stop<start)
        return -2;
    gtloginfo("处理查询avi文件索引的命令,通道号%d,时间%04d-%02d-%02d %02d:%02d:%02d到%04d-%02d-%02d %02d:%02d:%02d\n",channel,rstart->year,rstart->month,rstart->day,rstart->hour,rstart->minute,rstart->second,rstop->year,rstop->month,rstop->day,rstop->hour,rstop->minute,rstop->second);

    
    return query_record_timesection(index,channel, start, stop);
}



int usr_query_index_with_time(struct gt_usr_cmd_struct *cmd, gateinfo *gate)
{
    usr_query_timesection_struct qtimesection;
    dev_timesection_ret_struct  *retqtimesection;
    mod_socket_cmd_type *modsocket;
    DWORD   send_buf[400];
    struct gt_usr_cmd_struct *gtcmd;
    int ret;
    
    
    modsocket=(mod_socket_cmd_type *)send_buf;
    modsocket->cmd=MOD_BYPASSTO_GATE_CMD;
    memcpy(&modsocket->gate,gate,sizeof(gateinfo));
    gtcmd = (struct gt_usr_cmd_struct *)modsocket->para;
    gtcmd->cmd=DEV_TIMESECTION_RETURN;
    gtcmd->en_ack=0;
    gtcmd->reserve0=0;
    gtcmd->reserve1=0;
    retqtimesection=(dev_timesection_ret_struct *)((char *)gtcmd->para);
    memcpy(&qtimesection,cmd->para,sizeof(qtimesection));
    
    

    printf("查询通道号%d,时间%04d-%02d-%02d %02d:%02d:%02d到%04d-%02d-%02d %02d:%02d:%02d\n",qtimesection.channel, 
    qtimesection.starttime.year,qtimesection.starttime.month,qtimesection.starttime.day,
    qtimesection.starttime.hour,qtimesection.starttime.minute,qtimesection.starttime.second,
    qtimesection.endtime.year,qtimesection.endtime.month,qtimesection.endtime.day,
    qtimesection.endtime.hour,qtimesection.endtime.minute,qtimesection.endtime.second);    

    ret=-process_timesection((char)qtimesection.channel,(char*)retqtimesection->index_file,&qtimesection); 
    
    gtloginfo("查询索引完毕结果0x%x,名称%s\n",ret,retqtimesection->index_file);
    if(ret==0)
        retqtimesection->result=RESULT_SUCCESS;
    else if(ret==2)
    {
        retqtimesection->result=ERR_DVC_INVALID_REQ;
    }
    else if(ret==ERR_DVC_NO_RECORD_INDEX)
    {
        retqtimesection->result=ERR_DVC_NO_RECORD_INDEX;
    }
    else
    {
        retqtimesection->result=ERR_DVC_INTERNAL;
    }


    retqtimesection->format=0;
    printf("send result %d,名称%s\n",retqtimesection->result,retqtimesection->index_file);
    gtcmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(gtcmd->para)+sizeof(dev_timesection_ret_struct);
    modsocket->len = gtcmd->len +sizeof(gtcmd->len);
    ret = mod_socket_send(com_fd,MAIN_PROCESS_ID,PLAYBACK_PROCESS_ID,modsocket,sizeof(mod_socket_cmd_type)-sizeof(modsocket->para)+modsocket->len);
    gtloginfo("发送用户查询%d通道录像索引结果0x%04x,len %d\n",qtimesection.channel,retqtimesection->result,modsocket->len);
    return 0;
}



    

static int process_hdplayback_start(struct gt_usr_cmd_struct *cmd,gateinfo *gate)
{
    viewer_subscribe_record_struct playbackopen;
    viewer_subsrcibe_answer_record_struct *openret;
    int opench = 0;
    mod_socket_cmd_type *modsocket;
    DWORD   send_buf[200];
    struct gt_usr_cmd_struct *gtcmd;
    int ret;


   printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "enter");
    
    modsocket=(mod_socket_cmd_type *)send_buf;
    modsocket->cmd=MOD_BYPASSTO_GATE_CMD;
    memcpy(&modsocket->gate,gate,sizeof(gateinfo));
    gtcmd = (struct gt_usr_cmd_struct *)modsocket->para;
    gtcmd->cmd=VIEWER_SUBSCRIBE_ANSWER_RECORD;
    gtcmd->en_ack=0;
    gtcmd->reserve0=0;
    gtcmd->reserve1=0;
    openret=(viewer_subsrcibe_answer_record_struct *)((char *)gtcmd->para);
    memcpy(&playbackopen,cmd->para,sizeof(playbackopen));

    printf("playbackopen para peer_port:%d,channel:%d\n",playbackopen.peer_port, playbackopen.channel);
    //gtloginfo("查询索引完毕结果0x%x,名称%s\n",ret,openret->);

    printf("check starttime:%d%d%d %d-%d-%d endtime:%d%d%d %d-%d-%d\n", 
    playbackopen.starttime.year,playbackopen.starttime.month,playbackopen.starttime.day,
    playbackopen.starttime.hour,playbackopen.starttime.minute,playbackopen.starttime.second,
    playbackopen.endtime.year,playbackopen.endtime.month,playbackopen.endtime.day,
    playbackopen.endtime.hour,playbackopen.endtime.minute,playbackopen.endtime.second);    
    opench = playbackOpen(&playbackopen);
    if(opench < 0)
    {
        openret->status = opench;
        openret->query_usr_id = opench;
    }
    else
     {
        openret->status = 0;
    }       
    openret->query_usr_id = opench;
    printf("playbackopen return ,channel :%d,status:%d\n",openret->query_usr_id, openret->status);
    
    gtcmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(gtcmd->para)+sizeof(viewer_subsrcibe_answer_record_struct);
    modsocket->len = gtcmd->len +sizeof(gtcmd->len);
    ret = mod_socket_send(com_fd,MAIN_PROCESS_ID,PLAYBACK_PROCESS_ID,modsocket,sizeof(mod_socket_cmd_type)-sizeof(modsocket->para)+modsocket->len);
    //gtloginfo("发送用户查询%d通道录像索引结果0x%04x,len %d\n",qtimesection.channel,retqtimesection->result,modsocket->len);

    return 0;
}


static int process_hdplayback_stop(struct gt_usr_cmd_struct *cmd, gateinfo *gate)
{
    viewer_unsubscribe_record_struct *stopctrl;
    int  index;

    stopctrl = (viewer_unsubscribe_record_struct *)cmd->para;

    index = stopctrl->query_usr_id;
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

    g_playback[index].oper = PLAYBACK_CTRL_CLOSE;
   
    return 0;
}


static int process_hdplayback_ctrl(struct gt_usr_cmd_struct *cmd, gateinfo *gate)
{

    viewer_subscribe_record_ctl_struct *pctrl;
    viewer_subscribe_record_ctl_ret_struct *pretctrl;
    int ret;
    mod_socket_cmd_type *modsocket;
    DWORD   send_buf[200];
    struct gt_usr_cmd_struct *gtcmd;
    int index;


    modsocket=(mod_socket_cmd_type *)send_buf;
    modsocket->cmd=MOD_BYPASSTO_GATE_CMD;
    memcpy(&modsocket->gate,gate,sizeof(gateinfo));
    gtcmd = (struct gt_usr_cmd_struct *)modsocket->para;
    gtcmd->cmd = VIEWER_SUBSCRIBE_RECORD_CONTROL_RESULT;
    gtcmd->en_ack = 0;
    gtcmd->reserve0 = 0;
    gtcmd->reserve1 = 0;
   // pretctrl = (viewer_subsrcibe_answer_record_struct *)&gtcmd->para;
   pretctrl = (viewer_subscribe_record_ctl_ret_struct *)&gtcmd->para;


    pctrl=(viewer_subscribe_record_ctl_struct *)cmd->para;


    printf("playback_ctrl:%d [cmd]:%d\n", pctrl->query_usr_id, pctrl->ctl_cmd);
    index = pctrl->query_usr_id;
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

    if(pctrl->ctl_cmd == PAUSE)
    {
        g_playback[index].oper = PLAYBACK_CTRL_PAUSE;
    }
    else if(pctrl->ctl_cmd == RESUME)
    {
        g_playback[index].oper = PLAYBACK_CTRL_IDLE;
    }
    else if(pctrl->ctl_cmd == SPEED)
    {
        printf("playback %d set speed:%d\n", pctrl->query_usr_id,pctrl->speed);
        g_playback[index].speed = pctrl->speed;
    }
    else if(pctrl->ctl_cmd == SEEK)
    {
        struct gt_time_struct *start; //起始时间
        struct gt_time_struct *stop;  //结束时间
        struct tm timestart;
        struct tm timestop;
        time_t start_timet,stop_timet;  
      
        /*g_playback[i].starttime、endtime在打印的时候使用,比较的时候使用start,stop*/
        memcpy(&g_playback[index].starttime, &(pctrl->starttime),sizeof(struct gt_time_struct));
        memcpy(&g_playback[index].endtime, &(pctrl->endtime),sizeof(struct gt_time_struct));
        start = (struct gt_time_struct *)&pctrl->starttime;
        stop = (struct gt_time_struct *)&pctrl->endtime;
        gttime2tm(start,&timestart);
        gttime2tm(stop,&timestop);
        start_timet = mktime(&timestart);
        stop_timet = mktime(&timestop);

        g_playback[index].start =  start_timet;
        g_playback[index].stop =  stop_timet;
        g_playback[index].oper = PLAYBACK_CTRL_SEEK;
        printf("playback command:%d seek\n", pctrl->query_usr_id);
        
    }
     
    pretctrl->status = 0;
    gtcmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(gtcmd->para)+sizeof(viewer_subscribe_record_ctl_ret_struct);
    modsocket->len = gtcmd->len +sizeof(gtcmd->len);
    ret = mod_socket_send(com_fd,MAIN_PROCESS_ID,PLAYBACK_PROCESS_ID,modsocket,sizeof(mod_socket_cmd_type)-sizeof(modsocket->para)+modsocket->len);

    return 0;
}



//处理由主进程转发过来的网关命令
static int process_gate_cmd(struct gt_usr_cmd_struct *cmd,gateinfo *gate)
{

    switch(cmd->cmd)
    {
        case VIEWER_SUBSCRIBE_RECORD:
            gtloginfo("recv a VIEWER_SUBSCRIBE_RECORD cmd!\n");
            process_hdplayback_start(cmd, gate);
        break;
        case USR_QUERY_TIMESECTION: //录像控制，改为转发 -wsy
            gtloginfo("recv a USR_QUERY_TIMESECTION cmd!\n");
            usr_query_index_with_time(cmd,gate);
        break;
        case VIEWER_SUBSCRIBE_RECORD_CONTROL:
            gtloginfo("recv VIEWER_SUBSCRIBE_RECORD_CONTROL cmd !\n");        
            process_hdplayback_ctrl(cmd,gate);
        break;

        case VIEWER_UNSUBSCRIBE_RECORD:
            gtloginfo("recv VIEWER_UNSUBSCRIBE_RECORD cmd !\n");   
            process_hdplayback_stop(cmd,gate);
        break;
        default:
            printf("hdmodule recv a unknow bypass cmd 0x%04x\n",cmd->cmd);
            gtloginfo("hdmodule recv a unknow bypass cmd 0x%04x\n",cmd->cmd);           
            //send_ack_to_main(com_fd,PLAYBACK_PROCESS_ID,cmd->cmd,ERR_EVC_NOT_SUPPORT, gate);
        break;
    }
    return 0;
}



/*************************************************************************
 * 以上是主进程转发过来的网关命令
*************************************************************************/


static int process_modsocket_cmd(int sourceid, mod_socket_cmd_type *cmd)
{
    int rc=0;
    switch(cmd->cmd)
    {
        case GATE_BYPASSTO_MOD_CMD://由主进程转发过来的网关命令
            process_gate_cmd((struct gt_usr_cmd_struct *)&cmd->para,&cmd->gate);
        break;
        case MAIN_QUERY_STATE:
            gtloginfo("recv MAIN_QUERY_STATE cmd !\n");
            send_state2main();
        break;

        default:
            printf("hdmodule recv a unknow cmd %x:",cmd->cmd);
            gtloginfo("hdmodule recv a unknow cmd %x:",cmd->cmd);
        break;
    }
    return rc;
}

int creat_playback_modsocket_thread(void)
{
    return creat_modsocket_thread(&modsocket_thread_id, 
    com_fd,PLAYBACK_PROCESS_ID,"HDPLAYBACK",process_modsocket_cmd);
}


static void playback_stopconnect(int fd)
{

    int i;


    printf("stop connect,fd :%d\n",fd);
    for(i = 0; i < PLAYBACK_NUM; i++)
    {
        
        if((g_playback[i].state == PLAYBACK_STAT_OK)&&(g_playback[i].socket ==fd))
        {
            //g_playback[i].socket = 0;/*端口已经关闭*/
            printf("stop connect,index:%d ,fd :%d\n",i, fd);
            g_playback[i].oper = PLAYBACK_CTRL_CLOSE;
#if 0	    
            /*先发送命令，等待执行完毕*/
            while(g_playback[i].state !=  PLAYBACK_STAT_IDLE)
            {
                 usleep(10);
            }
#endif			
        }
    }

    return 0;
}

int playback_listen_command(char *pbuff, int len, int fd)
{
    viewer_subscribe_record_struct playbackopen;
    viewer_subsrcibe_answer_record_struct *openret;
    int opench = 0;
    mod_socket_cmd_type *modsocket;
    DWORD   send_buf[200];
    struct gt_pkt_struct            *sendpkt=(struct gt_pkt_struct *)send_buf;           ///<要发送的数据的结构指针

    struct gt_usr_cmd_struct *gtcmd;
    struct gt_pkt_struct *pPkt = (struct gt_usr_cmd_struct *)pbuff;


    if(len < (sizeof(struct gt_pkt_struct ) + sizeof(struct gt_usr_cmd_struct) + 
                    sizeof(viewer_subscribe_record_struct) -4))
    {

        printf("len:%d,need:%d\n",len, sizeof(struct gt_pkt_struct ) + sizeof(struct gt_usr_cmd_struct) + 
                    sizeof(viewer_subscribe_record_struct) -4);
        return -1;
    }

    
    struct gt_usr_cmd_struct *cmd = (struct gt_usr_cmd_struct *)pPkt->msg;
    memcpy(&playbackopen,cmd->para,sizeof(playbackopen));

    gtcmd = (struct gt_usr_cmd_struct *)sendpkt->msg;
    gtcmd->cmd=VIEWER_SUBSCRIBE_ANSWER_RECORD;
    gtcmd->en_ack=0;
    gtcmd->reserve0=0;
    gtcmd->reserve1=0;
    openret=(viewer_subsrcibe_answer_record_struct *)((char *)gtcmd->para);

    printf("playbackopen para peer_port:%d,channel:%d\n",playbackopen.peer_port, playbackopen.channel);
    //gtloginfo("查询索引完毕结果0x%x,名称%s\n",ret,openret->);

    printf("check starttime:%d%d%d %d-%d-%d endtime:%d%d%d %d-%d-%d\n", 
    playbackopen.starttime.year,playbackopen.starttime.month,playbackopen.starttime.day,
    playbackopen.starttime.hour,playbackopen.starttime.minute,playbackopen.starttime.second,
    playbackopen.endtime.year,playbackopen.endtime.month,playbackopen.endtime.day,
    playbackopen.endtime.hour,playbackopen.endtime.minute,playbackopen.endtime.second);    
    opench = playbackfileOpen(&playbackopen);
    if(opench < 0)
    {
        openret->status = opench;
        openret->query_usr_id = opench;
    }
    else
     {
        openret->status = 0;
    }       
    openret->query_usr_id = opench;
    printf("playbackopen return ,channel :%d,status:%d\n",openret->query_usr_id, openret->status);
    
    gtcmd->len= sizeof(viewer_subsrcibe_answer_record_struct) + 6;
    //gtcmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(gtcmd->para)+sizeof(viewer_subsrcibe_answer_record_struct);
    //ret = net_write_buf(fd,send_buf,gtcmd->len);


    printf("playbackopen return ,channel :%d,status:%d\n",openret->query_usr_id, openret->status);
    
    gt_cmd_send_noencrypt(fd, send_buf, gtcmd->len+2, NULL, 0);

    printf("gt_cmd_send_noencrypt return ,socket :%d channel :%d,status:%d\n",fd,openret->query_usr_id, openret->status);
    
    //gtloginfo("发送用户查询%d通道录像索引结果0x%04x,len %d\n",qtimesection.channel,retqtimesection->result,modsocket->len);

    if(openret->status == 0)
        playbackConnectOK(opench, fd);

    return openret->status;
}



static unsigned short  playback_get_port()
{

    dictionary    *ini=NULL;
    FILE            *fp=NULL;
    int               port,i;
    char              secbuf[20]={0};

    ini=iniparser_load_lockfile(IPMAIN_PARA_FILE,1,&fp);
    if(ini==NULL)
    {
          printf("playback_get_port() cannot parse ini file file [%s]", IPMAIN_PARA_FILE);
          gtlogerr("playback_get_port() cannot parse ini file file [%s]", IPMAIN_PARA_FILE);
          return -1 ;
    }


    //当前录像盘
    port = iniparser_getint(ini,"port:playback_port",8900);
  
    if(fp!=NULL)
    {
        unlock_file(fileno(fp));
        fsync(fileno(fp));
        fclose(fp);
    }   
    iniparser_freedict(ini);
    
    return (unsigned short)port;
    
}

SOCKS_TABLE  socketable[MAX_PLAYBACK_NUM];

void *playback_listen_thread(void *para)
{
    int     listen_port;
    int     fd,accept_fd,maxfd;
    int     ret,i;
    fd_set readfds;
    struct timeval  timeval;
    char     buff[502];
    struct sockaddr addr;
    int addrlen=sizeof(addr);
    time_t timenow;


    listen_port = playback_get_port();

    printf("get  port:%d !\n",listen_port);
    fd = create_tcp_listen_port(INADDR_ANY,listen_port); 
    if(fd <= 0)
    {
        printf("can't create listen socket port:%d error, exit !\n",listen_port);
        exit(1);
    }
    //net_activate_keepalive(fd);
    listen(fd,5); 

    for(i = 0; i < MAX_PLAYBACK_NUM; i++)
    {
        socketable[i].IsUsed = 0;
        socketable[i].fd = 0;
    }



    while(1)
    {

        timenow = time((time_t *)NULL);

        for(i = 0; i < MAX_PLAYBACK_NUM; i++)
        {
            if (socketable[i].IsUsed)
            {
                /*超时12小时关闭连接*/
                if (timenow - socketable[i].start >= 12*3600)
                {
                    printf("socket %d timeout, close it !\n",socketable[i].fd);
                    close(socketable[i].fd);
                    socketable[i].IsUsed = 0;
                    socketable[i].fd = 0;
                }
            }
        }

        timeval.tv_sec = 1;
        timeval.tv_usec = 0;
        FD_ZERO(&readfds);

        FD_SET(fd,&readfds);
        maxfd = fd;
        for(i = 0; i < MAX_PLAYBACK_NUM; i++)
        {
            if (socketable[i].IsUsed)
            {
                if (maxfd <= socketable[i].fd)
                {
                    maxfd = socketable[i].fd;
                }
                FD_SET(socketable[i].fd,&readfds);
            }
        }

        ret=select(maxfd +1, &readfds, NULL, NULL, &timeval);
        if(ret == 0)
        {
            continue;
        }

        if(FD_ISSET(fd,&readfds))
        {
            accept_fd = accept(fd,&addr,&addrlen);
            printf("accept connect:socket:%d !\n",accept_fd);
            for(i = 0; i < MAX_PLAYBACK_NUM; i++)
            {
                if (socketable[i].IsUsed == 0)
                {
                    socketable[i].IsUsed = 1;
                    socketable[i].fd = accept_fd;
                    socketable[i].start = time((time_t *)NULL);
                    break;
                }
            }
            if(i == 4)
               printf("table full ,socket discard:%d !\n",accept_fd); 
            
        }

        for(i = 0; i < MAX_PLAYBACK_NUM; i++)
        {
            if (socketable[i].IsUsed)
            {
                if(FD_ISSET(socketable[i].fd, &readfds))
                {
                    ret = recv(socketable[i].fd, buff, sizeof(buff),0);
                    printf("net_read_buf,socket:%d,len:%d\n",socketable[i].fd, ret);
                    if (ret > 0)
                    {
                        ret= playback_listen_command(buff, ret, socketable[i].fd);
                        if(ret <0 )
                        {
                            close(socketable[i].fd);
                            socketable[i].IsUsed = 0;
                            socketable[i].fd = 0;
                        }
                    }
                    else
                    {
                    
                        //close(socketable[i].fd);
                        playback_stopconnect(socketable[i].fd);
                        socketable[i].IsUsed = 0;
                        socketable[i].fd = 0;
                    }

                        
                        
                }  
            }
			else
			{
				FD_CLR(socketable[i].fd,&readfds);
			}
        }

        

    
    }

    

}


