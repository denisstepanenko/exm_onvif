/*
	lsk 2010-2-11 GPS功能支持
	
*/
#include "ipmain.h"
#include <commonlib.h>
#include "watch_process.h"
#include "netcmdproc.h"
#include "ipmain_para.h"
#include "alarm_process.h"
#include <sys/types.h>
#include "netinfo.h"
#include "gate_connect.h"
#include "maincmdproc.h"
#include "watch_board.h"
#include "devstat.h"
#include "infodump.h"
#include "hdmodapi.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <gate_cmd.h>
//#include <gpsapi.h>
#include <iniparser.h>
#include <csvparser.h>
#include "gpsupport.h"
//#ifdef  GPS_SUPPORT


/*
	int gps_state; 
*	      0:    正常定位
*	    -1:     通讯端口未初始化
*         -2:     未定位
*         -3:     与gps模块通讯失败 （没有收到任何gps数据）
*	    -4:	与GPS的通讯协议不匹配（解析gps数据失败或波特率不匹配）
*/

struct GPS_contrl
{
	int gps_fd;
	int enable;
	int port;
	int baud;
	int parity;
	int stop;
	int datebit;
	int gps_state; 
	pthread_t thread_id_send;
	pthread_t thread_id_recv;
	pthread_mutex_t mutex_gps ;                        //定义一个线程互斥锁
	struct usr_req_position_struct sendinfo;
};

static int gps_fd=-1;
static struct GPS_contrl gps_para;
static struct dev_position_return_struct pos_info;

void clear_position_info(void)
{
	memset(&pos_info,0,sizeof(struct dev_position_return_struct));
	pos_info.state=1;
}

void clear_gps_para(void)
{
	memset(&gps_para,0,sizeof(struct GPS_contrl));
	pthread_mutex_init(&gps_para.mutex_gps,NULL);
	gps_para.thread_id_recv=-1;
	gps_para.thread_id_send=-1;
	gps_para.gps_fd=-1;
}
/*****************************************************************************************************
* 函数名		 :gps_close_module()
* 功能               :关闭串口和线程
* 返回值 :  
                   0:  关闭串口和创建线程成功
                 -1:  关闭失败
  ***********************************************************************************************************/ 
int gps_close_module(void)
{
	int ret;
	if(gps_fd < 0)
	{
		return 0;
	}
//	clear_position_info();
	ret = close(gps_fd);
	gps_fd = -1;
	return ret;
}

void swap_word(int len, void* dValue)
{
	int i;
	unsigned char temp;
	for(i=0;i<len/2;i++)
	{
		temp=((unsigned char*)dValue)[i];
		((unsigned char*)dValue)[i]= ((unsigned char*)dValue)[len/2+i];
		((unsigned char*)dValue)[len/2+i]=temp;
	}
}

//// gps 模块数据发送1秒一次
/** 
 *   @brief  发送设备的gps位置信息到指定的网络描述符
 *   @param  fd 目的描述符
 *   @param  info 设备的位置信息
 *   @return   负值表示出错,非负表示发送成功
 */ 

