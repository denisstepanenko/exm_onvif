#include "mp_hdmodule.h"
#include "hdmod.h"
#include <mod_com.h>
#include <gt_com_api.h>
#include <gt_errlist.h>
//#include <ime6410api.h>
#include <devinfo.h>
#include "process_modcmd.h"
#include <unistd.h>
#include "commonlib.h"
#include "mod_socket.h"

static int  com_fd= -1; //发送和接收命令的udp socket
static pthread_t modsocket_thread_id=-1;

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
    *state=get_hdmodstatint();
    return mod_socket_send(com_fd,MAIN_PROCESS_ID,HQSAVE_PROCESS_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);
}



/*
  *让hdmodule重新读取配置
*/
int refresh_hdmodule_para(void)
{
    /*
    struct hd_enc_struct    *hdenc;
    int ch; 
    char sec[20];
    
    for(ch=0;ch<get_total_hqenc_num();ch++)
        {
            hdenc=get_hdch(ch);
            sprintf(sec,"hqenc%d",ch);
            //read_enc_para_file(HDMOD_PARA_FILE,sec,&hdenc->encoder);//读取配置文件
            read_hqsave_para_file(HDMOD_PARA_FILE,"hqpara",ch);
        }
    */
    read_hqsave_para_file(HDMOD_PARA_FILE,"hqpara",0);
    if(get_total_hqenc_num()==2)//有两路
        read_hqsave_para_file(HDMOD_PARA_FILE, "hqpara1",1);
    return 0;
}



//抓图响应送给网关
int get_hq_pic_answer(gateinfo *gate, WORD result, BYTE *timeprint,char* indexname)
{
    DWORD send_buf[50];//响应命令包不会超过200字节
    //BYTE  dev_buf[20];
    struct gt_usr_cmd_struct *cmd;
    mod_socket_cmd_type *modsocket;
    struct hqpic_answer_struct *picanswer;

    char timebuf[20];
    int ret=0;
    BYTE *index;
    if((timeprint==NULL)||(indexname==NULL))
        return -1;
    sprintf(timebuf,"%02x%02x%02x%02x-%02x%02x%02x%02x",
        timeprint[0],timeprint[1],timeprint[2],timeprint[3],
        timeprint[4],timeprint[5],timeprint[6],timeprint[7]);

    modsocket=(mod_socket_cmd_type *)send_buf;
    modsocket->cmd=MOD_BYPASSTO_GATE_CMD;
    printf("[%s:%d]gatefd=%d\n",__FILE__,__LINE__,(int)gate->gatefd);
    gate->gatefd = -1; //抓图只能送回-1
    memcpy(&modsocket->gate,gate,sizeof(gateinfo));
    cmd=(struct gt_usr_cmd_struct *)modsocket->para;
    cmd->cmd=HQ_PIC_ANSWER;
    cmd->en_ack=0;
    cmd->reserve0=0;
    cmd->reserve1=0;
    picanswer=(struct hqpic_answer_struct*)cmd->para;
    picanswer->state=result;

    picanswer->reserve2=0;
    memcpy(picanswer->timeprint,timeprint,8);
    
    index=(BYTE*)&picanswer->index_file;
    sprintf(index,indexname);
    cmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para)+sizeof(struct hqpic_answer_struct)-sizeof(picanswer->index_file)+strlen(indexname)+1;
    modsocket->len = cmd->len+sizeof(cmd->len);
    //ret=send_gate_pkt(-1,modcom,cmd->len+2);  
    ret=mod_socket_send(com_fd,MAIN_PROCESS_ID,HQSAVE_PROCESS_ID,send_buf,sizeof(mod_socket_cmd_type)-sizeof(modsocket->para)+modsocket->len);
    gtloginfo("发送抓图响应,结果%x,路径%s,send结果%d\n",result,indexname,ret);
    return 0;
} 

