#include "video_para_api.h"
#include "ipmain_para.h"
#include "devstat.h"
#include "netcmdproc.h"
#include "commonlib.h"
#include "alarm_process.h"
#include "hdmodapi.h"
#include "infodump.h"
#include "watch_board.h"
#include "pthread.h"
//#include "hi_comm_video.h"
#include <sys/socket.h>

static unsigned char blindmask = 0;//遮挡侦测的mask,某位为1表示该位允许侦测
static unsigned char motionmask = 0;//移动侦测的mask,同上

pthread_t vda_thread_id =-1;

//VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_PAL;
//HI_U32    gs_u32ViFrmRate = 0;

//设置视频adc故障状态 0表示正常 1表示故障
static void set_vadc_stat(int flag)
{
	static int  old_stat=0;	
	struct ip1004_state_struct * gtstate=NULL;
	if(old_stat!=flag)
	{		
		gtstate=get_ip1004_state(0);
		pthread_mutex_lock(&gtstate->mutex);		
		gtstate->reg_dev_state.quad_dev_err=flag;
		pthread_mutex_unlock(&gtstate->mutex);
		send_dev_state(-1,1,0,0,0,0);
		old_stat=flag;
	}
}

//设置视频vda故障状态 0表示正常 1表示故障
static void set_vda_stat(int flag)
{
	static int old_vda_stat = 0;
	struct ip1004_state_struct * gtstate=NULL;
	if(old_vda_stat!=flag)
	{		
		gtstate=get_ip1004_state(0);
		pthread_mutex_lock(&gtstate->mutex);		
		//gtstate->reg_dev_state.vda_mod_err=flag;
		pthread_mutex_unlock(&gtstate->mutex);
		send_dev_state(-1,1,0,0,0,0);
		old_vda_stat=flag;
	}
}


//打开vadc的设备节点
int open_video_adc_dev(void)
{
	return 0;
}

//为单路视频初始化亮度色度等值
int init_video_params(struct video_para_struct *vadc, int videono)
{
	struct enc_front_struct * enc_front;

	if((vadc==NULL)||(videono>=get_video_num()))
		return -EINVAL;
	enc_front=&vadc->enc_front[videono];
	set_video_bright(videono,enc_front->bright);
	set_video_hue(videono,enc_front->hue);
	set_video_contrast(videono,enc_front->contrast);
	set_video_saturation(videono,enc_front->saturation);
	
	return 0;

}

int uninit_video_vadc()
{
	int i;
	int ret = 0;
#ifdef USE_VDA
	HI_U32 u32ViChnCnt = get_video_num();
	VDA_CHN mdchn,odchn;
	SAMPLE_VI_MODE_E enViMode;

	if(u32ViChnCnt == 4)
		enViMode = SAMPLE_VI_MODE_4_D1;
	else
		enViMode = SAMPLE_VI_MODE_8_D1;
	
	for(i = 0; i < get_video_num() ; i++)
	{
		mdchn = i;
		odchn = i + u32ViChnCnt;
		ret = SAMPLE_COMM_VDA_MdStop(mdchn,i);
		if(ret != 0)
		{
			printf("SAMPLE_COMM_VDA_MdStop chnidx %d err!\n",i);
			return -1;
		}
		ret = SAMPLE_COMM_VDA_OdStop(odchn,i);
		if(ret != 0)
		{
			printf("SAMPLE_COMM_VDA_OdStart chnidx %d err!\n",i);
			return -1;
		}
	}

	SAMPLE_COMM_VI_Stop(enViMode);
#endif

	return ret;
}

int init_video_vda_param(struct video_para_struct *vadc)
{
	int ret = 0;
#ifdef USE_VDA
	int i;
	int vdaMdch,vdaOdch;
	

	if(vadc==NULL)
		return -EINVAL;

	struct motion_struct *motion;
	struct blind_struct *blind;

	for(i=0;i<get_video_num();i++)
	{
		vdaMdch = i;
		vdaOdch = i+get_video_num();
		motion=&vadc->quad.motion[i];
		ret = SAMPLE_COMM_SET_VDA_MDPara(vdaMdch,motion->sen);
		if(ret != 0)
		{
#ifdef SHOW_WORK_INFO
			printf("SAMPLE_COMM_SET_VDA_MDPara %d error\n",vdaMdch);
#endif
			gtlogerr("SAMPLE_COMM_SET_VDA_MDPara %d error\n",vdaMdch);
			continue;
		}
			
		//set_motion_para(i,motion->sen,motion->area);
		if(motion->sen != 0)
			motionmask |= 1<<i;
				
		blind=&vadc->quad.blind[i];
		//lc do 设置遮挡参数
		ret = SAMPLE_COMM_SET_VDA_ODPara(vdaOdch, blind->sen,blind->alarm_time,blind->cancelalarm_time);
		if(ret != 0)
		{
#ifdef SHOW_WORK_INFO
			printf("SAMPLE_COMM_SET_VDA_ODPara %d error\n",vdaOdch);
#endif
			gtlogerr("SAMPLE_COMM_SET_VDA_ODPara %d error\n",vdaOdch);
			continue;
		}
		//set_blind_sen(i,blind->sen);
		if(blind->sen != 0)
			blindmask |= 1<<i;
	}
#endif
	return ret;
}

