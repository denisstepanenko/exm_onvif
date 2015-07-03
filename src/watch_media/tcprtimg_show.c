#include "watch_media.h"
#include "media_svr.h"
#include "tcprtimg_show.h"
#include <calc_bitrate.h>
#include "mshm.h"
static MEM_CORE		*AVMCore=NULL;			//共享内存指针	
static AVSERVER_TYPE	*AVServer=NULL;			//媒体服务数据结构
BIT_RATE_T			UsrBitRate[MAX_AV_USER_NUM];	//网络用户流量

#define         TCPRTIMG_STAT_KEY       	0x41000


void InitTcprtimgState(void)
{	
	int i;
	void *p;
	memset((void*)UsrBitRate,0,sizeof(UsrBitRate));
	for(i=0;i<MAX_AV_USER_NUM;i++)
	{
		UsrBitRate[i].AvgInterval=5;
		UsrBitRate[i].LastCheckBytes=0;
	}
	AVMCore=(MEM_CORE*)MShmCoreAttach(TCPRTIMG_STAT_KEY,NULL);
	if(AVMCore==NULL)
		return ;
	p=(void*)AVMCore;
	p+=AVMCore->uoffset;
	AVServer=(AVSERVER_TYPE*)p;	
}


void ShowTcprtimgUsrState(AVUSR_TYPE *Usr)
{
	time_t Curtime;
	if(!Usr->Enable)
		return;
	if(Usr->No>=MAX_AV_USER_NUM)
		return;

	Curtime=time(NULL);
	WriteTermStr(C_WHITE,0," %d ",Usr->No);
	WriteTermStr(C_WHITE,0,"  %d ",Usr->VEncNo);
	WriteTermStr(C_WHITE,0," %3d ",Usr->NetFd);
	//WriteTermStr(C_WHITE,0,"%12s ",Usr->UsrName);
	WriteTermStr(C_WHITE,0,"%10d ",(int)(Curtime-Usr->ConnectStart.tv_sec));	//连接时间
	WriteTermStr(C_WHITE,0,"%16s ",inet_ntoa(Usr->Addr.sin_addr));

	//WriteTermStr(C_WHITE,0,"%2d ",(int)(Usr->CmdStart.tv_sec-Usr->ConnectStart.tv_sec));		//收到命令的时间


	WriteTermStr(C_WHITE,0,"%10d ",Usr->LastVSeq);
	WriteTermStr(C_WHITE,0,"  %d ",Usr->AudioFlag);
	if(Usr->AudioFlag)
	{
		WriteTermStr(C_WHITE,0,"%10d ",Usr->LastASeq);
	}




	WriteTermStr(C_WHITE,0,"\n");
}
void ShowTcprtimgUsrDetail(AVUSR_TYPE *Usr)
{
	BIT_RATE_T			*BR;
	if(!Usr->Enable)
		return;
	if(Usr->No>=MAX_AV_USER_NUM)
		return;
	BR=&UsrBitRate[Usr->No];
	//printf("test send bytest=%d\n",(int)Usr->SendBytes-);
	CalcBitRate(BR,Usr->SendBytes-BR->LastCheckBytes);
	BR->LastCheckBytes=Usr->SendBytes;
	WriteTermStr(C_WHITE,0," %d ",Usr->No);
	WriteTermStr(C_WHITE,0,"   %5.1f ",BR->PeakBitrate/1000);	//峰值流量
	WriteTermStr(C_WHITE,0,"    %5.1f ",BR->AvgBitrate/1000);	//峰值流量
	WriteTermStr(C_WHITE,0,"   %5.1fKB",(double)Usr->SendBufBytes/(double)1000);
	WriteTermStr(C_WHITE,0,"     %3d",Usr->SendBufInfo.VFrames);
	WriteTermStr(C_WHITE,0,"    %3d",Usr->SendBufInfo.AFrames);
	WriteTermStr(C_WHITE,0,"%6d",Usr->DropFrames);
	WriteTermStr(C_WHITE,0,"\n");
}

void DisplayTcprtimgState(void)
{
	int i;
	AVUSR_TYPE *Usr;
	if(AVServer==NULL)
	{
		WriteTermStr(C_HWHITE,0,"\t\t\tno tcprtimg2 service \n");
		return;
	}
	else
		WriteTermStr(C_WHITE,0,"\t\t\ttcprtimg2 service list \n");
	//WriteTermStr(C_WHITE,1,"No   STAT   LastSeq NewEle  Peak(kbps)  Avg(kbps)\n");	
	WriteTermStr(C_WHITE,1,"MaxUsr UsrNum SvrPort \t\t\t\t\t\t\n");
	WriteTermStr(C_WHITE,0,"   %d     %d     %d\n",AVServer->MaxUsr,AVServer->UsrNum,AVServer->SvrPort);
	if(AVServer->UsrNum>0)
	{
		WriteTermStr(C_WHITE,0,"\t\t\ttcprtimg2 user list \n");
		WriteTermStr(C_WHITE,1,"UNo VEnc Net Connect(s)    Remote addr     LastVSeq AFlag LastASeq\t\n");
		Usr=AVServer->Users;
		for(i=0;i<MAX_AV_USER_NUM;i++)
		{
			ShowTcprtimgUsrState(Usr);
			Usr++;
		}

		
		WriteTermStr(C_WHITE,0,"\t\t\ttcprtimg2 user detail \n");
		WriteTermStr(C_WHITE,1,"UNo Peak(kbps) Avg(kbps) SendBuf  VFrames AFrames DropF\t\n");
		Usr=AVServer->Users;
		for(i=0;i<MAX_AV_USER_NUM;i++)
		{
			ShowTcprtimgUsrDetail(Usr);
			Usr++;
		}

	}

	
}
