
/*-------------------------------------------------------------------------*/
/**
   @file    iniparser.h
   @author  N. Devillard
   @date    Mar 2000
   @version $Revision: 1.1.1.1 $
   @brief   Parser for ini files.
*/
/*--------------------------------------------------------------------------*/

#ifndef _INIPARSER_H_
#define _INIPARSER_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "dictionary.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
	//windows 使用
	#define EXPORT_DLL __declspec(dllexport)
#else
	//linux 使用
	#define EXPORT_DLL
#endif


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
 /**********************************************************************************************
 * 函数名	:iniparser_getnsec()
 * 功能	:获取ini文件中的总节数
 * 输入	:d:描述ini文件的结构
 * 返回值	:正值表示ini结构中的总节数，负值表示出错
 **********************************************************************************************/
EXPORT_DLL int iniparser_getnsec(dictionary * d);


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
 /**********************************************************************************************
 * 函数名	:iniparser_getsecname()
 * 功能	:获取ini文件中的第n个节的节名
 * 输入	:d:描述ini文件的结构
 *			 n:要取名的节序号
 * 返回值	:指定序号的节的名字，NULL表示没找到(参数错)
 **********************************************************************************************/
EXPORT_DLL char * iniparser_getsecname(dictionary * d, int n);


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
/**********************************************************************************************
 * 函数名	:iniparser_dump_ini()
 * 功能	:将ini结构输出到一个已经打开的文件流(ini格式)
 * 输入	:d:描述ini文件的结构
 * 输出	:f:已经打开的文件流，ini结构将输出到这个流中
 * 返回值	:无
 **********************************************************************************************/
EXPORT_DLL void iniparser_dump_ini(dictionary * d, FILE * f);

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
/**********************************************************************************************
 * 函数名	:iniparser_dump()
 * 功能	:将ini结构中的变量输出到一个已经打开的文件流
 * 输入	:d:描述ini文件的结构
 * 输出	:f:已经打开的文件流，ini结构将输出到这个流中
 * 返回值	:无
 * 注:		本函数输出的文件格式为
 *			[section:key]=[val]
 *			该文件不能被iniparser_load调出
 **********************************************************************************************/
EXPORT_DLL void iniparser_dump(dictionary * d, FILE * f);

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, return NULL if not found
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
EXPORT_DLL char * iniparser_getstr(dictionary * d, char * key);


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
/**********************************************************************************************
 * 函数名	:iniparser_getstring()
 * 功能	:从ini结构中获取指定key的变量字符串值
 * 输入	:d:描述ini文件的结构
 *			 key:要访问的变量key	section:key
 *			def:如果找不到指定变量的返回值
 * 返回值	:变量字符串值,如果没有找到则返回def
 
 **********************************************************************************************/
EXPORT_DLL char * iniparser_getstring(dictionary * d, char * key, char * def);

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
/**********************************************************************************************
 * 函数名	:iniparser_getint()
 * 功能	:从ini结构中获取指定key的变量整数值
 * 输入	:d:描述ini文件的结构
 *			 key:要访问的变量key	section:key
 *			notfound:如果找不到指定变量的返回值
 * 返回值	:指定变量的整数值,如果没有找到则返回notfound
 **********************************************************************************************/
EXPORT_DLL int iniparser_getint(dictionary * d, char * key, int notfound);

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
EXPORT_DLL double iniparser_getdouble(dictionary * d, char * key, double notfound);

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
EXPORT_DLL int iniparser_getboolean(dictionary * d, char * key, int notfound);


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
/**********************************************************************************************
 * 函数名	:iniparser_setstr()
 * 功能	:将字符串值设置到ini结构中的变量
 * 输入	:d:描述ini文件的结构
 *			entry:要访问的变量key	section:key，如果只有节名则表示创建该节
 *			val:要设置的字符串值
 * 返回值	:0表示成功，负值表示失败
 * 注		:如果节名不存在则创建该节，如果变量不存在则创建该变量
 **********************************************************************************************/
EXPORT_DLL int iniparser_setstr(dictionary * ini, char * entry, char * val);

/*-------------------------------------------------------------------------*/
/**
  @brief    Delete an entry in a dictionary
  @param    ini     Dictionary to modify
  @param    entry   Entry to delete (entry name)
  @return   void

  If the given entry can be found, it is deleted from the dictionary.
 */
/*--------------------------------------------------------------------------*/
/**********************************************************************************************
 * 函数名	:iniparser_unset()
 * 功能	:从ini结构中删除指定的节或变量
 * 输入	:d:描述ini文件的结构
 *			entry:要删除的节或变量
 * 返回值	:无
 **********************************************************************************************/
EXPORT_DLL void iniparser_unset(dictionary * ini, char * entry);

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
/**********************************************************************************************
 * 函数名	:iniparser_find_entry()
 * 功能	:查找是否有指定的节或变量存在
 * 输入	:d:描述ini文件的结构
 *			entry:要查找的节或变量名
 * 返回值	:1表示存在 0表示不存在
 **********************************************************************************************/