int gt_send_position_info(int fd,struct dev_position_return_struct *info,int ack, int env, int enc,int dev_no)
{
//	int rc;
	struct gt_usr_cmd_struct *cmd;
	struct mod_com_type *modcom;
//	struct sockaddr_in peeraddr;
//	int addrlen=sizeof(struct sockaddr);	
	DWORD send_buf[50];			
//	char buf[50];
	struct dev_position_return_struct gpsinfo;
//	i=getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);

	modcom=(struct mod_com_type *)send_buf;
	modcom->env = env;
	modcom->enc = enc;
	cmd=(struct gt_usr_cmd_struct *)modcom->para;
	cmd->cmd=DEV_POSITION_RETURN;
	cmd->en_ack=ack;
	cmd->reserve0=0;
	cmd->reserve1=0;
#ifdef GPS_DEBUG
//// lsk 2010-8-11 test gps 
	printf(" stat=%d , sat_num= %7d \nlon=%15f lat=%15f alt=%12f, direction=%12f, speed=%12f\n",
	info->state, info->reserved[0],info->lon,info->lat,info->altitude,info->direction,info->speed); 
//	printf("lon= 0x%016x   lat = 0x%016x alt = 0x%016x\n", info->lon, info->lat, info->altitude);
//	gtloginfo(" stat=%d , sat_num= %7d \nlon=%15f lat=%15f alt=%12f, direction=%12f, speed=%12f\n",
//	info->state, info->reserved[0],info->lon,info->lat,info->altitude,info->direction,info->speed); 
//	if((info->reserved[0])>=4)
//	info->state=2;
#endif
//// lsk 2010-8-11 test gps  只有 ARM 的double 数据需要swap
	memcpy(&gpsinfo,info,sizeof(struct dev_position_return_struct));
	swap_word(sizeof(double),&gpsinfo.altitude);
	swap_word(sizeof(double),&gpsinfo.lat);
	swap_word(sizeof(double),&gpsinfo.lon);
	swap_word(sizeof(double),&gpsinfo.speed);
	swap_word(sizeof(double),&gpsinfo.direction);
	
//	memcpy(&cmd->para,info,sizeof(struct dev_position_return_struct));
	memcpy(&cmd->para,&gpsinfo,sizeof(struct dev_position_return_struct));
	virdev_get_devid(dev_no,cmd->para);
	cmd->len=SIZEOF_GT_USR_CMD_STRUCT-sizeof(cmd->para)-2+sizeof(struct dev_position_return_struct);
	return send_gate_pkt(fd,modcom,cmd->len+2,dev_no);	
}
/**************************************************************************************************
 * 函数名: gps_get_position_info
 * 功能:  返回gps的状态和定位信息
  * 返回值:
 *           0 :    正常定位
 *	     -1:     通讯端口未初始化
 *         -2:     未定位
 *         -3:     与gps模块通讯失败 （没有收到任何gps数据）
	     -4：与GPS的通讯协议不匹配（解析gps数据失败或波特率不匹配）
 *        其他值待定
 *
 *         pos:返回的已经填充好的数据缓冲区
 * 参数:
 *         pos:  指向要存放接收数据的缓冲区(4字节对其)
 * 说明:   
 * 	   纬度：北纬为正值，南纬为负值
 *	  经度：东经为正值，西经为负值 
 *	   速度：是绝对值
 ******************************************************************************************************/
int gps_get_position_info(struct dev_position_return_struct* pos)
{
	int ret;
#ifdef GPS_DEBUG
	struct dev_position_return_struct* info=&pos_info;
	printf(" stat=%d , sat_num= %7d \nlon=%15f lat=%15f alt=%12f, direction=%12f, speed=%12f\n",
	info->state, info->reserved[0],info->lon,info->lat,info->altitude,info->direction,info->speed); 
#endif
	pthread_mutex_lock(&gps_para.mutex_gps);
	memcpy(pos, &pos_info, sizeof(struct dev_position_return_struct));  // 内存复制
	ret = gps_para.gps_state;
	pthread_mutex_unlock(&gps_para.mutex_gps);
	if(pos->reserved[0]<4)
	{
		pos->state=2;
		ret= -2;
	}
	else
	{
		pos->state=0;
		ret= 0;
	}
    return ret ;
}

int dev_position_info_return(void)
{
	int ret;
	struct dev_position_return_struct gpsinfo;
	
	memset(&gpsinfo,0,sizeof(gpsinfo));
	ret=gps_get_position_info(&gpsinfo);
#if 1
	switch(ret)
	{
		case 0:
//		printf("GPS module OK\n");	
		break;
		case -1:
		printf("GPS module 通讯端口未初始化\n");	
		gtlogerr("GPS module 通讯端口未初始化\n");	
		break;
		case -2:
		printf("GPS module 未定位\n");	
		gtlogerr("GPS module 未定位\n");	
		break;
		case -3:
		printf("与GPS module 通讯失败 \n");	
		gtlogerr("与GPS module 通讯失败 \n");	
		break;
		case -4:
		printf("GPS module 的通讯协议不匹配\n");	
		gtlogerr("GPS module 的通讯协议不匹配\n");	
		break;
		default:
		gtlogerr("gps_get_position_info return err %d \n",ret);
		break;
	};
#endif	
	gt_send_position_info(1,&gpsinfo,0,0,0,0);
	return ret;
}


