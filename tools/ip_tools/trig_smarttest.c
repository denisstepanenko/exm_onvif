#include "hdctl.h"
#include "stdio.h"
#include "error.h"
#include "stdlib.h"

#include <unistd.h>
#include "string.h"

int main(int argc, char **argv)
{
	int diskno;
	int ret;
	int percent;
	
	if(argc ==1)
		diskno = 0;
	else
		diskno = *argv[1];
		
	printf("进行%d号磁盘的短测试!请耐心等待2分钟左右\n",diskno);
	ret = run_hd_smarttest(diskno,GT_SMART_SHORTTEST); //shorttest
	if(ret != 0)
	{
		printf("%d号磁盘短测试失败,%d:%s\n",diskno,ret,strerror(-ret));
		return -1;
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
	return 0;
}