EXPORT_DLL int iniparser_find_entry(dictionary * ini, char * entry) ;

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
/**********************************************************************************************
 * 函数名	:iniparser_load()
 * 功能	:读取指定的ini文件，转化为ini数据结构，将指针返回
 * 输入	:ininame:要读取的ini文件名
 * 返回值	:描述ini文件的结构指针，NULL表示出错
 * 注:		这个指针在用完后要用iniparser_freedict进行释放
 **********************************************************************************************/
EXPORT_DLL dictionary * iniparser_load(char * ininame);

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
/**********************************************************************************************
 * 函数名	:iniparser_freedict()
 * 功能	:释放已经打开并且不再使用的ini结构指针
 * 输入	:d:要释放的指针(iniparser_load的返回值)
 * 返回值	:无
 **********************************************************************************************/
EXPORT_DLL void iniparser_freedict(dictionary * d);



/*-------------------------------------------------------------------------*/
/**
  @brief    调出ini文件到数据结构并且锁住文件
  @param    	ininame:要打开的配置文件名
  	    		wait:如果该配置文件已经被其他程序打开是否进行等待 1表示等待 0表示直接退出
  	    		lockf:指向被锁文件的指针，本函数会将打开的锁文件指针存入此指针
  @return   dictionary结构 ,NULL表示出错 lockf中存放着锁文件的指针，save_inidict_file会用到
  @如果lockf!=NULL的话要对其调用save_inidict_file进行存储或调用fclose(lockf)进行关闭，这个指针在用完后要用iniparser_freedict进行释放

**/
/*--------------------------------------------------------------------------*/

EXPORT_DLL dictionary * iniparser_load_lockfile(char * ininame,int wait,FILE**lockf);
/*-------------------------------------------------------------------------*/
/**
  @brief    将ini数据结构更新到配置文件，并解锁
  @param    filename:要存储的配置文件名
  	    	ini:已经填充好的数据结构
  	    lockf:锁文件指针，调用iniparser_load_lockfile时参数lockf的值
  @return   0表示成功负值表示出错
**/
/*--------------------------------------------------------------------------*/
EXPORT_DLL int save_inidict_file(char *filename,dictionary * ini,FILE**lockf);

/*-------------------------------------------------------------------------*/
/**
  @brief    将整型变量的值存入指定的数据结构中的key
  @param    d:描述ini配置文件的结构指针
  	    	key:要存储的key
  	    	val:要存储的值
  @return   0表示成功负值表示出错
**/
/*--------------------------------------------------------------------------*/
EXPORT_DLL int iniparser_setint(dictionary * d, char * key, int val);
/*-------------------------------------------------------------------------*/
/**
  @brief    将整型变量的值以16进制存入指定的数据结构中的key
  @param    	ini:描述ini配置文件的结构指针
  	    		section:要存储的key
  	    		val:要存储的值
  @return   0表示成功负值表示出错
**/
/*--------------------------------------------------------------------------*/
EXPORT_DLL int iniparser_sethex(dictionary *ini,char* section,int val);

/*-------------------------------------------------------------------------*/
/**
  @brief    查找ini文件中是否有指定的节,如果没有则创建
  @param    filename:配置文件名
  	    section:节名
  @return   0表示成功负值表示出错
**/
/*--------------------------------------------------------------------------*/
EXPORT_DLL int iniparser_find_creat_sec(char *filename,char*section);

/*-------------------------------------------------------------------------*/
/**
  @brief    将指定变量的字符串存入配置文件中
  @param    filename:文件名
  	    	    section:"节名:变量名"
  	    	    vstr:变量的字符串形式的值  	   
  @return   0表示成功负值表示出错
**/
/*--------------------------------------------------------------------------*/
EXPORT_DLL int save2para_file(char *filename,char *section,char *vstr);

/*-------------------------------------------------------------------------*/
/**
  @brief    将指定变量的字符串存入ini数据结构
  @param    ini:ini文件结构指针
  	    	    section:"节名:变量名"
  	    	    vstr:变量的字符串形式的值  	   
  @return   0表示成功负值表示出错
**/
/*--------------------------------------------------------------------------*/
EXPORT_DLL int save2para(dictionary      *ini,char *section,char *vstr);

/******************************************************************
 * 函数名	ini_diff()
 * 功能:	比较两个ini文件是否相同
 * 输入:	oldfile,newfile,两个ini文件名
 * 返回值:  两个文件相等时返回0，不等时返回1,发生错误返回-1
 *
 * 如果两个文件不同则在终端和日志上记录信息:
 * alarm:snap_pic_num 5->4				表示变量alarm:snap_pic_num原来是5，新值是4
 * alarm:snap_pic_interval NULL->500	表示变量alarm: snap_pic_interval原来没有，新值是500
 * port:telnet_port 23->NULL		    表示变量port: telnet_port原来是23，后来把这个变量删除了
******************************************************************/

EXPORT_DLL int ini_diff(char *oldfile,char *newfile);

/******************************************************************
 *  * 函数名	ini_set_file()
 *   * 功能:	将source文件的每个节读出，与target文件相应节读出内容比较，若不同则改写target,若target不存在该节则创建,均记日志
 *    * 输入:	source:源文件名
 *     *			target,待改写的ini文件名
 *      * 返回值: 0表示成功,负值表示失败
 *       * 
 *        ******************************************************************/
EXPORT_DLL int ini_set_file(char *source,char *target);

#ifdef __cplusplus
}
#endif

#endif


