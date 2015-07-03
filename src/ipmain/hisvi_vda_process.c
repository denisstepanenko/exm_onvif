#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"
#include "hisvi_vda_process.h"
#include "exdrv_3520Ademo/tw2865/tw2865.h"

typedef struct hiVDA_OD_PARAM_S
{
    HI_BOOL bThreadStart;
    VDA_CHN VdaChn;
}VDA_OD_PARAM_S;
typedef struct hiVDA_MD_PARAM_S
{
    HI_BOOL bThreadStart;
    VDA_CHN VdaChn;
}VDA_MD_PARAM_S;

//#define SAMPLE_VDA_MD_CHN 0
//#define SAMPLE_VDA_OD_CHN 1

#define MAX_VDA_MD_CHN_NUM  8
#define MAX_VDA_OD_CHN_NUM  8
#define MAX_VDA_CHN_NUM     16

static pthread_t gs_VdaPid[MAX_VDA_CHN_NUM];
static VDA_MD_PARAM_S gs_stMdParam[MAX_VDA_MD_CHN_NUM];
static VDA_OD_PARAM_S gs_stOdParam[MAX_VDA_OD_CHN_NUM];

VI_DEV_ATTR_S DEV_ATTR_BT656D1_4MUX =
{
    /*接口模式*/
    VI_MODE_BT656,
    /*1、2、4路工作模式*/
    VI_WORK_MODE_4Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0x0},
    /*逐行or隔行输入*/
    VI_SCAN_INTERLACED,
    /*AdChnId*/
    {-1, -1, -1, -1}
};

VI_DEV_ATTR_S DEV_ATTR_BT656D1_2MUX =
{
    /*接口模式*/
    VI_MODE_BT656,
    /*1、2、4路工作模式*/
    VI_WORK_MODE_2Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0x0},
    /*逐行or隔行输入*/
    VI_SCAN_INTERLACED,
    /*AdChnId*/
    {-1, -1, -1, -1}
};

VI_DEV_ATTR_S DEV_ATTR_BT656D1_1MUX =
{
    /*接口模式*/
    VI_MODE_BT656,
    /*1、2、4路工作模式*/
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0x0},
    /*逐行or隔行输入*/
    VI_SCAN_INTERLACED,
    /*AdChnId*/
    {-1, -1, -1, -1}
};


VI_DEV_ATTR_S DEV_ATTR_7441_BT1120_1080P =
/* 典型时序3:7441 BT1120 1080P@60fps典型时序 (对接时序: 时序)*/
{
    /*接口模式*/
    VI_MODE_BT1120_STANDARD,
    /*1、2、4路工作模式*/
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0xFF0000},
    /*逐行or隔行输入*/
    VI_SCAN_PROGRESSIVE,
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, 仅支持YUV格式*/
    VI_INPUT_DATA_UVUV,
     
    /*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_NORM_PULSE,VI_VSYNC_VALID_NEG_HIGH,
    
    /*timing信息，对应reg手册的如下配置*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1920,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            1080,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/ 
     0,            0,            0}
    }
};


VI_DEV_ATTR_S DEV_ATTR_7441_BT1120_720P =
/* 典型时序3:7441 BT1120 720P@60fps典型时序 (对接时序: 时序)*/
{
    /*接口模式*/
    VI_MODE_BT1120_STANDARD,
    /*1、2、4路工作模式*/
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF00,    0xFF},
    /*逐行or隔行输入*/
    VI_SCAN_PROGRESSIVE,
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, 仅支持YUV格式*/
    VI_INPUT_DATA_UVUV,
     
    /*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_NORM_PULSE,VI_VSYNC_VALID_NEG_HIGH,
    
    /*timing信息，对应reg手册的如下配置*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/ 
     0,            0,            0}
    }
};

VI_DEV_ATTR_S DEV_ATTR_7441_INTERLEAVED_720P =
/* 典型时序3:7441 BT1120 720P@60fps典型时序 (对接时序: 时序)*/
{
    /*接口模式*/
    VI_MODE_BT1120_INTERLEAVED,
    /*1、2、4路工作模式*/
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0x0},
    /*逐行or隔行输入*/
    VI_SCAN_PROGRESSIVE,
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, 仅支持YUV格式*/
    VI_INPUT_DATA_UVUV,
     
    /*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_NORM_PULSE,VI_VSYNC_VALID_NEG_HIGH,
    
    /*timing信息，对应reg手册的如下配置*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/ 
     0,            0,            0}
    }
};

/********************SAMPLE_COMMON_SYS****************************************/

/******************************************************************************
* function : calculate VB Block size of picture.
******************************************************************************/
HI_U32 SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, PIXEL_FORMAT_E enPixFmt, HI_U32 u32AlignWidth)
{
    HI_S32 s32Ret = HI_FAILURE;
    SIZE_S stSize;

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get picture size[%d] failed!\n", enPicSize);
            return HI_FAILURE;
    }

    if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 != enPixFmt && PIXEL_FORMAT_YUV_SEMIPLANAR_420 != enPixFmt)
    {
        SAMPLE_PRT("pixel format[%d] input failed!\n", enPixFmt);
            return HI_FAILURE;
    }

    if (16!=u32AlignWidth && 32!=u32AlignWidth && 64!=u32AlignWidth)
    {
        SAMPLE_PRT("system align width[%d] input failed!\n",\
               u32AlignWidth);
            return HI_FAILURE;
    }
    if (704 == stSize.u32Width)
    {
        stSize.u32Width = 720;
    }
    
    SAMPLE_PRT("w:%d, u32AlignWidth:%d\n", CEILING_2_POWER(stSize.u32Width,u32AlignWidth), u32AlignWidth);
    return (CEILING_2_POWER(stSize.u32Width, u32AlignWidth) * \
            CEILING_2_POWER(stSize.u32Height,u32AlignWidth) * \
           ((PIXEL_FORMAT_YUV_SEMIPLANAR_422 == enPixFmt)?2:1.5));
}

