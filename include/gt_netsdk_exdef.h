/*
 * gt_netsdk 的一些扩展命令结构定义
 *
 */
#ifndef GT_NETSDK_EX_DEFINE
#define GT_NETSDK_EX_DEFINE

typedef struct {///设备当前的报警状态信息
	int	trig[32];			///端子报警状态			0:正常 1:有报警
	int motion[32];			///移动侦测报警状态		0:正常	1:有报警
}dev_alarm_stat_t;


typedef struct{///设备硬盘信息
	WORD    diskno;                 // 硬盘编号,0，1,2。。等（多硬盘时）
    BYTE    model[16];              //硬盘型号，字符串    
    BYTE    serialno[16];           //硬盘序列号，字符串
    BYTE    firmware[8];            //固件版本号，字符串
    WORD    volume;                 //容量(G为单位，如250G，320G)
    WORD    temprature;             //当前温度(摄氏)
	WORD    maxtemprature;			//历史最高温度(摄氏),值在100以内有效
    DWORD   age;                    //工作小时数
    DWORD   relocate;               //重分配扇区数
    DWORD   pending;                //当前挂起扇区数
    DWORD   error_no;               //错误日志数
	int		shot_test_res;			///短测试结果 0通过，1失败,2读不到或未测试过,3进行中
	int		shortstatus;			/////短测试若在进行中，完成的百分比，0-100的整数   
	int		long_test_res;			///长测试结果 0通过，1失败,2读不到或未测试过,3进行中
	int		longstatus;				//长测试若在进行中，完成的百分比，0-100的整数   
}dev_hd_info_t;

typedef struct{///设备当前触发状态信息
	int		trig[32];			///<触发端子的状态 0:正常状态 1:触发状态
	int		motion[16];			///<移动触发0:正常 1:触发状态
}dev_trig_info_t;

typedef struct{///设备的注册信息
		BYTE	dev_guid[8];			// 8字节的guid
		BYTE	dev_guid_str[128];		// guid的字符串形式

        DWORD vendor;                   //设备制造商标识(4) +设备型号标识(4)
        DWORD device_type;              //设备型号      
        BYTE site_name[40];             //安装地点名称  

        WORD video_num;					//设备视频输入通道数。
        WORD com_num;                   // 串口数
        WORD storage_type;              //设备的存储介质类型 0：没有 1：cf卡 2：硬盘
        DWORD storage_room;				//设备的存储介质容量 单位Mbyte
        WORD compress_num;			    //压缩通道数    目前为1，2或5
        WORD compress_type;				//压缩数据类型，(压缩芯片，压缩格式)
        WORD audio_compress;			//声音输入压缩类型
        WORD audio_in;                  //声音输入通道数，目前为1
        WORD switch_in_num;             //开关量输入通道数
        WORD switch_out_num;			//开关量输出通道数
       
		DWORD    cmd_port;              //命令服务端口
        DWORD    image_port;            //图像服务端口  
        DWORD    audio_port;            //音频服务端口
        BYTE    firmware[20];           //固件版本号，暂时不用
        BYTE    dev_info[40];           //设备的一些相关信息
        BYTE    ex_info[160];           //外接dvs(如果有的话)的相关信息，包括品牌，端口，用户名，密码

		BYTE    video_names[64][30];    //最多64路通道名称

}dev_regist_info_t;





#endif
