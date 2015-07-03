///GTVS1000系列视频服务器型号定义
#ifndef GTVS1000_DEV_TYPE_H
#define GTVS1000_DEV_TYPE_H
//只能被devtype.c在包含了 include <devinfo.h>和定义了dev_name_t 后包含
/************************具体的设备型号的子设备列表*************************************************************/
/*
	定义设备的子设备列表时，同类型的多个设备一定要按照顺序定义，如Ime6410VEnc0一定要在Ime6410VEnc1之前
	定义一个新的设备型号时需要定义新型号的static DevType_T 结构数组和static GTSeriesDVSR结构资源定义，然后
	把static GTSeriesDVSR的指针加入DEVSupportList即可
*/

//GT1024
static DevType_T *GTVS1024_O_List[]=
{	//GT1024 子设备列表
	&UART0,
	&UART1,
	&QuadDev,
	&Ime6410VEnc0,
	&Ime6410VEnc1,
	&IDEDisk,
	&NetPort0
};

static GTSeriesDVSR     DEV_GTVS1024_O={
        .type		=	T_GTVS1024_O,      //int     type;      //#设备型号
	 .comment	=	"两个视频压缩芯片,带画面分割器",
        .trignum		=	6,              //int     trignum;        	//#设备最高报警输入数
	 .outnum		=	4,		   //		outnum;		//输出端子数
        .com		=	2,              //int     com;            	//#设备串口数不需要在界面上设置
        .quad		=	1,              //int     quad;           	//#是否有画面分割器，1表示有 0表示没有
	 .osd		=	1,							//有osd
        .videonum	=	4,              //int     videonum;       	//#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum=	2,              //int     videoencnum;    //#视频编码器数量
	 .hqencnum	=	1,			
	 .use_cpu_iic  =	0,		  //int	use_cpu_iic	不使用cpuiic总线控制视频ad转换芯片
        .ide			=	1,              //int     ide;   //#1表示有cf卡或硬盘 0表示没有
		.audionum      =   1,             //int audio
        .eth_port	=	1,              //int     eth_port;       	//#网口数 1表示一个 2表示两个
        .list_num	=	sizeof(GTVS1024_O_List)/sizeof(DevType_T*),
        .list			=	(DevType_T**)GTVS1024_O_List        //DevType_T *List[];      //子设备列表
};

