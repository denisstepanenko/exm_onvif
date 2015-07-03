#ifndef		__G711_H__
#define		__G711_H__

#include "sys_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned char linear2alaw(int pcm_val);
int alaw2linear(unsigned char a_val);
unsigned char linear2ulaw(int pcm_val);
int ulaw2linear(unsigned char u_val);
unsigned char alaw2ulaw(unsigned char aval);
unsigned char ulaw2alaw(unsigned char uval);

#ifdef __cplusplus
}
#endif

#endif	//	__G711_H__
