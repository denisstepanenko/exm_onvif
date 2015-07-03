///GTVS3000系列视频服务器型号定义
#ifndef GTVS3000_DEV_TYPE_H
#define GTVS3000_DEV_TYPE_H
//只能被devtype.c在包含了 include <devinfo.h>和定义了dev_name_t 后包含
/************************具体的设备型号的子设备列表*************************************************************/
/*
    定义设备的子设备列表时，同类型的多个设备一定要按照顺序定义，如Ime6410VEnc0一定要在Ime6410VEnc1之前
    定义一个新的设备型号时需要定义新型号的static DevType_T 结构数组和static GTSeriesDVSR结构资源定义，然后
    把static GTSeriesDVSR的指针加入DEVSupportList即可
*/

//GTVS3021
/*****************************************************************
 * 设备名称:    GTVS3021
 * ***************************************************************/
static DevType_T *GTVS3021_List[]=
{   //GTVS3021 子设备列表
    &UART0,
    &UART1,
    &IDEDisk,
    &NetPort0
};

static GTSeriesDVSR     DEV_GTVS3021={
        .type           =       T_GTVS3021,      //int     type;      //#设备型号
        .comment        =       "双码流,单视频输入,带osd",
        .trignum        =       8,              //int     trignum;          //#设备最高报警输入数
        .outnum         =       4,         //       outnum;     //输出端子数
        .com            =       2,              //int     com;              //#设备串口数不需要在界面上设置
        .quad           =       0,              //int     quad;             //#是否有画面分割器，1表示有 0表示没有
        .osd            =       1,                          //有osd
        .videonum       =       1,              //int     videonum;         //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum    =       2,              //int     videoencnum;    //#视频编码器数量
        .hqencnum       =       1,          
        .use_cpu_iic    =       1,        //int use_cpu_iic 不使用cpuiic总线控制视频ad转换芯片
        .ide            =       1,              //int     ide;   //#1表示有cf卡或硬盘 0表示没有
        .audionum       =       1,             //int audio
        .eth_port       =       1,              //int     eth_port;         //#网口数 1表示一个 2表示两个
        .list_num       =       sizeof(GTVS3021_List)/sizeof(DevType_T*),
        .list           =       (DevType_T**)GTVS3021_List        //DevType_T *List[];      //子设备列表
};


/*******************************************************************
 * 设备名称:    GTVS3021L
 * *****************************************************************/
static DevType_T *GTVS3021L_List[]=
{       //GTVS3021L 子设备列表
        &UART0,
        &UART1,
        &IDEDisk,
        &NetPort0
};

static GTSeriesDVSR     DEV_GTVS3021L={
        .type           =       T_GTVS3021L,      //int     type;      //#设备型号
        .comment        =       "双码流,单视频输入,带osd,低功耗",
        .trignum        =       0,              //int     trignum;              //#设备最高报警输入数
        .outnum         =       0,                 //           outnum;         //输出端子数
        .com            =       2,              //int     com;                  //#设备串口数不需要在界面上设置
        .quad           =       0,              //int     quad;                 //#是否有画面分割器，1表示有 0表示没有
        .osd            =       1,                                                      //有osd
        .videonum       =       1,              //int     videonum;             //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为
        .videoencnum    =       2,              //int     videoencnum;    //#视频编码器数量
        .hqencnum       =       1,
        .use_cpu_iic    =       1,                //int use_cpu_iic     不使用cpuiic总线控制视频ad转换芯片
        .ide            =       2,              //int     ide;   //#1表示有cf卡或硬盘 0表示没有
        .audionum       =       1,             //int audio
        .eth_port       =       1,              //int     eth_port;             //#网口数 1表示一个 2表示两个
        .list_num       =       sizeof(GTVS3021L_List)/sizeof(DevType_T*),
        .list           =       (DevType_T**)GTVS3021L_List        //DevType_T *List[];      //子设备列表

};

/**************************************************************
 * 设备名称:    GTMV3121
 * ***********************************************************/
static DevType_T *GTMV3121_List[]=
{       //GTVS3121 子设备列表
        &UART0,
        &UART1,
        &IDEDisk,
        &NetPort0
};

