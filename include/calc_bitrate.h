/**********************************************************************************************
 * file:calc_bitrate.h
 * 计算码流的功能函数声明
 * 
 **********************************************************************************************/
#ifndef	CALC_BITRATE_H
#define	CALC_BITRATE_H
//计算流量
//#include <types.h>
 #include <sys/time.h>


typedef	struct{					//计算流量用到的结构定义
	struct timeval	PeakTime;		//上次计算峰值用的时间
	struct timeval 	AvgTime;		//上次计算平均值用的时间
	unsigned long		LastCheckBytes;	//上次计算码率时的已发送字节数
	unsigned long		PeakBytes;	//上次计算峰值时的字节数
	unsigned long		AvgBytes;	//上次计算平均值时的字节数
	double			PeakBitrate;	//上次计算出的峰值	bit per second
	double			AvgBitrate;	//上次计算出的平均值	bit per second
	int			AvgInterval;	//计算平均值的时间间隔(秒)
}BIT_RATE_T;
 
 /**********************************************************************************************
 * 函数名:GetPeakBitRate()
 * 功能:获取BR结构中的峰值码流
 * 参数: BR:指向描述对象码流结构的指针
 * 返回值:对象中的峰值码流
 **********************************************************************************************/
 static __inline__ double GetPeakBitRate(BIT_RATE_T *BR)
 {
 	return BR->PeakBitrate;
 }
 
 /**********************************************************************************************
 * 函数名:GetPeakBitRate()
 * 功能:获取BR结构中的码流平均值
 * 参数: BR:指向描述对象码流结构的指针
 * 返回值:对象中的码流平均值
 **********************************************************************************************/
 static __inline__ double GetAvgBitRate(BIT_RATE_T *BR)
 {
 	return BR->AvgBitrate;
 }

 /**********************************************************************************************
 * 函数名:SetCalcAvgTime()
 * 功能:设置计算码流平均值的时间间隔
 * 参数: BR:指向描述对象码流结构的指针
 *		   second:间隔时间(秒)
 * 返回值:无
 **********************************************************************************************/
 static __inline__ void SetCalcAvgTime(BIT_RATE_T *BR,int second)
 {
 	if(second<=0)
		second=1;
 	BR->AvgInterval=second;
 }

/**********************************************************************************************
 * 函数名:DiffTv()
 * 功能:计算两个时间的间隔，返回tv1-tv2的浮点数值
 * 参数: tv1:第一个时间
 * 	 	   tv2:第二个时间
 * 返回值:(tv1-tv2)的浮点数值
 **********************************************************************************************/
static __inline__ double	DiffTv(struct timeval *tv1,struct timeval *tv2)
{
	double res;
	struct timeval temp;
	temp.tv_sec=tv1->tv_sec-tv2->tv_sec;
	if(tv1->tv_usec<tv2->tv_usec)
	{
		temp.tv_sec--;
		temp.tv_usec=1000000+tv1->tv_usec-tv2->tv_usec;	
	}
	else
		temp.tv_usec=tv1->tv_usec-tv2->tv_usec;
	res=(double)((double)temp.tv_sec*1000000+(double)temp.tv_usec)/(double)1000000;
	return res;
}

/**********************************************************************************************
 * 函数名:CalcBitRate()
 * 功能:计算比特率，用NewBytes的字节数刷新BR结构
 * 参数: BR:指向描述流量用的数据结构指针
 * 	 NewBytes:新增加的字节数
 * 返回值:无
 **********************************************************************************************/
#if 0
static __inline__ void CalcBitRate(BIT_RATE_T *BR,unsigned long NewBytes)
{
	struct timeval	CurTv;
	double	PDiff;		//峰值时间差
	double	ADiff;		//平均值时间差
	long		PBytes;		//峰值字节数
	long		ABytes;		//平均值字节数
	if(BR==NULL)
		return;
	if(gettimeofday(&CurTv,NULL)<0)
		return;
	PDiff=DiffTv(&CurTv,&BR->PeakTime);
	PBytes=NewBytes-BR->PeakBytes;

	ADiff=DiffTv(&CurTv,&BR->AvgTime);

	if(PDiff>0.1)//大于2贞的时间才计算
	{
		BR->PeakBitrate=(double)(((double)PBytes/(double)PDiff)*8)/(double)1000;
		BR->PeakBytes=NewBytes;
		*(double*)&BR->PeakTime=	*(double*)&CurTv;			
	}
	//printf("%f-%f-%d-%d!!\n",PDiff,ADiff,PBytes,ABytes);
	if((ADiff+0.2)>=BR->AvgInterval)
	{
		ABytes=NewBytes-BR->AvgBytes;
		BR->AvgBitrate=(((double)ABytes/(double)ADiff)*8)/(double)1000;
		BR->AvgBytes=NewBytes;
		*(double*)&BR->AvgTime=	*(double*)&CurTv;
	}	
	
	
}
#endif

static __inline__ void CalcBitRate(BIT_RATE_T *BR,unsigned long NewBytes)
{
	struct timeval	CurTv;
	double	PDiff;		//峰值时间差
	double	ADiff;		//平均值时间差
//	long		PBytes;		//峰值字节数
//	long		ABytes;		//平均值字节数
	if(BR==NULL)
		return;
	if(gettimeofday(&CurTv,NULL)<0)
		return;
	PDiff=DiffTv(&CurTv,&BR->PeakTime);
//	PBytes=NewBytes-BR->PeakBytes;

	ADiff=DiffTv(&CurTv,&BR->AvgTime);
	BR->PeakBytes+=NewBytes;
	
	if((double)PDiff>(double)0.2)//大于2贞的时间才计算
	{
		BR->PeakBitrate=(double)(((double)BR->PeakBytes/(double)PDiff)*8);
		BR->PeakBytes=0;
		*(double*)&BR->PeakTime=	*(double*)&CurTv;			
	}
	//printf("%f-%f-%d-%d!!\n",PDiff,ADiff,PBytes,ABytes);
	BR->AvgBytes+=NewBytes;	
	if((ADiff+0.2)>=(double)BR->AvgInterval)
	{
//		ABytes=NewBytes-BR->AvgBytes;
		BR->AvgBitrate=(((double)BR->AvgBytes/(double)ADiff)*8);///(double)1000;
		BR->AvgBytes=0;
		*(double*)&BR->AvgTime=	*(double*)&CurTv;
	}	
	
	
}

#endif