static void gps_stop_send(void)
{
	pthread_mutex_lock(&gps_para.mutex_gps);
	clear_position_info();
	gps_para.sendinfo.enable=0;
	gps_para.thread_id_send=-1;
	pthread_mutex_unlock(&gps_para.mutex_gps);
}
static void gps_clean_up(void)
{
	gps_stop_send();
	gps_close_module();
}

/**************************************************************************************************
函数名:parse_gps_gpgga
功能      : 解析gps_gpgga一行的数据
返回值:
                   0  : 定位
                   -2: 未定位
 参数:
               CSV_T* csv :指向一行数据的首地址

**************************************************************************************************/
int parse_gps_gpgga(struct  GPS_contrl*gpsctrl, CSV_T* csv)
{
	int number_ver = 0 ;                 //  卫星个数
	int csv_get_position = 0;            //定位信息   ，针对 "$GPGGA"
	const char *sptr=NULL;
	const char * csv_get_NS = NULL ;     //获得南纬还是北纬值
	const char * csv_get_EW  = NULL;     //获得东经还是西经值
	double lat = 0;                         //获得维度值的整数部分
	double lon = 0;                         //获得经度值的整数部分
	
	
	csv_get_position = csv_get_int(csv,GPS_GPGGA_POSITION_FIX_INDICTOR,0);  //非0即为定位
	if(csv_get_position == 0) 
	{
	   pthread_mutex_lock(&gpsctrl->mutex_gps);
	   clear_position_info();
	   pthread_mutex_unlock(&gpsctrl->mutex_gps);
	   printf("GGA not vailid\n");
	   return -2;
	}
	else
	{
		pthread_mutex_lock(&gpsctrl->mutex_gps);
		gpsctrl->gps_state = 0;           //状态0 即定位
	 	clear_position_info();
	 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
	}
	sptr = csv_get_str(csv,GPS_GPGGA_ALTITUDE,DEF_VAL_CHAR);
	if(sptr!=NULL)
	{
		pthread_mutex_lock(&gpsctrl->mutex_gps);
		pos_info.altitude= atof(sptr);  //海拔高度
	 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
	}
	else
	return -1;

	number_ver = csv_get_int(csv,GPS_GPGGA_SATELLITES_USED,DEF_VAL_INT);   //卫星个数
	pthread_mutex_lock(&gpsctrl->mutex_gps);
	pos_info.reserved[0] = number_ver;	////卫星数
 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
	
	csv_get_NS =  csv_get_str(csv,GPS_GPGGA_NORTH_SOUTH,DEF_VAL_CHAR);//直接获得的维度单位是:度度分分，分分分分		   
       sptr = csv_get_str(csv,GPS_GPGGA_LATITUDE ,DEF_VAL_CHAR);
	if(sptr!=NULL)
	{
		pthread_mutex_lock(&gpsctrl->mutex_gps);
		pos_info.lat = (atof(sptr))*0.01;  //要把字符串转换成double
		lat = pos_info.lat;
		pos_info.lat = (int)lat + (lat - (int)lat)*100/60;   //转换后的维度单位为:度度。度度度度
		if(strncmp(csv_get_NS, "S",strlen("S")) == 0)   //纬度：北纬为正值，南纬为负值
		{
		   pos_info.lat = -pos_info.lat; 
		}
	 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
	} 
	else
	return -1;

	csv_get_EW  = csv_get_str(csv,GPS_GPGGA_EAST_WEST,DEF_VAL_CHAR); //直接获得的经度 单位是:度度度。度度度度
	sptr=csv_get_str(csv,GPS_GPGGA_LONGITUDE,DEF_VAL_CHAR);
	if(sptr)
	{
		pthread_mutex_lock(&gpsctrl->mutex_gps);
		pos_info.lon = (atof(sptr))*0.01; //要把字符串转换成double
		lon = pos_info.lon;
		pos_info.lon = (int)lon + (lon-(int)lon)*100/60;
		if(strncmp(csv_get_EW,"W",strlen("W")) == 0)    // 经度：东经为正值，西经为负值 
		{
		   pos_info.lon = -pos_info.lon; 
		}
	 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
	}
	else
	return -1 ;
//	printf("all GGA data parsed\n");
	return 0;
}
/**************************************************************************************************
函数名:parse_gps_gprmc
功能      : 解析gps_gprmc一行的数据
返回值:
                   0  : 定位
                   -2: 未定位
 参数:
               char* line_buf :指向一行数据的首地址

**************************************************************************************************/
int parse_gps_gprmc(struct  GPS_contrl*gpsctrl, CSV_T* csv)
{
	const char *csv_get_position = NULL;      //定位信息，针对 "$GPRMC"
	const char * sptr=NULL;
	
	csv_get_position = csv_get_str(csv,GPS_GPRMC_STATUS,DEF_VAL_CHAR);  //非0即为定位
	if(strncmp(csv_get_position,"V",strlen("V")) == 0) 
	{
	 	pthread_mutex_lock(&gpsctrl->mutex_gps);
		clear_position_info();
	 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
		return -2;
	}

	if(strncmp(csv_get_position,"A",strlen("A")) == 0) 
	{
		sptr=csv_get_str(csv,GPS_GPRMC_POSITION_SPEED_KNOT,DEF_VAL_CHAR);
		if(sptr)
		{
		 	pthread_mutex_lock(&gpsctrl->mutex_gps);
			pos_info.speed	= (atof(sptr))*1.852;   //单位从knot(节)换算成km/h		   
			if(pos_info.speed < 0)   // 速度：是绝对值
				pos_info.speed = -pos_info.speed;
		 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
		}
		else
		return -1;
		sptr=csv_get_str(csv,GPS_GPRMC_DIRECTION,DEF_VAL_CHAR);
		if(sptr)
		{
			pthread_mutex_lock(&gpsctrl->mutex_gps);
			pos_info.direction	= atof(sptr); 
			pthread_mutex_unlock(&gpsctrl->mutex_gps);
		}
		else
		return -1;
	}   
	return 0;
}
/**************************************************************************************************
函数名:parse_gps_gpvtg
功能      : 解析gps_gpvtg一行的数据
返回值:
                   0  : 定位
 参数:

**************************************************************************************************/
int parse_gps_gpvtg(struct  GPS_contrl*gpsctrl, CSV_T* csv)
{
	const char* sptr=NULL;
	
	sptr = csv_get_str(csv, GPS_GPVTG_SPEED_KM_HOUR, DEF_VAL_CHAR);    //单位从knot (节)换算成km/h	
	if(sptr)
	{
	 	pthread_mutex_lock(&gpsctrl->mutex_gps);
		pos_info.speed = atof(sptr);
		if(pos_info.speed < 0)   // 速度：是绝对值
		pos_info.speed = -pos_info.speed;
	 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
	}
	else 
	return -1;
	sptr=csv_get_str(csv,GPS_GPVTG_COURSE,DEF_VAL_CHAR);
	if(sptr)
	{
	 	pthread_mutex_lock(&gpsctrl->mutex_gps);
		pos_info.direction  = atof(sptr); 
		pos_info.state = 0;   //只要是获得GPVTG这行数据，则说明已经定位
	 	pthread_mutex_unlock(&gpsctrl->mutex_gps);
	}
	else 
	return -1;
	return 0;
}