int init_video_adc_params(struct video_para_struct *vadc)
{
	int i;
	
	if(vadc==NULL)
		return -EINVAL;

	//lc to do初始化前端AD设备
	/*
	for(i=0;i<get_video_num();i++)
	{
		init_video_params(vadc,i);
	}
	*/
	
	
	//初始化四分割器--如果有的话
	//if(get_quad_flag()==1)
	{
		
		if(vadc->quad.current_net_ch>3)
			set_net_scr_quad(NULL,NULL);
		else
			set_net_scr_full(vadc->quad.current_net_ch,NULL,NULL);
		
		/*
		if(!get_mainpara()->hq_follow_net)
		{
			if(vadc->quad.current_local_ch>3)
				set_local_scr_quad();
			else
				set_local_scr_full(vadc->quad.current_local_ch);	
		}
		*/

	}
	
	return 0;
}

int set_motion_vda_sen(int ch,int sen,WORD *area)
{
	int ret = -1;
	int vdaMdch;
#ifdef USE_VDA
	do
	{
		if(ch <0 || ch > 7)
			break;

		vdaMdch = ch;

		ret = SAMPLE_COMM_SET_VDA_MDPara(vdaMdch,sen);
		if(ret != 0)
		{
#ifdef SHOW_WORK_INFO
			printf("SAMPLE_COMM_SET_VDA_MDPara %d error\n",vdaMdch);
#endif
			gtlogerr("SAMPLE_COMM_SET_VDA_MDPara %d error\n",vdaMdch);
			break;
		}

	}while(0);
#endif
	return ret;
}

int set_blind_vda_sen(int ch,int sen)
{
	int ret = -1;
	int vdaOdch;
#ifdef USE_VDA
	do
	{
		if(ch <0 || ch > 7)
			break;

		vdaOdch = ch;

		ret = SAMPLE_COMM_SET_VDA_ODPara(vdaOdch, sen,-1,-1);
		if(ret != 0)
		{
#ifdef SHOW_WORK_INFO
			printf("SAMPLE_COMM_SET_VDA_ODPara %d error\n",vdaOdch);
#endif
			gtlogerr("SAMPLE_COMM_SET_VDA_ODPara %d error\n",vdaOdch);
			break;
		}

	}while(0);

#endif
	return ret;
}

static pthread_mutex_t tw2835_mutex;

#ifdef USE_VDA
HI_S32 SetVideoLossUserPic(HI_CHAR *pszYuvFile, HI_U32 u32Width, HI_U32 u32Height,
        HI_U32 u32Stride,VIDEO_FRAME_INFO_S *pstFrame)
{
	FILE *pfd;
    VI_USERPIC_ATTR_S stUserPicAttr;

    /* open YUV file */
    pfd = fopen(pszYuvFile, "rb");
    if (!pfd)
    {
        printf("open file -> %s fail \n", pszYuvFile);
        return -1;
    }

    /* read YUV file. WARNING: we only support planar 420) */
    if (SAMPLE_COMM_VI_GetVFrameFromYUV(pfd, u32Width, u32Height, u32Stride, pstFrame))
    {
        return -1;
    }
    fclose(pfd);

    stUserPicAttr.bPub= HI_TRUE;
    stUserPicAttr.enUsrPicMode = VI_USERPIC_MODE_PIC;
    memcpy(&stUserPicAttr.unUsrPic.stUsrPicFrm, pstFrame, sizeof(VIDEO_FRAME_INFO_S));
    if (HI_MPI_VI_SetUserPic(0, &stUserPicAttr))
    {
        return -1;
    }

	printf("set vi user pic ok, yuvfile:%s\n", pszYuvFile);
	gtloginfo("set vi user pic ok, yuvfile:%s\n", pszYuvFile);
    return HI_SUCCESS;
}
#endif


