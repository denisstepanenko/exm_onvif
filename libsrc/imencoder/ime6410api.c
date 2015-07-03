
#include <file_def.h>	//°üº¬ÁËµçÂ·°åÐÍºÅµÄ¶¨Òå
#include <iniparser.h>
#include "mdebug.h"
//#include "rtimg_para.h"
#include "iic.h"
#include "ime6410.h"
#include "ime6410api.h"
#include "iicdevtable.h"
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <commonlib.h>
#include <devinfo.h>
#include <tw9903api.h>
#include <imemotion.h>	//lsk 2006-12-1 for motion detection 
//static char frame_buf[MAX_FRAME_SIZE];
//#define SAVE_RAW_DATA

static int   AudBlockSize[NR_CHANNEL];	//ÉùÒô¿é´óÐ¡
static int   AudSize[NR_CHANNEL];		//ÒÑ½ÓÊÕµÄ´óÐ¡
static int  APacketStart[NR_CHANNEL]={1};//changed by shixin,1,1,1};
static char audio_temp_buf[(NR_AUDIO_PKT+1)*(DBLOCKSIZE-4)];//ÎªÁË°²È«¶à·ÖÅäÒ»¸öåååååååååBLOCKSIZEµÄµ¥Ôª
static int  warray[16] = {320, 640, 720, 176, 320, 352, 704, 720,0,640,720,0,0,0,704,720};
static int  harray[16] = {240, 480, 480, 144, 288, 288, 576, 576,0, 240,240,0,0,0,288,288};
int set_imevideo_sharp1(struct compress_struct *encoder,int val);
int set_imevideo_sharp2(struct compress_struct *encoder,int val);

int get_pic_width(struct compress_struct *enc)
{
	return warray[enc->i64reg.com.VidPicSize];
}
int get_pic_height(struct compress_struct *enc)
{
	return harray[enc->i64reg.com.VidPicSize];
}
//½«±àÂëÆ÷²ÎÊýÉèÖÃÎªÄ¬ÈÏÖµ
//³õÊ¼»¯²ÎÊý£¬½«ÏµÍ³²ÎÊýÉèÖÃ³É³õÊ¼Öµ
#define RESET_COMMAND			0x02000289	//0x19800a88//0x19800808//0xf9800808
										
#define RESET_AUDIO			0x00000040	//0xf0000080
#define RESET_CBRCTRL			0x34343434	//0x0a0a0a0a
#define RESET_MOTIONDETECT		0x28282828
#define RESET_CBRVAL			0x00002800	//0x0c000400

/**********************************************************************************************
 * º¯ÊýÃû	:init_enc_default_val()
 * ¹¦ÄÜ	:½«±àÂëÆ÷²ÎÊýÉèÖÃÎªÄ¬ÈÏÖµ,Ó¦¸ÃÔÚ¶ÁÈ¡ÅäÖÃÎÄ¼þÖ®Ç°µ÷ÓÃÒ»´Î
 * ÊäÈë	:ÎÞ
 * Êä³ö:encoder Ö¸ÏòÐèÒª±»Ìî³äµÄ±àÂëÆ÷²ÎÊý½á¹¹µÄÖ¸Õë
 *					º¯Êý·µ»ØÊ±±»Ìî³äºÃ 
 * ·µ»ØÖµ	:ÎÞ
 **********************************************************************************************/
void init_enc_default_val(struct compress_struct *encoder)
{
	struct I64Reg_int  *pinfo;
	pinfo=(struct I64Reg_int  *)&encoder->i64reg;
	pthread_mutex_init(&encoder->mutex, NULL);//Ê¹ÓÃÈ±Ê¡ÉèÖÃ
	pthread_mutex_lock(&encoder->mutex);
	pinfo->com = RESET_COMMAND;
	pinfo->audio = RESET_AUDIO;
	pinfo->cbr = RESET_CBRCTRL;
	pinfo->md  = RESET_MOTIONDETECT;
	pinfo->cbrval[0] = pinfo->cbrval[1] = 
	pinfo->cbrval[2] = pinfo->cbrval[3] = RESET_CBRVAL;

	encoder->fd=-1;
	encoder->NTSC_PAL=PAL;
	encoder->frame_rate=25;
	encoder->AudioFrameType = FRAMETYPE_ADPCM;
	pthread_mutex_unlock(&encoder->mutex);
	encoder->full_ch=0;

	//add by wsy
	encoder->saturation=128;
	encoder->brightness=128;
	encoder->hue=128;
	encoder->contrast=155;
	
	encoder->init_tw9903flag=0;
}

int get_frame_rate(int ispal,int rate)
{
	if(rate>5)
		return 0;
	return rate;
/*
	int frame;
	if(ispal==PAL)
		frame=25;
	else
		frame=30;
	if(rate>frame)
		rate=frame;
	return ((frame/rate)-1);
*/	
}




/**********************************************************************************************
 * º¯ÊýÃû	:OpenEnc()
 * ¹¦ÄÜ	:´ò¿ª±àÂëÆ÷ µÈÍ¬ÓÚopen_ime6410£¬Ö»ÊÇ½ÚµãÃûÒÑ¾­Ìî³äÔÚ½á¹¹ÖÐÁË
 * ÊäÈë	:encoderÃèÊö±àÂëÆ÷µÄÊý¾Ý½á¹¹
 * ·µ»ØÖµ	:0 ±íÊ¾³É¹¦£¬¸ºÖµ±íÊ¾Ê§°Ü
 **********************************************************************************************/
int OpenEnc(struct compress_struct *encoder)
{
	int ime_fd;
	char *devname;
	if(encoder==NULL)
		return -1;
	if(encoder->fd>0)
		return 0;
	devname=encoder->dev_node;
#if EMBEDED
	ime_fd = open(devname,O_RDWR);
#else
	ime_fd = open(devname,O_RDONLY);//²âÊÔ³ÌÐòÖ±½Ó´Ó6410»ñÈ¡µÄÔ­Ê¼Êý¾Ý
#endif
	if(ime_fd> 0){
		printf("Open device %s=%d\n",devname,ime_fd);//DBGL("Open /dev/IME6410!!\n");
	} 
	else 
	{
		printf("Can't Open %s!!\n",devname);
		return -1;
	}
	encoder->fd=ime_fd;
	return 0;
}
/*
  * ´ò¿ª6410Éè±¸£¬·µ»Ø0±íÊ¾ÕýÈ·£¬ÎÄ¼þÃèÊö·ûÌîÈë½á¹¹
  * devname==NULL±íÊ¾´ò¿ªencoder½á¹¹ÖÐµÄÉè±¸Ãû
  */
int open_ime6410(char *devname,struct compress_struct *encoder)
{
	int ime_fd;

	if(encoder==NULL)
		return -1;
	if(encoder->fd>0)
		return 0;
	if(devname==NULL)
		devname=encoder->dev_node;
#if EMBEDED
	ime_fd = open(devname,O_RDWR);
#else
	ime_fd = open(devname,O_RDONLY);//²âÊÔ³ÌÐòÖ±½Ó´Ó6410»ñÈ¡µÄÔ­Ê¼Êý¾Ý
#endif
	if(ime_fd> 0){
		printf("Open device %s=%d\n",devname,ime_fd);//DBGL("Open /dev/IME6410!!\n");
	} 
	else 
	{
		printf("Can't Open %s!!\n",devname);
		return -1;
	}
	encoder->fd=ime_fd;
	return 0;
}
//¹Ø±ÕÉè±¸
int close_ime6410(struct compress_struct *encoder)
{
	int imefd;
	if(encoder==NULL)
		return -1;
	imefd=encoder->fd;
	if(imefd>0)
		close(imefd);
	encoder->fd=-1;
	return 0;
}
/*
 * °´ÕÕÖ¸¶¨µÄ½á¹¹³õÊ¼»¯6410
 */