int usr_query_index_with_channel(mod_socket_cmd_type *cmd)
{
    struct query_index_with_channel qindexch;
    struct index_return_struct  *retindex;
    mod_socket_cmd_type *modsocket;
    DWORD   send_buf[200];
    struct gt_usr_cmd_struct *gtcmd;
    int ret;
    
    if((cmd == NULL)||(cmd->cmd!=USR_QUERY_INDEX))
        return -EINVAL;
    
    
    
    modsocket=(mod_socket_cmd_type *)send_buf;
    modsocket->cmd=MOD_BYPASSTO_GATE_CMD;
    memcpy(&modsocket->gate,&cmd->gate,sizeof(gateinfo));
    gtcmd = (struct gt_usr_cmd_struct *)modsocket->para;
    gtcmd->cmd=DEV_RECORD_RETURN;
    gtcmd->en_ack=0;
    gtcmd->reserve0=0;
    gtcmd->reserve1=0;
    retindex=(struct index_return_struct *)((char *)gtcmd->para+8);
    memcpy(&qindexch,cmd->para,sizeof(qindexch));
    
    
#ifdef SHOW_WORK_INFO
    printf("正在查询索引...\n");
#endif
    
    ret=-process_net_index(qindexch.channel,retindex->indexname,&qindexch.queryindex); 
    
#ifdef SHOW_WORK_INFO
    printf("索引查询完毕ret=%d name=%s\n",ret,retindex->indexname);
#endif
    gtloginfo("查询索引完毕结果0x%x,名称%s\n",ret,retindex->indexname);
    if(ret==0)
        retindex->result=RESULT_SUCCESS;
    else if(ret==2)
    {
        retindex->result=ERR_DVC_INVALID_REQ;
    }
    else if(ret==ERR_DVC_NO_RECORD_INDEX)
    {
        retindex->result=ERR_DVC_NO_RECORD_INDEX;
    }
    else
    {
        retindex->result=ERR_DVC_INTERNAL;
    }


    retindex->format=0;
    retindex->reserve=0;
    //memcpy(retindex->indexname,RECORD_INDEX_FILE,strlen(RECORD_INDEX_FILE)+1);
    gtcmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(gtcmd->para)-sizeof(gtcmd->len)+8+sizeof(struct index_return_struct)-sizeof(retindex->indexname)+strlen(retindex->indexname)+1;
    modsocket->len = gtcmd->len +sizeof(gtcmd->len);
    ret = mod_socket_send(com_fd,MAIN_PROCESS_ID,HQSAVE_PROCESS_ID,modsocket,sizeof(mod_socket_cmd_type)-sizeof(modsocket->para)+modsocket->len);
    gtloginfo("发送用户查询%d通道录像索引结果0x%04x,len %d\n",qindexch.channel,retindex->result,modsocket->len);
    return 0;
}


//查询索引，被usr_query_index_cmd调用
//传来的index是索引结果文件的名称，query是起始+终止时间的结构
//返回0表示正常
//-1 内部错误
//-2 远程传来的参数错误
int process_net_index(int channel,char *index,struct query_index_struct *query)
{
    struct tm tm;
    struct gt_time_struct *rstart,*rstop;
    time_t start,stop,timenow;

    
    if((index==NULL)||(query==NULL))
        return -1;
        
    sprintf(index,"%s","not found");

    rstart=(struct gt_time_struct *)&query->start;
    rstop=(struct gt_time_struct *)&query->stop;
        
    gttime2tm(rstart,&tm);
    start = mktime(&tm);
    if(start<0)
    {
        start=0;
        //printf("process_net_index bad start time:%d\n",inttime);
        //gtlogwarn("查询录像索引所提供的起始时间不正确 %d\n",inttime);
        //return -2;
    }
    gttime2tm(rstop,&tm);   
    stop = mktime(&tm);
    if(stop<0)
    {
        stop=0;
        //printf("process_net_index bad stop time:%d\n",inttime);
        //gtlogwarn("查询录像索引所提供的起始终止不正确 %d\n",inttime);
        //return -2;
    }
        
    if(stop<start)
        return -2;
    gtloginfo("处理查询avi文件索引的命令,通道号%d,时间%04d-%02d-%02d %02d:%02d:%02d到%04d-%02d-%02d %02d:%02d:%02d\n",channel,rstart->year,rstart->month,rstart->day,rstart->hour,rstart->minute,rstart->second,rstop->year,rstop->month,rstop->day,rstop->hour,rstop->minute,rstop->second);
    timenow=time((time_t *)NULL);
    if(timenow-180 < stop) //查询文件包括
    {
            get_hdch(0)->cutflag =1;    
            sleep(1);//供关闭文件，更新数据库
    }
    
    return query_record_index(index,channel, start, stop,-1);
    
}




