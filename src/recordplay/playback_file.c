#include <signal.h>
#include <devinfo.h>
#include <commonlib.h>
#include <gtthread.h>
#include "diskinfo.h"
#include "mpdisk.h"
#include "hdutil.h"
#include "playback.h"
#include "fileindex.h"
#include "playback_file.h"
#include "gtsocket.h"
#include "gtsf.h"



int query_index_in_partition(char *devname, char* mountpath, void *fn_arg)
{

    query_index_struct * query;
    
    if((mountpath == NULL)||(fn_arg == NULL))
    {
        return -EINVAL;
    }
        
    query = (query_index_struct *) fn_arg;
 
    if(query->index_fp == NULL)
    {
        return -EINVAL;
    }
    return fileindex_query_index(mountpath,query);
}

int query_record_timesection(char *tsname, int ch, time_t start, time_t stop)
{
    int result;//查询结果
    FILE *fp,*fwrite;
    struct stat statbuf;
    query_index_struct queryindex;
    char path[20];
    char cmd[200];
    char tmpname[100];//存放排序前的索引名称
    char  filename[MAX_FILE_NAME_SIZE];
    char  timestring[200];   	
    struct file_info_struct fileinfo;
    int     starttime = 0, preendtime = 0;
    struct tm *p_starttm,*p_endtm;
    char  tmp[200];
    char indexname[200];

    
    
    sprintf(path,"%s/index",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    sprintf(tmpname,"%s/index/%d-tmp.txt",HDSAVE_PATH,(int)start);
    fp=fopen(tmpname,"w+");
    if(fp==NULL)
    {
        printf("[Func]:%s [file open err]:%s\n", __FUNCTION__,tmpname);
        return PLAYBACK_ERR_FILE_ACCESS;
    }
    
    queryindex.index_fp =   fp;
    queryindex.ch       =   -1;//ch;ip1004只有4画面，全部查
    queryindex.start    =   start;
    queryindex.stop     =   stop;
    queryindex.trig_flag =   -1; 
    result = mpdisk_process_all_partitions(query_index_in_partition, &queryindex);

    fclose(fp);
    stat(tmpname,&statbuf);
    if(statbuf.st_size == 0)
    {

       printf("[Func]:%s [file size]:%d\n", __FUNCTION__,(int)statbuf.st_size);
       return PLAYBACK_ERR_NO_FILE;
 
    }

    printf("生成临时文件:%s\n",tmpname);
    sprintf(indexname,"%s/index/%d.txt",HDSAVE_PATH,(int)start);
    sprintf(cmd,"/ip1004/record_sort %s %s",tmpname,indexname);
    system(cmd);
    sprintf(indexname,"%s/index/%d.txt",HDSAVE_PATH,(int)start);

    printf("生成临时文件:%s\n",indexname);
    printf("删除临时文件:%s\n",tmpname);
    /*删除临时文件*/
    sprintf(cmd,"rm -rf %s",tmpname);
    system(cmd);

    /*计算时间段*/
    fp = fopen(indexname,"r");
    if(fp == NULL)    
    {
         printf("[Func]:%s [Line]:%d [buflen]:%s\n", __FUNCTION__, __LINE__, "index is NULL");
         return PLAYBACK_ERR_NO_FILE;  
    }


    sprintf(tmpname,"%s/index/%d-time.txt",HDSAVE_PATH,(int)start);
    printf("生成临时文件:%s\n",tmpname);
    fwrite = fopen(tmpname,"w+");
    if(fwrite == NULL)
    {
        printf("[Func]:%s [file open err]:%s\n", __FUNCTION__,tmpname);
        return PLAYBACK_ERR_FILE_ACCESS;
    }
    
    while(fgets(filename,MAX_FILE_NAME_SIZE,fp) != NULL)
    {
       hdutil_filename2finfo(filename,&fileinfo);
       if(starttime == 0)
       {
            starttime =  fileinfo.stime;
            preendtime = starttime;
       }

       /*如果时间差在3秒之内算同一时间段*/
       if(abs(preendtime -fileinfo.stime) > 3)
       {
            /*写时间段starttime，endtime*/
            printf("starttime:%d~endtime:%d\n",starttime,preendtime);
            p_starttm = gmtime((const time_t  *)&starttime); /*获取GMT时间*/
            sprintf(timestring,"%04d-%02d-%02d %02d:%02d:%02d~",
            p_starttm->tm_year+1900, p_starttm->tm_mon+1, p_starttm->tm_mday,
            p_starttm->tm_hour, p_starttm->tm_min,  p_starttm->tm_sec);
            p_endtm = gmtime((const time_t  *)&preendtime); 

            sprintf(tmp,"%04d-%02d-%02d %02d:%02d:%02d",
            p_endtm->tm_year+1900, p_endtm->tm_mon+1, p_endtm->tm_mday,
            p_endtm->tm_hour, p_endtm->tm_min,  p_endtm->tm_sec);
            strcat(timestring,tmp);
            fprintf(fwrite,"%s\n",timestring);
            printf("时间段:%s\n",timestring);
            
            starttime = fileinfo.stime;
       }
       preendtime = fileinfo.stime  + fileinfo.len;
       
    }

    printf("starttime:%d~endtime:%d\n",starttime,preendtime);
    p_starttm = gmtime((const time_t  *)&starttime); /*获取GMT时间*/
    sprintf(timestring,"%04d-%02d-%02d %02d:%02d:%02d~",
    p_starttm->tm_year+1900, p_starttm->tm_mon+1, p_starttm->tm_mday,
    p_starttm->tm_hour, p_starttm->tm_min,  p_starttm->tm_sec);
    p_endtm = gmtime((const time_t  *)&preendtime); 

    sprintf(tmp,"%04d-%02d-%02d %02d:%02d:%02d",
    p_endtm->tm_year+1900, p_endtm->tm_mon+1, p_endtm->tm_mday,
    p_endtm->tm_hour, p_endtm->tm_min,  p_endtm->tm_sec);
    strcat(timestring,tmp);
    fprintf(fwrite,"%s\n",timestring);
    printf("时间段:%s\n",timestring);
    fclose(fp);
    fclose(fwrite);
    

    /*删除临时文件*/
    sprintf(cmd,"rm -rf %s",indexname);
    system(cmd);
    sprintf(tsname,"/index/%d-time.txt",(int)start);
    printf("删除临时文件:%s\n",indexname);
    printf("返回文件%s\n",tsname);
    
    return PLAYBACK_SUCCESS;

}



//改为从索引文件中查,
int query_record_index(char *indexname, int queryid, int ch, time_t start, time_t stop)
{
    int result;//查询结果
    FILE *fp;
    struct stat statbuf;
    query_index_struct queryindex;
    char path[20];
    char cmd[200];
    char tmpname[100];//存放排序前的索引名称
    
    
    sprintf(path,"%s/index",HDSAVE_PATH);
    if(access(path,F_OK)!=0)
        mkdir(path,0755);
    sprintf(tmpname,"%s/index/%d-%d-tmp.txt",HDSAVE_PATH,(int)start,queryid);
    fp=fopen(tmpname,"w+");
    if(fp==NULL)
    {
        printf("[Func]:%s [file open err]:%s\n", __FUNCTION__,tmpname);
        return PLAYBACK_ERR_FILE_ACCESS;
    }
    
    queryindex.index_fp =   fp;
    queryindex.ch       =   ch;//ip1004只有4画面，全部查
    queryindex.start    =   start;
    queryindex.stop     =   stop;
    queryindex.trig_flag =   -1; 
    result = mpdisk_process_all_partitions(query_index_in_partition, &queryindex);

    fclose(fp);
    stat(tmpname,&statbuf);
    if(statbuf.st_size == 0)
    {

       printf("[Func]:%s [file size]:%d\n", __FUNCTION__,(int)statbuf.st_size);
       return PLAYBACK_ERR_NO_FILE;
 
    }
 
    sprintf(indexname,"%s/index/%d-%d.txt",HDSAVE_PATH,(int)start,queryid);
    sprintf(cmd,"/ip1004/record_sort %s %s",tmpname,indexname);
    system(cmd);
    
    sprintf(cmd,"rm -rf %s",tmpname);
    system(cmd);    
    sprintf(indexname,"%s/index/%d-%d.txt",HDSAVE_PATH,(int)start,queryid);
    return PLAYBACK_SUCCESS;

}


int playback_openfile(struct hd_playback_struct *phdplayback)
{

    int     result;
    FILE  *fp = NULL;
    char  firstfile[MAX_FILE_NAME_SIZE];
    char  lastfile[MAX_FILE_NAME_SIZE];
    char  openfile[MAX_FILE_NAME_SIZE];   	
    struct file_info_struct firstinfo;
    int     passtime = 0;
    int     lastf;                                      /*结束帧计数，只有播放到最后一个文件时使用*/
    avi_t * avifile =NULL;

    memset(firstfile, 0, sizeof(firstfile));
    memset(lastfile, 0, sizeof(lastfile));
    
    result = 
    query_record_index(phdplayback->Indexfilename,phdplayback->playbackindex, phdplayback->channel,
                    phdplayback->start, phdplayback->stop);

    if(result != PLAYBACK_SUCCESS)
    {
        return result;
    }

    phdplayback->frames = 0;
    phdplayback->fileindex = 0;
    phdplayback->recordfiletotal = 0;


    /*打开临时的索引文件，查找第一个录像文件并打开，
       定位开始播放的位置，如果只有一个录像文件，还要定位
       播放的结束位置*/
    fp = fopen(phdplayback->Indexfilename,"r");
    if(fp == NULL)    
    {
         printf("[Func]:%s [Line]:%d [buflen]:%s\n", __FUNCTION__, __LINE__, "index is NULL");
         return PLAYBACK_ERR_NO_FILE;  
    }


    if(fgets(firstfile,MAX_FILE_NAME_SIZE,fp) == NULL)
    {
       printf("[Func]:%s [Line]:%d [buflen]:%s\n", __FUNCTION__, __LINE__, "index is NULL");
       return PLAYBACK_ERR_NO_FILE;
    }
    
    firstfile[(strlen(firstfile)-1)] = '\0';
    phdplayback->recordfiletotal++;
    while(fgets(lastfile,MAX_FILE_NAME_SIZE,fp) != NULL)
    {
        phdplayback->recordfiletotal++;
    }
    lastfile[(strlen(lastfile)-1)] = '\0';
	

    fclose(fp);
    printf("first record name:%s, last record name:%s. total:%d\n", firstfile,lastfile,phdplayback->recordfiletotal);

    /*定位开始帧*/
    hdutil_filename2finfo(firstfile,&firstinfo);
    if(phdplayback->start > firstinfo.stime)
    {
        passtime = phdplayback->start - firstinfo.stime;
        printf("[Func]:%s [Line]:%d [buflen]:%s,跨过%d秒\n", __FUNCTION__, __LINE__, firstfile,passtime);
        //return PLAYBACK_ERR_FILE_ACCESS;
    }
    printf("first record name:%s. last:%s, total:%d\n", firstfile,lastfile,phdplayback->recordfiletotal);
    sprintf(openfile,"%s%s","/hqdata",firstfile);
    avifile = AVI_open_input_file(openfile, 1);
    if(avifile == NULL)
    {
        printf("[Func]:%s [Line]:%d [buflen]:%s%s\n", __FUNCTION__, __LINE__, openfile,"open error");
        return PLAYBACK_ERR_FILE_ACCESS;
    }

    /*获取开始播放要跨过的帧*/
    if(passtime != 0 )
    {
        if(avifile->fps == 0)
        {
            avifile->video_pos = passtime*PLAYBACK_FRAMERATE_25;
        }
        else
        {
            avifile->video_pos = passtime*avifile->fps;
        }
     }

    if(avifile->video_pos < 0 || avifile->video_pos >= avifile->video_frames)
    {
    
        printf("[Func]:%s [frame pos]:%d [frames]:%d\n", __FUNCTION__,(int)(avifile->video_pos),(int)(avifile->video_frames));
        AVI_close(avifile);
        return PLAYBACK_ERR_FILE_ACCESS;
    }

    phdplayback->aviinfo = avifile;
    
    printf("第一个文件%s,跨过%d秒，跳了%d帧，总共帧数:%d\n",
        firstfile,passtime,(int)(avifile->video_pos),(int)(avifile->video_frames));


    /*只有一个文件,第一个文件就是最后一个文件*/
    if(phdplayback->recordfiletotal == 1)
    {
        /*定位结束帧*/
        //hdutil_filename2finfo(firstfile,&firstinfo);
        /*phdplayback->stop的结束时间是下一秒*/
        passtime = phdplayback->stop+1 - firstinfo.stime;
        if(phdplayback->stop  < firstinfo.stime)
        {
            printf("[Func]:%s [Line]:%d [buflen]:%s%d\n", __FUNCTION__, __LINE__, firstfile,passtime);
            return PLAYBACK_ERR_FILE_ACCESS;
        }
        
        /*获取要跨过的帧*/
        if(passtime != 0 )
        {
            if(avifile->fps == 0)
            {
                lastf = passtime*PLAYBACK_FRAMERATE_25;
            }
            else
            {
                lastf = passtime*avifile->fps;
            }
            /*因为前面有认为调整(end+1)，所以有越界的可能*/
            if(lastf > avifile->video_frames)
            {
                lastf = avifile->video_frames;
            }
            
            phdplayback->lastframe = lastf;
        }
        
    
    }

    phdplayback->fileindex++;

/*定位文件指针*/


    if(!avifile->video_index)
    { 
        printf("[Func]:%s [Line]:%d [buflen]:%s\n", __FUNCTION__, __LINE__, "index is NULL");
        return PLAYBACK_ERR_FILE_ACCESS;
    }

    if(avifile->video_pos < 0 || avifile->video_pos >= avifile->video_frames)
    {
    
        printf("[Func]:%s [frame pos]:%d [frames]:%d\n", __FUNCTION__,(int)(avifile->video_pos),(int)(avifile->video_frames));
        return PLAYBACK_ERR_FILE_ACCESS;
    }
/*    
    n = avifile->video_index[avifile->video_pos].len;

    if(bufflen < n)
    {
        printf("[Func]:%s [frame size]:%d [buflen]:%d\n", __FUNCTION__,(int)n, bufflen);
        return PLAYBACK_ERR_BUF_SIZE;
    }

    *keyframe = (avifile->video_index[avifile->video_pos].key==0x10) ? 1:0;
*/

    lseek(avifile->fdes, avifile->video_index[avifile->video_pos].pos-8, SEEK_SET);

/*  这些将放在读数据帧的函数里面    
    if (avi_read(AVI->fdes,vidbuf,n) != n)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "AVI READ");
        return PLAYBACK_ERR_FILE_ACCESS;
    }

    AVI->video_pos++;
*/    

/*定位文件指针*/
    
    return result;


}

           
    
  /**   
      *   计算两个时间的间隔，得到时间差   
      *   @param   struct   timeval*   resule   返回计算出来的时间   
      *   @param   struct   timeval*   x             需要计算的前一个时间   
      *   @param   struct   timeval*   y             需要计算的后一个时间   
      *   return   -1   failure   ,0   success   
  **/   
  int   timeval_subtract(struct   timeval*   result,   struct   timeval*   x,   struct   timeval*   y)   
  {   

        if(x->tv_sec > y->tv_sec)   
            return   -1;   
    
        if((x->tv_sec == y->tv_sec) && (x->tv_usec > y->tv_usec))   
            return   -1;   
    
        result->tv_sec = (y->tv_sec-x->tv_sec);   
        result->tv_usec = (y->tv_usec-x->tv_usec);   
    
        if(result->tv_usec < 0)   
        {   
            result->tv_sec--;   
            result->tv_usec += 1000000;   
        }   
    
        return   0;   
  }   
    
void milliseconds_sleep(unsigned long mSec){
    struct timeval tv;
    tv.tv_sec=mSec/1000;
    tv.tv_usec=(mSec%1000)*1000;
    int err;
    do{
       err=select(0,NULL,NULL,NULL,&tv);
    }while(err<0 && errno==EINTR);
}

extern struct hd_playback_struct g_playback[PLAYBACK_NUM];

static unsigned short  playback_get_source()
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
	for(i=0;i<PLAYBACK_NUM;i++)
	{
		sprintf(secbuf,"audio%d:interval",i);
		g_playback[i].audio_source=iniparser_getint(ini,secbuf,1);
	}
  
    if(fp!=NULL)
    {
        unlock_file(fileno(fp));
        fsync(fileno(fp));
        fclose(fp);
    }   
    iniparser_freedict(ini);
    
    return 0;
    
}


