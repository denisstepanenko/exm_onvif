#ifndef PUB_ERR_H
#define PUB_ERR_H

#define SUCCESS                 		0	//成功
#define ERR_DEVICE_BAD			1	//设备有故障,具体结果以文本方式存入结果文件
#define ERR_CANNOT_OPEN_FILE    4	//不能打开指定的结果文件
#define ERR_INTERNAL			5	//测试程序内部错

/*
****************************************************************************
测试结果数据结构 0 表示正常，其它值表示相应的错误
	-1 无法加载驱动模块
	-2  无法打开驱动模块
	-3  无法对驱动模块读写
	-4  无法对驱动模块进行I/O 控制
****************************************************************************
*/
#define	Net_error 			"编码器0故障"
#define	Hq_error 			"编码器1故障"
#define	Tw2834_error 		"视频切割器故障"
#define	Ide_error 			"IDE 驱动器故障"
#define	Dsp_error			"音频处理器故障"
#define    TW9903_error		"TW9903故障"
#define    UNKNOW 			"未知的设备故障"

#define ERR_NO6410			"编码器芯片没有焊上,"		// error code 1
#define ERR_NODATA			"编码器无数据输出/6410虚焊,"			// error code 3
#define ERR_NOINPUT			"编码器无数据输入/6410虚焊/C95144 虚焊/TW2834虚焊,"			// error code 2
#define ERR_UNSTABLE		"编码器工作不稳定丢数据,"  // error code 4

#define ERR_NO2834			"TW2834 芯片没有焊上,"		// error code 1
//#define ERR_NO2834			"TW2834 芯片没有焊上,"		// error code 1

#define ERR_NOIDE			"IDE 驱动器容量错误或没有焊上, "  //error code 1
#define ERR_NODISK			"没有找到IDE设备,"
#define ERR_NOPART			"磁盘没有分区,"
#define ERR_OPEN			"打开测试文件失败,"
#define ERR_WRITE			"写测试文件失败,"
#define ERR_READ				"读测试文件失败,"
//#define ERR_NOPART			"磁盘没有格式化分区,"


#define ERR_NODSP			"语音芯片没有焊上,"			//error code 1

#define ERR_IIC				"模拟IIC驱动没加载"
#define ERR_NO9903			"读写9903失败"
//错误代码定义
#define	NETENC_ERR		10
#define	HQENC_ERR			20
#define	QUARD_ERR			30
#define	IDE_ERR			40
#define	DSP_ERR			50
#define 	TW9903_ERR		60
#define 	USB_ERR		70

#endif


