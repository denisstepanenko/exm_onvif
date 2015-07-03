
/*
 *  Title : AVIEncoder.h
 * 
 *  Note :  AVI Header File
 *    
 *  Author : 
 *    S.H Shin       (likeue@intime.co.kr)
 *    Y.M Lee        (mgandi@intime.co.kr)
 * 
 *  Copyright :
 *    Copyright (C) InTime Corp.
 *    This Program is an application software for IME6400-RDK3,
 *    
 *  History :
 *    10/31/02  First Release
 *
 */  


#ifndef _AVIENCODER_H_
#define _AVIENCODER_H_

#include <stdio.h>
#include <ime6410api.h>
#include <media_api.h>
// AVI (Standard) Data Structure - Start >>>>>>>>>>>>>>>>>>>>>>>>>>>>>

// AVI Header -------------------------------------------

typedef struct {
	unsigned long dwMicroSecPerFrame;
	unsigned long dwMaxBytesPerSec;
	unsigned long dwReserved1;
	unsigned long dwFlags;
	unsigned long dwTotalFrames;
	unsigned long dwInitialFrames;
	unsigned long dwStreams;
	unsigned long dwSuggestedBufferSize;
	unsigned long dwWidth;
	unsigned long dwHeight;
	unsigned long dwScale;
	unsigned long dwRate;
	unsigned long dwStart;
	unsigned long dwLength;
} AVIMAINHEADER;

// Media Stream Header ----------------------------------

#define AVIF_HASINDEX		0x00000010
#define AVIF_MUSTUSEINDEX	0x00000020
#define AVIF_ISINTERLEAVED	0x00000100
#define AVIF_TRUSTCKTYPE	0x00000800
#define AVIF_WASCAPTUREFILE	0x00010000
#define AVIF_COPYRIGHTED	0x00020000

#define AVIF_KNOWN_FLAGS	0x00030130

#define Defined_MainAVIHeader_Size	(14*4)

#define AVISF_DISABLED		0x00000001
#define AVISF_VIDEO_PALCHANGES	0x00010000

#define AVISF_KNOWN_FLAGS	0x00010001

#define Defined_AVIStreamHeader_Size_old	(12*4)
#define Defined_AVIStreamHeader_Size		(14*4)


#define AVIIF_KNOWN_FLAGS	0x0fff0171

#define Defined_AVIINDEXENTRY_Size	(4*4)

typedef struct {
	unsigned long fccType;
	unsigned long fccHandler;
	unsigned long dwFlags;
	unsigned long dwPriority;
	unsigned long dwInitialFrames;
	unsigned long dwScale;
	unsigned long dwRate;
	unsigned long dwStart;
	unsigned long dwLength;
	unsigned long dwSuggestedBufferSize;
	unsigned long dwQuality;
	unsigned long dwSampleSize;
	unsigned short rcFrame[4];
} AVISTREAMHEADER;

// Video Format Header ----------------------------------
typedef struct {
	unsigned long biSize;
	unsigned long biWidth;
	unsigned long biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned long biCompression;
	unsigned long biSizeImage;
	unsigned long biXPelsPerMeter;
	unsigned long biYPelsPerMeter;
	unsigned long biClrUsed;
	unsigned long biClrImportant;
} BITMAPINFOHEADER;

// Audio Format Header -----------------------------------

// wFormatTag Value
#define WFORMAT_RAWPCM		0x0001
#define WFORMAT_ALAW			0x0006
#define WFORMAT_ULAW			0x0007
#define WFORMAT_AAC				0x00FF
#define WFORMAT_ADPCM			0x0011
#define WFORMAT_MPEG			0x0050
#define WFORMAT_MPEGLAYER3	0x0055

typedef struct {
	unsigned short wFormatTag;
	unsigned short nChannels;
	unsigned long nSamplesPerSec;
	unsigned long nAvgBytesPerSec;
	unsigned short nBlockAlign;
	unsigned short wBitsPerSample;
	unsigned short cbSize; // 22
} WAVEFORMATEX;	// 18

