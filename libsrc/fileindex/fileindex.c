/*
		处理录像文件索引的相关函数  --wsy Dec 2007
*/


#include "fileindex.h"
#include <errno.h>
#include "gtlog.h"
#include "filelib.h"
#include "time.h"
#include "sqlite3.h"
#include "ftw.h"
#include "file_def.h"
#include "pthread.h"
#include "commonlib.h"
#include "hdutil.h"
#include "avilib.h"
#include "fixdisk.h"
#include <unistd.h>
#include <sys/time.h>
#define 	INDEX_FILE_NAME  "index.db"  //每个分区根目录下的索引文件名称
#define		INDEX_TABLE_NAME	"aviindex"		//数据库内的表格名


#define	AVIINDEX_DB_VERSION		1
//版本	1	初始化，如果版本比此更早就重建db


#define DATABASE_NAME_LEN 64
#define DATABASE_MAX 4
typedef struct 
{
	 sqlite3 *db;
        char    dbname[DATABASE_NAME_LEN];
}DB_INFO;


DB_INFO db_info[DATABASE_MAX] = {0};


int GetDabase(char *mountpath, sqlite3 **pdb)
{
    int i;
    
    for(i = 0; i <DATABASE_MAX; i++)
    {
        /*mountpath和db_info[i].dbname都是"/hqdata/sda1"
           这里为了提高速度，之比较第10位和第11位*/
        if((db_info[i].dbname[10] == mountpath[10]) && (db_info[i].dbname[11] == mountpath[11]))
        {
            *pdb = db_info[i].db;
            return 0;
        }
    }
    
    //gtlogerr("[%s:%d] 数据库%s 不存在!\n",__FILE__,__LINE__,mountpath);
    return -1;

}


/*初始化硬盘上的数据库，使用mpdisk_process_all_partitions比较合理，但是
mpdisk_process_all_partitions在mpdisk模块中，mp_diskman和mp_hdmodule调用时太繁琐*/
int InitAllDabase()
{

    int             i = 0;
    int             ret;
    sqlite3      *pdb = NULL;
    

    memset(db_info, 0, sizeof(db_info));

    if(access("/hqdata/sda1/index.db", F_OK) == 0 )//不存在
    {
        
        ret = fileindex_open_db("/hqdata/sda1", &pdb);
        if(ret != 0)
        {
            gtlogerr("无法打开%s下的数据库,原因%s\n","/hqdata/sda1/index.db",sqlite3_errmsg(pdb));
        }
        else
        {
            strcpy(db_info[i].dbname,"/hqdata/sda1");
            db_info[i].db = pdb;
            gtloginfo("打开%s成功,db:%x\n","/hqdata/sda1/index.db",db_info[i].db);
        }
        //sqlite3_exec(pdb, "PRAGMA synchronous=off;", 0,0,0);
    }
    else
    {
            gtlogerr("没有%s数据库,硬盘坏或没有挂接\n","/hqdata/sda1/index.db");
    }    
 
    if(access("/hqdata/sda2/index.db", F_OK) == 0 )//不存在
    {
            
        i++;
        ret = fileindex_open_db("/hqdata/sda2", &pdb);
        if(ret != 0)
        {
            gtlogerr("无法打开%s下的数据库,原因%s\n","/hqdata/sda2/index.db",sqlite3_errmsg(pdb));
        }
        else
        {
            strcpy(db_info[i].dbname,"/hqdata/sda2");
            db_info[i].db = pdb;
            gtloginfo("打开%s成功,db:%x\n","/hqdata/sda2/index.db",db_info[i].db);
        }
        //sqlite3_exec(pdb, "PRAGMA synchronous=off;", 0,0,0);
    }
    else
    {
            gtlogerr("没有%s数据库,硬盘坏或没有挂接\n","/hqdata/sda2/index.db");
    }
    


    
    if(access("/hqdata/sda3/index.db", F_OK) == 0 )//不存在
    {
            
        i++;
        ret = fileindex_open_db("/hqdata/sda3", &pdb);
        if(ret != 0)
        {
            gtlogerr("无法打开%s下的数据库,原因%s\n","/hqdata/sda3/index.db",sqlite3_errmsg(pdb));
        }
        else
        {
        
            strcpy(db_info[i].dbname,"/hqdata/sda3");
            db_info[i].db = pdb;
            gtloginfo("打开%s成功,db:%x\n","/hqdata/sda3/index.db",db_info[i].db);
        }
        //sqlite3_exec(pdb, "PRAGMA synchronous=off;", 0,0,0);
    }
    else
    {
            gtlogerr("没有%s数据库,硬盘坏或没有挂接\n","/hqdata/sda3/index.db");
    }


     
    if(access("/hqdata/sda4/index.db", F_OK) == 0 )//不存在
    {
            
        i++;
        ret = fileindex_open_db("/hqdata/sda4", &pdb);
        if(ret != 0)
        {
            gtlogerr("无法打开%s下的数据库,原因%s\n","/hqdata/sda4/index.db",sqlite3_errmsg(pdb));
        }
        else
        {
            strcpy(db_info[i].dbname,"/hqdata/sda4");
            db_info[i].db = pdb;
            gtloginfo("打开%s成功,db:%x\n","/hqdata/sda4/index.db",db_info[i].db);
        }
        //sqlite3_exec(pdb, "PRAGMA synchronous=off;", 0,0,0);
    }
    else
    {
            gtlogerr("没有%s数据库,硬盘坏或没有挂接\n","/hqdata/sda4/index.db");
    }    

    return 0;

}

