#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gtlog.h>
#include <iniparser.h>
#include "ini_conv.h"
#include "ini_conv_init.h"

static char *str_gt1;		///<接收gt1000的第一个节点名key指向的字符串地址
static char *str_gt2;		///<接收gt1000的第二个节点名key指向的字符串地址
static char *str1;			///<接收yy第一个配置文件key指向字符串地址
static char *str2;			///<接收yy第二个配置文件key指向字符串地址
static int val_temp;		///<修改cmd_port的临时存放变量
static char str_temp[32];	///<修改guid的临时存放数组

static int is_ini_changed = NO_CHANGE;							///<判断配置文件是否修改过，有则保存
static int conv_total = sizeof(conv_ini_all) / sizeof(conv_ini_t);		///<计算结构数组总数
static int spcial_total = sizeof(conv_spcial) / sizeof(conv_spcial_t);	///<计算特殊节点数组个数


static int help(void)
{
	printf("\n欢迎使用ini_conv配置文件转换工具!(version %s)\n\n", INI_CONV_VER);
	printf("使用说明: ini_conv [OPTION]\n");
	printf("\t-h\t输出帮助信息\n");
	printf("\t-v\t输出版本信息\n");
	printf("\t-m\t二转一，即%s 和%s  -> %s\n", VIRDEV_0_INI, VIRDEV_1_INI, MAIN_INI);
	printf("\t-s\t一转二，即 %s -> %s 和%s\n", MAIN_INI, VIRDEV_0_INI, VIRDEV_1_INI);
	return -1;
}

/**
	@brief: 	compare two strings , to see if the same
	@return:	0 is same; -1 is different
*/
static int str_compare(char *pstr1, char *pstr2)
{
	if(pstr2 != NULL)
	{
		if(strncmp(pstr1, pstr2, strlen(pstr2)) != 0)
		{
			return 0;
		}
	}
	
	return -1;	
}

/**
	@brief: 	在循环转换前需要对一些特殊节点进行特殊转换
	@param:	direction为0，是-s前进行转换；为1反之；
	@param:	d, d1, d2为ini结构描述指针；
	@return:
*/
static int spcial_proc(dictionary * d, dictionary *d1, dictionary *d2, int direction)
{
	int i;
	int port_num;

	/**
		-s 前特殊节点处理
		处理devinfo和port
	*/
	if(direction == CONV_S)
	{
		///转换前需要先把特殊环节进行转换，便于以后转换
		///<赋值给gt1000.ini  devinfo_dev2 的guid
		str_gt1 = iniparser_getstring(d, conv_spcial[0].sec_key_gt1, NO_FOUND_STR);
		str_gt2 = iniparser_getstring(d, conv_spcial[0].sec_key_gt2, NO_FOUND_STR);
		if(strcmp(str_gt1, NO_FOUND_STR) != 0)
		{
			if(strcmp(str_gt2, NO_FOUND_STR) == 0)
			{
				is_ini_changed = CHANGE_SAVE;
			}
			snprintf(str_temp, sizeof(str_temp), "%s", str_gt1);
			str_temp[1] = '1';
			str_gt2 = str_temp;
			iniparser_setstr(d, conv_spcial[0].sec_key_gt2, str_gt2);
		}
		///<赋值给gt1000.ini  port_dev2的cmd_port/com0_port/com1_port的key
		for(i = 1; i < 4; i++)
		{	
			if(i==1)
			{
				port_num = 100;
			}
			else
			{
				port_num = 1;
			}
			str_gt1 = iniparser_getstring(d, conv_spcial[i].sec_key_gt1, NO_FOUND_STR);
			str_gt2 = iniparser_getstring(d, conv_spcial[i].sec_key_gt2, NO_FOUND_STR);
			if(strcmp(str_gt1, NO_FOUND_STR) != 0)
			{
				if(strcmp(str_gt2, NO_FOUND_STR) == 0)
				{
					is_ini_changed = CHANGE_SAVE;
				}
				val_temp = atoi(str_gt1)+port_num;
				iniparser_setint(d, conv_spcial[i].sec_key_gt2, val_temp); 
			}
		}
		///<赋值给gt1000.ini  其它port_dev2的key
		for(i = 4; i < spcial_total; i++)
		{
			str_gt1 = iniparser_getstring(d, conv_spcial[i].sec_key_gt1, NO_FOUND_STR);
			str_gt2 = iniparser_getstring(d, conv_spcial[i].sec_key_gt2, NO_FOUND_STR);
			if(strcmp(str_gt1, NO_FOUND_STR) != 0)
			{
				if(strcmp(str_gt2, NO_FOUND_STR) == 0)
				{
					is_ini_changed = CHANGE_SAVE;
				}
				iniparser_setstr(d, conv_spcial[i].sec_key_gt2, str_gt1); 
			}
		}
			
	}
	/**
		-m 转换前特殊节点处理
		表从第二个开始，第一个属于-s only
		这里只处理port
	*/
	else
	{
		///<a.ini赋值给b.ini  的cmd_port/com0_port/com1_port
		for(i = 1; i < 4; i++)
		{	
			if(i==1)
			{
				port_num = 100;
			}
			else
			{
				port_num = 1;
			}
			str1 = iniparser_getstring(d1, conv_spcial[i].sec_key_a, NO_FOUND_STR);
			str2 = iniparser_getstring(d2, conv_spcial[i].sec_key_b, NO_FOUND_STR);
			if(strcmp(str1, NO_FOUND_STR) != 0)
			{
				if(strcmp(str2, NO_FOUND_STR) == 0)
				{
					is_ini_changed = CHANGE_SAVE;
				}
				val_temp = atoi(str1)+port_num;
				iniparser_setint(d2, conv_spcial[i].sec_key_b, val_temp); 
			}
		}
		///<a.ini赋值给b.ini  其它port
		for(i = 4; i < spcial_total; i++)
		{
			str1 = iniparser_getstring(d1, conv_spcial[i].sec_key_a, NO_FOUND_STR);
			str2 = iniparser_getstring(d2, conv_spcial[i].sec_key_b, NO_FOUND_STR);
			if(strcmp(str1, NO_FOUND_STR) != 0)
			{
				if(strcmp(str2, NO_FOUND_STR) == 0)
				{
					is_ini_changed = CHANGE_SAVE;
				}
				iniparser_setstr(d2, conv_spcial[i].sec_key_b, str1); 
			}
		}
	}

	return 0;
}

