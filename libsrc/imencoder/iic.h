
/*
 *  Title : iic.h
 * 
 *  Note : 
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


#ifndef _IIC_H_
#define _IIC_H_
#ifndef IME6410Arg
typedef struct {
        unsigned int address;
        unsigned int data;
} IME6410Arg;
#define IME6410_IOCTL_BASE         1000 //changed by shixin 0

#define IME6410_IOCTL_READ              IME6410_IOCTL_BASE + 0//changed by shixin _IOR('v', IME6410_IOCTL_BASE + 0, IME6410Arg)
#define IME6410_IOCTL_WRITE             IME6410_IOCTL_BASE +1 //changed by shixin _IOW('v', IME6410_IOCTL_BASE + 1, IME6410Arg)
#if 0   /* ??¨¤? ??????¨¢? ?¨º¡ä? IOCTL */
#define IME6410_IOCTL_MSC2
#define IME6410_IOCTL_RESET
#define IME6410_IOCTL_READH
#define IME6410_IOCTL_WRITEH
#define IME6410_IOCTL_IRQENABLE
#define IME6410_IOCTL_IRQDISABLE
#endif
#define IME6410_IOCTL_RESET             IME6410_IOCTL_BASE + 8 
#endif


// address[31:24] | dev_id[23:16] | wsize[15:8] | write/done[1] start/end[0]
	
typedef struct _UserIICTab
{
	int	user_iic_ctrl;	
	int	user_iic_data;		// big-endian
	
} UserIICTab;

extern void WriteUserIICTab(int devfd, UserIICTab* pUserIIC, int dev_id);
extern void OpenUserIIC(int devfd);
extern void CloseUserIIC(int devfd);
extern int ReadUserIIC(int devfd, int dev_id, int addr, int nbyte, int mode);
extern void WriteUserIIC(int devfd, int dev_id, int addr, int nbyte, int data, int mode);
extern void FPGAChannelDisable(int devfd);
extern void FPGAChannelEnable(int devfd, int ch1, int ch2, int ch3, int ch4);

extern void FPGAChannelSet(int dev, int ch, int width, int height, int framediv);
#endif
