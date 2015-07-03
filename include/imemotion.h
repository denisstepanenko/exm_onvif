/* imemotion.h*/ 
#ifndef IME_MOTION_H
#define IME_MOTION_H
#define DEV_IME6410			//lsk 2007 -1-5 

/*
*************************************************************
 * 函数名	:set_ime_motion_sens()
 * 功能	: 设置 ime6410 移动侦测灵敏度 
 * 输入	: 
 		struct compress_struct *enc  编码器参数
 		int ch 视频通道号
 		int sens 灵敏度  0 禁止移动侦测  5移动侦测灵敏度最高 lsk 2007 －6 －27
 * 返回值	:0 表示成功，负值表示失败
*************************************************************
 */
int set_ime_motion_sens(struct compress_struct *enc, int ch, int sens);
/*
*************************************************************
 * 函数名	:get_ime_motion_stat()
 * 功能	: 设置 ime6410 移动侦测灵敏度 
 * 输入	: 
 		struct compress_struct *enc  编码器参数
 		int ch 视频通道号
 * 返回值	:0 表示没有移动, 1表示有移动，负值表示失败
*************************************************************
 */
int get_ime_motion_stat(struct compress_struct *enc, int ch);
#ifndef DEV_IME6410
/*
*************************************************************
 * 函数名	:set_ime_motion_para()
 * 功能	: 设置 ime6410 移动侦测参数
 * 输入	: 
 		struct compress_struct *enc  编码器参数
 		unsigned char ch 视频通道号
 		int sen	灵敏度参数
 		unsigned short *area  区域参数缓冲区	!!目前没有实现区域侦测的功能
 		 int pic_size  图像的大小 (D1,  HD1, CIF)
 * 返回值	:0 表示成功，负值表示失败
*************************************************************
 */
int set_ime_motion_para(struct compress_struct *enc, unsigned char ch, int sen,unsigned short *area, int pic_size);

/*
*************************************************************
 * 函数名	:process_motion_pkt()
 * 功能	: 处理ime6410 移动侦测数据包 
 * 输入	: 
  		int ch 视频通道号
		unsigned char *buf  参数缓冲区	!!目前没有实现区域侦测的功能
 		 int pic_size  图像的大小 (D1,  HD1, CIF)
 * 返回值	:0 表示没有移动侦测，1表示有移动侦测， 负值表示失败
*************************************************************
 */
int process_motion_pkt(unsigned char ch, unsigned char *buf, int pic_size);
//int init_ime_motion_para(struct compress_struct *enc, unsigned char ch, int sen, int pic_size);
#endif
#endif
