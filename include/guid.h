//GUID.h
#ifndef _GUID_TAO_20050314_H_
#define _GUID_TAO_20050314_H_
#define GUID_HEX_LEN 16
#include <stdlib.h>
#include <ctype.h>
#ifndef BYTE
#define BYTE	unsigned char
#endif
struct GT_GUID{
	BYTE serial[4]; //流水号
	BYTE fix[3];		//固定信息
	BYTE year;
};

/************************************************************************/
/* 将GT_GUID转换十六进制字符串						*/
/************************************************************************/
static __inline__ void guid2hex(struct GT_GUID guid,char * hex)
{
	const char *  HEX_TABLE = "0123456789abcdef";
	BYTE * p = (BYTE *)&guid + sizeof(guid) - 1;
	int j = 0;

	while(j < GUID_HEX_LEN ) 
	{
		hex[j++] =  HEX_TABLE[*p >> 4];
		hex[j++] =  HEX_TABLE[*p & 0x0f];
		p--;
	}
	hex[j] = 0;
};

/************************************************************************/
/* 将十六进制字符串转换为GT_GUID					*/
/************************************************************************/
static __inline__ struct GT_GUID hex2guid(char * hexv)
{
	const char *  HEX_TABLE = "0123456789abcdef";
	char			hex[GUID_HEX_LEN*3];
	struct GT_GUID guid;
	int j = sizeof(guid) - 1;
	int i;
	char * p = (char*)&guid;
	char tmp[2];
	tmp[1] = 0;

	//if(strlen(hex) != GUID_HEX_LEN)
	//	return guid;
	memset(hex,0,sizeof(hex));
	i=strlen(hexv);
	if(i>GUID_HEX_LEN)
		i=GUID_HEX_LEN;	
	memcpy(hex,hexv,i);
	hex[GUID_HEX_LEN]='\0';
	memset(p,0,sizeof(guid));	
	for(i=0;i< GUID_HEX_LEN;i++) 
	{
		tmp[0] = hex[i++];
		if(isupper(tmp[0]))
			tmp[0]=tolower(tmp[0]);
		p[j] =  (BYTE)strcspn(HEX_TABLE,tmp)<< 4; 

		tmp[0] = hex[i];
                if(isupper(tmp[0]))
                        tmp[0]=tolower(tmp[0]);

		p[j--] |= (BYTE)strcspn(HEX_TABLE,tmp);
	}	
	return guid;
};

/************************************************************************/
/* 创建一个新的GUID							*/
/* arguments: 								
	unsigned int current	当前的流水号
	int year				两位的年		*/
/************************************************************************/
static __inline__ struct GT_GUID new_guid(unsigned int current,int year)
{
	const BYTE  GUID_PROTO[8] = {0x0,0x0,0x0,0x0,'t',0x0,'g',0x0};
	struct GT_GUID guid;
	char * p = (char*)&guid;
//	unsigned long  s = current + 1;	//lsk 2007-2-14
	memcpy(p,GUID_PROTO,sizeof(GUID_PROTO));

	memcpy(p,&current,sizeof(unsigned long));
	p[sizeof(guid) - 1] = (BYTE)(year);
	return guid;
};

#endif