static GTSeriesDVSR     DEV_GTVS1024_OA={
        .type		=	T_GTVS1024_OA,      //int     type;      //#设备型号
	 .comment	=	"比GTVS1024-O多了音视频放大器",
        .trignum		=	6,              //int     trignum;        	//#设备最高报警输入数
	 .outnum		=	4,		   //		outnum;		//输出端子数
        .com		=	2,              //int     com;            	//#设备串口数不需要在界面上设置
        .quad		=	1,              //int     quad;           	//#是否有画面分割器，1表示有 0表示没有
	 .osd		=	1,							//有osd
        .videonum	=	4,              //int     videonum;       	//#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum=	2,              //int     videoencnum;    //#视频编码器数量
	 .use_cpu_iic  =	0,		  //int	use_cpu_iic	不使用cpuiic总线控制视频ad转换芯片
	 .hqencnum	=	1,			
        .ide			=	1,              //int     ide;            	//#1表示有cf卡或硬盘 0表示没有
        .audionum      =   1,             //int audio
        .eth_port	=	1,              //int     eth_port;       	//#网口数 1表示一个 2表示两个
        .list_num	=	sizeof(GTVS1024_O_List)/sizeof(DevType_T*),
        .list			=	(DevType_T**)GTVS1024_O_List        //DevType_T *List[];      //子设备列表
};
//GT1011
static DevType_T *GTVS1011_List[]=
{	//GT1024 子设备列表
	&UART0,
	&UART1,
	&Ime6410VEnc0,
	&IDEDisk,
	&NetPort0,
};
static GTSeriesDVSR     DEV_GTVS1011={
        .type		=	T_GTVS1011,      //int     type;      //#设备型号
	 .comment	=	"一个视频压缩芯片,不带画面分割器",
	 .trignum		=	6,              //int     trignum;        	//#设备最高报警输入数
	 .outnum		=	4,		   //		outnum;		//输出端子数
        .com		=	2,              //int     com;            	//#设备串口数不需要在界面上设置
        .quad		=	0,              //int     quad;           	//#是否有画面分割器，1表示有 0表示没有
	 .osd		=	0,							//有osd
        .videonum	=	1,              //int     videonum;       	//#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum=	1,              //int     videoencnum;    //#视频编码器数量
	 .hqencnum	=	1,			
	 .use_cpu_iic  =	1,		  //int	use_cpu_iic	使用cpuiic总线控制视频ad转换芯片
        .ide			=	1,              //int     ide;            	//#1表示有cf卡或硬盘 0表示没有
        .audionum      =   1,             //int audio
        .eth_port	=	1,              //int     eth_port;       	//#网口数 1表示一个 2表示两个
        .list_num	=	sizeof(GTVS1011_List)/sizeof(DevType_T*),
        .list			=	(DevType_T**)GTVS1011_List        //DevType_T *List[];      //子设备列表
};
static GTSeriesDVSR     DEV_GTVS1011_A={
        .type		=	T_GTVS1011_A,      //int     type;      //#设备型号
	 .comment	=	"一个视频压缩芯片,不带画面分割器",
	 .trignum		=	6,              //int     trignum;        	//#设备最高报警输入数
	 .outnum		=	4,		   //		outnum;		//输出端子数
        .com		=	2,              //int     com;            	//#设备串口数不需要在界面上设置
        .quad		=	0,              //int     quad;           	//#是否有画面分割器，1表示有 0表示没有
	 .osd		=	0,							//有osd
        .videonum	=	1,              //int     videonum;       	//#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum=	1,              //int     videoencnum;    //#视频编码器数量
	 .hqencnum	=	1,			
	 .use_cpu_iic  =	1,		  //int	use_cpu_iic	不使用cpuiic总线控制视频ad转换芯片
        .ide			=	1,              //int     ide;            	//#1表示有cf卡或硬盘 0表示没有
        .audionum      =   1,             //int audio
        .eth_port	=	1,              //int     eth_port;       	//#网口数 1表示一个 2表示两个
        .list_num	=	sizeof(GTVS1011_List)/sizeof(DevType_T*),
        .list			=	(DevType_T**)GTVS1011_List        //DevType_T *List[];      //子设备列表
};