/******************************************************************************
* function : vb init & MPI system init
******************************************************************************/
HI_S32 SAMPLE_COMM_SYS_Init(VB_CONF_S *pstVbConf,HI_BOOL bneedExit)
{
    MPP_SYS_CONF_S stSysConf = {0};
    HI_S32 s32Ret = HI_FAILURE;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    if (NULL == pstVbConf)
    {
        SAMPLE_PRT("input parameter is null, it is invaild!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VB_SetConf(pstVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VB_SetConf failed!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VB_Init failed!\n");
        return HI_FAILURE;
    }

    stSysConf.u32AlignWidth = SAMPLE_SYS_ALIGN_WIDTH;
    s32Ret = HI_MPI_SYS_SetConf(&stSysConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_SetConf failed\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Init failed!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/******************************************************************************
* function : get picture size(w*h), according Norm and enPicSize
******************************************************************************/
HI_S32 SAMPLE_COMM_SYS_GetPicSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, SIZE_S *pstSize)
{
    switch (enPicSize)
    {
        case PIC_QCIF:
            pstSize->u32Width = D1_WIDTH / 4;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?144:120;
            break;
        case PIC_CIF:
            pstSize->u32Width = D1_WIDTH / 2;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?288:240;
            break;
        case PIC_D1:
            pstSize->u32Width = D1_WIDTH;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case PIC_960H:
            pstSize->u32Width = 960;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;			
        case PIC_2CIF:
            pstSize->u32Width = D1_WIDTH / 2;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case PIC_QVGA:    /* 320 * 240 */
            pstSize->u32Width = 320;
            pstSize->u32Height = 240;
            break;
        case PIC_VGA:     /* 640 * 480 */
            pstSize->u32Width = 640;
            pstSize->u32Height = 480;
            break;
        case PIC_XGA:     /* 1024 * 768 */
            pstSize->u32Width = 1024;
            pstSize->u32Height = 768;
            break;
        case PIC_SXGA:    /* 1400 * 1050 */
            pstSize->u32Width = 1400;
            pstSize->u32Height = 1050;
            break;
        case PIC_UXGA:    /* 1600 * 1200 */
            pstSize->u32Width = 1600;
            pstSize->u32Height = 1200;
            break;
        case PIC_QXGA:    /* 2048 * 1536 */
            pstSize->u32Width = 2048;
            pstSize->u32Height = 1536;
            break;
        case PIC_WVGA:    /* 854 * 480 */
            pstSize->u32Width = 854;
            pstSize->u32Height = 480;
            break;
        case PIC_WSXGA:   /* 1680 * 1050 */
            pstSize->u32Width = 1680;
            pstSize->u32Height = 1050;
            break;
        case PIC_WUXGA:   /* 1920 * 1200 */
            pstSize->u32Width = 1920;
            pstSize->u32Height = 1200;
            break;
        case PIC_WQXGA:   /* 2560 * 1600 */
            pstSize->u32Width = 2560;
            pstSize->u32Height = 1600;
            break;
        case PIC_HD720:   /* 1280 * 720 */
            pstSize->u32Width = 1280;
            pstSize->u32Height = 720;
            break;
        case PIC_HD1080:  /* 1920 * 1080 */
            pstSize->u32Width = 1920;
            pstSize->u32Height = 1080;
            break;
        default:
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}




/*******************SAMPLE_COMM_VDA********************************************/

/******************************************************************************
* funciton : vda MD mode print -- Md OBJ
******************************************************************************/
HI_S32 SAMPLE_COMM_VDA_MdPrtObj(FILE *fp, VDA_DATA_S *pstVdaData)
{
    VDA_OBJ_S *pstVdaObj;
    HI_S32 i;
	
    fprintf(fp, "===== %s =====\n", __FUNCTION__);
    
    if (HI_TRUE != pstVdaData->unData.stMdData.bObjValid)
    {
        fprintf(fp, "bMbObjValid = FALSE.\n");
        return HI_SUCCESS;
    }
	
    fprintf(fp, "ObjNum=%d, IndexOfMaxObj=%d, SizeOfMaxObj=%d, SizeOfTotalObj=%d\n", \
                   pstVdaData->unData.stMdData.stObjData.u32ObjNum, \
		     pstVdaData->unData.stMdData.stObjData.u32IndexOfMaxObj, \
		     pstVdaData->unData.stMdData.stObjData.u32SizeOfMaxObj,\
		     pstVdaData->unData.stMdData.stObjData.u32SizeOfTotalObj);
    for (i=0; i<pstVdaData->unData.stMdData.stObjData.u32ObjNum; i++)
    {
        pstVdaObj = pstVdaData->unData.stMdData.stObjData.pstAddr + i;
        fprintf(fp, "[%d]\t left=%d, top=%d, right=%d, bottom=%d\n", i, \
			  pstVdaObj->u16Left, pstVdaObj->u16Top, \
			  pstVdaObj->u16Right, pstVdaObj->u16Bottom);
    }
    fflush(fp);
    return HI_SUCCESS;
}
/******************************************************************************
* funciton : vda MD mode print -- Alarm Pixel Count
******************************************************************************/
HI_S32 SAMPLE_COMM_VDA_MdPrtAp(FILE *fp, VDA_DATA_S *pstVdaData)
{
    fprintf(fp, "===== %s =====\n", __FUNCTION__);
    
    if (HI_TRUE != pstVdaData->unData.stMdData.bPelsNumValid)
    {
        fprintf(fp, "bMbObjValid = FALSE.\n");
        return HI_SUCCESS;
    }
	
    fprintf(fp, "AlarmPixelCount=%d\n", pstVdaData->unData.stMdData.u32AlarmPixCnt);
    fflush(fp);
    return HI_SUCCESS;
}

/******************************************************************************
* funciton : vda MD mode print -- SAD
******************************************************************************/
HI_S32 SAMPLE_COMM_VDA_MdPrtSad(FILE *fp, VDA_DATA_S *pstVdaData)
{
    HI_S32 i, j;
    HI_VOID *pAddr;
	
    fprintf(fp, "===== %s =====\n", __FUNCTION__);
    if (HI_TRUE != pstVdaData->unData.stMdData.bMbSadValid)
    {
        fprintf(fp, "bMbSadValid = FALSE.\n");
        return HI_SUCCESS;
    }

    for(i=0; i<pstVdaData->u32MbHeight; i++)
    {
		pAddr = (HI_VOID *)((HI_U32)pstVdaData->unData.stMdData.stMbSadData.pAddr
		  			+ i * pstVdaData->unData.stMdData.stMbSadData.u32Stride);
	
		for(j=0; j<pstVdaData->u32MbWidth; j++)
		{
	        HI_U8  *pu8Addr;
	        HI_U16 *pu16Addr;
		
	        if(VDA_MB_SAD_8BIT == pstVdaData->unData.stMdData.stMbSadData.enMbSadBits)
	        {
	            pu8Addr = (HI_U8 *)pAddr + j;

	            fprintf(fp, "%d ",*pu8Addr);

	        }
	        else
	        {
	            pu16Addr = (HI_U16 *)pAddr + j;

				fprintf(fp, "%d ",*pu16Addr);
	        }
		}
		
        printf("\n");
    }
	
    fflush(fp);
    return HI_SUCCESS;
}
/******************************************************************************
* funciton : vda MD mode thread process
******************************************************************************/
HI_VOID *SAMPLE_COMM_VDA_MdGetResult(HI_VOID *pdata)
{
    HI_S32 s32Ret;
    VDA_CHN VdaChn;
    VDA_DATA_S stVdaData;
    VDA_MD_PARAM_S *pgs_stMdParam;
    HI_S32 maxfd = 0;
    FILE *fp = stdout;
    HI_S32 VdaFd;
    fd_set read_fds;
    struct timeval TimeoutVal;

    pgs_stMdParam = (VDA_MD_PARAM_S *)pdata;

    VdaChn   = pgs_stMdParam->VdaChn;

    /* decide the stream file name, and open file to save stream */
    /* Set Venc Fd. */
    VdaFd = HI_MPI_VDA_GetFd(VdaChn);
    if (VdaFd < 0)
    {
        SAMPLE_PRT("HI_MPI_VDA_GetFd failed with %#x!\n", 
               VdaFd);
        return NULL;
    }
    if (maxfd <= VdaFd)
    {
        maxfd = VdaFd;
    }
    while (HI_TRUE == pgs_stMdParam->bThreadStart)
    {
        FD_ZERO(&read_fds);
        FD_SET(VdaFd, &read_fds);

        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            SAMPLE_PRT("select failed!\n");
            break;
        }
        else if (s32Ret == 0)
        {
            SAMPLE_PRT("get venc stream time out, exit thread\n");
            break;
        }
        else
        {
            if (FD_ISSET(VdaFd, &read_fds))
            {
                /*******************************************************
                   step 2.3 : call mpi to get one-frame stream
                   *******************************************************/
                s32Ret = HI_MPI_VDA_GetData(VdaChn, &stVdaData, HI_TRUE);
                if(s32Ret != HI_SUCCESS)
                {
                    SAMPLE_PRT("HI_MPI_VDA_GetData failed with %#x!\n", s32Ret);
                    return NULL;
                }
                /*******************************************************
                   *step 2.4 : save frame to file
                   *******************************************************/
		        SAMPLE_COMM_VDA_MdPrtSad(fp, &stVdaData);
		        SAMPLE_COMM_VDA_MdPrtObj(fp, &stVdaData);
                SAMPLE_COMM_VDA_MdPrtAp(fp, &stVdaData);
                /*******************************************************
                   *step 2.5 : release stream
                   *******************************************************/
                s32Ret = HI_MPI_VDA_ReleaseData(VdaChn,&stVdaData);
                if(s32Ret != HI_SUCCESS)
	            {
	                SAMPLE_PRT("HI_MPI_VDA_ReleaseData failed with %#x!\n", s32Ret);
	                return NULL;
	            }
            }
        }
    }

    return HI_NULL;
}

/******************************************************************************
* funciton : vda OD mode thread process
******************************************************************************/
HI_S32 SAMPLE_COMM_VDA_OdPrt(FILE *fp, VDA_DATA_S *pstVdaData)
{
    HI_S32 i;

    fprintf(fp, "===== %s =====\n", __FUNCTION__);
    fprintf(fp, "OD region total count =%d\n", pstVdaData->unData.stOdData.u32RgnNum);
    for(i=0; i<pstVdaData->unData.stOdData.u32RgnNum; i++)
    {
        fprintf(fp, "OD region[%d]: %d\n", i, pstVdaData->unData.stOdData.abRgnAlarm[i]);
    }
    fflush(fp);
    return HI_SUCCESS;
}

/******************************************************************************
* funciton : vda OD mode thread process
******************************************************************************/
HI_VOID *SAMPLE_COMM_VDA_OdGetResult(HI_VOID *pdata)
{
    HI_S32 i;
    HI_S32 s32Ret;
    VDA_CHN VdaChn;
    VDA_DATA_S stVdaData;
    HI_U32 u32RgnNum;
    VDA_OD_PARAM_S *pgs_stOdParam;
    //FILE *fp=stdout;

    pgs_stOdParam = (VDA_OD_PARAM_S *)pdata;

    VdaChn    = pgs_stOdParam->VdaChn;
    

    while(HI_TRUE == pgs_stOdParam->bThreadStart)
    {
        s32Ret = HI_MPI_VDA_GetData(VdaChn,&stVdaData,HI_TRUE);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VDA_GetData failed with %#x!\n", s32Ret);
            return NULL;
        }

	    //SAMPLE_COMM_VDA_OdPrt(fp, &stVdaData);

		u32RgnNum = stVdaData.unData.stOdData.u32RgnNum;
		
        for(i=0; i<u32RgnNum; i++)
        {
            if(HI_TRUE == stVdaData.unData.stOdData.abRgnAlarm[i])
            { 
                printf("################VdaChn--%d,Rgn--%d,Occ!\n",VdaChn,i);
                s32Ret = HI_MPI_VDA_ResetOdRegion(VdaChn,i);
                if(s32Ret != HI_SUCCESS)
                {
		            SAMPLE_PRT("HI_MPI_VDA_ResetOdRegion failed with %#x!\n", s32Ret);
                    return NULL;
                }
            }
        }

        s32Ret = HI_MPI_VDA_ReleaseData(VdaChn,&stVdaData);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VDA_ReleaseData failed with %#x!\n", s32Ret);
            return NULL;
        }

        usleep(200*1000);
    }

    return HI_NULL;
}

