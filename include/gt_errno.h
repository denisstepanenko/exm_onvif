#ifndef GT_ERRNO_H
#define GT_ERRNO_H
#include <stdio.h>
#include <errno.h>


#define	GT_ERRNO_BASE		1000		///<国通自定义错误码的基数
#define E_ACCOUNT		(GT_ERRNO_BASE +  0)	///账号或密码错误
#define E_NOINIT		(GT_ERRNO_BASE +  1)	///没有初始化
#define E_CHANNEL		(GT_ERRNO_BASE +  2)	///通道号错误
#define E_MAXUSER		(GT_ERRNO_BASE +  3)	///用户数达到最大
#define E_MACHVER		(GT_ERRNO_BASE +  4)	///版本不匹配
#define E_MACHTYPE		(GT_ERRNO_BASE +  5)	///类型、型号不匹配
#define E_READ			(GT_ERRNO_BASE +  6)	///读取失败
#define E_WRITE			(GT_ERRNO_BASE +  7)	///写入失败
#define E_DATA			(GT_ERRNO_BASE +  8)	///数据错误
#define E_ORDER			(GT_ERRNO_BASE +  9)	///调用次序错误
#define E_STATE			(GT_ERRNO_BASE + 10)	///状态错误
#define E_DISK			(GT_ERRNO_BASE + 11)	///磁盘错误
#define E_OPERATE		(GT_ERRNO_BASE + 12) 	///操作失败
#define E_NORESOURCE		(GT_ERRNO_BASE + 13) 	///资源不足
#define E_OPEN			(GT_ERRNO_BASE + 14)	///打开资源或文件失败
#define E_FORMAT		(GT_ERRNO_BASE + 15)	///格式错误
#define E_UPDATE		(GT_ERRNO_BASE + 16)	///升级失败
#define E_MACHIP		(GT_ERRNO_BASE + 17)	///IP地址不匹配
#define E_MACHMAC		(GT_ERRNO_BASE + 18)	///MAC地址不匹配
#define E_UPDATEFILE		(GT_ERRNO_BASE + 19)	///升级文件不匹配
#define E_UNKNOW		(GT_ERRNO_BASE + 20)	///未知错误
				












#ifdef _WIN32
   // error code mapping for windows
#undef  EINTR
#define EINTR 					WSAEINTR			/* Interrupted system call */

#define ETIME                   ERROR_SEM_TIMEOUT	/* Timer expired */
#define EWOULDBLOCK             WSAEWOULDBLOCK		/* Operation would block */
#define EINPROGRESS             WSAEINPROGRESS		/* Operation now in progress */
#define EALREADY                WSAEALREADY			/* Operation already in progress */
#define ENOTSOCK                WSAENOTSOCK			/* Socket operation on non-socket */
#define EDESTADDRREQ            WSAEDESTADDRREQ		/* Destination address required */
#define EMSGSIZE                WSAEMSGSIZE			/* Message too long */
#define EPROTOTYPE              WSAEPROTOTYPE		/* Protocol wrong type for socket */
#define ENOPROTOOPT             WSAENOPROTOOPT		/* Protocol not available */
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT	/* Protocol not supported */
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT	/* Socket type not supported */
#define EOPNOTSUPP              WSAEOPNOTSUPP		/* Operation not supported on transport endpoint */
#define EPFNOSUPPORT            WSAEPFNOSUPPORT		/* Protocol family not supported */
#define EAFNOSUPPORT            WSAEAFNOSUPPORT		/* Address family not supported by protocol */
#define EADDRINUSE              WSAEADDRINUSE		/* Address already in use */
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL	/* Cannot assign requested address */
#define ENETDOWN                WSAENETDOWN			/* Network is down */
#define ENETUNREACH             WSAENETUNREACH		/* Network is unreachable */
#define ENETRESET               WSAENETRESET		/* Network dropped connection because of reset */
#define ECONNABORTED            WSAECONNABORTED		/* Software caused connection abort */
#define ECONNRESET              WSAECONNRESET		/* Connection reset by peer */
#define ENOBUFS                 WSAENOBUFS			/* No buffer space available */
#define EISCONN                 WSAEISCONN			/* Transport endpoint is already connected */
#define ENOTCONN                WSAENOTCONN			/* Transport endpoint is not connected */
#define ESHUTDOWN               WSAESHUTDOWN		/* Cannot send after transport endpoint shutdown */
#define ETOOMANYREFS            WSAETOOMANYREFS		/* Too many references: cannot splice */
#ifndef ETIMEDOUT
#define ETIMEDOUT               WSAETIMEDOUT		/* Connection timed out */
#endif
#define ECONNREFUSED            WSAECONNREFUSED		/* Connection refused */
#define ELOOP                   WSAELOOP			/* Too many symbolic links encountered */
#define EHOSTDOWN               WSAEHOSTDOWN		/* Host is down */
#define EHOSTUNREACH            WSAEHOSTUNREACH		/* No route to host */
#define EPROCLIM                WSAEPROCLIM
#define EUSERS                  WSAEUSERS			/* Too many users */
#define EDQUOT                  WSAEDQUOT			/* Quota exceeded */
#define ESTALE                  WSAESTALE			/* Quota exceeded */
#define EREMOTE                 WSAEREMOTE			/* Object is remote */
 // #define ENAMETOOLONG            WSAENAMETOOLONG	/* File name too long */


#endif

///返回错误码对应的描述字符串
///需要使用gt_errno库
//
//char *gt_strerror(int err_no);
#endif
