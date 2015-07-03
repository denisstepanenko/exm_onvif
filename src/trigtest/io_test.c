#include<stdio.h>
#include<unistd.h>

#include "gtvs_io_api.h"		//控制io输入输出
#include "testmod.h"
#include "io_test.h"
#include <unistd.h>
#include <commonlib.h>
#include <iniparser.h>
#include <devinfo.h>

#define CHECK_VAL_A		(0x00)	//第1次测试端子期望值
#define CHECK_VAL_B		(0xff)	//第2次测试端子期望值

/**********************************************************************************************
* 函数名   :check_trigin_bit()
* 功能  :       定位输入输出端口位
* 输入  :       stat		读出的端子状态
*			   val		期望的值
* 输出  :       void        
* 返回值:   0正确，负值错误码
**********************************************************************************************/
int check_trigin_bit(unsigned int stat,unsigned int val)
{
	unsigned int i;
	unsigned int tmp_stat;
	unsigned int tmp_val;
	char tmp_info[128];
	
	for(i=0;i<8;i++)
	{
		tmp_stat=(stat>>i) & 0x01;
		tmp_val =(val>>i) & 0x01;

		memset(tmp_info,0,sizeof(tmp_info));
		if(tmp_stat != tmp_val)
		{
			sprintf(tmp_info,"%s%d%s\n","端子通道IN",i,"故障或测试工装未连接");
			//printf("端子通道IN[%d]错误，请检查.\n",i);
			printf(tmp_info);
			s_test_rp(tmp_info,30);
		}


	}
	return 0;	

}




	
/**********************************************************************************************
* 函数名   :test_io_mod()
* 功能  :       测试io模块
* 输入  :       void
* 输出  :       void        
* 返回值:   0正确，负值错误码
**********************************************************************************************/
int test_io_mod(void)
{
	//int vs3_fd;
	int ret;
	int ch=0;
	DWORD status;
	int err_flag;
	int i;
	
	printf("开始测试...\n");
	
	ret=init_gpiodrv();
	if(ret<0)
	{
		printf("初始化io驱动失败:%d\n",ret);
		return -1;
	}
	//printf("打开vs3iodrv成功\n");

	for(i=0;i<4;i++)
	{
		//立即触发，没有延时
		//printf("初始化继电器%d触发时间\n",i);
		set_trigin_delay(0,0);
		
		//printf("继电器%d初始化.\n",i);
		set_relay_output(i,0);
	}

	//输出全为1测试
	printf("开始测试端子 \n");

	printf("--------------------------\n");
	for(ch=0;ch<4;ch++)
	{
		//printf("第[%d]个端子动作\n",ch);
		ret=set_relay_output(ch,1);
		if(ret>0)
		{
			printf("设置端子失败\n");
			return -1;
		}
	}
	sleep(1);
	read_trigin_status(&status);
	printf("继电器动作读出的状态为[0x%x]\n",status);
	err_flag=0;
	check_trigin_bit(status,CHECK_VAL_A);	
	if(status!=CHECK_VAL_A)
	{
		printf("端子第1次测试错误\n");
		//s_test_rp("端子第1次测试错误",10);
		err_flag|=0x01;
	}
	else
	{
		printf("端子第1次测试正确\n");
	}

	printf("--------------------------\n");
	sleep(2);

	//输出全为0测试 
	for(ch=0;ch<4;ch++)
	{
		//printf("第[%d]个端子恢复\n",ch);
		ret=set_relay_output(ch,0);
		if(ret>0)
		{
			printf("设置端子失败\n");
			return -1;
		}
	}
	sleep(1);
	read_trigin_status(&status);
	printf("继电器恢复后读出的状态为[0x%x]\n",status);
	check_trigin_bit(status,CHECK_VAL_B);
	if(status!=CHECK_VAL_B)
	{
		printf("端子第2次测试错误\n");
		//s_test_rp("端子第2次测试错误",10);
		err_flag|=0x20;
	}
	else
	{
		printf("端子第2次测试正确\n");
	}
	printf("--------------------------\n");

	if(err_flag==0)
	{
		s_test_rp("端子正常",35);
		return 0;
	}
	else
	{
		s_test_rp("端子错误",35);
		return -1;
	}

	exit_gpiodrv();

       return 0;

}


