
/*
 *  Title : AVIEncoder.c
 * 
 *  Note : Formatting Media Stream into AVI Format
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

//#include "rtimage.h"

#include <stdlib.h>
#include "ime6410.h"
#include "ime6410api.h"
#include <string.h>
#include "AVIEncoder.h"
#include <mdebug.h>
#include <unistd.h>
#include <errno.h>
//#include "rtimg_para.h"
// Common(Fixed) Header ===========================================
//static AVIFixHeader  AVI_FixHeader;
//static AVIIDXHeader  AVI_IDXHeader;
//static AVIMOVHeader  AVI_MOVHeader;
//static int  size_AVI_FixHeader;



#define SIZE_AVI_MOVHEADER		12
#define SIZE_AVI_IDXHEADER		8

#define SIZE_JUNKHDR	8


#define SIZE_CHUNK_HDR  8
//static struct CHUNK_HDR  chk_vhdr;
//static struct CHUNK_HDR  chk_ahdr;

#define  VHDR_SCALE				100
#define  AHDR_SAMPLEPERFRAME	1152.	// sample/frame

int get_pic_width(struct compress_struct *enc);
int get_pic_height(struct compress_struct *enc);
//=================================================================
/**********************************************************************************************
 * 函数名	:set_avi_val()
 * 功能	:将编码器的参数设置到avi的简要描述结构中
 * 输入	:enc:指向编码器参数结构的指针
 * 输出	:defval:指向被填充的avi结构的指针
 * 返回值	:0表示成功 负值表示失败
 **********************************************************************************************/
int set_avi_val(struct defAVIVal *defval,struct compress_struct *enc)
{
	static int  sarray[2] = {8000,0};
	int streams,t;
	// Set Default Value To AVI Fix Header
	if(enc==NULL)
		return -1;
	defval->ispal=enc->NTSC_PAL;

	if(enc->AudioFrameType==0)//FIXME
		t=1;
	else
		t=3;
	//t=enc->i64reg.com.EncodingType&3;
	//streams=streams&1+(streams>>1)&1;
	streams=t&1;
	streams+=(t>>1);
	defval->nr_stream =streams;//enc_info_get(enc,SYS_VideoPresent)+enc_info_get(enc,SYS_AudioPresent);
	defval->v_buffsize = 22168;
	defval->v_width = get_pic_width(enc);
	defval->v_height =get_pic_height(enc);
	//printf("width=%d height=%d!!!!\n",get_pic_width(enc),get_pic_height(enc));
	defval->v_frate = (1 << enc->i64reg.com.FrameRate);//enc_info_get(enc,SYS_FrameRateValue);
	defval->a_sampling = sarray[enc->i64reg.audio.Sampling];//enc_info_get(enc,SYS_AudioSampleValue);
	defval->a_nr_frame = NR_AUDIO_PKT;
	defval->a_channel = enc->i64reg.audio.Channel+1;//enc_info_get(enc,SYS_AudioChannel);
	if(enc->AudioFrameType==FRAMETYPE_PCM)	
	//if(enc->i64reg.audio.AudEncMode == 1) 
	{
		defval->a_wformat = WFORMAT_ULAW;
		defval->a_bitrate = 64000;
	}
	else if(enc->AudioFrameType==FRAMETYPE_ADPCM)
	{
		defval->a_wformat = WFORMAT_ADPCM;
		defval->a_bitrate = 32216;
	}
	else if(enc->AudioFrameType==FRAMETYPE_RAWPCM)
	{
		defval->a_wformat = WFORMAT_RAWPCM;
		defval->a_bitrate = 128000;
	}
	return 0;
}
#if 0
//将一个缓冲区格式化成avi格式的文件头
/**********************************************************************************************
 * 函数名	:format_avihead_buf()
 * 功能	:用 编码器的 参数结构填充一个avi文件头缓冲区
 * 输入	:enc:视频编码器参数结构指针
 *			:hb_len:目标缓冲区的 长度 
 * 输出	:hb:被填充的目标缓冲区
 * 返回值	:0表示成功负值表示出错
 **********************************************************************************************/
