#ifndef CSVPARSER_EX_H
#define CSVPARSER_EX_H

#include "csvparser.h"

typedef  struct {
char filename[100]; 
}csv_dict ;      //fixme later

/**
  @brief    调出csv文件到数据结构并且锁住文件
  @param    csvfile:要打开的csv文件名
  	    	wait:如果该配置文件已经被其他程序打开是否进行等待 1表示等待 0表示直接退出
  @return   csv_dict结构 ,NULL表示出错 
**/
csv_dict * csvparser_load_lockfile(char * csvfile,int wait);



/**
  @brief    将csvdict数据结构更新到配置文件，并解锁
  @param    filename:要存储的配置文件名
  	    	csvdict:已经填充好的数据结构
  	     
  @return   0表示成功负值表示出错
**/
int csvparser_savefile (char *filename,csv_dict *csvdict);


/************************************************************************
*函数名	:csvparser_get_total_records
*功能	:获取一个csv_dict数据结构中的csv记录总数
*输入	:  	 IN csv_dict * csvdict ;	//csv_dict数据结构
*返回值	:非负值表示成功负值表示失败
*修改日志:
*************************************************************************/
int csvparser_get_total_records(csv_dict * csvdict);

/************************************************************************
*函数名	:csvparser_get_record
*功能	:从csv_dict数据结构中获取指定记录号的结构信息
*输入	:
	 IN csv_dict * csvdict;	//csv_dict数据结构
	 IN int record_no;		//csv结构中的记录号(正值表示按正序访问,-1表示倒数第一条)
*输出 	:OUT CSV_T *csv;		//指定记录号的记录信息
*返回值	:0表示成功负值表示失败 
*************************************************************************/
int csvparser_get_record(csv_dict * csvdict,int record_no, CSV_T *csv);

/************************************************************************
*函数名	:csvparser_set_record
*功能	:将csv结构写入csv_dict数据结构中
*输入	:  
	 IN csv_dict * csvdict;	//csv_dict数据结构
	 IN int record_no;		//csv结构中的记录号(正值表示按正序访问,负值表示按倒序访问)
	 				//正序时如果超出最大序号则表示从文件尾部插入
					//倒序时如果超出范围则表示从首部插入
* 	 IN CSV_T *csv;			//指定记录号的记录信息
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************/
int csvparser_set_record(csv_dict * csvdict,int record_no, IN CSV_T *csv);

/************************************************************************
*函数名	:csvparser_insert_record
*功能	:将csv结构插入csv_dict数据结构中
*输入	:  
	 	IN csv_dict * csvdict;	//csv_dict数据结构
	 IN int record_no;			//csv结构中的记录号(正值表示按正序访问,负值表示按倒序访问)
	 						//正序时如果超出最大序号则表示从文件尾部插入
							//倒序时如果超出范围则表示从首部插入
* 	 IN CSV_T *csv;			//指定记录号的记录信息
*返回值	:0表示成功负值表示失败
*修改日志:
*************************************************************************/
int csvparser_insert_record(csv_dict * csvdict,int record_no,IN CSV_T *csv);

/************************************************************************
*函数名	:csvparser_rm_record
*功能	:将filename文件中的第record_no条信息删除掉
*输入	:  
		IN csv_dict * csvdict;	//csv_dict数据结构
	 IN int record_no;		//csv结构中的记录号(正值表示按正序访问,负值表示按倒序访问)
	 				//正序时如果超出最大序号则表示从文件尾部删除
					//倒序时如果超出范围则表示从首部删除
*返回值	:0表示成功，负值表示失败
*修改日志:
*************************************************************************/
int csvparser_rm_record(csv_dict * csvdict,int record_no);



#endif