static int process_hdrecord_ctrl(mod_socket_cmd_type *cmd)
{

    struct hdrecord_ctrl_struct *ctrl;
    int ch=0;

    if((cmd==NULL)||(cmd->cmd!=HDRECORD_CTRL))
        return -1;
    ctrl=(struct hdrecord_ctrl_struct *)cmd->para;

    if(get_hdch(ch)->enable == 0)//not enabled
    {
        gtloginfo("%d路视频禁用,高清晰录像控制无效\n",ch);
        return -1;
    }
    switch(ctrl->mode)
    {
        case 0:
            gtloginfo("停止高清晰录像命令\n");
            stop_recordfilethread(get_hdch(ch));
        break;
        case 1:
            gtloginfo("开始高清晰录像命令\n");
            get_hdch(ch)->threadexit = 0;
            start_audio_thread(get_hdch(ch));
            usleep(500000);            
            start_recordfilethread(get_hdch(ch));
        break;
        case 2:
            gtloginfo("重启高清晰录像命令\n");
            restart_recordfilethread(get_hdch(ch));
        break;
        default:
        break;
    }
    return 0;
}

static int process_trig_record_event(mod_socket_cmd_type *cmd)
{
    struct trig_record_event_struct *trig;
    if((cmd==NULL)||(cmd->cmd!=TRIG_RECORD_EVENT))
        return -1;
    trig=(struct trig_record_event_struct *)cmd->para;
    printf("收到ipmain录像命令，channel %d, trig_flag 0x%lx,triglen %d\n",(int)trig->channel,trig->trig_flag,(int)trig->reclen);
    return trig_record_event(get_hdch((int)trig->channel),trig->trig_flag,trig->reclen);
}

static int  process_remote_trig_record(mod_socket_cmd_type *cmd)
{
    struct remote_trig_record_struct *rmt;


    if((cmd==NULL)||(cmd->cmd!=REMOTE_TRIG_RECORD))
        return -1;
    rmt = (struct remote_trig_record_struct *)cmd->para;
    if(rmt->mode==1)
    {
        remote_start_record(get_hdch((int)rmt->channel),rmt->reclen);
    }
    else
    {
        remote_stop_record(get_hdch((int)rmt->channel));
    }
    return 0;
}

static int process_alarm_snapshot(mod_socket_cmd_type *cmd)
{
    struct takepic_struct *rmt;
    struct hd_enc_struct *hd;

      /*IP1004不支持抓图，直接返回*/
      return ERR_EVC_NOT_SUPPORT;

    if((cmd==NULL)||(cmd->cmd!=ALARM_SNAPSHOT))
        return -1;
    rmt=(struct takepic_struct *)cmd->para;
    //gtloginfo("test 1,ch=%d\n",rmt->channel);
    
    hd=get_hdch(rmt->channel);
    if(hd==NULL)
        return -1;

    
    //gtloginfo("test 1\n");
    gtloginfo("收到vsmain的报警抓图命令,通道%d,间隔%d,张数%d\n",rmt->channel,rmt->interval,rmt->takepic);
    
    if((get_takingpicflag(rmt->channel)==1))//正在抓图
    {
        set_alarmpic_required(1,rmt->channel);  
        gtloginfo("%d通道正在抓图，置需要报警抓图标志，退出\n",rmt->channel);
    }
    else
    { 
        if (get_alarmpicflag(rmt->channel)==1)
        {
            set_alarmpic_required(1,rmt->channel);  
            gtloginfo("%d通道正在报警抓图，置需要报警抓图标志，退出\n",rmt->channel);
        }
        else
        {
            //gtloginfo("test,timeval %ld-%ld\n",((struct timeval *)rmt->time)->tv_sec,((struct timeval *)rmt->time)->tv_usec);
            get_time_before((struct timeval *)rmt->time,rmt->interval*(rmt->takepic-1),(struct timeval *)rmt->time);
            //gtloginfo("test,timeval %ld-%ld\n",((struct timeval *)rmt->time)->tv_sec,((struct timeval *)rmt->time)->tv_usec);
            //gtloginfo("test,snap!\n");
            set_alarmpicflag(1,rmt->channel);   
            set_takingpicflag(1,rmt->channel);  //zw-add 2012-05-14
            printf("[%s:%d]env=%d,enc=%d,dev_no=%d\n",__FILE__,__LINE__,cmd->gate.env,cmd->gate.enc,cmd->gate.dev_no);
            usr_take_pic(&cmd->gate,rmt);
        }
    }       
    return 0;
}





