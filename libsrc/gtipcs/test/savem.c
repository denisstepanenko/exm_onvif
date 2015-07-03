#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "mshm.h"




int main(int argc,char *argv[])
{
	int ret;
	long total;
	MEM_CORE *mc;
	FILE *fp;
	long key=1026;
	if(argc>1)
		key=atoi(argv[1]);
	mc=MShmCoreAttach(key,NULL);
	printf("MShmCoreAttach key:0x%x mc=%x  errno=%d:%s\n",(int)key,(int)mc,errno,strerror(errno));
	if(mc==NULL)
	{
		printf("can't attach mem key:0x%x\n",(int)key);
		exit(1);
	}
	printf("shm total size=%d usersize=%d\n",mc->tsize,mc->usize);
	total=mc->tsize;
	fp=fopen("shm.mem","w");
	if(fp==NULL)
		printf("can't open file %s!\n","shm.mem");
	ret=fwrite((void*)mc,1,total,fp);
	fclose(fp);
	printf("write %d bytes to file\n",ret);


	exit(0);
}
