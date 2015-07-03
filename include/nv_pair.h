
/**
  * 名值对解析库
  * 说明: 名值对由多个字符串格式的"名"和"值"通过分隔符和相等符串接而成, 如:
  *       name==alan^^age==18^^mobile==1392332324
  *       其中, == 为 相等符, ^^为 分割符
  *	应用范围:
  *	   	名值对的个数不能超过50 
  *  		名值对的长度不能超过100 个字节
  *		分隔号和等号的长度不能超过20个字节
  *
  * 使用方法(异常处理自己加):
  * 	顺序执行
  *	NVP_TP	*nvp=NULL;	///<NVP结构变量
  *	int	nvp_values;	///<NVP结构中的名值对数量
  *	char	*nv_arg="name==alan^^age==18^^mobile==1392332324";	///<待解析的名值对字符串
  *	nvp=nvp_create();	
  *	nvp_set_seperator(nvp,"^^");
  *	nvp_set_equal_mark(nvp,"==");
  *	nvp_values=nvp_parse_string(nvp,nv_arg);	///<解析名值对字符串，获取串中的名值对个数
  *
  *
  *
  * 	nvp_destroy(nvp);				///<用完后释放nvp结构
  */

#ifndef _GTCA_NVPAIR_H_20060918_
#define _GTCA_NVPAIR_H_20060918_
typedef	void		NVP_TP;		//应用程序调用库时用到的描述nvp结构的类型
/****************************错误码定义**********************************************/
#define MAX_MARK_LEN     	20		// 等号的最大长度
#define MAX_SEP_LEN      	20		//分隔符的最大长度
#define MAX_CMD_NUM	 		50		// 最大支持的命令个数
#define MAX_DATA_LEN	 	100		//最大命令长度

#define	NVP_SUCCESS			0	//操作成功
#define	NVP_NO_MEM			1000	//内存不足
#define NVP_PARA_ERR  		1001	//参数错误 

//其它值待定
/////////////////////////////////////////////////////////////////////////////////////
#undef IN
#undef OUT
#undef IO

	#define IN 
	#define OUT
	#define IO