int init_ime6410(struct compress_struct *encoder)
{//FIXME
#if	EMBEDED
	int  ChkData;//, i;
	int fd;
	struct I64Reg_int  *pinfo;
	struct compress_struct   *enc;
	int  TimeInc_ntsc[6] = { 0x3e9, 0x7d2, 0xfa4, 0x1f48, 0x3e90, 0x7d20 };
	int  TimeInc_pal[6] =   { 0x4b0, 0x960, 0x12c0, 0x2580, 0x4b00, 0x9600 };

	// Open IME6410 Device Driver	
	if(encoder==NULL)
		return -1;
	fd=encoder->fd;
	if (fd<0)
		return -1;
	// Init

	enc=encoder;
	enc->fd=fd;
	pinfo = (struct I64Reg_int *) &(enc->i64reg);

	// Set TimeIncreasement		
	if(enc->i64reg.com.VidPicSize < 3)
		ChkData = TimeInc_ntsc[enc->i64reg.com.FrameRate];
	else
		ChkData = TimeInc_pal[enc->i64reg.com.FrameRate];
	ioctl(fd, 0x2008 | CMD_WRITEM, ChkData);
	MSG("REG-IncTime = %08x\n", ChkData);

	// Set AUDIO Param
	ioctl(fd, REG_AUDIO | CMD_WRITEM, pinfo->audio);
	MSG("REG-AUDIO = %08x\n", pinfo->audio);
	
	// Set CBR CTRL
	ioctl(fd, REG_CBRCTRL | CMD_WRITEM, pinfo->cbr);
	MSG("REG-CBR = %08x\n", pinfo->cbr);

	// Set Motion Detection
	//	enc->i64reg.md.Ch1MdType = num;

	ioctl(fd, REG_MOTIONDETECT | CMD_WRITEM, pinfo->md);
	MSG("REG-MD = %08x\n", pinfo->md);
	
	// Set CBR Value
	ioctl(fd, REG_CH1CBR | CMD_WRITEM, pinfo->cbrval[0]);
	MSG("REG-CH1CBR = %08x\n", pinfo->cbrval[0]);
	ioctl(fd, REG_CH2CBR | CMD_WRITEM, pinfo->cbrval[1]);
	MSG("REG-CH2CBR = %08x\n", pinfo->cbrval[1]);
	ioctl(fd, REG_CH3CBR | CMD_WRITEM, pinfo->cbrval[2]);
	MSG("REG-CH3CBR = %08x\n", pinfo->cbrval[2]);
	ioctl(fd, REG_CH4CBR | CMD_WRITEM, pinfo->cbrval[3]);
	MSG("REG-CH4CBR = %08x\n", pinfo->cbrval[3]);
//// lsk 2006 -12-21 test motion detection
	enc->md_var[0] = 0;
	enc->md_var[1] = 0;
	enc->md_var[2] = 0;
	enc->md_var[3] = 0;
#if 0
	pinfo->md_thred[0][0]= 0x00051388;	// Tr1 = 5 Tr2 = 5000
	pinfo->md_thred[0][1]= 0x00ff1388;	// Tr3 = 255 Tr4 = 5000

	pinfo->md_thred[1][0]= 0x00051388;	// Tr1 = 5 Tr2 = 5000
	pinfo->md_thred[1][1]= 0x00ff1388;	// Tr3 = 255 Tr4 = 5000

	pinfo->md_thred[2][0]= 0x00051388;	// Tr1 = 5 Tr2 = 5000
	pinfo->md_thred[2][1]= 0x00ff1388;	// Tr3 = 255 Tr4 = 5000

	pinfo->md_thred[3][0]= 0x00051388;	// Tr1 = 5 Tr2 = 5000
	pinfo->md_thred[3][1]= 0x00ff1388;	// Tr3 = 255 Tr4 = 5000
	//// initialize motion detection threshold for each channal
	ioctl(fd, REG_MDTHRED0 | CMD_WRITEM, pinfo->md_thred[0][0]);
	MSG("REG-md_thred1[0] = %08x\n", pinfo->md_thred[0][0]);
	ioctl(fd, (REG_MDTHRED0+4) | CMD_WRITEM, pinfo->md_thred[0][1]);
	MSG("REG-md_thred1[1]  = %08x\n", pinfo->md_thred[0][1]);
	ioctl(fd, REG_MDTHRED1 | CMD_WRITEM, pinfo->md_thred[1][0]);
	MSG("REG-md_thred2[0] = %08x\n", pinfo->md_thred[1][0]);
	ioctl(fd, (REG_MDTHRED1+4) | CMD_WRITEM, pinfo->md_thred[1][1]);
	MSG("REG-md_thred2[1]  = %08x\n", pinfo->md_thred[1][1]);
	ioctl(fd, REG_MDTHRED2 | CMD_WRITEM, pinfo->md_thred[2][0]);
	MSG("REG-md_thred3[0] = %08x\n", pinfo->md_thred[2][0]);
	ioctl(fd, (REG_MDTHRED2+4) | CMD_WRITEM, pinfo->md_thred[2][1]);
	MSG("REG-md_thred3[1]  = %08x\n", pinfo->md_thred[2][1]);
	ioctl(fd, REG_MDTHRED3 | CMD_WRITEM, pinfo->md_thred[3][0]);
	MSG("REG-md_thred4[0] = %08x\n", pinfo->md_thred[3][0]);
	ioctl(fd, (REG_MDTHRED3+4) | CMD_WRITEM, pinfo->md_thred[3][1]);
	MSG("REG-md_thred4[1]  = %08x\n", pinfo->md_thred[3][1]);
#endif
#endif

	return 0;
}
#if 0
UserIICTab tw9903_reset[] =
{
        {0x02880410, 0x00000000},//     ; 0x02~0x05  //hg_old value
        {0x06880410, 0x80000000},//     ; 0x06~0x09  //0x08=0x18

        // slicer
        {0x0a880410, 0x00000000},//     ; 0x0a~0x0d  //0x85d08c00
        {0x0e880410, 0x00000000},//     ; 0x0e~0x11  //0x11002060
        {0x12880410, 0x00000000},//     ; 0x12~0x15 //0x017f5a00
        {0x16880110, 0x00000000},//     ; 0x16           0x17 reserved//
        {0x18880210, 0x00000000},// ; 0x18~0x19

        {0x1a880410, 0x00000000},//     ; 0x1a~0x1d
        {0x1e880410, 0x00000000},//     ; 0x1e~0x21//
        {0x22880410, 0x00000000},//     ; 0x22~0x25
        {0x26880410, 0x00000000},//     ; 0x26~0x29   0x29=0x00

        // scaler
        {0x2a880410, 0x00000000},//     ; 0x2a~0x2d   0x2c=0x37
        {0x2e880410, 0x00000000},//     ; 0x2e~0x31   0x31=0x00
        {0x32880410, 0x00000000},//     ; 0x32~0x35   0x35=0x01
        {0x36880110, 0x00000000},//     ; 0x36

        // saa7114h_global
        //{0x87400210, 0x01780000},//   ; 0x87~0x88
        {0x00000000, 0x00000000}


};

int reset_tw9903(struct compress_struct *enc)
{//Èí¸´Î»9903
	#define TW9903_RST_REG	0x06
	int tw9903_devid;
	int dwdata;
	int devfd;
	if(enc==NULL)
		return -1;
	devfd=enc->fd;
	if(devfd<0)
		return -3;
	tw9903_devid=0x88;//9903µÄÉè±¸µØÖ·
	dwdata=0x80;//FIXME ÐèÒª±£ÁôÔ­À´µÄÖµÂðå
	dwdata<<=24;	//bigendian
	pthread_mutex_lock(&enc->mutex);
	OpenUserIIC(devfd);
	WriteUserIIC(devfd, tw9903_devid, TW9903_RST_REG,1, dwdata,0);
	// WriteUserIICTab(enc->fd, tw9903_reset, tw9903_devid);	
	CloseUserIIC(devfd);
	pthread_mutex_unlock(&enc->mutex);
	
	usleep(500000);
	
	return 0;		
	
}
#endif

/**********************************************************************************************
 * º¯ÊýÃû	:I64_Start()
 * ¹¦ÄÜ	:Æô¶¯6410£¬¿ªÊ¼Ñ¹ËõÊý¾Ý
 * ÊäÈë	:encÃèÊö±àÂëÆ÷µÄÊý¾Ý½á¹¹
 * ·µ»ØÖµ	:0 ±íÊ¾³É¹¦£¬¸ºÖµ±íÊ¾Ê§°Ü
 **********************************************************************************************/
