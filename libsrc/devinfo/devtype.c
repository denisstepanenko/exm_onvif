#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <devinfo.h>
#include "devtype.h"
typedef struct {//
	int 	code;	//设备型号代码
	char	*name;	//设备型号字符串
}dev_name_t;
#include "devtype_gtvs1000.h"		///GTVS1000系列产品的定义
#include "devtype_gtvs3000.h"		///GTVS3000系列产品的定义

static dev_name_t support_lists[]=
{//支持的设备型号名称列表
 //设备型号代码与名称转换时用到
	{T_GT1000,T_GT1000_STR},
	{T_GTVS1024_O,T_GTVS1024_O_STR},
	{T_GTVS1024_OA,T_GTVS1024_OA_STR},
	{T_GTVS1011,T_GTVS1011_STR},
	{T_GTVS1011_A,T_GTVS1011_A_STR},
	{T_GTVS1011_O,T_GTVS1011_O_STR},
	{T_GTVS1011_OA,T_GTVS1011_OA_STR},
	{T_GTVS1011_N,T_GTVS1011_N_STR},
	{T_GTVS1011_AN,T_GTVS1011_AN_STR},
	{T_GTVS1014_O,T_GTVS1014_O_STR},
	{T_GTVS1014_OA,T_GTVS1014_OA_STR},
	{T_GTVS1021,T_GTVS1021_STR},
	{T_GT1001,T_GT1001_STR},

 	{T_GTVM3001,T_GTVM3001_STR},
	{T_GTVM3111,T_GTVM3111_STR},
 	
       {T_GTVS3021,T_GTVS3021_STR}, 
       {T_GTVS3022,T_GTVS3022_STR}, 
       {T_GTVS3024,T_GTVS3024_STR}, 
       {T_GTVS3021L,T_GTVS3021L_STR},
//       {T_GTVS3121,T_GTVS3121_STR},
       {T_GTMV3121,T_GTMV3121_STR},
 		{T_GTVS3024L,T_GTVS3024L_STR},
 	{T_GTVS3124,T_GTVS3124_STR},
	{T_GTMV3122,T_GTMV3122_STR},
	{T_GTIP1004,T_GTIP1004_STR},
	{T_GTIP2004,T_GTIP2004_STR}

};


static GTSeriesDVSR	*CurDVSR=NULL;	//当前的设备信息
static GTSeriesDVSR	*DEVSupportList[]={//目前支持的型号列表
	&DEV_GT1000,
	&DEV_GTVS1024_O,
	&DEV_GTVS1024_OA,
	&DEV_GTVS1011,
	&DEV_GTVS1011_A,
	&DEV_GT1001,
	&DEV_GTVS1011_N,
	&DEV_GTVS1011_AN,
	&DEV_GTVS1014_O,
	&DEV_GTVS1014_OA,
	&DEV_GTVS1021,
	&DEV_GTVM3001,
	&DEV_GTVM3111,

       &DEV_GTVS3021,
       &DEV_GTVS3022,
       &DEV_GTVS3024,
       &DEV_GTVS3024L,
       &DEV_GTVS3021L,
       &DEV_GTMV3121,
       &DEV_GTVS3124,
       &DEV_GTMV3122,
       & DEV_GTIP1004,
	&DEV_GTIP2004
};

static int total_devs=sizeof(DEVSupportList)/sizeof(GTSeriesDVSR*);//函数库支持的设备型号总数
//////////////////////////////////////////////////////////////////////////////////////////////////////
char *conv_dev_type2str(int type)
{
	dev_name_t *dev=NULL;
	int total=sizeof(support_lists)/sizeof(dev_name_t);
	int i;
	for(i=0;i<total;i++)
	{
		dev=&support_lists[i];
		if(dev->code==type)
			return dev->name;
	}
	return T_GT_UNKNOW_STR;
#if 0
	switch(type)
	{
			
		case T_GT1000:       //老版本的视频服务器统称GT1000
			return T_GT1000_STR;
		case T_GTVS1024_O:     //视频2个编码器4路视频输入，带画面分割器
			return T_GTVS1024_O_STR;
		case T_GTVS1024_OA:    //比GT1024-O多了音频放大器
			return T_GTVS1024_OA_STR;
		case T_GTVS1011:
			return T_GTVS1011_STR;
		break;
		case T_GTVS1011_A:
			return T_GTVS1011_A_STR;
		break;
		case T_GTVS1011_O:         // 1个视频编码器带画面分割器
			return T_GTVS1011_O_STR;
		case T_GTVS1011_OA:    //比GT1011-O多了音频放大器
			return T_GTVS1011_OA_STR;
		case T_GTVS1011_N:     //一个视频编码器，不带画面分割器，带2个网口
			return T_GTVS1011_N_STR;
		case T_GTVS1011_AN:    //比GT1011-N+多了音频放大器
			return T_GTVS1011_AN_STR;
		break;
		case T_GTVS1014_O:
			return T_GTVS1014_O_STR;
		break;
		case T_GTVS1014_OA:
			return T_GTVS1014_OA_STR;
		break;
		case T_GTVS1021:
			return T_GTVS1021_STR;
		break;
		default:
			return T_GT_UNKNOW_STR;
		break;
	}
#endif
}
//根据字符串获取设备类型代码
int conv_dev_str2type(char *type_str)
{
        dev_name_t *dev=NULL;
        int total=sizeof(support_lists)/sizeof(dev_name_t);
	int i;
	if(type_str==NULL)
		return -EINVAL;
	for(i=0;i<total;i++)
	{
		dev=&support_lists[i];
		if(strcasecmp(type_str,dev->name)==0)
			return dev->code;
	}
	return T_GT1000;//不认识的型号则认为是gt1000 T_GT_UNKNOW;
}



