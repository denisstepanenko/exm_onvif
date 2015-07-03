#include "mp_diskman.h"
#include <mod_com.h>
#include "diskmanager.h"
#include "gt_com_api.h"
#include <gt_errlist.h>
#include "devinfo.h"
#include "hdutil.h"
#include "process_modcmd.h"
#include "mod_socket.h"
#include "mod_cmd.h"


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
    cmd->cmd    =   DISKMAN_STATE_RETURN;
    cmd->len    =   4+sizeof(pid_t);
    pid=(pid_t*)cmd->para;
    *pid=getpid();
    state=(DWORD*)&cmd->para[sizeof(pid_t)];
    *state=get_diskmanstatint();

        #if 0
        //2011-06-21 zw-modified-back
    if((*state&0x01)==0x01)
    {
        //应lx要求，修改当没有硬盘时 不报错zw-modify-2010-12-21
        return 0;
    }
        #endif
    return mod_socket_send(com_fd,MAIN_PROCESS_ID,DISKMAN_ID,cmd,sizeof(mod_socket_cmd_type)-sizeof(cmd->para)+cmd->len);
}



/*
  *让diskman重新读取配置
*/
int refresh_diskman_para(void)
{
    read_diskman_para_file(IPMAIN_PARA_FILE);
    return 0;
}




/*************************************************
函数名称：usr_lock_file_time_cmd
简要描述：处理网关发来的按时段锁文件的命令，调用和返回
输入：    文件描述符fd,网关命令结构指针*cmd
            env,enc,加密和签名算法
输出：    返回值为0表示正确，-1表示有误
修改日志：wsy@Jan11,2006创建
*************************************************/

static int usr_lock_file_time_cmd(gateinfo *gate,struct gt_usr_cmd_struct *cmd)
{
    struct usr_lock_file_time_struct *locktime; //传来的命令结构
    int ret,result; //记录返回值
    if( (gate == NULL) || (cmd == NULL) || (cmd->cmd != USR_LOCK_FILE_TIME) )
    {
        return -1;  
    }
    locktime = (struct usr_lock_file_time_struct *)cmd->para;

    gtloginfo("收到网关发来加解锁命令，%04d-%02d-%02d-%02d-%02d-%02d到%04d-%02d-%02d-%02d-%02d-%02d,模式%d,通道%d\n", locktime->start.year,locktime->start.month,locktime->start.day,locktime->start.hour,locktime->start.minute,locktime->start.second,locktime->stop.year,locktime->stop.month,locktime->stop.day,locktime->stop.hour,locktime->stop.minute,locktime->stop.second, locktime->mode, locktime->lock_ch);
    
    if(cmd->en_ack != 0)
    {
        send_ack_to_main(com_fd,DISKMAN_ID,cmd->cmd,RESULT_SUCCESS,gate);
    }

    sleep(1);//避免文件还没切分，而错过加解锁
            
    //调用lock_file_by_time函数并将返回结果送到网关
    ret = usr_lock_file_time(HDSAVE_PATH,locktime);
    if(ret >= 0)
    {
        result = RESULT_SUCCESS;
    }
    else
    {   
        result=ERR_DVC_INTERNAL;
    }
    gtloginfo("按时间段加解锁命令结果%d,%s\n", result,get_gt_errname(result));

    cmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para)-2;
    
    return 0;
    
}