HI_S32 SAMPLE_COMM_SET_VDA_MDPara(VDA_CHN VdaChn,int SadThreshold)
{
	HI_S32 s32Ret = HI_SUCCESS;

    VDA_CHN_ATTR_S stVdaChnAttr;	
	memset((void*)&stVdaChnAttr,0,sizeof(VDA_CHN_ATTR_S));

	s32Ret = HI_MPI_VDA_GetChnAttr(VdaChn,&stVdaChnAttr);
	if(s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("HI_MPI_VDA_GetChnAttr Md %d failed!\n",VdaChn);
		return s32Ret;
	}

	stVdaChnAttr.unAttr.stMdAttr.u32SadTh      = SadThreshold;

	s32Ret = HI_MPI_VDA_SetChnAttr(VdaChn,&stVdaChnAttr);
	if(s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("HI_MPI_VDA_SetChnAttr Md %d failed!\n",VdaChn);
		return s32Ret;
	}

	return s32Ret;
	
}

HI_S32 SAMPLE_COMM_SET_VDA_ODPara(VDA_CHN VdaChn,int SadThreshold,int OccCntTh,int UnOccCntTh)
{
	HI_S32 s32Ret = HI_SUCCESS;

    VDA_CHN_ATTR_S stVdaChnAttr;	
	memset((void*)&stVdaChnAttr,0,sizeof(VDA_CHN_ATTR_S));

	s32Ret = HI_MPI_VDA_GetChnAttr(VdaChn,&stVdaChnAttr);
	if(s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("HI_MPI_VDA_GetChnAttr Od %d failed!\n",VdaChn);
		return s32Ret;
	}

	if(SadThreshold > 0)
		stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].u32SadTh      = SadThreshold;
	if(OccCntTh > 0)
	    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].u32OccCntTh   = OccCntTh;
	if(UnOccCntTh > 0)
		stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].u32UnOccCntTh = UnOccCntTh;

	s32Ret = HI_MPI_VDA_SetChnAttr(VdaChn,&stVdaChnAttr);
	if(s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("HI_MPI_VDA_SetChnAttr Od %d failed!\n",VdaChn);
		return s32Ret;
	}

	return s32Ret;
}

