#ifndef __GPIO_H__
#define __GPIO_H__

typedef struct TimeDelay
{
        unsigned char beginToEnd[8];         //0-1的延时
        unsigned char endToBegin[8];         //1-0的延时
} __attribute__((packed))TimeDelay;

#endif