typedef struct {
	unsigned short wFormatTag;
	unsigned short nChannels;
	unsigned long nSamplesPerSec;
	unsigned long nAvgBytesPerSec;
	unsigned short nBlockAlign;
	unsigned short wBitsPerSample;
	unsigned short cbSize; // 22	
	unsigned short  wSamplesPerBlock;
} ADWAVEFORMATEX;

typedef struct mpeg1waveformat_tag {
	WAVEFORMATEX    wfx;
	unsigned short	fwHeadLayer;
	unsigned long	dwHeadBitrate;
	unsigned short	fwHeadMode;
	unsigned short	fwHeadModeExt;
	unsigned short	wHeadEmphasis;
	unsigned short	fwHeadFlags;
	unsigned long	dwPTSLow;
	unsigned long	dwPTSHigh;
} MPEG1WAVEFORMAT; // 40

typedef	MPEG1WAVEFORMAT	*PMPEG1WAVEFORMAT;

#define ACM_MPEG_LAYER1		(0x0001)
#define ACM_MPEG_LAYER2		(0x0002)
#define ACM_MPEG_LAYER3		(0x0004)
#define ACM_MPEG_STEREO		(0x0001)
#define ACM_MPEG_JOINTSTEREO	(0x0002)
#define ACM_MPEG_DUALCHANNEL	(0x0004)
#define ACM_MPEG_SINGLECHANNEL	(0x0008)
#define ACM_MPEG_PRIVATEBIT	(0x0001)
#define ACM_MPEG_COPYRIGHT	(0x0002)
#define ACM_MPEG_ORIGINALHOME	(0x0004)
#define ACM_MPEG_PROTECTIONBIT	(0x0008)
#define ACM_MPEG_ID_MPEG1	(0x0010)

#define MPEGLAYER3_ID_UNKNOWN		0
#define MPEGLAYER3_ID_MPEG		1
#define MPEGLAYER3_ID_CONSTANTFRAMESIZE	2

#define MPEGLAYER3_FLAG_PADDING_ISO	0x00000000
#define MPEGLAYER3_FLAG_PADDING_ON	0x00000001
#define MPEGLAYER3_FLAG_PADDING_OFF	0x00000002

//
// MPEG Layer3 WAVEFORMATEX structure
// for WAVE_FORMAT_MPEGLAYER3 (0x0055)
//
#define MPEGLAYER3_WFX_EXTRA_BYTES   12

//WAVE_FORMAT_MPEGLAYER3 format sructure

typedef struct mpeglayer3waveformat_tag {
	WAVEFORMATEX  wfx;
	unsigned short	wID;
	unsigned short	fdwFlags;
	unsigned short	nBlockSize;
	unsigned short	nFramesPerBlock;
	unsigned short	nCodecDelay;
} MPEGLAYER3WAVEFORMAT;

// Chunk type --------------------------------------------------

#ifndef DWORD
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
#endif
typedef DWORD FOURCC;