/**
	@brief: 	对比输入key内容是否改变，若改变，则进行ini文件间key值的转换
	@param: d,d1,d2分别是描述gt1000和另外两个ini文件的结构
	@param: conv_ini是指向要转换的配置文件变量的结构指针
	@param: conv_direction是转换方向-s/-m,0为-s，1为-m
	@return: 成功返回0	失败返回负值
*/
static int comp_conv(dictionary * d, dictionary *d1, dictionary *d2, conv_ini_t *conv_ini, int conv_direction)
{
	/*
		开始gt1000到其它配置文件的转换
	*/
	if(conv_direction == CONV_S)
	{
		///判断gt1000.ini中是否存在此节点，防止乱增加节点
		str_gt1= iniparser_getstring(d, conv_ini->sec_key_gt1, NO_FOUND_STR);
		///如果不存在，除了xxx,yyy且非-s only那项外，其它直接返回
		if(strcmp(str_gt1, NO_FOUND_STR) == 0)
		{
			printf("%s 没有%s节\n", MAIN_INI, conv_ini->sec_key_gt1);
			//gtloginfo("%s 没有%s节\n", MAIN_INI, conv_ini->sec_key_gt1);
			
			if((conv_ini->conv_att == NO_ACCORD2_A)&&(conv_ini->is_s_only == NOT_S_ONLY)&&(conv_ini->sec_key_gt2 != NULL))
			{
				str_gt2 = iniparser_getstring(d, conv_ini->sec_key_gt2, NO_FOUND_STR);
				if(strcmp(str_gt2, NO_FOUND_STR) == 0)
				{
					printf("%s 没有%s节\n", MAIN_INI, conv_ini->sec_key_gt2);
					//gtloginfo("%s 没有%s节\n", MAIN_INI, conv_ini->sec_key_gt2);
					return -1;
				}
				else
				{
					str2 = iniparser_getstring(d2, conv_ini->sec_key_b, NO_FOUND_STR);
					if(strncmp(str2, str_gt2, MAX(str2, str_gt2)) != 0)
					{
						gtloginfo("%s中%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt2);
						printf("%s中%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt2);
					
						iniparser_setstr(d2, conv_ini->sec_key_b, str_gt2);///<赋值给b.ini
					
						///说明有修改过
						is_ini_changed = CHANGE_SAVE;
					}
				}
			}
			else
			{
				return -1;
			}
		}
		///存在的话都需要进行转换
		else
		{
			///读取其它节点key，以便对比
			str_gt2 = iniparser_getstring(d, conv_ini->sec_key_gt2, NO_FOUND_STR);
			str1 = iniparser_getstring(d1, conv_ini->sec_key_a, NO_FOUND_STR);
			str2 = iniparser_getstring(d2, conv_ini->sec_key_b, NO_FOUND_STR);

			///dev2相关节点转换
			if(str_compare(conv_ini->sec_key_gt1, conv_ini->sec_key_gt2) == 0)
			{	
				if(strncmp(str1, str_gt1, MAX(str1, str_gt1)) != 0)
				{	
					gtloginfo("%s中%s:\t%s->%s\n", VIRDEV_0_INI, conv_ini->sec_key_a, str1, str_gt1);
					printf("%s中%s:\t%s->%s\n", VIRDEV_0_INI, conv_ini->sec_key_a, str1, str_gt1);
	
					iniparser_setstr(d1, conv_ini->sec_key_a, str_gt1);///<赋值给a.ini
					is_ini_changed = CHANGE_SAVE;
				}
				if(strcmp(str_gt2, NO_FOUND_STR) == 0)
				{
					printf("%s 没有%s节\n", MAIN_INI, conv_ini->sec_key_gt2);
					return -1;
				}
				else if(strncmp(str2, str_gt2, MAX(str2, str_gt2)) != 0)
				{
					gtloginfo("%s中%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt2);
					printf("%s中%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt2);
	
					iniparser_setstr(d2, conv_ini->sec_key_b, str_gt2);///<赋值给b.ini
					is_ini_changed = CHANGE_SAVE;
				}
			}
			///同名字节点转换
			else
			{
				if(conv_ini->is_s_only == IS_S_ONLY)
				{
					if(strncmp(str1, str_gt1, MAX(str1, str_gt1)) != 0)
					{
						gtloginfo("%s中%s:\t%s->%s\n", VIRDEV_0_INI, conv_ini->sec_key_a, str1, str_gt1);
						printf("%s中%s:\t%s->%s\n", VIRDEV_0_INI, conv_ini->sec_key_a, str1, str_gt1);
	
						iniparser_setstr(d1, conv_ini->sec_key_a, str_gt1);///<赋值给a.ini
						is_ini_changed = CHANGE_SAVE;
					}
					if(strncmp(str2, str_gt1, MAX(str2, str_gt1)) != 0)
					{
						gtloginfo("%s中%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt1);
						printf("%s中%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt1);
	
						iniparser_setstr(d2, conv_ini->sec_key_b, str_gt1);///<赋值给b.ini
						is_ini_changed = CHANGE_SAVE;
					}
				}
				else 
				{
					if(strncmp(str1, str_gt1, MAX(str1, str_gt1)) != 0)
					{
						gtloginfo("%s中%s:\t%s->%s\n", VIRDEV_0_INI, conv_ini->sec_key_a, str1, str_gt1);
						printf("%s中%s:\t%s->%s\n", VIRDEV_0_INI, conv_ini->sec_key_a, str1, str_gt1);
	
						iniparser_setstr(d1, conv_ini->sec_key_a, str_gt1);///<赋值给a.ini
						is_ini_changed = CHANGE_SAVE;
						if(conv_ini->conv_att == IS_ACCORD2_A)
						{
							gtloginfo("%s中%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt1);
							printf("%s中%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str_gt1);
	
							iniparser_setstr(d2, conv_ini->sec_key_b, str_gt1);///<赋值给b.ini
							is_ini_changed = CHANGE_SAVE;
						}
					}

				}
			}
			
		}///<end 节点存在的情况
		
	}///<end -s mode
	///start -m mode!!!
	/*
		开始其它配置文件向gt1000里转换
	*/
	else
	{
		///单向转换节点直接返回
		if(conv_ini->is_s_only == IS_S_ONLY)
		{
			return -1;
		}
		///判断a.ini中是否存在此节点，防止乱增加节点
		str1 = iniparser_getstring(d1, conv_ini->sec_key_a, NO_FOUND_STR);
		///如果不存在，除了xxx,yyy且非-s only那项外，其它直接返回
		if(strcmp(str1, NO_FOUND_STR) == 0)
		{
			printf("%s 没有%s节\n", VIRDEV_0_INI, conv_ini->sec_key_a);
			//gtloginfo("%s 没有%s节\n", VIRDEV_0_INI, conv_ini->sec_key_a);
			
			if((conv_ini->conv_att == NO_ACCORD2_A)&&(conv_ini->is_s_only == NOT_S_ONLY)&&(conv_ini->sec_key_gt2 != NULL))
			{ 
				str2 = iniparser_getstring(d2, conv_ini->sec_key_b, NO_FOUND_STR);
				if(strcmp(str2, NO_FOUND_STR) == 0)
				{
					printf("%s 没有%s节\n", VIRDEV_1_INI, conv_ini->sec_key_b);
					//gtloginfo("%s 没有%s节\n", VIRDEV_1_INI, conv_ini->sec_key_b);
					return -1;
				}
				else
				{
					str_gt2 = iniparser_getstring(d, conv_ini->sec_key_gt2, NO_FOUND_STR);
					if(strncmp(str2, str_gt2, MAX(str2, str_gt2)) != 0)
					{
						gtloginfo("%s中%s:\t%s->%s\n", MAIN_INI, conv_ini->sec_key_gt2, str_gt2, str2);
						printf("%s中%s:\t%s->%s\n", MAIN_INI, conv_ini->sec_key_gt2, str_gt2, str2);
					
						iniparser_setstr(d, conv_ini->sec_key_gt2, str2);///<赋值给gt1000.ini
					
						///说明有修改过
						is_ini_changed = CHANGE_SAVE;
					}
				}
			}
			else
			{
				return -1;
			}
		}
		///a.ini中节点存在的话
		else
		{
			str2 = iniparser_getstring(d2, conv_ini->sec_key_b, NO_FOUND_STR);
			str_gt1 = iniparser_getstring(d, conv_ini->sec_key_gt1, NO_FOUND_STR);
			str_gt2 = iniparser_getstring(d, conv_ini->sec_key_gt2, NO_FOUND_STR);
				
			if(strncmp(str1, str_gt1, MAX(str1, str_gt1)) != 0)
			{
				gtloginfo("%s中%s:\t%s->%s\n", MAIN_INI, conv_ini->sec_key_gt1, str_gt1, str1);
				printf("%s中%s:\t%s->%s\n", MAIN_INI, conv_ini->sec_key_gt1, str_gt1, str1);
	
				iniparser_setstr(d, conv_ini->sec_key_gt1, str1);///<赋值给gt1000.ini
				is_ini_changed = CHANGE_SAVE;
				if((conv_ini->conv_att == IS_ACCORD2_A)&&(str_compare(conv_ini->sec_key_gt1, conv_ini->sec_key_gt2) != 0))
				{
					gtloginfo("%s中%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str1);
					printf("%s中%s:\t%s->%s\n", VIRDEV_1_INI, conv_ini->sec_key_b, str2, str1);
	
					iniparser_setstr(d2, conv_ini->sec_key_b, str1);///<赋值给gt1000.ini
				}
			}
			///对于yyy节点的还需要与b.ini对比下
			if(str_compare(conv_ini->sec_key_gt1, conv_ini->sec_key_gt2) == 0)
			{
				if(strcmp(str2, NO_FOUND_STR) == 0)
				{
					printf("%s 没有%s节\n", VIRDEV_1_INI, conv_ini->sec_key_b);
					return -1;
				}
				else if(strncmp(str2, str_gt2, MAX(str2, str_gt2)) != 0)
				{
					gtloginfo("%s中%s:\t%s->%s\n", MAIN_INI, conv_ini->sec_key_gt2, str_gt2, str2);
					printf("%s中%s:\t%s->%s\n", MAIN_INI, conv_ini->sec_key_gt2, str_gt2, str2);
	
					iniparser_setstr(d, conv_ini->sec_key_gt2, str2);///<赋值给gt1000.ini _dev2
					is_ini_changed = CHANGE_SAVE;
				}
			}
		}///<end a.ini节点存在
	}///<end -m mode
	
	return 0;
}	


/**
	@brief:   将gt1000.ini 转到两个配置文件中
	@param: conv_direc为0是执行-s，为1执行-m
	@return: 成功返回0，失败返回负值
*/
static int s2m_or_m2s(int conv_direc)
{
	dictionary *d;		///<配置文件结构描述指针
	dictionary *d1;
	dictionary *d2;
	FILE *lock;		///<锁配置文件指针
	FILE *lock1;
	FILE *lock2;
	int i;	

	
	///打开需要转换的ini配置文件
	d = iniparser_load_lockfile(MAIN_INI, 0, &lock);
	if(d == NULL)
	{
		printf("打开%s失败\n", MAIN_INI);
		gtloginfo("打开%s失败\n", MAIN_INI);
		return -1;
	}
	d1 = iniparser_load_lockfile(VIRDEV_0_INI, 0, &lock1);
	if(d1 == NULL)
	{
		printf("打开%s失败\n", VIRDEV_0_INI);
		gtloginfo("打开%s失败\n", VIRDEV_0_INI);
		return -1;
	}
	d2 = iniparser_load_lockfile(VIRDEV_1_INI, 0, &lock2);
	if(d2 == NULL)
	{
		printf("打开%s失败\n", VIRDEV_1_INI);
		gtloginfo("打开%s失败\n", VIRDEV_1_INI);
		return -1;
	}

	if(conv_direc == CONV_S)
	{
		///对devinfo和cmd_port先进行转换到gt1000自身的dev2节点中
		spcial_proc(d, d1, d2, CONV_S);
		
		///开始进行-s 转换
		gtloginfo("启动ini_conv, version:%s, -s mode\n", INI_CONV_VER);
		printf("启动ini_conv, version:%s, -s mode\n", INI_CONV_VER);
		
		for(i = 0; i < conv_total; i++)
		{
			comp_conv( d,  d1,  d2,  (conv_ini_t *)(conv_ini_all+i), CONV_S);
		}
	}
	
	else if(conv_direc == CONV_M)
	{
		///把a.ini的cmd_port + 1000赋值到b.ini中,其它port不变赋值到b.ini中
		spcial_proc(d, d1, d2, CONV_M);
		
		///开始进行-m 转换
		gtloginfo("启动ini_conv, version:%s, -m mode\n", INI_CONV_VER);
		printf("启动ini_conv, version:%s, -m mode\n", INI_CONV_VER);

		for(i = 0; i < conv_total; i++)
		{
			comp_conv( d,  d1,  d2,  (conv_ini_t *)(conv_ini_all+i), CONV_M);
		}

	}
	else
	{
		///<不执行任何函数
	}
	
	///没有改动的话，不进行存储
	if(is_ini_changed == NO_CHANGE)
	{
		fclose(lock);
		fclose(lock1);
		fclose(lock2);
		if(conv_direc == CONV_S)
		{
			printf("ini_conv, version:%s, -s mode, 未修改配置文件，退出\n", INI_CONV_VER);
			gtloginfo("ini_conv, version:%s, -s mode, 未修改配置文件，退出\n", INI_CONV_VER);
		}
		else
		{
			printf("ini_conv, version:%s, -m mode, 未修改配置文件，退出\n", INI_CONV_VER);
			gtloginfo("ini_conv, version:%s, -m mode, 未修改配置文件，退出\n", INI_CONV_VER);
		}
		
	}
	
	///有改动，存储到其他配置文件中
	else
	{
		save_inidict_file(MAIN_INI, d, &lock);
		save_inidict_file(VIRDEV_0_INI, d1, &lock1);
		save_inidict_file(VIRDEV_1_INI, d2, &lock2);
		if(conv_direc == CONV_S)
		{
			printf("ini_conv, version:%s, -s mode, 修改了配置文件，退出\n", INI_CONV_VER);
			gtloginfo("ini_conv, version:%s, -s mode, 修改了配置文件，退出\n", INI_CONV_VER);
		}
		else
		{
			printf("ini_conv, version:%s, -m mode, 修改了配置文件，退出\n", INI_CONV_VER);
			gtloginfo("ini_conv, version:%s, -m mode, 修改了配置文件，退出\n", INI_CONV_VER);
		}
	}
		
	iniparser_freedict(d);
	iniparser_freedict(d1);
	iniparser_freedict(d2);
		
	return 0;
}


int main(int argc, char **argv)
{

	int cmd_val;	///< 命令行处理后返回参数
	//int ret;		///<转换函数返回值


	///打开日志
	gtopenlog("ini_conv");
	
	
	///处理命令行参数
	if(argc != 2)
	{
		help();
		return -1;
	}
	
	opterr = 0 ;
	while( ( cmd_val = getopt( argc, argv, "smhv" ) ) != -1 )
	{
		switch( cmd_val )
		{
			case 's' :
				///两个配置文件转换到ip1004.ini中
				s2m_or_m2s(CONV_S);
				break;
				
			case 'm' :
				///ip1004.ini转换到两个配置文件中
				s2m_or_m2s(CONV_M);
				break;

			case 'v' :
				///打印版本号
				printf("Ini_conv verion is %s\n", INI_CONV_VER);
				break;

			case 'h' :
				///输出帮助信息
				
			default:
				help();
				break;
		}
	}
	
	
	return 0;
}