int format_avihead_buf(struct compress_struct *enc,unsigned char *hb,int hb_len)
{
	AVIMOVHeader *AVI_MOVHeader;
	AVIFixHeader *AVI_FixHeader;	
	struct defAVIVal defval,*pval;
	struct JUNK_HDR junk_hdr;
	int size_AVI_FixHeader;
	if(enc==NULL)
		return -1;
	if(hb==NULL)
		return -1;
	if(hb_len<(MOVI_POS_OFFSET+SIZE_AVI_MOVHEADER))
		return -1;
	if(set_avi_val(&defval,enc)<0)
		return -1;
	pval=&defval;	
	// AVI Fixed Header..
	memset(hb,0,sizeof(AVIFixHeader));
	// Video Header
	AVI_FixHeader=(AVIFixHeader *)hb;
	AVI_FixHeader->strh_vd_id = mmioFOURCC('s', 't', 'r', 'h');
	AVI_FixHeader->strh_vd_siz = sizeof(AVISTREAMHEADER);
	AVI_FixHeader->strh_vd.fccType = mmioFOURCC('v', 'i', 'd', 's');
	AVI_FixHeader->strh_vd.fccHandler = mmioFOURCC('D','I','V','X');
	AVI_FixHeader->strh_vd.dwScale = VHDR_SCALE;
#if 0
	if(pval->ispal)
		AVI_FixHeader->strh_vd.dwRate = (25.0*VHDR_SCALE) / pval->v_frate;
	else
		AVI_FixHeader->strh_vd.dwRate = (29.97*VHDR_SCALE) / pval->v_frate;
#endif
	AVI_FixHeader->strh_vd.dwRate= pval->v_frate*VHDR_SCALE;
	AVI_FixHeader->strh_vd.dwSuggestedBufferSize  = pval->v_buffsize;	
	AVI_FixHeader->strh_vd.rcFrame[0] = 0;
	AVI_FixHeader->strh_vd.rcFrame[1] = 0;
	AVI_FixHeader->strh_vd.rcFrame[2] = pval->v_width;
	AVI_FixHeader->strh_vd.rcFrame[3] = pval->v_height;

	// Video Format
	AVI_FixHeader->strf_vd_id = mmioFOURCC('s', 't', 'r', 'f');
	AVI_FixHeader->strf_vd_siz = sizeof(BITMAPINFOHEADER);
	AVI_FixHeader->strf_vd.biSize = sizeof(BITMAPINFOHEADER);
	AVI_FixHeader->strf_vd.biWidth = pval->v_width;
	AVI_FixHeader->strf_vd.biHeight = pval->v_height;
	AVI_FixHeader->strf_vd.biPlanes = 1;
	AVI_FixHeader->strf_vd.biBitCount = 24;
	memcpy(&AVI_FixHeader->strf_vd.biCompression, "divx", 4);
	AVI_FixHeader->strf_vd.biSizeImage = (pval->v_width)*(pval->v_height)*3;
	AVI_FixHeader->strf_vd.biXPelsPerMeter = 0;
	AVI_FixHeader->strf_vd.biYPelsPerMeter = 0;
	AVI_FixHeader->strf_vd.biClrUsed = 0;
	AVI_FixHeader->strf_vd.biClrImportant = 0;

	// Video List Form
	AVI_FixHeader->strl_vd_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_FixHeader->strl_vd_siz = AVI_FixHeader->strh_vd_siz + AVI_FixHeader->strf_vd_siz + 20;	
	AVI_FixHeader->strl_vd_name = mmioFOURCC('s', 't', 'r', 'l');

	// Audio Header
	AVI_FixHeader->strh_au_id = mmioFOURCC('s', 't', 'r', 'h');
	AVI_FixHeader->strh_au_siz = sizeof(AVISTREAMHEADER);
	AVI_FixHeader->strh_au.fccType = mmioFOURCC('a', 'u', 'd', 's');
	AVI_FixHeader->strh_au.fccHandler = 0;
	if(pval->a_wformat == WFORMAT_ADPCM) {
		AVI_FixHeader->strh_au.dwScale = 508;
		AVI_FixHeader->strh_au.dwRate = 4027;
	//	AVI_FixHeader->strh_au.dwLength = 0;
		AVI_FixHeader->strh_au.dwLength = 114;
		AVI_FixHeader->strh_au.dwSampleSize = 508;	
	}
	else {
		AVI_FixHeader->strh_au.dwScale = 1;
		AVI_FixHeader->strh_au.dwRate = pval->a_sampling;
	}
	AVI_FixHeader->strh_au.dwSuggestedBufferSize = (DBLOCKSIZE-4) *(pval->a_nr_frame);

	// Audio Format
	AVI_FixHeader->strf_au_id = mmioFOURCC('s', 't', 'r', 'f');
	if(pval->a_wformat == WFORMAT_ADPCM) {
		// ADPCM
		// DBLOCKSIZE = 512, HEADER = 4
		// nAudioSamplePerFrame = (DBLOCKSIZE-HEADER)*2+1
		// nAudioSampleRate = a_sampling
		// nAudioBitrate = a_bitrate (mono = 32216, stereo = 64000)
		// nAudioFrameCnt = 1
		// nAudioFrameSize = (nAudioBitrate*nAudioSamplePerFrame/
		//                nAudioSampleRate + 7)/8
		// nBlockAlign = wBitsPerSample * nChannels / 8;
		// nAvgBytesPerSec = a_bitrate / 8;
		// fwHeadLayer = nAudioSampleSize*nAudioSampleRate
		// 				/nAvgBytesPerSec

	//	printf("ADPCM, sizeof(ADWAVEFORMATEX) = %d\n",
		//		sizeof(ADWAVEFORMATEX));
		AVI_FixHeader->strf_au_siz = sizeof(ADWAVEFORMATEX);
		AVI_FixHeader->strf_au.wFormatTag = WFORMAT_ADPCM;
		AVI_FixHeader->strf_au.nChannels = pval->a_channel;
		AVI_FixHeader->strf_au.nSamplesPerSec = pval->a_sampling;
		AVI_FixHeader->strf_au.wBitsPerSample = 4;
		AVI_FixHeader->strf_au.nBlockAlign = 508;
		AVI_FixHeader->strf_au.nAvgBytesPerSec = 4027;
		AVI_FixHeader->strf_au.cbSize = 2;
		AVI_FixHeader->strf_au.wSamplesPerBlock = 1009;
	}
	else {
	//	printf("PCM, sizeof(ADWAVEFORMATEX) = %d\n",
		//		sizeof(ADWAVEFORMATEX));
		// U-LAW PCM
		// nAvgBytesPerSec = nSamplesPerSec * nBlockAlign
		//printf("sizeof(WAVEFORMATEX)=%d sizeof(ADWAVEFORMATEX)=%d\n",sizeof(WAVEFORMATEX),sizeof(ADWAVEFORMATEX));
		AVI_FixHeader->strf_au_siz =sizeof(ADWAVEFORMATEX);// - 2;//changed by shixin 
		AVI_FixHeader->strf_au.wFormatTag = WFORMAT_ULAW;
		AVI_FixHeader->strf_au.nChannels = pval->a_channel;
		AVI_FixHeader->strf_au.nSamplesPerSec = pval->a_sampling;
		AVI_FixHeader->strf_au.wBitsPerSample = 8;
		AVI_FixHeader->strf_au.nBlockAlign = 1;
		AVI_FixHeader->strf_au.nAvgBytesPerSec = AVI_FixHeader->strf_au.nBlockAlign * AVI_FixHeader->strf_au.nSamplesPerSec;
		AVI_FixHeader->strf_au.cbSize = 0;
	}

	// Audio List Form
	AVI_FixHeader->strl_au_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_FixHeader->strl_au_siz = AVI_FixHeader->strh_au_siz + AVI_FixHeader->strf_au_siz + 20;	
	AVI_FixHeader->strl_au_name = mmioFOURCC('s', 't', 'r', 'l');

	// AVI Header
	AVI_FixHeader->avih_id = mmioFOURCC('a', 'v', 'i', 'h');
	AVI_FixHeader->avih_siz = sizeof(AVIMAINHEADER);
//	AVI_FixHeader.avih.dwMicroSecPerFrame = (unsigned long)(1000000./AVI_FixHeader.strh_vd.dwRate*AVI_FixHeader.strh_vd.dwScale + 0.5);
	if(pval->ispal)
		AVI_FixHeader->avih.dwMicroSecPerFrame = (unsigned long)(1000000*(pval->v_frate)/25.0);
	else
		AVI_FixHeader->avih.dwMicroSecPerFrame = (unsigned long)(1000000*(pval->v_frate)/29.97);

	
	AVI_FixHeader->avih.dwFlags = AVIF_HASINDEX | AVIF_ISINTERLEAVED;
	AVI_FixHeader->avih.dwTotalFrames = 0;
	AVI_FixHeader->avih.dwStreams = pval->nr_stream;
	AVI_FixHeader->avih.dwSuggestedBufferSize = pval->v_buffsize;
	AVI_FixHeader->avih.dwWidth = pval->v_width;
	AVI_FixHeader->avih.dwHeight = pval->v_height;

	// Header List Form
	if(pval->nr_stream == 1)
	{
		// Video Only
		size_AVI_FixHeader = sizeof(AVIFixHeader) - 28 
				- sizeof(AVISTREAMHEADER)
				- sizeof(ADWAVEFORMATEX);
		AVI_FixHeader->hdlr_siz = AVI_FixHeader->avih_siz 
				+ AVI_FixHeader->strl_vd_siz + 20;
	}
	else {
		// Video & Audio
		size_AVI_FixHeader = sizeof(AVIFixHeader)
			  - sizeof(ADWAVEFORMATEX)
			  + AVI_FixHeader->strf_au_siz;
		AVI_FixHeader->hdlr_siz = AVI_FixHeader->avih_siz
			+ AVI_FixHeader->strl_vd_siz
			+ AVI_FixHeader->strl_au_siz + 28;
	}

	AVI_FixHeader->hdlr_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_FixHeader->hdlr_name = mmioFOURCC('h', 'd', 'r', 'l');

	// RIFF Header Form
	AVI_FixHeader->riff_id = mmioFOURCC('R', 'I', 'F', 'F');
	AVI_FixHeader->riff_siz = 0;
	AVI_FixHeader->riff_type = mmioFOURCC('A', 'V', 'I', ' ');


	//printf("size_AVI_FixHeader=%d \n",size_AVI_FixHeader);
	// JUNK Header
	//junk_hdr=(struct JUNK_HDR *)&hb[size_AVI_FixHeader];
	junk_hdr.junk_id = mmioFOURCC('J', 'U', 'N', 'K');
	junk_hdr.junk_siz = MOVI_POS_OFFSET - AVI_FixHeader->hdlr_siz - 28;	// Riff Header.. hdrl Header
	memcpy(&hb[size_AVI_FixHeader],(char*)&junk_hdr,8);


	
	// AVI MOV Header  
	AVI_MOVHeader=(AVIMOVHeader*)(hb+MOVI_POS_OFFSET);
	AVI_MOVHeader->movi_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_MOVHeader->movi_siz = 0;
	AVI_MOVHeader->movi_name = mmioFOURCC('m', 'o', 'v', 'i');
	
	//AVI MOV Header 不传送给客户端
	return (MOVI_POS_OFFSET);//+SIZE_AVI_MOVHEADER);
}