#ifndef mmioFOURCC
    #define mmioFOURCC( ch0, ch1, ch2, ch3 ) \
	( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
	( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

typedef struct 
{
	unsigned long	ckid;
	unsigned long	dwFlags;
	unsigned long	dwChunkOffset;		// Position of chunk
	unsigned long	dwChunkLength;		// Length of chunk
} AVIINDEXENTRY;

// AVI (Standard) Data Structure - End >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// Costom Data Structure - Start <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#define MOVI_POS_OFFSET		2048

typedef struct 
{
	// RIFF Header Form
	int  riff_id;				// "RIFF"	ASCII ??
	int  riff_siz;				//?ļ???С壿
	int  riff_type;			// "AVI " 

	// Header List Form
	int  hdlr_id;				// "LIST"
	int  hdlr_siz;
	int  hdlr_name;			// "hdlr"

	int  avih_id;				// "avih"
	int  avih_siz;
	AVIMAINHEADER avih;  

	// Video List Form
	int  strl_vd_id;			// "LIST"
	int  strl_vd_siz;	
	int  strl_vd_name;			// "strl"

	// Video Header
	int  strh_vd_id;			// "strh"
	int  strh_vd_siz;
	AVISTREAMHEADER strh_vd;
		
	// Video Format
	int  strf_vd_id;			// "strf"
	int  strf_vd_siz;
	BITMAPINFOHEADER strf_vd;

	// Audio List Form
	int  strl_au_id;			// "LIST"
	int  strl_au_siz;	
	int  strl_au_name;			// "strl"

	// Audio Header - Optional(!)
	int  strh_au_id;			// "strh"
	int  strh_au_siz;
	AVISTREAMHEADER strh_au;

	// Audio Format 
	int  strf_au_id;			// "strf"
	int  strf_au_siz;
	ADWAVEFORMATEX  strf_au;

} AVIFixHeader;

typedef struct 
{
	int  idx1_id;
	int  idx1_siz;
} AVIIDXHeader;

typedef struct
{
	int  movi_id;
	int  movi_siz;
	int  movi_name;
} AVIMOVHeader;

struct CHUNK_HDR{
	int  chk_id;
	int  chk_siz;
};

struct JUNK_HDR {
	int  junk_id;
	int  junk_siz;
};

typedef struct _INDEX {
	struct _INDEX *next;
	AVIINDEXENTRY data;
} INDEX;

typedef struct {
	INDEX *first;
	INDEX *last;
	int  nr_entry;
} INDEX_ROOT;



struct defAVIVal
{
	int ispal;			//?Ƿ???pal????Ƶ
	int nr_stream;	// 1????ֻ????Ƶ??????ֵ????????Ƶ????	
       char v_avitag[10];   ///<avi??Ƶ??ʽ????"divx","H264","MJPG"
	int v_width;		//ͼ??????
	int v_height;		//ͼ???߶?
	int v_frate;		//ͼ??????
	int v_buffsize;	//???黺??????С
	int a_sampling;	//??????????
	int a_channel;	//????ͨ??
	int a_nr_frame;	//һ???????????м???????
	int a_wformat;	//??????ʽ
	int a_bitrate;		//????????
};

struct avi_header_struct{
	int  				size_AVI_FixHeader;
	struct JUNK_HDR  junk_hdr;
 	AVIFixHeader  	AVI_FixHeader;
 	AVIIDXHeader  	AVI_IDXHeader;
	AVIMOVHeader  	AVI_MOVHeader;	
};
//?û???Ҫ????һ???˽ṹ?ı?��

typedef struct 
{
	// File Save Case
	FILE  *fp;

	// Mem Open Case
	char  *mbuf;
	int    msize;
	int    mpos;
	int    mcnt;

	// Field For 'movi' Chunk..
	unsigned long movcnt, offset;
	int  totalFrame;
	int totalAudioCnt;	
	INDEX_ROOT  idx1List;
	struct avi_header_struct avi_header;

} AVIVarHeader;
/**********************************************************************************************
 * ??????	:set_avi_val()
 * ????	:?????????Ĳ??????õ?avi?ļ?Ҫ?????ṹ??
 * ????	:enc:ָ?????????????ṹ??ָ??
 * ????	:defval:ָ??????????avi?ṹ??ָ??
 * ????ֵ	:0??ʾ?ɹ? ??ֵ??ʾʧ??
 **********************************************************************************************/
int set_avi_val(struct defAVIVal *defval,struct compress_struct *enc);
#define SetAviVal set_avi_val

/**********************************************************************************************
 * ??????	:defAVIFixHeader()
 * ????	:?????????Ĳ??????õ?avi??ͷ?ṹ??
 * ????	:enc:ָ?????????????ṹ??ָ??
 * ????	:header:ָ??????????aviͷ?ṹ??ָ??
 * ????ֵ	:??
 **********************************************************************************************/
void defAVIFixHeader(struct avi_header_struct *header,struct compress_struct *enc);

/**********************************************************************************************
 * ??????	:DefAVIFixHeaderVal()
 * ????	:ʹ??????Ƶý????Ϣ????????aviͷ?ṹ
 * ????	:video:??????Ƶ??Ϣ?Ľṹ
 *			 audio:??????Ƶ??Ϣ?Ľṹ,NULL??ʾ????Ҫ????
 * ????	:header:ָ??????????aviͷ?ṹ??ָ??
 * ????ֵ	:??
 **********************************************************************************************/
void DefAVIFixHeaderVal(struct avi_header_struct *header,video_format_t *video,audio_format_t *audio);
/**********************************************************************************************
 * ??????	:AVIFileOpen()
 * ????	:?Ը??????ļ??�����һ??avi?ļ?
 * ????	:path:Ҫ??????avi?ļ?·?????ļ???
 * ????	:pheader:????һ??avi?ļ??Ľṹָ??
 * ????ֵ	:0??ʾ?ɹ???ֵ??ʾ????
 **********************************************************************************************/
int AVIFileOpen(char *path, AVIVarHeader *pheader);

/**********************************************************************************************
 * ??????	:AVIFileClose()
 * ????	:?ر?һ???Ѿ??򿪵?avi?ļ?
 * ????	:pHeader:????һ???Ѿ??򿪵?avi?ļ??Ľṹָ??
 * ????	:0??ʾ?ɹ?????ֵ??ʾʧ??
 * ????ֵ	:0??ʾ?ɹ???ֵ??ʾ????
 **********************************************************************************************/
int AVIFileClose(AVIVarHeader *pHeader);

/**********************************************************************************************
 * ??????	:AVIFileWrite()
 * ????	:???Ѿ??򿪵?avi?ļ???д??һ֡????
 * ????	:pHeader:????һ???Ѿ??򿪵?avi?ļ??Ľṹָ??
 *			:data:Ҫд??????????ʼ??ַ
 *			:nSize:Ҫд???????ݳ???
 *			:media:????????ȡֵΪ: MEDIA_VIDEO,MEDIA_AUDIO
 *			:DataType:??Ƶ֡????(FRAMETYPE_I,FRAMETYPE_P),????????Ƶ????????
 *					  ???ĸ?ֵ
 * ????	:??
 * ????ֵ	:0??ʾ?ɹ???ֵ??ʾ????
 **********************************************************************************************/
int AVIFileWrite(AVIVarHeader *pHeader, char *data, int nSize, int media,int DataType);

/**********************************************************************************************
 * ??????	:format_avihead_buf()
 * ????	:?? ???????? ?????ṹ????һ??avi?ļ?ͷ??????
 * ????	:enc:??Ƶ???????????ṹָ??
 *			:hb_len:Ŀ?껺?????? ???? 
 * ????	:hb:????????Ŀ?껺????
 * ????ֵ	:0??ʾ?ɹ???ֵ??ʾ????
 **********************************************************************************************/
//int format_avihead_buf(struct compress_struct *enc,unsigned char *hb,int hb_len);


// Costom Data Structure - End >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**********************************************************************************************
 * ??????	:FormatAviHeadBufDef()
 * ????	:avi?ļ?Ҫ?ṹ????һ??avi?ļ?ͷ??????
 * ????	:defval:avi??Ҫ?ṹ ָ??
 *			:hb_len:Ŀ?껺?????? ???? 
 * ????	:hb:????????Ŀ?껺????
 * ????ֵ	:0??ʾ?ɹ???ֵ??ʾ????
 **********************************************************************************************/
int FormatAviHeadBufDef(struct defAVIVal *defval,unsigned char *hb,int hb_len);

#endif