int parse_gps_message(char* info)
{
	int ret=0;
  int alld;
	CSV_T* csv = NULL;
	const char* csv_get_first_str = NULL;               //用于获得一行数据逗号前的第一个字符串
	
	csv = csv_create();
	if(csv==NULL)
	{
		gtlogerr("error parse GPS message can not create csv\n");
		return -1;
	}
	ret=csv_parse_string(csv,info);
	if(ret)
	{
		csv_get_first_str = csv_get_error_str((IN int)ret);
		gtlogerr("parse gps message error %s \n", csv_get_first_str);
		csv_destroy(csv);
		return -1;
	}
	//////// 分别解析 GPGGA GPRMC GPVTG
//	printf("%s",info);
	csv_get_first_str = csv_get_str(csv,1,NULL); 
	  /////// $GPGGA
	if(strncmp(csv_get_first_str,"$GPGGA",strlen("$GPGGA")) == 0)
	ret=parse_gps_gpgga(&gps_para,csv);
	  /////// $GPRMC
	else if(strncmp(csv_get_first_str,"$GPRMC",strlen("$GPRMC")) == 0)
	ret=parse_gps_gprmc(&gps_para,csv);
	
	  /////// $GPVTG
	else if(strncmp(csv_get_first_str,"$GPVTG",strlen("$GPVTG")) == 0)	
	ret=parse_gps_gpvtg(&gps_para,csv);
	csv_destroy(csv);
	return ret;
}

