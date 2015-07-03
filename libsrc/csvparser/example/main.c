#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <file_def.h>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <csvparser.h>
#include <gtthread.h>
#include <commonlib.h>

#define DEVSET_LOCK_FILE "/lock/vserver/devset"
//#define VERSION	"0.02"
//#define FILE_PATH	"/tmp/tt.txt"
#define FILE_PATH	"tt.txt"
#define FILL		"gtvs_csv.txt"
#define TEST_FILE	"/tmp/test.txt"
void printf_file(char* filename)
{
	int i;
	FILE *fp_temp = NULL;
	fp_temp = fopen(filename, "r");
	while((i=fgetc(fp_temp))!=EOF)
	{
		printf("%c",i);
	}
	fclose(fp_temp);
}
int main(void)
{
	CSV_T* temp[10];
	CSV_T* test=NULL;

	int t_num;
	int i;
	int j;
	int ret;
	int test_fd= -1;
	FILE* test_fp=NULL;
//	struct flock tp;
	printf("program started \n");
#if 0
	test_fp = fopen(TEST_FILE, "w+");
	test_fd = fileno(test_fp);
	if(test_fp==NULL)
	perror("open file ");
	else
	printf("open file ok, fd =%d\n", test_fd);
///////test
	if(test_fd<0)
		exit(1);
	tp.l_type = F_WRLCK;
	tp.l_whence = SEEK_SET;
	tp.l_len = 0;
	tp.l_pid = getpid();
	if(fcntl(test_fd, F_SETLK, &tp)<0)
	{
		perror("lock file error");
	}
	printf("lock file ok \n");
	fclose(test_fp);
//	exit(0);
///////////	
#endif
#if 0
	test_fp = fopen(TEST_FILE, "w+");
	test_fd = fileno(test_fp);
	if(test_fp==NULL)
	perror("open file ");
	else
	printf("open file ok, fd =%d\n", test_fd);
	printf("fd = %d ,F_WRLCK = %d \n", test_fd, F_WRLCK);
	fprintf(test_fp, "%s", "ttt.............\n");
	printf("write to file\n");
	if(force_lockfile(test_fd, F_WRLCK|F_RDLCK, 1)<0)
	{
		perror("lock file error");
	}
	fprintf(test_fp, "%s", "testting\n");
	fprintf(test_fp, "%s", "testting\n");
	fprintf(test_fp, "%s", "testting\n");
	fprintf(test_fp, "%s", "testting\n");
	fprintf(test_fp, "%s", "testting\n");
	fflush(test_fp);
	sleep(20);
	close(test_fd);
	exit(0);
#endif
#if 1
	for(i=0;i<10;i++)
	{
		temp[i] = NULL;
		temp[i] = csv_create();
		if(temp[i]==NULL)
		{
			printf("error create CSV \n");
		}
	}
	csv_parse_string(temp[0], "inform ,                 1978   ,   01 ,     ,   201  ,    407   ,   end  ");
	csv_parse_string(temp[1], "inform ,                 1978   ,   02 ,     ,   201  ,    407   ,   end  ");
	csv_parse_string(temp[2], "inform ,                 1978   ,   03 ,,   201  ,    407   ,   end  ");
	csv_parse_string(temp[3], "inform ,                 1978   ,   04 ,,   201  ,    407   ,   end  ");
	csv_parse_string(temp[4], "inform ,                 1978   ,   05 ,  ,   201  ,    407   ,   end  ");
	csv_parse_string(temp[5], "inform ,                 1978   ,   06 , ,   201  ,    407   ,   end  ");
	csv_parse_string(temp[6], "inform ,                 1978   ,   07 ,   ,   201  ,    407   ,   end  ");
	csv_parse_string(temp[7], "inform ,                 1978   ,   08 ,    ,   201  ,    407   ,   end  ");
	csv_parse_string(temp[8], "inform ,                 1978   ,   09 ,     ,   201  ,    407   ,   end  ");
	csv_parse_string(temp[9], "inform ,                 1978   ,   10 ,     ,   201  ,    407   ,   end  ");
	for(i=0;i<10;i++)
	{
		t_num = csv_get_var_num(temp[i]);
		csv_set_str(temp[i], t_num+1, "clear");
		csv_set_int(temp[i], t_num+2, 128);
		t_num = csv_get_var_num(temp[i]);
#if 0
		printf("there are %d atoms\n", t_num);
		for(j=1;j<=t_num;j++)
		{
			printf("the %d atom=%s\n", j, csv_get_str(temp[i], j, "0"));
		}
		printf("%s",csv_get_string(temp[i]));
		printf("%d\n", csv_get_int(temp[i], 3, 133));
		printf("%d\n", csv_get_int(temp[i], 2, 10));
		sleep(1);
#endif
	}
	
#if 0
	t_num = csvfile_get_total_records(FILE_PATH);
	printf("%s has %d records\n", FILE_PATH, t_num);
	test = csv_create();
	printf("%s has %d records\n",FILL, csvfile_get_record(FILL, 1, test));
	printf("%s", csv_get_string(test));
	printf("..............\n %s \n", csv_get_str(test, 1, "hello"));
	printf("..............\n %d \n", csv_get_int(test, 1, 203));
	for(i=0;i<10;i++)
	{
		ret = csvfile_set_record(FILE_PATH, -100, temp[i]);
//		printf("set record ret =%d\n", ret);
	}
	for(i=0;i<10;i++)
	{
		ret = csvfile_insert_record(FILE_PATH, -10, temp[i]);
//		printf("insert record ret =%d\n", ret);
	}
#endif
	for(i=0;i<10;i++)
	{
		ret = csvfile_insert_record(FILE_PATH, -1,temp[i]);
//		printf("set record tail =%d\n", ret);
//		ret = csvfile_insert_record(FILE_PATH, 1, temp[i]);
//		printf("set record head =%d\n", ret);
	}
	t_num = csvfile_get_total_records(FILE_PATH);
	printf("%s has %d records\n", FILE_PATH, t_num);
	printf_file(FILE_PATH);
	sleep(5);
//	csvfile_set_record(FILE_PATH, 3,temp[4]);
//	csvfile_set_record(FILE_PATH, 4,temp[4]);
//	csvfile_set_record(FILE_PATH, 5,temp[4]);
	csvfile_get_record(FILE_PATH, 1, temp[0]);
        csv_set_str(temp[0],1,"xxxxxxx");
        csvfile_set_record(FILE_PATH,1,temp[0]);
        printf("changed \n");
 	printf_file(FILE_PATH);
       sleep(5);
	csvfile_get_record(FILE_PATH, 2, temp[0]);
        csv_set_str(temp[0],1,"xxxxxxx");
        csvfile_set_record(FILE_PATH,2,temp[0]);
        printf("changed \n");
	printf_file(FILE_PATH);
        sleep(5);
	csvfile_rm_record(FILE_PATH, 1);
	csvfile_rm_record(FILE_PATH, -1);
        printf("changed \n");
	printf_file(FILE_PATH);
        sleep(5);
	csvfile_rm_record(FILE_PATH, 0);
	csvfile_rm_record(FILE_PATH, 10);
        printf("changed \n");
	printf_file(FILE_PATH);
	
#if 0
	for(i=1;i<=t_num;i++)
	{
		printf("the %d atom=%s\n", i, csv_get_str(temp, i, "0"));
	}
#endif
	for(i=0;i<10;i++)
	{
		csv_destroy(temp[i]);
	}
#endif
	exit(0);
}