int CloseAllDabase()
{

    int            i = 0;

    for(i = 0; i <DATABASE_MAX; i++)
    {
        if(db_info[i].db != NULL)
        {
            sqlite3_close(db_info[i].db);
        }
    }
    
    return 0;

}




/*
		新的供调用的接口
*/

/****************************************************************************************
 *函数名称:pathname2indexname()
 * 功能:   将分区挂载点名称如"/hqdata/hda2"翻译成索引名称如"/hqdata/hda2/index.db"
 *输入 :   mountpath   将要修改的路径名 如:/hqdata/hda2
 *输出 :   indexname   返回的字符串，如:/hqdata/hda2/index.db
 *返回 :   正确返回0 ，错误返回负值
 * **************************************************************************************/
int pathname2indexname(IN char *mountpath, OUT char* indexname)
{
    if((mountpath == NULL)||(indexname == NULL))
        return -EINVAL;
        
    sprintf(indexname, "%s/%s",mountpath,INDEX_FILE_NAME);  
    return 0;
}


/*******************************************************************************************
 *函数名:  exec_sqlcmd()
 *功能  :  执行sql语句
 *输入  :  db         由open返回的指向db数据库文件的句柄， 
 *         sqlcmd     需要执行的sql语句
 *         callback   回调函数
 *         para       传送回调函数的参数
 *返回值:  正确返回0 ， 错误返回负值
 *******************************************************************************************/
int exec_sqlcmd(sqlite3 *db, char *sqlcmd,  int (*callback)(void*,int,char**,char**), void * para)
{

    int ret = SQLITE_BUSY;
    char *errmsg = NULL;
    
    if((db==NULL)||(sqlcmd == NULL))
        return -EINVAL;

    while((ret==SQLITE_BUSY)||(ret==SQLITE_LOCKED))
    {
        ret = sqlite3_exec(db,sqlcmd, callback,para, &errmsg);
        if(ret == 0)
        {
            if(errmsg) 
            {
                sqlite3_free(errmsg);
            }
            break;
        }
        else
        {       
            gtlogerr("[%s:%d]sqlite3_exec()[%s]...error=%d:%s\n",__FILE__,__LINE__,sqlcmd,ret,errmsg);
            if(errmsg) 
            {
                sqlite3_free(errmsg);
            }
            usleep(10);
        }
    }   

    return ret;
    
}


/**********************************************************************************************
 *函数名: isdir()
 *功能  : 检测filename指向的文件是否为一个目录
 *输入  : filename    文件名
 *输出  : void
 *返回值: 0是目录，1不是目录
 * ********************************************************************************************/
int isdir(char *filename)
{

    struct stat statbuf;

    if(stat(filename,&statbuf)==-1)
    {
            printf("Get stat on %s Error￡o%s\n",filename,strerror(errno));
            return(-1);
    }

    if(S_ISDIR(statbuf.st_mode))
    {
    //printf("%s is dir\n",filename);
            return 0;
    }
    else
    {
    //printf("%s is not dir\n",filename);
            return 1;
    }
    
}


