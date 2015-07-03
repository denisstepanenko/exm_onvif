#ifndef _SYS_ERROR_H_
#define _SYS_ERROR_H_
// 错误代码
#if 0
#define FMAT_OK		300
#define NO_DISK		301
#define NO_HDA1		302
#define MOUNT_ERR	303
#define FDISK_ERR	304
#define FMAT_ERR	305
#endif
#define CLR_OK		400
#define NO_LOG		401
#define NO_HQDATA	402
#define RM_HQD_ERR	403
#define RM_LOG_ERR	404
#if 0
#define NO_SERVER_IP 	501
#define SERVER_IP_ERR 	502
#define NO_PATH		 	503
#define NO_PASS		 	504
#define NO_USER		 	505
#define NO_SIZE		 	506
#define NO_PORT		 	507
// 格式化磁盘失败错误信息
static const char* format_OK	=	"磁盘格式化成功";
static const char* format_err	=	"格式化磁盘失败";
static const char* no_disk		=	"没有硬盘或cf卡";
static const char* no_hda1		=	"找不到/dev/hda1节点";
static const char* mount_err	=	"加载磁盘失败";
static const char* fdisk_err	=	"磁盘分区失败";
//#define FDISK_ERROR		"格式化磁盘失败"
#endif

static const char* clear_OK	=	"清除测试记录成功";

#if 0
// 清除测试记录失败错误信息
static const char* clear_OK	=	"清除测试记录成功";
static const char*	no_log 		=	"没有/log目录";
static const char* no_hqdata	=	"没有/hqdata目录";
static const char* rm_hqd_err 	=	"删除/hqdata下文件失败";
static const char* rm_log_err	=	"删除/log下文件失败";
#endif
#endif