/******************************************************************************
* funciton : start vda MD mode
******************************************************************************/
HI_S32 SAMPLE_COMM_VDA_MdInit(VDA_CHN VdaChn, VI_CHN ViChn, SIZE_S *pstSize)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VDA_CHN_ATTR_S stVdaChnAttr;
    MPP_CHN_S stSrcChn, stDestChn;
	
    if (VDA_MAX_WIDTH < pstSize->u32Width || VDA_MAX_HEIGHT < pstSize->u32Height)
    {
        SAMPLE_PRT("Picture size invaild!\n");
        return HI_FAILURE;
    }
	
    /* step 1 create vda channel */
    stVdaChnAttr.enWorkMode = VDA_WORK_MODE_MD;
    stVdaChnAttr.u32Width   = pstSize->u32Width;
    stVdaChnAttr.u32Height  = pstSize->u32Height;

    stVdaChnAttr.unAttr.stMdAttr.enVdaAlg      = VDA_ALG_REF;
    stVdaChnAttr.unAttr.stMdAttr.enMbSize      = VDA_MB_16PIXEL;
    stVdaChnAttr.unAttr.stMdAttr.enMbSadBits   = VDA_MB_SAD_8BIT;
    stVdaChnAttr.unAttr.stMdAttr.enRefMode     = VDA_REF_MODE_DYNAMIC;
    stVdaChnAttr.unAttr.stMdAttr.u32MdBufNum   = 8;
    stVdaChnAttr.unAttr.stMdAttr.u32VdaIntvl   = 4;  
    stVdaChnAttr.unAttr.stMdAttr.u32BgUpSrcWgt = 128;
    stVdaChnAttr.unAttr.stMdAttr.u32SadTh      = 100;
    stVdaChnAttr.unAttr.stMdAttr.u32ObjNumMax  = 128;

    s32Ret = HI_MPI_VDA_CreateChn(VdaChn, &stVdaChnAttr);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err!\n");
        return s32Ret;
    }

    /* step 2: vda chn bind vi chn */
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32ChnId = ViChn;
    stSrcChn.s32DevId = 0;

    stDestChn.enModId = HI_ID_VDA;
    stDestChn.s32ChnId = VdaChn;
    stDestChn.s32DevId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err!\n");
        return s32Ret;
    }

	//lc do start later
	/* step 3: vda chn start recv picture */
	/*
    s32Ret = HI_MPI_VDA_StartRecvPic(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err!\n");
        return s32Ret;
    }
    */
    /* step 4: create thread to get result */
	/*
	gs_stMdParam.bThreadStart = HI_TRUE;
    gs_stMdParam.VdaChn   = VdaChn;

    pthread_create(&gs_VdaPid[SAMPLE_VDA_MD_CHN],0, SAMPLE_COMM_VDA_MdGetResult, (HI_VOID *)&gs_stMdParam);
	*/
    return HI_SUCCESS;
}
/******************************************************************************
* funciton : start vda OD mode
******************************************************************************/
HI_S32 SAMPLE_COMM_VDA_OdInit(VDA_CHN VdaChn, VI_CHN ViChn, SIZE_S *pstSize)
{
    VDA_CHN_ATTR_S stVdaChnAttr;
    MPP_CHN_S stSrcChn, stDestChn;
    HI_S32 s32Ret = HI_SUCCESS;
    
    if (VDA_MAX_WIDTH < pstSize->u32Width || VDA_MAX_HEIGHT < pstSize->u32Height)
    {
        SAMPLE_PRT("Picture size invaild!\n");
        return HI_FAILURE;
    }
    
    /********************************************
     step 1 : create vda channel
    ********************************************/
    stVdaChnAttr.enWorkMode = VDA_WORK_MODE_OD;
    stVdaChnAttr.u32Width   = pstSize->u32Width;
    stVdaChnAttr.u32Height  = pstSize->u32Height;

    stVdaChnAttr.unAttr.stOdAttr.enVdaAlg      = VDA_ALG_REF;
    stVdaChnAttr.unAttr.stOdAttr.enMbSize      = VDA_MB_8PIXEL;
    stVdaChnAttr.unAttr.stOdAttr.enMbSadBits   = VDA_MB_SAD_8BIT;
    stVdaChnAttr.unAttr.stOdAttr.enRefMode     = VDA_REF_MODE_DYNAMIC;
    stVdaChnAttr.unAttr.stOdAttr.u32VdaIntvl   = 4;
    stVdaChnAttr.unAttr.stOdAttr.u32BgUpSrcWgt = 128;
    
    stVdaChnAttr.unAttr.stOdAttr.u32RgnNum = 1;
    
    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].stRect.s32X = 0;
    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].stRect.s32Y = 0;
    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].stRect.u32Width  = pstSize->u32Width;
    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].stRect.u32Height = pstSize->u32Height;

    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].u32SadTh      = 100;
    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].u32AreaTh     = 60;
    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].u32OccCntTh   = 6;
    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].u32UnOccCntTh = 2;
    
    s32Ret = HI_MPI_VDA_CreateChn(VdaChn, &stVdaChnAttr);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err!\n");
        return(s32Ret);
    }

    /********************************************
     step 2 : bind vda channel to vi channel
    ********************************************/
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32ChnId = ViChn;

    stDestChn.enModId = HI_ID_VDA;
    stDestChn.s32ChnId = VdaChn;
	stDestChn.s32DevId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err!\n");
        return s32Ret;
    }

	//lc do start later
    /* vda start rcv picture */
	/*
    s32Ret = HI_MPI_VDA_StartRecvPic(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err!\n");
        return(s32Ret);
    }

    /*........*/
	/*
    gs_stOdParam.bThreadStart = HI_TRUE;
    gs_stOdParam.VdaChn   = VdaChn;

    pthread_create(&gs_VdaPid[SAMPLE_VDA_OD_CHN], 0, SAMPLE_COMM_VDA_OdGetResult, (HI_VOID *)&gs_stOdParam);
	*/
    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VDA_MdStart(VDA_CHN VdaChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	s32Ret = HI_MPI_VDA_StartRecvPic(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err!\n");
        return s32Ret;
    }
	
    return s32Ret;
    /* step 4: create thread to get result */
	
	//gs_stMdParam[VdaChn].bThreadStart = HI_TRUE;
    //gs_stMdParam[VdaChn].VdaChn   = VdaChn;

    //pthread_create(&gs_VdaPid[VdaChn],0, mdfunc, (HI_VOID *)&gs_stMdParam[VdaChn]);
}

HI_S32 SAMPLE_COMM_VDA_OdStart(VDA_CHN VdaChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	s32Ret = HI_MPI_VDA_StartRecvPic(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err!\n");
        return(s32Ret);
    }

	return s32Ret;

    /*........*/
	
    //gs_stOdParam[VdaChn].bThreadStart = HI_TRUE;
    //gs_stOdParam[VdaChn].VdaChn   = VdaChn;

    //pthread_create(&gs_VdaPid[VdaChn], 0, Odfunc, (HI_VOID *)&gs_stOdParam[VdaChn]);
}


HI_S32 SAMPLE_COMM_VDA_MdStop(VDA_CHN VdaChn, VI_CHN ViChn)
{
	HI_S32 s32Ret = HI_SUCCESS;

	MPP_CHN_S stSrcChn, stDestChn;
	
	/* join thread */
    if (HI_TRUE == gs_stMdParam[VdaChn].bThreadStart)
    {
        gs_stMdParam[VdaChn].bThreadStart = HI_FALSE;
        pthread_join(gs_VdaPid[VdaChn], 0);
    }
    
    /* vda stop recv picture */
    s32Ret = HI_MPI_VDA_StopRecvPic(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err(0x%x)!!!!\n",s32Ret);
    }

	 /* unbind vda chn & vi chn */
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32ChnId = ViChn;
    stSrcChn.s32DevId = 0;
    stDestChn.enModId = HI_ID_VDA;
    stDestChn.s32ChnId = VdaChn;
    stDestChn.s32DevId = 0;
    
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err(0x%x)!!!!\n", s32Ret);
    }
	
    /* destroy vda chn */
    s32Ret = HI_MPI_VDA_DestroyChn(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err(0x%x)!!!!\n", s32Ret);
    }
    
	return s32Ret;
}

HI_S32 SAMPLE_COMM_VDA_OdStop(VDA_CHN VdaChn, VI_CHN ViChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn, stDestChn;
	/* join thread */
    if (HI_TRUE == gs_stOdParam[VdaChn].bThreadStart)
    {
        gs_stOdParam[VdaChn].bThreadStart = HI_FALSE;
        pthread_join(gs_VdaPid[VdaChn], 0);
    }
	
    /* vda stop recv picture */
    s32Ret = HI_MPI_VDA_StopRecvPic(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err(0x%x)!!!!\n", s32Ret);
    }

	/* unbind vda chn & vi chn */
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32ChnId = ViChn;
    stSrcChn.s32DevId = 0;
    stDestChn.enModId = HI_ID_VDA;
    stDestChn.s32ChnId = VdaChn;
    stDestChn.s32DevId = 0;
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err(0x%x)!!!!\n", s32Ret);
    }

    /* destroy vda chn */
    s32Ret = HI_MPI_VDA_DestroyChn(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err(0x%x)!!!!\n",s32Ret);
    }

	return s32Ret;
}