void process_playback_cmd(void)
{
    hd_playback_en();
}

/**********************以下是网关转发来的命令*********************/

//高清晰抓图
static int usr_get_hq_pic_cmd(struct gt_usr_cmd_struct *netcmd,gateinfo *gate)
{


      /*IP1004不支持抓图，直接返回*/
#if 0  
    struct user_get_hq_pic_struct *hqpic;
    struct timeval *time,*test;
    int i,ret;
    struct takepic_struct *takepicpara;
    DWORD buf[sizeof(struct takepic_struct)];//分配了4倍的空间
    int rc;

  
    hqpic=(struct user_get_hq_pic_struct*)netcmd->para;
    time=(struct timeval *)hqpic->time;
    
    takepicpara=(struct takepic_struct *)buf;
    
    test=(struct timeval *)takepicpara->time;
    memcpy(test,time,8);
    takepicpara->takepic=hqpic->getpic;
    takepicpara->interval=hqpic->interval;
    if(get_total_hqenc_num()==1)
        takepicpara->channel=0;
    else
        takepicpara->channel=hqpic->rec_ch;
    
    rc=ERR_DVC_BUSY;
    //gtloginfo("test,受到网关报警抓图命令,ch =%d\n",takepicpara->channel);
    for(i=0;i<5;i++)//最多等待5秒，能抓就发ack,否则发device busy
    {

        if((get_takingpicflag(takepicpara->channel)==0)&&(get_alarmpicflag(takepicpara->channel)==0))//当前所需通道没有在抓图
        {
            rc=RESULT_SUCCESS;
            set_takingpicflag(1,takepicpara->channel);
            break;
        }
        else
            sleep(1);
    }

    rc = send_ack_to_main(com_fd,HQSAVE_PROCESS_ID,USR_TAKE_HQ_PIC,(WORD)rc,gate);
    if(rc!=RESULT_SUCCESS)
    {
        gtloginfo("设备忙,不能完成%d通道抓图命令 \n",takepicpara->channel);
        return 0;
    }       
    rc = usr_take_pic(gate,takepicpara);

#endif  
    return 0;
}

//当用户要求锁全部文件时,
static int usr_lock_file_time(struct gt_usr_cmd_struct *cmd,gateinfo *gate)
{
    struct usr_lock_file_time_struct *locktime; //传来的命令结构
    struct gt_time_struct *start; //起始时间
    struct gt_time_struct *stop;  //结束时间
    struct tm timestart;
    struct tm timestop;
    long start_time,stop_time;
    //int   ch = (int)locktime->lock_ch;

    locktime = (struct usr_lock_file_time_struct *)cmd->para;
    start = &(locktime->start);
    stop = &(locktime->stop);

    
    gttime2tm(start,&timestart);
    gttime2tm(stop,&timestop);

    start_time = mktime(&timestart);
    stop_time = mktime(&timestop);
    //gtloginfo("mode %d, start-stop %d, \n",locktime->mode,start_time-stop_time);
    if((start_time>=stop_time)&&(locktime->mode==0))
    {
        gtloginfo("起始时间大于结束时间且要求解锁，取消报警标志\n");
        clear_hdmod_trig_flag(0);
    }

    return 0;
    
}