//GT1001
static GTSeriesDVSR     DEV_GT1001={
        .type           =       T_GT1001,      //int     type;      //#设备型号
        .comment        =       "一个视频压缩芯片,不带画面分割器,带视频环出",
        .trignum        =       2,              //int     trignum;              //#设备最高报警输入数
        .outnum         =       2,              //           outnum;         //输出端子数
        .com            =       2,              //int     com;                  //#设备串口数不需要在界面上设置
        .quad           =       0,              //int     quad;                 //#是否有画面分割器，1表示有 0表示没有
        .osd            =       0,                                                      //有osd
        .videonum       =       1,              //int     videonum;             //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum	=   	1,              //int     videoencnum;    //#视频编码器数量
        .hqencnum      	=       1,
        .use_cpu_iic  	=       1,                //int use_cpu_iic     不使用cpuiic总线控制视频ad转换芯片
        .ide            =       1,              //int     ide;                  //#1表示有cf卡或硬盘 0表示没有
        .audionum      =   1,             //int audio
        .eth_port       =       1,              //int     eth_port;             //#网口数 1表示一个 2表示两个
        .list_num       =       sizeof(GTVS1011_List)/sizeof(DevType_T*),
        .list           =       (DevType_T**)GTVS1011_List        //子设备列表,子设备与GTVS1011的相同
};
//GT1011-N+
static DevType_T *GTVS1011_N_List[]=
{	//GT1024 子设备列表
	&UART0,
	&UART1,
	&Ime6410VEnc0,
	&IDEDisk,
	&NetPort0,
	&NetPort1,
};
static GTSeriesDVSR     DEV_GTVS1011_N={
        .type		=	T_GTVS1011_N,      //int     type;      //#设备型号
	 .comment	=	"一个视频压缩芯片,不带画面分割器,两个网口",
	 .trignum		=	6,              //int     trignum;        	//#设备最高报警输入数
	 .outnum		=	4,		   //		outnum;		//输出端子数
        .com		=	2,              //int     com;            	//#设备串口数不需要在界面上设置
        .quad		=	0,              //int     quad;           	//#是否有画面分割器，1表示有 0表示没有
	 .osd		=	0,							//有osd
        .videonum	=	1,              //int     videonum;       	//#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum=	1,              //int     videoencnum;    //#视频编码器数量
	 .hqencnum	=	1,			
	 .use_cpu_iic  =	1,		  //int	use_cpu_iic	不使用cpuiic总线控制视频ad转换芯片
        .ide			=	1,              //int     ide;            	//#1表示有cf卡或硬盘 0表示没有
        .audionum      =   1,             //int audio
        .eth_port	=	2,              //int     eth_port;       	//#网口数 1表示一个 2表示两个
        .list_num	=	sizeof(GTVS1011_N_List)/sizeof(DevType_T*),
        .list			=	(DevType_T**)GTVS1011_N_List        //DevType_T *List[];      //子设备列表
};
static GTSeriesDVSR     DEV_GTVS1011_AN={
        .type		=	T_GTVS1011_AN,      //int     type;      //#设备型号
	 .comment	=	"比GTVS1011-N+多了音视频放大器",
        .trignum		=	6,              //int     trignum;        	//#设备最高报警输入数
	 .outnum		=	4,		   //		outnum;		//输出端子数
        .com		=	2,              //int     com;            	//#设备串口数不需要在界面上设置
        .quad		=	0,              //int     quad;           	//#是否有画面分割器，1表示有 0表示没有
	 .osd		=	0,							//有osd
        .videonum	=	1,              //int     videonum;       	//#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum=	1,              //int     videoencnum;    //#视频编码器数量
	 .hqencnum	=	1,			
	 .use_cpu_iic  =	1,		  //int	use_cpu_iic	不使用cpuiic总线控制视频ad转换芯片
        .ide			=	1,              //int     ide;            	//#1表示有cf卡或硬盘 0表示没有
        .audionum      =   1,             //int audio
        .eth_port	=	2,              //int     eth_port;       	//#网口数 1表示一个 2表示两个
        .list_num	=	sizeof(GTVS1011_N_List)/sizeof(DevType_T*),
        .list			=	(DevType_T**)GTVS1011_N_List        //DevType_T *List[];      //子设备列表
};

