/*
 * err.h
 *
 *  Created on: 2013-10-28
 *      Author: yangkun
 */

#ifndef ERR_H_
#define ERR_H_
#include <iostream>
using std::cout;
using std::endl;
namespace gtsda
{
#define UNKNOW_ERR 				-1
#define BLOCK_ERR_DATA_HEAD 	-2 		//（年块,天块,秒块中数据）数据块头错误
#define HD_ERR_READ				-3 		//硬盘读出错
#define HD_ERR_WRITE			-4 		//硬盘读出错
#define BLOCK_ERR_NULL 			-5		//传入指针参数为NULL
#define BLOCK_ERR_ZERO 			-6		//传入指针参数有效，但数据区全为0
#define BLOCK_ERR_FULL 			-7 		//（年块天块中数据）数据写满了，应该只有年块会写满
#define BLOCK_ERR_EMPTY 		-8		//（年块天块中数据）为空，没有有效数据
#define BLOCK_ERR_NOT_IN    	-9 		//时间，seek不在天块，年块，秒块中
#define BLOCK_ERR_NOT_ENOUGH	-10		//复制块时，目的地址没有足够的内存。
#define BLOCK_ERR_DAY_SEC_MUT 	-11		//天块中一个秒块的位置有多个秒块对应。有可能的情况是1、一秒有多个I帧2、系统时间出错，向前走了
#define HD_ERR_OVER				-12

extern const  char *cErr[];
#define  print_err( iNo) \
{ \
	if(iNo<0){ \
	cout << endl << "error: " << cErr[abs(iNo)-1] ; \
	cout << "\t"<< __FILE__ << "\t" << __FUNCTION__ << "\t" << __LINE__ << endl; \
	}\
}while(0)
#define ttt() 	cout << __FILE__ << "\t" << __FUNCTION__ << "\t" << __LINE__ << endl;
int myprint(const unsigned char *p, long size);

#define LOCKFILE "/var/run/mydaemon.pid"
#define LOCKREAD "/var/run/sdaread.pid"
#define LOCKWRITE "/var/run/sdawrite.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
int already_running(const char *filename);
} /* namespace gtsda */
#endif /* ERR_H_ */
