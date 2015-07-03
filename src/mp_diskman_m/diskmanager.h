#ifndef MP_DISKMANAGER_H
#define MP_DISKMANAGER_H
#include "gate_cmd.h"
#include "fileindex.h"
#include "mpdisk.h"


#define MPGDIR	"/hqdata/iframe" //存储I桢图片的目录
#define MPGNUMBER         40   //在MPGDIR下允许保留的I桢图片数
#define RM_MPG_ONCE       15    //一次删除的MPG张数

#define PICINDEXDIR "/hqdata/picindex" //存储图片索引的目录
#define PICINDEXNUMBER 		40 //在PICINDEXDIR下允许保留的文本文件数
#define RM_PICINDEX_ONCE       15    //一次删除的picindex个数

#define INDEXDIR "/hqdata/index" //存储录像文件索引的目录
#define INDEXNUMBER 40 //INDEXDIR允许保留的文本文件数
#define RM_INDEX_ONCE       15    //一次删除的index个数

#define UPDATEDIR "/log/update" //存储升级信息的目录
#define UPDATENUMBER	30
#define RM_UPDATE_ONCE	10

#define DISK_MAN_LOOP		10 //每这么多秒进行一次磁盘管理

#define SHOW_WORK_INFO //打印printf信息



struct lockfile_struct{
time_t 	start;
time_t 	stop;
int		trig;
int		ch;
int 	mode;
};
//把系统参数打印到指定文件
void dump_sysinfo(void);
//获取磁盘空余容量,单位为M
long	get_disk_free(char *mountpath);
//获取磁盘总容量,单位为M
long	get_disk_total(char *mountpath);

int disk_full(char *mountpath, int partitionindex) ;
int init_diskman(void);
int creat_diskman_thread(pthread_attr_t *attr,void *arg);
int read_diskman_para_file(char *filename);
DWORD get_diskmanstatint(void);
int isdir(char *filename);
int usr_lock_file_time(char *path,struct usr_lock_file_time_struct *lockfile);
void set_cferr_flag(int flag);
int	remove_oldest_file(void);
#endif 



