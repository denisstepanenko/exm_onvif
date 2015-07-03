
/*
 *  Title : ime6400.h
 * 
 *  Note : IME6410 Information
 *    
 *  Author : 
 *    Y.M Lee        (mgandi@intime.co.kr)
 * 
 *  Copyright :
 *    Copyright (C) InTime Corp.
 *    This Program is an application software for IME6410-RDK3,
 *    
 *  History :
 *    10/31/02  First Release
 *
 */  


#ifndef _IME6410_H_
#define _IME6410_H_

#define	NTSC				0
#define	PAL				1

// IOCTL PARAM TYPE ---------------------------------

// IME6410 IOCTL COMMAND
#define CMD_READM		0
#define CMD_WRITEM		1
#define CMD_MSC2		2
#define CMD_RESET		3
#define CMD_READH		4
#define CMD_WRITEH		5
#define CMD_IRQENABLE	6
#define CMD_IRQDISABLE	7

// IME6410 Register Address
#define REG_COMMAND         	0x2000
#define REG_AUDIO           		0x200C
#define REG_CBRCTRL         	0x2010
#define REG_MOTIONDETECT    0x2014
#define REG_MDFRAMECNT	0x2018
#define REG_I2CCTRL			0x2020
#define REG_I2CDATA			0x2024
#define REG_CH1CBR			0x2028
#define REG_CH2CBR			0x202c
#define REG_CH3CBR			0x2030
#define REG_CH4CBR			0x2034
//// motion detection threshold register  lsk 2006 -11-24
#define REG_MDTHRED0		0x3700
#define REG_MDTHRED1		0x3708
#define REG_MDTHRED2		0x3710
#define REG_MDTHRED3		0x3718

// ALARM-GPIO IOCTL COMMAND
#define ALARM_STATUS		1
#define ALARM_CLEAR			2

// IME6410 DRIVER ENVIRONMENT VARIABLES -------------

// NUMBER OF CHANNEL
#define NR_CHANNEL			1//changed by shixin from 4

// DRIVER BLOCK SIZE - Bytes
#define DBLOCKSIZE			512

// DRIVER BLOCK SIZE - 2-Power
#define DBLOCK2POWER		9

// AUDIO BLOCK
#define NR_AUDIO_BLK		6

// AUDIO PKT 
#define NR_AUDIO_PKT		4//12//changed by shixin

// AUDIO FRAME BYTE SIZE (In Case Of Stereo)
#define AUDIO_BYTE_PER_FRAME	768 

// VIDEO SIGNATURE CODE
#define FRAMETYPE_I		0x0		// IME6410 Header - I Frame
#define FRAMETYPE_P		0x1		// IME6410 Header - P Frame
#define FRAMETYPE_B		0x2
#define FRAMETYPE_PCM	0x5		// IME6410 Header - Audio Frame
#define FRAMETYPE_ADPCM	0x6		// IME6410 Header - Audio Frame
#define FRAMETYPE_P_MD	0x8			//lsk 2006 -11-24  IME6410 motion detection P packet header
#define FRAMETYPE_I_MD	0x9			//lsk 2006 -11-24 IME6410 motion detection I packet header
#define FRAMETYPE_RAWPCM	0x0f		//added by shixin 

#define VSIG_I			0xb0010000
#define VSIG_P			0xb6010000







// COMMAND
////////////////////////////////////////////////////////////

typedef struct tag_I64_COMMAND
{
	unsigned Start			: 1; // 0
	unsigned Stop			: 1; // 1
	unsigned Pause			: 1; // 2
	unsigned EncodingType	: 2; // 3 ~ 4
	unsigned VideoStand		: 2; // 5 ~ 6
	unsigned VidPicSize		: 4; // 7 ~ 10
	unsigned FrameRate		: 4; // 11 ~ 14
	unsigned PicEncType		: 2; // 15 ~ 16
	unsigned Reserved		: 6; // 17 ~ 22
	unsigned IInterval		: 4; // 23 ~ 26
	unsigned ChannelMode	: 1; // 27
	unsigned ChannelActive	: 4; // 28 ~ 31
} I64_COMMAND;

// AUDIO
//////////////////////////////////////////////////////////

typedef struct tag_I64_AUDIO
{
	unsigned Sampling		: 2;	// 0 ~ 1
	unsigned Channel			: 1;	// 2
	unsigned Bitrate			: 3;	// 3 ~ 5
	unsigned AudEncMode		: 2;	// 6 ~ 7
	unsigned Reserved		: 20;// 8 ~ 27
	unsigned ChannelActive	: 4;	// 28 ~ 31
} I64_AUDIO;


// CBRCTRL
//////////////////////////////////////////////////////////

typedef struct tag_I64_CBRCTRL
{
	unsigned Ch1Quant		: 5;
	unsigned Ch1CbrMode		: 3;

	unsigned Ch2Quant		: 5;
	unsigned Ch2CbrMode		: 3;
	
	unsigned Ch3Quant		: 5;
	unsigned Ch3CbrMode		: 3;
	
	unsigned Ch4Quant		: 5;
	unsigned Ch4CbrMode		: 3;
} I64_CBRCTRL;

typedef struct tag_I64_CBRVAL
{
	unsigned low			: 16;
	unsigned high			: 16;
} I64_CBRVAL;
// Motion Detect
/////////////////////////////////////////////////////////

typedef struct tag_I64_MOTIONDETECT
{
	unsigned Ch1MdLevel		: 6;
	unsigned Ch1MdType		: 1;
	unsigned Ch1MdEnable	: 1;
			
	unsigned Ch2MdLevel		: 6;
	unsigned Ch2MdType		: 1;
	unsigned Ch2MdEnable	: 1;

	unsigned Ch3MdLevel		: 6;
	unsigned Ch3MdType		: 1;
	unsigned Ch3MdEnable	: 1;

	unsigned Ch4MdLevel		: 6;
	unsigned Ch4MdType		: 1;
	unsigned Ch4MdEnable	: 1;
} I64_MOTIONDETECT;

typedef struct tag_I64_MOTIONTHRED
{   
	unsigned thrd1			: 16;
	unsigned thrd2			: 16;
}I64_MOTIONTHRED;  
struct I64Reg
{
	I64_COMMAND  com;
	I64_AUDIO    audio;
	I64_CBRCTRL  cbr;
	I64_MOTIONDETECT  md;
	I64_CBRVAL  cbrval[4];
	///// lsk 2006 -11-24
	I64_MOTIONTHRED md_thred[4][2];
};

struct I64Reg_int
{   
	int  com;
	int  audio;
	int  cbr;
	int  md;
	int  cbrval[4];
	//// lsk 2006 -11-24
	int md_thred[4][2];
};  


/////////////////////////////////////////////////////////////////////////////////

#endif