/*************************************************
函数名称：lock_file_time
简要描述：处理主模块发来的按时段文件的命令，调用和返回
输入：    cmd,通过socket通道发来的命令
输出：    返回值为0表示正确，-1表示有误
修改日志：wsy@Jan11,2006创建
*************************************************/
int  lock_file_time(mod_socket_cmd_type *cmd)
{
    
    struct usr_lock_file_time_struct *locktime; //传来的命令结构
    int ret,result; //记录返回值
    
    if( (cmd == NULL) || (cmd->cmd != LOCK_FILE_TIME) )
    {
        return -1;  
    }

    locktime = (struct usr_lock_file_time_struct *)cmd->para;

    gtloginfo("收到按时段加解锁命令，%04d-%02d-%02d-%02d-%02d-%02d到%04d-%02d-%02d-%02d-%02d-%02d,模式%d,通道%d\n", locktime->start.year,locktime->start.month,locktime->start.day,locktime->start.hour,locktime->start.minute,locktime->start.second,locktime->stop.year,locktime->stop.month,locktime->stop.day,locktime->stop.hour,locktime->stop.minute,locktime->stop.second, locktime->mode, locktime->lock_ch);
    
    if(cmd->gate.gatefd > 0) //需要发回
        send_ack_to_main(com_fd,DISKMAN_ID,cmd->cmd,RESULT_SUCCESS,&cmd->gate);
    
    //调用lock_file_by_time函数并将返回结果送到网关
    ret = usr_lock_file_time(HDSAVE_PATH,locktime);
    if(ret >= 0)
    {
        result = RESULT_SUCCESS;
    }
    else
    {   
        result=ERR_DVC_INTERNAL;
    }
    gtloginfo("按时间段加解锁命令结果%d,%s\n", result,get_gt_errname(result));

    return result;
    
}


/*************************************************************************
 * 以下是主进程转发过来的网关命令
*************************************************************************/

//处理由主进程转发过来的网关命令
static int process_gate_cmd(gateinfo *gate,struct gt_usr_cmd_struct *cmd)
{
    switch(cmd->cmd)
    {
        case USR_LOCK_FILE_TIME://将指定时间段的高清晰文件加锁或解锁
            gtloginfo("recv a USR_LOCK_FILE_TIME cmd!\n");
            usr_lock_file_time_cmd(gate,cmd);
        break;
        default:
            printf("diskman recv a unknow bypass cmd 0x%04x\n",cmd->cmd);
            gtloginfo("diskman recv a unknow bypass cmd 0x%04x\n",cmd->cmd);            
            send_ack_to_main(com_fd,DISKMAN_ID,cmd->cmd,ERR_EVC_NOT_SUPPORT,gate);
        break;
    }
    
    return 0;
    
}


/*************************************************************************
 * 以上是主进程转发过来的网关命令
*************************************************************************/



static int process_modsocket_cmd(int sourceid, mod_socket_cmd_type *modsocket)
{

    int rc;
    rc=0;
    
    if(modsocket == NULL)
        return -EINVAL;
    
    switch (sourceid)
    {
        case MAIN_PROCESS_ID:   
            switch(modsocket->cmd)
            {
                case GATE_BYPASSTO_MOD_CMD://由主进程转发过来的网关命令
                    rc= process_gate_cmd(&modsocket->gate,(struct gt_usr_cmd_struct *)&modsocket->para);
                    //para的开头4个字节存放的是主进程对网关的描述符
                break;  
                case MAIN_QUERY_STATE:
                    gtloginfo("recv MAIN_QUERY_STATE cmd !\n");
                    send_state2main();
                break;
                case MAIN_REFRESH_PARA:
                    gtloginfo("recv MAIN_REFRESH_PARA cmd !\n");
                    refresh_diskman_para();
                break;
                case UNLOCK_FILE:
                    gtloginfo("recv UNLOCK_ALL_FILES cmd !\n"); 
                    hdutil_lock_all_files(0);
                break;
                case LOCK_FILE_TIME:
                    gtloginfo("recv a USR_LOCK_FILE_TIME cmd!\n");
                    lock_file_time(modsocket);  
                break;
                default:
                    printf("diskman recv a unknown cmd 0x%x:",modsocket->cmd);
                    gtloginfo("diskman recv a unknown cmd 0x%x:",modsocket->cmd);
                break;
            }
            
        break;
        default:    break;
    }
    return 0;
}



int creat_diskman_modsocket_thread(void)
{
    return creat_modsocket_thread(&modsocket_thread_id,com_fd,DISKMAN_ID,"diskman",process_modsocket_cmd);
}