#endif
/**********************************************************************************************
 * 函数名	:AVIFileOpen()
 * 功能	:以给定的文件名创建一个avi文件
 * 输入	:path:要创建的avi文件路径及文件名
 * 输出	:pheader:描述一个avi文件的结构指针
 * 返回值	:0表示成功负值表示出错
 **********************************************************************************************/
int AVIFileOpen(char *path, AVIVarHeader *pheader)
{
	AVIFixHeader  *AVI_FixHeader;
	memset(pheader, 0, sizeof(AVIVarHeader)-sizeof(struct avi_header_struct));

	if(!path) 
		return -EINVAL;
	if((pheader->fp = fopen(path, "wb")) == NULL)
	{		
		return -errno;
	}
	AVI_FixHeader=&pheader->avi_header.AVI_FixHeader;
	fwrite((char*)AVI_FixHeader,1,sizeof(AVIFixHeader),pheader->fp);
	fsync(fileno(pheader->fp));
	pheader->movcnt = pheader->offset = 4;
	fseek(pheader->fp, MOVI_POS_OFFSET+SIZE_AVI_MOVHEADER, SEEK_SET);

	return 0;
}

#define DT_AUDIO	2
#define DT_VIDEO_P	0
#define DT_VIDEO_I	1

int AVIFileWrite(AVIVarHeader *pHeader, char *data, int nSize, int media,int DataType)
{
	INDEX *pentry;
	//struct CHUNK_HDR  *pCHdr;
	DWORD	headflag;
	DWORD	chk_size;
	if(pHeader->fp==NULL)
		return -1;
	pentry = (INDEX *) malloc(sizeof(INDEX));
	if(pentry == NULL)
	{
		ERR("malloc.. pentry\n");
		return -3;
	}
	// Write Chunk Header
	if(media == MEDIA_AUDIO) 
		headflag=IDX1_AID;//pCHdr = &chk_ahdr;//声音包
	else 
		headflag=IDX1_VID;//pCHdr = &chk_vhdr;//视频包
	chk_size = nSize;


	fwrite(&headflag, 1, 4, pHeader->fp);
	fwrite(&chk_size, 1, 4, pHeader->fp);//写一个数据块头，8字节，表示声音还是视频，以及数据块长度
	// Write Stream Data
	fwrite(data, 1, nSize, pHeader->fp);
	// Make Index Entry
	pentry->next = NULL;
	if(media == MEDIA_AUDIO) {
		pentry->data.ckid = IDX1_AID;
		pentry->data.dwFlags = 1;
		(pHeader->totalAudioCnt)+=12;
	}
	else {
		pentry->data.ckid = IDX1_VID;
		if(DataType==FRAMETYPE_I)
			pentry->data.dwFlags = 1;
		else
			pentry->data.dwFlags = 0;		
		++(pHeader->totalFrame);
	}
	pentry->data.dwChunkOffset = pHeader->offset;
	pHeader->offset += (nSize + SIZE_CHUNK_HDR);
	pentry->data.dwChunkLength = nSize;

	// Add Entry To Linked List
	if (!pHeader->idx1List.first) 
		pHeader->idx1List.first = pentry;
	else 
		pHeader->idx1List.last->next = pentry;
	pHeader->idx1List.last = pentry;

	// Update MovCnt..
	pHeader->movcnt += (nSize + SIZE_CHUNK_HDR);

	// Increase IDX1 Entry Count..
	++(pHeader->idx1List.nr_entry);

	return (nSize + 8);
}


/**********************************************************************************************
 * 函数名	:AVIFileClose()
 * 功能	:关闭一个已经打开的avi文件
 * 输入	:pHeader:描述一个已经打开的avi文件的结构指针
 * 输出	:0表示成功，负值表示失败
 * 返回值	:0表示成功负值表示出错
 **********************************************************************************************/
int AVIFileClose(AVIVarHeader *pHeader)
{
	AVIFixHeader  *AVI_FixHeader;
	AVIIDXHeader  *AVI_IDXHeader;
	AVIMOVHeader  *AVI_MOVHeader;
	INDEX  *r, *pentry = pHeader->idx1List.first;

	AVI_FixHeader=&pHeader->avi_header.AVI_FixHeader;
	AVI_IDXHeader=&pHeader->avi_header.AVI_IDXHeader;
	AVI_MOVHeader=&pHeader->avi_header.AVI_MOVHeader;
	//文件尾，加入索引信息
	// Index List Header
	AVI_IDXHeader->idx1_siz = pHeader->idx1List.nr_entry * sizeof(AVIINDEXENTRY);
	fwrite((char*)AVI_IDXHeader, 1, SIZE_AVI_IDXHEADER, pHeader->fp);
	// Index List Data
	//文件索引
	while (pentry) {
		fwrite(&pentry->data, 1, sizeof(AVIINDEXENTRY), pHeader->fp);
		r = pentry;
		pentry = pentry->next;
		free(r);
	}

	// Fixed Header
	fseek(pHeader->fp, 0, SEEK_SET);
	AVI_FixHeader->avih.dwTotalFrames = AVI_FixHeader->strh_vd.dwLength =	pHeader->totalFrame;
	AVI_FixHeader->strh_au.dwLength = pHeader->totalAudioCnt;
	AVI_FixHeader->riff_siz = MOVI_POS_OFFSET + pHeader->movcnt + AVI_IDXHeader->idx1_siz + 8;  // 
	fwrite((char*)AVI_FixHeader, 1, pHeader->avi_header.size_AVI_FixHeader, pHeader->fp);

	// Insert JUNK
	fwrite(&pHeader->avi_header.junk_hdr, 1, SIZE_JUNKHDR, pHeader->fp);
	
	// Movi Header
	fseek(pHeader->fp, MOVI_POS_OFFSET, SEEK_SET);
	AVI_MOVHeader->movi_siz = pHeader->movcnt;
	fwrite((char *) AVI_MOVHeader, 1, SIZE_AVI_MOVHEADER, pHeader->fp);

	// Close File
	return fclose(pHeader->fp);

	//return 0;
}



/**********************************************************************************************
 * 函数名	:defAVIFixHeader()
 * 功能	:将编码器的参数设置到avi的头结构中
 * 输入	:enc:指向编码器参数结构的指针
 * 输出	:header:指向被填充的avi头结构的指针
 * 返回值	:无
 **********************************************************************************************/
