#include<unistd.h>
#include<dirent.h>
#include<string.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<stdio.h>
#include"../filelib.h"


int main (int argc,char *argv[])
{	

	const char *rm;
	const char * file_path;
	int val;

	if(argc == 1)
	{
		rm ="dirtest";
		file_path = "dirtest/1-1/1-1-3/1-1-3-1/1-1-3-1-1";
	}
	else
	{
		rm = argv[1];
		file_path = argv[2];
	}	

	printf("argc=%d\n",argc);
	printf("argv[0]=%s\n",argv[0]);
	printf("argv[1]=%s\n",argv[1]);
	printf("argv[2]=%s\n",argv[2]);
	//file_path = NULL;
	//rm = "dirtest";
//	file_path = "dirtest/1-1/1-1-3/1-1-3-1/1-1-3-1-1";
	printf("======main========\n");
	val = rm_file_select(rm,file_path);
	printf("======return main====\n");
	printf("val===%d\n",val);

	return(val);
}