//GT1014
static DevType_T *GTVS1014_O_List[]=
{	//GT1024 子设备列表
	&UART0,
	&UART1,
	&QuadDev,
	&Ime6410VEnc0,
	&IDEDisk,
	&NetPort0
};
static GTSeriesDVSR     DEV_GTVS1014_O={
        .type		=	T_GTVS1014_O,      //int     type;      //#设备型号
	 .comment	=	"一个视频压缩芯片,带画面分割器",
        .trignum		=	6,              //int     trignum;        	//#设备最高报警输入数
	 .outnum		=	4,		   //		outnum;		//输出端子数
        .com		=	2,              //int     com;            	//#设备串口数不需要在界面上设置
        .quad		=	1,              //int     quad;           	//#是否有画面分割器，1表示有 0表示没有
	 .osd		=	1,							//有osd
        .videonum	=	4,              //int     videonum;       	//#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum=	1,              //int     videoencnum;    //#视频编码器数量
	 .hqencnum	=	1,			
	 .use_cpu_iic  =	0,		  //int	use_cpu_iic	不使用cpuiic总线控制视频ad转换芯片
	 .ide			=	1,              //int     ide;            	//#1表示有cf卡或硬盘 0表示没有
        .audionum      =   1,             //int audio
        .eth_port	=	1,              //int     eth_port;       	//#网口数 1表示一个 2表示两个
        .list_num	=	sizeof(GTVS1014_O_List)/sizeof(DevType_T*),
        .list			=	(DevType_T**)GTVS1014_O_List        //DevType_T *List[];      //子设备列表
};
static GTSeriesDVSR     DEV_GTVS1014_OA={
        .type		=	T_GTVS1014_OA,      //int     type;      //#设备型号
	 .comment	=	"比GTVS1014-O多了音视频放大器",
	 .trignum		=	6,              //int     trignum;        	//#设备最高报警输入数
	 .outnum		=	4,		   //		outnum;		//输出端子数
        .com		=	2,              //int     com;            	//#设备串口数不需要在界面上设置
        .quad		=	1,              //int     quad;           	//#是否有画面分割器，1表示有 0表示没有
	 .osd		=	1,							//有osd
        .videonum	=	4,              //int     videonum;       	//#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum=	1,              //int     videoencnum;    //#视频编码器数量
	 .hqencnum	=	1,			
	 .use_cpu_iic  =	0,		  //int	use_cpu_iic	不使用cpuiic总线控制视频ad转换芯片
        .ide			=	1,              //int     ide;            	//#1表示有cf卡或硬盘 0表示没有
        .audionum      =   1,             //int audio
        .eth_port	=	1,              //int     eth_port;       	//#网口数 1表示一个 2表示两个
        .list_num	=	sizeof(GTVS1014_O_List)/sizeof(DevType_T*),
        .list			=	(DevType_T**)GTVS1014_O_List        //DevType_T *List[];      //子设备列表
};

//GT1000
#define GT1000_List	GTVS1024_O_List
static GTSeriesDVSR     DEV_GT1000={
        .type		=	T_GT1000,  //int     type;           //#设备型号
	 .comment	=	"老型号的设备",
        .trignum		=	6,               //int     trignum;        //#设备最高报警输入数
 	 .outnum		=	4,		   //		outnum;		//输出端子数
        .com		=	2,              //int     com;            //#设备串口数不需要在界面上设置
        .quad		=	1,              //int     quad;           //#是否有画面分割器，1表示有 0表示没有
	 .osd		=	1,
        .videonum	=	4,              //int     videonum;       //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum=	2,              //int     videoencnum;    //#视频编码器数量
	 .hqencnum	=	1,
	 .use_cpu_iic  =	0,		  //int	use_cpu_iic	不使用cpuiic总线控制视频ad转换芯片
        .ide			=	1,              //int     ide;            //#1表示有cf卡或硬盘 0表示没有
        .audionum      =   1,             //int audio
        .eth_port	=	1,              //int     eth_port;       //#网口数 1表示一个 2表示两个
        .list_num	=	sizeof(GT1000_List)/sizeof(DevType_T*),
	 .list			=	(DevType_T**)GT1000_List	//DevType_T *List[];      //子设备列表
};
//gtvs1021
static DevType_T *GTVS1021_List[]=
{	//GT1021 子设备列表
	&UART0,
	&UART1,
	&Ime6410VEnc0,
	&Ime6410VEnc1,
	&IDEDisk,
	&NetPort0
};