/******************************************************************************
* funciton : stop vda, and stop vda thread -- MD
******************************************************************************/
HI_VOID SAMPLE_COMM_VDA_MdUnit(VDA_CHN VdaChn, VI_CHN ViChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    
    MPP_CHN_S stSrcChn, stDestChn;

    /* unbind vda chn & vi chn */

    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32ChnId = ViChn;
    stSrcChn.s32DevId = 0;
    stDestChn.enModId = HI_ID_VDA;
    stDestChn.s32ChnId = VdaChn;
    stDestChn.s32DevId = 0;
    
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err(0x%x)!!!!\n", s32Ret);
    }
	
    /* destroy vda chn */
    s32Ret = HI_MPI_VDA_DestroyChn(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err(0x%x)!!!!\n", s32Ret);
    }
    
    return;
}

/******************************************************************************
* funciton : stop vda, and stop vda thread -- OD
******************************************************************************/
HI_VOID SAMPLE_COMM_VDA_OdUnit(VDA_CHN VdaChn, VI_CHN ViChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn, stDestChn;

    /* unbind vda chn & vi chn */
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32ChnId = ViChn;
    stSrcChn.s32DevId = 0;
    stDestChn.enModId = HI_ID_VDA;
    stDestChn.s32ChnId = VdaChn;
    stDestChn.s32DevId = 0;
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err(0x%x)!!!!\n", s32Ret);
    }

    /* destroy vda chn */
    s32Ret = HI_MPI_VDA_DestroyChn(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("err(0x%x)!!!!\n",s32Ret);
    }
    return;
}


/***********************SAMPLE_COMM_VI**************************************/




HI_S32 SAMPLE_TW2865_CfgV(VIDEO_NORM_E enVideoMode,VI_WORK_MODE_E enWorkMode)
{
    int fd, i;
    int video_mode;
    tw2865_video_norm stVideoMode;
    tw2865_work_mode work_mode;

    int chip_cnt = 4;

    fd = open(TW2865_FILE, O_RDWR);
    if (fd < 0)
    {
        SAMPLE_PRT("open 2865 (%s) fail\n", TW2865_FILE);
        return -1;
    }

    video_mode = (VIDEO_ENCODING_MODE_PAL == enVideoMode) ? TW2865_PAL : TW2865_NTSC ;

    for (i=0; i<chip_cnt; i++)
    {
        stVideoMode.chip    = i;
        stVideoMode.mode    = video_mode;
        if (ioctl(fd, TW2865_SET_VIDEO_NORM, &stVideoMode))
        {
            SAMPLE_PRT("set tw2865(%d) video mode fail\n", i);
            close(fd);
            return -1;
        }
    }

    for (i=0; i<chip_cnt; i++)
    {
        work_mode.chip = i;
        if (VI_WORK_MODE_4Multiplex== enWorkMode)
        {
            work_mode.mode = TW2865_4D1_MODE;
        }
        else if (VI_WORK_MODE_2Multiplex== enWorkMode)
        {
            work_mode.mode = TW2865_2D1_MODE;
        }
        else if (VI_WORK_MODE_1Multiplex == enWorkMode)
        {
            work_mode.mode = TW2865_1D1_MODE;
        }
        else
        {
            SAMPLE_PRT("work mode not support\n");
            return -1;
        }
        ioctl(fd, TW2865_SET_WORK_MODE, &work_mode);
    }

    close(fd);
    return 0;
}

/*

HI_S32 SAMPLE_TW2960_CfgV(VIDEO_NORM_E enVideoMode,VI_WORK_MODE_E enWorkMode)
{
    int fd, i;
    int video_mode;
    tw2865_video_norm stVideoMode;
    tw2865_work_mode work_mode;

    int chip_cnt = 4;

    fd = open(TW2960_FILE, O_RDWR);
    if (fd < 0)
    {
        SAMPLE_PRT("open 2960(%s) fail\n", TW2960_FILE);
        return -1;
    }

    video_mode = (VIDEO_ENCODING_MODE_PAL == enVideoMode) ? TW2960_PAL : TW2960_NTSC ;

    for (i=0; i<chip_cnt; i++)
    {
        stVideoMode.chip    = i;
        stVideoMode.mode    = video_mode;
        if (ioctl(fd, TW2960_SET_VIDEO_NORM, &stVideoMode))
        {
            SAMPLE_PRT("set tw2865(%d) video mode fail\n", i);
            close(fd);
            return -1;
        }
    }

    for (i=0; i<chip_cnt; i++)
    {
        work_mode.chip = i;
        if (VI_WORK_MODE_4Multiplex== enWorkMode)
        {
            work_mode.mode = TW2960_4D1_MODE;
        }
        else if (VI_WORK_MODE_2Multiplex== enWorkMode)
        {
            work_mode.mode = TW2960_2D1_MODE;
        }
        else if (VI_WORK_MODE_1Multiplex == enWorkMode)
        {
            work_mode.mode = TW2960_1D1_MODE;
        }
        else
        {
            SAMPLE_PRT("work mode not support\n");
            return -1;
        }
        ioctl(fd, TW2960_SET_WORK_MODE, &work_mode);
    }

    close(fd);
    return 0;
}
*/

/******************************************************************************
* function : read frame
******************************************************************************/
HI_VOID SAMPLE_COMM_VI_ReadFrame(FILE * fp, HI_U8 * pY, HI_U8 * pU, HI_U8 * pV, HI_U32 width, HI_U32 height, HI_U32 stride, HI_U32 stride2)
{
    HI_U8 * pDst;

    HI_U32 u32Row;

    pDst = pY;
    for ( u32Row = 0; u32Row < height; u32Row++ )
    {
        fread( pDst, width, 1, fp );
        pDst += stride;
    }

    pDst = pU;
    for ( u32Row = 0; u32Row < height/2; u32Row++ )
    {
        fread( pDst, width/2, 1, fp );
        pDst += stride2;
    }

    pDst = pV;
    for ( u32Row = 0; u32Row < height/2; u32Row++ )
    {
        fread( pDst, width/2, 1, fp );
        pDst += stride2;
    }
}