#ifdef __cplusplus
extern "C" {
#endif
#undef EXPORT_DLL
#ifdef _WIN32
	//windows 使用

	#define EXPORT_DLL __declspec(dllexport)

#else

	//linux 使用

	#define EXPORT_DLL

#endif
	
/////////////////////////////////////////////////////////////////////////////////////
/*
*************************************************************************
*函数名	:nvp_create
*功能	: 创建一个名值对结构
*输入	:  无
*输出	: 
*修改日志:
*************************************************************************
*/
EXPORT_DLL NVP_TP	*nvp_create(void);
/*
*************************************************************************
*函数名	:nvp_set_seperator
*功能	: 设置分隔符 * 
*输入	:  
			NVP_TP	*nv,          之前使用nvp_create得到的指针
			const char * seperator < 分隔符字串 
*输出	: 正确0  错误码
*修改日志:
*************************************************************************
*/
EXPORT_DLL int nvp_set_seperator(
			IN NVP_TP	*nv,          /*之前使用nvp_create得到的指针*/
			IN const char * seperator /**< 分隔符字串 */
		);
/*
*************************************************************************
*函数名	:nvp_set_equal_mark
*功能	: 相等符
*输入	:  
			NVP_TP	*nv,          之前使用nvp_create得到的指针
			const char * mark 	  分隔符字串 
*输出	: 正确0  错误码
*修改日志:
*************************************************************************
*/
EXPORT_DLL int nvp_set_equal_mark(
			IN NVP_TP	*nv,          /*之前使用nvp_create得到的指针*/
	       	IN const char * mark 	/**< 相等符*/
		);
/*
*************************************************************************
*函数名	:nvp_set_pair_str
*功能	: 压入名值对(字符串型)
* 注意: 如果相同键值的数据存在则进行替换原有值
*输入	:  
		NVP_TP	*nv,          	之前使用nvp_create得到的指针
		const char * name, 	< 名 
		const char * value 	< 值 
*输出	: 0表示成功负值表示出错
*修改日志:
*************************************************************************
*/
EXPORT_DLL int nvp_set_pair_str(
			IN NVP_TP	*nv,          	/*之前使用nvp_create得到的指针*/
			IN const char * name, 	/**< 名 */
			IN const char * value 		/**< 值 */
		);
/*
*************************************************************************
*函数名	:nvp_set_pair_int
*功能	: 压入名值对(整数型)
* 注意: 如果相同键值的数据存在则进行替换原有值
*输入	:  
		NVP_TP	*nv,          	之前使用nvp_create得到的指针
		const char * name, 	< 名 
		int value 	< 值 
*输出	: 0表示成功负值表示出错
*修改日志:
*************************************************************************
*/
EXPORT_DLL int nvp_set_pair_int(
			IN NVP_TP *nv, 			/*之前使用nvp_create得到的指针*/
			IN const char * name, 	/**< 名 */
			IN int value 				/**< 值 */
		);

/*
*************************************************************************
*函数名	:nvp_get_pair_str
*功能	: 根据名称得到值,如果未找到则返回dev_val
*输入	:  
		NVP_TP	*nv,          	之前使用nvp_create得到的指针
		const char * name, 	< 名 
		const char * def_val 	< 默认值 
*输出	: 0表示成功负值表示出错
*修改日志:
*************************************************************************
*/
EXPORT_DLL const char * nvp_get_pair_str(
			IN NVP_TP	*nv,          	/*之前使用nvp_create得到的指针*/
			IN const	char * name ,		/**< 名 */
			IN const	char * def_val		//如果找不到指定名字返回的值
		);
/*
*************************************************************************
*函数名	:nvp_get_pair_int
*功能	: 根据名称得到值,如果未找到则返回dev_val
*输入	:  
		NVP_TP	*nv,          	之前使用nvp_create得到的指针
		const char * name, 	< 名 
		int def_val 	< 默认值 
		
*输出	: 名值对的整数值
*修改日志:
*************************************************************************
*/
EXPORT_DLL int nvp_get_pair_int(
			NVP_TP *nv, 			//之前使用nvp_create得到的指针
			const char * name , 	//const char * name, 	< 名
			const int def_val		//< 默认值 
		);
/*
************************************************************************
*函数名	:nvp_get_string
*功能	: 得到所有名值对的串接格式
*输入	:  
			NVP_TP	*nv,          之前使用nvp_create得到的指针
*输出	: 字符串指针
*修改日志:
*************************************************************************
*/
EXPORT_DLL const char * nvp_get_string(IN NVP_TP *nv    	/*之前使用nvp_create得到的指针*/);
 /*
************************************************************************
*函数名	:nvp_parse_string
*功能	:  解析名值对的串接格式
*输入	:  
			NVP_TP	*nv,          之前使用nvp_create得到的指针
			const char *  str      名值对的串接格式 
*输出	:  解析得到名值对的数量
*修改日志:
*************************************************************************
*/
EXPORT_DLL int nvp_parse_string(
			IN NVP_TP	*nv,          	/*之前使用nvp_create得到的指针*/
			IN const char *  str /**< 名值对的串接格式 */
		);
 /*
************************************************************************
*函数名	:nvp_dump
*功能	:  打印所有名值对的内容(调试用)
*输入	:  
			NVP_TP	*nv,          之前使用nvp_create得到的指针
*输出	:  解析得到名值对的数量
*修改日志:
*************************************************************************
*/
EXPORT_DLL int nvp_dump( IN NVP_TP	*nv      	/*之前使用nvp_create得到的指针*/);
 /*
************************************************************************
*函数名	:nvp_get_count
*功能	:  得到名值对的数量
*输入	:  
			NVP_TP	*nv,          之前使用nvp_create得到的指针
*输出	:  解析得到名值对的数量
*修改日志:
*************************************************************************
*/
EXPORT_DLL int nvp_get_count(IN NVP_TP	*nv     	/*之前使用nvp_create得到的指针*/);
 /*
************************************************************************
*函数名	:nvp_destroy
*功能	:  销毁一个已经使用过的nvp结构
*输入	:  
			NVP_TP	*nv,          之前使用nvp_create得到的指针
*输出	:  无
*修改日志:
*************************************************************************
*/
EXPORT_DLL void nvp_destroy(IN NVP_TP	*nv          	/*之前使用nvp_create得到的指针*/);

 /*
************************************************************************
*函数名	:nvp_get_error_str
*功能	:  获取错误码的字符串描述
*输入	:  
			int errno, 接口返回的错误代码的绝对值
*输出	:     错误描述字符串指针
*修改日志:
*************************************************************************
*/
EXPORT_DLL const char *nvp_get_error_str(IN int errno);

#ifdef __cplusplus
}
#endif

#endif //_GTCA_NVPAIR_H_20060918_