#define	USER4	0x18
static int 	iic_fd=-1; //2410µÄiic fd//changed 0->-1 by shixin 2006.11.13
int I64_Start(struct compress_struct *enc)
{
	struct I64Reg_int  *pinfo;
	UserIICTab	*pvideo;
	#if BOARD==INTIME_RDK5
		UserIICTab *s7114_video;
	#endif
	int  ch[4], i;//remed by shixin  framediv;
	int fd;
	int rc;
	int  ChkData;
	int  ChkAddr[10] = {
			0x2000, 0x2004, 0x2008, 0x200c,	0x2010, 0x2014,
			0x2028,	0x202c, 0x2030, 0x2034 };
	if(enc==NULL)
		return -1;

	fd=enc->fd;
	if(fd<0)
		return -1;
	pinfo=(struct I64Reg_int  *)(&enc->i64reg);

	// Start Video Decoder
#if BOARD==INTIME_RDK5
	//printf("intime rdk5 board\n");
	if(enc->i64reg.com.VidPicSize == 0)
		s7114_video = sa7114h_320x240;
	else if(enc->i64reg.com.VidPicSize == 1)
		s7114_video = sa7114h_640x480;
	else if(enc->i64reg.com.VidPicSize == 2)
		s7114_video = sa7114h_720x480;
	else if(enc->i64reg.com.VidPicSize == 4)
		s7114_video = sa7114h_320x288;
	else if(enc->i64reg.com.VidPicSize == 5)
		s7114_video = sa7114h_352x288;
	else if(enc->i64reg.com.VidPicSize == 6)
		s7114_video = sa7114h_704x576;
	else if(enc->i64reg.com.VidPicSize == 7)
		s7114_video = sa7114h_720x576;
	else {
		printf("Unsupported Picture Size\n");
		printf("Check Your Configuration and Restart Program..!\n");
		return -1;
	}
#else	//gt board
#if	EMBEDED
	if((get_quad_flag()==0)&&(get_cpu_iic_flag()==1)) 
	{
		if(iic_fd<0)
		{	
			iic_fd=init_iic_dev();
			if(iic_fd<0)
			{
				printf("gtboard iic init failed\n");
			}
			else
			{
				printf("gtboard iic init rc=%d\n",iic_fd);
			}
		}
	}
	//add by wsy for 2410->9903

	OpenUserIIC(fd);
	//gtloginfo("test,Start I64, PicSize = %d\n",enc->i64reg.com.VidPicSize);
//	printf("test,Start I64, PicSize = %d\n",enc->i64reg.com.VidPicSize);
//	printf("open  iic dev %d\n",iic_fd);//lsk test	2006-11-30
	switch(enc->i64reg.com.VidPicSize)
	{
		
		case 3:
			pvideo = tw9903_176x144;//qcif
			ioctl(fd,0xfffffffe ,3);
			if(iic_fd>0)
			{
				rc=init_tw9903(iic_fd,TW9903_ADDR,SCR_MODE_QCIF);//wsy for 2410->9903
				printf("init_tw9903 by iic result %d case 3\n",rc);
			}
		break;
		case 5:
			pvideo = tw9903_352x288;
			ioctl(fd,0xfffffffe ,5);
			if(iic_fd>0)
			{
				rc=init_tw9903(iic_fd,TW9903_ADDR,SCR_MODE_CIF);//wsy for 2410->9903
				printf("init_tw9903 by iic result %d case 5\n",rc);
			}		
		break;
		case 6://4CIF same as D1
		case 7:
			pvideo = tw9903_720x576;
			ioctl(fd,0xfffffffe ,7);
			if(iic_fd>0)
			{
				rc=init_tw9903(iic_fd,TW9903_ADDR,SCR_MODE_D1);//wsy for 2410->9903
				printf("init_tw9903 by iic result %d case 7\n",rc);
			}		
		break;
		case 14://FIXME!!!!!
			pvideo = tw9903_720x576;
			ioctl(fd,0xfffffffe ,15);	
			if(iic_fd>0)
			{
				rc=init_tw9903(iic_fd,TW9903_ADDR,SCR_MODE_D1);//wsy for 2410->9903
				printf("init_tw9903 by iic result %d case 14\n",rc);
			}		
		break;
		case 15://FIXME!!!!!
			pvideo = tw9903_720x576;
			ioctl(fd,0xfffffffe ,15);	
			if(iic_fd>0)
			{
				rc=init_tw9903(iic_fd,TW9903_ADDR,SCR_MODE_D1);//wsy for 2410->9903
				printf("init_tw9903 by iic result %d case 15\n",rc);
			}		
		break;
		default:
			
			printf("Unsupported Picture Size %d\n",enc->i64reg.com.VidPicSize);
			printf("Check Your Configuration and Restart Program..!\n");
			return -1;
		break;
	}
#endif

	//MSG("PicSize = %d x %d\n", sys_info_get(SYS_VidPicWidth,0),sys_info_get(SYS_VidPicHeight,0));

	ch[0] = (enc->i64reg.com.ChannelActive) & 0x1;
	ch[1] = (enc->i64reg.com.ChannelActive >> 1) & 0x1;
	ch[2] = (enc->i64reg.com.ChannelActive >> 2) & 0x1;
	ch[3] = (enc->i64reg.com.ChannelActive >> 3) & 0x1;


	
	printf("Ch1 = %s, Ch2 = %s, Ch3 = %s, Ch4 = %s\n",
		ch[0] ? "on" : "off",
		ch[1] ? "on" : "off",
		ch[2] ? "on" : "off",
		ch[3] ? "on" : "off");
#if 0// 1//remed by shixin
	if(get_pic_width(enc) > 352)
	{
		framediv = ch[0] + ch[1] + ch[2] + ch[3] - 1;
		if(get_pic_height(enc) <= 288)
			framediv = 0;
	}
	else 
		framediv = 0;
	if(framediv<0) 
		framediv = 0;
#endif
	pthread_mutex_lock(&enc->mutex);

//	FPGAChannelDisable(fd);
#if BOARD==INTIME_RDK5
	if(ch[0]) WriteUserIICTab(fd, s7114_video, 0x40);
	if(ch[1]) WriteUserIICTab(fd, s7114_video, 0x42);
	if(ch[2]) WriteUserIICTab(fd, s7114_video, 0x44);
	if(ch[3]) WriteUserIICTab(fd, s7114_video, 0x46);
#else	//gtboard
	if(1)//!initiicflag)
	if(!enc->init_tw9903flag)
	{		
		//fixme change the table here

		if(ch[0]) 
			WriteUserIICTab(fd, pvideo, 0x88);
		enc->init_tw9903flag=1;
	}
//	if(ch[1]) WriteUserIICTab(fd, pvideo, 0x82);
//	if(ch[2]) WriteUserIICTab(fd, pvideo, 0x84);
//	if(ch[3]) WriteUserIICTab(fd, pvideo, 0x86);


#endif

//	for(i=0; i<4; i++) {
//		if(ch[i]) {
	//FPGAChannelSet(fd, i, sys_info_get(SYS_VidPicWidth,0),
	//			sys_info_get(SYS_VidPicHeight,0), framediv);
//		}
//	}
	
//	FPGAChannelEnable(fd, ch[0], ch[1], ch[2], ch[3]);
	CloseUserIIC(fd);

	pthread_mutex_unlock(&enc->mutex);
#if 1 //BOARD==GT1000_1_0
//test!!!
//remed by shixin ¿ÉÄÜ»áµ¼ÖÂ6410¹¤×÷²»ÎÈ¶¨	switch_imevideo_channel(enc,enc->full_ch);
//printf("now b %d, h %d, c %d, s %d\n",enc->brightness,enc->hue,enc->contrast,enc->saturation);

	if(!get_quad_flag())
	{
		gtloginfo("Æô¶¯±àÂëÆ÷,ÁÁ¶È%d,É«µ÷%d,¶Ô±È%d,±¥ºÍ%d\n",enc->brightness,enc->hue,enc->contrast,enc->saturation);
	#if 1
		set_imevideo_brightness(enc,enc->brightness);
		set_imevideo_hue(enc,enc->hue);
		set_imevideo_contrast(enc,enc->contrast);
		set_imevideo_saturation(enc,enc->saturation);
		set_imevideo_sharp1(enc,enc->sharp1);
		set_imevideo_sharp2(enc,enc->sharp2);
	#endif
	}
#endif
	// Start IME6410 : Set COMMAND Param
	pinfo = (struct I64Reg_int *) &(enc->i64reg);
	pinfo->com &= 0xfffffffc;
	pinfo->com |= 0x1;


//// test changed by lsk 2006 -12-7
	ioctl(fd, REG_COMMAND | CMD_WRITEM, pinfo->com&0x07ffffff);//added by shixin
//	ioctl(fd, REG_COMMAND | CMD_WRITEM, pinfo->com);//changed  by lsk

	//ioctl(fd, REG_COMMAND | CMD_WRITEM, pinfo->audio);
	MSG("REG-COM = %08x\n", pinfo->com&0x07ffffff);

	// Check register map
	for(i=0; i<10; i++) {
		ChkData = ioctl(fd, ChkAddr[i], NULL);
		MSG("0x%04x -> 0x%08x\n", ChkAddr[i], ChkData);
	}
	
	// Write new value to USER4 register 
	//ioctl(fd, 5, 0);	//remed by shixin 
#endif

	return 0;
}