void playback_process(void * ch)
{

    int  result;
    char *pdata;
    int  isvedio;
    int  getbufflen;
    int  isIframe;
    int  speed = PLAYBACK_NSPEED;
    gtsf_stream_fmt  *pStream;
    stream_video_format_t  *pV_fmt;
    stream_audio_format_t  *pA_fmt;
    stream_format_t  *pmedia_format;
    struct   timeval   start,stop,diff;
    int  frame_interval =  40000;                           /*理论的帧间隔*/
    int channel=(int)ch;

    printf("playback thread %d  start\n", channel);

	if(playback_get_source()<0)
		return;


    playback_struct *phdplayback = getplayback(channel);
    if(phdplayback == NULL)
    {
        printf("phdplayback:%d  is NULL", channel);
        return ;
    }

    pStream = (gtsf_stream_fmt *)malloc(PLAYBACK_BUFF_LEN);
    if(pStream == NULL)
    {
        printf("memory error, malloc:%d  is NULL", channel);
        return ;
    }

    gettimeofday(&start,0); 
    
    while(1)
    {
    
        /*先检查有没有操作,如果有，先处理
          这些操作*/

         
        /*停止网络回放*/
        if(phdplayback->oper == PLAYBACK_CTRL_CLOSE)
        {
            printf("phdplayback %d stop\n", channel);
            playbackClose(channel);
        }

        /*暂停时操作码不修改*/
        if(phdplayback->oper == PLAYBACK_CTRL_PAUSE)
        {
            printf("phdplayback %d pause\n", channel);
            usleep(40000);
            continue;
        }

        if(phdplayback->oper == PLAYBACK_CTRL_SEEK)
        {
            char indexfile[256];
            printf("phdplayback %d seek \n", channel);

            sprintf(indexfile,"rm -rf %s",phdplayback->Indexfilename);
            system(indexfile);

            if(phdplayback->aviinfo != NULL)
            {
                AVI_close(phdplayback->aviinfo);
                phdplayback->aviinfo = NULL;
            }
            result = playback_openfile(phdplayback);
            if(result != PLAYBACK_SUCCESS)
            {
                printf(" playback_openfile error:%x,read file:%d  is over \n", result,channel);
                playbackClose(channel);
            }
            phdplayback->oper = PLAYBACK_CTRL_IDLE;
            continue;
        }

        if(phdplayback->state != PLAYBACK_STAT_OK)
        {
            usleep(40000);
            continue;
        }

        
        isIframe = 0;
        pdata =  (char*)pStream + GTSF_HEAD_SIZE;     
        result = playbackreadfileframe(phdplayback, pdata, PLAYBACK_BUFF_LEN, &isvedio, 
        &getbufflen,&isIframe);

        if(result == 0)
        {
            printf(" playback,read file:%d  is over \n", channel);
            playbackClose(channel);
            continue;
        }
        else if(result < 0)
        {
            printf(" playback,read file:%d error \n", channel);
            playbackClose(channel);
            continue;
        }


        /*计算帧间隔的时间，注意在只传I帧时,不是I帧的数据会扔掉*/
        if(speed != phdplayback->speed)
        {
            printf("channel :%d speed change, %d-->%d",channel, speed, phdplayback->speed);
            speed = phdplayback->speed;
            
        }         
        if(speed  == PLAYBACK_QSPEED)
        {
            frame_interval = 160000; 
        }
        else if(speed  == PLAYBACK_HSPEED)
        {
            frame_interval = 80000; 
        }
        else if(speed  == PLAYBACK_2SPEED)
        {
            frame_interval = 20000; 
        }
        else if(speed  == PLAYBACK_4SPEED)
        {
            frame_interval = 10000; 
        }
        else if(speed  == PLAYBACK_ISPEED)
        {
            frame_interval = 40000; 
        }
        else
        {
            /*NSPEED*/
            frame_interval = 40000;
        }
        

        /*数据打包*/
        pStream->mark = GTSF_MARK;
        pStream->encrypt_type = 0;
        pStream->len = getbufflen;
        if(isvedio == PLAYBACK_TRUE)
        {
            pStream->type = MEDIA_VIDEO;
            pmedia_format = &pStream->media_format;
            pV_fmt = (stream_video_format_t  *)&pmedia_format->v_fmt;
            pV_fmt->format = VIDEO_H264;
            if(isIframe == PLAYBACK_TRUE)
                pV_fmt->type = FRAMETYPE_I;
            else
                pV_fmt->type = FRAMETYPE_P;

            if(( phdplayback->aviinfo->width == 720)&&(phdplayback->aviinfo->height== 576))
            {
                pV_fmt->ratio = RATIO_D1_PAL;
            }
            else if(( phdplayback->aviinfo->width == 704)&&(phdplayback->aviinfo->height== 576))
            {
                pV_fmt->ratio = RATIO_D1_NTSC;
            }
            else if(( phdplayback->aviinfo->width == 352)&&(phdplayback->aviinfo->height== 288))
            {
                pV_fmt->ratio = RATIO_CIF_PAL;
            }
            else if(( phdplayback->aviinfo->width == 320)&&(phdplayback->aviinfo->height== 240))
            {
                pV_fmt->ratio = RATIO_CIF_NTSC;
            }            
        }
        else
        {
/*
	waveform.wFormatTag		=	WAVE_FORMAT_MULAW;
	waveform.nChannels		=	1;
	waveform.nSamplesPerSec	=	8000;
	waveform.nAvgBytesPerSec=	8000;
 	waveform.nBlockAlign	=	1;
	waveform.wBitsPerSample	=	8;
	waveform.cbSize			=	0;	

*/
			pStream->type = MEDIA_AUDIO;
            pmedia_format = &pStream->media_format;
            pA_fmt = (stream_audio_format_t  *)&pmedia_format->a_fmt;

			if(phdplayback->audio_source==1)
			{
				//internel
				pA_fmt->a_channel = 1;
				pA_fmt->a_wformat = 7;
				pA_fmt->a_sampling = 8000;
				pA_fmt->a_bits = 16;
				pA_fmt->a_bitrate = 64;
			}
			else
			{
				//outsource
				pA_fmt->a_channel = 2;
				pA_fmt ->a_wformat = 255;
				pA_fmt->a_sampling = 16000;
				pA_fmt->a_bits = 16;
				pA_fmt->a_bitrate = 0;

			}
            
        }
#if 0        
    {
        int i;
        char *p = (char *)pStream;
        for(i = 0;i<32;i++)
       {
                printf("%x ",p[i]);
                if(i%32 ==0)
                {
                    printf("\n");
                }
       }

    }
#endif
        if((speed  == PLAYBACK_ISPEED) &&(isIframe == 0))
        {
            printf("discard packet,channel:%d  type %d\n",channel,pStream->type);
            ;//do noting
        }
        else
        {
            result = fd_write_buf(phdplayback->socket,(char*)pStream,getbufflen+GTSF_HEAD_SIZE);
            //printf("channel:%d tcp type:%d,isIframe:%d send %d, speed:%d  result %d\n",channel, pStream->type,isIframe,getbufflen,speed,result);  
            if(result <= 0)
            {
                printf(" playback, net send:%d  is over \n", channel);
                playbackClose(channel);
                continue;
            }
            phdplayback->packetsum++;

        }
        
        /*帧率控制，开始*/

        /*音频不控制*/
        if(isvedio != PLAYBACK_TRUE)
            continue;

       gettimeofday(&stop,0);
       timeval_subtract(&diff,&start,&stop);
       //printf("总计用时:%d秒%d 微秒\n",diff.tv_sec, diff.tv_usec); 

        /*大于1秒了，直接下一个*/
        if(diff.tv_sec > 0)
        {
           gettimeofday(&start,0); 
           continue;
        }

        //printf("speed:%d frame_interval:%d, use:%d\n",speed,frame_interval,diff.tv_usec); 
        if(frame_interval > diff.tv_usec)
        {
             if(frame_interval - diff.tv_usec >9000)
                usleep(frame_interval - diff.tv_usec -9000);
        }
        gettimeofday(&start,0); 
         /*帧率控制，结束*/
        
    }
    
   free(pStream);
   playbackClose(channel);

       
}
void readfile_thread(int playbackIndex)
{
    int thread_node_t = -1;
    pthread_t thread_test;

    thread_node_t = GT_CreateThread(&thread_test, (void *)playback_process, (void *)playbackIndex);                  

    if(thread_node_t == 0)
    {
        printf("创建第%d路回放线程成功\n", playbackIndex);
    }
    else 
    {
        printf("创建第%d路线程失败\n", playbackIndex);
    }
}
static size_t avi_read(int fd, char *buf, size_t len)
{
   size_t n = 0;
   size_t r = 0;

   while (r < len) {
      n = read (fd, buf + r, len - r);

      if (n <= 0)
          break;
      r += n;
   }
   return r;
}