int fileindex_open_db(char *mountpath, sqlite3 **pdb)
{
	char indexname[100];
	
	if(mountpath == NULL)
		return -EINVAL;
		
	pathname2indexname(mountpath,indexname);
	return sqlite3_open(indexname,pdb);
      

}


int fileindex_close_db(IN char* mountpath, IN sqlite3* db)
{
	if(db == NULL)
		return -EINVAL;
	sqlite3_close(db);
	return 0;
}

//初始化并创建表，返回值有意义
int fileindex_init_db(IN char * partition)
{
    int ret;
    char sqlcmd[200];
    sqlite3 *db;
    char *errmsg=NULL;
    
    if((partition == NULL)||(isdir(partition)))
        return -EINVAL;
    
    ret=fileindex_open_db(partition,&db);
    if(ret != 0)
    {
        gtlogerr("无法打开%s下的数据库,原因%s\n",partition,sqlite3_errmsg(db));
        return ret; 
    }

    //创建表
    sprintf(sqlcmd,"create table %s(name nvarchar[80], start integer, stop integer, ch integer, trig integer,ing integer,lock integer, version integer)",INDEX_TABLE_NAME);
    ret = sqlite3_exec(db, sqlcmd, NULL,NULL, &errmsg);
    if(ret != 0)
    {
        gtlogerr("无法创建表%s,原因%s\n",INDEX_TABLE_NAME,sqlite3_errmsg(db));
    }

    if(errmsg) 
    {
        sqlite3_free(errmsg);
    }
    
    fileindex_close_db(partition, db);
    return ret; 
    
}

/*************************************************************************
 * 	函数名:	fileindex_add()
 *	功能:	把一条文件名加入其所在的分区的索引文件的末尾
 *	输入:	db,数据库指针
 *			filename,录像文件名
 *	输出:	
 * 	返回值:	成功返回0,否则返回负值
 *************************************************************************/
int fileindex_add(IN sqlite3 *db, IN char *filename)
{
    char sqlcmd[200];
    struct file_info_struct finfo;
    
    if((filename == NULL)||(db==NULL))
        return -EINVAL;
    
    hdutil_filename2finfo(filename, &finfo);
    //插入
    sprintf(sqlcmd,"insert into %s values ('%s','%d','%d','%d','%d','%d','%d','%d')",INDEX_TABLE_NAME,filename,(int)finfo.stime,(int)finfo.stime+finfo.len,finfo.channel,finfo.trig,finfo.ing_flag,finfo.lock,AVIINDEX_DB_VERSION);
    return exec_sqlcmd(db, sqlcmd,NULL,NULL);
}

//sqlite3 *db3 = NULL;
int fileindex_add_to_partition(IN char* mountpath, IN char *filename)
{
    sqlite3 *db=NULL;
    int  ret;
    
    if((mountpath == NULL)||(filename == NULL))
        return -EINVAL;


    ret = GetDabase(mountpath,&db);
    //db = db_info[1].db;
    //printf("[%s:%d] 数据库%s :不存在!\n",__FILE__,__LINE__,mountpath);
    if(ret != 0)
    {
        //gtloginfo("[%s:%d] 数据库%s :不存在!\n",__FILE__,__LINE__,mountpath);
        return 0;
    }
    
    fileindex_add(db,filename);
    //fileindex_close_db(mountpath,db);

    return 0;
}

int fileindex_del_file(sqlite3* db, char *filename)
{
    char sqlcmd[200];
    
    if((db==NULL)||(filename==NULL))
        return -EINVAL;
    
    sprintf(sqlcmd,"delete from %s where name = '%s'",INDEX_TABLE_NAME,filename);
    return exec_sqlcmd(db,sqlcmd,NULL,NULL);
    
}




