#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>

#include "testmod.h"
#include "pub_err.h"
#include <devinfo.h>
//#include "multicast_ctl.h"
//#include "hi_rtc.h"

/*
 * 将测试结果数据结构解析后存入指定的文件
 */
 
/*
****************************************************************************
测试结果数据结构 0 表示正常，其它值表示相应的错误
	-1 无法加载驱动模块
	-2  无法打开驱动模块
	-3  无法对驱动模块读写
	-4  无法对驱动模块进行I/O 控制
****************************************************************************
*/
/*
*****************************************************
*函数名称: print_rpt
*函数功能: 打印错误报告
*参数		    : err 错误器件代码
*			    : stat 错误状态代码
*	      		    : fp 记录文件的指针
*返回值      : 无
*****************************************************
*/

void print_rpt(int err, int stat, FILE * fp)
{
	fprintf(fp,"%d", err+stat);
	fprintf(fp, ":");

	switch(err)
	{
		case (NETENC_ERR):
		fprintf(fp, Net_error);
		fprintf(fp, "(");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NO6410);
			  break;
			  
		  case (2):
			  fprintf(fp, ERR_NOINPUT);
			  break;
			  
		  case (3):
			  fprintf(fp, ERR_NODATA);
			  break;
		  case (4):
		  	
			  fprintf(fp, ERR_UNSTABLE);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, ")");
		fprintf(fp, ",");
		break;
		
		case (HQENC_ERR):
		fprintf(fp, Hq_error);
		fprintf(fp, "(");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NO6410);
			  break;
			  
		  case (2):
			  fprintf(fp, ERR_NOINPUT);
			  break;
			  
		  case (3):
			  fprintf(fp, ERR_NODATA);
			  break;
		  case (4):
		  	
			  fprintf(fp, ERR_UNSTABLE);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, ")");
		fprintf(fp, ",");
		break;
		
		case (QUARD_ERR):
		fprintf(fp, Tw2834_error);
		fprintf(fp, "(");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NO2834);
			  break;
			  
		  case (2):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (3):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (4):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, ")");
		
		fprintf(fp, ",");
		break;
		
		case (DSP_ERR):
		fprintf(fp, Dsp_error);
		fprintf(fp, "(");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NODSP);
			  break;
			  
		  case (2):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (3):
			  fprintf(fp, UNKNOW);
			  break;
		  case (4):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, ")");

		fprintf(fp, ",");
		break;
		
		case (IDE_ERR):
		fprintf(fp, Ide_error);
		fprintf(fp, "(");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NODISK);
			  break;
			  
		  case (2):
			  fprintf(fp, ERR_NOPART);
			  break;
			  
		  case (3):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (4):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, ")");
		fprintf(fp, ",");
		break;

		case (TW9903_ERR):
		fprintf(fp, TW9903_error);
		fprintf(fp, "(");
		
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_IIC);
			  break;
			  
		  case (2):
			  fprintf(fp, ERR_NO9903);
			  break;
			  
		  case (3):
			  fprintf(fp, UNKNOW);
			  break;
		  case (4):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, ")");
		fprintf(fp, ",");
		break;
		
		default:
		fprintf(fp, UNKNOW);
		fprintf(fp, ",");
		break;
	}

}
/*
*****************************************************
*函数名称: print_code
*函数功能: 错误代码打印函数
*参数		  : err 错误器件代码
*			  : stat 错误状态代码
*	      		  : fp 记录文件的指针
*返回值      : 无
*****************************************************
*/