/******************************************
 * 初始化mpp\vda模块\ad模块
 * 返回值 0:成功，负值失败
******************************************/
int init_video_vadc(struct video_para_struct *vadc)
{
#ifdef USE_VDA
	int ret;
	//ipmain_para_struct *mainpara;
	
	if(vadc == NULL)
		return -EINVAL;

	//lc do 初始化mpp系统，创建vda通道，bind vi
	HI_S32 s32Ret = HI_SUCCESS;
    VB_CONF_S stVbConf ={0};	/* vb config define */

	HI_U32 u32BlkSize;
    SIZE_S stSize;
    SAMPLE_VI_MODE_E enViMode;

	PIC_SIZE_E enSize_Md,enSize_Od;
	
	
	HI_U32 u32ViChnCnt = get_video_num();
	if(u32ViChnCnt == 4)
		enViMode = SAMPLE_VI_MODE_4_D1;
	else
		enViMode = SAMPLE_VI_MODE_8_D1;

	//mainpara = get_mainpara();
	//if( mainpara->net_ch_osd_picsize == 0)  //cif  /* vda picture size */
	//{
	//	enSize_Md = enSize_Od = PIC_CIF;
	//}
	//else
		enSize_Md = enSize_Od = PIC_D1;
		
	VI_CHN ViChn, ViChnIndex_Md = 0, ViChnIndex_Od = 0;
    VDA_CHN VdaChnIndex_Md = 0, VdaChnIndex_Od = u32ViChnCnt;
	HI_S32 i;
	VIDEO_FRAME_INFO_S stUserFrame;
/*
    VO_DEV VoDev;
    VO_CHN VoChn;
    VO_PUB_ATTR_S stVoPubAttr; 
    SAMPLE_VO_MODE_E enVoMode;
    HI_U32 u32WndNum;
*/  
#if 0
	do
	{
	/******************************************
     step  1: init global  variable 
    ******************************************/
    //gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL== gs_enNorm)?25:30;
    
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                PIC_D1, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;

    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 10;
    memset(stVbConf.astCommPool[0].acMmzName,0,
        sizeof(stVbConf.astCommPool[0].acMmzName));

    /* hist buf*/
    stVbConf.astCommPool[1].u32BlkSize = (196*4);
    stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 10;
    memset(stVbConf.astCommPool[1].acMmzName,0,
        sizeof(stVbConf.astCommPool[1].acMmzName));

    /******************************************
     step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf,HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
#ifdef SHOW_WORK_INFO
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
#endif
		gtlogerr("初始化mpp系统失败，错误号为%d!\n",s32Ret);
        break;
    }

	/******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
#ifdef SHOW_WORK_INFO    
        SAMPLE_PRT("start vi failed!\n");
#endif
		gtlogerr("打开vi通道失败，错误号为%d!\n",s32Ret);
        break;
    }

	#if 0    
    /******************************************
     step 5: start VO to preview
    ******************************************/

    VoDev = SAMPLE_VO_DEV_DSD0;
    u32WndNum = 1;
    enVoMode = VO_MODE_1MUX;
    
    stVoPubAttr.enIntfSync = VO_OUTPUT_PAL;
    stVoPubAttr.enIntfType = VO_INTF_CVBS;
    stVoPubAttr.u32BgColor = 0x000000ff;
    stVoPubAttr.bDoubleFrame = HI_FALSE;

    /******************************************
         step 4: start VO to preview
    ******************************************/
    VoDev = SAMPLE_VO_DEV_DHD0;
    u32WndNum = 1;
    enVoMode = VO_MODE_1MUX;

    stVoPubAttr.enIntfSync = VO_OUTPUT_720P50;
    stVoPubAttr.enIntfType = VO_INTF_HDMI|VO_INTF_VGA;
    stVoPubAttr.u32BgColor = 0x000000ff;
    stVoPubAttr.bDoubleFrame = HI_FALSE;

    s32Ret = SAMPLE_COMM_VO_StartDevLayer(VoDev, &stVoPubAttr, gs_u32ViFrmRate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartDevLayer failed!\n");
        goto END0;
    }
    
    s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, &stVoPubAttr, enVoMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
        goto END0;
    }

    for(i=0;i<u32WndNum;i++)
    {
        VoChn = i;
        ViChn = i;
        s32Ret = SAMPLE_COMM_VO_BindVi(VoDev,VoChn,ViChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_COMM_VO_BindVi failed!\n");
            goto END0;
        }
    }
#endif
#endif
	do
	{
	//lc do 设置视频丢失时图片novideo.yuv**********************************************/
	s32Ret = SetVideoLossUserPic(USER_PIC_PATH, 704, 576, 704, &stUserFrame);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_SetUserPic failed!\n");
        break;
    }

	//lc do 设置vda各通道**************************************************************/
	for(i =0 ; i < u32ViChnCnt ; i++)
	{
		s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize_Md, &stSize);
    	if (HI_SUCCESS != s32Ret)
    	{
        	SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        	break;
    	}
		s32Ret = SAMPLE_COMM_VDA_MdInit(VdaChnIndex_Md+i, ViChnIndex_Md+i, &stSize);
		if (HI_SUCCESS != s32Ret)
    	{
        	SAMPLE_PRT("VDA Md Start failed!\n");
        	break;
    	}
		s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize_Od, &stSize);
    	if (HI_SUCCESS != s32Ret)
    	{
     	   SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
     	   break;
  	    }
  		s32Ret = SAMPLE_COMM_VDA_OdInit(VdaChnIndex_Od+i, ViChnIndex_Od+i, &stSize);
  	    if (HI_SUCCESS != s32Ret)
   	    {
        	SAMPLE_PRT("VDA OD Start failed!\n");
        	break;
    	}
		
	}
    
    if (HI_SUCCESS != s32Ret)
		break;
    
    }while(0);

	if(s32Ret != HI_SUCCESS)
	{
		//set_vda_stat(1);
		return -1;
	}
	else
	{
#ifdef SHOW_WORK_INFO
		printf("init vda mod success!\n");
#endif
		gtloginfo("初始化vda模块成功!\n");
		//set_vda_stat(0);
	}

	init_video_vda_param(vadc);

	/*
	ret =open_video_adc_dev();
	if(ret <0)
	{
		printf("can't open video front device ,ret = %d,%s\n",ret,strerror(-ret));
		gtloginfo("can't open video front device ,ret = %d,%s\n",ret,strerror(-ret));
		set_vadc_stat(1);	
		return ret;
	}
	
	
	#ifdef SHOW_WORK_INFO
		printf("open video front device success. \n");	
	#endif
	init_video_adc_params(vadc);
	set_vadc_stat(0);
	pthread_mutex_init(&tw2835_mutex, NULL);
	*/
	
#endif	
	return 0;

}