int del_file_callback(void *db, int argc, char **value, char **name)
{

    int ret;
    int retf;
    
    if(db == NULL)
        return 0;

    //printf("delete file %s \n",value[0]);

    //查看需要删除的那个录像文件是否存在    
    if(access(value[0],F_OK)!=0)//已经不存在了
    {
        gtlogerr("删除%s 文件，但是该文件已经不存在了",value[0]);
        return 0;

    }

    errno = 0;
    ret = remove(value[0]);
    if(ret != 0)
    {
        gtloginfo("删除%s失败,错误号%d:%s\n",value[0],errno,strerror(errno));
        fix_disk(value[0],errno);
    }
    
    return 0;
    
}


int get_version_callback(void *arg, int argc, char **value, char **name)
{

    int *version;
    
    if(arg == NULL)
        return -EINVAL;
    
    version = (int *)arg;
    *version =atoi(value[0]);
    return 0;
}

//返回db version,若还没有database则返回0
int get_db_version(IN char *dbname)
{

    char sqlcmd[200];
    sqlite3 *db;
    char *errmsg=NULL;  
    int version=0;
    
    if(dbname == NULL)
        return -EINVAL;
    
    sqlite3_open(dbname,&db);
    sprintf(sqlcmd,"select version from aviindex order by start limit 1");
    exec_sqlcmd(db, sqlcmd,get_version_callback,&version);
    sqlite3_close(db);
    
    return version;
    
}

/*************************************************************************
 * 	函数名:	fileindex_del_oldest()
 *	功能:	从分区索引中删去最老的未加锁记录并删除相应文件
 *	输入:	mountpath,分区名称，形如/hqdata/hda1
 *			no,需要删除的文件数目
 *	输出:	
 * 	返回值:	成功返回0,否则返回负值
 *************************************************************************/
//static sqlite3 *db1 = NULL;
int fileindex_del_oldest(IN char* mountpath, int no)
{
    char sqlcmd[200];
    int ret;
    sqlite3 *db= NULL;

    if((mountpath == NULL)||(no <=0))
    {
        printf("[%s:%d]mountpath==NULL or no <0\n",__FILE__,__LINE__);
        return -EINVAL;
    }

//     if(db1 == NULL)
//     {
//        fileindex_open_db(mountpath,&db);
//        db1 = db;
//    }
//    else
//        db = db1;
    ret = GetDabase(mountpath,&db);
    if(ret != 0)
    {
        gtloginfo("[%s:%d] 数据库%s :不存在!\n",__FILE__,__LINE__,mountpath);
        return 0;
    }
    //先查询后删除具体文件
    sprintf(sqlcmd,"select name from %s where lock = 0 and ing = 0 order by start limit %d",INDEX_TABLE_NAME,no);
    ret=exec_sqlcmd(db,sqlcmd,del_file_callback,db);
    if(ret!=0)
    {
        gtlogerr("sqlite3执行删除最老录像文件失败,ret=%d\n",ret);
    }

    sprintf(sqlcmd,"delete from %s where name in (select name from %s where lock = 0 and ing = 0 order by start limit %d)",INDEX_TABLE_NAME,INDEX_TABLE_NAME,no);
    ret=exec_sqlcmd(db,sqlcmd,NULL,NULL);
    if(ret!=0)
    {
        gtlogerr("sqlite3执行删除最老录像文件失败,ret=%d\n",ret);
    }

    //fileindex_close_db(mountpath,db);
    return ret;
}

/*************************************************************************
 * 	函数名:	fileindex_rename_in_partition()
 *	功能:	将分区索引文件的指定文件名改成给定的文件名
 *	输入:	mountpath,分区名称
 *			oldname,旧名称
 			newname,新名称
 *	输出:	
 * 	返回值:	成功返回0,否则返回负值
 *************************************************************************/
int fileindex_rename_in_partition(IN char*mountpath, IN char *oldname, IN char* newname)
{
    sqlite3 *db= NULL;
    int ret;
    
    if((mountpath == NULL)||(oldname == NULL)||(newname == NULL))
        return -EINVAL;

    //fileindex_open_db(mountpath,&db);
    ret = GetDabase(mountpath,&db);
    if(ret != 0)
    {
        gtloginfo("[%s:%d] 数据库%s :不存在!\n",__FILE__,__LINE__,mountpath);
        return 0;
    }
    ret = fileindex_rename(db, oldname,newname);
    //fileindex_close_db(mountpath,db);

    return ret;
}