//远程控制录像起始停止
static int usr_local_record_control(struct gt_usr_cmd_struct *cmd,gateinfo *gate)
{
    int rc=0;
    WORD result;
    struct local_record_ctl_struct *record;

    record=(struct local_record_ctl_struct *)cmd->para;
    if(record->mode==1)//其他值都表示停止
    {
        rc=remote_start_record(get_hdch((int)record->rec_ch),record->rec_time);       
    }
    else
    {
        rc=remote_stop_record(get_hdch((int)record->rec_ch));
    }

    if(rc>=0)
        result=RESULT_SUCCESS;
    else
        result=ERR_DVC_INTERNAL;
    gtloginfo("高清晰录像控制结果%d\n",result);
    if(cmd->en_ack!=0)
    {
        return send_ack_to_main(com_fd,HQSAVE_PROCESS_ID,USR_LOCAL_RECORDER_CONTROL,result,gate);
    }
    return 0;

}



//处理由主进程转发过来的网关命令
static int process_gate_cmd(struct gt_usr_cmd_struct *cmd,gateinfo *gate)
{

    switch(cmd->cmd)
    {
        case USR_TAKE_HQ_PIC:
            gtloginfo("recv a USR_TAKE_HQ_PIC cmd!\n");
            usr_get_hq_pic_cmd(cmd,gate);
        break;
        case USR_LOCAL_RECORDER_CONTROL: //录像控制，改为转发 -wsy
            gtloginfo("recv a USR_LOCAL_RECORDER_CONTROL cmd!\n");
            usr_local_record_control(cmd,gate);
        break;
        case USR_LOCK_FILE_TIME: //如起始时间大于结束时间则解除报警状态
            gtloginfo("recv a USR_LOCK_FILE_TIME cmd!\n");
            usr_lock_file_time(cmd,gate);
        break;
        default:
            printf("hdmodule recv a unknow bypass cmd 0x%04x\n",cmd->cmd);
            gtloginfo("hdmodule recv a unknow bypass cmd 0x%04x\n",cmd->cmd);           
            send_ack_to_main(com_fd,HQSAVE_PROCESS_ID,cmd->cmd,ERR_EVC_NOT_SUPPORT, gate);
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
        //以前查询索引走转发，现在因为需要引入channel,故改为模块间命令
        case USR_QUERY_INDEX:
            gtloginfo("recv USR_QUERY_INDEX cmd !\n");
            usr_query_index_with_channel(cmd);
        break;
        case MAIN_QUERY_STATE:
            gtloginfo("recv MAIN_QUERY_STATE cmd !\n");
            send_state2main();
        break;
        case MAIN_REFRESH_PARA:
            gtloginfo("recv MAIN_REFRESH_PARA cmd !\n");
            refresh_hdmodule_para();
        break;
        case HDRECORD_CTRL:
            gtloginfo("recv HDRECORD_CTRL cmd !\n");        
            process_hdrecord_ctrl(cmd);
        break;
        case TRIG_RECORD_EVENT:
            process_trig_record_event(cmd);
        break;
        case REMOTE_TRIG_RECORD:
            gtloginfo("recv REMOTE_TRIG_RECORD cmd !\n");   
            process_remote_trig_record(cmd);
        break;
        case CLEAR_TRIG_FLAG:
            gtloginfo("recv CLEAR_TRIG_FLAG cmd !\n");  
            dump_clearinfo_to_log();
            clear_hdmod_trig_flag(*(int *)cmd->para);
        break;
        case ALARM_SNAPSHOT:
            gtloginfo("recv ALARM_SNAPSHOT cmd !\n");
            process_alarm_snapshot(cmd);
        break;

        case MAIN_PLAYBACK_ENC_CMD:
            gtloginfo("recv MAIN_PLAYBACK_ENC_CMD cmd!\n");
            process_playback_cmd();
            break;
            
        default:
            printf("hdmodule recv a unknow cmd %x:",cmd->cmd);
            gtloginfo("hdmodule recv a unknow cmd %x:",cmd->cmd);
        break;
    }
    return rc;
}

int creat_hdmodule_modsocket_thread(void)
{
    return creat_modsocket_thread(&modsocket_thread_id, com_fd,HQSAVE_PROCESS_ID,"hdmodule",process_modsocket_cmd);
}