struct sockaddr_in1
  {
    short int sin_family;
    unsigned short int sin_port;                 /* Port number.  */
    struct in_addr sin_addr;            /* Internet address.  */

    /* Pad to size of `struct sockaddr'.  */
    unsigned char sin_zero[8];
};

int send_modcmd2rtimg(int ch,struct sockaddr *peer,char* usrname)
{
	mod_socket_cmd_type *send;
	struct sockaddr_in1 local_inet;
	//struct gt_usr_cmd_struct *bypass;
	int rc;
	BYTE sendbuf[300];
	BYTE user[20];
	//struct sockaddr_in local_inet;
	
	send=(mod_socket_cmd_type *)sendbuf;
	send->gate.env = 0;
	send->gate.enc = 0;
	send->gate.gatefd = -1;
	send->cmd = SET_SCREEN_DISPLAY;
	send->gate.dev_no = 0;
	send->len = sizeof(set_scr_display_struct);
	set_scr_display_struct setscrcmd;
	setscrcmd.channel = ch;
	memset(setscrcmd.peeruser,0,sizeof(setscrcmd.peeruser));
	if(usrname == NULL)
		memcpy((void*)setscrcmd.peeruser,"fortest",sizeof(setscrcmd.peeruser));
	else
		memcpy((void*)setscrcmd.peeruser,usrname,sizeof(setscrcmd.peeruser));
	if(peer == NULL)
	{
		memset(&local_inet,0,sizeof(struct sockaddr));
		local_inet.sin_family = AF_INET;
		local_inet.sin_port = htons(9000);
		inet_aton("127.0.0.1",&local_inet.sin_addr);		
		memcpy((void*)setscrcmd.peeraddr,(void*)&local_inet,sizeof(struct sockaddr));
	}
	else
		memcpy((void*)setscrcmd.peeraddr,(void*)peer,sizeof(struct sockaddr));

	printf("set_scr_display_struct size is %d\n",sizeof(set_scr_display_struct));
	memcpy((BYTE *)&send->para,(BYTE *)(&setscrcmd),sizeof(set_scr_display_struct));	
	
	rc=main_send_cmd(send,RTIMAGE_PROCESS_ID,sizeof(mod_socket_cmd_type)-sizeof(send->para)+send->len);
}

/********************************************************
 * 将网络传输通道的显示切换为全屏，第ch通道
 * 返回值   0：成功,负值表示错误
 * 输入:		ch：	需要切换的目标摄像头通道(有效值0,1,2,3)	
 ***************************************************/
int set_net_scr_full(BYTE ch,struct sockaddr* peer,char* username)
{
	//lc do 向rtimg发命令，切换通道
	int ich = ch;
	struct ipmain_para_struct * pm;
#ifdef SHOW_WORK_INFO
	printf("set_net_scr_full 通道号%d!\n",ich);
#endif
	pm = get_mainpara();
	pm->vadc.quad.current_net_ch=ch;
	return send_modcmd2rtimg(ich,peer,username);
}

/********************************************************
 * 将本地录像通道的显示切换为全屏，第ch通道
 * 返回值   0：成功,负值表示错误
 * 输入:		ch：	需要切换的目标摄像头通道(有效值0,1,2,3)	
 ***************************************************/
/*
int set_local_scr_full(BYTE ch)
 {
 	struct ipmain_para_struct * pm;
	int ret;

	pm=get_mainpara();
	pm->vadc.quad.current_local_ch=ch;
	//写osd信息

	//pthread_mutex_lock(&tw2835_mutex);

	write_osd_info(OSD_LOCAL_CH,get_mainpara()->local_ch_osd_picsize);	
	ret = set_tw2835_screen(LOCAL_CH,ch);
	//pthread_mutex_unlock(&tw2835_mutex);

 	return ret;
 }
 */
/********************************************************
 * 将网络传输通道的显示切换为四分割
 * 返回值   0：成功,负值表示错误
 * 输入:		ch：	需要切换的目标摄像头通道(有效值0,1,2,3)	
 ***************************************************/
 int set_net_scr_quad(struct sockaddr* peer,char* username)
 {
 	//lc do 向rtimg发命令，切换通道
 	struct ipmain_para_struct * pm;
#ifdef SHOW_WORK_INFO
	printf("set_net_scr_quad 4分屏!\n");
#endif
	pm = get_mainpara();
	pm->vadc.quad.current_net_ch=4;
	return send_modcmd2rtimg(4,peer,username);
	/*
 	struct ipmain_para_struct * pm;
	int ret;

	pm = get_mainpara();
	 pm->vadc.quad.current_net_ch=4;
 			
   if(get_videoenc_num()==2)
    {//如果有两个视频编码器则切换   //shixin add
    
		if(pm->hq_follow_net)
		{
				set_local_scr_quad();
		}
	
	}
	 //写osd信息
	//pthread_mutex_lock(&tw2835_mutex);
	write_osd_info(OSD_NET_CH,get_mainpara()->net_ch_osd_picsize);
	ret = set_tw2835_screen(NET_CH,4);
	//pthread_mutex_unlock(&tw2835_mutex);
  	
  	return ret;
  	*/
  	//return 0;
 }
 
/********************************************************
 * 将本地录像通道的显示切换为四分割
 * 返回值   0：成功,负值表示错误
 * 输入:		ch：	需要切换的目标摄像头通道(有效值0,1,2,3)	
 ***************************************************/ 