int fileindex_rename(IN sqlite3 *db, IN char *oldname, IN char *newname)
{
    int ret;
    if((db == NULL)||(oldname == NULL)||(newname == NULL))
        return -EINVAL;
    
    if(access(oldname,F_OK)!=0)
    {
        printf("欲重命名%s为%s但前者已不存在\n",oldname,newname);
        fileindex_del_file(db,oldname);
        return -EPERM;
    }
    ret = fileindex_add(db, newname);
    if(ret!=0)
        gtloginfo("in rename fileindex_add %s return %d\n",newname,ret);
    ret = rename(oldname,newname);
    //sprintf(sqlcmd,"update %s set name ='%s' and stop = '%d' and ing = '%d' and lock ='%d' where name='%s'",INDEX_TABLE_NAME,newname,finfo.stime+finfo.len,finfo.ing_flag,finfo.lock,oldname);
    
    ret =fileindex_del_file(db,oldname);
    
    if(ret!=0)
        gtloginfo("in rename fileindex_del %s return %d\n",oldname,ret);
    
    return ret;
}


int lock_file_callback(void *indexname, int argc, char **value, char **name)
{
    char oldname[200];
    char *filename; 
    int mode;
    FILE *fp= NULL;
    int ret;
    
    if(indexname==NULL)
        return 0;
    sprintf(oldname,value[0]);
    
    //将生成的文件名写入文件
    fp=fopen(indexname,"a+");
    if(fp!=NULL)
    {
        fprintf(fp,"%s\n",oldname);
        fclose(fp);
    }
    
    return 0;
}




int fileindex_lock_by_time(IN char* mountpath, IN int flag,IN int  starttime, IN int stoptime, IN int  trig, IN int ch)
{
	char sqlcmd[400];
	char indexname[200];
	char cmd[200];
	char oldname[200];
	char tname[200];
	FILE *fp=NULL;
	struct timeval tv;
       struct timezone tz;	
	sqlite3* db;
       int ret;
	
	if(mountpath==NULL)
		return -EINVAL;
	//fileindex_open_db(mountpath,&db);
	ret = GetDabase(mountpath,&db);
       if(ret != 0)
       {
            gtloginfo("[%s:%d] 数据库%s :不存在!\n",__FILE__,__LINE__,mountpath);
            return 0;
       }
	//先查询后操作具体文件
	sprintf(sqlcmd,"select name from %s where lock = %d and ing = 0",INDEX_TABLE_NAME,1-flag); 
	if(starttime!= -1)
	{
		sprintf(cmd," and start <= %d ",stoptime);
		strcat(sqlcmd,cmd);
	}
	if(stoptime != -1)
	{
		sprintf(cmd,"and stop >= %d ",starttime);
		strcat(sqlcmd,cmd);
	}
	if(trig != -1)
	{
		sprintf(cmd,"and trig =  %d ",trig);
		strcat(sqlcmd,cmd);
	}
	if(ch != -1)
	{
		sprintf(cmd,"and ch =  %d",ch);
		strcat(sqlcmd,cmd);
	}
	//生成索引文件
	gettimeofday(&tv,&tz);
	sprintf(indexname,"/hqdata/%ld-%06d-lockindex.txt",tv.tv_sec,tv.tv_usec);
	exec_sqlcmd(db,sqlcmd,lock_file_callback,indexname);

	//按索引文件执行具体操作
	fp=fopen(indexname,"r");
	if(fp!=NULL)
	{
		while(fgets(oldname,200,fp)!=NULL)
		{
			oldname[(strlen(oldname)-1)] = '\0';
			if(flag == 0)
				hdutil_unlock_filename(oldname,tname);
			else
				hdutil_lock_filename(oldname,tname);
			fileindex_rename(db, oldname,tname); 
		}
		
	}
	//删除索引文件
	//remove(indexname);--暂时留在内存中
	//fileindex_close_db(mountpath,db);
	return 0;
}



//被fileindex_create_index中的ftw调用，为一条文件创建索引
int create_index_for_file_fn (char *file, struct stat *sb, int flag, void *user_data)
{
	sqlite3 *db;
	int ret;
	if(user_data==NULL)
		return 0;
	
	db = (sqlite3 *)user_data;
	if((flag==FTW_F)&&((strstr(file,IMG_FILE_EXT)!=NULL)||(strstr(file,RECORDING_FILE_EXT)!=NULL))) 
 	{
 		ret = fileindex_add(db, file); 
  	}	
 	return 0;
} 

