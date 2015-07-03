 #ifndef UPNPD_H
#define UPNPD_H
#include "upnptools.h"

//upnpd的状态
#define	UPNPD_STATE_INIT		2
#define UPNPD_STATE_GETROUTER	3
#define	UPNPD_STATE_OK			UPNPD_SUCCESS
#define	UPNPD_STATE_FAILED		UPNPD_FAILURE


#define VERSION		"0.07"
//0.07  lc      2012-12                 移植到ip1004设备内
//0.06	wsy	2008-03-27		纠正了内存泄露的问题
//0.05  wsy 2008-03-19	如果用dhcp,则一直等到dhcp找到可用路由再往下进行
//0.04  wsy 2008-01-10	在路由器重起时也能自动发现并重新映射
//						端口映射的名称去掉前面的"port:"	
//0.03	wsy	2007-10-25	支持自动寻找当前路由的功能
//0.02  wsy 2007-10-24	重写了全局变量部分,增加了模块通讯和状态监控线程
//0.01	wsy	2007-10-19	初始化

#define UPNPD_LOCKFILE			"/lock/ipserver/upnpd"

#define	PORT_AVAILABLE		0	//端口空闲未被映射
#define PORT_OCCUPIED		1	//端口被其它服务占据
#define PORTMAP_FAILED		2	//端口映射失败
#define	PORTMAP_SUCCESSFUL	3	//端口已被本服务映射成功


#define IP_SERVICE_TYPE 		"urn:schemas-upnp-org:service:WANIPConnection:1"//upnp端口映射的关键字
#define UPNPD_PARA_FILE			"/conf/ip1004.ini" //upnpd 配置文件
#define UPNPD_SAVE_FILE			"/conf/ip1004-upnp.ini" //upnpd保存的文件，用于和上文作比较

#define SEARCH_ROUTER_INTERVAL	15	//单位为s,范围为2-80。发送搜寻可用路由器命令的间隔时间
#define KEEPALIVE_INTERVAL		10 //单位为s, 这么久查询一次端口映射是否健在,若失败就判定需要重新开始搜寻可用的路由器
#define SENDSTATE_INTERVAL		180 //单位为s,这么久没有可用路由器就报告主进程，从而报告网关

typedef struct 
{
	char	*port_dscrp;	//端口描述,同配置文件中保持一致，如"port:cmd_port"
}
port_t; //描述端口的结构


/********************************************************************************
 * 函数名:get_current_state()
 * 功能: 返回当前状态
 ********************************************************************************/
int get_current_state(void);

#endif