static GTSeriesDVSR     DEV_GTVS1021={
        .type		=	T_GTVS1021,      //int     type;      //#设备型号
	 .comment	=	"两个视频压缩芯片,不带画面分割器",
        .trignum		=	6,              //int     trignum;        	//#设备最高报警输入数
	 .outnum		=	4,		   //		outnum;		//输出端子数
        .com		=	2,              //int     com;            	//#设备串口数不需要在界面上设置
        .quad		=	0,              //int     quad;           	//#是否有画面分割器，1表示有 0表示没有
	 .osd		=	1,							//有osd
        .videonum	=	1,              //int     videonum;       	//#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum=	2,              //int     videoencnum;    //#视频编码器数量
	 .hqencnum	=	1,			
	 .use_cpu_iic  =	1,		  //int	use_cpu_iic	不使用cpuiic总线控制视频ad转换芯片
        .ide			=	1,              //int     ide;            	//#1表示有cf卡或硬盘 0表示没有
        .audionum      =   1,             //int audio
        .eth_port	=	1,              //int     eth_port;       	//#网口数 1表示一个 2表示两个
        .list_num	=	sizeof(GTVS1021_List)/sizeof(DevType_T*),
        .list			=	(DevType_T**)GTVS1021_List        //DevType_T *List[];      //子设备列表
};

//GTVM3001

static DevType_T *GTVM3001_List[]=
{       //GTVM 子设备列表
        &NetPort0,
        &NetPort1
};

static GTSeriesDVSR     DEV_GTVM3001={
        .type           =       T_GTVM3001,      //int     type;      //#设备型号
         .comment       =       "两个网口，无其他子设备",
        .trignum                =       0,              //int     trignum;              //#设备最高报警输入数
         .outnum                =       2,                 //           outnum;         //输出端子数
        .com            =       2,              //int     com;                  //#设备串口数不需要在界面上设置
        .quad           =       0,              //int     quad;                 //#是否有画面分割器，1表示有 0表示没有
         .osd           =       0,                                                      //有osd
        .videonum       =       0,              //int     videonum;             //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum=   0,              //int     videoencnum;    //#视频编码器数量
         .hqencnum      =       0,
         .use_cpu_iic  =        0,                //int use_cpu_iic     不使用cpuiic总线控制视频ad转换芯片
        .ide                    =       0,        //int     ide;                //#1表示有cf卡或硬盘 0表示没有
        .audionum     =   0,                    //int audio
        .eth_port       =       2,              //int     eth_port;             //#网口数 1表示一个 2表示两个
        .list_num       =       sizeof(GTVM3001_List)/sizeof(DevType_T*),
        .list                   =       (DevType_T**)GTVM3001_List        //DevType_T *List[];      //子设备列表
};

//GTVM3111
static DevType_T *GTVM3111_List[]=
{       //GTVM 子设备列表
        &NetPort0,
        &NetPort1,
        &UART0,
		&UART1,
		&Ime6410VEnc0
};

static GTSeriesDVSR     DEV_GTVM3111={
        .type           =       T_GTVM3111,      //int     type;      //#设备型号
         .comment       =       "两个网口，两个串口，一片6410",
        .trignum                =       6,              //int     trignum;              //#设备最高报警输入数
         .outnum                =       4,                 //           outnum;         //输出端子数
        .com            =       2,              //int     com;                  //#设备串口数不需要在界面上设置
        .quad           =       0,              //int     quad;                 //#是否有画面分割器，1表示有 0表示没有
         .osd           =       0,                                                      //有osd
        .videonum       =       1,              //int     videonum;             //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum=   1,              //int     videoencnum;    //#视频编码器数量
         .hqencnum      =       0,
         .use_cpu_iic  =        1,                //int use_cpu_iic     不使用cpuiic总线控制视频ad转换芯片
        .ide                    =       0,        //int     ide;                //#1表示有cf卡或硬盘 0表示没有
        .audionum     =   0,                    //int audio
        .eth_port       =       2,              //int     eth_port;             //#网口数 1表示一个 2表示两个
        .list_num       =       sizeof(GTVM3111_List)/sizeof(DevType_T*),
        .list                   =       (DevType_T**)GTVM3111_List        //DevType_T *List[];      //子设备列表
};



/***************************************************************************************************************/
#endif

