#ifndef SUBDEVDEF_H
#define SUBDEVDEF_H
#include <devinfo.h>
typedef struct{                 //视频服务器的子设备信息结构
        int             type;           //子设备类型
        int             no;                     //子设备在同类设备中的序号
        char    *name;          //子设备名
        char    *node;          //子设备节点
        char    *driver;                //子设备驱动程序
}DevType_T;
/************************具体的子设备定义**************************************************************/
static DevType_T QuadDev={
		//画面分割器
		.type		=	SUB_DEV_QUAD,
		.no			=	0,
		.name		=	"tw2834",
		.node		=	"/dev/quaddev",
		.driver		=	NULL		   
	};
static DevType_T Ime6410VEnc0={
		//第0路视频编码器
		.type		=	SUB_DEV_VENC,
		.no			=	0,
		.name		=	"ime6410_pcm",
		.node		=	"/dev/IME6410",
		.driver		=	"/gt1000/drivers/ime6410_pcm.o"
	};
static DevType_T Ime6410VEnc1={
		//第1路视频编码器
              .type		=	SUB_DEV_VENC,
				.no		=	1,
              .name		=	"ime6410_d1",
              .node		=	"/dev/IME6410_D1",
              .driver	=	"/gt1000/drivers/ime6410_d1.o"
        };
static DevType_T IDEDisk={
		//磁盘
		.type		=	SUB_DEV_IDE,
		.no			=	0,
		.name		=	"ide",
		.node		=	NULL,
		.driver		=	"/hqdata"
	};
static DevType_T AudioChip={
		//音频编解码芯片
		.type		=	SUB_DEV_ACDC,
		.name		=	"uda1341",
		.no			=	0,
		.node		=	"/dev/dsp",
		.driver		=	NULL
	};
static DevType_T UART0={
		.type		=	SUB_DEV_COM,
		.no		=	0,
		.name		=	"uart0",
		.node		=	"/dev/ttyS1",
		.driver		=	NULL
	};

static DevType_T UART1={
		//串口1
                .type		=	SUB_DEV_COM,
		  		.no			=	1,
                .name		=	"uart1",
                .node		=	"/dev/ttyS2",
                .driver		=	NULL
        };
static DevType_T IPUART0={
		.type		=	SUB_DEV_COM,
		.no		=	0,
		.name		=	"ipuart0",
		.node		=	"/dev/ttyAMA1",
		.driver		=	NULL
	};

static DevType_T IPUART1={
		//串口1
                .type		=	SUB_DEV_COM,
		  		.no			=	1,
                .name		=	"ipuart1",
                .node		=	"/dev/ttyAMA2",
                .driver		=	NULL
        };
static DevType_T NetPort0={
		//网口0
		.type		=	SUB_DEV_NETPORT,
		.no		=	0,
		.name		=	"eth0",
		.node		=	"eth0",
		.driver		=	"NULL"
	};
static DevType_T NetPort1={
		//网口1
                .type		=	SUB_DEV_NETPORT,
		.no		=	1,
                .name		=	"eth1",
                .node		=	"eth1",
                .driver		=	"NULL"
        };

/***************************************************************************************************************/

static DevType_T QuadDev_2835={
		//画面分割器
		.type		=	SUB_DEV_QUAD,
		.no			=	0,
		.name		=	"tw2835",
		.node		=	"/dev/quaddev",
		.driver		=	NULL		   
	};
#endif