/******************************************************************************
* function : Plan to Semi
******************************************************************************/
HI_S32 SAMPLE_COMM_VI_PlanToSemi(HI_U8 *pY, HI_S32 yStride,
                       HI_U8 *pU, HI_S32 uStride,
                       HI_U8 *pV, HI_S32 vStride,
                       HI_S32 picWidth, HI_S32 picHeight)
{
    HI_S32 i;
    HI_U8* pTmpU, *ptu;
    HI_U8* pTmpV, *ptv;
    HI_S32 s32HafW = uStride >>1 ;
    HI_S32 s32HafH = picHeight >>1 ;
    HI_S32 s32Size = s32HafW*s32HafH;

    pTmpU = malloc( s32Size ); ptu = pTmpU;
    pTmpV = malloc( s32Size ); ptv = pTmpV;

    memcpy(pTmpU,pU,s32Size);
    memcpy(pTmpV,pV,s32Size);

    for(i = 0;i<s32Size>>1;i++)
    {
        *pU++ = *pTmpV++;
        *pU++ = *pTmpU++;

    }
    for(i = 0;i<s32Size>>1;i++)
    {
        *pV++ = *pTmpV++;
        *pV++ = *pTmpU++;
    }

    free( ptu );
    free( ptv );

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VI_GetVFrameFromYUV(FILE *pYUVFile, HI_U32 u32Width, HI_U32 u32Height,HI_U32 u32Stride, VIDEO_FRAME_INFO_S *pstVFrameInfo)
{
    HI_U32             u32LStride;
    HI_U32             u32CStride;
    HI_U32             u32LumaSize;
    HI_U32             u32ChrmSize;
    HI_U32             u32Size;
    VB_BLK VbBlk;
    HI_U32 u32PhyAddr;
    HI_U8 *pVirAddr;

    u32LStride  = u32Stride;
    u32CStride  = u32Stride;

    u32LumaSize = (u32LStride * u32Height);
    u32ChrmSize = (u32CStride * u32Height) >> 2;/* YUV 420 */
    u32Size = u32LumaSize + (u32ChrmSize << 1);

    /* alloc video buffer block ---------------------------------------------------------- */
    VbBlk = HI_MPI_VB_GetBlock(VB_INVALID_POOLID, u32Size, NULL);
    if (VB_INVALID_HANDLE == VbBlk)
    {
        SAMPLE_PRT("HI_MPI_VB_GetBlock err! size:%d\n",u32Size);
        return -1;
    }
    u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(VbBlk);
    if (0 == u32PhyAddr)
    {
        return -1;
    }

    pVirAddr = (HI_U8 *) HI_MPI_SYS_Mmap(u32PhyAddr, u32Size);
    if (NULL == pVirAddr)
    {
        return -1;
    }

    pstVFrameInfo->u32PoolId = HI_MPI_VB_Handle2PoolId(VbBlk);
    if (VB_INVALID_POOLID == pstVFrameInfo->u32PoolId)
    {
        return -1;
    }
    SAMPLE_PRT("pool id :%d, phyAddr:%x,virAddr:%x\n" ,pstVFrameInfo->u32PoolId,u32PhyAddr,(int)pVirAddr);

    pstVFrameInfo->stVFrame.u32PhyAddr[0] = u32PhyAddr;
    pstVFrameInfo->stVFrame.u32PhyAddr[1] = pstVFrameInfo->stVFrame.u32PhyAddr[0] + u32LumaSize;
    pstVFrameInfo->stVFrame.u32PhyAddr[2] = pstVFrameInfo->stVFrame.u32PhyAddr[1] + u32ChrmSize;

    pstVFrameInfo->stVFrame.pVirAddr[0] = pVirAddr;
    pstVFrameInfo->stVFrame.pVirAddr[1] = pstVFrameInfo->stVFrame.pVirAddr[0] + u32LumaSize;
    pstVFrameInfo->stVFrame.pVirAddr[2] = pstVFrameInfo->stVFrame.pVirAddr[1] + u32ChrmSize;

    pstVFrameInfo->stVFrame.u32Width  = u32Width;
    pstVFrameInfo->stVFrame.u32Height = u32Height;
    pstVFrameInfo->stVFrame.u32Stride[0] = u32LStride;
    pstVFrameInfo->stVFrame.u32Stride[1] = u32CStride;
    pstVFrameInfo->stVFrame.u32Stride[2] = u32CStride;
    pstVFrameInfo->stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    pstVFrameInfo->stVFrame.u32Field = VIDEO_FIELD_INTERLACED;/* Intelaced D1,otherwise VIDEO_FIELD_FRAME */

    /* read Y U V data from file to the addr ----------------------------------------------*/
    SAMPLE_COMM_VI_ReadFrame(pYUVFile, pstVFrameInfo->stVFrame.pVirAddr[0],
       pstVFrameInfo->stVFrame.pVirAddr[1], pstVFrameInfo->stVFrame.pVirAddr[2],
       pstVFrameInfo->stVFrame.u32Width, pstVFrameInfo->stVFrame.u32Height,
       pstVFrameInfo->stVFrame.u32Stride[0], pstVFrameInfo->stVFrame.u32Stride[1] >> 1 );

    /* convert planar YUV420 to sem-planar YUV420 -----------------------------------------*/
    SAMPLE_COMM_VI_PlanToSemi(pstVFrameInfo->stVFrame.pVirAddr[0], pstVFrameInfo->stVFrame.u32Stride[0],
      pstVFrameInfo->stVFrame.pVirAddr[1], pstVFrameInfo->stVFrame.u32Stride[1],
      pstVFrameInfo->stVFrame.pVirAddr[2], pstVFrameInfo->stVFrame.u32Stride[1],
      pstVFrameInfo->stVFrame.u32Width, pstVFrameInfo->stVFrame.u32Height);

    HI_MPI_SYS_Munmap(pVirAddr, u32Size);
    return 0;
}



/*****************************************************************************
* function : set vi mask.
*****************************************************************************/
HI_VOID SAMPLE_COMM_VI_SetMask(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr)
{
    switch (ViDev % 4)
    {
        case 0:
            pstDevAttr->au32CompMask[0] = 0xFF000000;
            if (VI_MODE_BT1120_STANDARD == pstDevAttr->enIntfMode)
            {
                pstDevAttr->au32CompMask[1] = 0x00FF0000;
            }
            else if (VI_MODE_BT1120_INTERLEAVED == pstDevAttr->enIntfMode)
            {
                pstDevAttr->au32CompMask[1] = 0x0;
            }
            break;
        case 1:
            pstDevAttr->au32CompMask[0] = 0xFF0000;
            if (VI_MODE_BT1120_INTERLEAVED == pstDevAttr->enIntfMode)
            {
                pstDevAttr->au32CompMask[1] = 0x0;
            }
            break;
        case 2:
            pstDevAttr->au32CompMask[0] = 0xFF00;
            if (VI_MODE_BT1120_STANDARD == pstDevAttr->enIntfMode)
            {
                pstDevAttr->au32CompMask[1] = 0xFF;
            }
            else if (VI_MODE_BT1120_INTERLEAVED == pstDevAttr->enIntfMode)
            {
                pstDevAttr->au32CompMask[1] = 0x0;
            }

            #if HICHIP == HI3531_V100
                #ifndef HI_FPGA
                    if ((VI_MODE_BT1120_STANDARD != pstDevAttr->enIntfMode)
                        && (VI_MODE_BT1120_INTERLEAVED != pstDevAttr->enIntfMode))
                    {
                        /* 3531的ASIC板是两个BT1120口出16D1，此时dev2/6要设成dev1/5的MASK */
                        pstDevAttr->au32CompMask[0] = 0xFF0000; 
                    }
                #endif
            #endif
            
            break;
        case 3:
            pstDevAttr->au32CompMask[0] = 0xFF;
            if (VI_MODE_BT1120_INTERLEAVED == pstDevAttr->enIntfMode)
            {
                pstDevAttr->au32CompMask[1] = 0x0;
            }
            break;
        default:
            HI_ASSERT(0);
    }
}


HI_S32 SAMPLE_COMM_VI_Mode2Param(SAMPLE_VI_MODE_E enViMode, SAMPLE_VI_PARAM_S *pstViParam)
{
    switch (enViMode)
    {
        case SAMPLE_VI_MODE_1_D1:
		case SAMPLE_VI_MODE_1_D1Cif:
            pstViParam->s32ViDevCnt = 1;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 1;
            pstViParam->s32ViChnInterval = 1;
            break;
        case SAMPLE_VI_MODE_16_D1:
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 2;
            pstViParam->s32ViChnCnt = 16;
            pstViParam->s32ViChnInterval = 1;
            break;
        case SAMPLE_VI_MODE_16_960H:
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 2;
            pstViParam->s32ViChnCnt = 16;
            pstViParam->s32ViChnInterval = 1;
            break;
        case SAMPLE_VI_MODE_4_720P:
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 2;
            pstViParam->s32ViChnCnt = 4;
            pstViParam->s32ViChnInterval = 4;
            break;
        case SAMPLE_VI_MODE_4_1080P:
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 2;
            pstViParam->s32ViChnCnt = 4;
            pstViParam->s32ViChnInterval = 4;
            break;

        /*For Hi3521*/
		case SAMPLE_VI_MODE_8_D1:
            pstViParam->s32ViDevCnt = 2;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 8;
            pstViParam->s32ViChnInterval = 1;	
			break;
		case SAMPLE_VI_MODE_1_720P:
            pstViParam->s32ViDevCnt = 1;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 1;
            pstViParam->s32ViChnInterval = 1;	
			break;
		case SAMPLE_VI_MODE_16_Cif:
        case SAMPLE_VI_MODE_16_2Cif:
		case SAMPLE_VI_MODE_16_D1Cif:
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 16;
            pstViParam->s32ViChnInterval = 1;	
			break;
        case SAMPLE_VI_MODE_4_D1:
            pstViParam->s32ViDevCnt = 1;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 4;
            pstViParam->s32ViChnInterval = 1;	
			break; 
        case SAMPLE_VI_MODE_8_2Cif:
            pstViParam->s32ViDevCnt = 2;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 8;
            pstViParam->s32ViChnInterval = 1;	
			break;  
        default:
            SAMPLE_PRT("ViMode invaild!\n");
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}



/*****************************************************************************
* function : get vi parameter, according to vi type
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_Mode2Size(SAMPLE_VI_MODE_E enViMode, VIDEO_NORM_E enNorm, RECT_S *pstCapRect, SIZE_S *pstDestSize)
{
    pstCapRect->s32X = 0;
    pstCapRect->s32Y = 0;
    switch (enViMode)
    {
        case SAMPLE_VI_MODE_1_D1:
        case SAMPLE_VI_MODE_16_D1:
		case SAMPLE_VI_MODE_8_D1:
            pstDestSize->u32Width = D1_WIDTH;
            pstDestSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            pstCapRect->u32Width = D1_WIDTH;
            pstCapRect->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case SAMPLE_VI_MODE_16_960H:
            pstDestSize->u32Width = 960;
            pstDestSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            pstCapRect->u32Width = 960;
            pstCapRect->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case SAMPLE_VI_MODE_4_720P:
		case SAMPLE_VI_MODE_1_720P:	
            pstDestSize->u32Width = 1280;
            pstDestSize->u32Height = 720;
            pstCapRect->u32Width = 1280;
            pstCapRect->u32Height = 720;
            break;
        case SAMPLE_VI_MODE_4_1080P:
            pstDestSize->u32Width = 1920;
            pstDestSize->u32Height = 1080;
            pstCapRect->u32Width = 1920;
            pstCapRect->u32Height = 1080;
            break;
		/*For Hi3521*/
		case SAMPLE_VI_MODE_16_2Cif:
		    pstDestSize->u32Width = D1_WIDTH / 2;
            pstDestSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            pstCapRect->u32Width = D1_WIDTH;
            pstCapRect->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
			break;
        /*For Hi3520A*/
		case SAMPLE_VI_MODE_16_Cif:
		    pstDestSize->u32Width = D1_WIDTH /2 ;
            pstDestSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?288:240;
            pstCapRect->u32Width = D1_WIDTH;
            pstCapRect->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
			break;
        case SAMPLE_VI_MODE_4_D1:
            pstDestSize->u32Width = D1_WIDTH;
            pstDestSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            pstCapRect->u32Width = D1_WIDTH;
            pstCapRect->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case SAMPLE_VI_MODE_8_2Cif:
		    pstDestSize->u32Width = D1_WIDTH / 2;
            pstDestSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            pstCapRect->u32Width = D1_WIDTH;
            pstCapRect->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
			break;
        default:
            SAMPLE_PRT("vi mode invaild!\n");
            return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}


/*****************************************************************************
* function : star vi dev (cfg vi_dev_attr; set_dev_cfg; enable dev)
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_StartDev(VI_DEV ViDev, SAMPLE_VI_MODE_E enViMode)
{
    HI_S32 s32Ret;
    VI_DEV_ATTR_S    stViDevAttr;
    memset(&stViDevAttr,0,sizeof(stViDevAttr));

    switch (enViMode)
    {
        case SAMPLE_VI_MODE_1_D1:
		case SAMPLE_VI_MODE_1_D1Cif:
            memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_4MUX,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        case SAMPLE_VI_MODE_16_D1:
		case SAMPLE_VI_MODE_8_D1:
        case SAMPLE_VI_MODE_4_D1:
        case SAMPLE_VI_MODE_16_Cif:
		case SAMPLE_VI_MODE_16_2Cif:
        case SAMPLE_VI_MODE_8_2Cif:
		case SAMPLE_VI_MODE_16_D1Cif:
            memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_4MUX,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        case SAMPLE_VI_MODE_16_960H:
            memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_4MUX,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        case SAMPLE_VI_MODE_4_720P:
		case SAMPLE_VI_MODE_1_720P:
            memcpy(&stViDevAttr,&DEV_ATTR_7441_BT1120_720P,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        case SAMPLE_VI_MODE_4_1080P:
            memcpy(&stViDevAttr,&DEV_ATTR_7441_BT1120_1080P,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        default:
            SAMPLE_PRT("vi input type[%d] is invalid!\n", enViMode);
            return HI_FAILURE;
    }

    
    s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stViDevAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VI_SetDevAttr failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VI_EnableDev(ViDev);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VI_EnableDev failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


/*****************************************************************************
* function : star vi chn
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_StartChn(VI_CHN ViChn, RECT_S *pstCapRect, SIZE_S *pstTarSize, 
    SAMPLE_VI_MODE_E enViMode, SAMPLE_VI_CHN_SET_E enViChnSet)
{
    HI_S32 s32Ret;
    VI_CHN_ATTR_S stChnAttr;

    /* step  5: config & start vicap dev */
    memcpy(&stChnAttr.stCapRect, pstCapRect, sizeof(RECT_S));
    if (SAMPLE_VI_MODE_16_Cif == enViMode)
    {
        stChnAttr.enCapSel = VI_CAPSEL_BOTTOM;
    }
    else
    {
        stChnAttr.enCapSel = VI_CAPSEL_BOTH;
    }
    /* to show scale. this is a sample only, we want to show dist_size = D1 only */
    stChnAttr.stDestSize.u32Width = pstTarSize->u32Width;
    stChnAttr.stDestSize.u32Height = pstTarSize->u32Height;
    stChnAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;   /* sp420 or sp422 */
    stChnAttr.bMirror = (VI_CHN_SET_MIRROR == enViChnSet)?HI_TRUE:HI_FALSE;
    stChnAttr.bFlip = (VI_CHN_SET_FILP == enViChnSet)?HI_TRUE:HI_FALSE;

    stChnAttr.bChromaResample = HI_FALSE;
    stChnAttr.s32SrcFrameRate = -1;
    stChnAttr.s32FrameRate = -1;

    s32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VI_EnableChn(ViChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
* function : vi input is hd or not.
*****************************************************************************/
HI_BOOL SAMPLE_COMM_VI_IsHd(SAMPLE_VI_MODE_E enViMode)
{
    if (SAMPLE_VI_MODE_4_720P == enViMode || SAMPLE_VI_MODE_4_1080P == enViMode)
        return HI_TRUE;
    else
        return HI_FALSE;
}


HI_S32 SAMPLE_COMM_VI_GetSubChnSize(VI_CHN ViChn_Sub, VIDEO_NORM_E enNorm, SIZE_S *pstSize)
{
    VI_CHN ViChn;
    
    ViChn = ViChn_Sub - 16;

    if (0==(ViChn%4)) //(0,4,8,12) subchn max size is 960x1600
    {
        pstSize->u32Width = 720;
        pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
    }
    else if (0==(ViChn%2)) //(2,6,10,14) subchn max size is 640x720
    {
        pstSize->u32Width = SAMPLE_VI_SUBCHN_2_W;
        pstSize->u32Height = SAMPLE_VI_SUBCHN_2_H;
    }
    else
    {
        SAMPLE_PRT("Vi odd sub_chn(%d) is invaild!\n", ViChn_Sub);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

/*****************************************************************************
* function : get vi parameter, according to vi type
*****************************************************************************/
//lc do 根据实际AD做改动
HI_S32 SAMPLE_COMM_VI_ADStart(SAMPLE_VI_MODE_E enViMode, VIDEO_NORM_E enNorm)
{
    VI_WORK_MODE_E enWorkMode;
    HI_S32 s32Ret;
    
    switch (enViMode)
    {
        case SAMPLE_VI_MODE_1_D1:
		case SAMPLE_VI_MODE_1_D1Cif:
            enWorkMode = VI_WORK_MODE_4Multiplex;
            s32Ret = SAMPLE_TW2865_CfgV(enNorm, enWorkMode);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("SAMPLE_TW2865_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        case SAMPLE_VI_MODE_16_D1:
		case SAMPLE_VI_MODE_8_D1:
        case SAMPLE_VI_MODE_4_D1:
        case SAMPLE_VI_MODE_16_Cif:
		case SAMPLE_VI_MODE_16_2Cif:
        case SAMPLE_VI_MODE_8_2Cif:
		case SAMPLE_VI_MODE_16_D1Cif:
            enWorkMode = VI_WORK_MODE_4Multiplex;
            s32Ret = SAMPLE_TW2865_CfgV(enNorm, enWorkMode);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("SAMPLE_TW2865_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        case SAMPLE_VI_MODE_16_960H:
            enWorkMode = VI_WORK_MODE_4Multiplex;
			/*
			s32Ret = SAMPLE_TW2960_CfgV(enNorm, enWorkMode);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("SAMPLE_TW2960_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            */
            break;
        case SAMPLE_VI_MODE_4_720P:
		case SAMPLE_VI_MODE_1_720P:
            break;
        case SAMPLE_VI_MODE_4_1080P:
            break;
        default:
            SAMPLE_PRT("AD not support!\n");
            return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}




/*****************************************************************************
* function : star vi according to product type
*            if vi input is hd, we will start sub-chn for cvbs preview
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_Start(SAMPLE_VI_MODE_E enViMode, VIDEO_NORM_E enNorm)
{
    VI_DEV ViDev;
    VI_CHN ViChn, ViChn_Sub;
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_VI_PARAM_S stViParam;
    SIZE_S stMainTargetSize;
    SIZE_S stSubTargetSize;
    RECT_S stCapRect;
    
    /*** get parameter from Sample_Vi_Mode ***/
    s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("vi get param failed!\n");
        return HI_FAILURE;
    }
    s32Ret = SAMPLE_COMM_VI_Mode2Size(enViMode, enNorm, &stCapRect, &stMainTargetSize);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("vi get size failed!\n");
        return HI_FAILURE;
    }
    
    /*** Start AD ***/
    s32Ret = SAMPLE_COMM_VI_ADStart(enViMode, enNorm);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("Start AD failed!\n");
        return HI_FAILURE;
    }
    
    /*** Start VI Dev ***/
    for(i=0; i<stViParam.s32ViDevCnt; i++)
    {
        ViDev = i * stViParam.s32ViDevInterval;
        s32Ret = SAMPLE_COMM_VI_StartDev(ViDev, enViMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_COMM_VI_StartDev failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }
    }
    
    /*** Start VI Chn ***/
    for(i=0; i<stViParam.s32ViChnCnt; i++)
    {
        ViChn = i * stViParam.s32ViChnInterval;
        
        s32Ret = SAMPLE_COMM_VI_StartChn(ViChn, &stCapRect, &stMainTargetSize, enViMode, VI_CHN_SET_NORMAL);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("call SAMPLE_COMM_VI_StarChn failed with %#x\n", s32Ret);
            return HI_FAILURE;
        } 
        /* HD mode, we will start vi sub-chn */
        if (HI_TRUE == SAMPLE_COMM_VI_IsHd(enViMode))
        {
            ViChn_Sub = SUBCHN(ViChn);
            s32Ret = SAMPLE_COMM_VI_GetSubChnSize(ViChn_Sub, enNorm, &stSubTargetSize);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("SAMPLE_COMM_VI_GetSubChnSize(%d) failed!\n", ViChn_Sub);
                return HI_FAILURE;
            }
            s32Ret = SAMPLE_COMM_VI_StartChn(ViChn_Sub, &stCapRect, &stSubTargetSize,enViMode, VI_CHN_SET_NORMAL);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("SAMPLE_COMM_VI_StartChn (Sub_Chn-%d) failed!\n", ViChn_Sub);
                return HI_FAILURE;
            }
        }
    }

    return HI_SUCCESS;
}