/*************************************************************************
 * 	函数名:	fileindex_create_index()
 *	功能:	为整个分区下的录像文件创建索引
 *	输入:	path，分区挂载名称,如"/hqdata/hda2"
 *			forced: 0表示当前没有索引或索引过时才创建,1表示无论如何都重新创建，1
 *	输出:	
 * 	返回值:	成功返回0,否则返回负值
 *************************************************************************/
int fileindex_create_index(IN  char *path, IN int forced)
{
	sqlite3 *db;
	char cmd[200];
	char indexname[200];
	int ret;
	struct stat statbuf;

	if((path==NULL)||(get_disk_total(path)<200))
		return -EINVAL;
	
  //Generating the database index file /hqdata/hda1/index.db	
	pathname2indexname(path, indexname);		
	if(forced == 0) //没有索引时才创建
	{
		if(access(indexname,F_OK|W_OK|R_OK)==0)//有索引
		{
			stat(indexname,&statbuf);
			if(statbuf.st_size!=0)	//索引大小不为0
			{
				//判断是否数据库版本和当前一致
				if((get_db_version(indexname))==AVIINDEX_DB_VERSION)
					return 0;
			}
		}
	}	
	ret = remove(indexname);

	//增加标志文件
	sprintf(cmd,"touch %s/creating_db",path);
	system(cmd);
	fileindex_init_db(path);

	gtopenlog("hdutil");	
	gtloginfo("为%s分区重建数据库\n",path);

	sqlite3_open(indexname, &db);
	ret = ftw_sort_user(path, create_index_for_file_fn, db, FTW_SORT_ALPHA, 0, fix_disk_ftw);
	sqlite3_close(db);
  
	gtopenlog("hdutil");	
	gtloginfo("为%s分区创建数据库完毕\n",path);
	
	//删除标志文件
	sprintf(cmd,"rm -rf %s/creating_db",path);
	system(cmd);
	return ret;
}




int get_filetime_callback(void *time, int argc, char **value, char **name)
{
	int *timep;
	timep = (int *)time;
	
	if(time == NULL)
		return 0;
	
	*timep = atoi(value[0]);
	return 0;
}



/*************************************************************************
 * 	函数名:	fileindex_get_oldest_file_time()
 *	功能:	获取分区中最老的可删除文件的创建时间
 *	输入:	mountpath，分区挂载名称,如"/hqdata/hda2"
 *	输出:	
 * 	返回值:	成功返回创建时间,否则返回负值
 *************************************************************************/
int fileindex_get_oldest_file_time(char *mountpath)
{
	char sqlcmd[200];
	int time=0;
       int ret;
	sqlite3 *db;
	
	if(mountpath == NULL)
		return -EINVAL;
	
	//fileindex_open_db(mountpath, &db);
	ret = GetDabase(mountpath,&db);
       if(ret != 0)
       {
            gtloginfo("[%s:%d] 数据库%s :不存在!\n",__FILE__,__LINE__,mountpath);
            return 0;
       }    
	sprintf(sqlcmd,"select start from %s where ing = 0 and lock =0 order by start  limit 1",INDEX_TABLE_NAME);
	exec_sqlcmd(db,sqlcmd,get_filetime_callback ,&time);
	//fileindex_close_db(mountpath,db);
	return time;
}



//将一条.ING文件转成.AVI文件
/*****************************************************************************
 *函数名: convert_ing_file()
 *功能  : 将一条.ING文件转化为.AVI文件
 *输入  : db        打开sqlite2后open返回的句柄
 *        ingfile   ing文件名
 *输出  : finalname 转换后的文件名
 *返回值: 正确返回0，错误返回负值
 * ***************************************************************************/