static GTSeriesDVSR     DEV_GTMV3121={

        .type           =       T_GTMV3121, //int     type;      //#设备型号
        .comment        =       "双码流,单视频输入,带osd,低功耗",
        .trignum        =       0,              //int     trignum;              //#设备最高报警输入数
        .outnum         =       0,              //           outnum;         //输出端子数
        .com            =       2,              //int     com;                  //#设备串口数不需要在界面上设置
        .quad           =       0,              //int     quad;                 //#是否有画面分割器，1表示有 0表示没有
        .osd            =       1,                                                      //有osd
        .videonum       =       1,              //int     videonum;             //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为
        .videoencnum    =       2,              //int     videoencnum;    //#视频编码器数量
        .hqencnum       =       1,
        .use_cpu_iic    =       1,              //int use_cpu_iic     不使用cpuiic总线控制视频ad转换芯片
        .ide            =       3,              //int     ide;   //#1表示有cf卡或硬盘 0表示没有,2-->sd,3-->tf
        .audionum       =       0,              //int audio
        .eth_port       =       1,              //int     eth_port;             //#网口数 1表示一个 2表示两个
        .list_num       =       sizeof(GTMV3121_List)/sizeof(DevType_T*),
        .list           =       (DevType_T**)GTMV3121_List        //DevType_T *List[];      //子设备列表
};

/************************************************************
 * 设备名称:    GTVS3022
 * **********************************************************/
static DevType_T *GTVS3022_List[]=

{       //GTVS3022 子设备列表
        &UART0,
        &UART1,
        &IDEDisk,
        &NetPort0
};

static GTSeriesDVSR     DEV_GTVS3022={
        .type           =       T_GTVS3022,      //int     type;      //#设备型号
        .comment        =       "两路单码流，双视频输入,带osd,目前只支持一个摄像头输入",
        .trignum        =       8,              //int     trignum;              //#设备最高报警输入数
        .outnum         =       4,              //           outnum;         //输出端子数
        .com            =       2,              //int     com;                  //#设备串口数不需要在界面上设置
        .quad           =       0,              //int     quad;                 //#是否有画面分割器，1表示有 0表示没有
        .osd            =       1,                                                      //有osd
        .videonum       =       2,                    // 1,              //int     videonum;             //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该1 
        .videoencnum    =       2,              //int     videoencnum;    //#视频编码器数量
        .hqencnum       =       2,                    //    1,
        .use_cpu_iic    =       0,              //int use_cpu_iic     不使用cpuiic总线控制视频ad转换芯片
        .ide            =       1,              //int     ide;   //#1表示有cf卡或硬盘 0表示没有
        .audionum       =       1,              //int audio
        .eth_port       =       1,              //int     eth_port;             //#网口数 1表示一个 2表示两个
        .list_num       =       sizeof(GTVS3022_List)/sizeof(DevType_T*),
        .list           =       (DevType_T**)GTVS3022_List        //DevType_T *List[];      //子设备列表
};


/*************************************************
 *设备名称: GTMV3122
 *************************************************/
static DevType_T *GTMV3122_List[]=

{       //GTMV3122 子设备列表
        &UART0,
        &UART1,
        &IDEDisk,
        &NetPort0
};

static GTSeriesDVSR     DEV_GTMV3122={
        .type           =       T_GTMV3122,      //int     type;      //#设备型号
        .comment        =       "两路单码流，双视频输入,带osd,目前只支持一个摄像头输入",
        .trignum        =       0,              //int     trignum;              //#设备最高报警输入数
        .outnum         =       0,                 //           outnum;         //输出端子数
        .com            =       2,              //int     com;                  //#设备串口数不需要在界面上设置
        .quad           =       0,              //int     quad;                 //#是否有画面分割器，1表示有 0表示没有
        .osd            =       1,                                                      //有osd
        .videonum       =       2,                     // 1,              //int     videonum;             //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该1 
        .videoencnum    =       2,              //int     videoencnum;    //#视频编码器数量
        .hqencnum       =       2,                    //    1,
        .use_cpu_iic    =       0,              //int use_cpu_iic     不使用cpuiic总线控制视频ad转换芯片
        .ide            =       2,              //int     ide;   //#1表示有cf卡或硬盘 0表示没有 2表示SD卡，3表示TF卡
        .audionum       =       1,              //int audio
        .eth_port       =       1,              //int     eth_port;             //#网口数 1表示一个 2表示两个
        .list_num       =       sizeof(GTMV3122_List)/sizeof(DevType_T*),
        .list           =       (DevType_T**)GTMV3122_List        //DevType_T *List[];      //子设备列表
};

