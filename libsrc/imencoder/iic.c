
/*
 *  Title : iic.c
 * 
 *  Note : Setting FPGA and Video Decoder of Each Channel
 *    
 *  Author : 
 *    N.H Jung       (fatale@intime.co.kr)
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
#include <stdio.h>
#include <sys/ioctl.h>
#include "iic.h"
#include "ime6410.h"

// Set Value To FPGA
void WriteUserIICTab(int devfd, UserIICTab* pUserIIC, int dev_id)
{
	int flag;
	int cnt;

	ioctl(devfd, REG_I2CCTRL|1, 0x1);
	while (pUserIIC->user_iic_ctrl) {
		ioctl(devfd, REG_I2CDATA|1, pUserIIC->user_iic_data);
		ioctl(devfd, REG_I2CCTRL|1, pUserIIC->user_iic_ctrl | (dev_id<<16) | 0x00000003);
		
		cnt = 65536;
		while (cnt--) {
			if (!((flag = ioctl(devfd, REG_I2CCTRL, NULL))&0x00000002))
				break;
		}
		if(!cnt)
			printf("iic write error\n");
		pUserIIC++;
	}
//	ioctl(devfd, REG_I2CCTRL|1, 0x0);
}

void OpenUserIIC(int devfd)
{
	int  dwData;

	dwData = 0x00000001;
	ioctl(devfd, REG_I2CCTRL|1, dwData);
}

void WriteUserIIC(int devfd, int dev_id, int addr, int nbyte, int data, int mode)
{
	int  dwData, cnt;

	ioctl(devfd, REG_I2CDATA|1, data);//将data放进DATA
	dwData = (addr<<24) | (dev_id<<16) | (nbyte<<8) | (mode<<4) | 0x00000003;
	ioctl(devfd, REG_I2CCTRL|1, dwData); //执行写
	
	cnt = 65536;
	while (cnt--) {
		if(!((dwData = ioctl(devfd, REG_I2CCTRL, NULL))&0x00000002))
			break;
	}
	
	if(!cnt) {
		printf("[IIC] IIC Write Error\n");
	}
}
//add by wsy, try to read useriic
int ReadUserIIC(int devfd, int dev_id, int addr, int nbyte, int mode)
{
	int dwData;
	int value=0;
	int cnt;

	mode=1;

	ioctl(devfd,REG_I2CDATA|1,0x55555555);
	value=ioctl(devfd, REG_I2CDATA,NULL);
	printf("read out value 0x%8x",value);
	/*
	dwData = (addr<<24) | ((dev_id|1)<<16) | (nbyte<<8) | (mode<<4) | 0x00000003;
	printf("dwData is 0x%08x\n",dwData);

	ioctl(devfd, REG_I2CCTRL|1, dwData);//ctrl操作为读

	value=ioctl(devfd, REG_I2CDATA,NULL);

	printf("read out value now 0x%8x\n",value);
	
	

	

	WriteUserIIC(devfd, dev_id, addr,1, 0xA1,0);

	value=ioctl(devfd, REG_I2CDATA,NULL);

	printf("|0x%8x",value);

	
	cnt = 65536;
	while (cnt--) {
		if(!((dwData = ioctl(devfd, REG_I2CCTRL, NULL))&0x00000002))
			break;
	}
	if(!cnt) {
		printf("[IIC] IIC Write Error\n");
	}

	value=ioctl(devfd, REG_I2CDATA,NULL);
	printf("| 0x%8x",value);
*/
	dwData = (addr<<24) | ((dev_id|1)<<16) | (nbyte<<8) | (mode<<4) | 0x00000003;
	ioctl(devfd, REG_I2CCTRL|1, dwData);//ctrl操作为读

	value=ioctl(devfd, REG_I2CDATA,NULL);
	printf("| 0x%8x",value);

	cnt = 65536;
	while (cnt--) {
		if(!((dwData = ioctl(devfd, REG_I2CCTRL, NULL))&0x00000002))
			break;
	}
	if(!cnt) {
		printf("[IIC] IIC Write Error\n");
	}

	
	
	value=ioctl(devfd, REG_I2CDATA,NULL); //从DATA读出
	printf("| 0x%8x\n",value);
	return value;
}

void CloseUserIIC(int devfd)
{
	int  dwData;

	dwData = 0;
	ioctl(devfd, REG_I2CCTRL|1, dwData);
}

void FPGAChannelDisable(int devfd)
{
	WriteUserIIC(devfd, 0x48, 0x01, 2, 0x00000000, 0);
}

void FPGAChannelEnable(int devfd, int ch1, int ch2, int ch3, int ch4)
{
	int  data;

	data = (ch4<<19) | (ch3<<18) | (ch2<<17) | (ch1<<16);
	WriteUserIIC(devfd, 0x48, 0x01, 2, data, 0);
}

void FPGAChannelSet(int dev, int ch, int width, int height, int framediv)
{
	int  addr, field_ctrl;

	addr = ch * 0x20 + 0x20;

	field_ctrl = 0x01000000;
	if(height <= 288)
		field_ctrl |= 0x00010000;
	else {
		field_ctrl |= 0x00020000;
		height >>= 1;
	}
	field_ctrl |= (framediv<<18);

	WriteUserIIC(dev, 0x48, addr, 2, 0x00000000, 0);
	WriteUserIIC(dev, 0x48, addr+1, 2, ((width-1)<<16), 0);
	WriteUserIIC(dev, 0x48, addr+2, 2, 0x00000000, 0);
	WriteUserIIC(dev, 0x48, addr+3, 2, ((height-1)<<16), 0);
	WriteUserIIC(dev, 0x48, addr+4, 2, field_ctrl, 0);
	WriteUserIIC(dev, 0x48, addr+5, 2, (width<<16), 0);
	WriteUserIIC(dev, 0x48, addr+6, 2, (height<<16), 0);
}


