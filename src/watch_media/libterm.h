#ifndef _LIBTERM_H
#define	_LIBTERM_H
#include "ansi_tty.h"
#define	C_WHITE		WHT	/* 白色 */
#define	C_RED			RED	/* 红色 */
#define	C_GREEN		GRN	/* 绿色 */
#define 	C_YELLOW		YEL	/* 黄色 */
#define 	C_BLUE			BLU	/* 蓝色 */
#define 	C_MAGENTA		MAG	/* 紫色 */
#define	C_CYAN			CYN	/* 青色 */
#define	C_BLACK		BLK	/* 黑色 */

#define	C_HWHITE		HIW	/* 亮白色 */
#define	C_HRED			HIR	/* 亮红色 */
#define	C_HGREEN		HIG	/* 亮绿色 */
#define 	C_HYELLOW		HIY	/* 亮黄色 */
#define 	C_HBLUE		HIB	/* 亮蓝色 */
#define 	C_HMAGENTA	HIM	/* 亮紫色 */
#define	C_HCYAN		HIC	/* 亮青色 */

#define C_NORMAL        NOM

int 	InitTerminal(void);
void	RestoreTerminal(void);
void	ClearTermScr(void);

//CColor:字符颜色
//Attr:	0表示正常显示 1表示反白
#define	WriteTermStr(CColor,Attr,Arg...)	do { printf(CColor);if(Attr){printf(REV);} printf(Arg);printf(NOM);}while(0)//don't use printf(##Arg) in gcc 3.4.1
//清屏
#define	ClearTermScr()		do{printf("%s%s",CLR,HOME);}while(0)

#endif
