#ifndef GT_THREAD_H
#define GT_THREAD_H
#ifdef _WIN32
#include <windows.h>
#include <pthread/pthread.h>//pthread-win32
#else
#include <unistd.h>
#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
/**********************************************************************************************
 * 函数名	:gt_create_thread()
 * 功能	:创建一个分离状态的线程
 * 输入	:start_routine:线程的服务函数
 *			 arg:传递给线程服务函数的参数
 * 输出	:threadid:已创建的线程id
 * 返回值	:0表示成功负值表示出错
 **********************************************************************************************/
int  gt_create_thread(pthread_t  *threadid,void *(*start_routine)(void *), void * arg);

/**********************************************************************************************
 * 函数名	:gt_cond_wait_time()
 * 功能	:等待一个指定的条件变量 
 * 输入	:cond:条件变量指针
 *			 mutex:条件变量用到的互斥体指针
 *                   sec:超时时间(秒)
 * 返回值	:0表示收到信号
 *                     ETIMEDOUT 表示到达超时时间而没有收到信号
 *                     负值表示出错
 *                     
 **********************************************************************************************/
int gt_cond_wait_time(pthread_cond_t *cond,pthread_mutex_t *mutex,int sec);

/**********************************************************************************************
 * 函数名	:gt_cond_signal()
 * 功能	:给一个条件变量发送信号
 * 输入	:cond:条件变量指针
 *			 mutex:条件变量用到的互斥体指针
 * 返回值	:0表示成功
 *                   负值表示出错
 *                     
 **********************************************************************************************/
int gt_cond_signal(pthread_cond_t *cond,pthread_mutex_t *mutex);

#define	GT_CreateThread	gt_create_thread
#define GT_CancelThread         pthread_cancel                      ///<取消线程
#define  gt_cancle_thread           pthread_cancel
#ifdef _WIN32
#define	sleep(s)	Sleep(1000*s)
#define msleep(s)	Sleep(s);
int gettimeofday (struct timeval *tv, void *tz );
#else
#define msleep(s)	usleep(s*1000)
#endif

#ifdef __cplusplus
}
#endif

#endif