/*
int set_local_scr_quad(void)
{
	struct ipmain_para_struct * pm;
	int ret;
	#if EMBEDED==0
		return 0;
	#endif
	pm=get_mainpara();
	pm->vadc.quad.current_local_ch=4;
	
	//写osd信息
	//pthread_mutex_lock(&tw2835_mutex);
	write_osd_info(OSD_LOCAL_CH,get_mainpara()->local_ch_osd_picsize);
	
	ret = set_tw2835_screen(LOCAL_CH,4);
	//pthread_mutex_unlock(&tw2835_mutex);

	return ret;
}
*/














//设置视频输入亮度,val为百分比
int set_video_bright(BYTE ch,int val)			
{
	return 0;
}
//设置视频输入色度,val为百分比
int set_video_hue(BYTE ch,int val)				
{
	return 0;
}

//设置视频输入对比度,val为百分比
int set_video_contrast(BYTE ch,int val)		
{
	return 0;
}

//设置视频输入饱和度,val为百分比
int set_video_saturation(BYTE ch,int val)		
{	
	return 0;
}

//重起ad芯片
int reset_video_adc(int chipno)
{
	return 0;
}



static unsigned long old_motion[MAX_VIDEO_IN];
static unsigned long old_loss[MAX_VIDEO_IN];	
static unsigned long old_blind[MAX_VIDEO_IN];



static vstate_t loss_state;//记录视频丢失状态的相应参数
static vstate_t motion_state;//记录视频移动状态的相应参数
static vstate_t blind_state;//记录视频遮挡状态的相应参数





#include "netcmdproc.h"

/*
	video_loss_proc
	video_motion_detected
	video_blind_detected
	都应具有处理原始数据的能力(即自己判断变化)
*/

int video_loss_proc(unsigned long loss)
{
	int i;
	int lossbit=0;
	struct video_para_struct *vadc;
	struct ip1004_state_struct * gtstate;
	
	loss&=(1<<(get_video_num()))-1;//MASK
	struct ip1004_state_struct *gtstate1;
	gtstate=get_ip1004_state(0);
	pthread_mutex_lock(&gtstate->mutex);
	if(virdev_get_virdev_number()==2)
	{
		gtstate1=get_ip1004_state(1);
		pthread_mutex_lock(&gtstate1->mutex);
	}
	vadc = &get_mainpara()->vadc;
	for(i=0;i<get_video_num();i++)
	{
		//lc do 对于videoloss，无论是否enable，都发送状态
		//if(vadc->enc_front[i].enable != 1)//若不enable则不予处理
		//	continue;
		lossbit = ((loss>>i)&1);
		if(lossbit == old_loss[i])
			continue;
		//gtloginfo("wsytest,read vloss = 0x%02x\n",loss);
	
		old_loss[i] = lossbit;
		
		if(lossbit == 1)
		{
			printf("第%d路视频丢失\n",i);
			gtloginfo("第%d路视频丢失\n",i);
		}
		else
		{
			printf("第%d路视频恢复\n",i);
			gtloginfo("第%d路视频恢复\n",i);
		}
	
	if(virdev_get_virdev_number()==2)
	{
		switch(i)
		{
			case 0:	gtstate->reg_per_state.video_loss0=lossbit;break;
			case 1:	gtstate1->reg_per_state.video_loss0=lossbit;break;
			case 2:	gtstate->reg_per_state.video_loss2=lossbit;break;
			case 3:	gtstate->reg_per_state.video_loss3=lossbit;break;
		}
	}
	else
	{
		switch(i)
		{
			case 0:	gtstate->reg_per_state.video_loss0=lossbit;break;
			case 1:	gtstate->reg_per_state.video_loss1=lossbit;break;
			case 2:	gtstate->reg_per_state.video_loss2=lossbit;break;
			case 3:	gtstate->reg_per_state.video_loss3=lossbit;break;
		}
	}
	}
	pthread_mutex_unlock(&gtstate->mutex);	
	if(virdev_get_virdev_number()==2)
		pthread_mutex_unlock(&gtstate1->mutex);	
	for(i=0;i<virdev_get_virdev_number();i++)
		send_dev_state(-1,1,0,0,0,i);


	return 0;
}