#if BOARD==INTIME_RDK5
#else
#if 1 //wsy 

#define TW9903_LOSS_REG 0x01

//¶Á³ö9903µÄÊÓÆµ¶ªÊ§
int read_9903_video_loss(struct compress_struct *encoder)
{
//	int value=0;
//	int devfd;
//	int tw9903_devid;
	
	if(encoder==NULL)
		return -EINVAL;
	if(iic_fd>0)
		return read_tw9903_vloss(iic_fd,TW9903_ADDR);
	else
		return -ENOENT;//wsy 2410->9903
}

//½«ime6410Ç°¶ËµÄ9903µÄÁÁ¶Èµ÷½Úµ½Ö¸¶¨Öµ,Ð´ÈëµÄvalueÔÚ0~100%
int set_imevideo_brightness(struct compress_struct *encoder,int val)
{
	#define TW9903_BRIGHT_REG	0x10
	int tw9903_devid;
	int dwdata;
	int devfd;
	printf("set bri=%d percent\n",val);
	int value=get_value(-128,127,val);
	//gtloginfo("brightness value =%d\n",value);
	if(encoder==NULL)
		return -1;

	//encoder->brightness=value;
	devfd=encoder->fd;
	if(devfd<0)
		return -3;
//	if((value<0)||(value>255))
//		value=128;
	
	tw9903_devid=0x88;//9903µÄÉè±¸µØÖ·
	dwdata=value;//-128; //×¼±¸Ð´ÈëµÄÖµ
	dwdata<<=24;
//printf("bright= %d,0x%x\n",dwdata,dwdata);
	//gtloginfo("here\n");
	pthread_mutex_lock(&encoder->mutex);
	OpenUserIIC(devfd);
	WriteUserIIC(devfd, tw9903_devid, TW9903_BRIGHT_REG,1, dwdata,0);
	CloseUserIIC(devfd);
	pthread_mutex_unlock(&encoder->mutex);


	//add by wsy 2410->9903
	if(iic_fd>0)
		set_tw9903_brightness(iic_fd,TW9903_ADDR,val);
	
	return 0;	
}
//½«ime6410Ç°¶ËµÄ9903µÄÉ«¶Èµ÷½Úµ½Ö¸¶¨Öµ
int set_imevideo_hue(struct compress_struct *encoder,int val)
{

	#define TW9903_HUE_REG	0x15
	int tw9903_devid;
	int dwdata;
	int devfd;

	printf("set hue=%d percent\n",val);
	
	int value=get_value(-128,127,val);
	if(encoder==NULL)
		return -1;

	//encoder->hue=value;
	devfd=encoder->fd;
	if(devfd<0)
		return -3;
	
	
	tw9903_devid=0x88;//9903µÄÉè±¸µØÖ·
	dwdata=value;//-128;
	
	dwdata=dwdata<<24;
//printf("hue=0x%x\n",dwdata);
//gtloginfo("test,adjusting the hue! to %d \n",value);
	pthread_mutex_lock(&encoder->mutex);
	OpenUserIIC(devfd);
	WriteUserIIC(devfd, tw9903_devid, TW9903_HUE_REG,1, dwdata,0);
	CloseUserIIC(devfd);
	pthread_mutex_unlock(&encoder->mutex);

	//add by wsy 2410->9903
	if(iic_fd>0)
		set_tw9903_hue(iic_fd,TW9903_ADDR,val);
		
	return 0;
}
//½«ime6410Ç°¶ËµÄ9903µÄ¶Ô±È¶Èµ÷½Úµ½Ö¸¶¨Öµ
int set_imevideo_contrast(struct compress_struct *encoder,int val)
{
	#define TW9903_CONTRAST_REG	0x11
	int tw9903_devid;
	int dwdata;
	int devfd;

	printf("set con=%d percent\n",val);
	
	int value=get_value(0,255,val);
//printf("contrast value =%d\n",value);
	if(encoder==NULL)
		return -1;

	//encoder->contrast=value;
	devfd=encoder->fd;
	if(devfd<0)
		return -3;
	
	tw9903_devid=0x88;//9903µÄÉè±¸µØÖ·
	dwdata=value<<24;

//printf("contrast= %d,0x%x\n",dwdata,dwdata);
	pthread_mutex_lock(&encoder->mutex);
	OpenUserIIC(devfd);
	WriteUserIIC(devfd, tw9903_devid, TW9903_CONTRAST_REG,1, dwdata,0);
	CloseUserIIC(devfd);
	pthread_mutex_unlock(&encoder->mutex);


		//add by wsy 2410->9903
	if(iic_fd>0)
		set_tw9903_contrast(iic_fd,TW9903_ADDR,val);
	return 0;
}
//½«ime6410Ç°¶ËµÄ9903µÄ±¥ºÍ¶Èµ÷½Úµ½Ö¸¶¨Öµ0~100%
int set_imevideo_saturation(struct compress_struct *encoder,int val)
{	

	#define TW9903_SATU_REG	0x13
	#define TW9903_SATV_REG 0x14
	int tw9903_devid;
	int dwdatau;
	int dwdatav;
	int devfd;
	printf("set sat=%d percent\n",val);
	
	int value=get_value(0,200,val);
//printf("saturation value =%d\n",value);
	if(encoder==NULL)
		return -1;

	//encoder->saturation=value;
	devfd=encoder->fd;
	if(devfd<0)
		return -3;
	//printf("set sat=%d\n",value);
	tw9903_devid=0x88;//9903µÄÉè±¸µØÖ·
	dwdatav=value;
	dwdatau=dwdatav+37;
	//dwdatau=value-1;
	//dwdatav=value-38;
	dwdatau<<=24;
	dwdatav<<=24;
//printf("satu= 0x%x,satv=0x%x\n",dwdatau,dwdatav);
	pthread_mutex_lock(&encoder->mutex);
	OpenUserIIC(devfd);
	WriteUserIIC(devfd, tw9903_devid, TW9903_SATU_REG,1, dwdatau,0);
	WriteUserIIC(devfd, tw9903_devid, TW9903_SATV_REG,1, dwdatav,0);
	CloseUserIIC(devfd);
	pthread_mutex_unlock(&encoder->mutex);
	

	//add by wsy 2410->9903
	if(iic_fd>0)
		set_tw9903_saturation(iic_fd,TW9903_ADDR,val);

	return 0;
}

//sharp1¼Ä´æÆ÷,valÎªÊµ¼ÊÒªÐ´Èë¼Ä´æÆ÷µÄÖµ
int set_imevideo_sharp1(struct compress_struct *encoder,int val)
{
	#define TW9903_SHARP1_REG	0x12
	int tw9903_devid;
	int dwdata;
	int devfd;
	printf("set sharp1=0x%x\n",val);
	int value=val;
	if(encoder==NULL)
		return -1;

	devfd=encoder->fd;
	if(devfd<0)
		return -3;
	
	tw9903_devid=0x88;//9903µÄÉè±¸µØÖ·
	dwdata=value;
	dwdata<<=24;
	pthread_mutex_lock(&encoder->mutex);
	OpenUserIIC(devfd);
	WriteUserIIC(devfd, tw9903_devid, TW9903_SHARP1_REG,1, dwdata,0);
	CloseUserIIC(devfd);
	pthread_mutex_unlock(&encoder->mutex);

		//add by wsy 2410->9903
	if(iic_fd>0)
		set_tw9903_sharp1(iic_fd,TW9903_ADDR,val);
	
	return 0;	
}
int set_imevideo_sharp2(struct compress_struct *encoder,int val)
{
	#define TW9903_SHARP2_REG	0x16
	int tw9903_devid;
	int dwdata;
	int devfd;
	int value=val;
	if(encoder==NULL)
		return -1;

	devfd=encoder->fd;
	if(devfd<0)
		return -3;
	printf("set sharp2=0x%x\n",value);
	tw9903_devid=0x88;//9903µÄÉè±¸µØÖ·
	dwdata=value;
	dwdata<<=24;
	pthread_mutex_lock(&encoder->mutex);
	OpenUserIIC(devfd);
	WriteUserIIC(devfd, tw9903_devid, TW9903_SHARP2_REG,1, dwdata,0);
	CloseUserIIC(devfd);
	pthread_mutex_unlock(&encoder->mutex);

		//add by wsy 2410->9903
	if(iic_fd>0)
		set_tw9903_sharp2(iic_fd,TW9903_ADDR,val);
	
	return 0;	
}
#endif

