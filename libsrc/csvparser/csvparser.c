
/**
 * csv格式解析库
 * 说明: csv格式是由','分隔的记录
 * csv格式:
 * .每条记录占一行 
 * .以逗号为分隔符 
 * .逗号前后的空格会被忽略 
 * ?.字段中包含有逗号，该字段必须用双引号括起来 
 * ?.字段中包含有换行符，该字段必须用双引号括起来 
 * ?.字段前后包含有空格，该字段必须用双引号括起来 
 * ?.字段中的双引号用两个双引号表示 
 * ?.字段中如果有双引号，该字段必须用双引号括起来 
 * .第一行记录，可以是字段名 
 */
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <file_def.h>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <csvparser.h>
//#include <gtthread.h>
#include <commonlib.h>

#define MAGIC			0xeb90eb90
#define TEMP_REC_FILE	"/tmp/csvtemp.txt"
#define INSERT_HEAD		0		//从文件头插入记录信息
#define INSERT_TAIL		1		//从文件尾部插入记录信息
#define CHANGE_MID		2		//修改/添加文件中间的记录信息
#define INSERT_MID		3		//从文件中间插入/添加记录信息

#define CUT_HEAD		0		
#define CUT_MID			1
#define CUT_TAIL			2

#define VERSION			"0.02"
/*
V0.02增加了一个给定行号，删除指定一行纪录的函数
修改了文件中没有记录返回0的问题
*/


typedef struct 
{
	int atom_num;			//元素的个数
	unsigned long int magic;	//交验字
//存放元素字段
	char csv_buf[MAX_ATOM_NUM][MAX_ATOM_LEN];		
//存放完整记录信息
	char csv_str[MAX_ATOM_LEN*MAX_ATOM_NUM+MAX_ATOM_NUM];	
}CSV_ST;

