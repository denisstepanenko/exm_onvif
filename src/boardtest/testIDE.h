#ifndef	TESTIDE_H
#define	TESTIDE_H
#include "multicast_ctl.h"


//long get_disk_total(char *mountpath);
int get_cf_avail(void);
int test_IDE(multicast_sock* ns, int* prog);
#endif
