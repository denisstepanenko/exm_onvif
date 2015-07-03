#include "hdctl.h"
#include "smart.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "gtlog.h"
#include "devinfo.h"


int open_hd_dev(char *pathname)
{
	if((pathname == NULL) ||(access(pathname, F_OK|R_OK)!=0 ))//节点不存在
		return -ENODEV;
	else
		return open(pathname,O_RDONLY | O_NONBLOCK);
}






/****************************************************************
	函数名称	get_hd_temperature()
	功能		获取指定硬盘的当前温度（摄氏）
	输入		diskno,为0,1,2..的硬盘编号
	返回值		正值表示当前温度，负值表示错误码
****************************************************************/
int get_hd_temperature(int diskno)
{
	int fd;
	int ret;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		ret = (int)ataReadSmartValues(fd,TEMPERATURE_CELSIUS, RAWVALUE);		
		close(fd);
		return (ret & 0x3F);
	}
	else
		return -ENODEV;	
}


/****************************************************************
	函数名称	get_hd_max_temperature()
	功能		获取指定硬盘的最高温度（摄氏）
	输入		diskno,为0,1,2..的硬盘编号
	返回值		正值表示最高温度，负值表示错误码
****************************************************************/
int get_hd_max_temperature(int diskno)
{
	int fd;
	int ret;
	int manufactor;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		manufactor = get_hd_manufactor(fd);
		switch(manufactor) 
		{
			case(SEAGATE):	ret = (int)ataReadSmartValues(fd,TEMPERATURE_CELSIUS,WORSTVALUE);
							break;
			default:		ret = get_hd_temperature(diskno);
							break;
		}
		close(fd);
		return ret;
	}
	else
		return -ENODEV;	
}




/****************************************************************
	函数名称	get_hd_shorttest_result()
	功能		获取指定硬盘的上一次短测试结果
	输入		diskno,为0,1,2..的硬盘编号
	输出		percent_done:指向完成百分数的指针，其值取值0-100，在测试进行中的情况下才有意义
	返回值		0表示成功通过，1表示失败,2表示没有读到结果,3表示进行中
				负值表示失败的错误码
****************************************************************/
int get_hd_shorttest_result(int diskno, int *percent_done)
{
	int fd;
	int ret;
	
	if(percent_done == NULL)
		return -EINVAL;
	fd = open_hd_dev(get_hd_nodename(diskno));
	
	
	if(fd > 0)
	{
		
		ret = ataGetSelfTestLog(fd,GT_SMART_SHORTTEST,percent_done);
		close(fd);
		return ret;
	}
	else
		return -ENODEV;	
}


/****************************************************************
	函数名称	get_hd_shorttest_result()
	功能		获取指定硬盘的上一次长测试结果
	输入		diskno,为0,1,2..的硬盘编号
	输出		percent_done:指向完成百分数的指针，其值取值0-100，在测试进行中的情况下才有意义
	返回值		0表示成功通过，1表示失败,2表示没有读到结果,3表示进行中
				负值表示失败的错误码
****************************************************************/
int get_hd_longtest_result(int diskno, int *percent_done)
{
	int fd;
	int ret;
	
	
	if(percent_done == NULL)
		return -EINVAL;
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		ret = ataGetSelfTestLog(fd,GT_SMART_LONGTEST,percent_done);
	
		close(fd);
		return ret;
	}
	else
		return -ENODEV;	
}




/****************************************************************
	函数名称	run_hd_smarttest()
	功能		对指定硬盘进行指定性质的测试
	输入		diskno,为0,1,2..的硬盘编号
				testtype,为0表示短测试，为1表示长测试,为2表示短测试通过后再长测试
	返回值		0表示成功，其他值表示失败
****************************************************************/
int run_hd_smarttest(int diskno, int testtype)
{
	int fd,ret;
	char cmd[100];
	int percent;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if (fd > 0)
	{
		switch(testtype)
		{
			case(0):
			case(1):	ret = ataSmartTest(fd,testtype);
						break;
			case(2):	
			{
				//lc do 不通过系统命令，直接递归调用
				
				sprintf(cmd,"/ip1004/trigsmarttest %d &",diskno);
						ret = system(cmd);
						break;
				/*
				printf("进行%d号磁盘的短测试!请耐心等待2分钟左右\n",diskno);
				ret = run_hd_smarttest(diskno,GT_SMART_SHORTTEST); //shorttest
				if(ret != 0)
				{
					printf("%d号磁盘短测试失败,%d:%s\n",diskno,ret,strerror(-ret));
					break;
				}
				sleep(100);
	
				ret =get_hd_shorttest_result(diskno,&percent) ;
				if(ret == 0)
				{
					printf("%d号磁盘短测试通过。进行磁盘长测试!\n",diskno);
					run_hd_smarttest(diskno,GT_SMART_LONGTEST);
					printf("%d号磁盘已开始测试。请在5小时或更久后查询测试结果。\n",diskno);
				}
				else
					printf("%d号磁盘短测试结果 %d:%s，不进行长测试\n",diskno,ret,get_testresult_str(ret));

				break;
				*/
			}
			default:	ret = -EINVAL;
						break;
		}
		close(fd);
		return ret;
	}
	else
		return -ENODEV;
}

