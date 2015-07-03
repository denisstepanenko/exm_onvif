#ifndef GPSUPPORT_H
#define GPSUPPORT_H
#include <gt_com_api.h>	
/* 

GPS中NMEA 协议的一些重要参数的定义

*/
//#define GPS_DEBUG


#define  GPS_GPGGA 	                       1
#define  GPS_GPGGA_UTC_TIME 	           2
#define  GPS_GPGGA_LATITUDE 	           3
#define  GPS_GPGGA_NORTH_SOUTH 	           4
#define  GPS_GPGGA_LONGITUDE  	           5
#define  GPS_GPGGA_EAST_WEST 	           6
#define  GPS_GPGGA_POSITION_FIX_INDICTOR   7
#define  GPS_GPGGA_SATELLITES_USED         8
#define  GPS_GPGGA_ALTITUDE                10

#define  GPS_GPRMC 	                       1
#define  GPS_GPRMC_UTC_TIME 	           2
#define  GPS_GPRMC_STATUS                  3     
#define  GPS_GPRMC_LATITUDE  	           4
#define  GPS_GPRMC_NORTH_SOUTH 	           5
#define  GPS_GPRMC_LONGITUDE  	           6
#define  GPS_GPRMC_EAST_WEST 	           7 
#define  GPS_GPRMC_POSITION_SPEED_KNOT 	   8
#define  GPS_GPRMC_DIRECTION 	           9

#define  GPS_GPVTG 	                       1
#define  GPS_GPVTG_COURSE 	               1
#define  GPS_GPVTG_SPEED_KNOT 	           6
#define  GPS_GPVTG_SPEED_KM_HOUR 	       8





#define  DEF_VAL_INT      	               0
#define  DEF_VAL_CHAR                    NULL

//#define  DEF_VAL_INT      	               101
//#define  DEF_VAL_CHAR                      "CSV_RET_STR_ERROR"

int init_gps_para(char* filename);
int process_usr_query_position(int fd,struct gt_usr_cmd_struct *cmd, int env, int enc,int dev_no);


#endif

