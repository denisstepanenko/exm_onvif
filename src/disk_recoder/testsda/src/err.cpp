/*
 * err.cpp
 *
 *  Created on: 2013-10-28
 *      Author: yangkun
 */
#include "err.h"
#include <cstdlib>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <printf.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

namespace gtsda
{
const char *cErr[]={
		"UNKNOW_ERR",
		"BLOCK_ERR_DATA_HEAD",
		"HD_ERR_READ",
		"HD_ERR_WRITE",
		"BLOCK_ERR_NULL",
		"BLOCK_ERR_ZERO",
		"BLOCK_ERR_FULL",
		"BLOCK_ERR_EMPTY",
		"BLOCK_ERR_NOT_IN",
		"BLOCK_ERR_NOT_ENOUGH",			/*10*/
		"BLOCK_ERR_DAY_SEC_MUT",
		"HD_ERR_OVER"

};



/************************************************************
 * 格式化输出大量的数据
 ************************************************************/
int myprint(const unsigned char  *p, long size)
{
	unsigned int i;
	for (i = 0; i < (unsigned int)size; i++)
	{
		if (i % 10 == 0)
		{
			cout  << std::hex << i<<endl;
			cout.width(8);
		}
		printf("    %-2X", *(p + i));
		//cout.setf(std::ios_base::hex, std::ios_base::basefield);
		//cout << *(p+i) ;
	}
	cout <<std::dec<< endl;
	return 0;
}



/* set advisory lock on file */
int lockfile(int fd)
{
	struct flock fl;
	fl.l_type = F_WRLCK; /* write lock */
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0; //lock the whole file
	return (fcntl(fd, F_SETLK, &fl));
}
int already_running(const char *filename)
{
	int fd;
	char buf[16];
	fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
	if (fd < 0)
	{
		cout <<"can't open %s: " << filename << endl;
		exit(1);
	}
	/* 先获取文件锁 */
	if (lockfile(fd) == -1)
	{
		if (errno == EACCES || errno == EAGAIN)
		{
			cout << "file: "<< filename << "already locked";
			close(fd);
			return 1;
		}
		cout << "can't lock %s: " << filename << endl;
		exit(1);
	}

	/* 写入运行实例的pid */
	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long) getpid());
	write(fd, buf, strlen(buf) + 1);
	return 0;

}
} /* namespace gtsda */
