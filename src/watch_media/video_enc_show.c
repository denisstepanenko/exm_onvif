#include "watch_media.h"
#include <mshmpool.h>
#include <VEncApi.h>
#include <calc_bitrate.h>
#include <devinfo.h>

#define MAX_ONVIF_DEVICE    4
MSHM_POOL	VideoEncoder[MAX_ONVIF_DEVICE*2];	//视频编码器缓冲池结构
BIT_RATE_T	VBitRate[MAX_ONVIF_DEVICE*2];		//视频码流结构
BIT_RATE_T	frame_bitrate[MAX_ONVIF_DEVICE*2];	//帧率
int	old_frame_seq[MAX_ONVIF_DEVICE*2];

/*yk add */
inline int get_videoenc_num()
{
	return 4;
}
void InitVEncState(void)
{
	int i,j,total;
    int ret;
	memset(VideoEncoder,0,sizeof(VideoEncoder));
	memset(VBitRate,0,sizeof(VBitRate));
	total=get_video_num();
	for(i=0;i<total;i++)
	{
		j=i*2;
		ret=MSHmPoolAttach(&VideoEncoder[j], get_onvif_pool_key(i,0));
		if(ret!=0)
		{
			printf("connect video encoder%d ret=%d!!\n",j,ret);
			sleep(1);
		}
		VBitRate[j].AvgInterval=2;
		VBitRate[j].LastCheckBytes=0;

		frame_bitrate[j].AvgInterval=2;
		frame_bitrate[j].LastCheckBytes=0;
		old_frame_seq[j]=-1;
	}
	for(i=0;i<total;i++)
	{
		j=i*2+1;
		ret=MSHmPoolAttach(&VideoEncoder[j], get_onvif_pool_key(i,1));
		if(ret!=0)
		{
			printf("connect video encoder%d ret=%d!!\n",j,ret);
			sleep(1);
		}
		VBitRate[j].AvgInterval=2;
		VBitRate[j].LastCheckBytes=0;

		frame_bitrate[j].AvgInterval=2;
		frame_bitrate[j].LastCheckBytes=0;
		old_frame_seq[j]=-1;
	}

}
//显示指定编码器号的信息
//返回1表示编码器存在
static int	ShowEncState(int vno,int No,MSHM_POOL *VEnc)
{
	//int i;
	//int Val;
	ENC_ATTRIB 			*EAttr;
	SHPOOL_HEAD		*Ph;
	BIT_RATE_T			*BR=&VBitRate[No];
	BIT_RATE_T			*FR=&frame_bitrate[No];
	int					*OldSeq=&old_frame_seq[No];
	

	if(VEnc->mc==NULL)
	{//没有被初始化
		//WriteTermStr(C_RED,0,"%02d NE\n",No);
		return 0;
	}
	
	Ph=VEnc->ph;
	EAttr=MShmPoolGetInfo(VEnc);


	CalcBitRate(BR,Ph->send_bytes-BR->LastCheckBytes);	//计算码流
	BR->LastCheckBytes=Ph->send_bytes;				//更新字节信息

	if(*OldSeq>=0)
	{
		CalcBitRate(FR,Ph->num-*OldSeq);		
	}
	else
	{
		CalcBitRate(FR,0);		
	}
	*OldSeq=Ph->num;

	
	WriteTermStr(C_WHITE,0,"%02d  ",vno);				//视频编码器编号

	if(EAttr->State!=1)								//设备状态
		WriteTermStr(C_RED,0,"  %02d",EAttr->State);		//编码器有问题
	else
		WriteTermStr(C_WHITE,0,"  %02d",EAttr->State);	//编码器正常
	WriteTermStr(C_WHITE,0,"%10d ",Ph->num);			//当前序号
	WriteTermStr(C_WHITE,0,"%4d ",(Ph->tail));			//最新的元素
	WriteTermStr(C_WHITE,0,"      %4.1f ",BR->PeakBitrate/1024);	//峰值流量
	WriteTermStr(C_WHITE,0,"\t  %4.1f ",BR->AvgBitrate/1024);	//峰值流量
	WriteTermStr(C_WHITE,0,"    \t%4.1f ",FR->AvgBitrate/8);	//峰值流量
	
	WriteTermStr(C_WHITE,0,"\n");
	return 1;
}
//显示连接到指定编码器的用户信息
static int	ShowEncUsrState(int No,MSHM_POOL *VEnc)
{
	SHPOOL_USR   		*Usr;
	SHPOOL_HEAD		*Ph;
	int	i;
	time_t	CurTime	=time(NULL)	;	//当前时间

	if(VEnc->mc==NULL)
	{//没有初始化			
		return 0;
	}	
	Ph=VEnc->ph;

///用户信息
	for(i=0;i<SHPOOL_MAX_USERS;i++)
	{
		Usr=&Ph->users[i];
		if(Usr->valid)
		{	//有效用户
			//WriteTermStr(C_WHITE,0,"\n\t\t\t");
			WriteTermStr(C_WHITE,0,"%02d  ",No);		//编码器号
			WriteTermStr(C_WHITE,0,"  %02d ",i);			//用户号
			WriteTermStr(C_WHITE,0,"           ");			
			WriteTermStr(C_WHITE,0,"%4d ",Usr->curele);	//当前读取的元素序号
			WriteTermStr(C_WHITE,0,"%12s",Usr->name);//用户名
			WriteTermStr(C_WHITE,0,"%10d ",(int)(CurTime-Usr->stime));
			WriteTermStr(C_WHITE,0,"\n");	
		}		
		Usr++;
	}
	//WriteTermStr(C_WHITE,0,"\n");
	return 1;
}

void DisplayVideoEncState(void)
{//显示视频编码器的状态
	int i;
	//WriteTermStr(C_HWHITE,1,"GT1000 media resource usage display\n");
	WriteTermStr(C_WHITE,0,"\t\t    视频编码器(主码流)\n");
	WriteTermStr(C_WHITE,1,"VNo  状态   LastSeq NewEle  峰值(kbps) 平均值(kbps) 帧率(fps)\t\t\n");
	for(i=0;i<get_video_num();i++)
	{
		ShowEncState(i,i*2,&VideoEncoder[i*2]);
	}
	WriteTermStr(C_WHITE,0,"\t\t    视频编码器(辅码流)\n");
	WriteTermStr(C_WHITE,1,"VNo  状态   LastSeq NewEle  峰值(kbps) 平均值(kbps) 帧率(fps)\t\t\n");
	for(i=0;i<get_video_num();i++)
	{
		ShowEncState(i,i*2+1,&VideoEncoder[i*2+1]);
	}

	WriteTermStr(C_WHITE,0,"\t\t    视频用户信息(主码流)\n");
	WriteTermStr(C_WHITE,1,"VNo 用户号          NextEle     用户名  连接时间(s) \n");
	for(i=0;i<get_video_num();i++)
	{
		ShowEncUsrState(i,&VideoEncoder[i*2]);
	}
	WriteTermStr(C_WHITE,0,"\t\t    视频用户信息(辅码流)\n");
	WriteTermStr(C_WHITE,1,"VNo 用户号          NextEle     用户名  连接时间(s) \n");
	for(i=0;i<get_video_num();i++)
	{
		ShowEncUsrState(i,&VideoEncoder[i*2+1]);
	}
}