/****************************************************
 *设备名称: GTIP1004
 ****************************************************/
 static DevType_T *GTIP1004_List[]=
{
    //GTIP1004 子设备列表
    //lc to do  
    &IPUART0,
    &IPUART0,
    &NetPort0
};

static GTSeriesDVSR     DEV_GTIP1004={
        .type           =       T_GTIP1004,      //int     type;            //#设备型号
        .comment        =       "单码流,4视频输入,带osd",
        .trignum        =       8,              //int     trignum;          //#设备最高报警输入数
        .outnum         =       4,      //      outnum;     //输出端子数
        .com            =       2,              //int     com;              //#设备串口数不需要在界面上设置
        .quad           =       1,              //int     quad;             //#是否有画面分割器，1表示有 0表示没有
        .osd            =       1,                      //有osd
        .videonum       =       4,              //int     videonum;         //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum    =       5,              //int     videoencnum;        //#视频编码器数量
        .hqencnum       =       1,          
        .use_cpu_iic    =       0,      //int   use_cpu_iic     不使用cpuiic总线控制视频ad转换芯片
        .ide            =       1,              //int     ide;          //#1表示有cf卡或硬盘 0表示没有
        .audionum       =       4,              //int audio
        .eth_port       =       1,              //int     eth_port;         //#网口数 1表示一个 2表示两个
        .list_num       =       sizeof(GTIP1004_List)/sizeof(DevType_T*),
        .list           =       (DevType_T**)GTIP1004_List        //DevType_T *List[];      //子设备列表
};

/****************************************************
 *设备名称: GTIP2004
 ****************************************************/
 static DevType_T *GTIP2004_List[]=
{
    //GTIP2004 子设备列表
    //lc to do  
    &IPUART0,
    &IPUART0,
    &NetPort0
};

static GTSeriesDVSR     DEV_GTIP2004={
        .type           =       T_GTIP2004,      //int     type;            //#设备型号
        .comment        =       "单码流,4视频输入,带osd",
        .trignum        =       16,              //int     trignum;          //#设备最高报警输入数
        .outnum         =       4,      //      outnum;     //输出端子数
        .com            =       2,              //int     com;              //#设备串口数不需要在界面上设置
        .quad           =       1,              //int     quad;             //#是否有画面分割器，1表示有 0表示没有
        .osd            =       1,                      //有osd
        .videonum       =       4,              //int     videonum;         //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum    =       5,              //int     videoencnum;        //#视频编码器数量
        .hqencnum       =       1,          
        .use_cpu_iic    =       0,      //int   use_cpu_iic     不使用cpuiic总线控制视频ad转换芯片
        .ide            =       1,              //int     ide;          //#1表示有cf卡或硬盘 0表示没有
        .audionum       =       4,              //int audio
        .eth_port       =       1,              //int     eth_port;         //#网口数 1表示一个 2表示两个
        .list_num       =       sizeof(GTIP2004_List)/sizeof(DevType_T*),
        .list           =       (DevType_T**)GTIP2004_List        //DevType_T *List[];      //子设备列表
};


//GTVS3024
/******************************************************
 * 设备名称:    GTVS3024
 ******************************************************/
static DevType_T *GTVS3024_List[]=
{   //GT1024 子设备列表
    		&QuadDev_2835,
        &UART0,
        &UART1,
        &IDEDisk,
        &NetPort0
};