void defAVIFixHeader(struct avi_header_struct *header,struct compress_struct *enc)
{
	struct defAVIVal defval,*pval;
	AVIFixHeader  *AVI_FixHeader;
	AVIIDXHeader  *AVI_IDXHeader;
	AVIMOVHeader  *AVI_MOVHeader;
	if(header==NULL)
		return;
	if(set_avi_val(&defval,enc)<0)
		return ;
	pval=&defval;		
	AVI_FixHeader=&header->AVI_FixHeader;
	AVI_IDXHeader=&header->AVI_IDXHeader;
	AVI_MOVHeader=&header->AVI_MOVHeader;

	// AVI MOV Header
	AVI_MOVHeader->movi_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_MOVHeader->movi_siz = 0;
	AVI_MOVHeader->movi_name = mmioFOURCC('m', 'o', 'v', 'i');
	//AVI_MOVHeader_Byte = (char *) &AVI_MOVHeader;

	// AVI IDX1 Header
	AVI_IDXHeader->idx1_id = mmioFOURCC('i', 'd', 'x', '1');
	AVI_IDXHeader->idx1_siz = 0;
	//AVI_IDXHeader_Byte = (char *) &AVI_IDXHeader;

	// AVI Fixed Header..
	//AVI_FixHeader_Byte = (char *) &AVI_FixHeader;
	memset((char*)AVI_FixHeader, 0, sizeof(AVIFixHeader));

	// Video Header
	AVI_FixHeader->strh_vd_id = mmioFOURCC('s', 't', 'r', 'h');
	AVI_FixHeader->strh_vd_siz = sizeof(AVISTREAMHEADER);
	AVI_FixHeader->strh_vd.fccType = mmioFOURCC('v', 'i', 'd', 's');
	AVI_FixHeader->strh_vd.fccHandler = mmioFOURCC('D','I','V','X');
	AVI_FixHeader->strh_vd.dwScale = VHDR_SCALE;
#if 0
	if(pval->ispal)
		AVI_FixHeader->strh_vd.dwRate = (25.0*VHDR_SCALE) / pval->v_frate;
	else
		AVI_FixHeader->strh_vd.dwRate = (29.97*VHDR_SCALE) / pval->v_frate;
#endif
	AVI_FixHeader->strh_vd.dwRate=pval->v_frate*VHDR_SCALE;
	AVI_FixHeader->strh_vd.dwSuggestedBufferSize  = pval->v_buffsize;	
	AVI_FixHeader->strh_vd.rcFrame[0] = 0;
	AVI_FixHeader->strh_vd.rcFrame[1] = 0;
	AVI_FixHeader->strh_vd.rcFrame[2] = pval->v_width;
	AVI_FixHeader->strh_vd.rcFrame[3] = pval->v_height;

	// Video Format
	AVI_FixHeader->strf_vd_id = mmioFOURCC('s', 't', 'r', 'f');
	AVI_FixHeader->strf_vd_siz = sizeof(BITMAPINFOHEADER);
	AVI_FixHeader->strf_vd.biSize = sizeof(BITMAPINFOHEADER);
	AVI_FixHeader->strf_vd.biWidth = pval->v_width;
	AVI_FixHeader->strf_vd.biHeight = pval->v_height;
	AVI_FixHeader->strf_vd.biPlanes = 1;
	AVI_FixHeader->strf_vd.biBitCount = 24;
	memcpy(&AVI_FixHeader->strf_vd.biCompression, "divx", 4);
	AVI_FixHeader->strf_vd.biSizeImage = (pval->v_width)*(pval->v_height)*3;
	AVI_FixHeader->strf_vd.biXPelsPerMeter = 0;
	AVI_FixHeader->strf_vd.biYPelsPerMeter = 0;
	AVI_FixHeader->strf_vd.biClrUsed = 0;
	AVI_FixHeader->strf_vd.biClrImportant = 0;

	// Video List Form
	AVI_FixHeader->strl_vd_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_FixHeader->strl_vd_siz = AVI_FixHeader->strh_vd_siz + AVI_FixHeader->strf_vd_siz + 20;	
	AVI_FixHeader->strl_vd_name = mmioFOURCC('s', 't', 'r', 'l');

	// Audio Header
	AVI_FixHeader->strh_au_id = mmioFOURCC('s', 't', 'r', 'h');
	AVI_FixHeader->strh_au_siz = sizeof(AVISTREAMHEADER);
	AVI_FixHeader->strh_au.fccType = mmioFOURCC('a', 'u', 'd', 's');
	AVI_FixHeader->strh_au.fccHandler = 0;
	if(pval->a_wformat == WFORMAT_ADPCM) {
		AVI_FixHeader->strh_au.dwScale = 508;
		AVI_FixHeader->strh_au.dwRate = 4027;
	//	AVI_FixHeader->strh_au.dwLength = 0;
		AVI_FixHeader->strh_au.dwLength = 114;
		AVI_FixHeader->strh_au.dwSampleSize = 508;	
	}
	else {
		AVI_FixHeader->strh_au.dwScale = 1;
		AVI_FixHeader->strh_au.dwRate = pval->a_sampling;
	}
	AVI_FixHeader->strh_au.dwSuggestedBufferSize = (DBLOCKSIZE-4) *(pval->a_nr_frame);

	// Audio Format
	AVI_FixHeader->strf_au_id = mmioFOURCC('s', 't', 'r', 'f');
	if(pval->a_wformat == WFORMAT_ADPCM) {
		// ADPCM
		// DBLOCKSIZE = 512, HEADER = 4
		// nAudioSamplePerFrame = (DBLOCKSIZE-HEADER)*2+1
		// nAudioSampleRate = a_sampling
		// nAudioBitrate = a_bitrate (mono = 32216, stereo = 64000)
		// nAudioFrameCnt = 1
		// nAudioFrameSize = (nAudioBitrate*nAudioSamplePerFrame/
		//                nAudioSampleRate + 7)/8
		// nBlockAlign = wBitsPerSample * nChannels / 8;
		// nAvgBytesPerSec = a_bitrate / 8;
		// fwHeadLayer = nAudioSampleSize*nAudioSampleRate
		// 				/nAvgBytesPerSec
//		printf("ADPCM, sizeof(ADWAVEFORMATEX) = %d\n",
//				sizeof(ADWAVEFORMATEX));
		AVI_FixHeader->strf_au_siz = sizeof(ADWAVEFORMATEX);
		AVI_FixHeader->strf_au.wFormatTag = WFORMAT_ADPCM;
		AVI_FixHeader->strf_au.nChannels = pval->a_channel;
		AVI_FixHeader->strf_au.nSamplesPerSec = pval->a_sampling;
		AVI_FixHeader->strf_au.wBitsPerSample = 4;
		AVI_FixHeader->strf_au.nBlockAlign = 508;
		AVI_FixHeader->strf_au.nAvgBytesPerSec = 4027;
		AVI_FixHeader->strf_au.cbSize = 2;
		AVI_FixHeader->strf_au.wSamplesPerBlock = 1009;
	}
	else {
//		printf("PCM, sizeof(ADWAVEFORMATEX) = %d\n",
//				sizeof(ADWAVEFORMATEX));
		// U-LAW PCM
		// nAvgBytesPerSec = nSamplesPerSec * nBlockAlign
		AVI_FixHeader->strf_au_siz = sizeof(ADWAVEFORMATEX) - 2;
		AVI_FixHeader->strf_au.wFormatTag = WFORMAT_ULAW;
		AVI_FixHeader->strf_au.nChannels = pval->a_channel;
		AVI_FixHeader->strf_au.nSamplesPerSec = pval->a_sampling;
		AVI_FixHeader->strf_au.wBitsPerSample = 8;
		AVI_FixHeader->strf_au.nBlockAlign = 1;
		AVI_FixHeader->strf_au.nAvgBytesPerSec = 
		AVI_FixHeader->strf_au.nBlockAlign *
		AVI_FixHeader->strf_au.nSamplesPerSec;
		AVI_FixHeader->strf_au.cbSize = 0;
	}

	// Audio List Form
	AVI_FixHeader->strl_au_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_FixHeader->strl_au_siz = AVI_FixHeader->strh_au_siz + AVI_FixHeader->strf_au_siz + 20;	
	AVI_FixHeader->strl_au_name = mmioFOURCC('s', 't', 'r', 'l');

	// AVI Header
	AVI_FixHeader->avih_id = mmioFOURCC('a', 'v', 'i', 'h');
	AVI_FixHeader->avih_siz = sizeof(AVIMAINHEADER);
//	AVI_FixHeader->avih.dwMicroSecPerFrame = (unsigned long)(1000000./AVI_FixHeader.strh_vd.dwRate*AVI_FixHeader.strh_vd.dwScale + 0.5);
	if(pval->ispal)
		AVI_FixHeader->avih.dwMicroSecPerFrame = (unsigned long)(1000000*(pval->v_frate)/25.0);
	else
		AVI_FixHeader->avih.dwMicroSecPerFrame = (unsigned long)(1000000*(pval->v_frate)/29.97);

	
	AVI_FixHeader->avih.dwFlags = AVIF_HASINDEX | AVIF_ISINTERLEAVED;
	AVI_FixHeader->avih.dwTotalFrames = 0;
	AVI_FixHeader->avih.dwStreams = pval->nr_stream;
	AVI_FixHeader->avih.dwSuggestedBufferSize = pval->v_buffsize;
	AVI_FixHeader->avih.dwWidth = pval->v_width;
	AVI_FixHeader->avih.dwHeight = pval->v_height;

	// Header List Form
	if(pval->nr_stream == 1)
	{
		// Video Only
		header->size_AVI_FixHeader = sizeof(AVIFixHeader) - 28 
				- sizeof(AVISTREAMHEADER)
				- sizeof(ADWAVEFORMATEX);
		AVI_FixHeader->hdlr_siz = AVI_FixHeader->avih_siz 
				+ AVI_FixHeader->strl_vd_siz + 20;
	}
	else {
		// Video & Audio
		header->size_AVI_FixHeader = sizeof(AVIFixHeader)
			  - sizeof(ADWAVEFORMATEX)
			  + AVI_FixHeader->strf_au_siz;
		AVI_FixHeader->hdlr_siz = AVI_FixHeader->avih_siz
			+ AVI_FixHeader->strl_vd_siz
			+ AVI_FixHeader->strl_au_siz + 28;
	}

	AVI_FixHeader->hdlr_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_FixHeader->hdlr_name = mmioFOURCC('h', 'd', 'r', 'l');

	// RIFF Header Form
	AVI_FixHeader->riff_id = mmioFOURCC('R', 'I', 'F', 'F');
	AVI_FixHeader->riff_siz = 0;
	AVI_FixHeader->riff_type = mmioFOURCC('A', 'V', 'I', ' ');

	// JUNK Header
	header->junk_hdr.junk_id = mmioFOURCC('J', 'U', 'N', 'K');
	header->junk_hdr.junk_siz = MOVI_POS_OFFSET - AVI_FixHeader->hdlr_siz - 28;	// Riff Header.. hdrl Header
}