int video_motion_detected(unsigned long motion)
{

	struct quad_dev_struct *quad;
	struct motion_struct *motionstruct;
	struct alarm_motion_struct *alarm_motion;
	struct ipmain_para_struct *para;
	int i,j,event;
	char alarminfo[100];
	int motionbit;//该路的视频移动寄存器状态
	int alarmbit=0;//该路是否符合报警条件
	int changeflag=0;//记录devtrig是否有改变
	struct ip1004_state_struct *ip1004state;
	struct trig_state_struct *trigstate;
	struct alarm_trigin_struct *trigin;
	struct send_dev_trig_state_struct dev_trig;
	time_t timep; 
	struct video_para_struct *vadc;
	
	quad=&(get_mainpara()->vadc.quad);
	if(virdev_get_virdev_number()==2)
		return 0;
	else
		ip1004state=get_ip1004_state(0);
	
	alarm_motion=&(get_mainpara()->alarm_motion);
	vadc = &get_mainpara()->vadc;
	para=get_mainpara();
	
	for(i=0;i<get_video_num();i++)
	{	
		//看是否enable
		motionstruct=&quad->motion[i];		
		trigin=&alarm_motion->motion[i]; 

		if(vadc->enc_front[i].enable!=1)
		{	
			continue;
		}
		//若该路不enable或者已经vloss
		if((trigin->enable!=1)||(old_loss[i]==1))
		{
			continue;
		}
		
		changeflag	= 0;
		alarmbit 	= 0;
		motionbit	= (motion >> i)&1;
		
		if(motionbit == 1)//当前状态为移动
		{
			printf("video%d motion event(setalarm=%d alarm=%d)\n",i,trigin->setalarm,(int)motionstruct->alarm);//added by shixin

			//如果设为移动不报警，或撤防状态，或时间不合适
			if((motionstruct->alarm!=1)||(trigin->setalarm!=1)||(is_alarm_interval(2, i)!=1))	
			{	
				if(para->multi_channel_enable)
					trig_record_event(i,0,0);
				else
					trig_record_event(0,0,0);
				if((motionstruct->alarm == 1)&&(old_motion[i]!=motionbit))//需要记日志以方便调试
				{
					if(trigin->setalarm!=1)
						gtloginfo("%d路视频移动侦测布防状态为%d,不报警",i,trigin->setalarm);
					else
						gtloginfo("%d路移动侦测但不在%02d:%02d-%02d:%02d时间范围内\n",i,motionstruct->starthour,motionstruct->startmin,motionstruct->endhour,motionstruct->endmin);
				}
			}
			else
				alarmbit = 1;//需要报警
		}
		old_motion[i]= motionbit;
				
		 pthread_mutex_lock(&ip1004state->mutex);
		 trigstate=&(ip1004state->reg_trig_state);
		//lc do 根据字节序，反向赋值
		 switch(i)
 		 {
 		 	case(0): if(trigstate->motion0!=alarmbit)
					{
						trigstate->motion0=alarmbit;
						//trigstate->motion11=alarmbit;
						changeflag=1;
					}
					break;
		
			 case(1):if(trigstate->motion1!=alarmbit)
	 		 		{
	 		 			trigstate->motion1=alarmbit;
	 		 			//trigstate->motion10=alarmbit;
						changeflag=1;
	 				}
			 		  break;
			 case(2):if(trigstate->motion2!=alarmbit)
					{
						trigstate->motion2=alarmbit;
						//trigstate->motion9=alarmbit;
						changeflag=1;
		 			}	
			 		  break;
			 case(3):if(trigstate->motion3!=alarmbit)
		 		 	{
		 		 		trigstate->motion3=alarmbit;
		 		 		//trigstate->motion8=alarmbit;
						changeflag=1;
		 			}
			 		  	break;
			 default: break;
		}
 		 pthread_mutex_unlock(&ip1004state->mutex);

		if((alarmbit == 1)&&(changeflag == 1))
 		{
 			//有有效的新增
 			printf("第%d路移动侦测有有效触发\n",i);
			gtloginfo("第%d路移动侦测有有效触发\n",i);
		
			//报警，写alarm_log
			time(&timep);
			get_dev_trig(0,&dev_trig);
			sprintf(alarminfo,"[ALARM] TRIG:0x%04x\n",(int)dev_trig.alarmstate);
			printf("video_motion_detected,%s",alarminfo);
			//lc do 需sqlite支持
			dump_alarminfo_to_log((int)dev_trig.alarmstate,timep,"--");	
			
			send_dev_trig_state(-2,&dev_trig,1,0,0,0);


			//处理报警联动
			gtloginfo("开始处理第%d路移动触发的报警联动!\n",i);
			//启动录像
			trig_record_event(i,(int)dev_trig.alarmstate,0);
			//gtloginfo("移动%d触发报警联动事件,应于%d通道录像\n",i,i); 	
			//处理立即处理的
			for(j=0;j<MAX_TRIG_EVENT;j++)
			{
				event=get_alarm_event(i+get_trigin_num(),0,j);
				take_alarm_action(i,event);
			}
		}
	}

	return 0;
}		
int video_blind_detected(unsigned long blind)
{
	struct ip1004_state_struct *gtstate;
	struct quad_dev_struct*quad;
	int blindbit;
	int i;
	struct video_para_struct *vadc;
	
	vadc = &get_mainpara()->vadc;

	quad=&(get_mainpara()->vadc.quad);

	blind&=(1<<(get_video_num()))-1;//MASK
	if(virdev_get_virdev_number()==2)
		return 0;
	else
		gtstate=get_ip1004_state(0);
	pthread_mutex_lock(&gtstate->mutex);
		
		
	for(i=0;i<get_video_num();i++)
	{
		if(old_loss[i] == 1)//loss不考虑遮挡
			continue;

		if(vadc->enc_front[i].enable != 1)//disabled
			continue;
			
		blindbit = ((blind>>i)&1);
		if(blindbit == old_blind[i])
			continue;
		
		old_blind[i] = blindbit;
		
		if(blindbit == 1)
		{
			printf("#############%d路遮挡报状态#########\n",i);
			gtloginfo("##########%d路遮挡报状态\n",i);	
		}
		else
		{
			printf("#############%d路遮挡取消#########\n",i);
			gtloginfo("##########%d路遮挡取消\n",i);
		}
		
		switch(i)
		{
			case 0:	gtstate->reg_per_state.video_blind0=blindbit;break;
			case 1:	gtstate->reg_per_state.video_blind1=blindbit;break;
			case 2:	gtstate->reg_per_state.video_blind2=blindbit;break;
			case 3:	gtstate->reg_per_state.video_blind3=blindbit;break;
		}
	}
	pthread_mutex_unlock(&gtstate->mutex);	
	send_dev_state(-1,1,0,0,0,0);
	
		
	return 0;

}

