#ifndef DEV_TYPE_H
#define DEV_TYPE_H
#include <devinfo.h>
#include "subdevdef.h"
typedef struct{//GT系列DVSR描述定义
        int     type;           //#设备型号
        char *comment;  //对于该型号的说明
        int     trignum;        //#设备最高报警输入数
        int     outnum;         //输出端子数
        int     com;            //#设备串口数不需要在界面上设置
        int     quad;           //#是否有画面分割器，1表示有 0表示没有
        int     osd;            //是否有osd,1表示有,0表示没有
        int     videonum;       //#系统最多视频输入数(和移动侦测有关),在quad=0时videonum应该为1 
	 int     videoencnum;    //#视频编码器数量
        int     hqencnum;       //录像的通道数
	 int	   use_cpu_iic;		//使用cpu的iic管脚控制tw9903视频编码器
	 int     ide;            //1表示cf卡或硬盘的数目 0表示没有,2表示有SD卡,3表示TF卡
	 	int 	audionum;			//#表示声音通道的个数，0表示没有
        int     eth_port;       //#网口数 1表示一个 2表示两个
        int     list_num;       //子设备列表中的有效子设备数
        DevType_T **list;       //子设备列表
}GTSeriesDVSR;

GTSeriesDVSR *get_dvsr_by_typestr(char *dev_type_str);
int conv_dev_str2type(char *type_str);
char *conv_dev_type2str(int type);


#endif
