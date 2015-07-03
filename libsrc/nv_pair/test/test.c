#include <stdio.h>
#include <sys/types.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <file_def.h>
#include <commonlib.h>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <nv_pair.h>


int test_fac_para_set(void)
{
	const char* buf=NULL;
	NVP_TP *dist = NULL;			//一个操纵命值对的结构指针
	
	dist = nvp_create();
	nvp_set_equal_mark(dist, "==");	//设置名值对的等于符号
	nvp_set_seperator(dist, "&&");	//设置名值对的分隔符号


	nvp_set_pair_str(dist, "cmd", "set_factory");		// cmd == M_SET_FACTORY
	nvp_set_pair_int(dist, "seq", 210);				// seqence number =210
	nvp_set_pair_str(dist, "board_seq","gt3.61");			// board seqence number
	nvp_set_pair_str(dist, "batch_seq", "2007_001");		// batch seqence number
	nvp_set_pair_str(dist, "leave_factory", "20070103120000");	//time of leave factory
	
	buf = nvp_get_string(dist);	//获取整个字符串
	printf("the n_v string=%s\n",buf);
	nvp_dump(dist);			//打印所有的信息
	nvp_destroy(dist);		     // 销毁命值对结构指针
	return 0;
}

int main(void)
{
	int ret = 0;
	ret = test_fac_para_set();
	exit(0);
}