//½«ime6410Ç°¶ËµÄ9903ÇÐ»»³ÉÖ¸¶¨Í¨µÀ
int switch_imevideo_channel(struct compress_struct *encoder,int ch)
{
	#define TW9903_CH_REG	0x02
	int tw9903_devid;
	int dwdata;
	int devfd;
	//int value;
	return 0;//remed by shixin ´Ë¹¦ÄÜÓÉÓÚ²»Ê¹ÓÃ9903ÁËËùÒÔÃ»ÓÐÓÃÁË
	if(encoder==NULL)
		return -1;

	encoder->full_ch=ch;
	devfd=encoder->fd;
	if(devfd<0)
		return -3;
	if((ch<0)||(ch>3))
		return -2;
	tw9903_devid=0x88;//9903µÄÉè±¸µØÖ·
	dwdata=ch<<2|0x40;//FIXME ÐèÒª±£ÁôÔ­À´µÄÖµÂðå
	dwdata<<=24;	//bigendian
	pthread_mutex_lock(&encoder->mutex);
	OpenUserIIC(devfd);
	WriteUserIIC(devfd, tw9903_devid, TW9903_CH_REG,1, dwdata,0);
	//printf("writein 0x%x\n",dwdata);
	//#define TW9903_LOSS_REG 0xFE //
	
	//value=ReadUserIIC(devfd,tw9903_devid,TW9903_LOSS_REG,1,1);
	//printf("\n\n\n\n\n\n\nreadout=0x%x\n\n\n\n\n",value);
	CloseUserIIC(devfd);
	pthread_mutex_unlock(&encoder->mutex);
	return 0;	
	
}

#endif


int I64_CheckRegister(struct compress_struct *encoder)
{
#if	EMBEDED
	int  i, ChkData;
	int  ChkAddr[10] = {
			0x2000, 0x2004, 0x2008, 0x200c,	0x2010, 0x2014,
			0x2028,	0x202c, 0x2030, 0x2034 };

#endif
	int fd;
	if(encoder==NULL)
		return -1;
	fd=encoder->fd;
	if(fd<0)
		return -1;
#if	EMBEDED
	// Check register map
	for(i=0; i<10; i++) {
		ChkData = ioctl(fd, ChkAddr[i], NULL);
		MSG("0x%04x -> 0x%08x\n", ChkAddr[i], ChkData);
	}
#endif
	return 0;
}	
	

/**********************************************************************************************
 * º¯ÊýÃû	:is_keyframe()
 * ¹¦ÄÜ	:ÅÐ¶ÏÒ»Ö¡Êý¾ÝÊÇ·ñÊÇ¹Ø¼üÖ¡,±¾º¯ÊýÔÚÎÄ¼þÇÐ¸îÊ±»áÓÃµ½
 * ÊäÈë	:frame ÐèÒª±»ÅÐ¶ÏµÄÊý¾ÝÖ¡
 * ·µ»ØÖµ	:0 ±íÊ¾·Ç¹Ø¼üÕì
 *			 1±íÊ¾ÊÇ¹Ø¼üÕì
 **********************************************************************************************/
int is_keyframe(struct stream_fmt_struct *frame)
{
	if((frame->media==MEDIA_VIDEO)&&(frame->type==FRAMETYPE_I))
		return 1;
	return 0;
}

#ifdef SAVE_RAW_DATA
FILE *raw_fp=NULL;
#endif
static __inline__ int readdev(int fd,void *buf,int len)
{
	int rc;
	if((fd<0)||(buf==NULL))
		return -1;
        while(1)
        {
	    rc=read(fd,buf,len);
            if(rc<0)
            {
                if(errno==EINTR)
                    continue;
            }
            break;
        }
#ifdef SAVE_RAW_DATA
	if(raw_fp==NULL)
	{
		raw_fp=fopen("/nfs/raw6410.dat","wb+");
		printf("open /nfs/raw6410.dat\n");
	}
	if(raw_fp==NULL)
	{
		printf("can't open /nfs/shixin/raw6410.dat!!!\n");
		exit(0);
	}
	if(rc>0)
		fwrite(buf,1,rc,raw_fp);
#endif	 
	 return rc;
}

/**********************************************************************************************
 * º¯ÊýÃû	:i64_read_frame()
 * ¹¦ÄÜ	:´Ó6410ÖÐ¶ÁÈ¡1Ö¡ÊÓÆµ»òÕß1Ö¡ÒôÆµÊý¾Ý
 * ÊäÈë	:encoder:ÒÑ¾­Ìî³äºÃ²ÎÊý²¢ÇÒÉè±¸ÒÑ¾­´ò¿ªµÄ½á¹¹Ìå
 *			 buf_len:Êä³ö»º³åÇøµÄ´óÐ¡£¬Èç¹ûÒ»°üÊý¾Ý´óÓÚÕâ¸öÖµÔò½Ø¶Ï	
 *			 key_f:¶ÁÈ¡¹Ø¼üÕì±êÖ¾£¬Èç¹ûÎª1Ôò¶ªÆúÆäËûÕì£¬Ö»¶ÁÈ¡iframe
 *				µ±´æÎÄ¼þµÄµÚÒ»ÕìÊý¾ÝÊ±ÐèÒªÉèÖÃ´Ë²ÎÊý£¬ÆäËûÊ±ºòÎª0
 * Êä³ö	 buffer:Ä¿±ê»º³åÇø,±¾º¯Êý½«°Ñ¶ÁÈ¡µ½µÄÊý¾Ý·ÅÈë´Ë»º³åÇø
 * ·µ»ØÖµ	:0±íÊ¾³É¹¦¸ºÖµ±íÊ¾³ö´í
 * 			-6 ±íÊ¾encoderÖÐµÄÉè±¸fdÎª¸ºÖµ
 * 			-5 ±íÊ¾ÊäÈëµÄ²ÎÊýencoderÎª¿Õ
 * 			-4 ±íÊ¾ÊäÈëµÄbufferÎª¿Õ
 * 			-3±íÊ¾²ÎÊý´íÎó
 * 			-2 ±íÊ¾ÎÄ¼þ½áÊø
 * 			-1±íÊ¾ÆäËü´íÎó
 **********************************************************************************************/