/*
static long playback_read_frame(avi_t *AVI, char *vidbuf, int bufflen,  int *keyframe)
{

    long n;

    if(AVI->mode==AVI_MODE_WRITE) 
    {
        return PLAYBACK_ERR_FILE_ACCESS; 
    }
    if(!AVI->video_index)
    { 
        printf("[Func]:%s [Line]:%d [buflen]:%s\n", __FUNCTION__, __LINE__, "index is NULL");
        return PLAYBACK_ERR_FILE_ACCESS;
    }

    if(AVI->video_pos < 0 || AVI->video_pos >= AVI->video_frames)
    {
    
        printf("[Func]:%s [frame pos]:%d [frames]:%d\n", __FUNCTION__,(int)(AVI->video_pos),(int)(AVI->video_frames));
        return PLAYBACK_ERR_FILE_ACCESS;
    }
    n = AVI->video_index[AVI->video_pos].len;

    if(bufflen < n)
    {
        printf("[Func]:%s [frame size]:%d [buflen]:%d\n", __FUNCTION__,(int)n, bufflen);
        return PLAYBACK_ERR_BUF_SIZE;
    }

    *keyframe = (AVI->video_index[AVI->video_pos].key==0x10) ? 1:0;

    lseek(AVI->fdes, AVI->video_index[AVI->video_pos].pos, SEEK_SET);
    if (avi_read(AVI->fdes,vidbuf,n) != n)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "AVI READ");
        return PLAYBACK_ERR_FILE_ACCESS;
    }

    AVI->video_pos++;

    return n;

}
*/


