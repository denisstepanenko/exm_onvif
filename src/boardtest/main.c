#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <stddef.h>
#include <unistd.h>
#include <getopt.h>
#include<gtlog.h>
#include <commonlib.h>
#include <nv_pair.h>
#include <devinfo.h>
#include "pub_err.h"
#include "testmod.h"
#include "multicast_ctl.h"
#include <guid.h>

#define RESULT_FILE_NAME	"/tmp/testbd.txt"
#define PACKAGE    			"testbd"
#define VERSION 				"0.02"
#define BDTEST_LOCK_FILE 	"/lock/ipserver/testbd"

/*
  v0.01 从vs3024上移植，作为第一个版本
*/	

multicast_sock net_port;
void print_help(int exval) 
{
	 printf("Program %s, Version %s \n", PACKAGE, VERSION); 
	 printf("%s [-h] [-V] [-n time ][-s time ] [-o FILE]\n\n", PACKAGE);

	 printf("  -h              print this help and exit\n");
	 printf("  -V              print version and exit\n\n");
//	 printf("  -u   second           秒间隔发送升级命令 \n");
   	 printf("  -I	<multicast IP address> 	 \n");
	 printf("  -P  	<multicast port number>            \n");
	 printf("  -F  	<flag>  发送标志          \n");
	 printf("  -E  	<enc_type> 加密类型           \n");
	 printf("  -D 	<server_ID>\n");
	 printf("  -S 	<packet sequent number>     \n");
	 printf("  -r 	<result file path>     \n");

	 exit(exval);
}

int env_parser(multicast_sock* net_st,unsigned char*result, int argc, char * argv[])
{
	int opt;
	int stat=0;
	char* p_str=NULL;
	struct GT_GUID guid_st;
	while((opt = getopt(argc, argv, "hVI:P:F:E:D:S:r:")) != -1) 
	{
		  switch(opt)
		  {
			   case 'h':
				print_help(0);
			    break;

			   case 'V':
				printf("%s %s\n\n", PACKAGE, VERSION); 
				exit(0);
			    break;

			   case 'I':
			   	p_str = optarg;
				memcpy(net_st->hostname, p_str, strlen(p_str));
//				printf("%s\n",p_str);
				stat++;
			   break;

			   case 'P':
				net_st->multi_port = atoi(optarg);
//				printf("%d\n", net_st->multi_port);	
				stat++;
			    break;

			   case 'F':
				net_st->flag= atoi(optarg);
//				printf("%d\n", net_st->flag);	
				stat++;
			    break;
			    
			   case 'E':
				net_st->enc_type= atoi(optarg);
//				printf("%d\n", net_st->enc_type);	
				stat++;
			    break;
			    
			   case 'D':
			   	p_str = optarg;
//				printf("%s\n",p_str);
			   	guid_st = hex2guid(p_str);
				memcpy(net_st->target_id, &guid_st, sizeof(guid_st));
				stat++;
			    break;

			  case 'S':
				p_str = optarg;
//				printf("%s\n",p_str);
				memcpy(net_st->seq_num, p_str, strlen(p_str));
				stat++;
			   break;

			  case 'r':
				p_str= optarg;
//				printf("%s\n",p_str);
				memcpy(result, p_str,strlen(p_str));
				result[strlen(p_str)]='\0';
			   break;
			   
			   case ':':
				fprintf(stderr, "%s: Error - Option `%c' needs a value\n\n", PACKAGE, optopt);
				print_help(1);
			    break;

			   case '?':
				fprintf(stderr, "%s: Error - No such option: `%c'\n\n", PACKAGE, optopt);
				print_help(1);
			    break;
			    default:
				stat = 0;
			    break;
		   }
	}
	return stat;
}

int main(int argc, char * argv[])
{
	int rc;
	int stat=0;
	int lock_file=-1;
//	NVP_TP *dist = NULL;
	char pbuf[100];
	struct dev_test_struct devstat;
	char result_filename[120];

	memset(result_filename,0,sizeof(result_filename));
	memcpy(result_filename, RESULT_FILE_NAME,strlen(RESULT_FILE_NAME));
	memset(&net_port, 0 ,sizeof(net_port));
	stat = env_parser(&net_port,result_filename, argc, argv);
	if(init_devinfo()<0)
	{
		gtlogerr("boardtest 模块: init devinfo error\n");		
		exit(1);
	}
	if(stat>=4)
	{
		if(init_dev_net_port(&net_port)<0)
		{
			printf("can not open net port%s : %d\n", net_port.hostname, net_port.multi_port);
			exit(1);
		}
	}
	
	lock_file=create_and_lockfile(BDTEST_LOCK_FILE);
	if(lock_file<=0)
	{
		printf("devset are running!!\n");
		gtlogerr("devset 模块已运行，故启动无效退出\n");		
		exit(0);
	}
	sprintf(pbuf,"%d\nversion:%s\n",getpid(),VERSION);
	write(lock_file,pbuf,strlen(pbuf)+1);//将进程的id号存入锁文件中

	printf("Version is %s\n", VERSION);
	memset((void*)&devstat,0,sizeof(devstat));
	send_test_report(&net_port, "start board test", 0);

//////////kill all programs//////////////////
	   system("killall -9 watch_proc\n");
	   system("killall -15 rtimage \n");
	   system("killall -15 hdmodule \n");
	   system("killall -9 ipmain \n");						// lsk 2006 -8-1
	   system("killall -15 encbox \n");						// lsk 2006 -10-11
	   system("clear\n");
	   sleep(1);
/////////////////////////////////////////////////	
	testsim(&devstat, &net_port, 5);								//测试硬件模块功能

	//将测试结果存放在文件中

	rc=save_result_to_file(result_filename, &devstat, &net_port);		//将测试结果存放在文件中
	printf("result file=%s rc=%d\n", result_filename, rc);
	system("/ip1004/swrbt\n");							//系统软复位
//	system("watch_proc &\n");
//lsk 2007-6-1
//	result_report(TEST_BD_FLAG, &net_port); //汇报结果
	close(lock_file);
	exit(rc);											//返回测试结果参数
}