int i64_read_frame(struct compress_struct *encoder,struct stream_fmt_struct *buffer,int buf_len,int req_keyf)
{
	struct stream_fmt_struct *fb;
	int AudioFrameType;
	int rsize;
	char read_temp[DBLOCKSIZE+8];//¶à8¸ö×Ö½ÚÒÔ·ÀÍòÒ»
	unsigned char *temp;
	unsigned char *pbuff;
	unsigned long   dwChannel, dwType, dwSize, dwKeyframe;//changed by shixin from int
	int BlockSize = 0;
	int DropFrame[NR_CHANNEL];
	//int nFirstVideo[NR_CHANNEL];
	int devfd;
	if(buffer==NULL)
		return -4;	
	if(encoder==NULL)
		return -5;
	devfd=encoder->fd;
	if(devfd<0)
		return -6;
	fb=buffer;
	temp=read_temp;
	AudioFrameType=encoder->AudioFrameType;
#if 0
#ifdef SAVE_RAW_DATA
		while(1)
		{
			rsize = readdev(devfd, temp, DBLOCKSIZE) ;
			if(rsize!=DBLOCKSIZE)
			{
				printf("ime6410api rsize != DBLOCKSIZE,rsize=%d\n",rsize);
			}
		}

#endif
#endif
	while(1)
	{

		// Read Stream From IME6410 driver
		if((rsize = readdev(devfd, temp, DBLOCKSIZE)) < 0)
		{
			//ERR("Update ...\n");
	        	//ERR("[DSERVER] Error in Reading From Device\n");
			printf("6410api read:rsize=%d\n", rsize);
			return -1;
		}
		else if(rsize!=DBLOCKSIZE)
		{
			printf("6410api(%s) read %d bytes when read pkt head need 512\n",encoder->dev_node,rsize);
			if(rsize==0)
				return -2;
			continue;
		}

		// Check Block Type.. 
		// (Video Header Block?, Audio Block?, Video Data Block?)
		dwChannel = (temp[0] << 8) | temp[1];
		dwType    = (temp[2] << 8) | temp[3];
		if(dwChannel < NR_CHANNEL)//ÅÐ¶ÏÍ¨µÀºÅÊÇ·ñºÏ·¨£¬6400ÓÐ4¸öÍ¨µÀ£¬6410Ö»ÓÐÒ»¸ö
		{
		
			if(dwType == FRAMETYPE_I || dwType == FRAMETYPE_P)//´¦ÀíÍ¼Ïñ°ü
			{
				dwSize=0;
				dwSize = (temp[4]<<24) | (temp[5]<<16) | (temp[6]<<8) | temp[7];

				if(dwSize>0)//Èç¹ûÐ¡ÓÚµÈÓÚ0Ôò²»¹Ü
				{
					// Total Block Size (512bytes-aligned)
					BlockSize = (((dwSize+511)>>9)<<9) - DBLOCKSIZE;//8;	//8ÊÇÒÑ¾­¶ÁÈ¡µÄ°üÍ·
								//blockÊÇÐèÒª¶ÁÈ¡µÄÊý¾Ý´óÐ¡£¬°üÀ¨Ìî³äÎ»
					DropFrame[dwChannel] = 0;
					// Check I-frame
					if(dwType == FRAMETYPE_I)
					{
						dwKeyframe = 1;			//ÉèÖÃ¹Ø¼üìõ±êÖ¾
						//DropFrame[dwChannel] = 0;//ÊÕµ½iìõºóÇå³ý¶ªÆú°ü±êÖ¾
						//nFirstVideo[dwChannel] = 1;
					}
					else
					{
						dwKeyframe = 0;
						if(req_keyf==1)
							DropFrame[dwChannel]=1;
					}
					/*if(dwChannel!=0)
					{
						DropFrame[dwChannel]=1;//Ä¿Ç°6410Ö»Ö§³ÖÒ»Â·Ñ¹Ëõ
						printf("ime6410api recv a channel:%d\n", (int)dwChannel);
					}*/
					if((BlockSize+30)>buf_len)
					{
						DropFrame[dwChannel]=1;
//						printf("%s drop a len=%d frame\n",encoder->dev_node,BlockSize);
					}
					// Drop P-Frame
					if(DropFrame[dwChannel]) {
//						printf("%s Drop a frame : P-frame\n",encoder->dev_node);
						while(BlockSize>0)
						{
							if(BlockSize < DBLOCKSIZE)
								rsize = readdev(devfd, temp, BlockSize);
							else
								rsize = readdev(devfd, temp, DBLOCKSIZE);
							if(rsize<0)
							{
								printf("error in reading from %s:%d\n",encoder->dev_node,rsize);
								return -1;
							}
							else if(rsize==0)
							{
								printf("read 0 byte from %s:%d\n",encoder->dev_node,rsize);
							
								return -2;
							}
							BlockSize -= rsize;
						}
						continue;
					}
					
			
					// Make I/F Header With RealTime Viewer..
					fb->channel=dwChannel;
					fb->media=MEDIA_VIDEO;
					fb->type=dwType;
					pbuff=fb->data;					
					memcpy(pbuff,&temp[8],(DBLOCKSIZE-8));
					pbuff+=(DBLOCKSIZE-8);
					// Read All Remaining Data..	
					while(BlockSize>0)
					{
						if(BlockSize<DBLOCKSIZE)
							printf("BlockSize(%d)<DBLOCKSIZE!!!\n",BlockSize);
						rsize = readdev(devfd, pbuff, DBLOCKSIZE);
						if(rsize < 0) {
							//ERR("[DSERVER] Error in Reading From Device (%d)\n", rsize);							
							printf("error in read remain data  from device :%d error=%s\n",rsize,strerror(errno));
							return -2;//-1;
						}
						else if(rsize==0)
							return -2;

						pbuff += rsize;
						BlockSize -= rsize;
						if(BlockSize<0)
						{
							printf("%s remain block size=%d",encoder->dev_node,BlockSize);
						}
						
					}
					// Put VideoFrame into Queue	
					//fb->len=dwSize-8;
					dwSize-=8;
					dwSize = ((dwSize+1)>>1)<<1;//changed by shixin
					fb->len=dwSize;
					/*if((fb->len&1)==1)
					{
						*pbuff=0;//²¹0					
						//fb->len=(fb->len+1)>>1;
						//fb->len<<=1;//°´16bit¶ÔÆë
						fb->len+=1;
					}*/
					//PSend(FrameBuff, dwSize);
	// ################################################################
					#if EMBEDED==0
						usleep(40000);
					#endif
					if(gettimeofday(&buffer->tv,NULL)<0)
					{
						printf("err gettimeofday!\n");
						memset(&buffer->tv,0,sizeof(buffer->tv));
					}
					return 0;
				}
			}
			// Audio Frame has 6 Audio Block,
			// and each block has 512 bytes.
			// You can adjust this size by changing 'NR_AUDIO_PKT'
			// ABuf => Pointing Start Of Audio Frame Buffer
			// AReBuf => Pointing Current Write Position Of Audio Frame
			//           Buffer
			// AudSize[] => Currently Received Data-Size For Audio Frame
			//              (Increaing Counter)
			// AudBlockSize => Data-Size Left For Audio Frame
			//              (Decreaing Counter)
			else if(dwType == AudioFrameType)
			{				
				//DBGH("line:%d\n",__LINE__);
				if(APacketStart[dwChannel])
				{
				//DBGH("\n audio packet:%d",dwType);
				//	pentry = getentry(&FramePool);
					
					AudSize[dwChannel]  = 0;//
					AudBlockSize[dwChannel] = NR_AUDIO_PKT*(DBLOCKSIZE-4);
					APacketStart[dwChannel] = 0;
					// Make I/F Header With RealTime Viewer..
					/*
					*(pbuff-4) = SP_DATA;
					*(pbuff-3) = MEDIA_AUDIO;
					*(pbuff) = (aPacketSeq[dwChannel]++);
					*(pbuff+3) = ((dwChannel & 0xf) << 4) | 2; // 2 = AudioType
					pbuff += 4
					*/
				}
				memcpy(&audio_temp_buf[AudSize[dwChannel]],temp+4,DBLOCKSIZE-4);//¶Á»Ø°üÍ·µÄ4¸ö×Ö½Ú£¬¶ÔÓÚÉùÒôÀ´ËµÕâÊÇÊý¾Ý
				AudSize[dwChannel]+=(DBLOCKSIZE-4);
			#if 0
				// Read a audio block				
				rsize = readdev(devfd, &audio_temp_buf[AudSize[dwChannel]], DBLOCKSIZE-8);	//È¥µôÍ·ÉÏµÄ8×Ö½Ú
				if(rsize>0)
					AudSize[dwChannel] += rsize;	
				else if(rsize==0)
					return -2;
				else
				{
					printf("error in read from 6410 %d\n",rsize);
					return -1;
				}
			#endif	

				// Send a audio block
				if((AudSize[dwChannel] >= AudBlockSize[dwChannel]) )//&& nFirstVideo[dwChannel])//ÔÚ
				{
					//DBGH("line:%d\n",__LINE__);
					//PSend(ABuf[dwChannel], AudBlockSize[dwChannel]+4);
					fb->channel=dwChannel;
					fb->media=MEDIA_AUDIO;
					fb->type=dwType;
					fb->len=AudBlockSize[dwChannel];
					AudSize[dwChannel] = 0;
					APacketStart[dwChannel] = 1;
					memcpy(fb->data,audio_temp_buf,AudBlockSize[dwChannel]);
					if(gettimeofday(&buffer->tv,NULL)<0)
					{
						printf("err gettimeofday!\n");
						memset(&buffer->tv,0,sizeof(buffer->tv));
					}
					return 0;
				}
				/*else if(AudSize[dwChannel] == 0)//ÊÕÂúÁËÒ»°ü£¬»»ÏÂÒ»°ü
				{
					APacketStart[dwChannel] = 1;
				}*/

				continue;
			}
			///lsk 2006 -11-24 
//			else if(dwType == FRAMETYPE_MD)
			else if((dwType == FRAMETYPE_P_MD)||(dwType == FRAMETYPE_I_MD))
//			else if((dwType == FRAMETYPE_I_MD))
			{
				// External motion detection data
				/*rsize=readdev(devfd, temp+8, DBLOCKSIZE-8);
				if(rsize<=0)
					return -2;
				*/
//				if(dwType == FRAMETYPE_I_MD)
//				if(dwType == FRAMETYPE_P_MD)
//					continue;
				// lsk 2006 -12 -1
//				printf("External motion detection pkt received comd = %08lx!!!!!!!!!\n", encoder->i64reg.com);
				#if 0
				switch(encoder->i64reg.com.VidPicSize)
				{
					
					case 3:
						process_motion_pkt(dwChannel, &temp[4], SCR_MODE_QCIF);
					break;
					case 5:
						process_motion_pkt(dwChannel, &temp[4], SCR_MODE_CIF);
					break;
					case 7:
						process_motion_pkt(dwChannel, &temp[4], SCR_MODE_D1);
					break;
					case 14://FIXME!!!!!
						process_motion_pkt(dwChannel, &temp[4], SCR_MODE_D1);
					break;
					case 15://FIXME!!!!!
						process_motion_pkt(dwChannel, &temp[4], SCR_MODE_HD1);
					break;
					default:
						printf("Unsupported Picture Size %d\n",encoder->i64reg.com.VidPicSize);
						return -1;
					break;
				}
				#endif
				continue;
			}
			else
			{
				if(AudioFrameType == FRAMETYPE_PCM)
					printf("This firmware is for PCM. Please check your firmware.\n");
				else
				{
					printf("6410api recv unknow type pkt %d\n",(int)dwType);
				}
			}
			memset(read_temp,0,sizeof(read_temp)); // lsk  2006-12-21
		}
		
	//		printf("Search Start Block(T:%08x C:%08x)\n", dwType, dwChannel);
		/*if(readdev(devfd, temp, DBLOCKSIZE-8)<0) // Drop 512-8 bytes
		{
			printf("error in read data from 6410 !\n");
			return -1;
		}
		else
			*/
		printf("6410 api drop unknow channel data ch=0x%08x\n",(int)dwChannel);
		//fflush(stdout);
	}	
	return 0;	
}


















