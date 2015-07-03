#ifndef AUDIOOUT_API_H
#define AUDIOOUT_API_H
#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long

#define AUDIOOUT_MAJOR     255 	
#define AUDIOOUT_NAME      "LEDS"
#define AUDIOOUT_VERSION   "Version 0.1"


//0.1 对3路io控制，实现音频下行8选1


#define SCULL_IOC_MAGIG 'k'

#define SET_0       _IOW(SCULL_IOC_MAGIG,100,char)
#define SET_1       _IOW(SCULL_IOC_MAGIG,101,char)
#define SET_2       _IOW(SCULL_IOC_MAGIG,102,char)
int choose_enable(int ch);
#endif
