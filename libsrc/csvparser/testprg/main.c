#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <file_def.h>
#include <gtthread.h>
#include <commonlib.h>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <csvparser.h>

#define DEVSET_LOCK_FILE "/lock/vserver/devset"
#define VERSION	"0.02"


int main(void)
{
	CSV_T* temp;
	int t_num;
	int i;
	printf("program started \n");
	temp = csv_create();
	if(temp==NULL)
	{
		printf("error create CSV \n");
	}
	csv_parse_string(temp, "inform ,                 1978   ,   01 ,   03  ,   201  ,    407   ,   end  ");
	t_num = csv_get_var_num(temp);
	printf("there are %d atoms\n", t_num);
	for(i=1;i<=t_num;i++)
	{
		printf("the %d atom=%s\n", i, csv_get_str(temp, i, "0"));
	}
	printf("%s",csv_get_string(temp));
	csv_destroy(temp);
	exit(0);
}
