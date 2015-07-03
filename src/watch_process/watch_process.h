#ifndef WATCH_PROCESS_H
#define WATCH_PROCESS_H
#include <file_def.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
 #include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define	VERSION		"2.49"

//2.49  针对multichannel选项的不同，采用两套程序，不同的watch_target
//2.48  加入对playback程序的监控
// 2.47 修正对线程数获取的bug
//2.46  加入对于hdmodule和diskman的监控
// 2.45 用于ip1004设备，取消某些进程监控，和线程数控制
// 2.44 videoenc的线程数大于6个就重启videoenc
// 2.43 diskman的线程监控数 4->3
// 2.42 统计行数的函数放到了commonlib中
// 2.41 hdmodule由3->4
// 2.40 支持多型号,去掉对/log/vsftpd.txt的监控
// 2.12 发现有Z的进程也将信息记入/log/debug
// 2.11 加入对vsftpd的日志文件大小的监视
// 2.10 changed vsmain thread num 16->15
// 2.09 改进了判断线程数的方法，
// 2.08 发现线程数少时则sleep 1，然后再读取一次
// 2.07 将验证mac地址合法性的工作放到patch_conf中，将应用程序名改为watch_proc
// 2.06 加入当线程数量不足时将当时的情况记录日志/log/debug
// 2.04 added warning info about USE_V1 when compile
// 2.03 修正了由于启动时调用了 signal(SIGCHLD,SIG_IGN);导致的无法调用system的问题
// 2.02 改变了程序启动顺序
// 2.02 close_all_res 以配合fix_disk; USEV1宏定义可监测v1版程序 wsy
// 2.01 加入通过判断线程数量来判断进程是否正常运行的功能
// 2.00 更换结构及监控对象的程序名
// 0.40 fix problem when other program not exist
// 0.39 将启动vsmain的程序也单独开一个进程
// 0.38 fix close_all_res() from lib
// 0.37加入启动时对监控时间参数的检查，如果小于1则置1
//0.36 加入启动时创建并检查锁文件
// 0.35 added diskman support
#define WATCH_PARA_FILE 	"/conf/ip1004.ini"
#define WATCH_LOCK_FILE	"/lock/ipserver/watch_process"



#endif
