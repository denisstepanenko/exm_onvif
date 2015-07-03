#ifndef PLAYBACK_FILE_H
#define PLAYBACK_FILE_H

typedef struct{
    time_t   start;
    int         fd;
    int         IsUsed;//0,没有使用，1使用
}SOCKS_TABLE;
#define MAX_PLAYBACK_NUM 5



typedef struct query_index_process_struct query_index_struct; 


int playback_openfile(struct hd_playback_struct *phdplayback);
int playbackreadfileframe(playback_struct *phdplayback, void *buf, int buf_len, int 
*isvideo, int *getbuflen, int *flag);
int playbackclosefile(struct hd_playback_struct *phdplayback);
int query_record_timesection(char *tsname, int ch, time_t start, time_t stop);
void readfile_thread(int playbackIndex);

#endif



