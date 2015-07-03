
/*-------------------------------------------------------------------------*/
/**
   @file    iniparser.c
   @author  N. Devillard
   @date    Mar 2000
   @version $Revision: 1.1.1.1 $
   @brief   Parser for ini files.
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: iniparser.c,v 1.1.1.1 2005/02/19 03:36:51 shixin Exp $
    $Author: shixin $
    $Date: 2005/02/19 03:36:51 $
    $Revision: 1.1.1.1 $
*/

#if 1 //ndef  _WIN32
/*---------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/
#define  _CRT_SECURE_NO_WARNINGS
#define  _CRT_SECURE_NO_DEPRECATE
#include "iniparser.h"
#include "strlib.h"
#include <gtlog.h>
//#include <commonlib.h>
//#include "file_def.h"

#define ASCIILINESZ         1024
#define INI_INVALID_KEY     ((char*)-1)

/*---------------------------------------------------------------------------
                        Private to this module
 ---------------------------------------------------------------------------*/

/* Private: add an entry to the dictionary */
static void iniparser_add_entry(
    dictionary * d,
    char * sec,
    char * key,
    char * val)
{
    char longkey[2*ASCIILINESZ+1];

    /* Make a key as section:keyword */
    if (key!=NULL) {
        sprintf(longkey, "%s:%s", sec, key);
    } else {
        strcpy(longkey, sec);
    }

    /* Add (key,val) to dictionary */
    dictionary_set(d, longkey, val);
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Get number of sections in a dictionary
  @param    d   Dictionary to examine
  @return   int Number of sections found in dictionary

  This function returns the number of sections found in a dictionary.
  The test to recognize sections is done on the string stored in the
  dictionary: a section name is given as "section" whereas a key is
  stored as "section:key", thus the test looks for entries that do not
  contain a colon.

  This clearly fails in the case a section name contains a colon, but
  this should simply be avoided.

  This function returns -1 in case of error.
 */
/*--------------------------------------------------------------------------*/

int iniparser_getnsec(dictionary * d)
{
    int i ;
    int nsec ;

    if (d==NULL) return -1 ;
    nsec=0 ;
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        if (strchr(d->key[i], ':')==NULL) {
            nsec ++ ;
        }
    }
    return nsec ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Get name for section n in a dictionary.
  @param    d   Dictionary to examine
  @param    n   Section number (from 0 to nsec-1).
  @return   Pointer to char string

  This function locates the n-th section in a dictionary and returns
  its name as a pointer to a string statically allocated inside the
  dictionary. Do not free or modify the returned string!

  This function returns NULL in case of error.
 */
/*--------------------------------------------------------------------------*/

char * iniparser_getsecname(dictionary * d, int n)
{
    int i ;
    int foundsec ;

    if (d==NULL || n<0) return NULL ;
    foundsec=0 ;
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        if (strchr(d->key[i], ':')==NULL) {
            foundsec++ ;
            if (foundsec>n)
                break ;
        }
    }
    if (foundsec<=n) {
        return NULL ;
    }
    return d->key[i] ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a dictionary to an opened file pointer.
  @param    d   Dictionary to dump.
  @param    f   Opened file pointer to dump to.
  @return   void

  This function prints out the contents of a dictionary, one element by
  line, onto the provided file pointer. It is OK to specify @c stderr
  or @c stdout as output files. This function is meant for debugging
  purposes mostly.
 */
/*--------------------------------------------------------------------------*/
void iniparser_dump(dictionary * d, FILE * f)
{
    int     i ;

    if (d==NULL || f==NULL) return ;
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        if (d->val[i]!=NULL) {
            fprintf(f, "[%s]=[%s]\n", d->key[i], d->val[i]);
        } else {
            fprintf(f, "[%s]=UNDEF\n", d->key[i]);
        }
    }
    return ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Save a dictionary to a loadable ini file
  @param    d   Dictionary to dump
  @param    f   Opened file pointer to dump to
  @return   void

  This function dumps a given dictionary into a loadable ini file.
  It is Ok to specify @c stderr or @c stdout as output files.
 */
/*--------------------------------------------------------------------------*/

void iniparser_dump_ini(dictionary * d, FILE * f)
{
    int     i, j ;
    char    keym[ASCIILINESZ+1];
    int     nsec ;
    char *  secname ;
    int     seclen ;

    if (d==NULL || f==NULL) return ;

    nsec = iniparser_getnsec(d);
    if (nsec<1) {
        /* No section in file: dump all keys as they are */
        for (i=0 ; i<d->size ; i++) {
            if (d->key[i]==NULL)
                continue ;
            fprintf(f, "%s = %s\n", d->key[i], d->val[i]);
        }
        return ;
    }
    for (i=0 ; i<nsec ; i++) {
        secname = iniparser_getsecname(d, i) ;
        seclen  = (int)strlen(secname);
        fprintf(f, "\n[%s]\n", secname);
        sprintf(keym, "%s:", secname);
        for (j=0 ; j<d->size ; j++) {
            if (d->key[j]==NULL)
                continue ;
            if (!strncmp(d->key[j], keym, seclen+1)) {
                fprintf(f,
                        "%-30s = %s\n",
                        d->key[j]+seclen+1,
                        d->val[j] ? d->val[j] : "");
            }
        }
    }
    fprintf(f, "\n");
    return ;
}




/*-------------------------------------------------------------------------*/
/**
  @brief	Get the string associated to a key, return NULL if not found
  @param    d   Dictionary to search
  @param    key Key string to look for
  @return   pointer to statically allocated character string, or NULL.

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  NULL is returned.
  The returned char pointer is pointing to a string allocated in
  the dictionary, do not free or modify it.

  This function is only provided for backwards compatibility with 
  previous versions of iniparser. It is recommended to use
  iniparser_getstring() instead.
 */
/*--------------------------------------------------------------------------*/
char * iniparser_getstr(dictionary * d, char * key)
{
    return iniparser_getstring(d, key, NULL);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key
  @param    d       Dictionary to search
  @param    key     Key string to look for
  @param    def     Default value to return if key not found.
  @return   pointer to statically allocated character string

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the pointer passed as 'def' is returned.
  The returned char pointer is pointing to a string allocated in
  the dictionary, do not free or modify it.
 */
/*--------------------------------------------------------------------------*/
char * iniparser_getstring(dictionary * d, char * key, char * def)
{
    char * lc_key ;
    char * sval ;

    if (d==NULL || key==NULL)
        return def ;
#ifndef _WIN32
    lc_key = strdup(strlwc(key));
#else
	
	lc_key = _strdup(strlwc(key));
#endif
	sval = dictionary_get(d, lc_key, def);
    free(lc_key);
    return sval ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to an int
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   integer

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.
 */
/*--------------------------------------------------------------------------*/
int iniparser_getint(dictionary * d, char * key, int notfound)
{
    char    *   str ;

    str = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (str==INI_INVALID_KEY) return notfound ;
    return atoi(str);
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to a double
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   double

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.
 */
/*--------------------------------------------------------------------------*/
double iniparser_getdouble(dictionary * d, char * key, double notfound)
{
    char    *   str ;

    str = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (str==INI_INVALID_KEY) return notfound ;
    return atof(str);
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to a boolean
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   integer

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.

  A true boolean is found if one of the following is matched:

  - A string starting with 'y'
  - A string starting with 'Y'
  - A string starting with 't'
  - A string starting with 'T'
  - A string starting with '1'

  A false boolean is found if one of the following is matched:

  - A string starting with 'n'
  - A string starting with 'N'
  - A string starting with 'f'
  - A string starting with 'F'
  - A string starting with '0'

  The notfound value returned if no boolean is identified, does not
  necessarily have to be 0 or 1.
 */
/*--------------------------------------------------------------------------*/
int iniparser_getboolean(dictionary * d, char * key, int notfound)
{
    char    *   c ;
    int         ret ;

    c = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (c==INI_INVALID_KEY) return notfound ;
    if (c[0]=='y' || c[0]=='Y' || c[0]=='1' || c[0]=='t' || c[0]=='T') {
        ret = 1 ;
    } else if (c[0]=='n' || c[0]=='N' || c[0]=='0' || c[0]=='f' || c[0]=='F') {
        ret = 0 ;
    } else {
        ret = notfound ;
    }
    return ret;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Finds out if a given entry exists in a dictionary
  @param    ini     Dictionary to search
  @param    entry   Name of the entry to look for
  @return   integer 1 if entry exists, 0 otherwise

  Finds out if a given entry exists in the dictionary. Since sections
  are stored as keys with NULL associated values, this is the only way
  of querying for the presence of sections in a dictionary.
 */
/*--------------------------------------------------------------------------*/

int iniparser_find_entry(
    dictionary  *   ini,
    char        *   entry
)
{
    int found=0 ;
    if (iniparser_getstring(ini, entry, INI_INVALID_KEY)!=INI_INVALID_KEY) {
        found = 1 ;
    }
    return found ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Set an entry in a dictionary.
  @param    ini     Dictionary to modify.
  @param    entry   Entry to modify (entry name)
  @param    val     New value to associate to the entry.
  @return   int 0 if Ok, -1 otherwise.

  If the given entry can be found in the dictionary, it is modified to
  contain the provided value. If it cannot be found, -1 is returned.
  It is Ok to set val to NULL.
 */
/*--------------------------------------------------------------------------*/

int iniparser_setstr(dictionary * ini, char * entry, char * val)
{
//shixin added
	char section[64];
	int 	i;
	for(i=0;i<sizeof(section);i++)
	{
		section[i]=entry[i];
		if(section[i]==':')
		{
			section[i]='\0';
			if(iniparser_find_entry(ini,section)==0)
				dictionary_set(ini, strlwc(section), val);		
			break;
		}
		else if (section[i]=='\0')
			break;
	}
//
    	dictionary_set(ini, strlwc(entry), val);
    return 0 ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Delete an entry in a dictionary
  @param    ini     Dictionary to modify
  @param    entry   Entry to delete (entry name)
  @return   void

  If the given entry can be found, it is deleted from the dictionary.
 */
/*--------------------------------------------------------------------------*/
void iniparser_unset(dictionary * ini, char * entry)
{
    dictionary_unset(ini, strlwc(entry));
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Parse an ini file and return an allocated dictionary object
  @param    ininame Name of the ini file to read.
  @return   Pointer to newly allocated dictionary

  This is the parser for ini files. This function is called, providing
  the name of the file to be read. It returns a dictionary object that
  should not be accessed directly, but through accessor functions
  instead.

  The returned dictionary must be freed using iniparser_freedict().
 */
/*--------------------------------------------------------------------------*/

dictionary * iniparser_load(char * ininame)
{
    dictionary  *   d ;
    char        lin[ASCIILINESZ+1];
    char        sec[ASCIILINESZ+1];
    char        key[ASCIILINESZ+1];
    char        val[ASCIILINESZ+1];
    char    *   where ;
    FILE    *   ini ;
    int         lineno ;

    if ((ini=fopen(ininame, "r"))==NULL) {
        return NULL ;
    }

    sec[0]=0;

    /*
     * Initialize a new dictionary entry
     */
    d = dictionary_new(0);
    lineno = 0 ;
    while (fgets(lin, ASCIILINESZ, ini)!=NULL) {
        lineno++ ;
        where = strskp(lin); /* Skip leading spaces */
        if (*where==';' || *where=='#' || *where==0)
            continue ; /* Comment lines */
        else {
            if (sscanf(where, "[%[^]]", sec)==1) {
                /* Valid section name */
                strcpy(sec, strlwc(sec));
                iniparser_add_entry(d, sec, NULL, NULL);
            } else if (sscanf (where, "%[^=] = \"%[^\"]\"", key, val) == 2
                   ||  sscanf (where, "%[^=] = '%[^\']'",   key, val) == 2
                   ||  sscanf (where, "%[^=] = %[^;#]",     key, val) == 2) {
                strcpy(key, strlwc(strcrop(key)));
                /*
                 * sscanf cannot handle "" or '' as empty value,
                 * this is done here
                 */
                if (!strcmp(val, "\"\"") || !strcmp(val, "''")) {
                    val[0] = (char)0;
                } else {
                    strcpy(val, strcrop(val));
                }
                iniparser_add_entry(d, sec, key, val);
            }
        }
    }
    fclose(ini);
    return d ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Free all memory associated to an ini dictionary
  @param    d Dictionary to free
  @return   void

  Free all memory associated to an ini dictionary.
  It is mandatory to call this function before the dictionary object
  gets out of the current context.
 */
/*--------------------------------------------------------------------------*/

void iniparser_freedict(dictionary * d)
{
    dictionary_del(d);
}

/* vim: set ts=4 et sw=4 tw=75 */












/////////////////////////////////////////////////////////////////////
///added by shixin

#include <errno.h>

//将ini结构存入指定文件
int save_inidict_file(char *filename,dictionary * ini,FILE**lockf)
{
	FILE *fp;
	//int lock;
	if((filename==NULL)||(ini==NULL))
		return -1;
	fp=fopen(filename,"w");
	if((fp!=NULL))
	{
		iniparser_dump_ini(ini,fp);	
#ifndef _WIN32
		fsync(fileno(fp));
#endif
		fclose(fp);
	}
	#ifndef _WIN32
	if(lockf!=NULL)
	{
		if(*lockf!=NULL)
		{
			unlock_file(fileno(*lockf));
			fsync(fileno(*lockf));
			fclose(*lockf);
			*lockf=NULL;
		
		}
	}
	#endif
	
	return 0;
}


int iniparser_setint(dictionary * d, char * key, int val)
{
//	int ret;
	char sbuf[30];
	if((d==NULL)||(key==NULL))
		return -1;
	sprintf(sbuf,"%d",val);
	return iniparser_setstr(d,key,sbuf);
}

int iniparser_sethex(dictionary *ini,char* section,int val)
{
	char pbuf[30];
	sprintf(pbuf,"0x%4x",val);
	return save2para(ini,section,pbuf);
}
#ifndef _WIN32
/*-------------------------------------------------------------------------*/
/**
  @brief    将指定变量的字符串存入配置文件中
  @param    filename:文件名
  	    	    section:"节名:变量名"
  	    	    vstr:变量的字符串形式的值  	   
  @return   0表示成功负值表示出错
**/
/*--------------------------------------------------------------------------*/
int save2para_file(char *filename,char *section,char *vstr)
{
	dictionary      *ini;	
	FILE *fp;
	char oldval[100],*old;
	int rc=-1;
	if((filename==NULL)||(section==NULL)||(vstr==NULL))
	{
		return -1;
	}
	ini=iniparser_load_lockfile(filename,1,&fp);
	if(ini==NULL)
		return -1;
	do{
		old=iniparser_getstring(ini,section,"未定义");
		if(old!=NULL)
		{
			sprintf(oldval,"%s",old);
		}
		else
			oldval[0]='\0';
		rc=iniparser_setstr(ini,section,vstr);
		if(rc<0)
			break;
		//fp=fopen(filename,"w+");
		//if(fp==NULL)
		//{
		//	rc=-1;
		//	break;
		//}
		gtloginfo("参数设置 %s:  %s -> %s\n",section,oldval,vstr);
		//iniparser_dump_ini(ini,fp);
		//fclose(fp);
		save_inidict_file(filename,ini,&fp);
		//fp=fopen(IPMAIN_PARA_FILE,"w");
		//iniparser_freedict(ini);

		
	}while(0);
	

	
	iniparser_freedict(ini);
	
	return rc;
}
#endif
/*-------------------------------------------------------------------------*/
/**
  @brief    将指定变量的字符串存入ini数据结构
  @param    ini:ini文件结构指针
  	    	    section:"节名:变量名"
  	    	    vstr:变量的字符串形式的值  	   
  @return   0表示成功负值表示出错
**/
/*--------------------------------------------------------------------------*/
int save2para(dictionary      *ini,char *section,char *vstr)
{
	char oldval[100],*old;
	int rc=-1;
	if((ini==NULL)||(section==NULL)||(vstr==NULL))
	{
		return -1;
	}
	do{
		old=iniparser_getstring(ini,section,"未定义");
		if(old!=NULL)
		{
			sprintf(oldval,"%s",old);
		}
		else
			oldval[0]='\0';
		rc=iniparser_setstr(ini,section,vstr);
		if(rc<0)
			break;
		gtloginfo("参数设置 %s:  %s -> %s\n",section,oldval,vstr);
		
	}while(0);
	
	return rc;
}

//查找ini文件中是否有指定的节,如果没有则创建
//0表示正常
int iniparser_find_creat_sec(char *filename,char*section)
{
	dictionary      *ini;
	int i,secs;
	char *secname;
	int findflag;
	FILE *fp;
	if((filename==NULL)||(section==NULL))
		return -1;
 	ini=iniparser_load(filename);
        if (ini==NULL) {
                printf("init_devinfo() cannot parse ini file file [%s]", filename);
                return -1 ;
        }			
	secs=iniparser_getnsec(ini);
	findflag=0;
	for(i=0;i<secs;i++)
	{
		secname=iniparser_getsecname(ini,i);
		if(memcmp(secname,section,strlen(section))==0)//寻找是否有叫
		{
			findflag=1;
			break;
		}
		
	}
	iniparser_freedict(ini);
	if(!findflag)
	{
		fp=fopen(filename,"a+");//追加方式打开文件
		if(fp==NULL)
		{
			printf("can't open file :%s\n",filename);
			return -1;
		}
		fprintf(fp,"\n\n[%s]\n",section);
		fclose(fp);
	}
	return 0;

}



//调出ini文件到数据结构并且锁住文件,返回时lockf填充的是文件指针
dictionary * iniparser_load_lockfile(char * ininame,int wait,FILE**lockf)
{
	FILE *fp=NULL;
	dictionary *ini;
#ifndef _WIN32
	int lock;
#endif
	if(ininame==NULL)
		return NULL;
	//if(lockf==NULL)
	//	return NULL;
	if(lockf!=NULL)
		*lockf=NULL;
#ifndef _WIN32
	fp=fopen(ininame,"r");
	if(fp==NULL)
	{
		return NULL;
	}
	lock=lock_file(fileno(fp),wait);
	fsync(fileno(fp));
	if(lock<0)
	{
		//gtloginfo("iniparser_load_lockfile lock=%d(%s)!!!!\n",lock,strerror(errno));
		fclose(fp);
		return NULL;
	}
	else
#endif
	{		
		ini=iniparser_load(ininame);
		if(ini==NULL)
		{
#ifndef _WIN32
			unlock_file(fileno(fp));
			fclose(fp);
#endif
		}
		else
			*lockf=fp;
		return ini;
	}
	
}

/******************************************************************
 * 函数名	ini_diff()
 * 功能:	比较两个ini文件是否相同
 * 输入:	oldfile,newfile,两个ini文件名
 * 返回值:  两个文件相等时返回0，不等时返回1,发生错误返回-1
 *
 * 如果两个文件不同则在终端和日志上记录信息:如
 * alarm:snap_pic_num 5->4				表示变量alarm:snap_pic_num原来是5，新值是4
 * alarm:snap_pic_interval NULL->500	表示变量alarm: snap_pic_interval原来没有，新值是500
 * port:telnet_port 23->NULL		    表示变量port: telnet_port原来是23，后来把这个变量删除了

 
******************************************************************/

int ini_diff(char *oldfile,char *newfile)
{
	dictionary *oldini, *newini;
	int i;
	char *key;
	int changeflag=0;
	int compare_len=0; //需要比较的字节数

	if((oldfile==NULL)||(newfile==NULL))
	{
		gtloginfo("ini_diff传进参数为空,返回\n");
		printf("ini_diff传进参数为空,返回\n");
		return -1;
	}

	//分别解析出ini，遇错返回
	oldini=iniparser_load(oldfile);
	if(oldini==NULL)
        {
             	printf("cannot parse ini file [%s]",oldfile);
		gtloginfo("解析%s失败退出\n",oldfile);	
             	return -1 ;
	}

	newini=iniparser_load(newfile);
	if(newini==NULL)
        {
             	printf("cannot parse ini file [%s]",newfile);
		gtloginfo("解析%s失败退出\n",newfile);	
		iniparser_freedict(oldini);
             	return -1 ;
	}

	//比较从oldfile到newfile
	for (i=0 ; i<oldini->size ; i++) 
	{
		if (oldini->key[i]==NULL)
            		continue ;
        	if (strchr(oldini->key[i], ':')==NULL)//没有:,为节名 
        		continue;
    
        //看目标文件是否有
       	 key=iniparser_getstring(newini,oldini->key[i],NULL);
		if(key==NULL)//没有
        	{
        		changeflag++;
        		gtloginfo("%-30s %s->NULL\n",oldini->key[i],oldini->val[i]);
				printf("%-30s %s->NULL\n",oldini->key[i],oldini->val[i]);
		}
		else
		{
			compare_len= (strlen(key)>strlen(oldini->val[i])) ? strlen(key) : strlen(oldini->val[i]);
			//if(strncmp(key,oldini->val[i],strlen(oldini->val[i]))!=0)//有但不相同
			if(strncmp(key,oldini->val[i],compare_len)!=0)//有但不相同
			{
					changeflag++;
						printf("%-30s %s->%s\n",oldini->key[i],oldini->val[i],key);//changed by shixin oldini->val[i]<->key
					gtloginfo("%-30s %s->%s\n",oldini->key[i],oldini->val[i],key);
			}
		}
	}

	//比较从newfile到oldfile
	for (i=0 ; i<newini->size ; i++) 
	{
		if (newini->key[i]==NULL)
            		continue ;
        	if (strchr(newini->key[i], ':')==NULL)//没有:,为节名 
        		continue;
    
        //看目标文件是否有
        	key=iniparser_getstring(oldini,newini->key[i],NULL);
		if(key==NULL)//没有
        	{
        		changeflag++;
        		gtloginfo("%-30s NULL->%s\n",newini->key[i],newini->val[i]);
			printf("%-30s NULL->%s\n",newini->key[i],newini->val[i]);
		}
	}

	iniparser_freedict(newini);
	iniparser_freedict(oldini);
	
	if(changeflag!=0)
		return 1;
	else
		return 0;
}



/******************************************************************
 * 函数名	ini_set_file()
 * 功能:	将source文件的每个节读出，与target文件相应节读出内容比较，若不同则改写target,若target不存在该节则创建,均记日志
 * 输入:	source:源文件名
 *			target,待改写的ini文件名
 * 返回值: 0表示成功,负值表示失败
 * 
 ******************************************************************/
int ini_set_file(char *source,char *target)
{
        dictionary *srcini,*tgtini;
        FILE *filetgt=NULL;
        int i;
        char *key=NULL;
        int changeflag=0;
        int compare_len=0; //需要比较的字节长度

        if((source==NULL)||(target==NULL))
                {
                        printf("传来文件名为NULL,退出ini_set_file\n");
                        gtloginfo("传来文件名为NULL,退出ini_set_file\n");
                        return -1;
                }

        srcini=iniparser_load(source);
        if (srcini==NULL)
        {
             printf("cannot parse ini file [%s]",source);
                         gtloginfo("解析%s失败退出\n",source);
             return(-1);
                }

        tgtini=iniparser_load_lockfile(target,1,&filetgt);
        for (i=0 ; i<srcini->size ; i++) //对于源文件的每个变量
        {
        if (srcini->key[i]==NULL)
            continue ;
        if (strchr(srcini->key[i], ':')==NULL)//没有:,为节名 
                continue;

        //看目标文件是否有，如果没有则创建
        key=iniparser_getstring(tgtini,srcini->key[i],NULL);
                if(key==NULL)//没有，创建
                {
                        changeflag++;
                        iniparser_setstr(tgtini,srcini->key[i],srcini->val[i]);
                           printf("<%s> %s NULL->%s\n",target,srcini->key[i],srcini->val[i]);
                        gtloginfo("<%s> %s NULL->%s\n",target,srcini->key[i],srcini->val[i]);

				}
        else //有
                {

                                //if(strncmp(key,srcini->val[i],strlen(srcini->val[i]))!=0)//有但不相同
                                compare_len=(strlen(key)>strlen(srcini->val[i])) ? strlen(key) : strlen(srcini->val[i]);

                                if(strncmp(key,srcini->val[i],compare_len)!=0)//有但不相同
                                {
                                        changeflag++;
                                        gtloginfo("<%s> %s %s->%s\n",target,srcini->key[i],key,srcini->val[i]);
                                        printf   ("<%s> %s %s->%s\n",target,srcini->key[i],key,srcini->val[i]);
                                        iniparser_setstr(tgtini,srcini->key[i],srcini->val[i]);
                                }
                }

        }

        if(changeflag!=0)
                gtloginfo("%s被设置(ini file set!)\n",target);
        iniparser_freedict(srcini);
        //save_inidict_file(IPMAIN_PARA_FILE,tgtini,&filetgt);
        save_inidict_file(target,tgtini,&filetgt);
        iniparser_freedict(tgtini);
        return 0;
}
#endif


