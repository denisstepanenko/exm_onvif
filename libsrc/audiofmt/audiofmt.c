#include "audiofmt.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/soundcard.h>
#include <typedefine.h>
#include "ulaw16.h"
#include "string.h"
#include <errno.h>
#ifndef DWORD
#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#endif

/**********************************************************************************************
 * 函数名	:conv_raw2ulaw()
 * 功能	:将raw-pcm格式的数据转换为u-law的数据
 * 输入	:source:存放raw-pcm数据的源缓冲区指针
 *			 len:源缓冲区中的有效字节数
 * 输出	:target:存放转换后u-law数据的缓冲区 
 * 返回值	:正值表示成功转换后target缓冲区中的有效字节数 负值表示出错
  **********************************************************************************************/
int conv_raw2ulaw(char* target,char *source,int len)
{
	unsigned char *t=target;
	unsigned short *s=(unsigned short*)source;
	int			l=len/2;
	int			i;
	for(i=0;i<l;i++)
	{
		*t=linear2ulaw(*s);
		t++;
		s++;
	}
	return l;
}

#if 0	
/**********************************************************************************************
 * 函数名	:conv_data()
 * 功能	:将单声道,u-law数据块转换成指定字节数的数据块
 * 输入	:source:存放u-law数据的源缓冲区指针
 *		tgtlen:目标字节数
 *		srclen:源缓冲区中的有效字节数		
 * 输出	:target:存放转换后数据的缓冲区 
 * 返回值	:正值表示成功转换后target缓冲区中的有效字节数 负值表示出错
  **********************************************************************************************/
int conv_data(char* target,char *source,int tgtlen, int srclen)
{
	unsigned short *t=(unsigned short*)target;
	unsigned char *s=source;
	int i;
	int interval; //特殊处理的周期
	int convlen=0;
	
	if(tgtlen==srclen)//相等
	{
		for(i=0;i<srclen;i++)
		{
			*t=*s;
			t++;
			s++;
		}
		return srclen;
	}
	if(tgtlen>srclen)//需要拉长
	{
		interval=srclen/(tgtlen-srclen);
		//printf("%s-%d\n",__FILE__,__LINE__);
		for(i=0;i<srclen;i++)
		{
			if(i%interval!=0)
			{
				*t=*s;
				t++;
				convlen++;
				s++;
			}
			else
			{
				*t=*s;
				t++;
				s++;
				*t=*(s-1);
				t++;
				convlen++;
				convlen++;
			}
			
		}
		//printf("%s-%d\n",__FILE__,__LINE__);
		return convlen;
	}
	if(tgtlen<srclen)//需要压缩
	{
		interval=srclen/(srclen-tgtlen);
		for(i=0;i<srclen;i++)
		{
			if(i%interval!=0)
			{
				*t=*s;
				t++;
				s++;
				convlen++;
			}
			else
			{
				s++;
			}
		}
		return convlen;

	}
	return 0;
}


int conv_data_new(char* target,char *source,int len)
{
	char *t=(char *)target;
	char *s=source;
	int i;
	int interval=4096;
	//char ch1,ch2,ch3,ch4;
	for(i=0;i<len;i=i+interval)
	{
		
		memcpy(t,s+len-interval,interval);
		t=t+interval;
	}

	ch1=*(s+len-4);
	ch2=*(s+len-3);
	ch3=*(s+len-2);
	ch4=*(s+len-1);
		
	
		for(i=0;i<len;i=i+4)
		{
			*t=ch1;
			t++;
			*t=ch2;
			t++;
			*t=ch3;
			t++;
			*t=ch4;
			t++;
		}

		printf("\n\nnow s:\n");
		s=(char*)source;
		for(i=0;i<len;i++)
		{
			printf("%02X ",*s);
			s++;
		}
		printf("\n\nnow t:\n");
		t=(char *)target;
		for(i=0;i<len;i++)
		{
			printf("%02X ",*t);
			t++;
		}

		return len;
}
#endif



/**********************************************************************************************
 * 函数名	:conv_ulaw2raw()
 * 功能	:将u-law格式的数据转换为raw-pcm的数据
 * 输入	:source:存放u-law数据的源缓冲区指针
 *			 len:源缓冲区中的有效字节数
 * 输出	:target:存放转换后raw-pcm数据的缓冲区 
 * 返回值	:正值表示成功转换后target缓冲区中的有效字节数 负值表示出错
  **********************************************************************************************/
int conv_ulaw2raw(char* target,char *source,int len)
{
	unsigned short *t=(unsigned short*)target;
	unsigned char *s=source;
	int			l=len;
	int			i;
	for(i=0;i<l;i++)
	{
		*t=ulaw2linear(*s);
		t++;
		s++;
	}
	return l*2;
}

/**********************************************************************************************
 * 函数名	:conv_mono2stereo()
 * 功能	:将单声道的raw-pcm数据转换成双声道
 * 输入	:src:指向数据源缓冲区的指针
 *			 srclen:数据源缓冲区中的有效字节数
 * 输出	:target:转换结果填充的目标缓冲区指针
 * 返回值	:正值表示转换后target缓冲区中的有效字节数 负值表示出错
 **********************************************************************************************/