int convert_ing_file(IN sqlite3 *db, IN char *ingfile, OUT char * finalname)
{
    char aviname[200];//xx_L00.AVI
    char oldaviname[200];//xx_L00_OLD.AVI
    char oldname[200];
    char tmp[50];
    struct stat fstate;
    long freesize;
    int time;
    char *lp,*lk;
    int ret;
    struct file_info_struct info;
    
  //检查参数有效性
    if((ingfile == NULL)||(finalname == NULL)||(db == NULL))
    {
        return 0;
    }
  //检查指定ing文件的有效性
    if(access(ingfile,F_OK)!=0)//文件不存在
    {
        gtloginfo("索引中的ing文件%s实际已不存在\n",ingfile);
        fileindex_del_file(db,ingfile);
        return 0;
    }
    printf("find ing file  name is %s\n",ingfile);
    gtloginfo("找到一条ing文件，名为%s\n",ingfile);
    lp = strstr(ingfile,RECORDING_FILE_EXT);
    if(lp == NULL)
    {
        gtlogerr("[%s:%d]解析文件名[%s]错误\n",__FILE__,__LINE__,ingfile);
        return 0;
    }

    //zw-modified 2012-04-20
    ret=strlen(lp);
    if(ret>4)
    {
        // 此处为ps录像文件
        *lp='\0';
        sprintf(oldaviname,"%s_OLD%s",ingfile,"-ps.mpg");     //oldaviname=xxxx_OLD-ps.mpg
        sprintf(aviname,"%s%s",ingfile,"-ps.mpg");            //aviname=xxxxx-ps.mpg
        strcat(ingfile,".ING");                   //ingfile=xxxx.ING-ps.mpg
        strcat(ingfile,"-ps.mpg");
    } 
    else if(ret>0)
    {
        //此处为一般的xxxx.ING文件
        *lp='\0';
        sprintf(oldaviname,"%s%s",ingfile,OLD_FILE_EXT);  //oldaviname=xxxx_OLD.AVI
        sprintf(aviname,"%s%s",ingfile,IMG_FILE_EXT);     //aviname=xxxx.GAVI
        strcat(ingfile,RECORDING_FILE_EXT);               //ingfile=xxxx.ING
    }

    printf("[%s:%d]ingfile=%s\n",__FILE__,__LINE__,ingfile);

    //将xxx_OLD.AVI文件名写入数据库，并将文件本身改名为xxx_OLD.AVI
    fileindex_add(db,oldaviname);
    ret = rename(ingfile,oldaviname);//先把ing文件重命名为xxx_OLD.AVI或 xxxx_OLD-ps.mpg
    if(ret!=0)
    {
        printf("将%s重命名为%s失败返回%d,无法修理\n",ingfile,oldaviname,ret);
        gtloginfo("将%s重命名为%s失败返回%d,无法修理\n",ingfile,oldaviname,ret);
        fileindex_del_file(db, oldaviname);
        return 0;
    
    }
  //删除数据库中名为xxxx.ING的文件记录,因为这个文件已经改名为xxx_OLD.AVI,并且已经加入了数据库记录
    fileindex_del_file(db,ingfile);
    //进行判断，如果剩余空间小于该文件的大小，则不进行修理
    ret=stat(oldaviname,&fstate);
    if(ret<0)
    {
        printf("取%s文件状态失败返回%d,无法修理\n",oldaviname,ret);
        gtloginfo("取%s文件状态失败返回%d,无法修理\n",oldaviname,ret);
        return 0;
    }
  //从录像文件的名字中解析文件的属性    
    hdutil_filename2finfo(oldaviname, &info);
    freesize = get_disk_free(info.partition);
    if(freesize < (fstate.st_size>>20))
    {
        
        printf("%s剩余空间%ldM不够,暂不修理文件%s\n",info.partition,freesize,oldaviname);
        gtloginfo("%s剩余空间%ldM不够,暂不修理文件%s\n",info.partition,freesize,oldaviname);
        return 0;
    }

    lp=strstr(oldaviname,"ps");
    if(lp!=NULL)
    {
        //此时为ps的文件 
        time=22;
    }   
    else
    {
        //此时为avi后缀的普通文件
        //进行转换
        time=avi_fix_bad_file(oldaviname);  //在原来的文件中添加avi格式的索引才能被播放器播放,因为这些文件是处于非正常停止录像的，后缀为ING
    }
    
    if(time>=0) //转换成功,取时间长度，重命名OLD.AVI为.AVI
    {
        lp=index(aviname,'L');
        if(lp==NULL)
        {
            printf("invalid file name\n");
        }
        else
        {
            lp++;                 //lp=00_T00.AVI
            printf("[%s:%d]lp=%s\n",__FILE__,__LINE__,lp);
            lk=index(lp,'_');     //lk=_T00.AVI
            printf("[%s:%d]lk=%s\n",__FILE__,__LINE__,lk);
            strncpy(tmp,lk,40);   //tmp=_T00.AVI
            *lp='\0';
            printf("[%s:%d]lp=%s\n",__FILE__,__LINE__,lp);
            sprintf(finalname,"%s%02d%s",aviname,time,tmp);
            printf("[%s:%d]finalname=%s\n",__FILE__,__LINE__,finalname);
            fileindex_add(db,finalname);
            if(rename(oldaviname,finalname)==0)
            {
                fileindex_del_file(db,oldaviname);
            }
            else
            {
                fileindex_del_file(db,finalname);
            }
        }
    }   
    else//转换失败，保持现状
    {
        sprintf(finalname,oldaviname);
        printf("修理ING文件 %s failed ret=%d!\n",oldaviname,time);  
        gtloginfo("convert_ing_to_avi %s failed ret=%d!\n",oldaviname,time);
    }
    return 0;
}


