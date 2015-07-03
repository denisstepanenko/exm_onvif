#ifndef TESTMOD_H
#define TESTMOD_H
#include "testIDE.h" 
/*
#include "testdsp.h"
#include "test6410.h"
#include "test2834.h"
*/
#include "multicast_ctl.h"

#define NETENC 1
#define HQENC   0
#define IDE_DEV 		"/dev/hda"

struct dev_test_struct
{
        int ide_stat;		//Ó²ÅÌ»òcf¿¨²âÊÔ½á¹û
        int netenc_stat;	//ÍøÂçÊÓÆµ±àÂëÆ÷²âÊÔ½á¹û
        int hqenc0_stat;	//¸ßÇåÎúÂ¼Ïñ0Â·Ğ¾Æ¬²âÊÔ½á¹û
        int audio_stat;		//ÉùÒôĞ¾Æ¬²âÊÔ½á¹û
        int quad_stat;		//»­Ãæ·Ö¸îĞ¾Æ¬²âÊÔ½á¹û
        int tw9903_stat;
        int rtc_stat;
        int usb_stat;
};
void print_stat(int err, int stat, FILE * fp);
void print_code(int err, int stat, FILE * fp);
int save_result_to_file(char *filename,struct dev_test_struct *devstat,multicast_sock* ns);
int testsim(struct dev_test_struct *stat, multicast_sock* net_st, int prog);
/*
int test_IDE(void);
int test_Quad(void);
int test_6410(int channal);
int test_dsp(void);
*/
#endif
