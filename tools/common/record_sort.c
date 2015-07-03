/*
 	@ sort the name of video recorders
 	@aah 09.04.03
 	@aah modified in 09.09.07
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

///存储录像名的结构
typedef struct{
	char key[80];
	struct record_key_t *next;
}record_key_t;

static int record_num=0;///文件名总行数

/*
 	对比录像日期先后，大于key2返回正值；小于返回负值；等于返回0
 */
static int record_compare(const void *key_1, const void *key_2)
{
	int num_1 = 0;
	int num_2 = 0;
	char *k1=NULL;
	char *k2=NULL;
	int ret;


	k1=*(char **)key_1;
	k2=*(char **)key_2;
	
	num_1 = atoi((char *)k1+44);
	num_2 = atoi((char *)k2+44);

	ret = memcmp(((char *)k1+28), ((char *)k2+28), 14);
	if(ret < 0)
		return -1;
	
	else if(ret > 0)
		return 1;
	
	else if(ret == 0)
	{
		if(num_1 < num_2)
			return -1;
		else if(num_1 > num_2)
			return 1;
		else
			return 0;
	}

	printf("record_compare failed-\n");
	return -1;
}

/*
	获取录像名字，并存储到单链表中	
 */
static int record_getnames(FILE **fp, record_key_t **head)
{
	char 	*rec=NULL;
	record_key_t  *p1=NULL;
	record_key_t  *p2=NULL;
	int 		ret;
	
	if(fp==NULL)
	{
		printf("params error in record_getnames\n");
		return -1;
	}
	
	*head=(record_key_t *)malloc(sizeof(record_key_t ));
	if(head==NULL)
	{
		printf("malloc  error in record_getnames\n");
		return -1;
	}
	
	p1=*head;
	rewind(*fp);
	while(1)
	{
		rec=fgets(p1->key, 80, *fp);
		if(rec==NULL)
		{
			ret=feof(*fp);
			if(ret==0)
			{
				printf("fgets error\n");	
				return -1;
			}
			else
			{
				p1->next=NULL;
				//printf("read the end\n");
				break;
			}
		}
		p2=(record_key_t *)malloc(sizeof(record_key_t ));
		if(p2==NULL)
		{
			printf("malloc head error\n");
			return -1;
		}
		p1->next=(struct record_key_t *)p2;
		p1=p2;
		record_num++;
		
	}


	return 0;
}



/*
	@brief: 在单链表中排序,按日期排序录像名
	@param: plist链表头部地址, record_num文件名条数
	@return: 成功返回0，失败负值
 */
static int record_sort(record_key_t  *plist, FILE *fp_w)
{
	record_key_t  *p1=NULL;
	char 		*record_vla[record_num];
	int			i=0;

	
	p1=plist;
	if(plist==NULL)
	{
		return -1;
	}
	else
	{
		///传递每条的文件名指针赋给动态指针数组
		for(i=0; i<record_num; i++)
		{
			if(p1->key==NULL)
			{
				printf("record_sort 行指针赋值给动态数组错误\n");
				return -1;
			}
			record_vla[i] = p1->key;
			p1 = (record_key_t *)p1->next;
		}

		///进行快速排序
		qsort(record_vla, record_num, sizeof(char *), record_compare);

		///写入到文件中
		for(i=0; i<record_num; i++)
		{
			//printf("%s\n",record_vla[i]);
			fputs(record_vla[i], fp_w);
			fflush(fp_w);
		}
			
	}
	
	return 0;
}


static void record_help(void)
{
	printf("usage: ./record_sort [--read files] [--write files]\n");
	exit(1);
}

int  main(int argc, char **argv)
{


	FILE *fp_r=NULL;
	FILE *fp_w=NULL;
	record_key_t  *list_head=NULL;
	record_key_t  *plist=NULL;
	int ret;

	if((argc==2)&&(strcmp(argv[1], "-h")==0))
	{
		record_help();
		exit(0);
	}
	else if(argc!=3)
	{
		record_help();
		exit(1);
	}

	if(strcmp(argv[1], argv[2])==0)
	{
		printf("Please input different file names\n");
		exit(1);
	}
	
	fp_r=fopen(argv[1], "r");
	if(fp_r==NULL)
	{
		printf("open %s failed\n",argv[1]);
		exit(1);
	}
	fp_w=fopen(argv[2], "w");
	if(fp_w==NULL)
	{
		printf("open %s failed\n",argv[2]);
		exit(1);
	}

	ret=record_getnames(&fp_r, &list_head);
	if(list_head==NULL)
	{
		printf("record_getnames failed\n");
		fclose(fp_r);
		fclose(fp_w);
		exit(1);
	}
	
	ret=record_sort(list_head, fp_w);
	if(ret==-1)
	{
		printf("record_sort error\n");
		fclose(fp_r);
		fclose(fp_w);
		exit(1);
	}

	plist=list_head;
	do
	{
		list_head=plist;
		plist=list_head->next;
		free(list_head);
	}while(plist!=NULL);

	fclose(fp_r);
	fclose(fp_w);
	exit(0);
}