int playbackreadfileframe(playback_struct *phdplayback, void *buf, int buf_len, int 
*isvideo, int *getbuflen, int *flag)
{

    FILE *fp = NULL;
    char  filename[MAX_FILE_NAME_SIZE];
    char  openfile[MAX_FILE_NAME_SIZE];
    int     result;
    int     passtime;
    int     getfileindex = 0;
    struct file_info_struct fileinfo;
  
    

    int     lastf;                                      /*结束帧计数，只有播放到最后一个文件时使用*/
    avi_t * avifile =NULL;

    if(phdplayback == NULL)
    {
        return PLAYBACK_ERR_PARAM;
    }

    /*先看是不是最后一个文件*/
    if(phdplayback->fileindex == phdplayback->recordfiletotal)
    {
        if(phdplayback->aviinfo->video_pos >= phdplayback->lastframe)
        {
            /*播放结束*/
            printf("播放结束:共播放%d个文件\n",phdplayback->recordfiletotal);
            AVI_close(phdplayback->aviinfo);
            phdplayback->aviinfo = NULL;
            return 0;
        }
    }
    else
    {
        if(phdplayback->aviinfo->video_pos >= (phdplayback->aviinfo->video_frames -1))
        {
            /*播放一个文件结束*/
            //printf("playback the %d record name:%s\n", getfileindex,filename);    
            AVI_close(phdplayback->aviinfo);
            phdplayback->aviinfo = NULL;

            /*获取下一个文件名*/
            fp = fopen(phdplayback->Indexfilename,"r");
            if(fp != NULL)
            {

                while(fgets(filename,MAX_FILE_NAME_SIZE,fp)!=NULL)
                {
                    getfileindex++;
                    if(getfileindex == (phdplayback->fileindex + 1))
                    {
                        break;
                    }
                }
                filename[(strlen(filename)-1)] = '\0';
            }
            fclose(fp);
            printf("open the %d record name:%s\n", getfileindex,filename);            
            /*打开下一个文件*/
            hdutil_filename2finfo(filename,&fileinfo);
            sprintf(openfile,"%s%s","/hqdata",filename);
            avifile = AVI_open_input_file(openfile, 1);
            if(avifile == NULL)
            {
                printf("[Func]:%s [Line]:%d [buflen]:%s%s\n", __FUNCTION__, __LINE__, filename,"open error");
                return PLAYBACK_ERR_FILE_ACCESS;
            }
            phdplayback->aviinfo = avifile;
            /*最后一个文件*/
            if(phdplayback->recordfiletotal == (phdplayback->fileindex + 1))
            {
                /*定位结束帧*/
                /*phdplayback->stop的结束时间是下一秒*/
                
                passtime = phdplayback->stop+1 - fileinfo.stime;
                if(phdplayback->stop  < fileinfo.stime)
                {
                    printf("[Func]:%s [Line]:%d [buflen]:%s%d\n", __FUNCTION__, __LINE__, filename,passtime);
                    return PLAYBACK_ERR_FILE_ACCESS;
                }
                
                /*获取要跨过的帧*/
                if(passtime != 0)
                {
                    if(avifile->fps == 0)
                    {
                        lastf = passtime*PLAYBACK_FRAMERATE_25;
                    }
                    else
                    {
                        lastf = passtime*avifile->fps;
                    }
                    /*因为前面有认为调整(end+1)，所以有越界的可能*/
                    if(lastf > avifile->video_frames)
                    {
                        lastf = avifile->video_frames;
                    }
                    
                    phdplayback->lastframe = lastf;
                }
                printf("the last file, name:%s. 播放:%d秒,帧:%d total:%d\n", 
                filename,passtime,lastf,phdplayback->recordfiletotal);

            
            }
            phdplayback->fileindex++;
            phdplayback->aviinfo->video_pos = 0;

            if(!phdplayback->aviinfo->video_index)
            { 
                printf("[Func]:%s [Line]:%d [buflen]:%s\n", __FUNCTION__, __LINE__, "index is NULL");
                return PLAYBACK_ERR_FILE_ACCESS;
            }

            lseek(avifile->fdes, avifile->video_index[avifile->video_pos].pos-8, SEEK_SET);
  
                   
        }
    
    }

    if(phdplayback->aviinfo == NULL)
    {
        /*查找下一个录像文件*/
         printf("播放错误,第%d 个录像文件没有打开\n",phdplayback->fileindex);
         return PLAYBACK_ERR_FILE_ACCESS;
    }


//    result = playback_read_frame(phdplayback->aviinfo, buf, buf_len,  flag);
  
    result = AVI_read_data(phdplayback->aviinfo,  buf, buf_len,
                               buf, buf_len,
                              (long*)getbuflen);
/*
 * Return codes:
 *
 *    1 = video data read
 *    2 = audio data read
 *    0 = reached EOF
 *   -1 = video buffer too small
 *   -2 = audio buffer too small
 */     
    if(result == 1)
    {
        *isvideo = 1;
        if(!phdplayback->aviinfo->video_index)         
        { 
            return PLAYBACK_ERR_FILE_NOINDEX;
        }

        if(phdplayback->aviinfo->video_pos < 0 || phdplayback->aviinfo->video_pos > phdplayback->aviinfo->video_frames) 
        {
            return PLAYBACK_ERR_FILE_NOINDEX;
        }


        *flag = 
        (phdplayback->aviinfo->video_index[phdplayback->aviinfo->video_pos-1].key==0x10) ? 1:0;
        //lc inavailted
		//printf("read frame,type:%d len %d is video:%d , video frame:%d fileindex:%d total:%d\n", *flag,*getbuflen,result,phdplayback->aviinfo->video_pos,phdplayback->fileindex,phdplayback->recordfiletotal);
        if((phdplayback->aviinfo->video_pos %10 == 0)&&(*isvideo == 1))
        {
            //printf("\nread frame,len %d is video:%d , video frame:%d fileindex:%d total:%d\n", *getbuflen,result,phdplayback->aviinfo->video_pos,phdplayback->fileindex,phdplayback->recordfiletotal);
        }        
    }
    else if(result == 2)
    {
        *isvideo = 0;
    }
    else if(result == 0)
    {
        printf("file read over \n");
        return result;
    }
    else
    {
        printf("error:[Func]:%s [Line]:%d :read file errior :%d \n", __FUNCTION__, 
        __LINE__, result);
        return result;
    }

    return result;
}


int playbackclosefile(struct hd_playback_struct *phdplayback)
{
    //int ret;
    char indexfile[256];

    sprintf(indexfile,"rm -rf %s",phdplayback->Indexfilename);
    system(indexfile);

	printf("phdplayback->aviinfo is %p,socket is %d\n",phdplayback->aviinfo,phdplayback->socket);
    if(phdplayback->aviinfo != NULL)
    {
        AVI_close(phdplayback->aviinfo);
        phdplayback->aviinfo = NULL;
    }
    if(phdplayback->socket != 0)
    {
        close(phdplayback->socket);
        phdplayback->socket = 0;
    }
	//phdplayback->state=PLAYBACK_STAT_IDLE;
	
    return PLAYBACK_SUCCESS;
}


  
/*没有用，为了编译通过*/
void set_cferr_flag(int flag)
{   


}