int conv_mono2stereo(char *src,char* target,int srclen)
{
	int i,len;
	WORD *ps,*pt;
	int convlen;
	if((src==NULL)||(target==NULL))
	{
		return -EINVAL;
	}
#if 0
	ps=(WORD*)src;
	pt=(WORD*)target;
	len=srclen/2;
	convlen=0;
	for(i=0;i<len;i++)
	{
		*pt=*ps;
		 pt++;
		*pt=*ps;
		 pt++;
		 ps++;
		 convlen++;
	}
	return convlen*4;//srclen*2;
#else
	//changed by shixin 支持覆盖(源缓冲区和目标缓冲区使用相同的缓冲区)
       len=srclen/2;
       ps=(WORD*)src;
	ps=&ps[len-1];
	pt=(WORD*)target;
       pt=&pt[len*2-1];
       
	convlen=0;
	for(i=0;i<len;i++)	
	{
		*pt=*ps;
		pt--;
		*pt=*ps;
		pt--;
		ps--;
		convlen++;
	}
	return convlen*4;
#endif
}

/**********************************************************************************************
 * 函数名	:conv_stereo2left()
 * 功能	:将双声道的raw-pcm数据转换成单声道(左声道)
 * 输入	:src:指向数据源缓冲区的指针
 *			 srclen:数据源缓冲区中的有效字节数
 * 输出	:target:转换结果填充的目标缓冲区指针
 * 返回值	:正值表示转换后target缓冲区中的有效字节数 负值表示出错
 **********************************************************************************************/
int conv_stereo2left(char *src,char* target,int srclen)
{
	int i,len;
	WORD *ps,*pt;
	int convlen;
	if((src==NULL)||(target==NULL))
	{
		return -EINVAL;
	}
	ps=(WORD*)src;
	pt=(WORD*)target;
	len=srclen/4;
	convlen=0;
	for(i=0;i<len;i++)
	{
		*pt=*ps;
		 pt++;
		 ps=ps+2;
		 convlen++;
	}
	return convlen*2;
}

/**********************************************************************************************
 * 函数名	:conv_stereo2right()
 * 功能	:将双声道的raw-pcm数据转换成单声道(右声道)
 * 输入	:src:指向数据源缓冲区的指针
 *			 srclen:数据源缓冲区中的有效字节数
 * 输出	:target:转换结果填充的目标缓冲区指针
 * 返回值	:正值表示转换后target缓冲区中的有效字节数 负值表示出错
 **********************************************************************************************/
int conv_stereo2right(char *src,char* target,int srclen)
{
	int i,len;
	WORD *ps,*pt;
	int convlen;
	if((src==NULL)||(target==NULL))
	{
		return -EINVAL;
	}
	ps=(WORD*)src;
	pt=(WORD*)target;
	len=srclen/4;
	convlen=0;
	ps++;
	for(i=0;i<len;i++)
	{
		*pt=*ps;
		 pt++;
		 ps=ps+2;
		 convlen++;
	}
	return convlen*2;
}
#ifdef ALAW
/**********************************************************************************************
 * 函数名	:conv_raw2alaw()
 * 功能	:将raw-pcm格式的数据转换为a-law的数据
 * 输入	:source:存放raw-pcm数据的源缓冲区指针
 *			 len:源缓冲区中的有效字节数
 * 输出	:target:存放转换后a-law数据的缓冲区 
 * 返回值	:正值表示成功转换后target缓冲区中的有效字节数 负值表示出错
  **********************************************************************************************/
int conv_raw2alaw(char* target,char *source,int len)
{
	unsigned char *t=target;
	unsigned short *s=(unsigned short*)source;
	int			l=len/2;
	int			i;
	int  sign   = 0;
       int  exponent  = 0;
       int  mantissa  = 0;

	for(i=0;i<l;i++)
	{
    		sign = ((~(*s)) >> 8) & 0x80;
    		if (sign == 0)
    		{
       		 *s = (unsigned short)-(*s);
    		}
    		if (*s > 0x7F7B)
    		{
        		*s = 0x7F7B;
    		}
   		if (*s >= 0x100)
    		{
        		exponent  = (int)dsp16_alaw[((*s) >> 8) & 0x7F];
        		mantissa  = ((*s) >> (exponent + 3)) & 0x0F;
        		*t = (unsigned char)((exponent << 4) | mantissa);
    		}
    		else
    		{
        		*t = (unsigned char)((*s) >> 4);
    		}
    		(*t) ^= (unsigned char)(sign ^ 0x55);

    		t++;
    		s++;
	}
	
   	 return l;
}

/**********************************************************************************************
 * 函数名	:conv_alaw2raw()
 * 功能	:将a-law格式的数据转换为raw-pcm的数据
 * 输入	:source:存放a-law数据的源缓冲区指针
 *			 len:源缓冲区中的有效字节数
 * 输出	:target:存放转换后raw-pcm数据的缓冲区 
 * 返回值	:正值表示成功转换后target缓冲区中的有效字节数 负值表示出错
  **********************************************************************************************/
int conv_alaw2raw(char* target,char *source,int len)
{
	unsigned short *t=(unsigned short*)target;
	unsigned char *s=source;
	int			l=len;
	int			i;
	for(i=0;i<l;i++)
	{
		*t=alaw2linear(*s);
		t++;
		s++;
	}
	return l*2;
}

#endif

