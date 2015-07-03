#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/file.h>
#include <termios.h>


/*
*****************************************************
*函数名称: testUSB
*函数功能: 我们检测本公司的key
*参数	: 无
*返回值   : 0 成功 -1 失败
*
*****************************************************
*/
#define USB_TEST_BUF_LEN 256
int testUSB()
{
    FILE* fp=NULL;
    int    ret;
    char temp[USB_TEST_BUF_LEN];
    int    totall = 0;
    char path[] = "/tmp/usbtest.txt";

    system("lsusb|grep ""096e:0302""|wc -l > /tmp/usbtest.txt");

    fp = fopen(path, "rb");
    if(fp==NULL)
    {
        printf("open read error %s\n", path);
        return 1;
    }

    memset(temp, 0, sizeof(temp));
    ret = fread(temp, sizeof(temp),1,fp);
    if(ret < 0)
    {
        fclose(fp);
        printf("read file %s error ,ret:%d \n", path,ret);
        return 1;
    }

   printf("read file %s,%s ret:%d \n", path, temp, ret);  
   totall = atoi(temp);
   if(totall != 2)
   {
        fclose(fp);
        printf("USB file test ok\n");
        return 1;
   }

    fclose(fp);
    system("rm /tmp/usbtest.txt");
    printf("USB file test ok\n");
    return 0;
    
}


#if 0
int testUSB()
{
	
    int usbfd1,usbfd2;
    int harddisk = 0;
    char pbuf[] = "test usb   ";
    int writted,readbytes;

       
    printf("请插入测试盘插入USB接口\n");

    if(access("/hqdata/sda1", F_OK) == 0 )//不存在
    {
        harddisk = 1;
    }
    else
    {
        harddisk = 0;
    }       
    
    if(harddisk == 0)
    {
        system("umount /hqdata/sda*");
        system("umount /hqdata/sdb*");
        usbfd1 = open("/dev/sda", O_RDWR);
        usbfd2 = open("/dev/sdb", O_RDWR);
        printf("usb use sda and sdb\n");
    }
    else
    {
        system("umount /hqdata/sdb*");
        system("umount /hqdata/sdc*");
        usbfd1 = open("/dev/sdb", O_RDWR);
        usbfd2 = open("/dev/sdc", O_RDWR);
        printf("usb use sdb and sdc\n");
    }
    
    if ((usbfd1 < 0)||(usbfd2 < 0))
    {
        printf("open error %s %d %s\n", __FILE__, __LINE__, strerror(errno));
        return (1);
    }

    writted = write(usbfd1,pbuf,strlen(pbuf)+1);
    if(writted<=0) /* 出错了*/ 
    { 
        printf("write error %s %d %s\n", __FILE__, __LINE__, strerror(errno));
        return (2);
    } 
    writted = write(usbfd2,pbuf,strlen(pbuf)+1);
    if(writted<=0) /* 出错了*/ 
    { 
        printf("write error %s %d %s\n", __FILE__, __LINE__, strerror(errno));
        return (3);
    }     
    readbytes = read(usbfd1,pbuf,5);
    if(readbytes <= 0) /* 出错了*/ 
    { 
        printf("read error %s %d %s\n", __FILE__, __LINE__, strerror(errno));
        return (4);
    } 
    readbytes = read(usbfd2,pbuf,5);
    if(readbytes <= 0) /* 出错了*/ 
    { 
        printf("read error %s %d %s\n", __FILE__, __LINE__, strerror(errno));
        return (5);
    }    
    
    close(usbfd1);
    close(usbfd2);
    return 0;
    
}
#endif    

