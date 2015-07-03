#include "watch_media.h"
#include <mshmpool.h>
#include <calc_bitrate.h>
#include <devinfo.h>
#include <media_api.h>
#include <time.h>

MSHM_POOL	audio_encoder[MAX_AUDIO_ENCODER*2];		//音频编码器缓冲池结构
BIT_RATE_T	audio_bitrate[MAX_AUDIO_ENCODER*2];		//音频码流结构
MSHM_POOL	audio_decoder[MAX_AUDIO_DECODER];		//音频编码器缓冲池结构
BIT_RATE_T	dec_bitrate[MAX_AUDIO_DECODER];		//音频码流结构

void init_audio_enc_state(void)
{
	int i,j,total;
    int ret;
	memset(audio_encoder,0,sizeof(audio_encoder));
	memset(audio_bitrate,0,sizeof(audio_bitrate));
	total=4;//目前只有一个音频编码器get_audioenc_num();
	for(i=0;i<total;i++)
	{
		j=i*2;

		ret=MSHmPoolAttach(&audio_encoder[j],get_onvif_pool_key(i,0)+0x20000);
		if(ret!=0)
		{
			printf("MSHmPoolAttach audio encoder%d ret=%d!!\n",j,ret);
			sleep(1);
		}
		audio_bitrate[j].AvgInterval=2;
		audio_bitrate[j].LastCheckBytes=0;
	}
	for(i=0;i<total;i++)
	{
		j=i*2+1;
		ret=MSHmPoolAttach(&audio_encoder[j],get_onvif_pool_key(i,1)+0x20000);
		if(ret!=0)
        {
            printf("MSHmPoolAttach audio encoder%d ret=%d!!\n",j,ret);
            sleep(1);
        }
		audio_bitrate[j].AvgInterval=2;
		audio_bitrate[j].LastCheckBytes=0;
	}

}
void init_audio_dec_state(void)
{
	int i,total;
       int ret;
	memset(audio_decoder,0,sizeof(audio_decoder));
	memset(dec_bitrate,0,sizeof(dec_bitrate));
	total=4;
	for(i=0;i<total;i++)
	{
		ret=MSHmPoolAttach(&audio_decoder[i],get_audio_dec_key(i));
	  if(ret!=0)
	  {
		  printf("MSHmPoolAttach audio decoder%d ret=%d!!\n",i,ret);
		  sleep(1);
	  }
		dec_bitrate[i].AvgInterval=2;
		dec_bitrate[i].LastCheckBytes=0;
	}

}

//显示指定编码器号的信息
//返回1表示编码器存在
static int	show_audio_enc_state(int ano,int no,MSHM_POOL *a_enc)
{
	//int i;
	//int Val;
	media_attrib_t 		*e_attr;
	SHPOOL_HEAD		*ph;
	BIT_RATE_T			*br=&audio_bitrate[no];

	if(a_enc->mc==NULL)
	{//没有被初始化
		//WriteTermStr(C_RED,0,"%02d NE\n",No);
		return 0;
	}
	
	ph=a_enc->ph;
	e_attr=MShmPoolGetInfo(a_enc);


	CalcBitRate(br,ph->send_bytes-br->LastCheckBytes);	//计算码流
	br->LastCheckBytes=ph->send_bytes;				//更新字节信息

	
	WriteTermStr(C_WHITE,0,"%02d  ",ano);				//视频编码器编号

	if(e_attr->stat!=1)								//设备状态
		WriteTermStr(C_RED,0,"  %02d",e_attr->stat);		//编码器有问题
	else
		WriteTermStr(C_WHITE,0,"  %02d",e_attr->stat);	//编码器正常
	WriteTermStr(C_WHITE,0,"%10d ",ph->num);			//当前序号
	WriteTermStr(C_WHITE,0,"%4d ",(ph->tail));			//最新的元素
	WriteTermStr(C_WHITE,0,"      %4.1f ",br->PeakBitrate/1024);	//峰值流量
	WriteTermStr(C_WHITE,0,"\t  %4.1f ",br->AvgBitrate/1024);	//峰值流量
	WriteTermStr(C_WHITE,0,"\n");
	return 1;
}
static int	show_audio_dec_state(int no,MSHM_POOL *a_dec)
{
	//int i;
	//int Val;
	media_attrib_t 		*e_attr;
	SHPOOL_HEAD		*ph;
	BIT_RATE_T			*br=&dec_bitrate[no];

	if(a_dec->mc==NULL)
	{//没有被初始化
		//WriteTermStr(C_RED,0,"%02d NE\n",No);
		return 0;
	}
	
	ph=a_dec->ph;
	e_attr=MShmPoolGetInfo(a_dec);


	CalcBitRate(br,ph->send_bytes-br->LastCheckBytes);	//计算码流
	br->LastCheckBytes=ph->send_bytes;				//更新字节信息

	
	WriteTermStr(C_WHITE,0,"%02d  ",no);				//视频编码器编号

	if(e_attr->stat!=1)								//设备状态
		WriteTermStr(C_RED,0,"  %02d",e_attr->stat);		//编码器有问题
	else
		WriteTermStr(C_WHITE,0,"  %02d",e_attr->stat);	//编码器正常
	WriteTermStr(C_WHITE,0,"%10d ",ph->num);			//当前序号
	WriteTermStr(C_WHITE,0,"%4d ",(ph->tail));			//最新的元素
	WriteTermStr(C_WHITE,0,"      %4.1f ",br->PeakBitrate/1024);	//峰值流量
	WriteTermStr(C_WHITE,0,"\t  %4.1f ",br->AvgBitrate/1024);	//峰值流量
	WriteTermStr(C_WHITE,0,"\n");
	return 1;
}
//显示连接到指定编码器的用户信息
static int	show_audio_enc_usr_state(int no,MSHM_POOL *a_enc)
{
	SHPOOL_USR   		*usr;
	SHPOOL_HEAD		*ph;
	int	i;
	time_t	CurTime	=time(NULL)	;	//当前时间

	if(a_enc->mc==NULL)
	{//没有初始化			
		return 0;
	}	
	ph=a_enc->ph;

///用户信息
	for(i=0;i<SHPOOL_MAX_USERS;i++)
	{
		usr=&ph->users[i];
		if(usr->valid)
		{	//有效用户
			//WriteTermStr(C_WHITE,0,"\n\t\t\t");
			WriteTermStr(C_WHITE,0,"%02d  ",no);		//编码器号
			WriteTermStr(C_WHITE,0,"  %02d ",i);			//用户号
			WriteTermStr(C_WHITE,0,"           ");			
			WriteTermStr(C_WHITE,0,"%4d ",usr->curele);	//当前读取的元素序号
			WriteTermStr(C_WHITE,0,"%12s",usr->name);//用户名
			WriteTermStr(C_WHITE,0,"%10d ",(int)(CurTime-usr->stime));
			WriteTermStr(C_WHITE,0,"\n");	
		}		
	}
	//WriteTermStr(C_WHITE,0,"\n");
	return 1;
}
static int	show_audio_dec_usr_state(int no,MSHM_POOL *a_dec)
{
	SHPOOL_USR   		*usr;
	SHPOOL_HEAD		*ph;
	int	i;
	time_t	CurTime	=time(NULL)	;	//当前时间

	if(a_dec->mc==NULL)
	{//没有初始化			
		return 0;
	}	
	ph=a_dec->ph;

///用户信息
	for(i=0;i<SHPOOL_MAX_USERS;i++)
	{
		usr=&ph->users[i];
		if(usr->valid)
		{	//有效用户
			//WriteTermStr(C_WHITE,0,"\n\t\t\t");
			WriteTermStr(C_WHITE,0,"%02d  ",no);		//编码器号
			WriteTermStr(C_WHITE,0,"  %02d ",i);			//用户号
			WriteTermStr(C_WHITE,0,"           ");			
			WriteTermStr(C_WHITE,0,"%4d ",usr->curele);	//当前读取的元素序号
			WriteTermStr(C_WHITE,0,"%12s",usr->name);//用户名
			WriteTermStr(C_WHITE,0,"%10d ",(int)(CurTime-usr->stime));
			WriteTermStr(C_WHITE,0,"\n");	
		}		
	}
	//WriteTermStr(C_WHITE,0,"\n");
	return 1;
}