static char gps_buf[2048];
void GPS_recv_thread(struct GPS_contrl* para)
{
	int i=0;
	int err_read_cnt=0;
	int ret=0;
	struct GPS_contrl *gps_info=para;
	if(para==NULL)
	{
		gtlogerr("gps recv proc input para =NULL\n");
		pthread_exit(NULL);
	}
	if(gps_fd<0)
	{
		gtlogerr("gps recv proc gps_fd = %d\n",gps_fd);
              pthread_mutex_lock(&gps_info->mutex_gps);
		gps_info->gps_state=-1;
              pthread_mutex_unlock(&gps_info->mutex_gps);
		pthread_exit(NULL);
	}
		
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);		//设置线程为可取消的
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);		//执行到撤销点才进行撤销
	gtloginfo("get uer command :gps recv thread start \n");
	while(1)
	{
		pthread_testcancel();
		ret = read(gps_fd, &gps_buf[i++], 1);
		if(ret!=1)
		{
			if(err_read_cnt++>5*1024)
			{
//				err_read_cnt=0;
				gtlogerr("gps read error \n");
		              pthread_mutex_lock(&gps_info->mutex_gps);
				gps_info->gps_state=-3;
		              pthread_mutex_unlock(&gps_info->mutex_gps);
				break;
			}
			continue;
		}
		err_read_cnt=0;
		if(gps_buf[i-1]=='$')		///// start a message
		{
			memset(gps_buf,0,sizeof(gps_buf));
			i=1;
			gps_buf[0]='$';
			continue;
		}
		if((gps_buf[i-1]=='\r')||(gps_buf[i-1]=='\n')) 	//// end of a message
//		if((gps_buf[i-1]=='\r')) 	//// end of a message
		{
			gps_buf[i-1]='\n';
			if(gps_buf[0]=='$')	//// message received completely
			{
//				printf("0x%02x 0x%02x 0x%02x \n",gps_buf[0],gps_buf[1],gps_buf[2]);
				pthread_mutex_lock(&gps_info->mutex_gps);
				pos_info.state=2;
				pthread_mutex_unlock(&gps_info->mutex_gps);
				parse_gps_message(gps_buf);
			}
			i=0;
		}
		if(i>=2047)	////2048个字节没有收到'$'
		{
			gtlogerr("gps receive 2048 bytes without a vailid message\n");
	              pthread_mutex_lock(&gps_info->mutex_gps);
			pos_info.state=3;
			gps_info->gps_state=-4;
	              pthread_mutex_unlock(&gps_info->mutex_gps);
			i=0;
		}
	}
	////// can not be here 
	gps_close_module();
	gtlogerr("gps receive thread faild\n");
	pthread_exit(NULL);
}

