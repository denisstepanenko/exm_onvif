#ifndef __INI_CONV_H_
#define __INI_CONV_H_

/**
	@file:	ini_conv.c
	@brief:	在gt3022上用作gt1000.ini与yy所用两个配置文件之间进行相应的变量值转换，
			对有变动的进行修改并保存，无改动的不做任何保存
	@date:	aah 2009.02.09开始编写，2009.02.19递交第一版
	@modify:2009.09.09	ver:0.03增加remote_gate:alarm_gate的转换 (wsy,BUG 743)
			2009.02.20  ver:0.02去掉冗余的命令行处理函数；
							   增加宏定义，处理不易理解的数字；
			2009.02.27  ver:0.03 修改变量函数为static,增加参数宏定义，便于维护；
							   改变维护表，换成两个属性；
							   改变模块架构，按两个属性归类；
							   single2multi和multi2single函数合并成一个函数，即s2m_or_m2s；
							   增加字符串异同对比函数str_compare；
							   增加特殊节点处理函数spcial_proc及特殊节点表;
							   程序分.c和.h进行维护；
			2009.03.02  ver:0.04 节点hqenc0与netencode互换位置；
							   找不到源节点不记录日志；
							   不存储也记录到日志里面；
			2009.03.23  ver:0.05 增加数据列表初始化头文件，从本头文件分离出；
			2009.05.21	ver:0.06 增加对于net_config:route_default和dns_server的转换
			2011.03.31     ver:0.07 zsk 增加对于gps信息gps:band gps:enable gps:port gps:stop gps:parity gps:date的转换
*/

#include <string.h>


#define VIRDEV_0_INI	 "/conf/virdev/0/ip1004.ini"	///<yy第一个配置文件路径
#define VIRDEV_1_INI 	"/conf/virdev/1/ip1004.ini"	///<yy第二个配置文件路径
#define MAIN_INI		     	"/conf/ip1004.ini"	///<ip1004.ini路径
#define INI_CONV_VER  	("0.07")					///<程序版本号

#define NO_FOUND_STR 	"no_str"					///<ini节点不存在时的返回值

///<对比A.B所指向的字符串的长度，返回给strncmp函数，便于比较是否有参数修改
#define MAX(A, B)			((strlen(A)) >= (strlen(B)) ? (strlen(A)) : (strlen(B)))	

///配置文件转换方向
#define CONV_S				0					///<gt1000向其他配置文件转换
#define CONV_M				1					///<其它配置文件向gt1000转换

///节点key类型
#define NO_ACCORD2_A		0					///<gt1000与其它配置文件转换时对比是否修改以A.INI为主
#define IS_ACCORD2_A		1					///<不以A为主
	
///节点key属性
#define NOT_S_ONLY			0					///<不属于-s only，可以相互转换
#define IS_S_ONLY			1					///<是-s only

///判断配置文件是否修改
#define NO_CHANGE			0					///<判断是否有改动，0没有改动
#define CHANGE_SAVE		1					///<判断是否有改动,    1发生改动，需要保存

//static  int main_ini_changed =	NO_CHANGE;			///<判断gt1000.ini配置文件是否修改过，0表示没有改动过，不需要保存；1表示有改动，需要保存
//static  int vir0_ini_changed =	NO_CHANGE;			///<判断virdev 0配置文件是否修改过，0表示没有改动过，不需要保存；1表示有改动，需要保存
//static  int vir1_ini_changed =	NO_CHANGE;			///<判断virdev 1配置文件是否修改过，0表示没有改动过，不需要保存；1表示有改动，需要保存



///转换配置文件信息所用结构体
typedef struct{
	int conv_att;			///<配置文件转换属性:conv_att为0表示不以A为主；为1表示以A为主
	int is_s_only;		///<判断是否是-s only，0说明是-s only，1为-s/-m均可
	char *sec_key_gt1;	///<gt1000里要与第一个yy的ini配置文件转换的key
	char *sec_key_gt2;	///<gt1000里要与第二个yy的ini配置文件转换的key
	char *sec_key_a;		///<yy第一个配置文件对应gt1000的key
	char *sec_key_b;		///<yy第二个配置文件对应gt1000的key
}conv_ini_t;


///在转换前需要进行特殊处理的节点key结构
typedef struct{
	char *sec_key_gt1;	///<gt1000里要与第一个yy的ini配置文件转换的key
	char *sec_key_gt2;	///<gt1000里要与第二个yy的ini配置文件转换的key
	char *sec_key_a;		///<yy第一个配置文件对应gt1000的key
	char *sec_key_b;		///<yy第二个配置文件对应gt1000的key
}conv_spcial_t;


#endif