void display_audio_enc_state(void)
{//显示音频编码器的状态
	int i;
	//WriteTermStr(C_HWHITE,1,"GT1000 media resource usage display\n");
	WriteTermStr(C_WHITE,0,"\t\t   音频编码器(主码流)\n");
	WriteTermStr(C_WHITE,1,"ANo  状态   LastSeq NewEle  峰值(kbps)  平均值(kbps)\t\t\t\n");
	for(i=0;i<get_audio_num();i++)
	{
		show_audio_enc_state(i,i*2,&audio_encoder[i*2]);
	}
	WriteTermStr(C_WHITE,0,"\t\t   音频编码器(辅码流)\n");
	WriteTermStr(C_WHITE,1,"ANo  状态   LastSeq NewEle  峰值(kbps)  平均值(kbps)\t\t\t\n");
	for(i=0;i<get_audio_num();i++)
	{
		show_audio_enc_state(i,i*2+1,&audio_encoder[i*2+1]);
	}

	WriteTermStr(C_WHITE,0,"\t\t    音频编码器用户信息(主码流)\n");
	WriteTermStr(C_WHITE,1,"ANo 用户号          NextEle     用户名  连接时间(s) \n");
	for(i=0;i<get_audio_num();i++)
	{
		show_audio_enc_usr_state(i,&audio_encoder[i*2]);
	}
	WriteTermStr(C_WHITE,0,"\t\t    音频编码器用户信息(辅码流)\n");
	WriteTermStr(C_WHITE,1,"ANo 用户号          NextEle     用户名  连接时间(s) \n");
	for(i=0;i<get_audio_num();i++)
	{
		show_audio_enc_usr_state(i,&audio_encoder[i*2+1]);
	}
}
void display_audio_dec_state(void)
{//显示音频解码器的状态
	int i;
	WriteTermStr(C_WHITE,0,"\t\t   音频解码器\n");
	WriteTermStr(C_WHITE,1,"ANo  状态   LastSeq NewEle  峰值(kbps)  平均值(kbps)\t\t\t\n");
	for(i=0;i<MAX_AUDIO_DECODER;i++)
	{
		show_audio_dec_state(i,&audio_decoder[i]);
	}
	WriteTermStr(C_WHITE,0,"\t\t    音频解码器用户信息\n");
	WriteTermStr(C_WHITE,1,"ANo 用户号          NextEle     用户名  连接时间(s) \n");
	for(i=0;i<MAX_AUDIO_DECODER;i++)
	{
		show_audio_dec_usr_state(i,&audio_decoder[i]);
	}
}





