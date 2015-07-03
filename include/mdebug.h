
#ifndef _MDEBUG_H_
#define _MDEBUG_H_

#include <stdio.h>

// Video Booting Mode
////////////////////////////////////////////////////
//#undef USEIIC
#define USEIIC


// DEBUG Message 
////////////////////////////////////////////////////

#define MDEBUG 2//0

#if (MDEBUG == 2)

// DEBUG LEVEL HIGH
#define  DBGH(string,args...) printf(string, ##args)
#define  DBGL(string,args...) printf(string, ##args)
#define  ERR(string,args...) printf(string, ##args)
#define  MSG(string,args...) printf(string, ##args)

#elif (MDEBUG == 1)

// DEBUG LEVEL LOW
#define  DBGH(string, args...)
#define  DBGL(string, args...) printf(string, ##args)
#define  ERR(string,args...) printf(string, ##args)
#define  MSG(string,args...) printf(string, ##args)

#else

// NO DEBUG
#define  DBGH(string, args...)
#define  DBGL(string, args...)
#define  ERR(string, args...) printf(string, ##args)
#define  MSG(string,args...) printf(string, ##args)

#endif

#endif