void print_code(int err, int stat, FILE * fp)
{
	fprintf(fp,"%d,", err+stat);
}
/*
*****************************************************
*函数名称: print_stat
*函数功能: 错误信息打印函数
*参数		  : err 错误器件代码
*	      		  : stat 错误状态代码
*	      		  : fp 记录文件的指针
*返回值	  : 无
*****************************************************
*/
void  print_stat(int err, int stat, FILE * fp)
{
  switch(err){
	case (NETENC_ERR):
		fprintf(fp, Net_error);
		fprintf(fp,":");
		
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NO6410);
			  break;
			  
		  case (2):
			  fprintf(fp, ERR_NOINPUT);
			  break;
			  
		  case (3):
			  fprintf(fp, ERR_NODATA);
			  break;
		  case (4):
		  	
			  fprintf(fp, ERR_UNSTABLE);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, "\n            ");
		break;
	case (HQENC_ERR):
		
		fprintf(fp, Hq_error);
		fprintf(fp,":");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NO6410);
			  break;
			  
		  case (2):
			  fprintf(fp, ERR_NOINPUT);
			  break;
			  
		  case (3):
			  fprintf(fp, ERR_NODATA);
			  break;
			  
		  case (4):
			  fprintf(fp, ERR_UNSTABLE);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, "\n            ");

		break;
		
	case (QUARD_ERR):
		fprintf(fp, Tw2834_error );
		fprintf(fp,":");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NO2834);
			  break;
			  
		  case (2):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (3):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (4):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, "\n            ");

		break;
		
	case (IDE_ERR):
		fprintf(fp, Ide_error );
		fprintf(fp,":");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NODISK);
			  break;
			  
		  case (2):
			  fprintf(fp, ERR_NOPART);
			  break;
			  
		  case (3):
			  fprintf(fp, ERR_OPEN);		///lsk 2007-11-8
			  break;
			  
		  case (4):
			  fprintf(fp, ERR_WRITE);		///lsk 2007-11-8
			  break;
			  
		  case (5):
			  fprintf(fp, ERR_READ);		///lsk 2007-11-8
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, "\n            ");
		break;
		
	case (DSP_ERR):
		fprintf(fp, Dsp_error);
		fprintf(fp,":");
		switch (stat){
		  case (1):
			  fprintf(fp, ERR_NODSP);
			  break;
			  
		  case (2):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (3):
			  fprintf(fp, UNKNOW);
			  break;
		  case (4):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  case (5):
			  fprintf(fp, UNKNOW);
			  break;
			  
		  default:
			  fprintf(fp, UNKNOW);
			  break;
		  }
		fprintf(fp, "\n            ");
		break;
		
	case (TW9903_ERR):
	fprintf(fp, TW9903_error);
	fprintf(fp,":");
	switch (stat){
	  case (1):
		  fprintf(fp, ERR_IIC);
		  break;
		  
	  case (2):
		  fprintf(fp, ERR_NO9903);
		  break;
		  
	  case (3):
		  fprintf(fp, UNKNOW);
		  break;
	  case (4):
		  fprintf(fp, UNKNOW);
		  break;
		  
	  case (5):
		  fprintf(fp, UNKNOW);
		  break;
		  
	  default:
		  fprintf(fp, UNKNOW);
		  break;
	  }
	fprintf(fp, "\n            ");
	break;
	
	default:
		fprintf(fp, UNKNOW);
		fprintf(fp, "\n            ");
		break;
  	}
}
/*
*****************************************************
*函数名称: save_result_to_file
*函数功能: 结果存储函数
*参数		  : filename 文件名
*	      		  : devstat 器件状态数据结构指针
*返回值	  : 错误代码
*****************************************************
*/
int save_result_to_file(char *filename,struct dev_test_struct *devstat, multicast_sock* ns)
{
	FILE *fp;
	struct dev_test_struct *stat;
	int rc;
	if(filename==NULL)
		return ERR_CANNOT_OPEN_FILE;
	if(devstat==NULL)
		return ERR_INTERNAL; 
	fp=fopen(filename,"w");
	if(fp==NULL)
		return ERR_CANNOT_OPEN_FILE;
	stat=devstat;
	fprintf(fp,"[boardtest]\n");
        if((stat->ide_stat==0)&&
	   (stat->netenc_stat==0)&&
	   (stat->hqenc0_stat==0)&&
	   (stat->audio_stat==0)&&
	   (stat->quad_stat==0)&&
	   (stat->tw9903_stat==0)&&
	   (stat->rtc_stat==0)&&
	   (stat->usb_stat==0))	//lsk 2006 -12-27
	{
		fprintf(fp,"result = 0, \n");
		fprintf(fp,"description = 测试成功,\n");
		fprintf(fp,"report = 0: 测试成功,\n");
		
		rc=SUCCESS;
		printf("all OK!\n");
		result_report(SUCCESS, ns);// lsk 2007-10-26
	}
	else
	{
	   fprintf(fp,"result = ");
	   rc=ERR_DEVICE_BAD;

	   if(stat->netenc_stat!=0)
	   print_code(NETENC_ERR, stat->netenc_stat, fp);
	   
	   if(stat->hqenc0_stat!=0)
	   print_code(HQENC_ERR, stat->hqenc0_stat, fp);
	   
	   if(stat->quad_stat!=0)
	   print_code(QUARD_ERR, stat->quad_stat, fp);
	   
	   if(stat->ide_stat!=0)
	   print_code(IDE_ERR, stat->ide_stat, fp);

          if(stat->usb_stat!=0)
	   print_code(IDE_ERR, stat->usb_stat, fp);
	   
	   if(stat->audio_stat!=0)
	   print_code(DSP_ERR, stat->audio_stat, fp);
	   
	   if(stat->tw9903_stat!=0)	// lsk 2006-12-27
	   print_code(TW9903_ERR, stat->tw9903_stat, fp);
	   
#if 0
	   
	   printf("%d,%d,%d,%d,%d ",
	   	stat->netenc_stat,
	   	stat->hqenc0_stat,
	   	stat->quad_stat,
	   	stat->ide_stat,
	   	stat->audio_stat
	   	);
#endif
	   
	fprintf(fp,"\ndescription=");

	if(stat->netenc_stat!=0)
	{
		print_stat(NETENC_ERR, stat->netenc_stat, fp);
		result_report(NETENC_ERR+stat->netenc_stat, ns);//lsk 2007 -6-1
	} 

	if(stat->hqenc0_stat!=0)
	{	  
		print_stat(HQENC_ERR, stat->hqenc0_stat, fp);
		result_report(HQENC_ERR+stat->hqenc0_stat, ns);//lsk 2007 -6-1
	}	   

	if(stat->quad_stat!=0)
	{
		print_stat(QUARD_ERR, stat->quad_stat, fp);
		result_report(QUARD_ERR+stat->quad_stat, ns);//lsk 2007 -6-1
	}

	if(stat->usb_stat!=0)
	{
		print_stat(USB_ERR, stat->usb_stat, fp);
		result_report(USB_ERR+stat->usb_stat, ns);//lsk 2007 -6-1
	}
	
	if(stat->ide_stat!=0)
	{
		print_stat(IDE_ERR, stat->ide_stat, fp);
		result_report(IDE_ERR+stat->ide_stat, ns);//lsk 2007 -6-1
	}
	
	if(stat->audio_stat!=0)
	{
		print_stat(DSP_ERR, stat->audio_stat, fp);
		result_report(DSP_ERR+stat->audio_stat, ns);//lsk 2007 -6-1
	}
	
	if(stat->tw9903_stat!=0)	// lsk 2006-12-27
	{
		print_stat(TW9903_ERR, stat->tw9903_stat, fp);
		result_report(TW9903_ERR+stat->tw9903_stat, ns);//lsk 2007 -6-1
	}

//////////  new format for net report
	   fprintf(fp,"\nreport=");
	   if(stat->netenc_stat!=0)
   	   print_rpt(NETENC_ERR, stat->netenc_stat, fp);
	   
	   if(stat->hqenc0_stat!=0)
	   print_rpt(HQENC_ERR, stat->hqenc0_stat, fp);
	   
	   if(stat->quad_stat!=0)
	   print_rpt(QUARD_ERR, stat->quad_stat, fp);
	   
	   if(stat->ide_stat!=0)
	   print_rpt(IDE_ERR, stat->ide_stat, fp);
	   
	   if(stat->audio_stat!=0)
	   print_rpt(DSP_ERR, stat->audio_stat, fp);

	   if(stat->tw9903_stat!=0)	// lsk 2006-12-27
	   print_rpt(TW9903_ERR, stat->tw9903_stat, fp);
	}
	fclose(fp);
	return rc;
}
/*
 * 模拟测试流程控制
 */
 