#ifdef USE_VDA
static HI_S32 s_astViLastIntCnt[VIU_MAX_CHN_NUM] = {0};

static void * vda_proc_thread(void * para)
{
	unsigned long loss=0;
	unsigned long chiploss;
	int i,j= 0;
	HI_S32 s32Ret;
    VDA_CHN VdaMdChn,VdaOdChn;
	VI_CHN ViChn;
	VI_CHN_STAT_S stStat;
	int newbit,oldbit;
    VDA_DATA_S stVdaData;
    HI_S32 maxfd = 0;
    //FILE *fp = stdout;
    HI_S32 VdaFd[4];
    fd_set read_fds;
    struct timeval TimeoutVal;
	unsigned long motion;
	unsigned long blind;
	WORD video_mask=0;
	unsigned long result=0;
	video_mask = get_mainpara()->video_mask;
	HI_U32 u32RgnNum;
	
	/* decide the stream file name, and open file to save stream */
    /* Set Venc Fd. */
	for(i=0;i<get_video_num();i++)
	{
    	VdaFd[i] = HI_MPI_VDA_GetFd(i);
		if (VdaFd[i] < 0)
    	{
        	SAMPLE_PRT("HI_MPI_VDA_GetFd failed with %#x!\n", 
            	   VdaFd);
        	return NULL;
    	}
		if (maxfd <= VdaFd[i])
   		{
   		    maxfd = VdaFd;
  		}
	}
	
	gtloginfo("start vda_proc_thread...\n");
	printf("start vda_proc_thread...\n");

	//处理侦测结果
	while (1)
    {
		sleep(1);
		
		//lc do 视频丢失检测******************************************************************/
		for(i=0;i<get_video_num();i++)
		{
			ViChn = i;
			s32Ret = HI_MPI_VI_Query(ViChn, &stStat);
			if (HI_SUCCESS !=s32Ret)
            {
                SAMPLE_PRT("HI_MPI_VI_Query ViChn %d failed with %#x!\n",ViChn,s32Ret);
                continue;
            }
			printf("HI_MPI_VI_Query %d VI Chn,IntCnt is %d\n",ViChn,stStat.u32IntCnt);
            if (stStat.u32IntCnt == s_astViLastIntCnt[i])
            {
                printf("VI Chn (%d) int lost , int_cnt:%d \n", ViChn, stStat.u32IntCnt);
				HI_MPI_VI_EnableUserPic(ViChn);
                loss |= 1 << i;
            }
            else
            {
                HI_MPI_VI_DisableUserPic(ViChn);
				loss &= ~(1 << i);
            }
            s_astViLastIntCnt[i] = stStat.u32IntCnt;
		}

		loss=delay_on_vstate(loss&video_mask,0,VLOSS_CANCEL_TIME,&loss_state);
		video_loss_proc(loss);
		

		//lc do 移动检测处理******************************************************************/
		FD_ZERO(&read_fds);
		for(i=0;i<get_video_num();i++)
			FD_SET(VdaFd[i], &read_fds);

        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            SAMPLE_PRT("select failed!continue\n");    //fix me
            continue;
        }
        else if (s32Ret == 0)
        {
            SAMPLE_PRT("select md vdachn timeout,continue\n");
            continue;
        }
        else
        {
			for(i=0;i<get_video_num();i++)
			{
				if (FD_ISSET(VdaFd[i], &read_fds))
            	{
               	 /*******************************************************
                   step 2.3 : call mpi to get one-frame stream
                   *******************************************************/
                   VdaMdChn = i;
                   s32Ret = HI_MPI_VDA_GetData(VdaMdChn, &stVdaData, HI_TRUE);
                   if(s32Ret != HI_SUCCESS)
                   {
                       SAMPLE_PRT("HI_MPI_VDA_GetData failed with %#x!\n", s32Ret);
                       continue;
                   }
                /*******************************************************
                   *step 2.4 : save frame to file
                   *******************************************************/
		        //SAMPLE_COMM_VDA_MdPrtSad(fp, &stVdaData);
		        //SAMPLE_COMM_VDA_MdPrtObj(fp, &stVdaData);
                //SAMPLE_COMM_VDA_MdPrtAp(fp, &stVdaData);
				
				  //lc do 根据结果转换为当前通道是否报警，并向网关发信息
				  if (HI_TRUE != stVdaData.unData.stMdData.bPelsNumValid)
   				  {
      				  //fprintf(fp, "bMbObjValid = FALSE.\n");
    			  #ifdef SHOW_WORK_INFO
      			       printf("%d chn bMbObjValid = FALSE.\n",VdaMdChn);
				  #endif
					   gtlogerr("%d chn bMbObjValid = FALSE.\n",VdaMdChn);

					   s32Ret = HI_MPI_VDA_ReleaseData(VdaMdChn,&stVdaData);
               		   if(s32Ret != HI_SUCCESS)
	                   {
	                	 SAMPLE_PRT("HI_MPI_VDA_ReleaseData failed with %#x!\n", s32Ret);
	                	 return NULL;
	            	   }
        			   continue;
   				  }

				  if(stVdaData.unData.stMdData.u32AlarmPixCnt > get_mainpara()->vadc.quad.motion[i].sen)
				  {
					  //lc 目前最简单处理方式，用sen表示报警像素点个数阈值，并作为判断依据
					  //motion = motion_state.old_value|(1<<i);
					  //newbit = 1;
					  printf("HI_MPI_VDA_GetData %d VI Chn is alarmed!,u32AlarmPixCnt is %d\n",ViChn,stVdaData.unData.stMdData.u32AlarmPixCnt);
					  motion |= 1<<i;
				  }
				  else
				  {
					  motion &= ~(1<<i);
				  }

				/*******************************************************
                   *step 2.5 : release stream
                   *******************************************************/
                  s32Ret = HI_MPI_VDA_ReleaseData(VdaMdChn,&stVdaData);
                  if(s32Ret != HI_SUCCESS)
	              {
	                  SAMPLE_PRT("HI_MPI_VDA_ReleaseData failed with %#x!\n", s32Ret);
	                  return NULL;
	              }

				}

			}
        }
		//lc 得到4路的新motion值
		motion &= motionmask;
		motion=delay_on_vstate(motion&video_mask,0,VMOTION_CANCEL_TIME,&motion_state);
		printf("lctest,motion now = 0x%x\n",motion);
		video_motion_detected(motion);
				
		//lc do blind检测处理************************************************************************/
		for(i = 0 ;i < get_video_num(); i++)
		{
			VdaOdChn = i + get_video_num();
			s32Ret = HI_MPI_VDA_GetData(VdaOdChn,&stVdaData,HI_TRUE);
       		if(s32Ret != HI_SUCCESS)
       		{
            	SAMPLE_PRT("HI_MPI_VDA_GetData Od %d failed with %#x!\n", VdaOdChn,s32Ret);
            	return NULL;
        	}
			u32RgnNum = stVdaData.unData.stOdData.u32RgnNum;
			for(j=0; j<u32RgnNum; j++)
       		{
            	if(HI_TRUE == stVdaData.unData.stOdData.abRgnAlarm[j])
            	{ 
               	    printf("################VdaChn--%d,Rgn--%d,Occ!\n",VdaOdChn,i);
                	s32Ret = HI_MPI_VDA_ResetOdRegion(VdaOdChn,i);
               		if(s32Ret != HI_SUCCESS)
                	{
		           		SAMPLE_PRT("HI_MPI_VDA_ResetOdRegion failed with %#x!\n", s32Ret);
                   		return NULL;
                	}

						blind |= 1<<i ;
            	}
        	 }
			 
			 s32Ret = HI_MPI_VDA_ReleaseData(VdaOdChn,&stVdaData);
			 if(s32Ret != HI_SUCCESS)
        	 {
            	SAMPLE_PRT("HI_MPI_VDA_ReleaseData failed with %#x!\n", s32Ret);
            	return NULL;
        	 }
		 }

		 blind &= blindmask; 
		 //lc 3520A内部已有延迟机制，无需此步骤
		 //blind=delay_on_vstate(blind&video_mask,blindstruct->alarm_time,blindstruct->cancelalarm_time,&blind_state);
		 //printf("info.blind = 0x%x\t, blind now 0x%x,\n",info.blind,blind);
		 video_blind_detected(blind&video_mask); 

       }
    
	return 0;
}
#endif