//static pthread_mutex_t	CSV_file_mutex=PTHREAD_MUTEX_INITIALIZER;		
//其它值待定
/*
*************************************************************************
*函数名	:rm_unused_space
*功能	: 删除字符串前后的空格，调整后存放在原来的缓冲区
*输入	:  char* para
*输出	: char* para
*修改日志:
*************************************************************************
*/
static __inline__ int csv_rm_unused_space(char * para)
{	
	char *p_str;
	int i;
	int len;
	char buf[MAX_ATOM_LEN];
	char swap_buf[MAX_ATOM_LEN];
	if(para==NULL)
	{
		return -CSV_PARA_ERR;
	}
	if(strlen(para)>sizeof(buf))
	{
		return -CSV_NO_MEM;
	}
	memset(buf, 0, sizeof(buf));
	memset(swap_buf, 0, sizeof(swap_buf));
	if(strlen(para)>0)
	{
		memcpy(buf, para, strlen(para));
	}
	else
	{
		return CSV_SUCCESS;
	}
	len = strlen(para);
	///remove space at end of string
	for(i=0;i<len;i++)
	{
		p_str=strrchr(buf, ' ');
		if(p_str!=NULL)
		{
			if(p_str==(buf+strlen(buf)-1))
			{
				*p_str = 0;
				continue;
			}
			else
			{ 
				break;
			}
		}
		else
		{
			break;
		}
	}

	///remove space at beginning of string
	for(i=0;i<len;i++)
	{
		p_str=strchr(buf, ' ');
		if(p_str!=NULL)
		{
			if(p_str==buf)
			{
				memcpy(swap_buf, &buf[1], (strlen(buf)-1));
				memset(buf, 0, strlen(buf));
				memcpy(buf,swap_buf, strlen(swap_buf));
				memset(swap_buf, 0, sizeof(swap_buf));
				continue;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	memset(para, 0 ,len);
	memcpy(para, &buf, strlen(buf));
//	printf("%s\n", para);
	return CSV_SUCCESS;
}
/*
*************************************************************************
*函数名	:csv_rm_enter
*功能	: 删除字符串后的换行
*输入	:  char* para
*输出	: char* para
*修改日志:
*************************************************************************
*/
static __inline__ int csv_rm_enter(char * para)
{
	int len;
	if(para==NULL)
		return -CSV_PARA_ERR;
	len = strlen(para);
	if((para[len-1]==0x0a)&&(para[len-2]==0x0d))
		para[len-2]=0;
	return CSV_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
/*
*************************************************************************
*函数名	:csv_get_line_num
*功能	: 将用户指定的行号转化成正序的行号
*输入	:  
			int total  总共包含的行数
			int no 	用户需要操作的行号
*输出	: 	正序的操作行号
*修改日志:
*************************************************************************
*/
static __inline__ int csv_get_line_num(int total, int no)
{
	if(no>0)
	{
		if(no>total)
		{
			return total;
		}
		else
		{
			return no;
		}
	}
	if(no<0)
	{
		if(-no>total)
		{
			return 1;
		}
		else
		{
			return total+no+1;
		}
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
/*
*************************************************************************
*函数名	:csv_create
*功能	: 创建一个解析csv记录的数据结构
*输入	:  无
*输出	: 
*修改日志:
*************************************************************************
*/
 CSV_T	*csv_create(void)
{
	CSV_ST* cs = NULL;
	cs = malloc(sizeof(CSV_ST));
	if(cs==NULL)
	{
		return NULL;
	}
	cs->magic = MAGIC;
	cs->atom_num = 0;
	memset(cs->csv_buf, 0 , sizeof(cs->csv_buf));
	memset(cs->csv_str, 0 , sizeof(cs->csv_str));
	return cs;
}
/*
*************************************************************************
*函数名	:csv_set_str
*功能	:设置csv结构中变量的值(字符串型)
* 注意  :如果相同位置的数据存在则进行替换原有值
*输入	:  
		CSV_T	*csv,          	之前使用csv_create得到的指针
		const 	int no,		变量序号(正序)
		const char * value 	变量值 
*输出	: 0表示成功负值表示出错
*修改日志:
*************************************************************************
*/
 int csv_set_str(CSV_T	*csv, const int no, const char  *value)
{
	CSV_ST *cs=NULL;
	int index;
	cs = (CSV_ST*)csv;
	if((cs==NULL)||((cs->magic)!=MAGIC))
	{
		return -CSV_PARA_ERR;
	}
		
	if((no<=0)||(value==NULL)||(no>MAX_ATOM_NUM)||(strlen(value)>MAX_ATOM_LEN))
	{
		return -CSV_PARA_ERR;
	}
	index = no-1;
	if(cs->csv_buf[index][0]==0)
	{
		cs->atom_num++;
	}
	else
	{
		memset(cs->csv_buf[index], 0 , sizeof(cs->csv_buf[index]));
	}
	memcpy(cs->csv_buf[index], value, strlen(value));
	csv_rm_unused_space(cs->csv_buf[index]);
	return CSV_SUCCESS;
}
/*
*************************************************************************
*函数名	:csv_set_int
*功能	:设置csv结构中变量的值(整型值)
* 注意  :如果相同位置的数据存在则进行替换原有值
*输入	:  
		CSV_T	*csv,          	之前使用csv_create得到的指针
		const char * name, 	< 名 
		const char * value 	< 值 
*输出	: 0表示成功负值表示出错
*修改日志:
*************************************************************************
*/
 int csv_set_int(CSV_T	*csv,    const int	no, const int	value )
{
	char buf[MAX_ATOM_LEN];
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", value);
	return csv_set_str(csv , no , (const char *)buf);
}

/* 
 *************************************************************************
 *函数名:csv_get_int
 *功能	:读取csv结构中指定序号的变量(整数型)
 *输入	:  
		IN CSV_T 	*csv, 		//之前使用csv_create得到的指针
		IN const int	no, 		//变量序号
		IN const int 	def_val		//没有找到变量时的返回值
 *返回值: 	指定序号变量的值,没有找到则返回def_val
 *修改日志:
 *************************************************************************
*/
int csv_get_int(CSV_T 	*csv, const int no, const int def_val	)
{
	const char* p_str=NULL;
	char buf[MAX_ATOM_LEN];
	
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", def_val);
	p_str = csv_get_str(csv,no,(const char *)buf);
	if(p_str == buf)
	{
		return def_val;		
	}
	return atoi(p_str);
}
/* 
 *************************************************************************
 *函数名:csv_get_str
 *功能	:读取csv结构中指定序号的变量(字符串型)
 *输入	:  
		IN CSV_T 	*csv, 		//之前使用csv_create得到的指针
		IN const int	no, 		//变量序号
		IN const char 	*def_val	//没有找到变量时的返回值
 *返回值: 	指定序号变量的值,没有找到则返回def_val
 *修改日志:
 *************************************************************************
 */
const char *csv_get_str(CSV_T *csv, const int no, const char *def_val)
{
	CSV_ST* cs=NULL;
	int index;
	cs = (CSV_ST*)csv;
	if((cs==NULL)||((cs->magic)!=MAGIC))
	{
		return def_val;
	}
////如果没有对应序号的参数就返回默认值
	if((no>MAX_ATOM_NUM)||(no<=0))
	{
		return def_val;
	}
	index = no-1;
	if((cs->csv_buf[index][0])==0)
	{
		return def_val;
	}
	return (const char *)cs->csv_buf[index];
}
/* 
 *
 *************************************************************************
 *函数名:csv_get_var_num
 *功能	:获取csv结构中变量的总数
 *输入	:  
		IN CSV_T 	*csv, 		//之前使用csv_create得到的指针
 *返回值: 	负值表示出错,非负值表示csv结构中变量的总数
 *修改日志:
 *************************************************************************
 */
int csv_get_var_num(CSV_T 	*csv)
{
	CSV_ST* cs=NULL;
	cs = (CSV_ST*)csv;
	if((cs==NULL)||((cs->magic)!=MAGIC))
	{
		return -CSV_PARA_ERR;
	}
	return cs->atom_num;
}
/*
************************************************************************
*函数名	:csv_get_string
*功能	:得到csv结构的字符串描述
*输入	:  
		CSV_T	*csv,          之前使用csv_create得到的指针
*输出	: 描述csv结构的字符串指针
*修改日志:
*************************************************************************
*/
const char * csv_get_string( CSV_T *csv)
{
	CSV_ST* cs=NULL;
	int i;
	int len;

	cs = (CSV_ST*)csv;
	if((cs==NULL)||((cs->magic)!=MAGIC))
	{
		return NULL;
	}
	memset(cs->csv_str, 0 ,sizeof(cs->csv_str));
	len = 0;
	// 将单独的字段连成一个字符串
	for(i=0;i<cs->atom_num;i++)
	{
		sprintf(&cs->csv_str[len], "%s,", cs->csv_buf[i]);
		len = strlen(cs->csv_str);
	}
//	printf("%s!!!!!!!!!!!!",cs->csv_str);
	////将字符串末尾的字符(',') 换成'\r' 和'\n'
//	cs->csv_str[len]=0x0d;
//	cs->csv_str[len+1]=0x0a;
#if 1
	if((cs->csv_str[len-1]!=0x0a)&&(cs->csv_str[len-1]!=0x0d))
	{
		cs->csv_str[len-1]=0x0d;
		cs->csv_str[len]=0x0a;
	}
#endif
//	printf("%s!!!!!!!!!!!!",cs->csv_str);
	return cs->csv_str;
}

/*
************************************************************************
*函数名	:csv_parse_string
*功能	:  解析csv格式的字符串到结构
*输入	:  
		IN const char *  str 		//描述csv格式的字符串
*输出	: 
		IN CSV_T	*csv,        	//之前使用csv_create得到的指针
*返回值 :	0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
 int csv_parse_string( CSV_T	*csv, const char *  str )
{
	CSV_ST* cs=NULL;
	char* p = NULL;
	char* s = NULL;
	char* index = NULL;
	int offset =0;
	int i = 0;
	int len=0;
	cs = (CSV_ST*)csv;
	
	if((cs==NULL)||((cs->magic)!=MAGIC))
	{
		return -CSV_PARA_ERR;
	}
	if(strlen(str)>(MAX_ATOM_LEN*MAX_ATOM_NUM+1))
	{
		return -CSV_NO_MEM;
	}
////清空接收缓冲区
	memset(cs->csv_str, 0 , sizeof(cs->csv_str));
	memset(cs->csv_buf, 0 , sizeof(cs->csv_buf));
	cs->atom_num = 0;
	index = cs->csv_str;
////在前面添加"," 作为标记
	if(*str!=',')
	{
		cs->csv_str[0] = ',';
		memcpy(&cs->csv_str[1], str, strlen(str));
	}
	else
	{
		memcpy(cs->csv_str, str, strlen(str));
	}

//	printf("%s\n",cs->csv_str);
	len = strlen(cs->csv_str);
	for(i=0;i<MAX_ATOM_NUM;i++)
	{
		p = strchr(index, ',');
		if(p!=NULL)
		{
			index = p + 1;
			////根据两个逗号的间隔获得字段长度
			s = strchr(index, ',');
			if(s!=NULL)
			{
				offset =(int)(s-index);
			}
			else				// 到了数据包的结尾
			{
				offset = (int)(cs->csv_str + len - index);
			}
////字段 不为空
			if(offset>=1)
			{
				memcpy(cs->csv_buf[cs->atom_num], index, offset);
//				printf("%s\n",cs->csv_buf[cs->atom_num]);
			}
////字段为空
			else
			{
				; 	//暂时不做处理
			}
			cs->atom_num++;
		}
		else
		{
			if(cs->atom_num==0)	//没有找到任何符合格式的字段
			{
				return -CSV_PARSER_ERR;
			}
			break;
		}
	}
	for(i=0;i<cs->atom_num;i++)
	{
//		printf("%d\n",strlen(cs->csv_buf[i]));
		csv_rm_unused_space(cs->csv_buf[i]);	//删除字段前后没用的空格
//		printf("%s",cs->csv_buf[i]);
	}
	return CSV_SUCCESS;
}
 /*
************************************************************************
*函数名	:csv_destroy
*功能	:  销毁一个已经使用过的csv结构
*输入	:  
			CSV_T	*csv,          之前使用csv_create得到的指针
*输出	:  无
*修改日志:
*************************************************************************
*/
void csv_destroy(CSV_T *csv)
{
	CSV_ST* cs=NULL;
	cs = (CSV_ST*)csv;

	if((cs!=NULL)&&((cs->magic)==MAGIC))
	{
		cs->magic=0;	
		free(cs);
	}
}
/*
************************************************************************
*函数名	:csv_get_error_str
*功能	:  获取错误码的字符串描述
*输入	:  
		int errno, 接口返回的错误代码的绝对值
*返回值	:     错误描述字符串指针
*修改日志:
*************************************************************************
*/
const char *csv_get_error_str(int errno)
{
	switch(errno)
	{
		case CSV_NO_MEM:
		return "not enough memory";
		break;
		
		case CSV_PARA_ERR:
		return "parameters error";
		break;

		case CSV_PARSER_ERR:
		return "can not parser string";
		break;
		
		case CSV_OPEN_FILE_ERR:
		return "can not open file";
		break;

		case CSV_LOCK_FILE_ERR:
		return "can not lock file";
		break;
		
		case CSV_NO_RECORD:
		return "can not find records in file";
		break;
		
		default:
		return "unknow errno";
		break;	
	};
}
#if 1
////added by lsk 2006 -11-7
//强制加锁文件  
//输入 fd 文件控制字 ,cmd 命令, wait 等待标志
//返回0 成功 -1 失败
// cmd = F_RDLCK 读禁止 ； F_WRLCK 写禁止; F_UNLCK 解除锁定
// wait = 0 无法锁定则立即返回， =1 等待锁定
int force_lockfile(int fd, short int cmd, int wait)
{
//	int ret;
	struct flock tp;
	if(fd<0)
		return -1;
	memset(&tp , 0, sizeof(tp));		/// 必须清零
//	printf("cmd = %d , wait = %d, fd = %d \n", cmd, wait, fd);
	tp.l_type = cmd;
	tp.l_whence = SEEK_SET;
	tp.l_len = 0; 
//	tp.l_start = 0;
	tp.l_pid = getpid();
//	printf("wait = %d pid = %d \n", wait, tp.l_pid);
	if(wait ==0)
	{
		return fcntl(fd , F_SETLK, &tp);
	}
	if(wait ==1)
	{
		return fcntl(fd , F_SETLKW, &tp);
	}
	
	return fcntl(fd , F_SETLKW, &tp);
}
/*
************************************************************************
*函数名	:csvfile_lock
*功能	:打开csv文件，并加锁
*输入	: 	 IN const char *filename;	//csv结构文件
*返回值	:成功返回文件控制字，负值表示失败
*修改日志:
*************************************************************************
*/
static __inline__ int csvfile_lock( const char *filename)
{
	int fd = -1;
	fd=open(filename, O_RDONLY);
	if(fd<0)
	{
		return -1;
	}
	if(lock_file(fd, 1)<0)
	{
		perror("lock file error");
		close(fd);
		return -1;
	}
	return fd;
}
/*
************************************************************************
*函数名	:csvfile_open
*功能	:打开csv文件
*输入	: 	 IN const char *filename;	//csv结构文件
			 IN const char* mode // 打开文件的方式
*返回值	:成功返回文件控制字，负值表示失败
*修改日志:
*************************************************************************
*/
static __inline__ FILE* csvfile_open( const char *filename, const char* mode )
{
	FILE *fp=NULL;
	if(filename==NULL)
	{
		return NULL;
	}
	fp = fopen(filename,mode);
	return fp; 
}

/*
************************************************************************
*函数名	:csvfile_close
*功能	:关闭csv文件，
*输入	: IN const char *filename;	//csv结构文件
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
static __inline__ int csvfile_close( FILE * csv_fp)
{
//	int fd = -1;
	if(csv_fp==NULL)
	{
		return -CSV_PARA_ERR;
	}
//	fd = fileno(csv_fp);
	fclose(csv_fp);
	return 0;
}

/*
************************************************************************
*函数名	:csvfile_get_total_records
*功能	:获取一个文件中的csv记录总数
*输入	:  	 IN const char *filename;	//csv结构文件
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
int csvfile_get_total_records(const char *filename)
{
	FILE* fp=NULL;
	char s[1024];
	int index;
	int fd;
	index=0;
	if(filename==NULL)
	return -CSV_PARA_ERR;
	fd = csvfile_lock(filename);
	if(fd<0)
	return -CSV_LOCK_FILE_ERR;

	fp = csvfile_open(filename, "r+");
	if(fp==NULL)
	return -CSV_OPEN_FILE_ERR;
	memset(s, 0 ,sizeof(s));
	while(fgets(s, sizeof(s),fp)!=NULL)
	{
		index++;
	}
	fclose(fp);
	unlock_file(fd);
	close(fd);
	return index;
}
/*
************************************************************************
*函数名	:csvfile_get_record
*功能	:从csv文件中获取指定记录号的结构信息
*输入	:
	 IN const char *filename;	//csv结构文件
	 IN int record_no;		//csv结构中的记录号(正值表示按正序访问,负值表示按倒序访问)
*输出 	:OUT CSV_T *csv;		//指定记录号的记录信息
*返回值	:0表示成功负值表示失败
*修改日志:
*注:目前只完成了从末尾追加记录的功能
*************************************************************************
*/
int csvfile_get_record(const char *filename , int record_no, CSV_T *csv)
{
	FILE* fp=NULL;
	CSV_ST* cs=NULL;
	int ret, index;
	int i, fd;
	char buf[1024];

	cs = (CSV_ST*)csv;
	if((cs==NULL)||((cs->magic)!=MAGIC))
	{
		return -CSV_PARA_ERR;
	}
	if((record_no==0)||(filename==NULL))
	{
		return -CSV_PARA_ERR;
	}
	
	ret = csvfile_get_total_records(filename);
	if(ret <=0)
		return ret;

	index = csv_get_line_num(ret , record_no);
	
	
	
	fd = csvfile_lock(filename);
	if(fd<0)
	{
		return -CSV_LOCK_FILE_ERR;
	}
	fp = csvfile_open(filename, "r+");
	if(fp==NULL)
	{
		return -CSV_OPEN_FILE_ERR;
	}
	for(i=0;i<index;i++)
	{
		memset(buf,0,sizeof(buf));
		fgets(buf, sizeof(buf),fp);
	}
	fclose(fp);
	unlock_file(fd);
	close(fd);
	csv_rm_enter(buf);
	ret = csv_parse_string(cs, (const char *) buf);
	return ret;
}
/*
************************************************************************
*函数名	:csvfile_set_sub
*功能	:将csv结构写入csv文件中
*输入	:
	 IN int cmd;		//命令字
	 IN const char *filename;	//csv结构文件
	 IN int record_no;		//csv结构中的记录号(正序)
* 	 IN CSV_T *csv;			//指定记录号的记录信息
*返回值	:0表示成功负值表示失败
*修改日志:
注意: 需要共用一个临时文件，采取互斥体保护
*************************************************************************
*/
static __inline__ int csvfile_set_sub(int cmd, const char *filename, int record_no, CSV_ST *csv)
{
	FILE* fp = NULL;
	FILE *fp_temp = NULL;
	CSV_ST* cs = NULL;
	const char *p_str = NULL;
	int i;
	int fd = -1;
	int fd_temp = -1;
	char temp_file[]="/tmp/csvtemp-XXXXXX";
	char buf[1024];
//	record_no=1;//test !!
	if(filename==NULL)
	return -CSV_PARA_ERR;
	cs = csv;
	p_str = csv_get_string(cs);
	if(p_str==NULL)
	{
		return -CSV_PARA_ERR;
	}
//	printf("%s!!!!!!!!!!!!!!!!!!",p_str); // test
	memset(buf, 0, sizeof(buf));
	fd = csvfile_lock(filename);
	if(fd<0)
	{
		return -CSV_LOCK_FILE_ERR;
	}
/// 只需要追加的情况处理比较简单
	if(cmd==INSERT_TAIL)
	{
		fp = csvfile_open(filename, "a+");
		if(fp==NULL)
		{
			unlock_file(fd);
			close(fd);
			return -CSV_OPEN_FILE_ERR;
		}
		fprintf(fp,"%s",p_str);
		fclose(fp);
		unlock_file(fd);
		close(fd);
		return CSV_SUCCESS;
	}
	fp = csvfile_open(filename, "r+");
	if(fp==NULL)
	{
		unlock_file(fd);
		close(fd);
		return -CSV_OPEN_FILE_ERR;
	}
	////建立并打开临时文件
	fd_temp = mkstemp(temp_file);
	if(fd_temp<0)
	{
		fclose(fp);
		unlock_file(fd);
		close(fd);
		return -CSV_LOCK_FILE_ERR;
	}
	fp_temp = fdopen(fd_temp, "w+");	//转换为文件指针
	if(fp_temp==NULL)
	{
		close(fd_temp);
		fclose(fp);
		unlock_file(fd);
		close(fd);
		return -CSV_OPEN_FILE_ERR;
	}

	switch(cmd)
	{
		case INSERT_HEAD:		// 从文件头插入记录信息
		fprintf(fp_temp,"%s",p_str);
		while((i=fgetc(fp))!=EOF)
		{
			fprintf(fp_temp,"%c",i);
		}
		break;
			
		case INSERT_TAIL:		//从文件尾部插入记录信息
		while((i=fgetc(fp))!=EOF)
		{
			fprintf(fp_temp,"%c",i);
		}
		fprintf(fp_temp,"%s",p_str);
		break;
//中间插入新的记录信息，原来的记录存放到新信息下面
		case INSERT_MID:			
		i=0;
		while(fgets(buf, sizeof(buf),fp)!=NULL)
		{
			i++;
			if(i==record_no)		//要求插入的行号
			{
				fprintf(fp_temp,"%s", p_str);	
				fprintf(fp_temp, "%s",buf);		// 原来的记录向下移
			}
			else
			{
				fprintf(fp_temp, "%s",buf);
			}
			memset(buf, 0, sizeof(buf));
		}
		break;

		case CHANGE_MID:			//更改中间的记录信息
		i=0;
		while(fgets(buf, sizeof(buf),fp)!=NULL)
		{
			i++;
			if(i==record_no)			//要求更改的行号
			{
				fprintf(fp_temp,"%s", p_str);	
			}
			else
			{
				fprintf(fp_temp, "%s",buf);
			}
			memset(buf, 0, sizeof(buf));
		}
		break;

		default:
		fclose(fp);
		unlock_file(fd);
		close(fd);
		fclose(fp_temp);
		close(fd_temp);
		sprintf(buf, "rm %s \n", temp_file);	//删除临时文件
		system(buf);
		return -CSV_PARA_ERR;
		break;
	}
	fflush(fp_temp);
	fclose(fp);
	fclose(fp_temp);
	close(fd_temp);
	
////将临时文件中的数据写回文件中
	fp_temp = fopen(temp_file, "r");
	fp = csvfile_open(filename, "w+");
	if((fp==NULL)||(fp_temp==NULL))
	{
		fclose(fp);
		unlock_file(fd);
		close(fd);
		fclose(fp_temp);
		close(fd_temp);
		sprintf(buf, "rm %s \n", temp_file);	//删除临时文件
		system(buf);
		return -CSV_OPEN_FILE_ERR;
	}
	while((i=fgetc(fp_temp))!=EOF)
	{
		fprintf(fp,"%c",i);
	}
	fflush(fp);
	fclose(fp);
	unlock_file(fd);
	close(fd);
	fclose(fp_temp);
	close(fd_temp);
	sprintf(buf, "rm %s \n", temp_file);	//删除临时文件
	system(buf);
	return CSV_SUCCESS;
}
/*
************************************************************************
*函数名	:csvfile_set_record
*功能	:将csv结构写入csv文件中
*输入	:  
	 IN const char *filename;	//csv结构文件
	 IN int record_no;		//csv结构中的记录号(正值表示按正序访问,负值表示按倒序访问)
	 				//正序时如果超出最大序号则表示从文件尾部插入
					//倒序时如果超出范围则表示从首部插入
* 	 IN CSV_T *csv;			//指定记录号的记录信息
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
int csvfile_set_record(const char *filename, int record_no, CSV_T *csv)
{
	CSV_ST* cs=NULL;
//	char *p_str=NULL;
	int line_num;
//	int ret;
	if(filename==NULL)
	return -CSV_PARA_ERR;	
	cs = (CSV_ST*)csv;
	if((cs==NULL)||((cs->magic)!=MAGIC))
	return -CSV_PARA_ERR;

//	if(record_no==0)
//	return -CSV_PARA_ERR;
//	printf("%s\n", filename);
////获取文件中记录数据的数目
	line_num = csvfile_get_total_records(filename);
	if(line_num <0)
	{
		return line_num;
	}
	if((line_num==0)||(record_no==0))
		return(csvfile_set_sub(INSERT_HEAD, filename, 1, cs));
		
////根据记录的行号将信息记录到文件中	
	if(record_no>0)
	{
		if(record_no<=line_num)
		{
			return(csvfile_set_sub(CHANGE_MID, filename, record_no, cs));
		}
		else
		{
			return(csvfile_set_sub(INSERT_TAIL, filename, line_num+1, cs));
		}
	}
	if(record_no<0)
	{
		if(-record_no>line_num)
		{
			return(csvfile_set_sub(INSERT_HEAD, filename, 1, cs));
		}
		else 
		{
			return(csvfile_set_sub(CHANGE_MID, filename, line_num+record_no+1, cs));
		}
	}
	return -CSV_PARA_ERR;
}

/*
************************************************************************
*函数名	:csvfile_insert_record
*功能	:将csv结构插入csv文件中
*输入	:  
	 IN const char *filename;	//csv结构文件
	 IN int record_no;			//csv结构中的记录号(正值表示按正序访问,负值表示按倒序访问)
	 						//正序时如果超出最大序号则表示从文件尾部插入
							//倒序时如果超出范围则表示从首部插入
* 	 IN CSV_T *csv;			//指定记录号的记录信息
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
int csvfile_insert_record(const char *filename , int record_no, CSV_T *csv)
{
	CSV_ST* cs=NULL;
//	char *p_str=NULL;
	int line_num;
	int ret;

	if(filename==NULL)
	return -CSV_PARA_ERR;
	cs = (CSV_ST*)csv;
	if((cs==NULL)||((cs->magic)!=MAGIC))
	{
		return -CSV_PARA_ERR;
	}
	if((record_no==1)||(record_no==0))
	{
		return csvfile_set_sub(INSERT_HEAD, filename, 1, cs);
	}
	if(record_no==-1)
	{
//		printf("set tail \n");
//		return csvfile_set_sub(INSERT_HEAD, filename, 1, cs);
		return csvfile_set_sub(INSERT_TAIL, filename, -1, cs);//
	}
//	printf("%s\n", filename);
////获取文件中记录数据的数目
	line_num = csvfile_get_total_records(filename);
	if(line_num <0)
	{
		return line_num;
	}
	if(line_num==0)
		return(csvfile_set_sub(INSERT_HEAD, filename, 1, cs));
////根据记录的行号将信息记录到文件中	
//	printf("%s\n", filename);
	if(record_no>0)
	{
		if(record_no<=line_num)
		{
			ret = csvfile_set_sub(INSERT_MID, filename, record_no, cs);
//			printf("ERROR INSERT_MID\n");
			return ret;
		}
		else
		{
			ret = csvfile_set_sub(INSERT_TAIL, filename, line_num+1, cs);
			return ret;
		}
	}
	if(record_no<0)
	{
		if(-record_no>line_num)
		{
			ret = csvfile_set_sub(INSERT_HEAD, filename, 1, cs);
			return ret;
		}
		else 
		{
			ret = csvfile_set_sub(INSERT_MID, filename, line_num+record_no+1, cs);
			return ret;
		}
	}
	return -CSV_PARA_ERR;

}
/*
************************************************************************
*函数名	:csvfile_rm_record_sub
*功能	:将文件中的一条记录删除
*输入	:
	 IN const char *filename;	//csv结构文件
	 IN int record_no;		//csv结构中的记录号(正序)
*返回值	:0表示成功负值表示失败
*修改日志:
注意: 需要共用一个临时文件，采取互斥体保护
*************************************************************************
*/
static __inline__ int csvfile_rm_record_sub(const char *filename, int record_no)
{
	FILE* fp = NULL;
	FILE *fp_temp = NULL;
	int i, line=record_no;	
	int fd = -1;
	int fd_temp = -1;
	char temp_file[]="/tmp/csvtemp-XXXXXX";
	char buf[1024];

	if((filename==NULL))
	return -CSV_PARA_ERR;
	if(line==0)
		line=1;
	memset(buf, 0, sizeof(buf));
	fd = csvfile_lock(filename);
	if(fd<0)
	{
		return -CSV_LOCK_FILE_ERR;
	}

	fp = csvfile_open(filename, "r+");
	if(fp==NULL)
	{
		unlock_file(fd);
		close(fd);
		return -CSV_OPEN_FILE_ERR;
	}
	////建立并打开临时文件
	fd_temp = mkstemp(temp_file);
	if(fd_temp<0)
	{
		fclose(fp);
		unlock_file(fd);
		close(fd);
		return -CSV_LOCK_FILE_ERR;
	}
	fp_temp = fdopen(fd_temp, "w+");	//转换为文件指针
	if(fp_temp==NULL)
	{
		close(fd_temp);
		fclose(fp);
		unlock_file(fd);
		close(fd);
		return -CSV_OPEN_FILE_ERR;
	}

	i=0;
	while(fgets(buf, sizeof(buf),fp)!=NULL)
	{
		i++;
		if(i==line)		//要求删除的行号
		{
			continue;
//			fprintf(fp_temp,"%s", p_str);	
//			fprintf(fp_temp, "%s",buf);		// 原来的记录向下移
		}
		else
		{
			fprintf(fp_temp, "%s",buf);
		}
		memset(buf, 0, sizeof(buf));
	}
	fflush(fp_temp);
	fclose(fp);
	fclose(fp_temp);
	close(fd_temp);
	
////将临时文件中的数据写回文件中
	fp_temp = fopen(temp_file, "r");
	fp = csvfile_open(filename, "w+");
	if((fp==NULL)||(fp_temp==NULL))
	{
		fclose(fp);
		unlock_file(fd);
		close(fd);
		fclose(fp_temp);
		close(fd_temp);
		sprintf(buf, "rm %s \n", temp_file);	//删除临时文件
		system(buf);
		return -CSV_OPEN_FILE_ERR;
	}
	while((i=fgetc(fp_temp))!=EOF)
	{
		fprintf(fp,"%c",i);
	}
	
	fflush(fp);
	fclose(fp);
	unlock_file(fd);
	close(fd);
	fclose(fp_temp);
	close(fd_temp);
	sprintf(buf, "rm %s \n", temp_file);	//删除临时文件
	system(buf);
	return CSV_SUCCESS;
}
/*
************************************************************************
*函数名	:csvfile_rm_record
*功能	:将filename文件中的第record_no条信息删除掉
*输入	:  
	 IN const char *filename;	//csv结构文件
	 IN int record_no;		//csv结构中的记录号(正值表示按正序访问,负值表示按倒序访问)
	 				//正序时如果超出最大序号则表示从文件尾部删除
					//倒序时如果超出范围则表示从首部删除
*返回值	:0表示成功，负值表示失败
*修改日志:
*************************************************************************
*/
int csvfile_rm_record(const char *filename, int record_no)
{
	int line_num;
	int line=1;
	if(filename==NULL)
	return -CSV_PARA_ERR;

//	if(record_no==0)
//	return -CSV_PARA_ERR;
//	printf("%s\n", filename);

////获取文件中记录数据的数目
	line_num = csvfile_get_total_records(filename);
	if(line_num <=0)
	{
		return line_num;
	}
		
////删除记录	
	if(record_no>0)
	{
		if(record_no<=line_num)
		line = record_no;
		else
		line = line_num;
	}
	if(record_no<0)
	{
		if(-record_no>line_num)
		line = 1;
		else 
		line =  line_num+record_no+1;
	}
	return(csvfile_rm_record_sub(filename,line));
}
#if 0
/*
************************************************************************
*函数名	:csvfile_insert_record_head
*功能	:将csv结构插入csv文件头部原来的记录信息下移
*输入	:  
	 IN const char *filename;	//csv结构文件
* 	 IN CSV_ST *csv;			//指定记录号的记录信息
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
static __inline__ int csvfile_insert_record_head(const char *filename, CSV_ST *csv)
{
////根据记录的行号将信息记录到文件中	
	return csvfile_set_sub(INSERT_HEAD, filename, 1, csv);
}
/*
************************************************************************
*函数名	:csvfile_insert_record_tail
*功能	:将csv结构插入csv文件尾部原来的记录信息不变
*输入	:  
	 IN const char *filename;	//csv结构文件
* 	 IN CSV_ST *csv;			//指定记录号的记录信息
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************
*/
static __inline__ int csvfile_insert_record_tail(const char *filename, CSV_ST *csv)
{
////根据记录的行号将信息记录到文件中	
	return csvfile_set_sub(INSERT_TAIL, filename, -1, csv);
}
#endif
#endif