/*******************************************************************************
	¶ÁÈ¡ÅäÖÃÎÄ¼þÏà¹ØµÄ´úÂë
********************************************************************************/

static int CheckVal(int low, int up, int val)
{
	if((val >= low) &&( val <= up)) 
		return 1;

	return 0;
};

//¶ÁÈ¡ÊÓÆµÂëÁ÷ÀàÐÍ
static int get_encoder_type(int type)
{
	if(type==1)
		return  3;//ÏµÍ³ÂëÁ÷(´øÉùÒô)
	else
		return 1;	//µ¥ÊÓÆµÂëÁ÷		
}
static int get_video_standard(char *stand)
{
	if(stand==NULL)
		return 0;	//mpeg4
					//0-mpeg4 1-mpeg2 2-mpeg1
	return 0;		//Ä¿Ç°Ö»Ö§³Ömpeg4
}
 int get_picture_size(int ispal,int size)
{
	int ret;
/*# 0 = 320x240
# 1 = 640x480
# 2 = 720x480
# 3 = 176x144
# 4 = 320x288
# 5 = 352x288
# 6 = 704x576
# 7 = 720x576
# 8
# 9
# 14 = 704x288
# 15 = 720x288

*/
	ret=5;
	switch(size)
	{
		case 0:
                	if(ispal==PAL)
                        	ret=7;  //D1
                	else
                        	ret=2;
		break;
		case 1://CIF
               		if(ispal==PAL)
                        	ret=5;
                	else
                        	ret=0;
		break;
		case 2://QCIF
                	if(ispal==PAL)
                	        ret=3;
                	else
                        	ret=0;
		break;
		case 3://HD1
                	if(ispal==PAL)
                        	ret=15;
                	else
                        	ret=0;
		break;
#if 0//ÐèÒªµ÷½ÚÊÓÆµÇ°¶Ë²ÅÄÜÖ§³Ö(9903,2824)
		case 4://4CIF
			if(ispal==PAL)
				ret=6;
		break;
#endif
	}
	
	return ret;
}


/**********************************************************************************************
 * º¯ÊýÃû	:reinstall_encdrv()
 * ¹¦ÄÜ	:ÖØÐÂ°²×°6410Çý¶¯³ÌÐò
 * ÊäÈë	:encÃèÊö±àÂëÆ÷µÄÊý¾Ý½á¹¹
 * ·µ»ØÖµ	:0 ±íÊ¾³É¹¦£¬¸ºÖµ±íÊ¾Ê§°Ü
 **********************************************************************************************/
int reinstall_encdrv(struct compress_struct *enc)
{
	int rc=0;
#if EMBEDED
	char pbuf[40];
	sprintf(pbuf,"rmmod %s",enc->dev_name);
	rc=system(pbuf);
	//printf("rm:%s\n",pbuf);
	sleep(1);
	sprintf(pbuf,"insmod %s",enc->driver_name);
	//printf("ins:%s\n",pbuf);
	rc=system(pbuf);
	enc->init_tw9903flag=0;	
#endif
	return rc;
}


/**********************************************************************************************
 * º¯ÊýÃû	:read_enc_para_file()
 * ¹¦ÄÜ	:´ÓÖ¸¶¨ÅäÖÃÎÄ¼þÀïµÄÖ¸¶¨½ÚÖÐ¶ÁÈ¡±àÂë²ÎÊý
 * ÊäÈë	:filename:ÅäÖÃÎÄ¼þÃû
 *			 section:ÅäÖÃÎÄ¼þÖÐµÄ½Ú,½ÚÃû²»ÄÜ³¬¹ý20byte
 * Êä³ö	 enc:Ö¸ÏòÒª´æ·Å²ÎÊýµÄÖ¸Õë	,º¯Êý·µ»ØÊ±»á±»Ìî³äºÃ
 * ·µ»ØÖµ	:0±íÊ¾³É¹¦¸ºÖµ±íÊ¾³ö´í
 **********************************************************************************************/