//通过设备型号字符串返回 对应的设备描述结构指针
//返回NULL表示没有找到
GTSeriesDVSR *get_dvsr_by_typestr(char *dev_type_str)
{
	int i;
	int type;
	GTSeriesDVSR *dev=NULL;
	if(dev_type_str==NULL)
		return NULL;

	type=conv_dev_str2type(dev_type_str);
	printf("yk debug:dev type:%d\n",type);
	for(i=0;i<sizeof(DEVSupportList)/sizeof(GTSeriesDVSR*);i++)
	{
		dev=DEVSupportList[i];
		if(dev->type==type)
			return dev;
	}
	return NULL;
}
//打印子设备信息
static void print_sub_dev_list(DevType_T *List[],int total)
{
        int i;
        DevType_T *Dev=NULL;
        for(i=0;i<total;i++)
        {
                Dev=List[i];
                printf("%d %12s %15s %25s\n",Dev->type,Dev->name,Dev->node,Dev->driver);
        }
        printf("\n");
}
//打印设备信息
static void print_device_info(GTSeriesDVSR *dev)
{
	if(dev==NULL)
		return;

	printf("设备型号\t:%s\n",conv_dev_type2str(dev->type));
	if(dev->comment!=NULL)
		printf("型号说明\t:%s\n",dev->comment);
	printf("报警输入端口数\t:%d\n",dev->trignum);
	printf("设备串口数\t:%d\n",dev->com);
	printf("画面分割器\t:%s\n",dev->quad?"有":"无");
	printf("视频输入通道\t:%d\n",dev->videonum);
	printf("视频编码器数量\t:%d\n",dev->videoencnum);
	printf("存储设备\t:%s\n",dev->ide?"有":"无");
	printf("网口数\t\t:%d\n",dev->eth_port);
	printf("声音通道数\t:%d\n",dev->audionum);
	if(dev->list!=NULL)
	{
		printf("			子设备列表\n");
		printf("类型      名称          节点                       驱动\n");
		print_sub_dev_list(dev->list,dev->list_num);		
	}	
}
//打印支持的设备信息
void print_support_deivce_list(void)
{
	int i;
	printf("目前支持%d种型号:\n",total_devs);
	for(i=0;i<total_devs;i++)
	{
		printf("%s  ",conv_dev_type2str(DEVSupportList[i]->type));
	}
	printf("\n");
	printf("每种型号的详细信息:\n");
	for(i=0;i<total_devs;i++)
	{
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		print_device_info(DEVSupportList[i]);
		printf("________________________________________________________________\n");

		printf("\n");
	}	
	printf("\n");
}
#if 0
void print_list(DevType_T *List[],int total)
{
        int i;
        DevType_T *Dev=NULL;
        for(i=0;i<total;i++)
        {
                Dev=List[i];
                printf("%d %12s %15s %25s\n",Dev->type,Dev->name,Dev->node,Dev->driver);
        }
        printf("\n");
}

void PrintDevInfo(GTSeriesDVSR *Dev)
{
	if(Dev==NULL)
		return;
	printf("设备型号\t:%s\n",ConvDevtype2Str(Dev->type));
	printf("报警输入端口数\t:%d\n",Dev->trignum);
	printf("设备串口数\t:%d\n",Dev->com);
	printf("画面分割器\t:%s\n",Dev->quad?"有":"无");
	printf("视频输入通道\t:%d\n",Dev->videonum);
	printf("视频编码器数量\t:%d\n",Dev->videoencnum);
	printf("存储设备\t:%s\n",Dev->ide?"有":"无");
	printf("网口数\t\t:%d\n",Dev->eth_port);
	

	if(Dev->List!=NULL)
	{
		printf("**********************子设备列表********************************\n");
		printf("类型      名称          节点                       驱动\n");
		print_list(Dev->List,Dev->list_num);		
		printf("****************************************************************\n");
	}	
}
void PrintSupportDev(GTSeriesDVSR     *Devs[],int total)
{
        int i;
        GTSeriesDVSR *Dev=NULL;
        for(i=0;i<total;i++)
        {
                Dev=Devs[i];
		printf("-----------------------------------------------------------------\n");
                printf("-=%s=-\n",ConvDevtype2Str(Dev->type));
		PrintDevInfo(Dev);
		printf("=================================================================\n");
        }
}

int main(void)
{
	PrintSupportDev(DEVSupportList,sizeof(DEVSupportList)/sizeof(GTSeriesDVSR*));
	//PrintDevInfo(&DEV_GT1000);
	return 0;
}


#endif