int convert_ing_callback(void *db, int argc, char **value, char **name)
{
	char fname[100];
	char finalname[100];
	char *filename;	
	
	if(db==NULL)
		return 0;
	
	return convert_ing_file((sqlite3*) db,value[0],finalname);
}



int fileindex_convert_ing(char *mountpath)
{
	sqlite3 *db = NULL;
	int ret;
	char sqlcmd[200];
	
	if(mountpath == NULL)
		return -EINVAL;
	
	//fileindex_open_db(mountpath, &db);
	ret = GetDabase(mountpath,&db);
       if(ret != 0)
       {
            //gtloginfo("[%s:%d] 数据库%s :不存在!\n",__FILE__,__LINE__,mountpath);
            return 0;
       }
	sprintf(sqlcmd,"select * from %s where ing = '1'",INDEX_TABLE_NAME);
	
	ret =exec_sqlcmd(db,sqlcmd, convert_ing_callback, db);
	//fileindex_close_db(mountpath, db);
	return 0;
}


int queryindex_callback(void *fp, int argc, char **value, char **name)
{
	char fname[100];
	char finalname[100];
	char *filename;	
	int len;

	//printf("come to queryindex callback with %s! strlen %d\n",value[0],strlen(value[0]));
	
	if(fp==NULL)
		return 0;
	
	len = strlen(value[0]);
	strncpy(fname,value[0],len);
	fname[len]='\0';
	if(access(fname,F_OK)!=0)//不存在
	{
		return 0;
	}
	strncpy(finalname,fname+7,len-7);
	finalname[len-7]='\0';
	fprintf((FILE *)fp,"%s\n",finalname);
	return 0;
}

int fileindex_query_index(char *mountpath, struct query_index_process_struct *qindex)
{
	sqlite3 *db;
	int ret = 0;
	char sqlcmd[200];
	char cmd[20];
	
	if((mountpath==NULL)||(qindex==NULL))
		return -EINVAL;
	//ret = fileindex_open_db(mountpath, &db);
	ret = GetDabase(mountpath,&db);
       if(ret != 0)
       {
            gtloginfo("[%s:%d] 数据库%s :不存在!\n",__FILE__,__LINE__,mountpath);
            return 0;
       }
    
	sprintf(sqlcmd,"select name from %s where  start <= %d and stop >= %d  and ing = 0 ",INDEX_TABLE_NAME,qindex->stop,qindex->start);
	if(qindex->trig_flag != -1)
	{
		sprintf(cmd," and trig = %d",qindex->trig_flag);
		strcat(sqlcmd,cmd);
	}
	
	if(qindex->ch != -1)
	{
		sprintf(cmd," and ch = %d",qindex->ch);
		strcat(sqlcmd,cmd);
	}
	exec_sqlcmd(db,sqlcmd, queryindex_callback,qindex->index_fp);
	//fileindex_close_db(mountpath,db);
	return 0;
}