/*****************************************************************************
* function : stop vi accroding to product type
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_Stop(SAMPLE_VI_MODE_E enViMode)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_VI_PARAM_S stViParam;

    /*** get parameter from Sample_Vi_Mode ***/
    s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }

    /*** Stop VI Chn ***/
    for(i=0;i<stViParam.s32ViChnCnt;i++)
    {
        /* Stop vi phy-chn */
        ViChn = i * stViParam.s32ViChnInterval;
        s32Ret = HI_MPI_VI_DisableChn(ViChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_COMM_VI_StopChn failed with %#x\n",s32Ret);
            return HI_FAILURE;
        }
        /* HD mode, we will stop vi sub-chn */
        if (HI_TRUE == SAMPLE_COMM_VI_IsHd(enViMode))
        {
            ViChn += 16;
            s32Ret = HI_MPI_VI_DisableChn(ViChn);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("SAMPLE_COMM_VI_StopChn failed with %#x\n", s32Ret);
                return HI_FAILURE;
            }
        }
    }

    /*** Stop VI Dev ***/
    for(i=0; i<stViParam.s32ViDevCnt; i++)
    {
        ViDev = i * stViParam.s32ViDevInterval;
        s32Ret = HI_MPI_VI_DisableDev(ViDev);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_COMM_VI_StopDev failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */













