void GPS_send_thread(struct GPS_contrl* para)
{
	int i=0;
	unsigned long j=0;
//	int ret=0;
	if(para==NULL)
	{
		gtlogerr("gps send proc input err value\n");
		pthread_exit(NULL);
	}
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);		//设置线程为可取消的
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);		//执行到撤销点才进行撤销
	gtloginfo("get uer command :gps send thread start \n");
       gtloginfo("GPS计时器为[%ld]秒\n",para->sendinfo.send_seconds);
	while(1)
	{
		for(i=0;i<para->sendinfo.interval;i++)
		{
			pthread_testcancel();
			sleep(1);
			j++;
		}
		dev_position_info_return();
		if(j>=para->sendinfo.send_seconds)
		{
			j=0;
			break;
		}
	}
	gps_stop_send();
       gtloginfo("当前[%ld]已超过GPS计时器最大值[%ld]，GPS停止发送位置信息\n",j,para->sendinfo.send_seconds);
//	gps_clean_up();
	pthread_exit(NULL);
}

static int create_gps_send_thread(void)
{
	return GT_CreateThread(&gps_para.thread_id_send,(void*)&GPS_send_thread, (void*)&gps_para);
}

static int create_gps_recv_thread(void)
{
	return GT_CreateThread(&gps_para.thread_id_recv,(void*)&GPS_recv_thread, (void*)&gps_para);
}
/**********************************************************************************************
 * 函数名 :gps_init_module()
 * 功能 :            设置gps串口的工作模式，发送初始化指令给gps模块
 * 参数 :
 *		  port             :设置GPS通讯的串口号（0，1，2......）
 *  		  baud            :要设置的波特率 4800,9600,19200...                     
 *		  databits        :数据位        7,8
 *             stopbits        :停止位        1,2
 *              parity          :奇偶校验位'n','o','e','s'
 * 返回值       :  0 表示成功,负值表示失败
 * 说明：如果初始化成功，下次再调用该函数的时候，如果输入参数相同，则直接返回0，
 *	 不同则需要重新设置串口，发送初始化命令。
 **********************************************************************************************/
int gps_init_module(int port,int baud,int databits,int stopbits,int parity)
{    
	char port_number[100];  //存放设备端口号
	int ret ;

	memset(port_number,0,sizeof(port_number));	
	sprintf(port_number, "/dev/ttyS%d", port);
	
	if(gps_fd >= 0)   // 在初始化时对已打开的串口先关闭再打开 
	{
        	ret = gps_close_module();
		if(ret < 0)
		{
			return -1;   
		}
	}	

	gps_fd = open(port_number, O_RDWR|O_NOCTTY);
	
	if(gps_fd  < 0)
	{
		gtlogerr("can not open com port %s\n",port_number);
		return -1;
	}

	ret = set_com_mode(gps_fd, databits, stopbits,parity);
	
	if(ret < 0)
	{
  		gtlogerr("errlor set gps com mode datebits=%d stopbits=%d parity=%c \n", 
			databits,stopbits,parity);
		close(gps_fd);
		gps_fd = -1;
		return -1;
	}

	ret = set_com_speed(gps_fd, baud);  
	
	if(ret < 0)
	{
  		gtlogerr("errlor set gps com speed baud=%d \n",baud);
	      	close(gps_fd);
		gps_fd = -1;
		return -1;
	}
	ret= create_gps_recv_thread();
       return ret;
}

int init_gps_para(char* filename)
{
	dictionary      *ini;	
	char *pstr=NULL;
	int ret=0;
	ini=iniparser_load(filename);
        if (ini==NULL) {
                printf("cannot parse ini file file [%s]", filename);
                return -1 ;
        }
	clear_gps_para();
	clear_position_info();
	gps_para.enable= iniparser_getint(ini,"GPS:enable",0);
	if(gps_para.enable==0)
	{
		iniparser_freedict(ini);
		printf("没有安装GPS模块\n");
		gtloginfo("没有安装GPS模块\n");
		return -2;
	}
	gps_para.port= iniparser_getint(ini,"GPS:port",0);
	gps_para.baud= iniparser_getint(ini,"GPS:baud",9600);
	gps_para.datebit= iniparser_getint(ini,"GPS:date",8);
	gps_para.stop= iniparser_getint(ini,"GPS:stop",1);
	pstr = iniparser_getstring(ini,"GPS:parity",NULL);
	if(pstr!=NULL)
		gps_para.parity=pstr[0];
	else
		gps_para.parity='N';
	iniparser_freedict(ini);
	printf("安装了GPS模块port %d baud %d %c %d %d \n",gps_para.port, gps_para.baud,
		gps_para.parity,gps_para.datebit,gps_para.stop);
	gtloginfo("安装了GPS模块port %d baud %d %c %d %d \n",gps_para.port, gps_para.baud,
		gps_para.parity,gps_para.datebit,gps_para.stop);
	ret=gps_init_module(gps_para.port, gps_para.baud, gps_para.datebit, gps_para.stop,gps_para.parity);
	if(ret<0)
	{
		gtlogerr("gps init err %d\n",ret);
//		ret= send_gate_ack(fd,cmd->cmd,ERR_DVC_INTERNAL,env,enc,dev_no);
//		return ret;
	}
	return ret;
}