/**********************************************************************************************
 * 函数名	:DefAVIFixHeaderVal()
 * 功能	:使用音视频媒体信息参数填充avi头结构
 * 输入	:video:描述视频信息的结构
 *			 audio:描述音频信息的结构,NULL表示不需要声音
 * 输出	:header:指向被填充的avi头结构的指针
 * 返回值	:无
 **********************************************************************************************/
void DefAVIFixHeaderVal(struct avi_header_struct *header,video_format_t *video,audio_format_t *audio)
{
	struct defAVIVal avi_v;
	struct defAVIVal *pval=&avi_v;
	AVIFixHeader  *AVI_FixHeader=NULL;
	AVIIDXHeader  *AVI_IDXHeader=NULL;
	AVIMOVHeader  *AVI_MOVHeader=NULL;
	if(video==NULL)
		return;
	memset((void*)pval,0,sizeof(struct defAVIVal));
	pval->ispal=video->ispal;			//是否是pal制视频
	pval->nr_stream=(audio==NULL)?1:2;
	pval->v_width=video->v_width;
	pval->v_height=video->v_height;
	pval->v_frate=video->v_frate;
	pval->v_buffsize=video->v_buffsize;

	if(audio!=NULL)
	{
		pval->a_sampling=audio->a_sampling;
		pval->a_channel=audio->a_channel;
		pval->a_bitrate=audio->a_bitrate;
		pval->a_wformat=audio->a_wformat;
		pval->a_nr_frame=audio->a_nr_frame;
	}
	else
	{
		pval->a_sampling=8000;
		pval->a_channel=0;
		pval->a_bitrate=8000;
		pval->a_wformat=WFORMAT_ULAW;
		pval->a_nr_frame=1;		
	}
	AVI_FixHeader=&header->AVI_FixHeader;
	AVI_IDXHeader=&header->AVI_IDXHeader;
	AVI_MOVHeader=&header->AVI_MOVHeader;

	// AVI MOV Header
	AVI_MOVHeader->movi_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_MOVHeader->movi_siz = 0;
	AVI_MOVHeader->movi_name = mmioFOURCC('m', 'o', 'v', 'i');
	//AVI_MOVHeader_Byte = (char *) &AVI_MOVHeader;

	// AVI IDX1 Header
	AVI_IDXHeader->idx1_id = mmioFOURCC('i', 'd', 'x', '1');
	AVI_IDXHeader->idx1_siz = 0;
	//AVI_IDXHeader_Byte = (char *) &AVI_IDXHeader;

	// AVI Fixed Header..
	//AVI_FixHeader_Byte = (char *) &AVI_FixHeader;
	memset((char*)AVI_FixHeader, 0, sizeof(AVIFixHeader));

	// Video Header
	AVI_FixHeader->strh_vd_id = mmioFOURCC('s', 't', 'r', 'h');
	AVI_FixHeader->strh_vd_siz = sizeof(AVISTREAMHEADER);
	AVI_FixHeader->strh_vd.fccType = mmioFOURCC('v', 'i', 'd', 's');
	AVI_FixHeader->strh_vd.fccHandler = mmioFOURCC('D','I','V','X');
	AVI_FixHeader->strh_vd.dwScale = VHDR_SCALE;
#if 0
	if(pval->ispal)
		AVI_FixHeader->strh_vd.dwRate = (25.0*VHDR_SCALE) / pval->v_frate;
	else
		AVI_FixHeader->strh_vd.dwRate = (29.97*VHDR_SCALE) / pval->v_frate;
#endif
	AVI_FixHeader->strh_vd.dwRate=pval->v_frate*VHDR_SCALE;

	AVI_FixHeader->strh_vd.dwSuggestedBufferSize  = pval->v_buffsize;	
	AVI_FixHeader->strh_vd.rcFrame[0] = 0;
	AVI_FixHeader->strh_vd.rcFrame[1] = 0;
	AVI_FixHeader->strh_vd.rcFrame[2] = pval->v_width;
	AVI_FixHeader->strh_vd.rcFrame[3] = pval->v_height;

	// Video Format
	AVI_FixHeader->strf_vd_id = mmioFOURCC('s', 't', 'r', 'f');
	AVI_FixHeader->strf_vd_siz = sizeof(BITMAPINFOHEADER);
	AVI_FixHeader->strf_vd.biSize = sizeof(BITMAPINFOHEADER);
	AVI_FixHeader->strf_vd.biWidth = pval->v_width;
	AVI_FixHeader->strf_vd.biHeight = pval->v_height;
	AVI_FixHeader->strf_vd.biPlanes = 1;
	AVI_FixHeader->strf_vd.biBitCount = 24;
	memcpy(&AVI_FixHeader->strf_vd.biCompression, "divx", 4);
	AVI_FixHeader->strf_vd.biSizeImage = (pval->v_width)*(pval->v_height)*3;
	AVI_FixHeader->strf_vd.biXPelsPerMeter = 0;
	AVI_FixHeader->strf_vd.biYPelsPerMeter = 0;
	AVI_FixHeader->strf_vd.biClrUsed = 0;
	AVI_FixHeader->strf_vd.biClrImportant = 0;

	// Video List Form
	AVI_FixHeader->strl_vd_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_FixHeader->strl_vd_siz = AVI_FixHeader->strh_vd_siz + AVI_FixHeader->strf_vd_siz + 20;	
	AVI_FixHeader->strl_vd_name = mmioFOURCC('s', 't', 'r', 'l');

	// Audio Header
	AVI_FixHeader->strh_au_id = mmioFOURCC('s', 't', 'r', 'h');
	AVI_FixHeader->strh_au_siz = sizeof(AVISTREAMHEADER);
	AVI_FixHeader->strh_au.fccType = mmioFOURCC('a', 'u', 'd', 's');
	AVI_FixHeader->strh_au.fccHandler = 0;
	if(pval->a_wformat == WFORMAT_ADPCM) {
		AVI_FixHeader->strh_au.dwScale = 508;
		AVI_FixHeader->strh_au.dwRate = 4027;
	//	AVI_FixHeader->strh_au.dwLength = 0;
		AVI_FixHeader->strh_au.dwLength = 114;
		AVI_FixHeader->strh_au.dwSampleSize = 508;	
	}
	else {
		AVI_FixHeader->strh_au.dwScale = 1;
		AVI_FixHeader->strh_au.dwRate = pval->a_sampling;
	}
	AVI_FixHeader->strh_au.dwSuggestedBufferSize = (DBLOCKSIZE-4) *(pval->a_nr_frame);

	// Audio Format
	AVI_FixHeader->strf_au_id = mmioFOURCC('s', 't', 'r', 'f');
	if(pval->a_wformat == WFORMAT_ADPCM) {
		// ADPCM
		// DBLOCKSIZE = 512, HEADER = 4
		// nAudioSamplePerFrame = (DBLOCKSIZE-HEADER)*2+1
		// nAudioSampleRate = a_sampling
		// nAudioBitrate = a_bitrate (mono = 32216, stereo = 64000)
		// nAudioFrameCnt = 1
		// nAudioFrameSize = (nAudioBitrate*nAudioSamplePerFrame/
		//                nAudioSampleRate + 7)/8
		// nBlockAlign = wBitsPerSample * nChannels / 8;
		// nAvgBytesPerSec = a_bitrate / 8;
		// fwHeadLayer = nAudioSampleSize*nAudioSampleRate
		// 				/nAvgBytesPerSec
//		printf("ADPCM, sizeof(ADWAVEFORMATEX) = %d\n",
//				sizeof(ADWAVEFORMATEX));
		AVI_FixHeader->strf_au_siz = sizeof(ADWAVEFORMATEX);
		AVI_FixHeader->strf_au.wFormatTag = WFORMAT_ADPCM;
		AVI_FixHeader->strf_au.nChannels = pval->a_channel;
		AVI_FixHeader->strf_au.nSamplesPerSec = pval->a_sampling;
		AVI_FixHeader->strf_au.wBitsPerSample = 4;
		AVI_FixHeader->strf_au.nBlockAlign = 508;
		AVI_FixHeader->strf_au.nAvgBytesPerSec = 4027;
		AVI_FixHeader->strf_au.cbSize = 2;
		AVI_FixHeader->strf_au.wSamplesPerBlock = 1009;
	}
	else {
//		printf("PCM, sizeof(ADWAVEFORMATEX) = %d\n",
//				sizeof(ADWAVEFORMATEX));
		// U-LAW PCM
		// nAvgBytesPerSec = nSamplesPerSec * nBlockAlign
		AVI_FixHeader->strf_au_siz = sizeof(ADWAVEFORMATEX) - 2;
		AVI_FixHeader->strf_au.wFormatTag = WFORMAT_ULAW;
		AVI_FixHeader->strf_au.nChannels = pval->a_channel;
		AVI_FixHeader->strf_au.nSamplesPerSec = pval->a_sampling;
		AVI_FixHeader->strf_au.wBitsPerSample = 8;
		AVI_FixHeader->strf_au.nBlockAlign = 1;
		AVI_FixHeader->strf_au.nAvgBytesPerSec = 
		AVI_FixHeader->strf_au.nBlockAlign *
		AVI_FixHeader->strf_au.nSamplesPerSec;
		AVI_FixHeader->strf_au.cbSize = 0;
	}

	// Audio List Form
	AVI_FixHeader->strl_au_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_FixHeader->strl_au_siz = AVI_FixHeader->strh_au_siz + AVI_FixHeader->strf_au_siz + 20;	
	AVI_FixHeader->strl_au_name = mmioFOURCC('s', 't', 'r', 'l');

	// AVI Header
	AVI_FixHeader->avih_id = mmioFOURCC('a', 'v', 'i', 'h');
	AVI_FixHeader->avih_siz = sizeof(AVIMAINHEADER);
//	AVI_FixHeader->avih.dwMicroSecPerFrame = (unsigned long)(1000000./AVI_FixHeader.strh_vd.dwRate*AVI_FixHeader.strh_vd.dwScale + 0.5);
	if(pval->ispal)
		AVI_FixHeader->avih.dwMicroSecPerFrame = (unsigned long)(1000000*(pval->v_frate)/25.0);
	else
		AVI_FixHeader->avih.dwMicroSecPerFrame = (unsigned long)(1000000*(pval->v_frate)/29.97);

	
	AVI_FixHeader->avih.dwFlags = AVIF_HASINDEX | AVIF_ISINTERLEAVED;
	AVI_FixHeader->avih.dwTotalFrames = 0;
	AVI_FixHeader->avih.dwStreams = pval->nr_stream;
	AVI_FixHeader->avih.dwSuggestedBufferSize = pval->v_buffsize;
	AVI_FixHeader->avih.dwWidth = pval->v_width;
	AVI_FixHeader->avih.dwHeight = pval->v_height;

	// Header List Form
	if(pval->nr_stream == 1)
	{
		// Video Only
		header->size_AVI_FixHeader = sizeof(AVIFixHeader) - 28 
				- sizeof(AVISTREAMHEADER)
				- sizeof(ADWAVEFORMATEX);
		AVI_FixHeader->hdlr_siz = AVI_FixHeader->avih_siz 
				+ AVI_FixHeader->strl_vd_siz + 20;
	}
	else {
		// Video & Audio
		header->size_AVI_FixHeader = sizeof(AVIFixHeader)
			  - sizeof(ADWAVEFORMATEX)
			  + AVI_FixHeader->strf_au_siz;
		AVI_FixHeader->hdlr_siz = AVI_FixHeader->avih_siz
			+ AVI_FixHeader->strl_vd_siz
			+ AVI_FixHeader->strl_au_siz + 28;
	}

	AVI_FixHeader->hdlr_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_FixHeader->hdlr_name = mmioFOURCC('h', 'd', 'r', 'l');

	// RIFF Header Form
	AVI_FixHeader->riff_id = mmioFOURCC('R', 'I', 'F', 'F');
	AVI_FixHeader->riff_siz = 0;
	AVI_FixHeader->riff_type = mmioFOURCC('A', 'V', 'I', ' ');

	// JUNK Header
	header->junk_hdr.junk_id = mmioFOURCC('J', 'U', 'N', 'K');
	header->junk_hdr.junk_siz = MOVI_POS_OFFSET - AVI_FixHeader->hdlr_siz - 28;	// Riff Header.. hdrl Header
}