int read_enc_para_file(char *filename,char *section, struct compress_struct   *enc)
{
	char parastr[100];
	char *pstr,*cat;
	int section_len,num;
	dictionary      *ini;	
	if(filename==NULL)
		return -1;
	if(section==NULL)
		return -1;
	if (enc==NULL)
		return -1;
	section_len=strlen(section);
	if(section_len>20)
		return -1;
	ini=iniparser_load(filename);
	if (ini==NULL) {
                printf("parse ini file file [%s]\n", filename);
                return -1 ;
        }
	//iniparser_dump_ini(ini,stdout);
	memcpy(parastr,section,section_len);
	parastr[section_len]=':';
	section_len++;
	
	parastr[section_len]='\0';
	cat=strncat(parastr,"EncodingType",strlen("EncodingType"));	
	num=iniparser_getint(ini,parastr,0);	//Ä¬ÈÏÎªÊÓÆµÁ÷
	num=get_encoder_type(num);
	enc->i64reg.com.EncodingType=num;


	parastr[section_len]='\0';
	cat=strncat(parastr,"VideoStand",strlen("VideoStand"));	
	pstr=iniparser_getstring(ini,parastr,"mpeg4");
	num=get_video_standard(pstr);
	enc->i64reg.com.VideoStand=num;

	parastr[section_len]='\0';
	cat=strncat(parastr,"VidPicSize",strlen("VidPicSize"));	
	enc->NTSC_PAL = PAL;		//Ç¿ÖÆÉè³Épal
	num=iniparser_getint(ini,parastr,1);//CIF
	num=get_picture_size(PAL,num);
	enc->i64reg.com.VidPicSize=num;

	parastr[section_len]='\0';
	cat=strncat(parastr,"FrameRate",strlen("FrameRate"));	
	num=iniparser_getint(ini,parastr,0);
	num=get_frame_rate(PAL,num);
	enc->i64reg.com.FrameRate=num;
	enc->frame_rate = enc->NTSC_PAL ? 25.0 : 29.97;//enc->NTSC_PAL ? 25.0 : 29.97;
	enc->frame_rate /= (1<<num);
	if(enc->frame_rate<=0)
		enc->frame_rate=1;
	parastr[section_len]='\0';
	cat=strncat(parastr,"PicEncType",strlen("PicEncType"));	
	num=iniparser_getint(ini,parastr,0);
	if(num>1)
		num=0;
	enc->i64reg.com.PicEncType=num;	//0:i/p 1:I only//FIXME support all mode?

	parastr[section_len]='\0';
	cat=strncat(parastr,"Quant",strlen("Quant"));	
	num=iniparser_getint(ini,parastr,12);	
	if(CheckVal(2, 30, num)) 
	{
		//ÔÚ·¶Î§ÄÚ
	}
	else
		num=15;
	enc->i64reg.cbr.Ch1Quant =
	enc->i64reg.cbr.Ch2Quant =
	enc->i64reg.cbr.Ch3Quant = 
	enc->i64reg.cbr.Ch4Quant = num;
	

	parastr[section_len]='\0';
	cat=strncat(parastr,"IInterval",strlen("IInterval"));	
	num=iniparser_getint(ini,parastr,3);
	if(CheckVal(0, 7, num)) 
	{
	}
	else
	{
		num=3;
	}
	enc->i64reg.com.IInterval=num;//I Picture Interval
							//# 0 =  2,  1 =  4,  2 =   8,  3 =  16
							//# 4 = 32,  5 = 64,  6 = 128,  7 = 256

	parastr[section_len]='\0';
	cat=strncat(parastr,"BitRateCon",strlen("BitRateCon"));	
	num=iniparser_getint(ini,parastr,1);
	if(CheckVal(0,2,num))
	{//0:vbr  1:cbr  2:hbr
		enc->i64reg.cbr.Ch1CbrMode =
		enc->i64reg.cbr.Ch2CbrMode =
		enc->i64reg.cbr.Ch3CbrMode = 
		enc->i64reg.cbr.Ch4CbrMode = num;
	}


	if(num==1)
	{//cbr
		parastr[section_len]='\0';
		cat=strncat(parastr,"TargetBitRate",strlen("TargetBitRate"));	
		num=iniparser_getint(ini,parastr,256);
		if(num>0)
		{
			num*=1000;
			num/=enc->frame_rate;
			enc->i64reg.cbrval[0].low=num&0xffff;
			enc->i64reg.cbrval[0].high=(num>>16)&0xffff;
			
		}
	}
	else if(num==2)
	{//hbr
		parastr[section_len]='\0';
		cat=strncat(parastr,"MinBitRate",strlen("MinBitRate"));	
		num=iniparser_getint(ini,parastr,64);
		if(num>0)
		{			//min
			num/=enc->frame_rate;
			enc->i64reg.cbrval[0].high=(num)&0xffff;
		}
		parastr[section_len]='\0';
		cat=strncat(parastr,"MaxBitRate",strlen("MaxBitRate"));	
		num=iniparser_getint(ini,parastr,256);
		if(num>0)
		{			//max
			num/=enc->frame_rate;
			enc->i64reg.cbrval[0].low=num&0xffff;
		}
	}
	else
	{
		//VBR
		//vbrÖ»Òª¶ÁÈ¡ÖÊÁ¿²ÎÊý¾Í¿ÉÒÔÁË£¬ÉÏÃæÒÑ¾­¶ÁÈ¡¹ýÁË
	}

	enc->i64reg.com.ChannelActive=1;

//audio
	if(enc->i64reg.com.EncodingType==3)//ÏµÍ³ÂëÁ÷
	{
		enc->i64reg.audio.ChannelActive = 1;		
	}
	else
	{	//ÊÓÆµÂëÁ÷
		enc->i64reg.audio.ChannelActive =0;
	}

	enc->i64reg.audio.Channel=0;	//µ¥ÉùµÀ
	enc->i64reg.audio.Bitrate=0;

	parastr[section_len]='\0';
	cat=strncat(parastr,"AudEncMode",strlen("AudEncMode"));
	num=iniparser_getint(ini,parastr,1);
	if(CheckVal(1, 2, num)) 
	{// 1:mu-law pcm 2:adpcm 3 raw-pcm
		enc->i64reg.audio.AudEncMode = num;
		//if(num == 1) //²»Ö§³Öadpcm
			enc->AudioFrameType = FRAMETYPE_PCM;
		//else 
		//	enc->AudioFrameType = FRAMETYPE_ADPCM;
	}
	else if(num==3)
	{//raw-pcm
		enc->i64reg.audio.AudEncMode = 0;
		enc->AudioFrameType=FRAMETYPE_RAWPCM;
	}
	else
		enc->AudioFrameType=0;


	num=iniparser_getint(ini,"video0:bright",49);
	enc->brightness=num;

	num=iniparser_getint(ini,"video0:color",49);
	enc->hue=num;

	num=iniparser_getint(ini,"video0:contrast",60);
	enc->contrast=num;

	num=iniparser_getint(ini,"video0:saturation",70);
	enc->saturation=num;

	num=iniparser_getint(ini,"video0:sharp1",15);//15);
	enc->sharp1=num;

	num=iniparser_getint(ini,"video0:sharp2",195);
	enc->sharp2=num;
////lsk 2007-1-5 motion detection 	
//	num = iniparser_getint(ini, "video0:motion_alarm",0);
//	enc->md_enable = num;
//	enc->i64reg.md.Ch1MdType = num;

	num = iniparser_getint(ini, "video0:motion_sen", 0);
	set_ime_motion_sens(enc,0,num);	//changed by shixin ÒÆ¶¯Õì²â²ÎÊý
	if(num>0)
	{
		enc->md_enable=1;              
	}
       else
		enc->md_enable=0;
       enc->md_sense=num;
       
	iniparser_freedict(ini);
	return 0;
	
}

#if 0
//¶¯Ì¬ÉèÖÃ6410µÄÂëÁ÷
//ÉèÖÃÍêÒÔºó¾Í±ä³É¶¨ÂëÁ÷ÁË
//bitrate µ¥Î» k
int SetVEncBitRate(struct compress_struct *Enc,int bitrate)
{
	unsigned long Val,V1;
	if(Enc->fd<0)
		return -EINVAL;
	Val=ioctl(Enc->fd,REG_CBRCTRL,NULL);
	printf("%x=%x ",REG_CBRCTRL,Val);
	Val&=~(7<<5);
	Val|=1<<5;
	 ioctl(Enc->fd, REG_CBRCTRL | CMD_WRITEM, Val);
	Val=ioctl(Enc->fd,REG_CBRCTRL,NULL);
	 printf("%x=%x \n",REG_CBRCTRL,Val);
	 
	 Val=bitrate*1000;
	 Val/=Enc->frame_rate;
	 
	 ioctl(Enc->fd, REG_CH1CBR | CMD_WRITEM, Val);
	 Val=ioctl(Enc->fd,REG_CH1CBR,NULL);
	  printf("%x=%x \n",REG_CH1CBR,Val);
	 return 0;	
}

//ÖØÐÂÆô¶¯ÊÓÆµ±àÂëÆ÷½øÐÐÑ¹Ëõ
int RestartVCompress(struct compress_struct *encoder)
{
	unsigned long Val;
	if(encoder->fd<0)
		return -EINVAL;
	Val= ioctl(encoder->fd, REG_COMMAND, NULL);
	printf("REG_COMMAND=%x ",Val);
	Val&=~7;
	Val|=0x02;
	 ioctl(encoder->fd, REG_COMMAND | CMD_WRITEM, Val&0x07ffffff);
	usleep(50000);
	
	Val&=~7;
	Val|=1;
	 ioctl(encoder->fd, REG_COMMAND | CMD_WRITEM, Val&0x07ffffff);
	Val= ioctl(encoder->fd, REG_COMMAND, NULL);
	printf("REG_COMMAND=%x \n",Val);
	return 0;
}

#endif