/****************************************************************
	函数名称	get_hd_info()
	功能		获取指定硬盘的型号,序列号,固件版本号等基本信息
	输入		diskno,为0,1,2..的硬盘编号
	输出		model,硬盘型号字符串
				serialno,硬盘序列号字符串
				firmware,固件版本字符串
	返回值		0表示成功，负值表示错误码
	说明		model字符串的缓冲区至少需要40字节
				serialno的缓冲区至少需要20字节
				firmware的缓冲区至少需要8字节
****************************************************************/
int get_hd_info(IN int diskno, OUT char *model, OUT char* serialno, OUT char *firmware)
{
	int fd,ret=0;
	struct ata_identify_device  drive;
	
	if((model==NULL)||(serialno==NULL)||(firmware==NULL))
		return -EINVAL;
	
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		if(ataReadHDIdentity(fd, &drive)== 0)
		{
			formatdriveidstring(model, (char *)drive.model,40);
  			formatdriveidstring(serialno, (char *)drive.serial_no,20);
  			formatdriveidstring(firmware, (char *)drive.fw_rev,8);
		}
		else
		 	ret = -EPERM;
		close(fd);
		return ret;
	}
	else
		return -ENODEV;
	
}









/***************************************************************
	函数名称	get_hd_volume_inGiga()
	功能		获取指定硬盘的容量，单位为G,按销售标准,即250G,320G等
	输入		diskno,为0,1,2..的硬盘编号
	返回值		正值表示容量,0或负值表示失败
****************************************************************/
int get_hd_volume_inGiga(int diskno)
{
	struct ata_identify_device  drive;
	long long capacity;
	
	int fd;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		if(ataReadHDIdentity(fd,&drive) == 0)
		{
			capacity = determine_capacity(&drive);
			close(fd);
			return capacity/1000000;
		}
		close (fd);
		return -EPERM;
	}
	else
		return -ENODEV;
}



/****************************************************************	
	函数名称	get_hd_runninghour()
	功能		获取指定硬盘的上电总小时数
	输入		diskno,为0,1,2..的硬盘编号
	返回值		非负值表示小时数,负值表示错误码
****************************************************************/
int get_hd_runninghour(int diskno)
{
	int fd;
	int ret;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		ret = (int)ataReadSmartValues(fd,POWER_ON_HOURS,RAWVALUE);
		if((get_hd_manufactor(fd)==MAXTOR)) //迈拓的是以分钟计算
			ret/= 60;
		close(fd);
		return ret;
	}
	else
		return -ENODEV;	

}



/****************************************************************	
	函数名称	get_hd_relocate_sector()
	功能		获取指定硬盘的重分配扇区数
	输入		diskno,为0,1,2..的硬盘编号
	返回值		非负值表示扇区数,负值表示错误码
****************************************************************/
int get_hd_relocate_sector(int diskno)
{
	int fd;
	int ret;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		ret = (int)ataReadSmartValues(fd,REALLOCATED_SECTOR_CT,RAWVALUE);
		close(fd);
		return ret;
	}
	else
		return -ENODEV;	

}




/****************************************************************	
	函数名称	get_hd_pending_sector()
	功能		获取指定硬盘的挂起扇区数
	输入		diskno,为0,1,2..的硬盘编号
	返回值		非负值表示扇区数,负值表示错误码
****************************************************************/
int get_hd_pending_sector(int diskno)
{
	int fd;
	int ret;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		ret = (int)ataReadSmartValues(fd,CURRENT_PENDING_SECTOR,RAWVALUE);
		close(fd);
		return ret;
	}
	else
		return -ENODEV;	
}


/****************************************************************	
	函数名称	get_hd_errorlog_num()
	功能		获取指定硬盘的错误日志条数
	输入		diskno,为0,1,2..的硬盘编号
	返回值		非负值表示错误日志条数,负值表示错误
****************************************************************/
int get_hd_errorlog_num(int diskno)
{
	int fd;
	int ret;
	
	fd = open_hd_dev(get_hd_nodename(diskno));
	if(fd > 0)
	{
		ret = ataReadErrorLog(fd);
		close(fd);
		return ret;
	}
	else
		return -ENODEV;
}