int creat_vda_proc_thread(pthread_attr_t *attr,void *arg)
{
#ifdef USE_VDA
	VDA_CHN mdchn,odchn;
	int i;
	int ret;
	//lc 启动md,od侦测
	for(i = 0; i < get_video_num() ; i++)
	{
		mdchn = i;
		odchn = i + get_video_num();
		ret = SAMPLE_COMM_VDA_MdStart(mdchn);
		if(ret != 0)
		{
			printf("SAMPLE_COMM_VDA_MdStart chnidx %d err!\n",i);
			return -1;
		}
		ret = SAMPLE_COMM_VDA_OdStart(odchn);
		if(ret != 0)
		{
			printf("SAMPLE_COMM_VDA_OdStart chnidx %d err!\n",i);
			return -1;
		}
	}

	return pthread_create(&vda_thread_id,attr, vda_proc_thread, NULL);
#endif
 	return 0;
}


static int vadc_cnt=0;
static int set_scr_cnt = 2; //为防止videoenc初始化2835时
void vadc_second_proc(void)
{
	int i;
	//char cmd[100];
	//time_t timep;
	//struct tm *p;
	
	if(get_quad_flag()==1)//为2835重新初始化切屏
	{
		if((set_scr_cnt <20)&&(((++set_scr_cnt) % 3) == 0))
			{	
				//set_tw2835_screen(NET_CH,get_mainpara()->vadc.quad.current_net_ch);
				//set_tw2835_screen(LOCAL_CH,get_mainpara()->vadc.quad.current_local_ch);
			}	
	}	
	return ;
}