/*
*****************************************************
*函数名称: testsim
*函数功能: 硬件测试函数
*参数		  : stat 器件状态数据结构指针
*返回值	  : 0 正常 -1 stat 无效
*****************************************************
*/
int testsim(struct dev_test_struct *stat, multicast_sock* net_st, int prog)
{  
	int progress=prog;
       int result;
       
	if(stat==NULL)
		return -1;
	init_devinfo();
	printf("设备型号: %s\n", get_devtype_str()); //lsk 2006-12-27  print device type

       result = testRTC();
       if(result )
	{
		printf("RTC test error\n" );
	}
	else 
	{
		printf("RTC test  OK\n");
		send_test_report(net_st, "RTC OK", progress);
	}

       result = testUSB();
       if(result )
	{
		printf("USB test error\n" );
              stat->usb_stat = result;
	}
	else 
	{
		printf("USB test  OK\n");
		send_test_report(net_st, "USB OK", progress);
	}       
	if(get_ide_flag()>=0)	//lsk 2006 -10 -26 get_ide_flag()==1 -> get_ide_flag()>=0
	{
		printf("testing IDE...");
		stat->ide_stat=test_IDE(net_st, &progress);		//测试IDE控制器
		if(stat->ide_stat)
		{
			printf("IDE state: %d\n", stat->ide_stat);
		}
		else 
		{
			printf("IDE OK\n");
			send_test_report(net_st, "IDE OK", progress);
		}
		sleep(1);
	}
	
	if(get_videoenc_num()>0)
	{
		if(get_audio_num()>0)
		{
		}
		if(get_quad_flag()==1)
		{
		}
		////lsk 2006 -12 -27
		if(get_quad_flag()==0)
		{
		}
	}

	send_test_report(net_st, "board test finished", 100);//完成后发送测试进度=100
	return 0;
}