//将一个缓冲区格式化成avi格式的文件头
/**********************************************************************************************
 * 函数名	:FormatAviHeadBufDef()
 * 功能	:avi的简要结构填充一个avi文件头缓冲区
 * 输入	:defval:avi简要结构 指针
 *			:hb_len:目标缓冲区的 长度 
 * 输出	:hb:被填充的目标缓冲区
 * 返回值	:0表示成功负值表示出错
 **********************************************************************************************/
int FormatAviHeadBufDef(struct defAVIVal *defval,unsigned char *hb,int hb_len)
{
#define SIZE_AVI_MOVHEADER		12
#define SIZE_AVI_IDXHEADER		8

#define SIZE_JUNKHDR	8


#define SIZE_CHUNK_HDR  8
//static struct CHUNK_HDR  chk_vhdr;
//static struct CHUNK_HDR  chk_ahdr;

#define  VHDR_SCALE				100
#define  AHDR_SAMPLEPERFRAME	1152.	// sample/frame
	AVIMOVHeader *AVI_MOVHeader;
	AVIFixHeader *AVI_FixHeader;	
	struct defAVIVal *pval;
	struct JUNK_HDR junk_hdr;
	int size_AVI_FixHeader;
	if(defval==NULL)
		return -1;
	if(hb==NULL)
		return -1;
	if(hb_len<(MOVI_POS_OFFSET+SIZE_AVI_MOVHEADER))
		return -1;
	pval=defval;	
	// AVI Fixed Header..
	memset(hb,0,sizeof(AVIFixHeader));
	// Video Header
	AVI_FixHeader=(AVIFixHeader *)hb;
	AVI_FixHeader->strh_vd_id = mmioFOURCC('s', 't', 'r', 'h');
	AVI_FixHeader->strh_vd_siz = sizeof(AVISTREAMHEADER);
	AVI_FixHeader->strh_vd.fccType = mmioFOURCC('v', 'i', 'd', 's');
       if(defval->v_avitag[0]<'0')   ///不是字母、数字,可能是旧版本的应用程序
       {        
	        AVI_FixHeader->strh_vd.fccHandler = mmioFOURCC('D','I','V','X');
        }
       else
               AVI_FixHeader->strh_vd.fccHandler = mmioFOURCC(defval->v_avitag[0],defval->v_avitag[1],defval->v_avitag[2],defval->v_avitag[3]); 
	AVI_FixHeader->strh_vd.dwScale = VHDR_SCALE;
#if 0
	if(pval->ispal)
		AVI_FixHeader->strh_vd.dwRate = (25.0*VHDR_SCALE) / pval->v_frate;
	else
		AVI_FixHeader->strh_vd.dwRate = (29.97*VHDR_SCALE) / pval->v_frate;
#endif
	AVI_FixHeader->strh_vd.dwRate=pval->v_frate*VHDR_SCALE;
	AVI_FixHeader->strh_vd.dwSuggestedBufferSize  = pval->v_buffsize;	
	AVI_FixHeader->strh_vd.rcFrame[0] = 0;
	AVI_FixHeader->strh_vd.rcFrame[1] = 0;
	AVI_FixHeader->strh_vd.rcFrame[2] = pval->v_width;
	AVI_FixHeader->strh_vd.rcFrame[3] = pval->v_height;

	// Video Format
	AVI_FixHeader->strf_vd_id = mmioFOURCC('s', 't', 'r', 'f');
	AVI_FixHeader->strf_vd_siz = sizeof(BITMAPINFOHEADER);
	AVI_FixHeader->strf_vd.biSize = sizeof(BITMAPINFOHEADER);
	AVI_FixHeader->strf_vd.biWidth = pval->v_width;
	AVI_FixHeader->strf_vd.biHeight = pval->v_height;
	AVI_FixHeader->strf_vd.biPlanes = 1;
	AVI_FixHeader->strf_vd.biBitCount = 24;

       if(defval->v_avitag[0]<'0')   ///不是字母、数字,可能是旧版本的应用程序
       {        
        	memcpy(&AVI_FixHeader->strf_vd.biCompression, "divx", 4);
        }
        else
        {
             memcpy(&AVI_FixHeader->strf_vd.biCompression, defval->v_avitag, 4);   
        }

    
	AVI_FixHeader->strf_vd.biSizeImage = (pval->v_width)*(pval->v_height)*3;
	AVI_FixHeader->strf_vd.biXPelsPerMeter = 0;
	AVI_FixHeader->strf_vd.biYPelsPerMeter = 0;
	AVI_FixHeader->strf_vd.biClrUsed = 0;
	AVI_FixHeader->strf_vd.biClrImportant = 0;

	// Video List Form
	AVI_FixHeader->strl_vd_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_FixHeader->strl_vd_siz = AVI_FixHeader->strh_vd_siz + AVI_FixHeader->strf_vd_siz + 20;	
	AVI_FixHeader->strl_vd_name = mmioFOURCC('s', 't', 'r', 'l');

	// Audio Header
	AVI_FixHeader->strh_au_id = mmioFOURCC('s', 't', 'r', 'h');
	AVI_FixHeader->strh_au_siz = sizeof(AVISTREAMHEADER);
	AVI_FixHeader->strh_au.fccType = mmioFOURCC('a', 'u', 'd', 's');
	AVI_FixHeader->strh_au.fccHandler = 0;
	if(pval->a_wformat == WFORMAT_ADPCM) {
		AVI_FixHeader->strh_au.dwScale = 508;
		AVI_FixHeader->strh_au.dwRate = 4027;
	//	AVI_FixHeader->strh_au.dwLength = 0;
		AVI_FixHeader->strh_au.dwLength = 114;
		AVI_FixHeader->strh_au.dwSampleSize = 508;	
	}
	else {
		AVI_FixHeader->strh_au.dwScale = 1;
		AVI_FixHeader->strh_au.dwRate = pval->a_sampling;
	}
	AVI_FixHeader->strh_au.dwSuggestedBufferSize = (DBLOCKSIZE-4) *(pval->a_nr_frame);

	// Audio Format
	AVI_FixHeader->strf_au_id = mmioFOURCC('s', 't', 'r', 'f');
	if(pval->a_wformat == WFORMAT_ADPCM) {
		// ADPCM
		// DBLOCKSIZE = 512, HEADER = 4
		// nAudioSamplePerFrame = (DBLOCKSIZE-HEADER)*2+1
		// nAudioSampleRate = a_sampling
		// nAudioBitrate = a_bitrate (mono = 32216, stereo = 64000)
		// nAudioFrameCnt = 1
		// nAudioFrameSize = (nAudioBitrate*nAudioSamplePerFrame/
		//                nAudioSampleRate + 7)/8
		// nBlockAlign = wBitsPerSample * nChannels / 8;
		// nAvgBytesPerSec = a_bitrate / 8;
		// fwHeadLayer = nAudioSampleSize*nAudioSampleRate
		// 				/nAvgBytesPerSec

	//	printf("ADPCM, sizeof(ADWAVEFORMATEX) = %d\n",
		//		sizeof(ADWAVEFORMATEX));
		AVI_FixHeader->strf_au_siz = sizeof(ADWAVEFORMATEX);
		AVI_FixHeader->strf_au.wFormatTag = WFORMAT_ADPCM;
		AVI_FixHeader->strf_au.nChannels = pval->a_channel;
		AVI_FixHeader->strf_au.nSamplesPerSec = pval->a_sampling;
		AVI_FixHeader->strf_au.wBitsPerSample = 4;
		AVI_FixHeader->strf_au.nBlockAlign = 508;
		AVI_FixHeader->strf_au.nAvgBytesPerSec = 4027;
		AVI_FixHeader->strf_au.cbSize = 2;
		AVI_FixHeader->strf_au.wSamplesPerBlock = 1009;
	}
	else {
	//	printf("PCM, sizeof(ADWAVEFORMATEX) = %d\n",
		//		sizeof(ADWAVEFORMATEX));
		// U-LAW PCM
		// nAvgBytesPerSec = nSamplesPerSec * nBlockAlign
		//printf("sizeof(WAVEFORMATEX)=%d sizeof(ADWAVEFORMATEX)=%d\n",sizeof(WAVEFORMATEX),sizeof(ADWAVEFORMATEX));
		AVI_FixHeader->strf_au_siz =sizeof(ADWAVEFORMATEX);// - 2;//changed by shixin 
		//AVI_FixHeader->strf_au.wFormatTag = WFORMAT_ULAW;
		AVI_FixHeader->strf_au.wFormatTag = pval->a_wformat;
		
		AVI_FixHeader->strf_au.nChannels = pval->a_channel;
		AVI_FixHeader->strf_au.nSamplesPerSec = pval->a_sampling;
		//AVI_FixHeader->strf_au.wBitsPerSample = 16;
		AVI_FixHeader->strf_au.wBitsPerSample = pval->a_bitrate;
		AVI_FixHeader->strf_au.nBlockAlign = 1;
		AVI_FixHeader->strf_au.nAvgBytesPerSec = AVI_FixHeader->strf_au.nBlockAlign * AVI_FixHeader->strf_au.nSamplesPerSec;
		AVI_FixHeader->strf_au.cbSize = 0;
	}

	// Audio List Form
	AVI_FixHeader->strl_au_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_FixHeader->strl_au_siz = AVI_FixHeader->strh_au_siz + AVI_FixHeader->strf_au_siz + 20;	
	AVI_FixHeader->strl_au_name = mmioFOURCC('s', 't', 'r', 'l');

	// AVI Header
	AVI_FixHeader->avih_id = mmioFOURCC('a', 'v', 'i', 'h');
	AVI_FixHeader->avih_siz = sizeof(AVIMAINHEADER);

	AVI_FixHeader->avih.dwMicroSecPerFrame = (unsigned long)(1000000./AVI_FixHeader->strh_vd.dwRate*AVI_FixHeader->strh_vd.dwScale + 0.5);

#if 0
	if(pval->ispal)
		AVI_FixHeader->avih.dwMicroSecPerFrame = (unsigned long)(1000000*(pval->v_frate)/25.0);
	else
		AVI_FixHeader->avih.dwMicroSecPerFrame = (unsigned long)(1000000*(pval->v_frate)/29.97);
#endif
	
	AVI_FixHeader->avih.dwFlags = AVIF_HASINDEX | AVIF_ISINTERLEAVED;
	AVI_FixHeader->avih.dwTotalFrames = 0;
	AVI_FixHeader->avih.dwStreams = pval->nr_stream;
	AVI_FixHeader->avih.dwSuggestedBufferSize = pval->v_buffsize;
	AVI_FixHeader->avih.dwWidth = pval->v_width;
	AVI_FixHeader->avih.dwHeight = pval->v_height;

	// Header List Form
	if(pval->nr_stream == 1)
	{
		// Video Only
		size_AVI_FixHeader = sizeof(AVIFixHeader) - 28 
				- sizeof(AVISTREAMHEADER)
				- sizeof(ADWAVEFORMATEX);
		AVI_FixHeader->hdlr_siz = AVI_FixHeader->avih_siz 
				+ AVI_FixHeader->strl_vd_siz + 20;
	}
	else {
		// Video & Audio
		size_AVI_FixHeader = sizeof(AVIFixHeader)
			  - sizeof(ADWAVEFORMATEX)
			  + AVI_FixHeader->strf_au_siz;
		AVI_FixHeader->hdlr_siz = AVI_FixHeader->avih_siz
			+ AVI_FixHeader->strl_vd_siz
			+ AVI_FixHeader->strl_au_siz + 28;
	}

	AVI_FixHeader->hdlr_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_FixHeader->hdlr_name = mmioFOURCC('h', 'd', 'r', 'l');

	// RIFF Header Form
	AVI_FixHeader->riff_id = mmioFOURCC('R', 'I', 'F', 'F');
	AVI_FixHeader->riff_siz = 0;
	AVI_FixHeader->riff_type = mmioFOURCC('A', 'V', 'I', ' ');


	//printf("size_AVI_FixHeader=%d \n",size_AVI_FixHeader);
	// JUNK Header
	//junk_hdr=(struct JUNK_HDR *)&hb[size_AVI_FixHeader];
	junk_hdr.junk_id = mmioFOURCC('J', 'U', 'N', 'K');
	junk_hdr.junk_siz = MOVI_POS_OFFSET - AVI_FixHeader->hdlr_siz - 28;	// Riff Header.. hdrl Header
	memcpy(&hb[size_AVI_FixHeader],(char*)&junk_hdr,8);


	
	// AVI MOV Header  
	AVI_MOVHeader=(AVIMOVHeader*)(hb+MOVI_POS_OFFSET);
	AVI_MOVHeader->movi_id = mmioFOURCC('L', 'I', 'S', 'T');
	AVI_MOVHeader->movi_siz = 0;
	AVI_MOVHeader->movi_name = mmioFOURCC('m', 'o', 'v', 'i');
	
	//AVI MOV Header 不传送给客户端
	return (MOVI_POS_OFFSET);//+SIZE_AVI_MOVHEADER);
}