static GTSeriesDVSR     DEV_GTVS3024={
        .type           =       T_GTVS3024,      //int     type;            //#设备型号
        .comment        =       "双码流,4视频输入,带osd",
        .trignum        =       8,              //int     trignum;          //#设备最高报警输入数
        .outnum         =       4,      //      outnum;     //输出端子数
        .com            =       2,              //int     com;              //#设备串口数不需要在界面上设置
        .quad           =       1,              //int     quad;             //#是否有画面分割器，1表示有 0表示没有
        .osd            =       1,                      //有osd
        .videonum       =       4,              //int     videonum;         //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
        .videoencnum    =       2,              //int     videoencnum;        //#视频编码器数量
        .hqencnum       =       1,          
        .use_cpu_iic    =       0,      //int   use_cpu_iic     不使用cpuiic总线控制视频ad转换芯片
        .ide            =       1,              //int     ide;          //#1表示有cf卡或硬盘 0表示没有
        .audionum       =       1,              //int audio
        .eth_port       =       1,              //int     eth_port;         //#网口数 1表示一个 2表示两个
        .list_num       =       sizeof(GTVS3024_List)/sizeof(DevType_T*),
        .list           =       (DevType_T**)GTVS3024_List        //DevType_T *List[];      //子设备列表
};

/*******************************************************
 * 设备名称:    GTVS3024L
 * *****************************************************/
static DevType_T *GTVS3024L_List[]=
{       //GTVS3024L 子设备列表
        &QuadDev_2835,
        &NetPort0
};
static GTSeriesDVSR     DEV_GTVS3024L={
        .type           =       T_GTVS3024L,      //int     type;      //#设备型号
        .comment        =       "双码流,四视频输入,带osd,低功耗",
        .trignum        =       6,              //int     trignum;              //#设备最高报警输入数
        .outnum         =       4,                 //           outnum;         //输出端子数
        .com            =       2,              //int     com;                  //#设备串口数不需要在界面上设置
        .quad           =       1,              //int     quad;                 //#是否有画面分割器，1表示有 0表示没有
        .osd            =       1,                                                      //有osd
        .videonum       =       4,              //int     videonum;             //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为
        .videoencnum    =       2,              //int     videoencnum;    //#视频编码器数量
        .hqencnum       =       1,
        .use_cpu_iic    =       0,                //int use_cpu_iic     不使用cpuiic总线控制视频ad转换芯片
        .ide            =       2,              //int     ide;   //#1表示有cf卡或硬盘 0表示没有
        .audionum       =       0,             //int audio
        .eth_port       =       1,              //int     eth_port;             //#网口数 1表示一个 2表示两个
        .list_num       =       sizeof(GTVS3024L_List)/sizeof(DevType_T*),
        .list           =       (DevType_T**)GTVS3024L_List        //DevType_T *List[];      //子设备列表

};


/**************************************************************
 * 设备名称:    GTVS3124
 * ************************************************************/
static DevType_T *GTVS3124_List[]=
{       //GTVS3124 子设备列表
    &QuadDev_2835,
    &NetPort0
};
static GTSeriesDVSR     DEV_GTVS3124={
        .type           =       T_GTVS3124,      //int     type;      //#设备型号
        .comment        =       "双码流,四视频输入,带osd,低功耗",
        .trignum        =       1,              //int     trignum;              //#设备最高报警输入数   lsk 2009-11-6 0->1
        .outnum         =       0,                 //           outnum;         //输出端子数
        .com            =       0,              //int     com;                  //#设备串口数不需要在界面上设置
        .quad           =       1,              //int     quad;                 //#是否有画面分割器，1表示有 0表示没有
        .osd            =       1,                                                      //有osd
        .videonum       =       4,              //int     videonum;             //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为
        .videoencnum    =       2,              //int     videoencnum;    //#视频编码器数量
        .hqencnum       =       1,
        .use_cpu_iic    =       0,                //int use_cpu_iic     不使用cpuiic总线控制视频ad转换芯片
        .ide            =       2,              //int     ide;   //#1表示有cf卡或硬盘 0表示没有
        .audionum       =       0,             //int audio
        .eth_port       =       1,              //int     eth_port;             //#网口数 1表示一个 2表示两个
        .list_num       =       sizeof(GTVS3124_List)/sizeof(DevType_T*),
        .list           =       (DevType_T**)GTVS3124_List        //DevType_T *List[];      //子设备列表

};
#endif

