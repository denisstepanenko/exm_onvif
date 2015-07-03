#include "stdio.h"
#include "hdutil.h"

int set_cferr_flag()
{
	return 0;
}


int main(void)
{
	printf("check lost_found dirs for all partitions:\n");
	hdutil_init_all_partitions();
	printf("check lost_found done\n");
	return 0;
}