int process_usr_query_position(int fd,struct gt_usr_cmd_struct *cmd, int env, int enc,int dev_no)
{
	int ret;
	struct usr_req_position_struct *req_cmd;
//	BYTE dev_guid[20];
	struct sockaddr_in peeraddr;
	int addrlen=sizeof(struct sockaddr);

	getpeername(fd,(struct sockaddr *)&peeraddr,&addrlen);

	if(cmd->cmd!=USR_QUERY_DEVICE_POSITION)
	return -EINVAL;

	req_cmd=(struct usr_req_position_struct *)cmd->para;

	printf("%s(fd=%d)发来获取设备位置信息命令:enable=%d,target=%d,interval=%d,send_seconds=%ld\n",inet_ntoa(peeraddr.sin_addr),fd,req_cmd->enable,req_cmd->target,req_cmd->interval,req_cmd->send_seconds);
	if(gps_para.enable==0)		////没有安装gps模块
	{
		ret= send_gate_ack(fd,cmd->cmd,ERR_EVC_NOT_SUPPORT,env,enc,dev_no);
		return ret;
	}
	if(gps_fd<0)
	{
		gtlogerr("did not initialize gps module\n");
		send_gate_ack(fd,cmd->cmd,ERR_DVC_INTERNAL,env,enc,dev_no);
		init_gps_para(IPMAIN_PARA_FILE);
		return -1;
	}
	
	if(req_cmd->enable==0)		//// 停止发送gps数据
	{
		if(gps_para.sendinfo.enable==1)
		{
			if(gps_para.thread_id_send>=0)	
			{
				ret=pthread_cancel(gps_para.thread_id_send);
				if(ret>=0)
				{
					gtloginfo("get uer command :gps thread canceled \n");
				}
				else
				{
					gtlogerr("can not cancel gps thread\n");
				}
			}
			sleep(2);
			gps_stop_send();
//			gps_clean_up();
		}
	}
	else							////开始发送gps数据
	{
		if(gps_para.sendinfo.enable==0)
		{
			memcpy(&gps_para.sendinfo,req_cmd,sizeof(struct usr_req_position_struct));
#if 0
			if(gps_para.thread_id_recv==-1)
			{
				ret=create_gps_recv_thread();
				if(ret>=0)
				{
					gtloginfo("get uer command :gps recv thread start \n");
					gps_para.sendinfo.enable=1;
				}
				else
				{
					gtlogerr("can not create  gps recv thread\n");
					gps_clean_up();
				}
			}
#endif			
			ret=create_gps_send_thread();
			if(ret>=0)
			{
				gtloginfo("get uer command :gps send thread start \n");
				gps_para.sendinfo.enable=1;
			}
			else
			{
				gtlogerr("can not create  gps send thread\n");
				gps_clean_up();
			}
		}
		
	}
	
	if(ret>=0)
	{
		ret= send_gate_ack(fd,cmd->cmd,RESULT_SUCCESS,env,enc,dev_no);
	}
	else
	{
		ret= send_gate_ack(fd,cmd->cmd,ERR_DVC_INTERNAL,env,enc,dev_no);
	}
	return ret;
}

//#endif



