#ifndef UART_CTRL_H
#define UART_CTRL_H
/////////////////////////////////////////////////////////
/*对内部串口1，2 和 外扩串口1，2，3，4 进行编号*/
#define	INT_COM1		0
#define	INT_COM2		1
#define	EXT_COM0		2
#define	EXT_COM1		3
#define	EXT_COM2		4
#define	EXT_COM3		5
#define   COM_SET		6
/////////////////////////////////////////////////////
/*建立Socket 连接的通讯命令字*/
#define	SETTING_ERR		"setting error\n"	//串口参数设置错误
#define	SOCKETBUSSY		"bussy\n"			//设备端口忙
#define	SOCKETCONNECT		"connected\n"		//已经成功连接

////////////////////////////////////////////////////
/*Socket 通讯的路径*/
#define  IntUartSockPath1		 "/dev/intcom1"		//内部通用串口1
#define  IntUartSockPath2		 "/dev/intcom2"		//内部通用串口1
#define  ExtUartSockPath1	 "/dev/extcom0"		//外部扩展串口1
#define  ExtUartSockPath2	 "/dev/extcom1"		//外部扩展串口2
#define  ExtUartSockPath3	 "/dev/extcom2"		//外部扩展串口3
#define  ExtUartSockPath4	 "/dev/extcom3"		//外部扩展串口4
#define  ExtUartSetPath 		 "/dev/setbaud"		//串口参数设置通道

//void PrintBuf(char *buf, int len);  //test
//int CreateSockMonThread(void);
//int  InitComPortPara(void);	  //初始化变量；
//Errno :  -ENXIO  -EBUSY
int  InitComPortCtrlDev(void);	  //初始化变量；
int  FreeComPortCtrlDev(void);	  //释放串口控制设备

//Errno :  -ENXIO  -EBUSY
int OpenComPort(int ComIndex); // 打开一个串口, 返回控制句柄 com_fd；

// 设置串口参数
//Errno :  -ENXIO  -EBUSY  -EINVAL
int SetComPort(int ComIndex , unsigned long int baudrate, char parity , int data , int stop); 
//关闭串口
//Errno :  -ENXIO  -EBUSY 
int CloseComPort(int ComIndex);	// 关闭一个串口；
//读串口
//Errno :  -ENXIO  -EBUSY  -EINVAL  -EPIPE 
int ReadComPort(int ComFd, char *Buf , int Len); // 从一个串口读取数据
//写串口
//Errno :  -ENXIO  -EBUSY  -EINVAL  -EPIPE - EFBIG
int WriteComPort(int ComFd, char *Msg , int Len); // 写数据到一个串口

#endif

