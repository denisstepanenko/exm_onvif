#include<unistd.h>
#include<dirent.h>
#include<string.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<stdio.h>
#include<ftw.h>
#include"../filelib.h"
//#include"memwatch.h"




/*****************************************
err_fn 需要自己定义，下面的为演示
*****************************************/
int err_fn_user(const char *dir_file,int errnum)
{
	printf("in the err_fn\n");
//	printf("#######################===%s=====%d\n",dir_file,errnum);
//	printf("stop\n");
	return (-1);
}


/******************************************
fn函数需要自己定义，下面的为演示
******************************************/
int fn_user(const char *file,const struct stat *sb,int flag,void *user_data)
{
	char *dirtest = NULL;

	printf("user_data=[%s]\n",(char *)user_data);	
	printf("in the fn===file==\t%s\n",file);
	
	//printf("\t\t\tflag = %d\n",flag);
		sb = sb;
		flag = flag;
		/*当目录名中含有"2d-2d"时返回1值,用来测试:

		a)   当返回值为FN_RETURN_CONTINUE时，结束对当前节点
		及子节点的遍历和操作（如判断是否需要删除空
		目录等操作），跳转到当前节点的下一个平行节
		点去遍历；

		*/
		dirtest = strstr(file,"1e-13e");
		if(dirtest != 0 )
		{	
			printf("found 1e-1e \n");
			return FN_RETURN_CONTINUE;
		}

		/*当目录名中含有"4b-1b"时，返回不是FN_RETURN_CONTINUE的其他
		非零值，用来测试:

		b)  当返回值为其他非零值时，结束遍历，
		     将该值返回给用户；
		*/
		dirtest = strstr(file,"13-1e4");
		//ret = strcmp(file,"/hqdata/2007/06/14/05/HQ_C00_D20070614050744_L10_T00.AVI");
		if(dirtest != 0 )
		{	
			printf("found 4b-1b\n");
			return 4;
		}
		
		return 0;
	
}
	
/**************************************
主函数
***************************************/
int main(int argc,char *argv[])
{
	
	int ftw_re_val = 0;
	char *dir;	
	char *str_test="user_data_test";
	

	if(argc == 1)
	{
		dir ="dirtest";
	}
	if(argc == 2)
	{
		dir = argv[1];
	}
	
	//char * dir ="/hqdata";
	printf("================================\n");
	printf("========DIR_DEPTH=================\n");
		
	printf("the target dir= %s\n",dir);
	printf("++++++++++++++++++before entry the check_dir()\n");

	printf("==main()=done......... to=the==%s=\n",get_current_dir_name());

	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	int while_cnt = 0;

//	while(1)
	{
			ftw_re_val = ftw_sort_user(dir,fn_user,(void *)str_test,1,0,err_fn_user);
			
			printf("==ftw_sort_return=====%d\n",ftw_re_val);
		
			printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@===%d\n",while_cnt);
		while_cnt ++;		
	//getchar();
		sleep(1);
	}
	return (0);
}


